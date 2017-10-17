//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_class.cc
//
//  Contains zmap class and other main code for ZQuest.
//
//--------------------------------------------------------

/************************/
/****** ZMAP class ******/
/************************/

#include "zq_class.h"
#include "zq_misc.h"
#include "zquest.h"
#include "qst.h"
#include "colors.h"
#include "tiles.h"
#include "zquestdat.h"
#include "zsys.h"
#include "sprite.h"
#include "items.h"
#include <string.h>
#include "maps.h"
#include "zc_sys.h"

zmap Map;

void reset_dmap(int index)
{
  bound(index,0,MAXDMAPS-1);
  memset(&DMaps[index],0,sizeof(dmap));
  sprintf(DMaps[index].title, "                    ");
  sprintf(DMaps[index].intro, "                                                                        ");
}

void reset_dmaps()
{
  for(int i=0; i<MAXDMAPS; i++)
    reset_dmap(i);
}


zmap::zmap() { can_undo=can_paste=false; }
bool zmap::CanUndo() { return can_undo; }
bool zmap::CanPaste() { return can_paste; }
int  zmap::CopyScr() { return (copymap<<8)+copyscr; }
int zmap::getMapCount() {return map_count;}
bool zmap::isDungeon(int scr)
{
  for(int i=0; i<4; i++)
  {
    if(screens[scr].data[i]!=screens[TEMPLATE].data[i])
    {
      return false;
    }
  }
  return true;
}

bool zmap::clearall(bool validate)
{
  Color=0;
  char tbuf[10];
  if ((header.templatepath[0]!=0)&&validate)
  {
    if (!valid_zqt(header.templatepath))
    {
      jwin_alert("Error","Invalid Quest Template",NULL,NULL,"O&K",NULL,'k',0,lfont);
      return false;
    }
  }
  for(int i=0; i<map_count; i++)
  {
    setCurrMap(i);
    sprintf(tbuf, "%d", i);
    clearmap(true);
  }
  setCurrMap(0);
  return true;
}

bool zmap::reset_templates(bool validate)
{
  //why are we doing this?
  if(colordata==NULL)
  {
    return false;
  }

  char *deletefilename;
  deletefilename=(char *)malloc(1);
  deletefilename[0]=0;

  //int ret;
  word version, build, dummy;
  //long section_size;
  word temp_map_count;
  mapscr temp_mapscr;
  PACKFILE *f=NULL;

  packfile_password(datapwd);
  f=open_quest_template(&header, deletefilename, validate);
  get_version_and_build(f, &version, &build);

  if (!find_section(f, ID_MAPS))
  {
    packfile_password(NULL);
    return false;
  }

  //section version info
  if(!p_igetw(&dummy,f,true))
  {
    return false;
  }
  if(!p_igetw(&dummy,f,true))
  {
    return false;
  }

  //section size
  if(!p_igetl(&dummy,f,true))
  {
    return false;
  }

  //finally...  section data
  if(!p_igetw(&temp_map_count,f,true))
  {
    return false;
  }

  for (int i=0; i<MAPSCRSNORMAL; ++i)
  {
    readmapscreen(f, &header, &temp_mapscr);
  }
  readmapscreen(f, &header, TheMaps+128);
  readmapscreen(f, &header, TheMaps+129);
  for (int i=0; i<(MAPSCRS-(MAPSCRSNORMAL+2)); ++i)
  {
    readmapscreen(f, &header, &temp_mapscr);
  }
  for (int i=0; i<MAPSCRSNORMAL; ++i)
  {
    readmapscreen(f, &header, &temp_mapscr);
  }
  readmapscreen(f, &header, TheMaps+MAPSCRS+128);
  readmapscreen(f, &header, TheMaps+MAPSCRS+129);

  pack_fclose(f);
  if (deletefilename[0]==0)
  {
    delete_file(deletefilename);
  }
  packfile_password(NULL);

  return true;
}

bool zmap::clearmap(bool newquest)
{
  if(currmap<map_count)
  {
    for(int i=0; i<MAPSCRS-(newquest?0:TEMPLATES); i++)
    {
      clearscr(i);
    }
    setCurrScr(0);
    if (newquest)
    {
      if (!reset_templates(false))
      {
        jwin_alert("Error","Error resetting","template screens.",NULL,"O&K",NULL,'k',0,lfont);
      }
    }
  }
  return true;
}

mapscr* zmap::CurrScr()                     { return screens+currscr; }
mapscr* zmap::Scr(int scr)                  { return screens+scr; }
mapscr* zmap::AbsoluteScr(int scr)          { return TheMaps+scr; }
mapscr* zmap::AbsoluteScr(int map, int scr) { return TheMaps+(map*MAPSCRS)+scr; }
int zmap:: getCurrMap()                     { return currmap; }
bool zmap::isDark()                         { return screens[currscr].flags&4; }
void zmap::setCurrMap(int index)
{
  int oldmap=currmap;
  scrpos[currmap]=currscr;
  currmap=bound(index,0,map_count);
  screens=TheMaps+(currmap*MAPSCRS);

  currscr=scrpos[currmap];
  loadlvlpal(getcolor());

  if(currmap!=oldmap)
    can_undo=false;
}

int zmap:: getCurrScr()        { return currscr; }
void zmap::setCurrScr(int scr)
{
  if (scr==currscr) return;

  int oldscr=currscr;
  int oldcolor=getcolor();
  if (!(screens[currscr].valid&mVALID))
  {
    oldcolor=-1;
  }
  currscr=bound(scr,0,MAPSCRS-1);
  int newcolor=getcolor();
  loadlvlpal(newcolor);
  //setcolor(newcolor);
  if (!(screens[currscr].valid&mVALID))
  {
    newcolor=-1;
  }

  if (newcolor!=oldcolor)
  {
    rebuild_trans_table();
  }
  if(currscr!=oldscr)
    can_undo=false;
}

void zmap::setcolor(int c)
{
  if(screens[currscr].valid&mVALID)
  {
    screens[currscr].color = c;
    if(Color!=c)
    {
      Color = c;
      loadlvlpal(c);
    }
  }
}

int zmap::getcolor()
{
  return screens[currscr].color;
}

void zmap::resetflags()
{
  byte *di=&(screens[currscr].valid);
  for(int i=1; i<48; i++)
    *(di+i)=0;
}

word zmap::tcmbdat(int pos)
{
  return screens[TEMPLATE].data[pos];
}

word zmap::tcmbcset(int pos)
{
  return screens[TEMPLATE].cset[pos];
}

int zmap::tcmbflag(int pos)
{
  return screens[TEMPLATE].sflag[pos];
}

word zmap::tcmbdat2(int pos)
{
  return screens[TEMPLATE2].data[pos];
}

word zmap::tcmbcset2(int pos)
{
  return screens[TEMPLATE2].cset[pos];
}

int zmap::tcmbflag2(int pos)
{
  return screens[TEMPLATE2].sflag[pos];
}

// void put_door(BITMAP *dest,int pos,int side,int type,int xofs,int yofs,bool ignorepos);
// void over_door(BITMAP *dest,int pos,int side,int xofs,int yofs,bool ignorepos);

void zmap::TemplateAll()
{
  for(int i=0; i<128; i++)
  {
    if((screens[i].valid&mVALID) && isDungeon(i))
      Template(-1,i);
  }
}

void zmap::Template(int floorcombo, int floorcset)
{
  Template(floorcombo, floorcset, currscr);
}

void zmap::Template(int floorcombo, int floorcset, int scr)
{
  if(scr==TEMPLATE)
    return;

  if(!(screens[scr].valid&mVALID))
    screens[scr].color=Color;
  screens[scr].valid|=mVALID;

  for(int i=0; i<32; i++)
  {
    screens[scr].data[i]=screens[TEMPLATE].data[i];
    screens[scr].cset[i]=screens[TEMPLATE].cset[i];
    screens[scr].sflag[i]=screens[TEMPLATE].sflag[i];
  }
  for(int i=144; i<176; i++)
  {
    screens[scr].data[i]=screens[TEMPLATE].data[i];
    screens[scr].cset[i]=screens[TEMPLATE].cset[i];
    screens[scr].sflag[i]=screens[TEMPLATE].sflag[i];
  }
  for(int y=2; y<=9; y++)
  {
    int j=y<<4;
    screens[scr].data[j]=screens[TEMPLATE].data[j];
    screens[scr].cset[j]=screens[TEMPLATE].cset[j];
    screens[scr].sflag[j]=screens[TEMPLATE].sflag[j++];
    screens[scr].data[j]=screens[TEMPLATE].data[j];
    screens[scr].cset[j]=screens[TEMPLATE].cset[j];
    screens[scr].sflag[j]=screens[TEMPLATE].sflag[j++];
    j+=12;
    screens[scr].data[j]=screens[TEMPLATE].data[j];
    screens[scr].cset[j]=screens[TEMPLATE].cset[j];
    screens[scr].sflag[j]=screens[TEMPLATE].sflag[j++];
    screens[scr].data[j]=screens[TEMPLATE].data[j];
    screens[scr].cset[j]=screens[TEMPLATE].cset[j];

    screens[scr].sflag[j]=screens[TEMPLATE].sflag[j++];
  }

  if(floorcombo!=-1)
  {
    for(int y=2; y<9; y++)
      for(int x=2; x<14; x++)
    {
      int i=(y<<4)+x;
      screens[scr].data[i] = floorcombo;
      screens[scr].cset[i] = floorcset;
    }
  }
  for(int i=0; i<4; i++)
    putdoor(scr,i,screens[scr].door[i]);
}

void zmap::putdoor(int side,int door)
{
  putdoor(currscr,side,door);
}

void zmap::putdoor2(int side,int door)
{
  putdoor2(currscr,side,door);
}



// void putdoor(int scr,int side,int door);
// void putdoor2(int scr,int side,int door);
// void dowarp(int type);
// void dowarp(int ring,int index);

void zmap::clearscr(int scr)
{
  byte *di=((byte*)screens)+(scr*sizeof(mapscr));
  /*
   for(unsigned i=0; i<sizeof(mapscr); i++)
     *(di++) = 0;
  */
  memset(di, 0, sizeof(mapscr));
  /*
   for(int i=0; i<4; i++)
     screens[scr].secret[i] = i;
   for(int i=0; i<16; i++)
     screens[scr].secret[i+4] = ((i>>2)<<4) + (i&3) + 12;
  */
  for (int i=0; i<6; i++)
  {
    screens[scr].layerxsize[i]=16;
    screens[scr].layerysize[i]=11;
    screens[scr].layeropacity[i]=255;
  }

  screens[scr].valid=mVERSION;
}

void zmap::savescreen()
{
  PACKFILE *f=pack_fopen("def.map",F_WRITE);
  if(!f)
    return;
  if(!pfwrite(screens+currscr,sizeof(mapscr),f))
  {
    pack_fclose(f);
    return;
  }
  pack_fclose(f);
  return;
}

char *loaderror[] =
{

  "OK","File not found","Incomplete data",
  "Invalid version","Invalid file"

};

int zmap::load(char *path)
{
  // int size=file_size(path);

  PACKFILE *f=pack_fopen(path,F_READ);
  if(!f)
    return 1;

  short version;
  byte build;
  //get the version
  if(!p_igetw(&version,f,true))
  {
    goto file_error;
  }
  //get the build
  if(!p_getc(&build,f,true))
  {
    goto file_error;
  }

  mapscr tmpimportscr;

  // clearmap();

  for(int i=0; i<MAPSCRS; i++)
  {
    if(!pfread(&tmpimportscr,sizeof(mapscr),f,true))
      goto file_error;
    switch (ImportMapBias)

    {
      case 0:
        memcpy(screens+i, &tmpimportscr, (sizeof(mapscr)));
        break;
      case 1:
        if (!(screens[i].valid&mVALID))
        {
          memcpy(screens+i, &tmpimportscr, (sizeof(mapscr)));
        }
        break;
      case 2:
        if (tmpimportscr.valid&mVALID)
        {
          memcpy(screens+i, &tmpimportscr, (sizeof(mapscr)));
        }
        break;
    }
  }


  pack_fclose(f);

  if(!(screens[0].valid&mVERSION))
  {
    jwin_alert("Confirm Clear All","Clear all?",NULL,NULL,"O&K",NULL,'k',0,lfont);
    clearmap(false);
    return 3;
  }

  setCurrScr(0);
  return 0;

  file_error:
  pack_fclose(f);
  clearmap(false);
  return 2;
}

int zmap::save(char *path)
{
  PACKFILE *f=pack_fopen(path,F_WRITE);
  if(!f)
    return 1;
  short version=ZELDA_VERSION;
  byte  build=VERSION_BUILD;
  if(!p_iputw(version,f))
  {
    pack_fclose(f);
    return 3;
  }
  if(!p_putc(build,f))
  {
    pack_fclose(f);
    return 3;
  }
  for(int i=0; i<MAPSCRS; i++)
  {
    if(!pfwrite(screens+i,sizeof(mapscr),f))
    {
      pack_fclose(f);
      return 2;
    }
  }
  pack_fclose(f);
  return 0;
}

void put_walkflags(BITMAP *dest,int x,int y,word cmbdat)
{

  newcombo c = combobuf[cmbdat];

  for(int i=0; i<4; i++)
  {
    int tx=((i&2)<<2)+x;
    int ty=((i&1)<<3)+y;
    if(c.walk&(1<<i))
      rectfill(dest,tx,ty,tx+7,ty+7,vc(12));
  }
}

void put_flags(BITMAP *dest,int x,int y,word cmbdat,int cset,int flags,int sflag)
{


  newcombo c = combobuf[cmbdat];

  if((flags&cFLAGS)&&(sflag))
  {
    rectfill(dest,x,y,x+15,y+15,vc(sflag));
    //    text_mode(-1);
    textprintf_ex(dest,sfont,x+1,y+1,(sflag)==0x7800?vc(0):vc(15),-1,"%d",sflag);
  }
  if(flags&cCSET)
  {
    bool inv = (((cmbdat&0x7800)==0x7800)&&(flags&cFLAGS));
    //    text_mode(inv?vc(15):vc(0));
    textprintf_ex(dest,sfont,x+9,y+9,inv?vc(0):vc(15),inv?vc(15):vc(0),"%d",cset);
  }
  else if(flags&cCTYPE)
  {
    bool inv = (((cmbdat&0x7800)==0x7800)&&(flags&cFLAGS));
    //    text_mode(inv?vc(15):vc(0));
    textprintf_ex(dest,sfont,x+1,y+9,inv?vc(0):vc(15),inv?vc(15):vc(0),"%d",c.type);
  }
}

void put_combo(BITMAP *dest,int x,int y,word cmbdat,int cset,int flags,int sflag)
{

  newcombo c = combobuf[cmbdat];

  if(c.tile==0)
  {
    rectfill(dest,x,y,x+15,y+15,0);
    rectfill(dest,x+3,y+3,x+12,y+12,vc(4));
    return;
  }

  putcombo(dest,x,y,cmbdat,cset);

  /* moved to put_walkflags
    for(int i=0; i<4; i++) {

      int tx=((i&2)<<2)+x;
      int ty=((i&1)<<3)+y;
      if((flags&cWALK) && (c.walk&(1<<i)))
        rectfill(dest,tx,ty,tx+7,ty+7,vc(12));
    }
  */

  //  if((flags&cFLAGS)&&(cmbdat&0xF800))
  if((flags&cFLAGS)&&(sflag))
  {
    //    rectfill(dest,x,y,x+15,y+15,vc(cmbdat>>10+1));
    rectfill(dest,x,y,x+15,y+15,vc(sflag));
    //    text_mode(-1);
    textprintf_ex(dest,sfont,x+1,y+1,(sflag)==0x7800?vc(0):vc(15),-1,"%d",sflag);
  }
  if(flags&cCSET)
  {
    bool inv = (((cmbdat&0x7800)==0x7800)&&(flags&cFLAGS));
    //    text_mode(inv?vc(15):vc(0));
    textprintf_ex(dest,sfont,x+9,y+9,inv?vc(0):vc(15),inv?vc(15):vc(0),"%d",cset);
  }
  else if(flags&cCTYPE)
  {
    bool inv = (((cmbdat&0x7800)==0x7800)&&(flags&cFLAGS));
    //    text_mode(inv?vc(15):vc(0));
    textprintf_ex(dest,sfont,x+1,y+9,inv?vc(0):vc(15),inv?vc(15):vc(0),"%d",c.type);
  }
}

void over_combo_translucent(BITMAP *dest,int x,int y,word cmbdat,int cset,int flags,int opacity)
{
  /*
    newcombo c = combobuf[cmbdat];

    if(c.tile)
      overtile16(dest,c.tile,x,y,cset,c.flip);
  */
  newcombo c = combobuf[cmbdat];
  overcombotranslucent(dest,x,y,cmbdat,cset,opacity);
  /* moved to put_walkflags
    for(int i=0; i<4; i++) {
      int tx=((i&2)<<2)+x;
      int ty=((i&1)<<3)+y;
      if((flags&cWALK) && (c.walk&(1<<i)))
        rectfill(dest,tx,ty,tx+7,ty+7,vc(12));
    }
  */
}

void over_combo(BITMAP *dest,int x,int y,word cmbdat,int cset,int flags)
{
  /*
    newcombo c = combobuf[cmbdat];


    if(c.tile)
      overtile16(dest,c.tile,x,y,cset,c.flip);

  //  over_combo_translucent(dest,x,y,cmbdat,cset,flags,255);
  */
  newcombo c = combobuf[cmbdat];
  overcombo(dest,x,y,cmbdat,cset);

  /* moved to put_walkflags
    for(int i=0; i<4; i++) {
      int tx=((i&2)<<2)+x;
      int ty=((i&1)<<3)+y;
      if((flags&cWALK) && (c.walk&(1<<i)))
        rectfill(dest,tx,ty,tx+7,ty+7,vc(12));
    }
  */

}

void zmap::put_door(BITMAP *dest,int pos,int side,int type,int xofs,int yofs,bool ignorepos)
{
  int x=0,y=0;
  switch (side)
  {
    case up:
    case down:
      x=((pos&15)<<4)+xofs;
      y=(ignorepos?0:(pos&0xF0))+yofs;
      break;
    case left:
    case right:
      x=(ignorepos?0:((pos&15)<<4))+xofs;
      y=(pos&0xF0)+yofs;
      break;
  }
  switch (type)
  {
    case dt_lock:
    case dt_shut:
    case dt_boss:
    case dt_bomb:
      switch (side)
      {
        case up:
          put_combo(dest,x,y,DoorComboSets[screens[currscr].door_combo_set].doorcombo_u[type][0],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_u[type][0],0,0);
          put_combo(dest,x+16,y,DoorComboSets[screens[currscr].door_combo_set].doorcombo_u[type][1],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_u[type][1],0,0);
          put_combo(dest,x,y+16,DoorComboSets[screens[currscr].door_combo_set].doorcombo_u[type][2],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_u[type][2],0,0);
          put_combo(dest,x+16,y+16,DoorComboSets[screens[currscr].door_combo_set].doorcombo_u[type][3],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_u[type][3],0,0);
          break;
        case down:
          put_combo(dest,x,y,DoorComboSets[screens[currscr].door_combo_set].doorcombo_d[type][0],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_d[type][0],0,0);
          put_combo(dest,x+16,y,DoorComboSets[screens[currscr].door_combo_set].doorcombo_d[type][1],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_d[type][1],0,0);
          put_combo(dest,x,y+16,DoorComboSets[screens[currscr].door_combo_set].doorcombo_d[type][2],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_d[type][2],0,0);
          put_combo(dest,x+16,y+16,DoorComboSets[screens[currscr].door_combo_set].doorcombo_d[type][3],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_d[type][3],0,0);
          break;
        case left:
          put_combo(dest,x,y,DoorComboSets[screens[currscr].door_combo_set].doorcombo_l[type][0],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_l[type][0],0,0);
          put_combo(dest,x+16,y,DoorComboSets[screens[currscr].door_combo_set].doorcombo_l[type][1],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_l[type][1],0,0);
          put_combo(dest,x,y+16,DoorComboSets[screens[currscr].door_combo_set].doorcombo_l[type][2],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_l[type][2],0,0);
          put_combo(dest,x+16,y+16,DoorComboSets[screens[currscr].door_combo_set].doorcombo_l[type][3],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_l[type][3],0,0);
          put_combo(dest,x,y+32,DoorComboSets[screens[currscr].door_combo_set].doorcombo_l[type][4],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_l[type][4],0,0);
          put_combo(dest,x+16,y+32,DoorComboSets[screens[currscr].door_combo_set].doorcombo_l[type][5],

            DoorComboSets[screens[currscr].door_combo_set].doorcset_l[type][5],0,0);
          break;
        case right:
          put_combo(dest,x,y,DoorComboSets[screens[currscr].door_combo_set].doorcombo_r[type][0],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_r[type][0],0,0);
          put_combo(dest,x+16,y,DoorComboSets[screens[currscr].door_combo_set].doorcombo_r[type][1],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_r[type][1],0,0);
          put_combo(dest,x,y+16,DoorComboSets[screens[currscr].door_combo_set].doorcombo_r[type][2],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_r[type][2],0,0);
          put_combo(dest,x+16,y+16,DoorComboSets[screens[currscr].door_combo_set].doorcombo_r[type][3],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_r[type][3],0,0);
          put_combo(dest,x,y+32,DoorComboSets[screens[currscr].door_combo_set].doorcombo_r[type][4],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_r[type][4],0,0);
          put_combo(dest,x+16,y+32,DoorComboSets[screens[currscr].door_combo_set].doorcombo_r[type][5],
            DoorComboSets[screens[currscr].door_combo_set].doorcset_r[type][5],0,0);
          break;
      }
      break;
    case dt_pass:
    case dt_wall:
    case dt_walk:
    default:
      break;
  }
}

void zmap::over_door(BITMAP *dest,int pos,int side,int xofs,int yofs,bool ignorepos)
{
  int x=((pos&15)<<4)+xofs;
  int y=(pos&0xF0)+yofs;

  switch (side)
  {
    case up:
      if (DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_u[0]!=0)
      {
        over_combo(dest,x,y,
          DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_u[0],
          DoorComboSets[screens[currscr].door_combo_set].bombdoorcset_u[0],0);
      }
      if (DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_u[1]!=0)
      {
        over_combo(dest,x+16,y,
          DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_u[1],

          DoorComboSets[screens[currscr].door_combo_set].bombdoorcset_u[1],0);
      }
      break;
    case down:
      if (DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_d[0]!=0)
      {
        over_combo(dest,x,y,
          DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_d[0],
          DoorComboSets[screens[currscr].door_combo_set].bombdoorcset_d[0],0);
      }
      if (DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_d[1]!=0)
      {
        over_combo(dest,x+16,y,
          DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_d[1],
          DoorComboSets[screens[currscr].door_combo_set].bombdoorcset_d[1],0);
      }
      break;
    case left:
      if (DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_l[0]!=0)
      {
        over_combo(dest,x,y,
          DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_l[0],
          DoorComboSets[screens[currscr].door_combo_set].bombdoorcset_l[0],0);
      }
      if (DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_l[1]!=0)
      {
        over_combo(dest,x,y+16,
          DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_l[1],
          DoorComboSets[screens[currscr].door_combo_set].bombdoorcset_l[1],0);
      }
      if (DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_l[2]!=0)
      {
        over_combo(dest,x,y+32,
          DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_l[2],
          DoorComboSets[screens[currscr].door_combo_set].bombdoorcset_l[2],0);
      }
      break;
    case right:
      if (DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_r[0]!=0)
      {
        over_combo(dest,x,y,
          DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_r[0],
          DoorComboSets[screens[currscr].door_combo_set].bombdoorcset_r[0],0);
      }
      if (DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_r[1]!=0)
      {
        over_combo(dest,x,y+16,

          DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_r[1],
          DoorComboSets[screens[currscr].door_combo_set].bombdoorcset_r[1],0);
      }
      if (DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_r[2]!=0)
      {
        over_combo(dest,x,y+32,
          DoorComboSets[screens[currscr].door_combo_set].bombdoorcombo_r[2],
          DoorComboSets[screens[currscr].door_combo_set].bombdoorcset_r[2],0);
      }
      break;
  }
}

bool zmap::misaligned(int map, int scr, int i, int dir)
{
  word cmbcheck1, cmbcheck2;
  newcombo combocheck1, combocheck2;
  combocheck1 = combobuf[0];
  combocheck2 = combobuf[0];
  combocheck1.walk = 0;
  combocheck2.walk = 0;

  int layermap, layerscreen;

  switch (dir)
  {
    case up:
      if (i>15)                                             //not top row of combos
      {
        return false;
      }
      if (scr<16)                                           //top row of screens
      {
        return false;

      }

      //check main screen
      cmbcheck1 = AbsoluteScr(map, scr)->data[i];
      cmbcheck2 = AbsoluteScr(map, scr-16)->data[i+160];
      combocheck1.walk|=combobuf[cmbcheck1].walk;
      combocheck2.walk|=combobuf[cmbcheck2].walk;

      //check layer 1
      layermap=AbsoluteScr(map, scr)->layermap[0]-1;
      if (layermap>-1)
      {
        layerscreen=AbsoluteScr(map, scr)->layerscreen[0];
        cmbcheck1 = AbsoluteScr(layermap, layerscreen)->data[i];
        combocheck1.walk|=combobuf[cmbcheck1].walk;
      }
      layermap=AbsoluteScr(map, scr-16)->layermap[0]-1;
      if (layermap>-1)
      {
        layerscreen=AbsoluteScr(map, scr-16)->layerscreen[0];
        cmbcheck2 = AbsoluteScr(layermap, layerscreen)->data[i+160];
        combocheck2.walk|=combobuf[cmbcheck2].walk;
      }

      //check layer 2
      layermap=AbsoluteScr(map, scr)->layermap[1]-1;
      if (layermap>-1)
      {
        layerscreen=AbsoluteScr(map, scr)->layerscreen[1];

        cmbcheck1 = AbsoluteScr(layermap, layerscreen)->data[i];
        combocheck1.walk|=combobuf[cmbcheck1].walk;
      }
      layermap=AbsoluteScr(map, scr-16)->layermap[1]-1;
      if (layermap>-1)
      {
        layerscreen=AbsoluteScr(map, scr-16)->layerscreen[1];
        cmbcheck2 = AbsoluteScr(layermap, layerscreen)->data[i+160];
        combocheck2.walk|=combobuf[cmbcheck2].walk;
      }
      if (((combocheck1.walk&5)*2)!=(combocheck2.walk&10))
      {
        return true;
      }
      break;
    case down:
      if (i<160)                                            //not bottom row of combos
      {
        return false;
      }
      if (scr>111)                                          //bottom row of screens
      {
        return false;
      }
      //check main screen
      cmbcheck1 = AbsoluteScr(map, scr)->data[i];
      cmbcheck2 = AbsoluteScr(map, scr+16)->data[i-160];
      combocheck1.walk|=combobuf[cmbcheck1].walk;
      combocheck2.walk|=combobuf[cmbcheck2].walk;


      //check layer 1
      layermap=AbsoluteScr(map, scr)->layermap[0]-1;
      if (layermap>-1)
      {
        layerscreen=AbsoluteScr(map, scr)->layerscreen[0];
        cmbcheck1 = AbsoluteScr(layermap, layerscreen)->data[i];
        combocheck1.walk|=combobuf[cmbcheck1].walk;
      }
      layermap=AbsoluteScr(map, scr+16)->layermap[0]-1;
      if (layermap>-1)
      {
        layerscreen=AbsoluteScr(map, scr+16)->layerscreen[0];
        cmbcheck2 = AbsoluteScr(layermap, layerscreen)->data[i-160];
        combocheck2.walk|=combobuf[cmbcheck2].walk;
      }

      //check layer 2
      layermap=AbsoluteScr(map, scr)->layermap[1]-1;
      if (layermap>-1)
      {
        layerscreen=AbsoluteScr(map, scr)->layerscreen[1];
        cmbcheck1 = AbsoluteScr(layermap, layerscreen)->data[i];
        combocheck1.walk|=combobuf[cmbcheck1].walk;
      }
      layermap=AbsoluteScr(map, scr+16)->layermap[1]-1;
      if (layermap>-1)
      {
        layerscreen=AbsoluteScr(map, scr+16)->layerscreen[1];
        cmbcheck2 = AbsoluteScr(layermap, layerscreen)->data[i-160];
        combocheck2.walk|=combobuf[cmbcheck2].walk;
      }
      if ((combocheck1.walk&10)!=((combocheck2.walk&5)*2))
      {
        return true;
      }
      break;
    case left:
      if ((i&0xF)!=0)                                       //not left column of combos
      {
        return false;
      }
      if ((scr&0xF)==0)                                     //left column of screens
      {
        return false;
      }
      //check main screen
      cmbcheck1 = AbsoluteScr(map, scr)->data[i];
      cmbcheck2 = AbsoluteScr(map, scr-1)->data[i+15];
      combocheck1.walk|=combobuf[cmbcheck1].walk;
      combocheck2.walk|=combobuf[cmbcheck2].walk;

      //check layer 1
      layermap=AbsoluteScr(map, scr)->layermap[0]-1;
      if (layermap>-1)
      {
        layerscreen=AbsoluteScr(map, scr)->layerscreen[0];
        cmbcheck1 = AbsoluteScr(layermap, layerscreen)->data[i];
        combocheck1.walk|=combobuf[cmbcheck1].walk;
      }
      layermap=AbsoluteScr(map, scr-1)->layermap[0]-1;
      if (layermap>-1)
      {
        layerscreen=AbsoluteScr(map, scr-1)->layerscreen[0];
        cmbcheck2 = AbsoluteScr(layermap, layerscreen)->data[i+15];
        combocheck2.walk|=combobuf[cmbcheck2].walk;
      }

      //check layer 2
      layermap=AbsoluteScr(map, scr)->layermap[1]-1;
      if (layermap>-1)
      {
        layerscreen=AbsoluteScr(map, scr)->layerscreen[1];
        cmbcheck1 = AbsoluteScr(layermap, layerscreen)->data[i];
        combocheck1.walk|=combobuf[cmbcheck1].walk;
      }
      layermap=AbsoluteScr(map, scr-1)->layermap[1]-1;
      if (layermap>-1)
      {
        layerscreen=AbsoluteScr(map, scr-1)->layerscreen[1];
        cmbcheck2 = AbsoluteScr(layermap, layerscreen)->data[i+15];
        combocheck2.walk|=combobuf[cmbcheck2].walk;
      }
      if (((combocheck1.walk&3)*4)!=(combocheck2.walk&12))
      {
        return true;
      }
      break;
    case right:

      if ((i&0xF)!=15)                                      //not right column of combos
      {
        return false;
      }
      if ((scr&0xF)==15)                                    //right column of screens
      {
        return false;
      }
      //check main screen
      cmbcheck1 = AbsoluteScr(map, scr)->data[i];
      cmbcheck2 = AbsoluteScr(map, scr+1)->data[i-15];
      combocheck1.walk|=combobuf[cmbcheck1].walk;
      combocheck2.walk|=combobuf[cmbcheck2].walk;

      //check layer 1
      layermap=AbsoluteScr(map, scr)->layermap[0]-1;
      if (layermap>-1)
      {
        layerscreen=AbsoluteScr(map, scr)->layerscreen[0];
        cmbcheck1 = AbsoluteScr(layermap, layerscreen)->data[i];
        combocheck1.walk|=combobuf[cmbcheck1].walk;
      }
      layermap=AbsoluteScr(map, scr+1)->layermap[0]-1;
      if (layermap>-1)
      {
        layerscreen=AbsoluteScr(map, scr+1)->layerscreen[0];
        cmbcheck2 = AbsoluteScr(layermap, layerscreen)->data[i-15];
        combocheck2.walk|=combobuf[cmbcheck2].walk;
      }

      //check layer 2
      layermap=AbsoluteScr(map, scr)->layermap[1]-1;
      if (layermap>-1)
      {
        layerscreen=AbsoluteScr(map, scr)->layerscreen[1];
        cmbcheck1 = AbsoluteScr(layermap, layerscreen)->data[i];
        combocheck1.walk|=combobuf[cmbcheck1].walk;
      }
      layermap=AbsoluteScr(map, scr+1)->layermap[1]-1;
      if (layermap>-1)
      {
        layerscreen=AbsoluteScr(map, scr+1)->layerscreen[1];

        cmbcheck2 = AbsoluteScr(layermap, layerscreen)->data[i-15];
        combocheck2.walk|=combobuf[cmbcheck2].walk;
      }
      if ((combocheck1.walk&12)!=((combocheck2.walk&3)*4))
      {
        return true;
      }
      break;
  }
  return false;
}

void zmap::check_alignments(BITMAP* dest,int x,int y,int scr)
{
  int checkcombo;
  if (alignment_arrow_timer>31)
  {
    if (scr<0)
    {
      scr=currscr;
    }

    if ((scr<128))                                          //do the misalignment arrows
    {
      for (checkcombo=1; checkcombo<15; checkcombo++)       //check the top row (except the corners)
      {
        if (misaligned(currmap, scr, checkcombo, up))
        {
          masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,0*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
        }
      }

      for (checkcombo=161; checkcombo<175; checkcombo++)    //check the top row (except the corners)
      {
        if (misaligned(currmap, scr, checkcombo, down))
        {
          masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,1*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
        }
      }

      for(checkcombo=16; checkcombo<160; checkcombo+=16)    //check the left side (except the corners)
      {
        if (misaligned(currmap, scr, checkcombo, left))
        {
          masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,2*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
        }
      }

      for(checkcombo=31; checkcombo<175; checkcombo+=16)    //check the right side (except the corners)
      {
        if (misaligned(currmap, scr, checkcombo, right))
        {
          masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,3*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
        }
      }

      int tempalign;

      //check top left corner
      checkcombo=0;
      tempalign=0;
      tempalign+=(misaligned(currmap, scr, checkcombo, up))?1:0;
      tempalign+=(misaligned(currmap, scr, checkcombo, left))?2:0;

      switch(tempalign)
      {
        case 0:
          break;
        case 1:                                             //up
          masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,0*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
          break;
        case 2:                                             //left
          masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,2*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
          break;
        case 3:                                             //up-left
          masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,4*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
          break;
      }

      //check top right corner
      checkcombo=15;
      tempalign=0;
      tempalign+=(misaligned(currmap, scr, checkcombo, up))?1:0;
      tempalign+=(misaligned(currmap, scr, checkcombo, right))?2:0;
      switch(tempalign)
      {
        case 0:
          break;
        case 1:                                             //up
          masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,0*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
          break;
        case 2:                                             //right
          masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,3*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
          break;
        case 3:                                             //up-right
          masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,5*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
          break;
      }

      //check bottom left corner
      checkcombo=160;
      tempalign=0;
      tempalign+=(misaligned(currmap, scr, checkcombo, down))?1:0;
      tempalign+=(misaligned(currmap, scr, checkcombo, left))?2:0;
      switch(tempalign)
      {
        case 0:
          break;
        case 1:                                             //down
          masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,1*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
          break;
        case 2:                                             //left
          masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,2*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
          break;
        case 3:                                             //down-left
          masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,6*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
          break;
      }

      //check bottom right corner

      checkcombo=175;
      tempalign=0;
      tempalign+=(misaligned(currmap, scr, checkcombo, down))?1:0;
      tempalign+=(misaligned(currmap, scr, checkcombo, right))?2:0;
      switch(tempalign)
      {
        case 0:
          break;
        case 1:                                             //down
          masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,1*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
          break;
        case 2:                                             //right
          masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,3*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
          break;
        case 3:                                             //down-right
          masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,7*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
          break;
      }
    }
  }
}

void zmap::draw(BITMAP* dest,int x,int y,int flags,int map,int scr)
{
  int antiflags=flags&~cFLAGS;

  if(map<0)
    map=currmap;
  if(scr<0)
    scr=currscr;

  mapscr* layer=AbsoluteScr(map,scr);
  int layermap, layerscreen;
  layermap=layer->layermap[CurrentLayer-1]-1;
  if (layermap<0)
  {
    CurrentLayer=0;
  }

  if(!(layer->valid&mVALID))
  {
    //  rectfill(dest,x,y,x+255,y+175,dvc(0+1));
    rectfill(dest,x,y,x+255,y+175,vc(1));
    return;
  }

  int dark = layer->flags&4;

  resize_mouse_pos=true;

  if (LayerMaskInt[0]!=0)
  {
    for(int i=0; i<176; i++)
    {
      word cmbdat = layer->data[i];
      byte cmbcset = layer->cset[i];
      int cmbflag = layer->sflag[i];
      put_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,cmbdat,cmbcset,antiflags|dark,cmbflag);
    }
  }
  else
  {
    rectfill(dest,x,y,x+255,y+175,0);
  }

  // int cs=2;

  for (int k=0; k<2; k++)
  {
    if(LayerMaskInt[k+1]!=0)
    {
      layermap=layer->layermap[k]-1;
      if (layermap>-1)
      {
        layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
        if (layer->layeropacity[k]==255)

        {
          for (int i=0; i<176; i++)
          {
            over_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],flags);
          }
        }
        else
        {
          for (int i=0; i<176; i++)
          {
            over_combo_translucent(dest,((i&15)<<4)+x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],flags,layer->layeropacity[k]);
          }
        }
      }
    }
  }

  int doortype[4];
  for (int i=0; i<4; i++)
  {
    switch (layer->door[i])
    {
      case dOPEN:
        doortype[i]=dt_pass;
        break;
      case dLOCKED:
        doortype[i]=dt_lock;
        break;
      case d1WAYSHUTTER:
      case dSHUTTER:
        doortype[i]=dt_shut;
        break;
      case dBOSS:
        doortype[i]=dt_boss;
        break;
      case dBOMB:
        doortype[i]=dt_bomb;
        break;
    }
  }
  switch(layer->door[up])
  {
    case dBOMB:
      over_door(dest,39,up,x,y,false);
    case dOPEN:
    case dLOCKED:
    case d1WAYSHUTTER:
    case dSHUTTER:
    case dBOSS:
      put_door(dest,7,up,doortype[up],x,y,false);
      break;
    case dWALK:
      if (get_bit(DoorComboSets[screens[currscr].door_combo_set].flags,df_walktrans))
      {
        over_combo(dest,((23&15)<<4)+8+x,(23&0xF0)+y,
          DoorComboSets[screens[currscr].door_combo_set].walkthroughcombo[0],
          DoorComboSets[screens[currscr].door_combo_set].walkthroughcset[0],0);
      }
      else

      {
        put_combo(dest,((23&15)<<4)+8+x,(23&0xF0)+y,
          DoorComboSets[screens[currscr].door_combo_set].walkthroughcombo[0],
          DoorComboSets[screens[currscr].door_combo_set].walkthroughcset[0],0,0);
      }
      break;
  }
  switch(layer->door[down])
  {
    case dBOMB:
      over_door(dest,135,down,x,y,false);
    case dOPEN:
    case dLOCKED:
    case d1WAYSHUTTER:
    case dSHUTTER:
    case dBOSS:
      put_door(dest,151,down,doortype[down],x,y,false);
      break;
    case dWALK:
      if (get_bit(DoorComboSets[screens[currscr].door_combo_set].flags,df_walktrans))
      {
        over_combo(dest,((151&15)<<4)+8+x,(151&0xF0)+y,
          DoorComboSets[screens[currscr].door_combo_set].walkthroughcombo[1],
          DoorComboSets[screens[currscr].door_combo_set].walkthroughcset[1],0);
      }
      else
      {
        put_combo(dest,((151&15)<<4)+8+x,(151&0xF0)+y,
          DoorComboSets[screens[currscr].door_combo_set].walkthroughcombo[1],
          DoorComboSets[screens[currscr].door_combo_set].walkthroughcset[1],0,0);
      }
      break;
  }
  switch(layer->door[left])
  {
    case dBOMB:
      over_door(dest,66,left,x,y,false);
    case dOPEN:
    case dLOCKED:
    case d1WAYSHUTTER:
    case dSHUTTER:
    case dBOSS:
      put_door(dest,64,left,doortype[left],x,y,false);
      break;
    case dWALK:
      if (get_bit(DoorComboSets[screens[currscr].door_combo_set].flags,df_walktrans))
      {
        over_combo(dest,((81&15)<<4)+x,(81&0xF0)+y,
          DoorComboSets[screens[currscr].door_combo_set].walkthroughcombo[2],
          DoorComboSets[screens[currscr].door_combo_set].walkthroughcset[2],0);
      }
      else
      {
        put_combo(dest,((81&15)<<4)+x,(81&0xF0)+y,
          DoorComboSets[screens[currscr].door_combo_set].walkthroughcombo[2],
          DoorComboSets[screens[currscr].door_combo_set].walkthroughcset[2],0,0);
      }
      break;
  }
  switch(layer->door[right])
  {

    case dBOMB:
      over_door(dest,77,right,x,y,false);
    case dOPEN:
    case dLOCKED:
    case d1WAYSHUTTER:
    case dSHUTTER:
    case dBOSS:
      put_door(dest,78,right,doortype[right],x,y,false);
      break;
    case dWALK:
      if (get_bit(DoorComboSets[screens[currscr].door_combo_set].flags,df_walktrans))
      {
        over_combo(dest,((94&15)<<4)+x,(94&0xF0)+y,
          DoorComboSets[screens[currscr].door_combo_set].walkthroughcombo[3],
          DoorComboSets[screens[currscr].door_combo_set].walkthroughcset[3],0);
      }
      else
      {
        put_combo(dest,((94&15)<<4)+x,(94&0xF0)+y,
          DoorComboSets[screens[currscr].door_combo_set].walkthroughcombo[3],
          DoorComboSets[screens[currscr].door_combo_set].walkthroughcset[3],0,0);
      }
      break;
  }

  if((layer->item) && !(flags&cNOITEM))
  {
    frame=0;
    putitem2(dest,layer->itemx+x,layer->itemy+y,layer->item,lens_hint_item[layer->item][0],lens_hint_item[layer->item][1], 0);
  }

  for (int k=2; k<4; k++)
  {
    if(LayerMaskInt[k+1]!=0)
    {
      layermap=layer->layermap[k]-1;
      if (layermap>-1)
      {
        layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
        if (layer->layeropacity[k]==255)
        {
          for (int i=0; i<176; i++)
          {
            over_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],flags);
          }
        }
        else
        {
          for (int i=0; i<176; i++)
          {
            over_combo_translucent(dest,((i&15)<<4)+x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],flags,layer->layeropacity[k]);
          }
        }
      }
    }
  }

  if (LayerMaskInt[0]!=0)
  {
    for (int i=0; i<176; i++)
    {
      int ct1=layer->data[i];
      //     int ct2=(ct1&0xFF)+(screens[currscr].cpage<<8);
      int ct3=combobuf[ct1].type;
      if (ct3==cOVERHEAD)
      {
        overcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,layer->data[i],layer->cset[i]);
      }
    }
  }

  for (int k=4; k<6; k++)
  {
    if(LayerMaskInt[k+1]!=0)
    {
      layermap=layer->layermap[k]-1;
      if (layermap>-1)
      {
        layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
        if (layer->layeropacity[k]==255)
        {
          for (int i=0; i<176; i++)
          {
            over_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],flags);
          }
        }
        else
        {
          for (int i=0; i<176; i++)
          {
            over_combo_translucent(dest,((i&15)<<4)+x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],flags,layer->layeropacity[k]);
          }
        }
      }
    }
  }

  if (flags&cWALK)
  {
    if (LayerMaskInt[0]!=0)
    {
      for(int i=0; i<176; i++)
      {
        put_walkflags(dest,((i&15)<<4)+x,(i&0xF0)+y,layer->data[i]);
      }
    }
    for (int k=0; k<2; k++)
    {
      if(LayerMaskInt[k+1]!=0)
      {
        layermap=layer->layermap[k]-1;
        if (layermap>-1)
        {
          layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
          for (int i=0; i<176; i++)
          {
            put_walkflags(dest,((i&15)<<4)+x,(i&0xF0)+y,TheMaps[layerscreen].data[i]);
          }
        }
      }
    }
  }

  if (flags&cFLAGS)
  {
    if (LayerMaskInt[CurrentLayer]!=0)
    {
      for(int i=0; i<176; i++)
      {
        if (CurrentLayer==0)
        {
          put_flags(dest,((i&15)<<4)+x,(i&0xF0)+y,layer->data[i],layer->cset[i],flags|dark,layer->sflag[i]);
        }
        else
        {
          put_flags(dest,((i&15)<<4)+x,(i&0xF0)+y,
            TheMaps[(screens[currscr].layermap[CurrentLayer-1]-1)*MAPSCRS+(screens[currscr].layerscreen[CurrentLayer-1])].data[i],
            TheMaps[(screens[currscr].layermap[CurrentLayer-1]-1)*MAPSCRS+(screens[currscr].layerscreen[CurrentLayer-1])].cset[i],flags|dark,
            TheMaps[(screens[currscr].layermap[CurrentLayer-1]-1)*MAPSCRS+(screens[currscr].layerscreen[CurrentLayer-1])].sflag[i]);
        }
      }
    }
  }
  /*
   if(!(flags&cDEBUG))
   {
     for(int j=168; j<176; j++)
      for(int i=0; i<256; i++)
        if(((i^j)&1)==0)
          putpixel(dest,x+i,y+j,vc(0));
   }
   if((dark) && !(flags&cNODARK))
   {
     for(int j=0; j<80; j++)
  for(int i=0; i<80-j; i++)
  if(((i^j)&1)==0)
  putpixel(dest,x+i,y+j,vc(0));
  }
  */


  if (ShowMisalignments)
  {
    check_alignments(dest,x,y,scr);
  }

  resize_mouse_pos=false;

}

void zmap::drawrow(BITMAP* dest,int x,int y,int flags,int c,int map,int scr)
{
  if(map<0)
    map=currmap;
  if(scr<0)
    scr=currscr;

  mapscr* layer=AbsoluteScr(map,scr);
  int layermap, layerscreen;
  layermap=layer->layermap[CurrentLayer-1]-1;
  if (layermap<0)
  {
    CurrentLayer=0;
  }

  if(!(layer->valid&mVALID))
  {
    //  rectfill(dest,x,y,x+255,y+15,dvc(0+1));
    rectfill(dest,x,y,x+255,y+15,vc(1));
    return;
  }

  int dark = layer->flags&4;

  resize_mouse_pos=true;

  if (LayerMaskInt[0]!=0)
  {
    for(int i=c; i<(c&0xF0)+16; i++)
    {
      word cmbdat = layer->data[i];
      byte cmbcset = layer->cset[i];
      int cmbflag = layer->sflag[i];
      put_combo(dest,((i&15)<<4)+x,y,cmbdat,cmbcset,flags|dark,cmbflag);
    }
  }
  else
  {
    rectfill(dest,x,y,x+255,y+15,0);
  }

  // int cs=2;

  for (int k=0; k<2; k++)
  {
    if(LayerMaskInt[k+1]!=0)
    {
      layermap=layer->layermap[k]-1;
      if (layermap>-1)
      {
        layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
        for(int i=c; i<(c&0xF0)+16; i++)
        {
          if (layer->layeropacity[k]<255)
          {
            over_combo_translucent(dest,((i&15)<<4)+x,y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],flags,layer->layeropacity[k]);
          }
          else
          {
            over_combo(dest,((i&15)<<4)+x,y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],flags);
          }
        }
      }
    }
  }

  int doortype[4];
  for (int i=0; i<4; i++)
  {
    switch (layer->door[i])
    {
      case dOPEN:
        doortype[i]=dt_pass;
        break;
      case dLOCKED:
        doortype[i]=dt_lock;
        break;
      case d1WAYSHUTTER:
      case dSHUTTER:
        doortype[i]=dt_shut;
        break;
      case dBOSS:
        doortype[i]=dt_boss;
        break;
      case dBOMB:
        doortype[i]=dt_bomb;
        break;
    }
  }
  if (c<16)
  {
    switch(layer->door[up])
    {
      case dBOMB:
      case dOPEN:
      case dLOCKED:
      case d1WAYSHUTTER:
      case dSHUTTER:
      case dBOSS:
        put_door(dest,7,up,doortype[up],x,y+176,true);
        break;
    }
  }
  else if (c>159)
  {
    switch(layer->door[down])
    {
      case dBOMB:
      case dOPEN:
      case dLOCKED:
      case d1WAYSHUTTER:
      case dSHUTTER:
      case dBOSS:
        put_door(dest,151,down,doortype[down],x,y-16,true);
        break;
    }
  }

  for (int k=2; k<4; k++)
  {
    if(LayerMaskInt[k+1]!=0)
    {
      layermap=layer->layermap[k]-1;
      if (layermap>-1)
      {
        layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
        for(int i=c; i<(c&0xF0)+16; i++)
        {
          if (layer->layeropacity[k]<255)
          {
            over_combo_translucent(dest,((i&15)<<4)+x,y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],flags,layer->layeropacity[k]);
          }
          else
          {
            over_combo(dest,((i&15)<<4)+x,y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],flags);
          }
        }
      }
    }
  }

  if (LayerMaskInt[0]!=0)
  {
    for(int i=c; i<(c&0xF0)+16; i++)
    {
      int ct1=layer->data[i];
      //     int ct2=(ct1&0xFF)+(screens[currscr].cpage<<8);
      int ct3=combobuf[ct1].type;
      if (ct3==cOVERHEAD)
      {
        overcombo(dest,((i&15)<<4)+x,y,layer->data[i],layer->cset[i]);
      }
    }
  }

  for (int k=4; k<6; k++)
  {
    if(LayerMaskInt[k+1]!=0)
    {
      layermap=layer->layermap[k]-1;
      if (layermap>-1)
      {
        layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
        for(int i=c; i<(c&0xF0)+16; i++)
        {
          if (layer->layeropacity[k]<255)
          {
            over_combo_translucent(dest,((i&15)<<4)+x,y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],flags,layer->layeropacity[k]);
          }
          else
          {
            over_combo(dest,((i&15)<<4)+x,y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],flags);
          }
        }
      }
    }
  }

  if (flags&cWALK)
  {
    if (LayerMaskInt[0]!=0)
    {
      for(int i=c; i<(c&0xF0)+16; i++)
      {
        put_walkflags(dest,((i&15)<<4)+x,y,layer->data[i]);
      }
    }
    for (int k=0; k<2; k++)
    {
      if(LayerMaskInt[k+1]!=0)
      {
        layermap=layer->layermap[k]-1;

        if (layermap>-1)
        {
          layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
          for(int i=c; i<(c&0xF0)+16; i++)
          {
            put_walkflags(dest,((i&15)<<4)+x,y,TheMaps[layerscreen].data[i]);
          }
        }
      }
    }
  }

  if (flags&cFLAGS)
  {
    if (LayerMaskInt[CurrentLayer]!=0)
    {
      for(int i=c; i<(c&0xF0)+16; i++)
      {
        if (CurrentLayer==0)
        {
          put_flags(dest,((i&15)<<4)+x,(i&0xF0)+y,layer->data[i],layer->cset[i],flags|dark,layer->sflag[i]);
        }
        else
        {
          put_flags(dest,((i&15)<<4)+x,(i&0xF0)+y,
            TheMaps[(screens[currscr].layermap[CurrentLayer-1]-1)*MAPSCRS+(screens[currscr].layerscreen[CurrentLayer-1])].data[i],
            TheMaps[(screens[currscr].layermap[CurrentLayer-1]-1)*MAPSCRS+(screens[currscr].layerscreen[CurrentLayer-1])].cset[i],flags|dark,
            TheMaps[(screens[currscr].layermap[CurrentLayer-1]-1)*MAPSCRS+(screens[currscr].layerscreen[CurrentLayer-1])].sflag[i]);
        }
      }
    }
    /*
        if (LayerMaskInt[0]!=0) {
          for(int i=c; i<(c&0xF0)+16; i++) {
            put_flags(dest,((i&15)<<4)+x,y,layer->data[i],layer->cset[i],flags|dark,layer->sflag[i]);
          }
        }
    */
  }

  if (ShowMisalignments)
  {
    if (c<16)
    {
      check_alignments(dest,x,y,scr);
    }
    else if (c>159)
    {
      check_alignments(dest,x,y-160,scr);
    }
  }

  resize_mouse_pos=false;

}

void zmap::drawcolumn(BITMAP* dest,int x,int y,int flags,int c,int map,int scr)
{
  if(map<0)
    map=currmap;
  if(scr<0)
    scr=currscr;

  mapscr* layer=AbsoluteScr(map,scr);
  int layermap, layerscreen;
  layermap=layer->layermap[CurrentLayer-1]-1;
  if (layermap<0)
  {
    CurrentLayer=0;
  }

  if(!(layer->valid&mVALID))
  {
    //  rectfill(dest,x,y,x+15,y+175,dvc(0+1));
    rectfill(dest,x,y,x+15,y+175,vc(1));
    return;
  }

  int dark = layer->flags&4;

  resize_mouse_pos=true;


  if (LayerMaskInt[0]!=0)
  {
    for(int i=c; i<176; i+=16)
    {
      word cmbdat = layer->data[i];
      byte cmbcset = layer->cset[i];
      int cmbflag = layer->sflag[i];
      put_combo(dest,x,(i&0xF0)+y,cmbdat,cmbcset,flags|dark,cmbflag);
    }
  }
  else
  {
    rectfill(dest,x,y,x+15,y+175,0);
  }

  // int cs=2;

  for (int k=0; k<2; k++)
  {
    if(LayerMaskInt[k+1]!=0)
    {
      layermap=layer->layermap[k]-1;
      if (layermap>-1)
      {
        layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
        for(int i=c; i<176; i+=16)
        {
          if (layer->layeropacity[k]<255)
          {
            over_combo_translucent(dest,x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],flags,layer->layeropacity[k]);
          }
          else
          {
            over_combo(dest,x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],flags);
          }
        }
      }
    }
  }

  int doortype[4];
  for (int i=0; i<4; i++)
  {
    switch (layer->door[i])
    {
      case dOPEN:
        doortype[i]=dt_pass;
        break;
      case dLOCKED:
        doortype[i]=dt_lock;
        break;
      case d1WAYSHUTTER:
      case dSHUTTER:
        doortype[i]=dt_shut;
        break;
      case dBOSS:
        doortype[i]=dt_boss;
        break;
      case dBOMB:
        doortype[i]=dt_bomb;
        break;
    }
  }
  if ((c&0x0F)==0)
  {
    switch(layer->door[left])
    {

      case dBOMB:
      case dOPEN:
      case dLOCKED:
      case d1WAYSHUTTER:
      case dSHUTTER:
      case dBOSS:
        //       put_door(dest,64,left,doortype[left],x+256,y,true);
        put_door(dest,64,left,doortype[left],x,y,true);
        break;
    }
  }
  else if ((c&0x0F)==15)
  {
    switch(layer->door[right])
    {
      case dBOMB:
      case dOPEN:
      case dLOCKED:
      case d1WAYSHUTTER:
      case dSHUTTER:
      case dBOSS:
        put_door(dest,78,right,doortype[right],x-16,y,true);
        break;
    }
  }

  for (int k=2; k<4; k++)
  {
    if(LayerMaskInt[k+1]!=0)
    {
      layermap=layer->layermap[k]-1;
      if (layermap>-1)
      {
        layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
        for(int i=c; i<176; i+=16)
        {
          if (layer->layeropacity[k]<255)
          {
            over_combo_translucent(dest,x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],flags,layer->layeropacity[k]);
          }
          else
          {
            over_combo(dest,x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],flags);
          }
        }
      }
    }
  }

  if (LayerMaskInt[0]!=0)
  {
    for(int i=c; i<176; i+=16)
    {
      int ct1=layer->data[i];
      //     int ct2=(ct1&0xFF)+(screens[currscr].cpage<<8);
      int ct3=combobuf[ct1].type;
      if (ct3==cOVERHEAD)
      {
        overcombo(dest,x,(i&0xF0)+y,layer->data[i],layer->cset[i]);
      }
    }
  }

  for (int k=4; k<6; k++)
  {
    if(LayerMaskInt[k+1]!=0)
    {
      layermap=layer->layermap[k]-1;
      if (layermap>-1)
      {
        layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
        for(int i=c; i<176; i+=16)
        {
          if (layer->layeropacity[k]<255)
          {
            over_combo_translucent(dest,x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],flags,layer->layeropacity[k]);
          }

          else
          {
            over_combo(dest,x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],flags);
          }
        }
      }
    }
  }

  if (flags&cWALK)
  {
    if (LayerMaskInt[0]!=0)
    {
      for(int i=c; i<176; i+=16)
      {
        put_walkflags(dest,x,(i&0xF0)+y,layer->data[i]);
      }
    }
    for (int k=0; k<2; k++)
    {
      if(LayerMaskInt[k+1]!=0)
      {
        layermap=layer->layermap[k]-1;
        if (layermap>-1)
        {

          layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
          for(int i=c; i<176; i+=16)
          {
            put_walkflags(dest,x,(i&0xF0)+y,TheMaps[layerscreen].data[i]);
          }
        }
      }
    }
  }

  if (flags&cFLAGS)
  {
    if (LayerMaskInt[CurrentLayer]!=0)
    {
      for(int i=c; i<176; i+=16)
      {
        if (CurrentLayer==0)
        {
          put_flags(dest,((i&15)<<4)+x,(i&0xF0)+y,layer->data[i],layer->cset[i],flags|dark,layer->sflag[i]);
        }
        else
        {
          put_flags(dest,((i&15)<<4)+x,(i&0xF0)+y,
            TheMaps[(screens[currscr].layermap[CurrentLayer-1]-1)*MAPSCRS+(screens[currscr].layerscreen[CurrentLayer-1])].data[i],
            TheMaps[(screens[currscr].layermap[CurrentLayer-1]-1)*MAPSCRS+(screens[currscr].layerscreen[CurrentLayer-1])].cset[i],flags|dark,
            TheMaps[(screens[currscr].layermap[CurrentLayer-1]-1)*MAPSCRS+(screens[currscr].layerscreen[CurrentLayer-1])].sflag[i]);
        }
      }
    }
    /*
        if (LayerMaskInt[0]!=0) {
          for(int i=c; i<176; i+=16) {
            put_flags(dest,x,(i&0xF0)+y,layer->data[i],layer->cset[i],flags|dark,layer->sflag[i]);
          }
        }
    */
  }

  if (ShowMisalignments)
  {
    if ((c&0x0F)==0)
    {
      check_alignments(dest,x,y,scr);
    }
    else if ((c&0x0F)==15)
    {
      check_alignments(dest,x-240,y,scr);
    }
  }

  resize_mouse_pos=false;
}

void zmap::drawblock(BITMAP* dest,int x,int y,int flags,int c,int map,int scr)
{
  if(map<0)
    map=currmap;
  if(scr<0)
    scr=currscr;

  mapscr* layer=AbsoluteScr(map,scr);
  int layermap, layerscreen;
  layermap=layer->layermap[CurrentLayer-1]-1;
  if (layermap<0)
  {
    CurrentLayer=0;
  }

  if(!(layer->valid&mVALID))
  {
    //  rectfill(dest,x,y,x+15,y+15,dvc(0+1));
    rectfill(dest,x,y,x+15,y+15,vc(1));
    return;
  }

  int dark = layer->flags&4;

  resize_mouse_pos=true;

  if (LayerMaskInt[0]!=0)
  {
    word cmbdat = layer->data[c];
    byte cmbcset = layer->cset[c];
    int cmbflag = layer->sflag[c];
    put_combo(dest,x,y,cmbdat,cmbcset,flags|dark,cmbflag);
  }
  else
  {
    rectfill(dest,x,y,x+15,y+15,0);
  }

  // int cs=2;

  for (int k=0; k<2; k++)
  {
    if(LayerMaskInt[k+1]!=0)
    {
      layermap=layer->layermap[k]-1;
      if (layermap>-1)
      {
        layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
        if (layer->layeropacity[k]<255)
        {
          over_combo_translucent(dest,x,y,TheMaps[layerscreen].data[c],TheMaps[layerscreen].cset[c],flags,layer->layeropacity[k]);
        }
        else
        {
          over_combo(dest,x,y,TheMaps[layerscreen].data[c],TheMaps[layerscreen].cset[c],flags);
        }
      }
    }
  }

  for (int k=2; k<4; k++)
  {
    if(LayerMaskInt[k+1]!=0)
    {
      layermap=layer->layermap[k]-1;
      if (layermap>-1)
      {
        layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
        if (layer->layeropacity[k]<255)
        {
          over_combo_translucent(dest,x,y,TheMaps[layerscreen].data[c],TheMaps[layerscreen].cset[c],flags,layer->layeropacity[k]);
        }
        else
        {
          over_combo(dest,x,y,TheMaps[layerscreen].data[c],TheMaps[layerscreen].cset[c],flags);
        }
      }
    }
  }

  if (LayerMaskInt[0]!=0)
  {
    int ct1=layer->data[c];
    int ct3=combobuf[ct1].type;
    if (ct3==cOVERHEAD)
    {
      overcombo(dest,x,y,layer->data[c],layer->cset[c]);
    }
  }

  for (int k=4; k<6; k++)
  {
    if(LayerMaskInt[k+1]!=0)
    {
      layermap=layer->layermap[k]-1;
      if (layermap>-1)
      {
        layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
        if (layer->layeropacity[k]<255)
        {
          over_combo_translucent(dest,x,y,TheMaps[layerscreen].data[c],TheMaps[layerscreen].cset[c],flags,layer->layeropacity[k]);
        }
        else
        {
          over_combo(dest,x,y,TheMaps[layerscreen].data[c],TheMaps[layerscreen].cset[c],flags);
        }
      }
    }
  }

  if (flags&cWALK)
  {
    if (LayerMaskInt[0]!=0)
    {
      put_walkflags(dest,x,y,layer->data[c]);
    }
    for (int k=0; k<2; k++)
    {
      if(LayerMaskInt[k+1]!=0)
      {
        layermap=layer->layermap[k]-1;
        if (layermap>-1)
        {
          layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
          put_walkflags(dest,x,y,TheMaps[layerscreen].data[c]);
        }
      }
    }
  }

  if (flags&cFLAGS)
  {
    if (LayerMaskInt[CurrentLayer]!=0)
    {
      for(int i=c; i==c; i++)
      {
        if (CurrentLayer==0)
        {
          put_flags(dest,((i&15)<<4)+x,(i&0xF0)+y,layer->data[i],layer->cset[i],flags|dark,layer->sflag[i]);
        }
        else
        {
          put_flags(dest,((i&15)<<4)+x,(i&0xF0)+y,
            TheMaps[(screens[currscr].layermap[CurrentLayer-1]-1)*MAPSCRS+(screens[currscr].layerscreen[CurrentLayer-1])].data[i],
            TheMaps[(screens[currscr].layermap[CurrentLayer-1]-1)*MAPSCRS+(screens[currscr].layerscreen[CurrentLayer-1])].cset[i],flags|dark,
            TheMaps[(screens[currscr].layermap[CurrentLayer-1]-1)*MAPSCRS+(screens[currscr].layerscreen[CurrentLayer-1])].sflag[i]);
        }
      }
    }
    /*
        if (LayerMaskInt[0]!=0) {
          put_flags(dest,x,y,layer->data[c],layer->cset[c],flags|dark,layer->sflag[c]);
        }
    */
  }

  if (ShowMisalignments)
  {
    switch (c)
    {
      case 0:
        check_alignments(dest,x,y,scr);
        break;
      case 15:
        check_alignments(dest,x-240,y,scr);
        break;
      case 160:
        check_alignments(dest,x,y-160,scr);
        break;
      case 175:
        check_alignments(dest,x-240,y-160,scr);
        break;
    }
  }

  resize_mouse_pos=false;

}

void zmap::drawstaticblock(BITMAP* dest,int x,int y)
{
  for (int dy=0; dy<16; dy++)
  {
    for (int dx=0; dx<16; dx++)
    {
      dest->line[y+dy][x+dx]=vc((((rand()%100)/50)?0:8)+(((rand()%100)/50)?0:7));
    }
  }
}

void zmap::drawstaticcolumn(BITMAP* dest,int x,int y)
{
  for (int i=0; i<11; i++)
  {
    drawstaticblock(dest,x, y+(i*16));
  }
}

void zmap::drawstaticrow(BITMAP* dest,int x,int y)
{
  for (int i=0; i<16; i++)
  {
    drawstaticblock(dest,x+(i*16),y);
  }
}

void zmap::draw_template(BITMAP* dest,int x,int y)
{
  for(int i=0; i<176; i++)
  {
    word cmbdat = screens[TEMPLATE].data[i];
    byte cmbcset = screens[TEMPLATE].cset[i];
    int cmbflag = screens[TEMPLATE].sflag[i];
    put_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,cmbdat,cmbcset,0,cmbflag);
  }
}

void zmap::draw_template2(BITMAP* dest,int x,int y)
{
  for(int i=0; i<176; i++)
  {
    word cmbdat = screens[TEMPLATE2].data[i];
    byte cmbcset = screens[TEMPLATE2].cset[i];
    int cmbflag = screens[TEMPLATE2].sflag[i];
    put_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,cmbdat,cmbcset,0,cmbflag);
  }
}

void zmap::draw_secret(BITMAP *dest, int pos)
{
  word cmbdat = screens[TEMPLATE].data[pos];
  byte cmbcset = screens[TEMPLATE].cset[pos];
  int cmbflag = screens[TEMPLATE].sflag[pos];
  put_combo(dest,0,0,cmbdat,cmbcset,0,cmbflag);
}

void zmap::draw_secret2(BITMAP *dest, int scombo)
{
  word cmbdat =  screens[currscr].secretcombo[scombo];
  byte cmbcset = screens[currscr].secretcset[scombo];
  byte cmbflag = screens[currscr].secretflag[scombo];
  put_combo(dest,0,0,cmbdat,cmbcset,0,cmbflag);
}

void zmap::scroll(int dir)
{
  if(currmap<map_count)
  {
    switch(dir)
    {
      case up:    if(currscr>15)  setCurrScr(currscr-16); break;
      case down:  if(currscr<MAPSCRS-16) setCurrScr(currscr+16); break;
      case left:  if(currscr&15)  setCurrScr(currscr-1);  break;
      case right: if((currscr&15)<15 && currscr<MAPSCRS-1) setCurrScr(currscr+1); break;
    }
  }
}

void zmap::putdoor2(int scr,int side,int door)
{
  if (door!=dWALL)
  {
    putdoor(scr,side,door);
  }
}

void zmap::putdoor(int scr,int side,int door)
{
  screens[scr].door[side]=door;
  word *di=screens[scr].data;
  byte *di2=screens[scr].cset;

  switch(side)
  {
    case up:
      switch(door)
      {
        case dWALL:
        case dBOMB:
        case dWALK:
          di[7]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_u[dt_wall][0];
          di2[7]  = DoorComboSets[screens[scr].door_combo_set].doorcset_u[dt_wall][0];
          di[8]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_u[dt_wall][1];
          di2[8]  = DoorComboSets[screens[scr].door_combo_set].doorcset_u[dt_wall][1];
          di[23]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_u[dt_wall][2];
          di2[23]  = DoorComboSets[screens[scr].door_combo_set].doorcset_u[dt_wall][2];
          di[24]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_u[dt_wall][3];
          di2[24]  = DoorComboSets[screens[scr].door_combo_set].doorcset_u[dt_wall][3];
          break;
        default:
          di[7]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_u[dt_pass][0];
          di2[7]  = DoorComboSets[screens[scr].door_combo_set].doorcset_u[dt_pass][0];
          di[8]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_u[dt_pass][1];
          di2[8]  = DoorComboSets[screens[scr].door_combo_set].doorcset_u[dt_pass][1];
          di[23]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_u[dt_pass][2];
          di2[23]  = DoorComboSets[screens[scr].door_combo_set].doorcset_u[dt_pass][2];
          di[24]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_u[dt_pass][3];
          di2[24]  = DoorComboSets[screens[scr].door_combo_set].doorcset_u[dt_pass][3];
          break;
      } break;
    case down:
      switch(door)
      {
        case dWALL:
        case dBOMB:
        case dWALK:
          di[151]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_d[dt_wall][0];
          di2[151]  = DoorComboSets[screens[scr].door_combo_set].doorcset_d[dt_wall][0];
          di[152]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_d[dt_wall][1];
          di2[152]  = DoorComboSets[screens[scr].door_combo_set].doorcset_d[dt_wall][1];
          di[167]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_d[dt_wall][2];
          di2[167]  = DoorComboSets[screens[scr].door_combo_set].doorcset_d[dt_wall][2];
          di[168]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_d[dt_wall][3];
          di2[168]  = DoorComboSets[screens[scr].door_combo_set].doorcset_d[dt_wall][3];
          break;

        default:
          di[151]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_d[dt_pass][0];
          di2[151]  = DoorComboSets[screens[scr].door_combo_set].doorcset_d[dt_pass][0];
          di[152]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_d[dt_pass][1];
          di2[152]  = DoorComboSets[screens[scr].door_combo_set].doorcset_d[dt_pass][1];
          di[167]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_d[dt_pass][2];
          di2[167]  = DoorComboSets[screens[scr].door_combo_set].doorcset_d[dt_pass][2];
          di[168]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_d[dt_pass][3];
          di2[168]  = DoorComboSets[screens[scr].door_combo_set].doorcset_d[dt_pass][3];
          break;
      } break;
    case left:
      switch(door)
      {
        case dWALL:
        case dBOMB:
        case dWALK:
          di[64]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_wall][0];
          di2[64]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_wall][0];
          di[65]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_wall][1];
          di2[65]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_wall][1];
          di[80]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_wall][2];
          di2[80]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_wall][2];
          di[81]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_wall][3];
          di2[81]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_wall][3];
          di[96]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_wall][4];
          di2[96]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_wall][4];
          di[97]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_wall][5];
          di2[97]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_wall][5];
          break;
        default:
          di[64]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_pass][0];
          di2[64]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_pass][0];
          di[65]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_pass][1];
          di2[65]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_pass][1];
          di[80]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_pass][2];
          di2[80]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_pass][2];
          di[81]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_pass][3];
          di2[81]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_pass][3];
          di[96]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_pass][4];
          di2[96]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_pass][4];
          di[97]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_pass][5];
          di2[97]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_pass][5];
          break;
      } break;
    case right:
      switch(door)
      {
        case dWALL:
        case dBOMB:
        case dWALK:
          di[78]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_wall][0];
          di2[78]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_wall][0];
          di[79]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_wall][1];
          di2[79]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_wall][1];
          di[94]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_wall][2];
          di2[94]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_wall][2];
          di[95]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_wall][3];
          di2[95]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_wall][3];
          di[110]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_wall][4];
          di2[110]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_wall][4];
          di[111]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_wall][5];
          di2[111]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_wall][5];
          break;
        default:
          di[78]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_pass][0];
          di2[78]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_pass][0];
          di[79]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_pass][1];
          di2[79]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_pass][1];
          di[94]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_pass][2];
          di2[94]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_pass][2];
          di[95]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_pass][3];
          di2[95]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_pass][3];
          di[110]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_pass][4];
          di2[110]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_pass][4];
          di[111]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_pass][5];
          di2[111]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_pass][5];
          break;
      } break;
  }
}

void zmap::Ugo()

{
  for (int x=0; x<128; x++)
    undomap[x]=screens[x];
  can_undo=true;
}

void zmap::Uhuilai()
{
  if(can_undo)
    for (int x=0; x<128; x++)

      swap(screens[x],undomap[x]);
}

void zmap::Copy()
{
  if(screens[currscr].valid&mVALID)
  {
    copymapscr=screens[currscr];
    can_paste=true;
    copymap=currmap;
    copyscr=currscr;
  }
}

void zmap::Paste()
{
  if(can_paste)
  {
    Ugo();
    if(!(screens[currscr].valid&mVALID))
    {
      screens[currscr].valid |= mVALID;
      screens[currscr].color = copymapscr.color;
    }
    for(int i=0; i<4; i++)
    {
      putdoor(currscr,i,0);
    }
    for(int i=0; i<4; i++)
    {

      screens[currscr].door[i]=copymapscr.door[i];
    }
    for(int i=0; i<176; i++)
    {
      screens[currscr].data[i] = copymapscr.data[i];
      screens[currscr].cset[i] = copymapscr.cset[i];
      screens[currscr].sflag[i] = copymapscr.sflag[i];
    }
    for(int i=0; i<4; i++)
    {
      putdoor2(currscr,i,screens[currscr].door[i]);
    }
    saved=false;
  }
}

void zmap::PasteAll()
{
  if(can_paste)
  {
    Ugo();
    screens[currscr]=copymapscr;
    saved=false;
  }
}


void zmap::PasteToAll()
{
  if(can_paste)
  {
    Ugo();
    for(int x=0; x<128; x++)
    {
      if(!(screens[x].valid&mVALID))
      {
        screens[x].valid |= mVALID;
        screens[x].color = copymapscr.color;
      }
      for(int i=0; i<176; i++)
      {
        screens[x].data[i] = copymapscr.data[i];
        screens[x].sflag[i] = copymapscr.sflag[i];
      }
      if(isDungeon(x))
        for(int i=0; i<4; i++)
          putdoor(currscr,i,screens[x].door[i]);

    }
    saved=false;
  }
}

void zmap::PasteAllToAll()
{
  if(can_paste)
  {
    Ugo();
    for (int x=0; x<128; x++)
    {
      screens[x]=copymapscr;
      saved=false;
    }
  }
}

void zmap::PasteEnemies()
{
  if(can_paste)
  {
    for(int i=0; i<10; i++)
      screens[currscr].enemy[i]=copymapscr.enemy[i];
  }
}

void zmap::update_combo_cycling()
{
  return;
}

void zmap::dowarp(int type)
{
  if(type==0)
  {

    int dmap=screens[currscr].tilewarpdmap;
    int scr=screens[currscr].tilewarpscr;
    switch(screens[currscr].tilewarptype)
    {
      case wtCAVE:
      case wtNOWARP:
        break;
      default:
        setCurrMap(DMaps[dmap].map);
        setCurrScr(scr+DMaps[dmap].xoff);
        break;
    }
  }
  else if(type==1)
  {
    int dmap=screens[currscr].sidewarpdmap;
    int scr=screens[currscr].sidewarpscr;
    switch(screens[currscr].sidewarptype)
    {
      case wtCAVE:
      case wtNOWARP:
        break;
      default:
        setCurrMap(DMaps[dmap].map);
        setCurrScr(scr+DMaps[dmap].xoff);
        break;
    }
  }
  else
  {
    int dmap=misc.wind[type-2].dmap;
    int scr=misc.wind[type-2].scr;
    setCurrMap(DMaps[dmap].map);
    setCurrScr(scr+DMaps[dmap].xoff);
  }
}

void zmap::dowarp(int ring,int index)
{
  int dmap=misc.warp[ring].dmap[index];
  int scr=misc.warp[ring].scr[index];
  setCurrMap(DMaps[dmap].map);
  setCurrScr(scr+DMaps[dmap].xoff);
}

/******************************/
/******** ZQuest stuff ********/
/******************************/
char *MsgString(int index)
{
  bound(index,0,MAXMSGS-1);
  char *s;
  char u[80];
  sprintf(u, "%d", index);
  s=strcat(u,": ");

  char *t = new char[73];
  memset(t, 0, 73);
  int i=0;
  //return s;

  //remove preceding spaces;
  for(; MsgStrings[index].s[i]==' '; i++);
  int msgptr=0;
  for(; msgptr<72 && i<72; i++)
  {
    if(MsgStrings[index].s[i]>=32 && MsgStrings[index].s[i]<=126)
    {
      t[msgptr++]=MsgStrings[index].s[i];
    }
  }
  for(;msgptr<=72;msgptr++)
    t[msgptr]=0;
  return strcat(s,t);
}

void reset_msgstr(int index)
{
  bound(index,0,MAXMSGS-1);
  /*
    char *s=MsgStrings[index].s;
    for(int i=0; i<76; i++)
    *(s++)=0;
    */
  memset(MsgStrings[index].s, 0, 73);
  MsgStrings[index].nextstring=0;
  //memset(MsgStrings[index].expansion, 0, 32);
}

void init_msgstrs()
{
  for(int i=0; i<MAXMSGS; i++)
    reset_msgstr(i);
  strcpy(MsgStrings[0].s,"(None)");
  msg_count=1;
}

bool save_msgstrs(char *path)
{
  PACKFILE *f = pack_fopen(path,F_WRITE);
  if (!f)
  {
    return false;
  }

  if (writestrings(f, ZELDA_VERSION, VERSION_BUILD, 0, MAXMSGS)==0)
  {
    pack_fclose(f);
    return true;
  }

  pack_fclose(f);
  return false;
}

bool load_msgstrs(char *path, int startstring)
{
  //these are here to bypass compiler warnings about unused arguments
  startstring=startstring;

  dword section_id;
  PACKFILE *f = pack_fopen(path,F_READ);
  if (!f)
  {
    return false;
  }
  if(!p_mgetl(&section_id,f,true))
  {
    return false;
  }
  if(section_id==ID_STRINGS)
  {
    if (readstrings(f, &header, true)==0)
    {
      pack_fclose(f);
      return true;
    }
    else
    {
      pack_fclose(f);
      return false;
    }
  }
  pack_fclose(f);
  return false;
}

bool save_pals(char *path)
{
  PACKFILE *f = pack_fopen(path,F_WRITE);
  if (!f)
  {
    return false;
  }

  if (writecolordata(f, &misc, ZELDA_VERSION, VERSION_BUILD, 0, newpdTOTAL)==0)
  {
    pack_fclose(f);
    return true;
  }

  pack_fclose(f);
  return false;
}

bool load_pals(char *path, int startcset)
{
  dword section_id;
  PACKFILE *f = pack_fopen(path,F_READ);
  if (!f)
  {
    return false;
  }
  if(!p_mgetl(&section_id,f,true))
  {
    return false;
  }
  if(section_id==ID_CSETS)
  {
    if (readcolordata(f, &misc, ZELDA_VERSION, VERSION_BUILD, startcset, newpdTOTAL-startcset, true)==0)
    {
      pack_fclose(f);
      loadlvlpal(Color);
      return true;
    }
    else
    {
      pack_fclose(f);
      return false;
    }
  }
  return false;
}

bool save_dmaps(char *path)
{
  PACKFILE *f = pack_fopen(path,F_WRITE);
  if (!f)
  {
    return false;
  }

  if (writedmaps(f, ZELDA_VERSION, VERSION_BUILD, 0, MAXDMAPS)==0)
  {
    pack_fclose(f);
    return true;
  }

  pack_fclose(f);
  return false;
}

bool load_dmaps(char *path, int startdmap)
{
  dword section_id;
  PACKFILE *f = pack_fopen(path,F_READ);
  if (!f)
  {
    return false;
  }
  if(!p_mgetl(&section_id,f,true))
  {
    return false;
  }
  if(section_id==ID_DMAPS)
  {
    if (readdmaps(f, NULL, ZELDA_VERSION, VERSION_BUILD, startdmap, MAXDMAPS-startdmap, true)==0)
    {
      pack_fclose(f);
      return true;
    }
    else
    {
      pack_fclose(f);
      return false;
    }
  }
  return false;
}

bool save_combos(char *path)
{
  PACKFILE *f = pack_fopen(path,F_WRITE);
  if (!f)
  {
    return false;
  }

  reset_combo_animations();
  //reset_combo_animations2();  //not sure what this is; it came from 2.11
  if (writecombos(f, ZELDA_VERSION, VERSION_BUILD, 0, MAXCOMBOS)==0)
  {
    pack_fclose(f);
    return true;
  }

  pack_fclose(f);
  return false;
}

bool load_combos(char *path, int startcombo)
{
  dword section_id;
  PACKFILE *f = pack_fopen(path,F_READ);
  if (!f)
  {
    return false;
  }
  if(!p_mgetl(&section_id,f,true))
  {
    return false;
  }
  if(section_id==ID_COMBOS)
  {
    if (readcombos(f, NULL, ZELDA_VERSION, VERSION_BUILD, startcombo, MAXCOMBOS-startcombo, true)==0)
    {
      pack_fclose(f);
      return true;
    }
    else
    {
      pack_fclose(f);
      //      init_combos(true, &header);
      return false;
    }
  }
  return false;
}

bool save_tiles(char *path)
{
  PACKFILE *f = pack_fopen(path,F_WRITE);
  if (!f)
  {
    return false;
  }

  //  reset_combo_animations();
  if (writetiles(f, ZELDA_VERSION, VERSION_BUILD, 0, NEWMAXTILES)==0)
  {
    pack_fclose(f);
    return true;
  }

  pack_fclose(f);
  return false;
}

bool load_tiles(char *path, int starttile)
{
  dword section_id;
  PACKFILE *f = pack_fopen(path,F_READ);
  if (!f)
  {
    return false;
  }
  if(!p_mgetl(&section_id,f,true))
  {
    return false;
  }
  if(section_id==ID_TILES)
  {
    if (readtiles(f, tilebuf, NULL, ZELDA_VERSION, VERSION_BUILD, starttile, NEWMAXTILES-starttile, false, true)==0)
    {
      pack_fclose(f);
      return true;
    }
    else
    {
      pack_fclose(f);
      init_tiles(true, &header);
      return false;
    }
  }
  return false;
}

bool setMapCount2(int c, bool reload)
{
  char tmpbuf[L_tmpnam];
  char *tmpfilename = tmpnam(tmpbuf);
  bool resized=true;
  PACKFILE *f=pack_fopen(tmpfilename,F_WRITE);
  int oldmapcount=map_count;
  if (reload)
  {
    for(int i=0; i<oldmapcount; i++)
    {
      if(!pfwrite(TheMaps+(i*MAPSCRS),sizeof(mapscr)*MAPSCRS,f))
      {
        jwin_alert("Error","Could not backup maps.","Change map count failed.",NULL,"O&K",NULL,'k',0,lfont);
        goto error;
      }
    }
  }
  pack_fclose(f);

  free(TheMaps);
  if(!(TheMaps=(mapscr*)malloc(sizeof(mapscr)*MAPSCRS*c)))
  {
    if(!(TheMaps=(mapscr*)malloc(sizeof(mapscr)*MAPSCRS*oldmapcount)))
    {
      jwin_alert("Error","There was a memory leak.","ZQuest will now close.",NULL,"O&K",NULL,'k',0,lfont);
      exit(1);
    }
    memset(TheMaps,0,sizeof(mapscr)*MAPSCRS*oldmapcount);
    jwin_alert("Error", "Not enough memory.","Change map count failed.",NULL,"O&K",NULL,'k',0,lfont);
    resized=false;
  }
  else
  {
    memset(TheMaps,0,sizeof(mapscr)*MAPSCRS*c);
  }

  if (resized)
  {
    bound(c,1,MAXMAPS2);
    map_count=c;
    int currmap=Map.getCurrMap();

    if (reload)
    {
      f=pack_fopen(tmpfilename,F_READ);
      for(int i=0; i<min(oldmapcount,map_count); i++)
      {
        if(!pfread(TheMaps+(i*MAPSCRS),sizeof(mapscr)*MAPSCRS,f,true))
        {
          jwin_alert("Error","Could not reload maps.",NULL,NULL,"O&K",NULL,'k',0,lfont);
          goto error;
        }
      }
    }

    if (map_count>oldmapcount)
    {
      for (int mc=oldmapcount; mc<map_count; mc++)
      {
        Map.setCurrMap(mc);
        for (int ms=0; ms<MAPSCRS; ms++)
        {
          Map.clearscr(ms);
        }
      }
    }

    Map.setCurrMap(bound(currmap,0,c-1));                   // for bound checking
    if (oldmapcount>c)
    {
      refresh(rALL);
    }
  }

  error:
  if (reload)
  {
    pack_fclose(f);
  }
  delete_file(tmpfilename);
  return resized;
}

int init_quest()
{
  char buf[2048];
  load_quest("qst.dat#DAT_NESQST",true,true);
  sprintf(buf,"ZQuest - Untitled Quest");
  set_window_title(buf);
  saved=true;
  return 0;
}

void set_questpwd(char *pwd)
{
  if(strlen(pwd)==0)
  {
    header.pwdkey=0;
    for(int i=0; i<30; i++)
      header.password[i]=rand();
  }
  else
  {
    short key=(rand()&0xFFF0)+11;
    header.pwdkey=key;
    memcpy(header.password,pwd,30);
    for(int i=0; i<30; i++)
    {
      header.password[i] += key;
      int t=key>>15;
      key = (key<<1)+t;
    }
  }
}

void get_questpwd(char *pwd)
{
  if(header.pwdkey==0)

  {
    pwd[0]=0;
  }
  else
  {
    short key = header.pwdkey;
    memcpy(pwd,header.password,30);
    pwd[30]=0;
    for(int i=0; i<30; i++)
    {
      pwd[i] -= key;
      int t=key>>15;
      key = (key<<1)+t;
    }
  }
}

static DIALOG pwd_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc, 48,   80,  224+1,   88+1,   vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Requires Authorization" },
  { d_dummy_proc,      160,  88,    0,   8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_text_proc,       64,   108,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "File name:" },
  // 3 (filename)
  { jwin_text_proc,       120,  108, 128,   8,    vc(11),  vc(1),  0,       0,          24,            0,       NULL },
  { jwin_text_proc,       64,   120,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Password:" },
  // 5 (password)
  { jwin_edit_proc,       120,  116, 120,   16,    vc(12),  vc(1),  0,       0,          24,            0,       NULL },
  { jwin_button_proc,     90,   140,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  140,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};

bool quest_access(char *filename, zquestheader *hdr, bool compressed)
{
  if (!compressed)
  {
    return true;
  }
  if(debug || hdr->pwdkey==0)
      return true;

  char pwd[32];
  char prompt[32]="";

  char keyfilename[256];
  replace_extension(keyfilename, filename, "key", 255);
  bool gotfromkey=false;
  if (exists(keyfilename))
  {
    char password[32];
    PACKFILE *fp = pack_fopen(keyfilename, F_READ);
    char msg[80];
    memset(msg,0,80);
    pfread(msg, 80, fp,true);
    if (strcmp(msg,"ZQuest Auto-Generated Quest Password Key File.  DO NOT EDIT!")==0)
    {
      short ver;
      byte  bld;
      p_igetw(&ver,fp,true);
      p_getc(&bld,fp,true);
      memset(password,0,32);
      pfread(password, 30, fp,true);
      get_questpwd(pwd);
      if (strcmp(pwd,password)==0)
      {
        gotfromkey=true;
      }
      memset(password,0,32);
      memset(pwd,0,32);
    }
    pack_fclose(fp);
  }
  if (gotfromkey)
  {
    return true;
  }
  pwd_dlg[0].dp2=lfont;
  pwd_dlg[3].dp=get_filename(filename);
  pwd_dlg[5].dp=prompt;
  popup_dialog(pwd_dlg,5);
  get_questpwd(pwd);
  bool ret = strcmp(pwd,prompt)==0;
  memset(pwd,0,32);
  return ret;

}

// wrapper to reinitialize everything on an error
int load_quest(char *filename, bool compressed, bool encrypted)
{
  char buf[2048];
  if(encrypted) packfile_password(datapwd);
  byte skip_flags[4];
  for (int i=0; i<4; ++i)
  {
    skip_flags[i]=0;
  }
  int ret=loadquest(filename,&header,&misc,customMIDIs,true,compressed,encrypted,true,skip_flags);
  packfile_password(NULL);

  if(ret!=qe_OK)
  {
    init_quest();
  }
  else if(!quest_access(filename, &header, compressed))
  {
    init_quest();
    ret=qe_pwd;
  }
  else
  {
    Map.setCurrMap(vbound(Map.getCurrMap(),0,map_count-1));
    refresh(rALL);
    refresh_pal();
/* Not sure what this is for
    if(map != NULL)
    {
      destroy_bitmap(map);
      map=NULL;
    }
*/
    sprintf(buf,"ZQuest - [%s]", get_filename(filename));
    if (compressed)
    {
      set_window_title(buf);
    }
  }
  return ret;
}

bool write_midi(MIDI *m,PACKFILE *f)

{
  int c;

  pack_mputw(m->divisions,f);

  for(c=0; c<MIDI_TRACKS; c++)
  {
    pack_mputl(m->track[c].len,f);

    if(m->track[c].len > 0)
    {
      if(!pfwrite(m->track[c].data,m->track[c].len,f))
        return false;
    }
  }
  return true;
}

bool write_unencoded_midi(MIDI *m,PACKFILE *f)
{
  int c;

  p_mputw(m->divisions,f);

  for(c=0; c<MIDI_TRACKS; c++)
  {
    p_mputl(m->track[c].len,f);

    if(m->track[c].len > 0)
    {
      if(int(!pfwrite(m->track[c].data,m->track[c].len,f))
        !=m->track[c].len)
        return false;
    }
  }
  return true;
}

static MIDI *read_unencoded_midi(PACKFILE *f)
{
  MIDI *m;
  int c;

  m = (MIDI*)malloc(sizeof(MIDI));
  if (!m)
    return NULL;

  for (c=0; c<MIDI_TRACKS; c++)
  {
    m->track[c].len = 0;
    m->track[c].data = NULL;
  }

  p_mgetw(&(m->divisions),f,true);

  for (c=0; c<MIDI_TRACKS; c++)
  {
    p_mgetl(&(m->track[c].len),f,true);
    if (m->track[c].len > 0)
    {
      m->track[c].data = (byte*)malloc(m->track[c].len);
      pfread(m->track[c].data,m->track[c].len,f,true);
      if (!m->track[c].data)
      {
        destroy_midi(m);
        return NULL;
      }
    }
  }

  LOCK_DATA(m, sizeof(MIDI));
  for (c=0; c<MIDI_TRACKS; c++)
    if (m->track[c].data)
      LOCK_DATA(m->track[c].data, m->track[c].len);

  return m;
}

int writeheader(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_HEADER;
  dword section_version=V_HEADER;
  dword section_cversion=CV_HEADER;
  dword section_size=38+18+131+20+280+1;                    //+29;

  //file header string
  if(!pfwrite(header->id_str,sizeof(header->id_str),f))
  {
    return 1;
  }

  //section id
  if(!p_mputl(section_id,f))
  {
    return 2;
  }

  //section version info
  if(!p_iputw(section_version,f))
  {
    return 3;
  }
  if(!p_iputw(section_cversion,f))
  {
    return 4;
  }

  //section size
  if(!p_iputl(section_size,f))
  {
    return 5;
  }

  //finally...  section data
  if(!p_iputw(header->zelda_version,f))
  {
    return 6;
  }
  if(!p_putc(header->build,f))
  {
    return 7;
  }
  if(!pfwrite(header->password,sizeof(header->password),f))
  {
    return 8;
  }
  if(!p_iputw(header->pwdkey,f))
  {
    return 9;
  }
  if(!p_iputw(header->internal,f))
  {
    return 10;
  }
  if(!p_putc(header->quest_number,f))
  {
    return 11;
  }
  if(!pfwrite(header->version,sizeof(header->version),f))
  {
    return 12;
  }
  if(!pfwrite(header->minver,sizeof(header->minver),f))
  {
    return 13;
  }
  if(!pfwrite(header->title,sizeof(header->title),f))
  {
    return 14;
  }
  if(!pfwrite(header->author,sizeof(header->author),f))
  {
    return 15;
  }
  if(!p_putc(header->use_keyfile,f))
  {
    return 16;
  }
  if(!pfwrite(header->data_flags,sizeof(header->data_flags),f))
  {
    return 17;
  }
  if(!pfwrite(header->templatepath,sizeof(header->templatepath),f))
  {
    return 19;
  }
  if(!p_putc(0,f))
  {
    return 20;
  }

  //unneeded data
  /*
    if(!pfwrite(midi_flags,sizeof(midi_flags),f)){
      return 18;
    }
    if(!pfwrite(header->foo,sizeof(header->foo),f)){
      return 21;
    }
    if(!pfwrite(header->foo2,sizeof(header->foo2),f)){
      return 22;
    }
    if(!p_putc(header->old_str_count,f)){
  return 23;
  }
  if(!p_putc(header->old_options,f)){
  return 24;
  }
  */
  return 0;
}

int writerules(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_RULES;
  dword section_version=V_RULES;
  dword section_cversion=CV_RULES;
  dword section_size=2*10;

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

  //finally...  section data
  if(!pfwrite(quest_rules,QUESTRULES_SIZE,f))
  {
    return 5;
  }
  return 0;
}


int writedoorcombosets(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_DOORS;
  dword section_version=V_DOORS;
  dword section_cversion=CV_DOORS;
  dword section_size=2+door_combo_set_count*
    (21+(3*9*4*2)+(3*9*6*2)+(3*2*2)+(3*2*3)+(3*4)+(2));

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

  //finally...  section data
  if(!p_iputw(door_combo_set_count,f))
  {
    return 5;
  }

  for (int i=0; i<door_combo_set_count; i++)
  {
    //name
    if(!pfwrite(&DoorComboSets[i].name,sizeof(DoorComboSets[0].name),f))
    {
      return 6;
    }
    //up door
    for (int j=0; j<9; j++)
    {
      for (int k=0; k<4; k++)
      {
        if(!p_iputw(DoorComboSets[i].doorcombo_u[j][k],f))
        {
          return 7;
        }
      }
    }
    for (int j=0; j<9; j++)
    {
      for (int k=0; k<4; k++)
      {
        if(!p_putc(DoorComboSets[i].doorcset_u[j][k],f))
        {
          return 8;
        }
      }
    }

    //down door
    for (int j=0; j<9; j++)
    {
      for (int k=0; k<4; k++)
      {
        if(!p_iputw(DoorComboSets[i].doorcombo_d[j][k],f))
        {
          return 9;
        }
      }
    }
    for (int j=0; j<9; j++)
    {
      for (int k=0; k<4; k++)
      {
        if(!p_putc(DoorComboSets[i].doorcset_d[j][k],f))
        {
          return 10;
        }
      }
    }


    //left door
    for (int j=0; j<9; j++)
    {
      for (int k=0; k<6; k++)
      {
        if(!p_iputw(DoorComboSets[i].doorcombo_l[j][k],f))

        {
          return 11;
        }
      }
    }
    for (int j=0; j<9; j++)
    {
      for (int k=0; k<6; k++)
      {
        if(!p_putc(DoorComboSets[i].doorcset_l[j][k],f))
        {
          return 12;
        }
      }
    }

    //right door
    for (int j=0; j<9; j++)
    {
      for (int k=0; k<6; k++)
      {
        if(!p_iputw(DoorComboSets[i].doorcombo_r[j][k],f))
        {
          return 13;
        }
      }
    }
    for (int j=0; j<9; j++)
    {
      for (int k=0; k<6; k++)
      {
        if(!p_putc(DoorComboSets[i].doorcset_r[j][k],f))
        {
          return 14;
        }
      }
    }


    //up bomb rubble
    for (int j=0; j<2; j++)
    {
      if(!p_iputw(DoorComboSets[i].bombdoorcombo_u[j],f))
      {
        return 15;
      }
    }
    for (int j=0; j<2; j++)
    {
      if(!p_putc(DoorComboSets[i].bombdoorcset_u[j],f))
      {
        return 16;
      }
    }

    //down bomb rubble
    for (int j=0; j<2; j++)
    {
      if(!p_iputw(DoorComboSets[i].bombdoorcombo_d[j],f))
      {
        return 17;
      }
    }
    for (int j=0; j<2; j++)
    {
      if(!p_putc(DoorComboSets[i].bombdoorcset_d[j],f))
      {
        return 18;
      }
    }

    //left bomb rubble
    for (int j=0; j<3; j++)
    {
      if(!p_iputw(DoorComboSets[i].bombdoorcombo_l[j],f))
      {
        return 19;
      }
    }
    for (int j=0; j<3; j++)
    {
      if(!p_putc(DoorComboSets[i].bombdoorcset_l[j],f))
      {
        return 20;
      }
    }

    //right bomb rubble
    for (int j=0; j<3; j++)
    {
      if(!p_iputw(DoorComboSets[i].bombdoorcombo_r[j],f))
      {
        return 21;
      }
    }
    for (int j=0; j<3; j++)
    {
      if(!p_putc(DoorComboSets[i].bombdoorcset_r[j],f))
      {
        return 22;
      }
    }

    //walkthrough stuff
    for (int j=0; j<4; j++)
    {
      if(!p_iputw(DoorComboSets[i].walkthroughcombo[j],f))
      {
        return 23;
      }
    }
    for (int j=0; j<4; j++)
    {
      if(!p_putc(DoorComboSets[i].walkthroughcset[j],f))
      {
        return 24;
      }
    }

    //flags
    for (int j=0; j<2; j++)
    {
      if(!p_putc(DoorComboSets[i].flags[j],f))
      {
        return 25;
      }
    }

  }
  return 0;
}

int writedmaps(PACKFILE *f, word version, word build, word start_dmap, word max_dmaps)
{
  word dmap_count=count_dmaps();
  dword section_id=ID_DMAPS;
  dword section_version=V_DMAPS;
  dword section_cversion=CV_DMAPS;
  dword section_size=2+(dmap_count*(16+42+73+12+56));

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

  dmap_count=min(dmap_count, max_dmaps);
  dmap_count=min(dmap_count, MAXDMAPS-start_dmap);

  //finally...  section data
  if(!p_iputw(dmap_count,f))
  {
    return 5;
  }


  for (int i=start_dmap; i<start_dmap+dmap_count; i++)
  {
    if(!p_putc(DMaps[i].map,f))
    {
      return 6;
    }
    if(!p_putc(DMaps[i].level,f))
    {
      return 7;
    }
    if(!p_putc(DMaps[i].xoff,f))
    {
      return 8;
    }
    if(!p_putc(DMaps[i].compass,f))
    {
      return 9;
    }
    if(!p_putc(DMaps[i].color,f))
    {
      return 10;
    }
    if(!p_putc(DMaps[i].midi,f))
    {
      return 11;
    }
    if(!p_putc(DMaps[i].cont,f))
    {
      return 12;
    }
    if(!p_putc(DMaps[i].type,f))
    {
      return 13;
    }
    for (int j=0; j<8; j++)
    {
      if(!p_putc(DMaps[i].grid[j],f))
      {
        return 14;
      }
    }
    //16
    if(!pfwrite(&DMaps[i].name,sizeof(DMaps[0].name),f))
    {
      return 15;
    }
    if(!pfwrite(&DMaps[i].title,sizeof(DMaps[0].title),f))
    {
      return 16;
    }
    if(!pfwrite(&DMaps[i].intro,sizeof(DMaps[0].intro),f))
    {
      return 17;
    }
    if(!p_iputw(DMaps[i].minimap_1_tile,f))
    {
      return 18;
    }
    if(!p_putc(DMaps[i].minimap_1_cset,f))
    {
      return 19;
    }
    if(!p_iputw(DMaps[i].minimap_2_tile,f))
    {
      return 20;
    }
    if(!p_putc(DMaps[i].minimap_2_cset,f))
    {
      return 21;
    }
    if(!p_iputw(DMaps[i].largemap_1_tile,f))
    {
      return 22;
    }
    if(!p_putc(DMaps[i].largemap_1_cset,f))
    {
      return 23;
    }
    if(!p_iputw(DMaps[i].largemap_2_tile,f))
    {
      return 24;
    }
    if(!p_putc(DMaps[i].largemap_2_cset,f))
    {
      return 25;
    }
    if(!pfwrite(&DMaps[i].tmusic,sizeof(DMaps[0].tmusic),f))
    {
      return 26;
    }
  }
  return 0;
}

int writemisc(PACKFILE *f, zquestheader *header, miscQdata *misc)
{
  dword section_id=ID_MISC;
  dword section_version=V_MISC;
  dword section_cversion=CV_MISC;
  word shops=count_shops(misc);
  word infos=count_infos(misc);
  word warprings=count_warprings(misc);
  word windwarps=count_windwarps(misc);
  word triforces=8;
  writesize=0;
//  dword section_size=0;
  dword section_size = sizeof(word) + shops * 3 * (sizeof(char) + sizeof(word))
                     + sizeof(word) + infos * 3 * 2 * sizeof(word)
                     + sizeof(word) + warprings * ((8 * (2 * sizeof(char))) + sizeof(char))
                     + sizeof(word) + windwarps * 2 * sizeof(char)
                     + triforces * sizeof(char)
                     + 21 * sizeof(char) + 6 * sizeof(word)
                     + 4 * sizeof(word)
                     + sizeof(word);

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


  //shops
  if(!p_iputw(shops,f))
  {
    return qe_invalid;
  }
  for (int i=0; i<shops; i++)
  {
    for (int j=0; j<3; j++)
    {
      if(!p_putc(misc->shop[i].item[j],f))
      {
        return qe_invalid;
      }
    }
    for (int j=0; j<3; j++)
    {
      if(!p_iputw(misc->shop[i].price[j],f))
      {
        return qe_invalid;
      }
    }
  }

  //infos
  if(!p_iputw(infos,f))
  {
    return qe_invalid;
  }
  for (int i=0; i<infos; i++)
  {
    for (int j=0; j<3; j++)
    {
      if(!p_iputw(misc->info[i].str[j],f))
      {
        return qe_invalid;
      }
    }
    for (int j=0; j<3; j++)
    {
      if(!p_iputw(misc->info[i].price[j],f))
      {
        return qe_invalid;
      }
    }
  }

  //warp rings
  if(!p_iputw(warprings,f))
  {
    return qe_invalid;
  }
  for (int i=0; i<warprings; i++)
  {
    for (int j=0; j<8; j++)
    {
      if(!p_putc(misc->warp[i].dmap[j],f))
      {
        return qe_invalid;
      }
    }
    for (int j=0; j<8; j++)
    {
      if(!p_putc(misc->warp[i].scr[j],f))
      {
        return qe_invalid;
      }
    }
    if(!p_putc(misc->warp[i].size,f))
    {
      return qe_invalid;
    }
  }

  //wind warps
  if(!p_iputw(windwarps,f))
  {
    return qe_invalid;
  }
  for (int i=0; i<windwarps; i++)
  {
    if(!p_putc(misc->wind[i].dmap,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->wind[i].scr,f))
    {
      return qe_invalid;
    }
  }

  //triforce pieces
  for (int i=0; i<triforces; i++)
  {
    if(!p_putc(misc->triforce[i],f))
    {
      return qe_invalid;
    }
  }

  //misc color data
  {                                                         //this brace is here to make it easier to fold all the color data code
    if(!p_putc(misc->colors.text,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.caption,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.overw_bg,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.dngn_bg,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.dngn_fg,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.cave_fg,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.bs_dk,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.bs_goal,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.compass_lt,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.compass_dk,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.subscr_bg,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.triframe_color,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.link_dot,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.bmap_bg,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.bmap_fg,f))
    {
      return qe_invalid;
    }

    if(!p_putc(misc->colors.triforce_cset,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.triframe_cset,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.overworld_map_cset,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.dungeon_map_cset,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.blueframe_cset,f))
    {
      return qe_invalid;
    }
    if(!p_iputw(misc->colors.triforce_tile,f))
    {
      return qe_invalid;
    }
    if(!p_iputw(misc->colors.triframe_tile,f))
    {
      return qe_invalid;
    }
    if(!p_iputw(misc->colors.overworld_map_tile,f))
    {
      return qe_invalid;
    }
    if(!p_iputw(misc->colors.dungeon_map_tile,f))
    {
      return qe_invalid;
    }
    if(!p_iputw(misc->colors.blueframe_tile,f))
    {
      return qe_invalid;
    }
    if(!p_iputw(misc->colors.HCpieces_tile,f))
    {
      return qe_invalid;
    }
    if(!p_putc(misc->colors.HCpieces_cset,f))
    {
      return qe_invalid;
    }
  }

  //save game icons
  for (int i=0; i<4; i++)
  {
    if(!p_iputw(misc->icons[i],f))
    {
      return qe_invalid;
    }
  }

  //end string
  if(!p_iputw(misc->endstring,f))
  {
    return qe_invalid;
  }

  return 0;
}

int writeitems(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_ITEMS;
  dword section_version=V_ITEMS;
  dword section_cversion=CV_ITEMS;
//  dword section_size=0;
  dword section_size = sizeof(word) + iMax * (sizeof(word) + 5 * sizeof(char) + sizeof(long));

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

  //finally...  section data
  if(!p_iputw(iMax,f))
  {
    return 5;
  }

  for (int i=0; i<iMax; i++)
  {
    if(!p_iputw(itemsbuf[i].tile,f))
    {
      return 6;
    }
    if(!p_putc(itemsbuf[i].misc,f))
    {
      return 7;
    }
    if(!p_putc(itemsbuf[i].csets,f))
    {
      return 8;
    }
    if(!p_putc(itemsbuf[i].frames,f))
    {
      return 9;
    }
    if(!p_putc(itemsbuf[i].speed,f))
    {
      return 10;
    }
    if(!p_putc(itemsbuf[i].delay,f))
    {

      return 11;
    }
    if(!p_iputl(itemsbuf[i].ltm,f))
    {
      return 12;
    }
  }
  return 0;
}

int writeweapons(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_WEAPONS;
  dword section_version=V_WEAPONS;
  dword section_cversion=CV_WEAPONS;
//  dword section_size=0;
  dword section_size = sizeof(word) + wMAX * (sizeof(word) + 5 * sizeof(char));

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

  //finally...  section data
  if(!p_iputw(wMAX,f))
  {
    return 5;
  }

  for (int i=0; i<wMAX; i++)
  {
    if(!p_iputw(wpnsbuf[i].tile,f))
    {

      return 6;
    }
    if(!p_putc(wpnsbuf[i].misc,f))
    {
      return 7;
    }
    if(!p_putc(wpnsbuf[i].csets,f))
    {
      return 8;
    }
    if(!p_putc(wpnsbuf[i].frames,f))
    {
      return 9;
    }
    if(!p_putc(wpnsbuf[i].speed,f))
    {
      return 10;
    }
    if(!p_putc(wpnsbuf[i].type,f))
    {
      return 11;
    }
  }
  return 0;
}

int writemaps(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_MAPS;
  dword section_version=V_MAPS;
  dword section_cversion=CV_MAPS;
//  dword section_size=0;
  dword section_size = sizeof(word) + map_count * MAPSCRS * 1298;

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

  //finally...  section data
  if(!p_iputw(map_count,f))
  {
    return 5;
  }

  int scr=0;

  for(int i=0; i<map_count && i<MAXMAPS2; i++)
  {
    for (int j=0; j<MAPSCRS; j++)
    {
      scr=i*MAPSCRS+j;

      if(!p_putc(TheMaps[scr].valid,f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].guy,f))
      {
        return qe_invalid;
      }

      {
        if(!p_iputw(TheMaps[scr].str,f))
        {
          return qe_invalid;
        }
      }

      if(!p_putc(TheMaps[scr].room,f))
      {
        return qe_invalid;
      }

      if(!p_putc(TheMaps[scr].item,f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].tilewarptype,f))
      {
        return qe_invalid;
      }

      {
        if(!p_iputw(TheMaps[scr].door_combo_set,f))
        {
          return qe_invalid;
        }
      }

      if(!p_putc(TheMaps[scr].warpreturnx,f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].warpreturny,f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].stairx,f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].stairy,f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].itemx,f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].itemy,f))
      {
        return qe_invalid;
      }

      if(!p_putc(TheMaps[scr].color,f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].enemyflags,f))
      {
        return qe_invalid;
      }
      for(int k=0; k<4; k++)
      {
        if(!p_putc(TheMaps[scr].door[k],f))
        {
          return qe_invalid;
        }
      }
      if(!p_putc(TheMaps[scr].tilewarpdmap,f))
      {
        return qe_invalid;

      }
      if(!p_putc(TheMaps[scr].tilewarpscr,f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].exitdir,f))
      {
        return qe_invalid;
      }

      for(int k=0; k<10; k++)
      {
        {
          if(!p_iputw(TheMaps[scr].enemy[k],f))
          {
            return qe_invalid;
          }
        }
      }

      if(!p_putc(TheMaps[scr].pattern,f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].sidewarptype,f))
      {

        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].warparrivalx,f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].warparrivaly,f))
      {
        return qe_invalid;
      }
      for(int k=0; k<4; k++)
      {
        if(!p_putc(TheMaps[scr].path[k],f))
        {
          return qe_invalid;
        }
      }
      if(!p_putc(TheMaps[scr].sidewarpscr,f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].sidewarpdmap,f))
      {
        return qe_invalid;
      }
      if(!p_iputw(TheMaps[scr].undercombo,f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].undercset,f))
      {
        return qe_invalid;
      }
      if(!p_iputw(TheMaps[scr].catchall,f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].flags,f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].flags2,f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].flags3,f))

      {
        return qe_invalid;
      }

      {
        for(int k=0; k<6; k++)
        {
          if(!p_putc(TheMaps[scr].layermap[k],f))
          {
            return qe_invalid;
          }
        }
        for(int k=0; k<6; k++)
        {
          if(!p_putc(TheMaps[scr].layerscreen[k],f))
          {
            return qe_invalid;
          }
        }
      }

      {
        for(int k=0; k<6; k++)
        {
          if(!p_putc(TheMaps[scr].layeropacity[k],f))
          {
            return qe_invalid;
          }
        }
      }

      {
        if(!p_iputw(TheMaps[scr].timedwarptics,f))
        {
          return qe_invalid;
        }
      }

      {
        for(int k=0; k<128; k++)
        {
          if(!p_iputw(TheMaps[scr].secretcombo[k],f))
          {
            return qe_invalid;
          }

        }
      }

      {
        for(int k=0; k<128; k++)
        {
          if(!p_putc(TheMaps[scr].secretcset[k],f))
          {
            return qe_invalid;
          }
        }
        for(int k=0; k<128; k++)
        {
          if(!p_putc(TheMaps[scr].secretflag[k],f))
          {
            return qe_invalid;
          }
        }
      }

      for(int k=0; k<16*11; k++)
      {
        if(!p_iputw(TheMaps[scr].data[k],f))
        {
          return qe_invalid;
        }
      }

      {
        for(int k=0; k<16*11; k++)
        {
          if(!p_putc(TheMaps[scr].sflag[k],f))
          {
            return qe_invalid;
          }
        }
      }

      {
        for(int k=0; k<16*11; k++)
        {
          if(!p_putc(TheMaps[scr].cset[k],f))
          {
            return qe_invalid;
          }
        }
      }
    }
  }
  return 0;
}

int writecombos(PACKFILE *f, word version, word build, word start_combo, word max_combos)
{
  //these are here to bypass compiler warnings about unused arguments
  version=version;
  build=build;

  word combos_used;
  dword section_id=ID_COMBOS;
  dword section_version=V_COMBOS;
  dword section_cversion=CV_COMBOS;
  //  dword section_size=0;
  combos_used = count_combos()-start_combo;
  combos_used = min(combos_used, max_combos);
  combos_used = min(combos_used, MAXCOMBOS);
  dword section_size = sizeof(word) + combos_used * 11;

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

  //finally...  section data
  combos_used=count_combos()-start_combo;
  combos_used=min(combos_used, max_combos);
  combos_used=min(combos_used, MAXCOMBOS);
  if(!p_iputw(combos_used,f))
  {
    return 5;
  }

  for (int i=start_combo; i<start_combo+combos_used; i++)
  {
    if(!p_iputw(combobuf[i].tile,f))
    {
      return 6;
    }
    if(!p_putc(combobuf[i].flip,f))
    {
      return 7;
    }
    if(!p_putc(combobuf[i].walk,f))
    {
      return 8;
    }
    if(!p_putc(combobuf[i].type,f))
    {
      return 9;
    }
    if(!p_putc(combobuf[i].csets,f))
    {
      return 10;
    }
    if(!p_putc(combobuf[i].frames,f))
    {
      return 11;
    }
    if(!p_putc(combobuf[i].speed,f))
    {
      return 12;
    }
    if(!p_iputw(combobuf[i].nextcombo,f))
    {
      return 13;
    }
    if(!p_putc(combobuf[i].nextcset,f))
    {
      return 14;
    }
  }
  return 0;
}

int writecolordata(PACKFILE *f, miscQdata *misc, word version, word build, word start_cset, word max_csets)
{
  //these are here to bypass compiler warnings about unused arguments
  version=version;
  build=build;
  start_cset=start_cset;
  max_csets=max_csets;

  dword section_id=ID_CSETS;
  dword section_version=V_CSETS;
  dword section_cversion=CV_CSETS;
  //  dword section_size=0;
  int palcycles = count_palcycles(misc);
  dword section_size = newpsTOTAL
                     + MAXLEVELS * PALNAMESIZE
                     + sizeof(word) + palcycles * 9;

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

  //finally...  section data
  if(!pfwrite(colordata,newpsTOTAL,f))
  {
    return 5;
  }
  if(!pfwrite(palnames,MAXLEVELS*PALNAMESIZE,f))
  {
    return 6;
  }
  if(!p_iputw(palcycles,f))
  {
    return 15;
  }
  for (int i=0; i<palcycles; i++)
  {
    for (int j=0; j<3; j++)
    {
      if(!p_putc(misc->cycles[i][j].first,f))
      {
        return 16;
      }
    }
    for (int j=0; j<3; j++)
    {
      if(!p_putc(misc->cycles[i][j].count,f))
      {
        return 17;
      }
    }
    for (int j=0; j<3; j++)
    {
      if(!p_putc(misc->cycles[i][j].speed,f))
      {
        return 18;
      }
    }
  }
  return 0;
}

int writestrings(PACKFILE *f, word version, word build, word start_msgstr, word max_msgstrs)
{
  dword section_id=ID_STRINGS;
  dword section_version=V_STRINGS;
  dword section_cversion=CV_STRINGS;
//  dword section_size=0;
  dword section_size = sizeof(word)
                     + msg_count * (sizeof(MsgStrings[0].s) + sizeof(word));

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

  //finally...  section data
  if(!p_iputw(msg_count,f))
  {
    return qe_invalid;
  }

  for (int i=0; i<msg_count; i++)
  {
    if(!pfwrite(MsgStrings[i].s,sizeof(MsgStrings[0].s),f))
    {
      return qe_invalid;
    }
    if(!p_iputw(MsgStrings[i].nextstring,f))
    {
      return qe_invalid;
    }
  }
  return 0;
}

int writetiles(PACKFILE *f, word version, word build, word start_tile, word max_tiles)
{
  //these are here to bypass compiler warnings about unused arguments
  version=version;
  build=build;

  word tiles_used;
  dword section_id=ID_TILES;
  dword section_version=V_TILES;
  dword section_cversion=CV_TILES;
  //  dword section_size=0;
  tiles_used = count_tiles(tilebuf)-start_tile;
  tiles_used = min(tiles_used, max_tiles);
  tiles_used = min(tiles_used, NEWMAXTILES);
  dword section_size = sizeof(word) + (int)tiles_used * (int)SINGLE_TILE_SIZE;

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

  //finally...  section data
  tiles_used=count_tiles(tilebuf)-start_tile;
  tiles_used=min(tiles_used, max_tiles);
  tiles_used=min(tiles_used, NEWMAXTILES);
  if(!p_iputw(tiles_used,f))
  {
    return 5;
  }

  for (dword i=0; i<tiles_used; ++i)
  {
    if(!pfwrite(tilebuf+((start_tile+i)*SINGLE_TILE_SIZE),SINGLE_TILE_SIZE,f))
    {
      return 7;
    }
  }
  return 0;
}

int writemidis(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_MIDIS;
  dword section_version=V_MIDIS;
  dword section_cversion=CV_MIDIS;
  dword section_size = 0;
  {
    int midi_count = 0;
    for(int i = 0; i < MAXMIDIS; i++ )
      if( get_bit(midi_flags, i) )
      {
        midi_count++;
        for(int c = 0; c < MIDI_TRACKS; c++ )
          section_size += customMIDIs[i].midi->track[c].len;
      }
    section_size += sizeof(midi_flags)
                  + midi_count * (sizeof(customMIDIs[0].title) + (3 + MIDI_TRACKS) * sizeof(long) + 3 * sizeof(word));
  }

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

  //finally...  section data
  if(!pfwrite(midi_flags,sizeof(midi_flags),f))
  {
    return 5;
  }
  for(int i=0; i<MAXMIDIS; i++)
  {
    if(get_bit(midi_flags,i))
    {
      if(!pfwrite(&customMIDIs[i].title,sizeof(customMIDIs[0].title),f))
      {
        return 6;
      }
      if(!p_iputl(customMIDIs[i].start,f))
      {
        return 7;
      }
      if(!p_iputl(customMIDIs[i].loop_start,f))
      {
        return 8;
      }
      if(!p_iputl(customMIDIs[i].loop_end,f))
      {
        return 9;
      }
      if(!p_iputw(customMIDIs[i].loop,f))
      {
        return 10;
      }
      if(!p_iputw(customMIDIs[i].volume,f))
      {
        return 11;
      }
      if(!write_midi(customMIDIs[i].midi,f))
      {
        return 12;
      }
    }
  }
  return 0;
}

int writecheats(PACKFILE *f, zquestheader *header)

{
  dword section_id=ID_CHEATS;
  dword section_version=V_CHEATS;
  dword section_cversion=CV_CHEATS;
//  dword section_size=0;
  dword section_size = sizeof(char);
  if(header->data_flags[ZQ_CHEATS2])
    section_size += sizeof(long) + sizeof(zcheats.codes);

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

  //finally...  section data
  if(!p_putc(header->data_flags[ZQ_CHEATS2],f))
  {
    return 5;
  }

  if(header->data_flags[ZQ_CHEATS2])
  {
    if(!p_iputl(zcheats.flags,f))
    {
      return 6;
    }
    if(!pfwrite(&zcheats.codes, sizeof(zcheats.codes), f))
    {
      return 7;
    }
  }

  return 0;
}

int writeguys(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_GUYS;
  dword section_version=V_GUYS;
  dword section_cversion=CV_GUYS;
  dword section_size=0;

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

  //finally...  section data

  //well, defaults are used for now.  sorry.

  return 0;
}

int writeinitdata(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_INITDATA;
  dword section_version=V_INITDATA;
  dword section_cversion=CV_INITDATA;
//  dword section_size=0;
  dword section_size = 174;

  zinit.last_map=Map.getCurrMap();
  zinit.last_screen=Map.getCurrScr();

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

  //finally...  section data
  if(!p_putc(zinit.raft,f))
  {
    return 5;
  }
  if(!p_putc(zinit.ladder,f))
  {
    return 6;
  }
  if(!p_putc(zinit.book,f))
  {
    return 7;
  }
  if(!p_putc(zinit.key,f))
  {
    return 8;
  }
  if(!p_putc(zinit.flippers,f))
  {
    return 9;
  }
  if(!p_putc(zinit.boots,f))
  {
    return 10;
  }
  if(!p_putc(zinit.ring,f))
  {
    return 11;
  }
  if(!p_putc(zinit.sword,f))
  {
    return 12;
  }
  if(!p_putc(zinit.shield,f))
  {
    return 13;
  }
  if(!p_putc(zinit.wallet,f))
  {
    return 14;
  }
  if(!p_putc(zinit.bracelet,f))
  {
    return 15;
  }
  if(!p_putc(zinit.amulet,f))
  {
    return 16;
  }
  if(!p_putc(zinit.bow,f))
  {
    return 17;
  }
  if(!p_putc(zinit.candle,f))
  {
    return 18;
  }
  if(!p_putc(zinit.boomerang,f))
  {
    return 19;
  }
  if(!p_putc(zinit.arrow,f))
  {
    return 20;
  }

  if(!p_putc(zinit.potion,f))
  {
    return 21;
  }
  if(!p_putc(zinit.whistle,f))
  {
    return 22;
  }
  if(!p_putc(zinit.bombs,f))
  {
    return 23;
  }
  if(!p_putc(zinit.super_bombs,f))
  {
    return 24;
  }
  if(!p_putc(zinit.wand,f))
  {
    return 25;
  }
  if(!p_putc(zinit.letter,f))

  {
    return 26;
  }
  if(!p_putc(zinit.lens,f))
  {
    return 27;
  }
  if(!p_putc(zinit.hookshot,f))
  {
    return 28;
  }
  if(!p_putc(zinit.bait,f))
  {
    return 29;
  }
  if(!p_putc(zinit.hammer,f))
  {
    return 30;
  }
  if(!p_putc(zinit.dins_fire,f))
  {
    return 31;
  }
  if(!p_putc(zinit.farores_wind,f))
  {
    return 32;
  }
  if(!p_putc(zinit.nayrus_love,f))
  {
    return 33;
  }
  if(!p_putc(zinit.cloak,f))
  {
    return 34;
  }
  if(!p_putc(zinit.hc,f))
  {
    return 35;
  }
  if(!p_putc(zinit.start_heart,f))
  {
    return 36;
  }
  if(!p_putc(zinit.cont_heart,f))
  {
    return 37;
  }
  if(!p_putc(zinit.hcp,f))
  {
    return 38;
  }
  if(!p_putc(zinit.max_bombs,f))
  {
    return 39;
  }
  if(!p_putc(zinit.keys,f))
  {
    return 40;
  }
  if(!p_iputw(zinit.rupies,f))
  {
    return 41;
  }
  if(!p_putc(zinit.triforce,f))
  {
    return 42;
  }
  for (int i=0; i<32; i++)
  {
    if(!p_putc(zinit.map[i],f))
    {
      return 43;
    }
  }
  for (int i=0; i<32; i++)
  {
    if(!p_putc(zinit.compass[i],f))
    {
      return 44;
    }
  }
  for (int i=0; i<32; i++)
  {
    if(!p_putc(zinit.boss_key[i],f))
    {
      return 45;
    }
  }
  for (int i=0; i<16; i++)
  {
    if(!p_putc(zinit.misc[i],f))
    {
      return 46;
    }
  }
  for (int i=0; i<4; i++)
  {
    if(!p_putc(zinit.sword_hearts[i],f))
    {
      return 47;
    }
  }
  if(!p_putc(zinit.last_map,f))
  {
    return 48;
  }
  if(!p_putc(zinit.last_screen,f))
  {
    return 49;
  }
  if(!p_putc(zinit.max_magic,f))
  {
    return 50;
  }
  if(!p_putc(zinit.magic,f))
  {
    return 51;
  }
  for (int i=0; i<4; i++)
  {
    if(!p_putc(zinit.beam_hearts[i],f))
    {
      return 52;
    }
  }
  if(!p_putc(zinit.beam_percent,f))
  {
    return 53;
  }
  for (int i=0; i<4; i++)
  {
    if(!p_putc(zinit.beam_power[i],f))
    {
      return 54;
    }
  }
  if(!p_putc(zinit.hookshot_links,f))
  {
    return 55;
  }
  if(!p_putc(zinit.msg_more_x,f))
  {
    return 56;
  }
  if(!p_putc(zinit.msg_more_y,f))
  {
    return 57;
  }
  if(!p_putc(zinit.subscreen,f))
  {
    return 58;
  }
  if(!p_putc(zinit.start_dmap,f))
  {
    return 59;
  }
  if(!p_putc(zinit.linkwalkstyle,f))
  {
    return 60;
  }
  return 0;
}

int save_unencoded_quest(char *filename, bool compressed)
{
  reset_combo_animations();
  strcpy(header.id_str,QH_NEWIDSTR);
  header.zelda_version = ZELDA_VERSION;
  header.internal = INTERNAL_VERSION;
  // header.str_count = msg_count;
  // header.data_flags[ZQ_TILES] = usetiles;
  header.data_flags[ZQ_TILES] = true;
  header.data_flags[ZQ_CHEATS2] = 1;
  header.build=VERSION_BUILD;
  for(int i=0; i<MAXMIDIS; i++)
  {
    set_bit(midi_flags,i,int(customMIDIs[i].midi!=NULL));
  }

  char keyfilename[256];
  // word combos_used;
  // word tiles_used;
  replace_extension(keyfilename, filepath, "key", 255);




  box_start(1, "Saving Quest", lfont, font, true);
  box_out("Saving Quest...");
  box_eol();
  box_eol();

  PACKFILE *f = pack_fopen(filename,compressed?F_WRITE_PACKED:F_WRITE);
  if(!f)
    return 1;

  box_out("Writing Header...");
  if (writeheader(f,&header)!=0)
  {
    return 2;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Rules...");
  if (writerules(f,&header)!=0)
  {
    return 3;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Strings...");
  if (writestrings(f, ZELDA_VERSION, VERSION_BUILD, 0, MAXMSGS)!=0)
  {
    return 4;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Doors...");
  if (writedoorcombosets(f,&header)!=0)
  {
    return 5;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing DMaps...");
  if (writedmaps(f,header.zelda_version,header.build,0,MAXDMAPS)!=0)
  {
    return 6;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Misc. Data...");
  if (writemisc(f,&header,&misc)!=0)
  {
    return 7;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Items...");
  if (writeitems(f,&header)!=0)
  {
    return 8;
  }

  box_out("okay.");
  box_eol();

  box_out("Writing Weapons...");
  if (writeweapons(f,&header)!=0)
  {
    return 9;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Maps...");
  if (writemaps(f,&header)!=0)
  {
    return 10;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Combos...");
  if (writecombos(f,header.zelda_version,header.build,0,MAXCOMBOS)!=0)
  {
    return 11;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Color Data...");
  if (writecolordata(f,&misc,header.zelda_version,header.build,0,newpdTOTAL)!=0)
  {
    return 12;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Tiles...");
  if (writetiles(f,header.zelda_version,header.build,0,NEWMAXTILES)!=0)
  {

    return 13;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing MIDIs...");
  if (writemidis(f,&header)!=0)
  {
    return 14;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Cheat Codes...");
  if (writecheats(f,&header)!=0)
  {
    return 15;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Init. Data...");
  if (writeinitdata(f,&header)!=0)
  {
    return 16;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Enemy Data...");
  if (writeguys(f,&header)!=0)
  {
    return 17;
  }
  box_out("okay.");
  box_eol();

  pack_fclose(f);

  if (header.use_keyfile&&compressed)
  {
    char password[32];
    packfile_password(NULL);
    PACKFILE *fp = pack_fopen(keyfilename, F_WRITE);
    char msg[80];
    memset(msg,0,80);
    sprintf(msg, "ZQuest Auto-Generated Quest Password Key File.  DO NOT EDIT!");
    msg[78]=13;msg[79]=10;
    pfwrite(msg, 80, fp);
    p_iputw(header.zelda_version,fp);
    p_putc(header.build,fp);
    get_questpwd(password);
    pfwrite(password, 30, fp);
    memset(password,0,32);
    pack_fclose(fp);
    packfile_password(datapwd);
  }
  return 0;
}

int save_quest(char *filename)
{

  char tmpbuf[L_tmpnam];
  char *tmpfilename = tmpnam(tmpbuf);

  packfile_password(datapwd);
  int ret = save_unencoded_quest(tmpfilename, true);





//  int ret = save_unencoded_quest(filename, false);
//  return ret;




  packfile_password(NULL);
  if(ret == 0)
  {
    box_out("Encrypting...");
    ret = encode_file_007(tmpfilename, filename,((INTERNAL_VERSION + rand()) & 0xffff) + 0x413F0000, ENC_STR, ENC_METHOD_MAX-1);
    if(ret)
    {
      ret += 100;
    }
  }

  box_out("okay.");
  box_eol();
  delete_file(tmpfilename);

  return ret;
}

bool load_zgp(char *path)
{
  dword section_id;
  dword section_version;
  dword section_cversion;

  PACKFILE *f=pack_fopen(path,F_READ);
  if(!f)
    return false;

  if(!p_mgetl(&section_id,f,true))
  {
    pack_fclose(f);
    return false;
  }
  if(section_id!=ID_GRAPHICSPACK)
  {
    pack_fclose(f);
    return false;
  }

  //section version info
  if(!p_igetw(&section_version,f,true))
  {
    return 2;
  }
  if(!p_igetw(&section_cversion,f,true))
  {
    return 3;
  }

  //tiles
  if(!p_mgetl(&section_id,f,true))
  {
    pack_fclose(f);
    return false;
  }
  if(section_id==ID_TILES)
  {
    if (readtiles(f, tilebuf, NULL, ZELDA_VERSION, VERSION_BUILD, 0, NEWMAXTILES, false, true)!=0)
    {
      pack_fclose(f);
      init_tiles(true, &header);
      return false;
    }
  }
  else
  {
    pack_fclose(f);
    return false;
  }

  //combos
  if(!p_mgetl(&section_id,f,true))
  {
    pack_fclose(f);
    return false;
  }
  if(section_id==ID_COMBOS)
  {
    if (readcombos(f, NULL, ZELDA_VERSION, VERSION_BUILD, 0, MAXCOMBOS, true)!=0)
    {
      pack_fclose(f);
      //      init_combos(true, &header);
      return false;
    }
  }
  else
  {
    pack_fclose(f);
    return false;
  }

  //palettes
  if(!p_mgetl(&section_id,f,true))
  {
    pack_fclose(f);
    return false;
  }
  if(section_id==ID_CSETS)
  {
    if (readcolordata(f, &misc, ZELDA_VERSION, VERSION_BUILD, 0, newpdTOTAL, true)!=0)
    {
      pack_fclose(f);
      return false;
    }
  }
  else
  {
    pack_fclose(f);
    return false;
  }

  //items
  if(!p_mgetl(&section_id,f,true))
  {
    pack_fclose(f);
    return false;
  }
  if(section_id==ID_ITEMS)
  {
    if (readitems(f, ZELDA_VERSION, VERSION_BUILD, true)!=0)
    {
      pack_fclose(f);
      return false;
    }
  }
  else
  {
    pack_fclose(f);
    return false;
  }

  //weapons
  if(!p_mgetl(&section_id,f,true))
  {
    pack_fclose(f);
    return false;
  }
  if(section_id==ID_WEAPONS)
  {
    if (readweapons(f, &header, true)!=0)
    {
      pack_fclose(f);
      return false;
    }
  }
  else
  {
    pack_fclose(f);
    return false;
  }

  //read the triforce pieces info and make sure it worked
  //really do this?

  //read the door combo sets and make sure it worked
  if(!p_mgetl(&section_id,f,true))
  {
    pack_fclose(f);
    return false;
  }
  if(section_id==ID_DOORS)
  {
    if (readdoorcombosets(f, &header, true)!=0)
    {
      pack_fclose(f);
      return false;
    }
  }
  else
  {
    pack_fclose(f);
    return false;
  }

  //read the template screens and make sure it worked
  //really do this?

  //yay!  it worked!  close the file and say everything was ok.
  loadlvlpal(Color);
  setup_combo_animations();
  pack_fclose(f);
  return true;
}

bool save_zgp(char *path)
{
//  jwin_alert("Error","This feature not yet implemented.",NULL,NULL,"O&K",NULL,'k',0,lfont);
//  return false;
  reset_combo_animations();

  //open the file
  PACKFILE *f=pack_fopen(path,F_WRITE);
  if(!f)
    return false;

  dword section_id=ID_GRAPHICSPACK;
  dword section_version=V_GRAPHICSPACK;
  dword section_cversion=CV_GRAPHICSPACK;

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

  //tiles
  if (writetiles(f, ZELDA_VERSION, VERSION_BUILD, 0, NEWMAXTILES)!=0)
  {
    pack_fclose(f);
    return false;
  }
  //combos
  if (writecombos(f, ZELDA_VERSION, VERSION_BUILD, 0, MAXCOMBOS)!=0)
  {
    pack_fclose(f);
    return false;
  }

  //palettes
  if (writecolordata(f, &misc, ZELDA_VERSION, VERSION_BUILD, 0, newpdTOTAL)!=0)
  {
    pack_fclose(f);
    return false;
  }

  //items
  if (writeitems(f, &header)!=0)
  {
    pack_fclose(f);
    return false;
  }

  //weapons
  if (writeweapons(f, &header)!=0)
  {
    pack_fclose(f);
    return false;
  }

  //write the triforce pieces info and make sure it worked
  //really do this?

  //write the door combo sets and make sure it worked
  if (writedoorcombosets(f, &header)!=0)
  {
    pack_fclose(f);
    return false;
  }

  //write the template screens and make sure it worked
  //really do this?

  pack_fclose(f);
  return true;
}

bool load_zqt(char *path)
{
  word tiles_used;
  word combos_used;
  clear_tiles(tilebuf);
  clear_combos();
  //open the file
  PACKFILE *f = pack_fopen(path, F_READ_PACKED);
  if(!f)
    return false;

  short version;
  byte build;

  //read the version and make sure it worked
  if(!p_igetw(&version,f,true))

    goto error;

  //read the build and make sure it worked
  if(!p_getc(&build,f,true))
    goto error;

  //read the tile info and make sure it worked
  if(!p_igetw(&tiles_used,f,true))
    goto error;
  if(!pfread(tilebuf,dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)),f,true))
    goto error;

  //read the combo info and make sure it worked
  if(!p_igetw(&combos_used,f,true))
  {
    goto error;
  }
  if(!pfread(combobuf,sizeof(newcombo)*combos_used,f,true))
  {
    goto error;
  }

  //read the palette info and make sure it worked
  if(!pfread(colordata,newpsTOTAL,f,true))
  {
    goto error;
  }
  if(!pfread(misc.cycles,sizeof(palcycle)*256*3,f,true))
  {
    goto error;
  }
  if(!pfread(palnames,MAXLEVELS*PALNAMESIZE,f,true))
  {
    goto error;
  }
  loadlvlpal(Color);

  //read the sprite info and make sure it worked
  if(!pfread(itemsbuf,sizeof(itemdata)*MAXITEMS,f,true))
  {
    goto error;
  }
  if(!pfread(wpnsbuf,sizeof(wpndata)*MAXWPNS,f,true))
  {
    goto error;
  }

  //read the triforce pieces info and make sure it worked
  for (int i=0; i<8; ++i)
  {
    if(!p_getc(&(misc.triforce[i]),f,true))
    {
      goto error;
    }
  }

  //read the game icons info and make sure it worked
  for (int i=0; i<4; i++)
  {
    if(!p_igetw(&(misc.icons[i]),f,true))
    {
      goto error;
    }
  }

  //read the misc colors info and map styles info and make sure it worked
  if(!pfread(&misc.colors,sizeof(zcolors),f,true))
  {
    goto error;
  }

  //read the template screens and make sure it worked
  byte num_maps;
  if(!p_getc(&num_maps,f,true))
  {
    goto error;
  }
  if (map_count<num_maps)
  {
    setMapCount2(num_maps,true);
  }
  if(!pfread(TheMaps+128,sizeof(mapscr)*TEMPLATES,f,true))
  {
    goto error;
  }
  if (num_maps>1)                                           //dungeon templates
  {
    if(!pfread(TheMaps+MAPSCRS+128,sizeof(mapscr)*TEMPLATES,f,true))
    {
      goto error;
    }
  }

  for(int i=0; i<MAXDOORCOMBOSETS; i++)
  {
    memset(DoorComboSets+i, 0, sizeof(DoorComboSet));
  }
  door_combo_set_count=0;

  if(!p_igetw(&door_combo_set_count,f,true))
    goto error;
  if(!pfread(DoorComboSets,sizeof(DoorComboSet)*door_combo_set_count,f,true))
    goto error;

  setup_combo_animations();

  //yay!  it worked!  close the file and say everything was ok.
  pack_fclose(f);
  return true;

  error:
  pack_fclose(f);
  return false;
}

bool save_zqt(char *path)
{
  reset_combo_animations();

  //open the file
  //  PACKFILE *f=pack_fopen(path,F_WRITE);
  PACKFILE *f = pack_fopen(path, F_WRITE_PACKED);
  if(!f)
    return false;

  short version=ZELDA_VERSION;
  byte  build=VERSION_BUILD;

  //write the version and make sure it worked
  if(!p_iputw(version,f))
    goto error;

  //write the build and make sure it worked
  if(!p_putc(build,f))
    goto error;

  //write the tile info and make sure it worked
  word tiles_used;
  tiles_used=count_tiles(tilebuf);
  if(!p_iputw(tiles_used,f))
    goto error;
  if(!pfwrite(tilebuf,dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)),f))
    goto error;

  //write the combo info and make sure it worked
  word combos_used;
  combos_used=count_combos();
  if(!p_iputw(combos_used,f))
  {
    goto error;
  }
  if(!pfwrite(combobuf,sizeof(newcombo)*combos_used,f))
  {
    goto error;
  }

  //write the palette info and make sure it worked
  if(!pfwrite(colordata,newpsTOTAL,f))
  {
    goto error;
  }
  if(!pfwrite(misc.cycles,sizeof(palcycle)*256*3,f))
  {
    goto error;
  }
  if(!pfwrite(palnames,MAXLEVELS*PALNAMESIZE,f))
  {
    goto error;
  }

  //write the sprite info and make sure it worked
  if(!pfwrite(itemsbuf,sizeof(itemdata)*MAXITEMS,f))
  {
    goto error;
  }
  if(!pfwrite(wpnsbuf,sizeof(wpndata)*MAXWPNS,f))
  {
    goto error;
  }

  //write the triforce pieces info and make sure it worked
  for (int i=0; i<8; i++)
  {
    if(!p_putc(misc.triforce[i],f))
    {
      goto error;
    }
  }

  //write the game icons info and make sure it worked
  for (int i=0; i<4; i++)
  {
    if(!p_iputw(misc.icons[i],f))
    {
      goto error;
    }
  }

  //write the misc colors info and map styles info and make sure it worked
  if(!pfwrite(&misc.colors,sizeof(zcolors),f))
  {
    goto error;
  }

  //write the template screens and make sure it worked
  byte num_maps;
  num_maps=(((map_count)<2)?1:2);
  if(!p_putc(num_maps,f))
  {
    goto error;
  }
  if(!pfwrite(TheMaps+128,sizeof(mapscr)*TEMPLATES,f))
  {
    goto error;
  }
  if (num_maps>1)                                           //dungeon templates
  {
    if(!pfwrite(TheMaps+MAPSCRS+128,sizeof(mapscr)*TEMPLATES,f))
    {
      goto error;
    }
  }

  if(!p_iputw(door_combo_set_count,f))
  {
    pack_fclose(f);
    return 3;
  }
  if(!pfwrite(DoorComboSets,sizeof(DoorComboSet)*door_combo_set_count,f))
  {
    pack_fclose(f);
    return 3;
  }

  //yay!  it worked!  close the file and say everything was ok.
  pack_fclose(f);
  return true;

  error:
  pack_fclose(f);
  return false;

}

bool save_qsu(char *path)
{
  reset_combo_animations();
  strcpy(header.id_str,QH_IDSTR);
  char tmppwd[30];
  memcpy(tmppwd,header.password,30);
  short tmppwdkey;
  memset(header.password, 0, 30);
  tmppwdkey=header.pwdkey;
  header.pwdkey=0;
  header.zelda_version = ZELDA_VERSION;
  header.internal = INTERNAL_VERSION;
  // header.str_count = msg_count;
  // header.data_flags[ZQ_TILES] = usetiles;
  header.data_flags[ZQ_TILES] = true;
  header.data_flags[ZQ_CHEATS2] = 1;
  header.build=VERSION_BUILD;

  word combos_used=count_combos();
  word tiles_used=count_tiles(tilebuf);

  for(int i=0; i<MAXMIDIS; i++)
    set_bit(midi_flags,i,int(customMIDIs[i].midi!=NULL));

  PACKFILE *f = pack_fopen(path,F_WRITE);
  if(!f)
    goto error;
  if(!pfwrite(&header,sizeof(zquestheader),f))
  {
    goto error;
  }
  if(!p_iputw(msg_count,f))
  {
    goto error;
  }
  if(!pfwrite(MsgStrings,sizeof(MsgStr)*msg_count,f))
  {
    goto error;
  }

  // goto done;
  if(!p_iputw(door_combo_set_count,f))
  {
    goto error;
  }
  if(!pfwrite(DoorComboSets,sizeof(DoorComboSet)*door_combo_set_count,f))
  {
    goto error;
  }
  if(!pfwrite(DMaps,sizeof(dmap)*MAXDMAPS,f))
  {
    goto error;
  }
  if(!pfwrite(&misc,sizeof(miscQdata),f))
  {
    goto error;
  }
  if(!pfwrite(itemsbuf,sizeof(itemdata)*MAXITEMS,f))
  {
    goto error;
  }
  if(!pfwrite(wpnsbuf,sizeof(wpndata)*MAXWPNS,f))
  {
    goto error;
  }

  for(int i=0; i<map_count; i++)
  {
    if(!pfwrite(TheMaps+(i*MAPSCRS),sizeof(mapscr)*MAPSCRS,f))
    {
      goto error;

    }
  }

  if(!p_iputw(combos_used,f))
  {
    goto error;
  }
  if(!pfwrite(combobuf,sizeof(newcombo)*combos_used,f))
  {
    goto error;
  }
  if(!pfwrite(colordata,newpsTOTAL,f))
  {
    goto error;
  }
  if(!pfwrite(palnames,MAXLEVELS*PALNAMESIZE,f))
  {
    goto error;
  }

  if(!p_iputw(tiles_used,f))
  {
    goto error;
  }
  if(!pfwrite(tilebuf,dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)),f))
  {
    goto error;
  }

  for(int i=0; i<MAXMIDIS; i++)
  {
    if(get_bit(midi_flags,i))
    {
      if((!pfwrite(customMIDIs+i,sizeof(music),f)) ||
        !write_unencoded_midi(customMIDIs[i].midi,f) )
      {
        goto error;
      }

    }
  }

  if(!pfwrite(&zcheats,sizeof(ZCHEATS),f))
  {
    goto error;
  }

  zinit.last_map=Map.getCurrMap();
  zinit.last_screen=Map.getCurrScr();

  if(!pfwrite(&zinit,sizeof(zinitdata),f))
  {
    goto error;
  }

  pack_fclose(f);
  strcpy(header.password, tmppwd);
  header.pwdkey=tmppwdkey;
  return true;

  error:
  pack_fclose(f);
  return false;
}

bool load_qsu(char *path)
{
  word combos_used=count_combos();
  word tiles_used=count_tiles(tilebuf);
  int i;

  PACKFILE *f = pack_fopen(path,F_READ);
  if(!f)
    goto error;
  if(!pfread(&header,sizeof(zquestheader),f,true))
  {
    goto error;
  }
  if(!p_igetw(&msg_count,f,true))
  {
    goto error;
  }
  if(!pfread(MsgStrings,sizeof(MsgStr)*msg_count,f,true))
  {
    goto error;
  }
  if(!p_igetw(&door_combo_set_count,f,true))
  {
    goto error;
  }
  if(!pfread(DoorComboSets,sizeof(DoorComboSet)*door_combo_set_count,f,true))
  {
    goto error;
  }
  if(!pfread(DMaps,sizeof(dmap)*MAXDMAPS,f,true))
  {
    goto error;
  }
  if(!pfread(&misc,sizeof(miscQdata),f,true))
  {
    goto error;
  }
  if(!pfread(itemsbuf,sizeof(itemdata)*MAXITEMS,f,true))
  {
    goto error;
  }
  if(!pfread(wpnsbuf,sizeof(wpndata)*MAXWPNS,f,true))
  {
    goto error;
  }

  free(TheMaps);
  if(!(TheMaps=(mapscr*)malloc(sizeof(mapscr)*MAPSCRS*map_count)))
  {
    goto error;
  }

  for(int i=0; i<map_count; i++)
  {
    if(!pfread(TheMaps+(i*MAPSCRS),sizeof(mapscr)*MAPSCRS,f,true))
    {

      goto error;
    }
  }

  if(!p_igetw(&combos_used,f,true))
  {
    goto error;
  }
  if(!pfread(combobuf,sizeof(newcombo)*combos_used,f,true))
  {
    goto error;
  }
  if(!pfread(colordata,newpsTOTAL,f,true))
  {
    goto error;
  }
  if(!pfread(palnames,MAXLEVELS*PALNAMESIZE,f,true))
  {

    goto error;
  }

  if(!p_igetw(&tiles_used,f,true))
  {
    goto error;
  }
  if(!pfread(tilebuf,dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)),f,true))
  {
    goto error;
  }

  for(i=0; i<MAXMIDIS; i++)
  {
    if(get_bit(midi_flags,i))
    {
      if((!pfread(customMIDIs+i,sizeof(music),f,true)) ||
        !(customMIDIs[i].midi=read_unencoded_midi(f)) )
      {
        goto error;
      }

    }
  }

  if(!pfread(&zcheats,sizeof(ZCHEATS),f,true))
  {
    goto error;
  }

  zinit.last_map=Map.getCurrMap();
  zinit.last_screen=Map.getCurrScr();
  if(!pfread(&zinit,sizeof(zinitdata),f,true))
  {
    goto error;
  }

  pack_fclose(f);
  return true;

  error:
  pack_fclose(f);
  return false;

}

void center_zq_class_dialogs()
{
  jwin_center_dialog(pwd_dlg);
}
