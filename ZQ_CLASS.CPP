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

#include "gui.h"
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
#include "maps.h"
#include "zc_sys.h"
#include "md5.h"
#include "zc_custom.h"
#include "subscr.h"
#include <string.h>

#define EPSILON 0.01 // Define your own tolerance
#define FLOAT_EQ(x,v) (((v - EPSILON) < x) && (x <( v + EPSILON)))

//const char zqsheader[30]="Zelda Classic String Table\n\x01";


zmap Map;
int prv_mode=0;
short ffposx[32]={-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
                  -1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000};
short ffposy[32]={-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
                  -1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000};
float ffprvx[32]={-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
                  -1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000};
float ffprvy[32]={-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
                  -1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000};

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


mapscr* zmap::get_prvscr() { return &prvscr; }

zmap::zmap()
{
  can_undo=can_paste=false;
  prv_cmbcycle=0;
  prv_advance=0;
  prv_freeze=0;
}
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
  word version, build, dummy, sversion=0;
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
  if(!p_igetw(&sversion,f,true))
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
    readmapscreen(f, &header, &temp_mapscr, sversion);
  }
  readmapscreen(f, &header, TheMaps+128, sversion);
  readmapscreen(f, &header, TheMaps+129, sversion);
  for (int i=0; i<(MAPSCRS-(MAPSCRSNORMAL+2)); ++i)
  {
    readmapscreen(f, &header, &temp_mapscr, sversion);
  }
  for (int i=0; i<MAPSCRSNORMAL; ++i)
  {
    readmapscreen(f, &header, &temp_mapscr, sversion);
  }
  readmapscreen(f, &header, TheMaps+MAPSCRS+128, sversion);
  readmapscreen(f, &header, TheMaps+MAPSCRS+129, sversion);

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
void zmap::set_prvscr(int map, int scr)
{
  prvscr=TheMaps[(map*MAPSCRS)+scr];
  prvlayers[0]=TheMaps[((prvscr.layermap[0]?prvscr.layermap[0]-1:0)*MAPSCRS)+prvscr.layerscreen[0]];
  prvlayers[1]=TheMaps[((prvscr.layermap[1]?prvscr.layermap[1]-1:0)*MAPSCRS)+prvscr.layerscreen[1]];
  prvlayers[2]=TheMaps[((prvscr.layermap[2]?prvscr.layermap[2]-1:0)*MAPSCRS)+prvscr.layerscreen[2]];
  prvlayers[3]=TheMaps[((prvscr.layermap[3]?prvscr.layermap[3]-1:0)*MAPSCRS)+prvscr.layerscreen[3]];
  prvlayers[4]=TheMaps[((prvscr.layermap[4]?prvscr.layermap[4]-1:0)*MAPSCRS)+prvscr.layerscreen[4]];
  prvlayers[5]=TheMaps[((prvscr.layermap[5]?prvscr.layermap[5]-1:0)*MAPSCRS)+prvscr.layerscreen[5]];
  prv_map=map;
  prv_scr=scr;
}
int zmap:: getCurrMap()                     { return currmap; }
bool zmap::isDark()                         { return (screens[currscr].flags&4)!=0; }
void zmap::setCurrMap(int index)
{
  int oldmap=currmap;
  scrpos[currmap]=currscr;
  currmap=bound(index,0,map_count);
  screens=TheMaps+(currmap*MAPSCRS);

  currscr=scrpos[currmap];
  loadlvlpal(getcolor());

  if(currmap!=oldmap)
  {
    can_undo=false;
  }
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
  {
    can_undo=false;
  }
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
  if(prv_mode)
  {
    return prvscr.color;
  }
  return screens[currscr].color;
}

void zmap::resetflags()
{
  byte *di=&(screens[currscr].valid);
  for(int i=1; i<48; i++)
  {
    *(di+i)=0;
  }
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
    // these will be uncommented later
    //screens[scr].layerxsize[i]=16;
    //screens[scr].layerysize[i]=11;
    screens[scr].layeropacity[i]=255;
  }

  screens[scr].valid=mVERSION;
  screens[scr].screen_midi = -1;
  screens[scr].csensitive = 1;
  for(int i=0;i<32;i++)
  {
    screens[scr].ffwidth[i]=15;
	screens[scr].ffheight[i]=15;
  }
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

  if((flags&cFLAGS)&&(sflag||combobuf[cmbdat].flag))
  {
    //    rectfill(dest,x,y,x+15,y+15,vc(cmbdat>>10+1));
    //    text_mode(-1);
    //    textprintf_ex(dest,sfont,x+1,y+1,(sflag)==0x7800?vc(0):vc(15),-1,"%d",sflag);
    if (sflag)
    {
      rectfill(dest,x,y,x+15,y+15,vc(sflag&15));
      textprintf_ex(dest,z3smallfont,x+1,y+1,vc(15-(sflag&15)),-1,"%d",sflag);
    }
    if (combobuf[cmbdat].flag)
      {
		rectfill(dest,x,y+(sflag?8:0),x+15,y+15,vc((combobuf[cmbdat].flag)&15));
        textprintf_ex(dest,z3smallfont,x+1,y+9,vc(15-((combobuf[cmbdat].flag)&15)),-1,"%d",combobuf[cmbdat].flag);
      }
  }
  if(flags&cCSET)
  {
    bool inv = (((cmbdat&0x7800)==0x7800)&&(flags&cFLAGS));
    //    text_mode(inv?vc(15):vc(0));
    textprintf_ex(dest,z3smallfont,x+9,y+9,inv?vc(0):vc(15),inv?vc(15):vc(0),"%d",cset);
  }
  else if(flags&cCTYPE)
    {
      bool inv = (((cmbdat&0x7800)==0x7800)&&(flags&cFLAGS));
      //    text_mode(inv?vc(15):vc(0));
      textprintf_ex(dest,z3smallfont,x+1,y+9,inv?vc(0):vc(15),inv?vc(15):vc(0),"%d",c.type);
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
  if((flags&cFLAGS)&&(sflag||combobuf[cmbdat].flag))
  {
    //    rectfill(dest,x,y,x+15,y+15,vc(cmbdat>>10+1));
    //    text_mode(-1);
    //    textprintf_ex(dest,sfont,x+1,y+1,(sflag)==0x7800?vc(0):vc(15),-1,"%d",sflag);
    if (sflag)
    {
      rectfill(dest,x,y,x+15,y+15,vc(sflag&15));
      textprintf_ex(dest,z3smallfont,x+1,y+1,vc(15-(sflag&15)),-1,"%d",sflag);
    }
    if (combobuf[cmbdat].flag)
      {
		  rectfill(dest,x,y+(sflag?8:0),x+15,y+15,vc((combobuf[cmbdat].flag)&15));
        textprintf_ex(dest,z3smallfont,x+1,y+1,vc(15-((combobuf[cmbdat].flag)&15)),-1,"%d",combobuf[cmbdat].flag);
      }
  }
  if(flags&cCSET)
  {
    bool inv = (((cmbdat&0x7800)==0x7800)&&(flags&cFLAGS));
    //    text_mode(inv?vc(15):vc(0));
    textprintf_ex(dest,z3smallfont,x+9,y+9,inv?vc(0):vc(15),inv?vc(15):vc(0),"%d",cset);
  }
  else if(flags&cCTYPE)
    {
      bool inv = (((cmbdat&0x7800)==0x7800)&&(flags&cFLAGS));
      //    text_mode(inv?vc(15):vc(0));
      textprintf_ex(dest,z3smallfont,x+1,y+9,inv?vc(0):vc(15),inv?vc(15):vc(0),"%d",c.type);
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
  mapscr *doorscreen=(prv_mode?get_prvscr():screens+currscr);
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
      put_combo(dest,x,y,DoorComboSets[doorscreen->door_combo_set].doorcombo_u[type][0],
                DoorComboSets[doorscreen->door_combo_set].doorcset_u[type][0],0,0);
      put_combo(dest,x+16,y,DoorComboSets[doorscreen->door_combo_set].doorcombo_u[type][1],
                DoorComboSets[doorscreen->door_combo_set].doorcset_u[type][1],0,0);
      put_combo(dest,x,y+16,DoorComboSets[doorscreen->door_combo_set].doorcombo_u[type][2],
                DoorComboSets[doorscreen->door_combo_set].doorcset_u[type][2],0,0);
      put_combo(dest,x+16,y+16,DoorComboSets[doorscreen->door_combo_set].doorcombo_u[type][3],
                DoorComboSets[doorscreen->door_combo_set].doorcset_u[type][3],0,0);
      break;
      case down:
      put_combo(dest,x,y,DoorComboSets[doorscreen->door_combo_set].doorcombo_d[type][0],
                DoorComboSets[doorscreen->door_combo_set].doorcset_d[type][0],0,0);
      put_combo(dest,x+16,y,DoorComboSets[doorscreen->door_combo_set].doorcombo_d[type][1],
                DoorComboSets[doorscreen->door_combo_set].doorcset_d[type][1],0,0);
      put_combo(dest,x,y+16,DoorComboSets[doorscreen->door_combo_set].doorcombo_d[type][2],
                DoorComboSets[doorscreen->door_combo_set].doorcset_d[type][2],0,0);
      put_combo(dest,x+16,y+16,DoorComboSets[doorscreen->door_combo_set].doorcombo_d[type][3],
                DoorComboSets[doorscreen->door_combo_set].doorcset_d[type][3],0,0);
      break;
      case left:
      put_combo(dest,x,y,DoorComboSets[doorscreen->door_combo_set].doorcombo_l[type][0],
                DoorComboSets[doorscreen->door_combo_set].doorcset_l[type][0],0,0);
      put_combo(dest,x+16,y,DoorComboSets[doorscreen->door_combo_set].doorcombo_l[type][1],
                DoorComboSets[doorscreen->door_combo_set].doorcset_l[type][1],0,0);
      put_combo(dest,x,y+16,DoorComboSets[doorscreen->door_combo_set].doorcombo_l[type][2],
                DoorComboSets[doorscreen->door_combo_set].doorcset_l[type][2],0,0);
      put_combo(dest,x+16,y+16,DoorComboSets[doorscreen->door_combo_set].doorcombo_l[type][3],
                DoorComboSets[doorscreen->door_combo_set].doorcset_l[type][3],0,0);
      put_combo(dest,x,y+32,DoorComboSets[doorscreen->door_combo_set].doorcombo_l[type][4],
                DoorComboSets[doorscreen->door_combo_set].doorcset_l[type][4],0,0);
      put_combo(dest,x+16,y+32,DoorComboSets[doorscreen->door_combo_set].doorcombo_l[type][5],

                DoorComboSets[doorscreen->door_combo_set].doorcset_l[type][5],0,0);
      break;
      case right:
      put_combo(dest,x,y,DoorComboSets[doorscreen->door_combo_set].doorcombo_r[type][0],
                DoorComboSets[doorscreen->door_combo_set].doorcset_r[type][0],0,0);
      put_combo(dest,x+16,y,DoorComboSets[doorscreen->door_combo_set].doorcombo_r[type][1],
                DoorComboSets[doorscreen->door_combo_set].doorcset_r[type][1],0,0);
      put_combo(dest,x,y+16,DoorComboSets[doorscreen->door_combo_set].doorcombo_r[type][2],
                DoorComboSets[doorscreen->door_combo_set].doorcset_r[type][2],0,0);
      put_combo(dest,x+16,y+16,DoorComboSets[doorscreen->door_combo_set].doorcombo_r[type][3],
                DoorComboSets[doorscreen->door_combo_set].doorcset_r[type][3],0,0);
      put_combo(dest,x,y+32,DoorComboSets[doorscreen->door_combo_set].doorcombo_r[type][4],
                DoorComboSets[doorscreen->door_combo_set].doorcset_r[type][4],0,0);
      put_combo(dest,x+16,y+32,DoorComboSets[doorscreen->door_combo_set].doorcombo_r[type][5],
                DoorComboSets[doorscreen->door_combo_set].doorcset_r[type][5],0,0);
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
  mapscr *doorscreen=(prv_mode?get_prvscr():screens+currscr);

  switch (side)
  {
    case up:
    if (DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_u[0]!=0)
    {
      over_combo(dest,x,y,
                 DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_u[0],
                 DoorComboSets[doorscreen->door_combo_set].bombdoorcset_u[0],0);
    }
    if (DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_u[1]!=0)
    {
      over_combo(dest,x+16,y,
                 DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_u[1],

                 DoorComboSets[doorscreen->door_combo_set].bombdoorcset_u[1],0);
    }
    break;
    case down:
    if (DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_d[0]!=0)
    {
      over_combo(dest,x,y,
                 DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_d[0],
                 DoorComboSets[doorscreen->door_combo_set].bombdoorcset_d[0],0);
    }
    if (DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_d[1]!=0)
    {
      over_combo(dest,x+16,y,
                 DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_d[1],
                 DoorComboSets[doorscreen->door_combo_set].bombdoorcset_d[1],0);
    }
    break;
    case left:
    if (DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_l[0]!=0)
    {
      over_combo(dest,x,y,
                 DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_l[0],
                 DoorComboSets[doorscreen->door_combo_set].bombdoorcset_l[0],0);
    }
    if (DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_l[1]!=0)
    {
      over_combo(dest,x,y+16,
                 DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_l[1],
                 DoorComboSets[doorscreen->door_combo_set].bombdoorcset_l[1],0);
    }
    if (DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_l[2]!=0)
    {
      over_combo(dest,x,y+32,
                 DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_l[2],
                 DoorComboSets[doorscreen->door_combo_set].bombdoorcset_l[2],0);
    }
    break;
    case right:
    if (DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_r[0]!=0)
    {
      over_combo(dest,x,y,
                 DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_r[0],
                 DoorComboSets[doorscreen->door_combo_set].bombdoorcset_r[0],0);
    }
    if (DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_r[1]!=0)
    {
      over_combo(dest,x,y+16,

                 DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_r[1],
                 DoorComboSets[doorscreen->door_combo_set].bombdoorcset_r[1],0);
    }
    if (DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_r[2]!=0)
    {
      over_combo(dest,x,y+32,
                 DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_r[2],
                 DoorComboSets[doorscreen->door_combo_set].bombdoorcset_r[2],0);
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
  mapscr *layer;
  if(prv_mode)
  {
    layer=get_prvscr();
  }
  else
  {
    layer=AbsoluteScr(map,scr);
  }
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
    if (ShowMisalignments)
    {
      check_alignments(dest,x,y,scr);
    }
    return;
  }

  int dark = layer->flags&4;

  resize_mouse_pos=true;

  for(int k=1;k<3;k++)
  {
    if(k==1&&layer->flags7&fLAYER2BG)
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
			  put_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],antiflags|dark,0);
          }
        }
        else
        {
          for (int i=0; i<176; i++)
          {
			  put_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],antiflags|dark,0);
          }
        }
      }
    }
	}
	if(k==2&&layer->flags7&fLAYER3BG)
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
		    if(!(layer->flags7&fLAYER2BG))
				put_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],antiflags|dark,0);
            else over_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],flags);
          }
        }
        else
        {
          for (int i=0; i<176; i++)
          {
		    if(!(layer->flags7&fLAYER2BG))
			  put_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],antiflags|dark,0);
            else over_combo_translucent(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],flags,layer->layeropacity[k]);
          }
        }
      }
    }
	}
  }
  if (LayerMaskInt[0]!=0)
  {
    for(int i=0; i<176; i++)
    {
      word cmbdat = layer->data[i];
      byte cmbcset = layer->cset[i];
      int cmbflag = layer->sflag[i];
	  if(layer->flags7&fLAYER3BG||layer->flags7&fLAYER2BG)
		over_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,cmbdat,cmbcset,flags);
      else put_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,cmbdat,cmbcset,antiflags|dark,cmbflag);
    }
  }
  else
  {
    rectfill(dest,x,y,x+255,y+175,0);
  }

  // int cs=2;

  for (int k=0; k<2; k++)
  {
    if(k==1&&layer->flags7&fLAYER2BG) continue;
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
            over_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],flags);
          }
        }
        else
        {
          for (int i=0; i<176; i++)
          {
            over_combo_translucent(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],flags,layer->layeropacity[k]);
          }
        }
      }
    }
    if(k==0)
    {
      /*for(int i=31;i>=0;i--)
        {
        if(layer->ffdata[i])
        {
          int c=combobuf[layer->ffdata[i]].type;
          if(c!=cCHANGE)
            over_combo(dest,(int)layer->ffx[i]+x,(int)layer->ffy[i]+y,layer->ffdata[i],layer->ffcset[i],flags);
        }
      }*/
      for(int i=31;i>=0;i--)
      {
        if (layer->ffdata[i])
        {
          if(!(layer->ffflags[i]&ffOVERLAY))
          {
            for(int j=0;j<(layer->ffheight[i]>>6)+1;j++)
            {
              for(int k=0;k<(layer->ffwidth[i]>>6)+1;k++)
              {
                if(layer->ffflags[i]&ffTRANS)
                {
                  overcombotranslucent(dest, ((layer->ffx[i]+(k*16))/10000), ((layer->ffy[i]+(j*16))/10000), layer->ffdata[i]+(j*20)+(k), layer->ffcset[i], 128);
                }
                else
                {
                  overcombo(dest, ((layer->ffx[i]+(k*16))/10000), ((layer->ffy[i]+(j*16))/10000), layer->ffdata[i]+(j*20)+(k), layer->ffcset[i]);
                }
              }
            }
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
    if(k==2&&layer->flags7&fLAYER2BG) continue;
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
            over_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],flags);
          }
        }
        else
        {
          for (int i=0; i<176; i++)
          {
            over_combo_translucent(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],flags,layer->layeropacity[k]);
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
            over_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],flags);
          }
        }
        else
        {
          for (int i=0; i<176; i++)
          {
            over_combo_translucent(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],flags,layer->layeropacity[k]);
          }
        }
      }
    }
	if(k==4)
	{
    /*for(int i=31;i>=0;i--)
      {
      if(layer->ffdata[i])
      {
        int c=combobuf[layer->ffdata[i]].type;
        if(c!=cCHANGE)
          over_combo(dest,(int)layer->ffx[i]+x,(int)layer->ffy[i]+y,layer->ffdata[i],layer->ffcset[i],flags);
      }
    }*/
    for(int i=31;i>=0;i--)
    {
      if (layer->ffdata[i])
      {
        if(layer->ffflags[i]&ffOVERLAY)
        {
          for(int j=0;j<(layer->ffheight[i]>>6)+1;j++)
          {
            for(int k=0;k<(layer->ffwidth[i]>>6)+1;k++)
            {
              if(layer->ffflags[i]&ffTRANS)
              {
                overcombotranslucent(dest, ((layer->ffx[i]+(k*16))/10000), ((layer->ffy[i]+(j*16))/10000), layer->ffdata[i]+(j*20)+(k), layer->ffcset[i], 128);
              }
              else
              {
                overcombo(dest, ((layer->ffx[i]+(k*16))/10000), ((layer->ffy[i]+(j*16))/10000), layer->ffdata[i]+(j*20)+(k), layer->ffcset[i]);
              }
            }
          }
        }
      }
    }
	}
	if(k==5)
	{
	  for(int i=31;i>=0;i--)
	  {
	    if(layer->ffdata[i])
	    {
	      int c=combobuf[layer->ffdata[i]].type;
	      if(c==cCHANGE)
	      {
		    putpixel(dest,(int)layer->ffx[i]+x,(int)layer->ffy[i]+y,vc(rand()%16));
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
            put_walkflags(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i]);
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
          if(prv_mode)
          {
            put_flags(dest,((i&15)<<4)+x,(i&0xF0)+y,prvlayers[CurrentLayer-1].data[i],prvlayers[CurrentLayer-1].cset[i],flags|dark,prvlayers[CurrentLayer-1].sflag[i]);
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

  /*
    extern byte isometric_tile_grid[11+(rtgxo*2)][16+(rtgyo*2)];
    for(int i=0; i<176; i++)
    {
    textprintf_shadowed_ex(dest,sfont,((i&15)<<4)+x,(i&0xF0)+y,vc(15),vc(0),-1,"%d",isometric_tile_grid[((i>>4)+rtgyo)][(i&15)+rtgxo]);
    textprintf_shadowed_ex(dest,sfont,((i&15)<<4)+x,(i&0xF0)+y+8,vc(15),vc(0),-1,"%d",(((i>>4)+rtgyo)*(16+(rtgxo*2)))+(i&15)+rtgxo);
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
    int oldcolor=getcolor();
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
    int newcolor=getcolor();
    loadlvlpal(newcolor);
    if (!(screens[currscr].valid&mVALID))
    {
      newcolor=-1;
    }
    if (newcolor!=oldcolor)
    {
      rebuild_trans_table();
    }
    saved=false;
  }
}

void zmap::PasteUnderCombo()
{
  if(can_paste)
  {
    Ugo();
    screens[currscr].undercombo = copymapscr.undercombo;
	screens[currscr].undercset = copymapscr.undercset;
    saved=false;
  }
}

void zmap::PasteSecretCombos()
{
  if(can_paste)
  {
    Ugo();
	for(int i=0;i<128;i++)
	{
      screens[currscr].secretcombo[i] = copymapscr.secretcombo[i];
	  screens[currscr].secretcset[i] = copymapscr.secretcset[i];
	  screens[currscr].secretflag[i] = copymapscr.secretflag[i];
	}
    saved=false;
  }
}

void zmap::PasteFFCombos()
{
  if(can_paste)
  {
    Ugo();
	screens[currscr].numff = copymapscr.numff;
	for(int i=0;i<128;i++)
	{
      screens[currscr].ffdata[i] = copymapscr.ffdata[i];
	  screens[currscr].ffcset[i] = copymapscr.ffcset[i];
	  screens[currscr].ffx[i] = copymapscr.ffx[i];
	  screens[currscr].ffy[i] = copymapscr.ffy[i];
	  screens[currscr].ffxdelta[i] = copymapscr.ffxdelta[i];
	  screens[currscr].ffydelta[i] = copymapscr.ffydelta[i];
	  screens[currscr].ffxdelta2[i] = copymapscr.ffxdelta2[i];
	  screens[currscr].ffydelta2[i] = copymapscr.ffydelta2[i];
	  screens[currscr].fflink[i] = copymapscr.fflink[i];
	  screens[currscr].ffdelay[i] = copymapscr.ffdelay[i];
	  screens[currscr].ffwidth[i] = copymapscr.ffwidth[i];
	  screens[currscr].ffheight[i] = copymapscr.ffheight[i];
	  screens[currscr].ffflags[i] = copymapscr.ffflags[i];
	}
    saved=false;
  }
}

void zmap::PasteWarps()
{
  if(can_paste)
  {
    Ugo();
	screens[currscr].sidewarpindex = copymapscr.sidewarpindex;
	for(int i=0;i<4;i++)
	{
      screens[currscr].tilewarptype[i] = copymapscr.tilewarptype[i];
	  screens[currscr].tilewarpdmap[i] = copymapscr.tilewarpdmap[i];
	  screens[currscr].tilewarpscr[i] = copymapscr.tilewarpscr[i];
	  screens[currscr].sidewarptype[i] = copymapscr.sidewarptype[i];
	  screens[currscr].sidewarpdmap[i] = copymapscr.sidewarpdmap[i];
	  screens[currscr].sidewarpscr[i] = copymapscr.sidewarpscr[i];
	}
    saved=false;
  }
}

void zmap::PasteScreenData()
{
  if(can_paste)
  {
    Ugo();
    screens[currscr].csensitive = copymapscr.csensitive;
	screens[currscr].flags = copymapscr.flags;
	screens[currscr].flags2 = copymapscr.flags2;
	screens[currscr].flags3 = copymapscr.flags3;
	screens[currscr].flags4 = copymapscr.flags4;
	screens[currscr].flags5 = copymapscr.flags5;
	screens[currscr].flags6 = copymapscr.flags6;
	screens[currscr].flags7 = copymapscr.flags7;
	screens[currscr].flags8 = copymapscr.flags8;
	screens[currscr].flags9 = copymapscr.flags9;
	screens[currscr].flags10 = copymapscr.flags10;
	screens[currscr].item = copymapscr.item;
	screens[currscr].itemx = copymapscr.itemx;
	screens[currscr].itemy = copymapscr.itemy;
	screens[currscr].nextmap = copymapscr.nextmap;
	screens[currscr].nextscr = copymapscr.nextscr;
	screens[currscr].nocarry = copymapscr.nocarry;
	screens[currscr].noreset = copymapscr.noreset;
	screens[currscr].path[0] = copymapscr.path[0];
	screens[currscr].path[1] = copymapscr.path[1];
	screens[currscr].path[2] = copymapscr.path[2];
	screens[currscr].path[3] = copymapscr.path[3];
	screens[currscr].pattern = copymapscr.pattern;
	screens[currscr].exitdir = copymapscr.exitdir;
	screens[currscr].enemyflags = copymapscr.enemyflags;
	screens[currscr].screen_midi = copymapscr.screen_midi;
	screens[currscr].stairx = copymapscr.stairx;
	screens[currscr].stairy = copymapscr.stairy;
	screens[currscr].timedwarptics = copymapscr.timedwarptics;
    saved=false;
  }
}

void zmap::PasteWarpLocations()
{
  if(can_paste)
  {
    Ugo();
	screens[currscr].warpreturnc = copymapscr.warpreturnc;
	screens[currscr].warparrivalx = copymapscr.warparrivalx;
	screens[currscr].warparrivaly = copymapscr.warparrivaly;
	for(int i=0;i<4;i++)
	{
	  screens[currscr].warpreturnx[i] = copymapscr.warpreturnx[i];
	  screens[currscr].warpreturny[i] = copymapscr.warpreturny[i];
	}
    saved=false;
  }
}

void zmap::PasteDoors()
{
  if(can_paste)
  {
    Ugo();
	for(int i=0;i<4;i++)
      screens[currscr].door[i] = copymapscr.door[i];
	screens[currscr].door_combo_set = copymapscr.door_combo_set;
    saved=false;
  }
}

void zmap::PasteLayers()
{
  if(can_paste)
  {
    Ugo();
	for(int i=0;i<6;i++)
	{
      screens[currscr].layermap[i] = copymapscr.layermap[i];
	  screens[currscr].layerscreen[i] = copymapscr.layerscreen[i];
	  screens[currscr].layeropacity[i] = copymapscr.layeropacity[i];
	}
    saved=false;
  }
}

void zmap::PasteRoom()
{
  if(can_paste)
  {
    Ugo();
    screens[currscr].room = copymapscr.room;
	screens[currscr].catchall = copymapscr.catchall;
    saved=false;
  }
}

void zmap::PasteGuy()
{
  if(can_paste)
  {
    Ugo();
    screens[currscr].guy = copymapscr.guy;
	screens[currscr].str = copymapscr.str;
    saved=false;
  }
}

void zmap::PasteAll()
{
  if(can_paste)
  {
    Ugo();
    int oldcolor=getcolor();
    screens[currscr]=copymapscr;
    int newcolor=getcolor();
    loadlvlpal(newcolor);
    if (!(screens[currscr].valid&mVALID))
    {
      newcolor=-1;
    }
    if (newcolor!=oldcolor)
    {
      rebuild_trans_table();
    }
    saved=false;
  }
}


void zmap::PasteToAll()
{
  if(can_paste)
  {
    Ugo();
    int oldcolor=getcolor();
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
    int newcolor=getcolor();
    loadlvlpal(newcolor);
    if (!(screens[currscr].valid&mVALID))
    {
      newcolor=-1;
    }
    if (newcolor!=oldcolor)
    {
      rebuild_trans_table();
    }
    saved=false;
  }
}

void zmap::PasteAllToAll()
{
  if(can_paste)
  {
    Ugo();
    int oldcolor=getcolor();
    for (int x=0; x<128; x++)
    {
      screens[x]=copymapscr;
    }
    int newcolor=getcolor();
    loadlvlpal(newcolor);
    if (!(screens[currscr].valid&mVALID))
    {
      newcolor=-1;
    }
    if (newcolor!=oldcolor)
    {
      rebuild_trans_table();
    }
    saved=false;
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
  if(!prv_mode||!prv_cmbcycle)
  {
    return;
  }
  int x,y;
  for (int i=0; i<176; i++)
  {
    x=prvscr.data[i];
    y=animated_combo_table[x][0];
    //time to restart
    if ((animated_combo_table4[y][1]>=combobuf[x].speed) &&
        (combobuf[x].tile-combobuf[x].frames>=animated_combo_table[x][1]-1) &&
        (combobuf[x].nextcombo!=0))
    {
      prvscr.data[i]=combobuf[x].nextcombo;
      prvscr.cset[i]=combobuf[x].nextcset;
    }
  }

  for(int i=0;i<32;i++)
  {
	  x=prvscr.ffdata[i];
	  y=animated_combo_table[x][0];
	  //time to restart
	  if ((animated_combo_table4[y][1]>=combobuf[x].speed) &&
        (combobuf[x].tile-combobuf[x].frames>=animated_combo_table[x][1]-1) &&
        (combobuf[x].nextcombo!=0))
      {
        prvscr.ffdata[i]=combobuf[x].nextcombo;
        prvscr.ffcset[i]=combobuf[x].nextcset;
      }
  }

  if(get_bit(quest_rules,qr_CMBCYCLELAYERS))
  {
    for (int j=0; j<6; j++)
    {
      for (int i=0; i<176; i++)
      {
        x=(prvlayers[j]).data[i];
        y=animated_combo_table[x][0];
        //time to restart
        if ((animated_combo_table4[y][1]>=combobuf[x].speed) &&
            (combobuf[x].tile-combobuf[x].frames>=animated_combo_table[x][1]-1) &&
            (combobuf[x].nextcombo!=0))
        {
          (prvlayers[j]).data[i]=combobuf[x].nextcombo;
          (prvlayers[j]).cset[i]=combobuf[x].nextcset;
        }
      }
    }
  }
  return;
}

void zmap::update_freeform_combos()
{
  if(!prv_mode||!prv_cmbcycle)
  {
    return;
  }
  for(int i=0;i<32;i++)
  {
	if(combobuf[prvscr.ffdata[i]].type!=cCHANGE && prvscr.ffdata[i]!=0 && !(prvscr.ffflags[i]&ffSTATIONARY))
	{

	  if(prvscr.fflink[i] ? !prvscr.ffdelay[prvscr.fflink[i]] : !prvscr.ffdelay[i])
	  {
	    if(prvscr.fflink[i]&&(prvscr.fflink[i]-1)!=i)
	    {
        ffprvx[i] = prvscr.ffx[i];
        ffprvy[i] = prvscr.ffy[i];
	      prvscr.ffx[i]+=prvscr.ffxdelta[prvscr.fflink[i]-1];
	      prvscr.ffy[i]+=prvscr.ffydelta[prvscr.fflink[i]-1];
	    }
	    else
	    {
        ffprvx[i] = prvscr.ffx[i];
        ffprvy[i] = prvscr.ffy[i];
	      prvscr.ffx[i]+=prvscr.ffxdelta[i];
	      prvscr.ffy[i]+=prvscr.ffydelta[i];
	      prvscr.ffxdelta[i]+=prvscr.ffxdelta2[i];
	      prvscr.ffydelta[i]+=prvscr.ffydelta2[i];
        if(prvscr.ffxdelta[i]>1280000) prvscr.ffxdelta[i]=1280000;
        if(prvscr.ffxdelta[i]<-1280000) prvscr.ffxdelta[i]=-1280000;
        if(prvscr.ffydelta[i]>1280000) prvscr.ffydelta[i]=1280000;
        if(prvscr.ffydelta[i]<-1280000) prvscr.ffydelta[i]=-1280000;
	    }
	  }
	  else
	  {
	    if(!prvscr.fflink[i] || (prvscr.fflink[i]-1)==i)
	      prvscr.ffdelay[i]--;
	  }
	  if(prvscr.ffx[i]<-320000)
	  {
	    if(prvscr.flags6&fWRAPAROUNDFF)
      {
        prvscr.ffx[i] = (2880000-(prvscr.ffx[i]+320000));
        ffprvx[i] = prvscr.ffx[i];
      }
      else
      {
        prvscr.ffdata[i]=0;
        prvscr.ffflags[i]&=~ffCARRYOVER;
      }
	  }
	  if(prvscr.ffy[i]<-320000)
	  {
	    if(prvscr.flags6&fWRAPAROUNDFF)
		{
			prvscr.ffy[i] = 2080000-(prvscr.ffy[i]+320000);
			ffprvx[i] = prvscr.ffx[i];
		}
		else
		{
			prvscr.ffdata[i]=0;
			prvscr.ffflags[i]&=~ffCARRYOVER;
		}
	  }
	  if(prvscr.ffx[i]>=2880000)
	  {
	    if(prvscr.flags6&fWRAPAROUNDFF)
      {
        prvscr.ffx[i] = prvscr.ffx[i]-2880000-320000;
        ffprvy[i] = prvscr.ffy[i];
      }
      else
      {
        prvscr.ffdata[i]=0;
        prvscr.ffflags[i]&=~ffCARRYOVER;
      }
	  }
	  if(prvscr.ffy[i]>=2080000)
	  {
	    if(prvscr.flags6&fWRAPAROUNDFF)
      {
        prvscr.ffy[i] = prvscr.ffy[i]-2080000-320000;
        ffprvy[i] = prvscr.ffy[i];
      }
      else
      {
        prvscr.ffdata[i]=0;
        prvscr.ffflags[i]&=~ffCARRYOVER;
      }
	  }

	  for(int j=0;j<32;j++)
	  {
	    if(i!=j)
		{
		  if(combobuf[prvscr.ffdata[j]].type==cCHANGE)
		  {
		    if((((int)prvscr.ffx[j]!=ffposx[i])||((int)prvscr.ffy[j]!=ffposy[i]))&&(prvscr.fflink[i]==0))
			{
			  if((isonline(prvscr.ffx[i],prvscr.ffy[i],ffprvx[i],ffprvy[i],prvscr.ffx[j],prvscr.ffy[j])||
				  (FLOAT_EQ(prvscr.ffx[i],prvscr.ffx[j])&&FLOAT_EQ(prvscr.ffy[i],prvscr.ffy[j])))&&(ffprvx[i]>-1000&&ffprvy[i]>-1000))
			  {
			    //prvscr.ffdata[i]=prvscr.ffdata[j];
			    //prvscr.ffcset[i]=prvscr.ffcset[j];
				if(prvscr.ffflags[j]&ffCHANGETHIS)
				{
				  prvscr.ffdata[i] = prvscr.ffdata[j];
				  prvscr.ffcset[i] = prvscr.ffcset[j];
				}
				if(prvscr.ffflags[j]&ffCHANGENEXT)
				  prvscr.ffdata[i]++;
				if(prvscr.ffflags[j]&ffCHANGEPREV)
				  prvscr.ffdata[i]--;
			    prvscr.ffdelay[i]=prvscr.ffdelay[j];
			    prvscr.ffx[i]=prvscr.ffx[j];
			    prvscr.ffy[i]=prvscr.ffy[j];
			    prvscr.ffxdelta[i]=prvscr.ffxdelta[j];
			    prvscr.ffydelta[i]=prvscr.ffydelta[j];
			    prvscr.ffxdelta2[i]=prvscr.ffxdelta2[j];
			    prvscr.ffydelta2[i]=prvscr.ffydelta2[j];
			    prvscr.fflink[i]=prvscr.fflink[j];
				prvscr.ffwidth[i]=prvscr.ffwidth[j];
				prvscr.ffheight[i]=prvscr.ffheight[j];
				if(prvscr.ffflags[i]&ffCARRYOVER)
				  prvscr.ffflags[i]=prvscr.ffflags[j]&ffCARRYOVER;
				else prvscr.ffflags[i]=prvscr.ffflags[j];
			    ffposx[i]=(short)prvscr.ffx[j];
			    ffposy[i]=(short)prvscr.ffy[j];

			    if(combobuf[prvscr.ffdata[j]].flag>15 && combobuf[prvscr.ffdata[j]].flag<32)
			    {
			      prvscr.ffdata[j]=prvscr.secretcombo[combobuf[prvscr.ffdata[j]].flag-16+4];
			    }
				if((prvscr.ffflags[j]&ffSWAPNEXT)||(prvscr.ffflags[j]&ffSWAPPREV))
				{
				  int k=0;
				  if(prvscr.ffflags[j]&ffSWAPNEXT)
				    k=j<31?j+1:0;
				  if(prvscr.ffflags[j]&ffSWAPPREV)
					k=j>0?j-1:31;
				  //swap(prvscr.ffx[j],prvscr.ffx[k]);
				  //swap(prvscr.ffy[j],prvscr.ffy[k]);
				  swap(prvscr.ffxdelta[j],prvscr.ffxdelta[k]);
				  swap(prvscr.ffydelta[j],prvscr.ffydelta[k]);
				  swap(prvscr.ffxdelta2[j],prvscr.ffxdelta2[k]);
				  swap(prvscr.ffydelta2[j],prvscr.ffydelta2[k]);
				  swap(prvscr.fflink[j],prvscr.fflink[k]);
				  swap(prvscr.ffwidth[j],prvscr.ffwidth[k]);
				  swap(prvscr.ffheight[j],prvscr.ffheight[k]);
				  swap(prvscr.ffflags[j],prvscr.ffflags[k]);
				}
			  }
			}
		  }
		}
	  }

	}
  }
}

void zmap::dowarp(int type, int index)
{
  if(type==0)
  {

    int dmap=screens[currscr].tilewarpdmap[index];
    int scr=screens[currscr].tilewarpscr[index];
    switch(screens[currscr].tilewarptype[index])
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
      int dmap=screens[currscr].sidewarpdmap[index];
      int scr=screens[currscr].sidewarpscr[index];
      switch(screens[currscr].sidewarptype[index])
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

extern int prv_twon;

void zmap::prv_dowarp(int type, int index)
{
  if(type==0)
  {

    int dmap=prvscr.tilewarpdmap[index];
    int scr=prvscr.tilewarpscr[index];
    switch(prvscr.tilewarptype[index])
    {
      case wtCAVE:
      case wtNOWARP:
      break;
      default:
      //setCurrMap(DMaps[dmap].map);
      //setCurrScr(scr+DMaps[dmap].xoff);
      set_prvscr(DMaps[dmap].map,scr+DMaps[dmap].xoff);
      loadlvlpal(getcolor());
      rebuild_trans_table();
      //prv_cmbcycle=0;
      break;
    }
  }
  else if(type==1)
    {
      int dmap=prvscr.sidewarpdmap[index];
      int scr=prvscr.sidewarpscr[index];
      switch(prvscr.sidewarptype[index])
      {
        case wtCAVE:
        case wtNOWARP:
        break;
        default:
        //setCurrMap(DMaps[dmap].map);
        //setCurrScr(scr+DMaps[dmap].xoff);
        set_prvscr(DMaps[dmap].map,scr+DMaps[dmap].xoff);
        loadlvlpal(getcolor());
        rebuild_trans_table();
        //prv_cmbcycle=0;
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
  if(prv_twon)
  {
    prv_time=get_prvscr()->timedwarptics;
  }
}

void zmap::dowarp2(int ring,int index)
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
  char *t=MsgStrings[index].s;
  while(*t==' ')
    ++t;
  s=strcat(s,t);
  return s;
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
  memset(MsgStrings[index].expansion, 0, 32);
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
    if (readtiles(f, NULL, ZELDA_VERSION, VERSION_BUILD, starttile, NEWMAXTILES-starttile, false, true)==0)
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

bool load_zgp(char *path)
{
//  jwin_alert("Error","This feature not yet implemented.",NULL,NULL,"O&K",NULL,'k',0,lfont);
//  return false;
  //open the file
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
    if (readtiles(f, NULL, ZELDA_VERSION, VERSION_BUILD, 0, NEWMAXTILES, false, true)!=0)
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
  
  //read the game icons info and make sure it worked
  if(!p_mgetl(&section_id,f,true))
  {
    pack_fclose(f);
    return false;
  }
  if(section_id==ID_ICONS)
  {
    if (readgameicons(f, &header, &misc, true)!=0)
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

  //read the misc colors info and map styles info and make sure it worked
  if(!p_mgetl(&section_id,f,true))
  {
    pack_fclose(f);
    return false;
  }
  if(section_id==ID_COLORS)
  {
    if (readmisccolors(f, &header, &misc, true)!=0)
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
  
  //write the game icons info and make sure it worked
  if (writegameicons(f, &header, &misc)!=0)
  {
    pack_fclose(f);
    return false;
  }

  //write the misc colors info and map styles info and make sure it worked
  if (writemisccolors(f, &header, &misc)!=0)
  {
    pack_fclose(f);
    return false;
  }

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

bool setMapCount2(int c, bool reload)
{
  char tmpbuf[L_tmpnam];
  char *tmpfilename = temp_name(tmpbuf);
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
    else
    {
      int scr=0;
      for(int i=0; i<map_count && i<MAXMAPS2; i++)
      {
        for (int j=0; j<MAPSCRS; j++)
        {
          scr=i*MAPSCRS+j;
          if (TheMaps[scr].nextmap>=map_count)
          {
            TheMaps[scr].nextmap=map_count-1;
          }
        }
      }
      for(int i=0; i<MAXDMAPS; i++)
      {
        if(DMaps[i].map>=map_count)
        {
          DMaps[i].map=map_count-1;
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

extern BITMAP *map;

int init_quest()
{
  char buf[2048];
  load_quest("qst.dat#DAT_NESQST",true);
  sprintf(buf,"ZQuest - Untitled Quest");
  set_window_title(buf);
  zinit.last_map = 0;
  zinit.last_screen = 0;
  if(map != NULL)
  {
	  destroy_bitmap(map);
	  map=NULL;
  }
  return 0;
/* WTF is all of this dead code? -DD
  char tpath[2048];
  strcpy(tpath, header.templatepath);
  memset(&header, 0, sizeof(zquestheader));
  strcpy(header.templatepath, tpath);
  memset(&zcheats, 0, sizeof(ZCHEATS));

  strcpy(header.id_str,QH_IDSTR);
  header.zelda_version = ZELDA_VERSION;
  header.internal = INTERNAL_VERSION;
  header.build=VERSION_BUILD;
  for(int i=0; i<ZQ_MAXDATA; i++)
    header.data_flags[i]=0;

  setMapCount2(2,false);
  Map.clearall(false);
  init_msgstrs();
  msg_count=1;

  init_tiles(false, &header);
  init_combos(false, &header);

  reset_dmaps();
  reset_midis(customMIDIs);

  memset(&misc, 0, sizeof(miscQdata));

  init_colordata(false, &header, &misc);
  reset_items(false, &header);
  reset_wpns(false, &header);
  reset_doorcombosets(false, &header);

  reset_mapstyles(false, &header, &misc);

  refresh_pal();

  memset(&zinit, 0, sizeof(zinitdata));
  zinit.shield=i_smallshield;
  zinit.hc=3;
  zinit.start_heart=3;
  zinit.cont_heart=3;
  zinit.max_bombs=8;

  zinit.sword_hearts[0]=0;
  zinit.sword_hearts[1]=5;
  zinit.sword_hearts[2]=12;
  zinit.sword_hearts[3]=21;
  zinit.beam_hearts[0]=100;
  zinit.beam_hearts[1]=100;
  zinit.beam_hearts[2]=100;
  zinit.beam_hearts[3]=100;
  for(int i=0; i<idBP_MAX; i++)
  {
    set_bit(&(zinit.beam_percent),i,1);
  }
  zinit.beam_power[0]=100;
  zinit.beam_power[1]=100;
  zinit.beam_power[2]=100;
  zinit.beam_power[3]=100;

  zinit.hookshot_length=6;
  zinit.hookshot_links=3;
  zinit.longshot_length=20;
  zinit.longshot_links=6;
  zinit.moving_fairy_hearts=3;
  zinit.moving_fairy_heart_percent=0;
  zinit.stationary_fairy_hearts=3;
  zinit.stationary_fairy_heart_percent=0;
  zinit.moving_fairy_magic=0;
  zinit.moving_fairy_magic_percent=0;
  zinit.stationary_fairy_magic=0;
  zinit.stationary_fairy_magic_percent=0;
  zinit.blue_potion_hearts=100;
  zinit.blue_potion_heart_percent=1;
  zinit.red_potion_hearts=100;
  zinit.red_potion_heart_percent=1;
  zinit.blue_potion_magic=100;
  zinit.blue_potion_magic_percent=1;
  zinit.red_potion_magic=100;
  zinit.red_potion_magic_percent=1;
  

  saved=true;
  sprintf(buf,"ZQuest - Untitled Quest");
  set_window_title(buf);
  return 0;*/
}

void set_questpwd(char *pwd, bool use_keyfile)
{
  memset(header.password,0,256);
  strcpy(header.password,pwd);
  header.dirty_password=true;
  md5_context ctx;
  md5_starts( &ctx );
  md5_update( &ctx, (uint8 *) pwd, (unsigned long)strlen( pwd ) );
  md5_finish( &ctx, header.pwd_hash );
}


bool is_null_pwd_hash(unsigned char *pwd_hash)
{
  md5_context ctx;
  unsigned char md5sum[16];
  char pwd[2]="";
  md5_starts( &ctx );
  md5_update( &ctx, (uint8 *) pwd, (unsigned long)strlen( pwd ) );
  md5_finish( &ctx, md5sum );

  return (memcmp(md5sum,pwd_hash,16)==0);
}


static DIALOG pwd_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,        0,   0,  224+22+1,   88+10+1,   vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Requires Authorization" },
  { jwin_text_proc,       16,   28,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "File name:" },
  // 2 (filename)
  { jwin_text_proc,       72,  28, 128,   8,    vc(11),  vc(1),  0,       0,          24,            0,       NULL },
  { jwin_text_proc,       16, 38, 0, 8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Challenge:" },
  // 4 (challenge hash)
  { jwin_text_proc,       72, 38, 0, 8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_text_proc,       16,  42+10,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Password:" },
  // 6 (password)
  { jwin_edit_proc,       72,  38+10,  120+39,   16,    vc(12),  vc(1),  0,       0,          255,            0,       NULL },
  { jwin_button_proc,     42,   62+10,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     122,  62+10,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};

int reverse_string(char* str)
{

  if (NULL==str)
  {
    return -1; //no string
  }
  int l=(int)strlen(str)-1; //get the string length
  if (1==l)
  {
    return 1;
  }

  for(int x=0;x < l;x++,l--)
  {
    str[x]^=str[l];  //triple XOR Trick
    str[l]^=str[x];  //for not using a temp
    str[x]^=str[l];
  }
  return 0;
}



bool quest_access(char *filename, bool compressed)
{
  char hash_string[33];
  if (!compressed)
  {
    return true;
  }
  if((get_debug() && (!(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])))|| is_null_pwd_hash(header.pwd_hash))
    return true;

  char pwd[256];
  char prompt[256]="";

  char keyfilename[256];
  replace_extension(keyfilename, filename, "key", 255);
  bool gotfromkey=false;
  if (exists(keyfilename))
  {
    char password[256];
    PACKFILE *fp = pack_fopen(keyfilename, F_READ);
    char msg[80];
    memset(msg,0,80);
    pfread(msg, 80, fp,true);
    if (strcmp(msg,"ZQuest Auto-Generated Quest Password Key File.  DO NOT EDIT!")==0)
    {
      short ver;
      byte  bld;
      short pwd_len;
      p_igetw(&ver,fp,true);
      p_getc(&bld,fp,true);
      memset(password,0,256);
      if ((ver > 0x211)||((ver == 0x211)&&(bld>1)))
      {
        pwd_len=256;
      }
      else
      {
        pwd_len=30;
      }
      pfread(password, pwd_len, fp,true);
      gotfromkey=check_questpwd(&header, password);
      memset(password,0,256);
      memset(pwd,0,256);
    }
    pack_fclose(fp);
  }
  if (gotfromkey)
  {
    return true;
  }
  pwd_dlg[0].dp2=lfont;
  pwd_dlg[2].dp=get_filename(filename);
  md5_context ctx;
  unsigned char md5sum[16];
  char response[33];
  memcpy(md5sum, header.pwd_hash, 16);
  for(int i=0; i<200; ++i)
  {
    for(int j=0; j<16; ++j)
    {
      sprintf(response+j*2, "%02x", md5sum[j]);
    }
    reverse_string(response);
    if (i==99)
    {
      strcpy(hash_string, response);
    }
    md5_starts(&ctx );
    md5_update(&ctx, (uint8 *)response, (unsigned long)strlen(response));
    md5_finish(&ctx, md5sum );
  }
  pwd_dlg[4].dp=hash_string;
  if (get_debug() && (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]))
  {
    //sprintf(prompt,"%s",response);
  }
  pwd_dlg[6].dp=prompt;
  zc_popup_dialog(pwd_dlg,6);
  //  get-questpwd(&header, pwd);
  //  bool ret = strcmp(pwd,prompt)==0;
  bool ret=check_questpwd(&header, prompt);
  if (!ret)
  {
    ret=(strcmp(response,prompt)==0);
  }
  memset(pwd,0,256);
  return ret;

}

// wrapper to reinitialize everything on an error
int load_quest(char *filename, bool compressed)
{
  char buf[2048];
  if(compressed) packfile_password(datapwd);
  int ret=loadquest(filename,&header,&misc,customMIDIs,true,compressed);
  packfile_password(NULL);

  if(ret!=qe_OK)
  {
    init_quest();
  }
  else if(!quest_access(filename, compressed))
    {
      init_quest();
      ret=qe_pwd;
    }
    else
    {
      Map.setCurrMap(vbound(Map.getCurrMap(),0,map_count-1));
      refresh(rALL);
      refresh_pal();
	  if(map != NULL)
	  {
		destroy_bitmap(map);
		map=NULL;
	  }
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

int writeheader(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_HEADER;
  dword section_version=V_HEADER;
  dword section_cversion=CV_HEADER;
  dword section_size=22+18+131+20+2048+1;                    //+29;

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
  /*
    if(!pfwrite(header->password,sizeof(header->password),f))
    {
    return 8;
    }
    if(!p_iputw(header->pwdkey,f))
    {
    return 9;
    }
    */
  if(!pfwrite(header->pwd_hash,sizeof(header->pwd_hash),f))
  {
    return 8;
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
  if(!p_putc(0,f)) //why are we doing this?
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
    if(!p_putc(DMaps[i].tmusictrack,f))
    {
      return 25;
    }
    if(!p_putc(DMaps[i].active_subscreen,f))
    {
      return 26;
    }
    if(!p_putc(DMaps[i].passive_subscreen,f))
    {
      return 27;
    }
  }
  return 0;
}

int writemisccolors(PACKFILE *f, zquestheader *header, miscQdata *misc)
{
  dword section_id=ID_COLORS;
  dword section_version=V_COLORS;
  dword section_cversion=CV_COLORS;
  dword section_size = 22 * sizeof(char) + 6 * sizeof(word);
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


  writesize=0;

  if(!p_putc(misc->colors.text,f))
  {
    return 5;
  }
  if(!p_putc(misc->colors.caption,f))
  {
    return 6;
  }
  if(!p_putc(misc->colors.overw_bg,f))
  {
    return 7;
  }
  if(!p_putc(misc->colors.dngn_bg,f))
  {
    return 8;
  }
  if(!p_putc(misc->colors.dngn_fg,f))
  {
    return 9;
  }
  if(!p_putc(misc->colors.cave_fg,f))
  {
    return 10;
  }
  if(!p_putc(misc->colors.bs_dk,f))
  {
    return 11;
  }
  if(!p_putc(misc->colors.bs_goal,f))
  {
    return 12;
  }
  if(!p_putc(misc->colors.compass_lt,f))
  {
    return 13;
  }
  if(!p_putc(misc->colors.compass_dk,f))
  {
    return 14;
  }
  if(!p_putc(misc->colors.subscr_bg,f))
  {
    return 15;
  }
  if(!p_putc(misc->colors.triframe_color,f))
  {
    return 16;
  }
  if(!p_putc(misc->colors.link_dot,f))
  {
    return 17;
  }
  if(!p_putc(misc->colors.bmap_bg,f))
  {
    return 18;
  }
  if(!p_putc(misc->colors.bmap_fg,f))
  {
    return 19;
  }
  if(!p_putc(misc->colors.triforce_cset,f))
  {
    return 20;
  }
  if(!p_putc(misc->colors.triframe_cset,f))
  {
    return 21;
  }
  if(!p_putc(misc->colors.overworld_map_cset,f))
  {
    return 22;
  }
  if(!p_putc(misc->colors.dungeon_map_cset,f))
  {
    return 23;
  }
  if(!p_putc(misc->colors.blueframe_cset,f))
  {
    return 24;
  }
  if(!p_iputw(misc->colors.triforce_tile,f))
  {
    return 25;
  }
  if(!p_iputw(misc->colors.triframe_tile,f))
  {
    return 26;
  }
  if(!p_iputw(misc->colors.overworld_map_tile,f))
  {
    return 27;
  }
  if(!p_iputw(misc->colors.dungeon_map_tile,f))
  {
    return 28;
  }
  if(!p_iputw(misc->colors.blueframe_tile,f))
  {
    return 29;
  }
  if(!p_iputw(misc->colors.HCpieces_tile,f))
  {
    return 30;
  }
  if(!p_putc(misc->colors.HCpieces_cset,f))
  {
    return 31;
  }
  if(!p_putc(misc->colors.subscr_shadow,f))
  {
    return 32;
  }

  return 0;
}

int writegameicons(PACKFILE *f, zquestheader *header, miscQdata *misc)
{
  dword section_id=ID_ICONS;
  dword section_version=V_ICONS;
  dword section_cversion=CV_ICONS;
  dword section_size = 4 * sizeof(word);
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

  writesize=0;

  for (int i=0; i<4; i++)
  {
    if(!p_iputw(misc->icons[i],f))
    {
      return 5;
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
  //  dword section_size=0;
  dword section_size = sizeof(word) + shops * 3 * (sizeof(char) + sizeof(word))
    + sizeof(word) + infos * 3 * 2 * sizeof(word)
    + sizeof(word) + warprings * (8 * sizeof(char) + 8 * sizeof(char) + sizeof(char))
    + sizeof(word) + windwarps * 2 * sizeof(char)
    + triforces * sizeof(char)
//    + 22 * sizeof(char) + 6 * sizeof(word)
//    + 4 * sizeof(word)
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


  writesize=0;
  //shops
  if(!p_iputw(shops,f))
  {
    return 5;
  }
  for (int i=0; i<shops; i++)
  {
    for (int j=0; j<3; j++)
    {
      if(!p_putc(misc->shop[i].item[j],f))
      {
        return 6;
      }
    }
    for (int j=0; j<3; j++)
    {
      if(!p_iputw(misc->shop[i].price[j],f))
      {
        return 7;
      }
    }
  }

  //infos
  if(!p_iputw(infos,f))
  {
    return 8;
  }
  for (int i=0; i<infos; i++)
  {
    for (int j=0; j<3; j++)
    {
      if(!p_iputw(misc->info[i].str[j],f))
      {
        return 9;
      }
    }
    for (int j=0; j<3; j++)
    {
      if(!p_iputw(misc->info[i].price[j],f))
      {
        return 10;
      }
    }
  }

  //warp rings
  if(!p_iputw(warprings,f))
  {
    return 11;
  }
  for (int i=0; i<warprings; i++)
  {
    for (int j=0; j<8; j++)
    {
      if(!p_putc(misc->warp[i].dmap[j],f))
      {
        return 12;
      }
    }
    for (int j=0; j<8; j++)
    {
      if(!p_putc(misc->warp[i].scr[j],f))
      {
        return 13;
      }
    }
    if(!p_putc(misc->warp[i].size,f))
    {
      return 14;
    }
  }

  //wind warps
  if(!p_iputw(windwarps,f))
  {
    return 15;
  }
  for (int i=0; i<windwarps; i++)
  {
    if(!p_putc(misc->wind[i].dmap,f))
    {
      return 16;
    }
    if(!p_putc(misc->wind[i].scr,f))
    {
      return 17;
    }
  }

  //triforce pieces
  for (int i=0; i<triforces; i++)
  {
    if(!p_putc(misc->triforce[i],f))
    {
      return 18;
    }
  }

/*
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
    if(!p_putc(misc->colors.subscr_shadow,f))
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
*/

  //end string
  if(!p_iputw(misc->endstring,f))
  {
    return 19;
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
  dword section_size = sizeof(word) + map_count * MAPSCRS * 1301;

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
      for(int k=0;k<4;k++)
      {
        if(!p_putc(TheMaps[scr].tilewarptype[k],f))
        {
          return qe_invalid;
        }
      }

      {
        if(!p_iputw(TheMaps[scr].door_combo_set,f))
        {
          return qe_invalid;
        }
      }

      for(int k=0;k<4;k++)
      {
        if(!p_putc(TheMaps[scr].warpreturnx[k],f))
        {
          return qe_invalid;
        }
      }
      for(int k=0;k<4;k++)
      {
        if(!p_putc(TheMaps[scr].warpreturny[k],f))
        {
          return qe_invalid;
        }
      }
      if(!p_putc(TheMaps[scr].warpreturnc,f))
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
      for(int k=0;k<4;k++)
      {
        if(!p_putc(TheMaps[scr].tilewarpdmap[k],f))
        {
          return qe_invalid;
        }
      }
      for(int k=0;k<4;k++)
      {
        if(!p_putc(TheMaps[scr].tilewarpscr[k],f))
        {
          return qe_invalid;
        }
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
      for(int k=0;k<4;k++)
      {
        if(!p_putc(TheMaps[scr].sidewarptype[k],f))
        {
          return qe_invalid;
        }
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
      for(int k=0;k<4;k++)
      {
        if(!p_putc(TheMaps[scr].sidewarpscr[k],f))
        {
          return qe_invalid;
        }
      }
      for(int k=0;k<4;k++)
      {
        if(!p_putc(TheMaps[scr].sidewarpdmap[k],f))
        {
          return qe_invalid;
        }
      }
      if(!p_putc(TheMaps[scr].sidewarpindex,f))
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
      if(!p_putc(TheMaps[scr].flags4,f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].flags5,f))
      {
        return qe_invalid;
      }
      if(!p_iputw(TheMaps[scr].noreset,f))
      {
        return qe_invalid;
      }
      if(!p_iputw(TheMaps[scr].nocarry,f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].flags6,f))
      {
        return qe_invalid;
      }
	  if(!p_putc(TheMaps[scr].flags7,f))
      {
        return qe_invalid;
      }
	  if(!p_putc(TheMaps[scr].flags8,f))
      {
        return qe_invalid;
      }
	  if(!p_putc(TheMaps[scr].flags9,f))
      {
        return qe_invalid;
      }
	  if(!p_putc(TheMaps[scr].flags10,f))
      {
        return qe_invalid;
      }
	  if(!p_putc(TheMaps[scr].csensitive,f))
      {
        return qe_invalid;
      }


      {  //this is enclosed in extra braces to allow quick section folding
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

      {  //this is enclosed in extra braces to allow quick section folding
        for(int k=0; k<6; k++)
        {
          if(!p_putc(TheMaps[scr].layeropacity[k],f))
          {
            return qe_invalid;
          }
        }
      }

      {  //this is enclosed in extra braces to allow quick section folding
        if(!p_iputw(TheMaps[scr].timedwarptics,f))
        {
          return qe_invalid;
        }
      }

      {  //this is enclosed in extra braces to allow quick section folding
        if(!p_putc(TheMaps[scr].nextmap,f))
        {
          return qe_invalid;
        }
        if(!p_putc(TheMaps[scr].nextscr,f))
        {
          return qe_invalid;
        }
      }

      {  //this is enclosed in extra braces to allow quick section folding
        for(int k=0; k<128; k++)
        {
          if(!p_iputw(TheMaps[scr].secretcombo[k],f))
          {
            return qe_invalid;
          }

        }
      }

      {  //this is enclosed in extra braces to allow quick section folding
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

      {  //this is enclosed in extra braces to allow quick section folding
        for(int k=0; k<16*11; k++)
        {
          if(!p_iputw(TheMaps[scr].data[k],f))
          {
            return qe_invalid;
          }
        }
      }

      {  //this is enclosed in extra braces to allow quick section folding
        for(int k=0; k<16*11; k++)
        {
          if(!p_putc(TheMaps[scr].sflag[k],f))
          {
            return qe_invalid;
          }
        }
      }

      {  //this is enclosed in extra braces to allow quick section folding
        for(int k=0; k<16*11; k++)
        {
          if(!p_putc(TheMaps[scr].cset[k],f))
          {
            return qe_invalid;
          }
        }
      }

      if(!p_iputw(TheMaps[scr].screen_midi,f))
      {
        return qe_invalid;
      }

	  //if(TheMaps[scr].numff > 32) TheMaps[scr].numff=32;
	  if(!p_iputl(TheMaps[scr].numff,f))
	  {
	    return qe_invalid;
	  }
	  for(int k=0;k<32;k++)
	  {
	    if((TheMaps[scr].numff>>k)&1)
		{
	    if(!p_iputw(TheMaps[scr].ffdata[k],f))
		{
		  return qe_invalid;
		}
		if(!p_putc(TheMaps[scr].ffcset[k],f))
		{
		  return qe_invalid;
		}
		if(!p_iputw(TheMaps[scr].ffdelay[k],f))
		{
		  return qe_invalid;
		}
		if(!pfwrite(&TheMaps[scr].ffx[k],4,f))
		{
		  return qe_invalid;
		}
		if(!pfwrite(&TheMaps[scr].ffy[k],4,f))
		{
		  return qe_invalid;
		}
		if(!pfwrite(&TheMaps[scr].ffxdelta[k],4,f))
		{
		  return qe_invalid;
		}
		if(!pfwrite(&TheMaps[scr].ffydelta[k],4,f))
		{
		  return qe_invalid;
		}
		if(!pfwrite(&TheMaps[scr].ffxdelta2[k],4,f))
		{
		  return qe_invalid;
		}
		if(!pfwrite(&TheMaps[scr].ffydelta2[k],4,f))
		{
		  return qe_invalid;
		}
		if(!p_putc(TheMaps[scr].fflink[k],f))
		{
		  return qe_invalid;
		}
		if(!p_putc(TheMaps[scr].ffwidth[k],f))
		{
		  return qe_invalid;
		}
		if(!p_putc(TheMaps[scr].ffheight[k],f))
		{
		  return qe_invalid;
		}
		if(!p_iputl(TheMaps[scr].ffflags[k],f))
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
  word combos_used;
  dword section_id=ID_COMBOS;
  dword section_version=V_COMBOS;
  dword section_cversion=CV_COMBOS;
  //  dword section_size=0;
  combos_used = count_combos()-start_combo;
  combos_used = min(combos_used, max_combos);
  combos_used = min(combos_used, MAXCOMBOS);
  dword section_size = sizeof(word) + combos_used * 12;

  for(int j=0; j<MAXCOMBOALIASES; j++)
  {
    section_size += (sizeof(byte)*3) + (sizeof(word)*((combo_aliases[j].width+1)*(combo_aliases[j].height+1)))*comboa_lmasktotal(combo_aliases[j].layermask);
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
    if(!p_putc(combobuf[i].flag,f))
    {
      return 15;
    }
	if(!p_putc(combobuf[i].skipanim,f))
    {
      return 16;
    }
	if(!p_iputw(combobuf[i].nexttimer,f))
    {
      return 17;
    }
  }

  return 0;
}

int writecomboaliases(PACKFILE *f, word version, word build)
{
  dword section_id=ID_COMBOALIASES;
  dword section_version=V_COMBOALIASES;
  dword section_cversion=CV_COMBOALIASES;
  dword section_size=0;

  for(int j=0; j<MAXCOMBOALIASES; j++)
  {
    section_size += (sizeof(byte)*3) + (sizeof(word)*((combo_aliases[j].width+1)*(combo_aliases[j].height+1)))*comboa_lmasktotal(combo_aliases[j].layermask);
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
  for(int j=0;j<MAXCOMBOALIASES;j++)
  {
    if(!p_iputw(combo_aliases[j].combo,f))
    {
      return 5;
    }
    if(!p_putc(combo_aliases[j].cset,f))
    {
      return 6;
    }
    int count = ((combo_aliases[j].width+1)*(combo_aliases[j].height+1))*(comboa_lmasktotal(combo_aliases[j].layermask)+1);
    if(!p_putc(combo_aliases[j].width,f))
    {
      return 7;
    }
    if(!p_putc(combo_aliases[j].height,f))
    {
      return 8;
    }
    if(!p_putc(combo_aliases[j].layermask,f))
    {
      return 9;
    }
    for(int k=0;k<count;k++)
    {
      if(!p_iputw(combo_aliases[j].combos[k],f))
      {
        return 10;
      }
    }
    for(int k=0;k<count;k++)
    {
      if(!p_putc(combo_aliases[j].csets[k],f))
      {
        return 11;
      }
    }
  }
  return 0;
}

int writecolordata(PACKFILE *f, miscQdata *misc, word version, word build, word start_cset, word max_csets)
{
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
  word tiles_used;
  dword section_id=ID_TILES;
  dword section_version=V_TILES;
  dword section_cversion=CV_TILES;
  //  dword section_size=0;
  tiles_used = count_tiles()-start_tile;
  tiles_used = min(tiles_used, max_tiles);
  tiles_used = min(tiles_used, NEWMAXTILES);
  dword section_size = sizeof(word) + (int)tiles_used * (int)tilesize(tf4Bit);

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
  tiles_used=count_tiles()-start_tile;
  tiles_used=min(tiles_used, max_tiles);
  tiles_used=min(tiles_used, NEWMAXTILES);
  if(!p_iputw(tiles_used,f))
  {
    return 5;
  }

  for (dword i=0; i<tiles_used; ++i)
  {
    if(!p_putc(newtilebuf[start_tile+i].format,f))
    {
      return 6;
    }
    if(!pfwrite(newtilebuf[start_tile+i].data,tilesize(newtilebuf[start_tile+i].format),f))
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
    for(int i = 0; i < MAXCUSTOMMIDIS; i++ )
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
  for(int i=0; i<MAXCUSTOMMIDIS; i++)
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

  //More data will come here

  return 0;
}

int writelinksprites(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_LINKSPRITES;
  dword section_version=V_LINKSPRITES;
  dword section_cversion=CV_LINKSPRITES;
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

  for(int i=0;i<4;i++)
  {
    if(!p_iputw((word)walkspr[i][spr_tile],f))
    {
      return 5;
    }
    if(!p_putc((byte)walkspr[i][spr_flip],f))
    {
      return 5;
    }
    if(!p_putc((byte)walkspr[i][spr_extend],f))
    {
      return 5;
    }
  }

  for(int i=0;i<4;i++)
  {
    if(!p_iputw((word)stabspr[i][spr_tile],f))
    {
      return 6;
    }
    if(!p_putc((byte)stabspr[i][spr_flip],f))
    {
      return 6;
    }
    if(!p_putc((byte)stabspr[i][spr_extend],f))
    {
      return 6;
    }
  }

  for(int i=0;i<4;i++)
  {
    if(!p_iputw((word)slashspr[i][spr_tile],f))
    {
      return 7;
    }
    if(!p_putc((byte)slashspr[i][spr_flip],f))
    {
      return 7;
    }
    if(!p_putc((byte)slashspr[i][spr_extend],f))
    {
      return 7;
    }
  }

  for(int i=0;i<4;i++)
  {
    if(!p_iputw((word)floatspr[i][spr_tile],f))
    {
      return 8;
    }
    if(!p_putc((byte)floatspr[i][spr_flip],f))
    {
      return 8;
    }
    if(!p_putc((byte)floatspr[i][spr_extend],f))
    {
      return 8;
    }
  }

  for(int i=0;i<4;i++)
  {
    if(!p_iputw((word)swimspr[i][spr_tile],f))
    {
      return 8;
    }
    if(!p_putc((byte)swimspr[i][spr_flip],f))
    {
      return 8;
    }
    if(!p_putc((byte)swimspr[i][spr_extend],f))
    {
      return 8;
    }
  }

  for(int i=0;i<4;i++)
  {
    if(!p_iputw((word)divespr[i][spr_tile],f))
    {
      return 9;
    }
    if(!p_putc((byte)divespr[i][spr_flip],f))
    {
      return 9;
    }
    if(!p_putc((byte)divespr[i][spr_extend],f))
    {
      return 9;
    }
  }

  for(int i=0;i<4;i++)
  {
    if(!p_iputw((word)poundspr[i][spr_tile],f))
    {
      return 10;
    }
    if(!p_putc((byte)poundspr[i][spr_flip],f))
    {
      return 10;
    }
    if(!p_putc((byte)poundspr[i][spr_extend],f))
    {
      return 10;
    }
  }

  if(!p_iputw((word)castingspr[spr_tile],f))
  {
    return 11;
  }
  if(!p_putc((byte)castingspr[spr_flip],f))
  {
    return 11;
  }
  if(!p_putc((byte)castingspr[spr_extend],f))
  {
    return 11;
  }

  for(int i=0;i<2;i++)
  {
    for(int j=0;j<2;j++)
    {
      if(!p_iputw((word)holdspr[i][j][spr_tile],f))
      {
        return 12;
      }
      if(!p_putc((byte)holdspr[i][j][spr_flip],f))
      {
        return 12;
      }
      if(!p_putc((byte)holdspr[i][j][spr_extend],f))
      {
        return 12;
      }
    }
  }

  //More data will come here

  return 0;
}

int writesubscreens(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_SUBSCREEN;
  dword section_version=V_SUBSCREEN;
  dword section_cversion=CV_SUBSCREEN;
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

  for(int i=0;i<MAXCUSTOMSUBSCREENS;i++)
  {
    int numsub = 0;
    if(!pfwrite(custom_subscreen[i].name, 64,f))
    {
      return 28;
    }
    for(int k=0;(k<MAXSUBSCREENITEMS&&(custom_subscreen[i].objects[k].type != ssoNULL));k++)
    {
      numsub++;
    }
    if(!p_putc(numsub,f))
    {
      return 4;
    }
    for(int j=0;(j<MAXSUBSCREENITEMS&&j<numsub);j++)
    {
      if(!p_putc(custom_subscreen[i].objects[j].type, f))
      {
        return 5;
      }
      if(!p_putc(custom_subscreen[i].objects[j].pos, f))
      {
        return 6;
      }
      if(!p_iputw(custom_subscreen[i].objects[j].x, f))
      {
        return 7;
      }
      if(!p_iputw(custom_subscreen[i].objects[j].y, f))
      {
        return 8;
      }
      if(!p_iputw(custom_subscreen[i].objects[j].w, f))
      {
        return 9;
      }
      if(!p_iputw(custom_subscreen[i].objects[j].h, f))
      {
        return 10;
      }
      if(!p_putc(custom_subscreen[i].objects[j].colortype1, f))
      {
        return 11;
      }
      if(!p_iputw(custom_subscreen[i].objects[j].color1, f))
      {
        return 12;
      }
      if(!p_putc(custom_subscreen[i].objects[j].colortype2, f))
      {
        return 13;
      }
      if(!p_iputw(custom_subscreen[i].objects[j].color2, f))
      {
        return 14;
      }
      if(!p_putc(custom_subscreen[i].objects[j].colortype3, f))
      {
        return 15;
      }
      if(!p_iputw(custom_subscreen[i].objects[j].color3, f))
      {
        return 16;
      }
      if(!p_iputl(custom_subscreen[i].objects[j].d1, f))
      {
        return 17;
      }
      if(!p_iputl(custom_subscreen[i].objects[j].d2, f))
      {
        return 18;
      }
      if(!p_iputl(custom_subscreen[i].objects[j].d3, f))
      {
        return 19;
      }
      if(!p_iputl(custom_subscreen[i].objects[j].d4, f))
      {
        return 20;
      }
      if(!p_iputl(custom_subscreen[i].objects[j].d5, f))
      {
        return 21;
      }
      if(!p_iputl(custom_subscreen[i].objects[j].d6, f))
      {
        return 22;
      }
      if(!p_iputl(custom_subscreen[i].objects[j].d7, f))
      {
        return 23;
      }
      if(!p_iputl(custom_subscreen[i].objects[j].d8, f))
      {
        return 24;
      }
      if(!p_iputl(custom_subscreen[i].objects[j].d9, f))
      {
        return 25;
      }
      if(!p_iputl(custom_subscreen[i].objects[j].d10, f))
      {
        return 26;
      }
	  if(!p_iputl(custom_subscreen[i].objects[j].speed, f))
	  {
	    return 27;
	  }
	    if(!p_iputl(custom_subscreen[i].objects[j].delay, f))
	  {
	    return 28;
	  }
	    if(!p_iputl(custom_subscreen[i].objects[j].frame, f))
	  {
	    return 29;
	  }
	  
      switch(custom_subscreen[i].objects[j].type)
      {
        case ssoTEXT:
        case ssoTEXTBOX:
        case ssoCURRENTITEMTEXT:
        case ssoCURRENTITEMCLASSTEXT:
        if(custom_subscreen[i].objects[j].dp1 != NULL)
        {
          if(strlen((char*)custom_subscreen[i].objects[j].dp1))
          {
            if(!p_iputw(strlen((char*)custom_subscreen[i].objects[j].dp1), f))
            {
              return 27;
            }
            if(!pfwrite(custom_subscreen[i].objects[j].dp1, strlen((char*)custom_subscreen[i].objects[j].dp1)+1,f))
            {
              return 28;
            }
          }
          else
          {
            if(!p_putc(0, f))
            {
              return 27;
            }
          }
        }
        else
        {
          if(!p_putc(0, f))
          {
            return 27;
          }
        }
        break;
        default:
        if(!p_putc(0, f))
        {
          return 27;
        }
      }
    }
  }
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
  if(!p_putc(zinit.hookshot_length,f))
  {
    return 56;
  }
  if(!p_putc(zinit.longshot_links,f))
  {
    return 57;
  }
  if(!p_putc(zinit.longshot_length,f))
  {
    return 58;
  }
  if(!p_putc(zinit.msg_more_x,f))
  {
    return 59;
  }
  if(!p_putc(zinit.msg_more_y,f))
  {
    return 60;
  }
  if(!p_putc(zinit.subscreen,f))
  {
    return 61;
  }
  if(!p_putc(zinit.start_dmap,f))
  {
    return 62;
  }
  if(!p_putc(zinit.linkanimationstyle,f))
  {
    return 63;
  }
  if(!p_putc(zinit.arrows,f))
  {
    return 64;
  }
  if(!p_putc(zinit.max_arrows,f))
  {
    return 65;
  }
  for(int i=0;i<256;i++)
  {
    if(!p_putc(zinit.level_keys[i],f))
  	{
  	  return 66;
  	}
  }
  if(!p_iputw(zinit.ss_grid_x,f))
  {
    return 67;
  }
  if(!p_iputw(zinit.ss_grid_y,f))
  {
    return 68;
  }
  if(!p_iputw(zinit.ss_grid_xofs,f))
  {
    return 68;
  }
  if(!p_iputw(zinit.ss_grid_yofs,f))
  {
    return 70;
  }
  if(!p_iputw(zinit.ss_grid_color,f))
  {
    return 71;
  }
  if(!p_iputw(zinit.ss_bbox_1_color,f))
  {
    return 72;
  }
  if(!p_iputw(zinit.ss_bbox_2_color,f))
  {
    return 73;
  }
  if(!p_iputw(zinit.ss_flags,f))
  {
    return 74;
  }
  if(!p_putc(zinit.moving_fairy_hearts,f))
  {
    return 75;
  }
  if(!p_putc(zinit.moving_fairy_heart_percent,f))
  {
    return 76;
  }
  if(!p_putc(zinit.quiver,f))
  {
	  return 77;
  }
  if(!p_putc(zinit.stationary_fairy_hearts,f))
  {
    return 78;
  }
  if(!p_putc(zinit.stationary_fairy_heart_percent,f))
  {
    return 79;
  }
  if(!p_putc(zinit.moving_fairy_magic,f))
  {
    return 80;
  }
  if(!p_putc(zinit.moving_fairy_magic_percent,f))
  {
    return 81;
  }
  if(!p_putc(zinit.stationary_fairy_magic,f))
  {
    return 82;
  }
  if(!p_putc(zinit.stationary_fairy_magic_percent,f))
  {
    return 83;
  }
  if(!p_putc(zinit.blue_potion_hearts,f))
  {
    return 84;
  }
  if(!p_putc(zinit.blue_potion_heart_percent,f))
  {
    return 85;
  }
  if(!p_putc(zinit.red_potion_hearts,f))
  {
    return 86;
  }
  if(!p_putc(zinit.red_potion_heart_percent,f))
  {
    return 87;
  }
  if(!p_putc(zinit.blue_potion_magic,f))
  {
    return 88;
  }
  if(!p_putc(zinit.blue_potion_magic_percent,f))
  {
    return 89;
  }
  if(!p_putc(zinit.red_potion_magic,f))
  {
    return 90;
  }
  if(!p_putc(zinit.red_potion_magic_percent,f))
  {
    return 91;
  }
  if(!p_putc(zinit.subscreen_style,f))
  {
    return 92;
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
  for(int i=0; i<MAXCUSTOMMIDIS; i++)
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

  box_out("Writing Misc. Colors...");
  if (writemisccolors(f,&header,&misc)!=0)
  {
    return 8;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Game Icons...");
  if (writegameicons(f,&header,&misc)!=0)
  {
    return 9;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Items...");
  if (writeitems(f,&header)!=0)
  {
    return 10;
  }

  box_out("okay.");
  box_eol();

  box_out("Writing Weapons...");
  if (writeweapons(f,&header)!=0)
  {
    return 11;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Maps...");
  if (writemaps(f,&header)!=0)
  {
    return 12;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Combos...");
  if (writecombos(f,header.zelda_version,header.build,0,MAXCOMBOS)!=0)
  {
    return 13;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Combo Aliases...");
  if (writecomboaliases(f,header.zelda_version,header.build)!=0)
  {
    return 14;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Color Data...");
  if (writecolordata(f,&misc,header.zelda_version,header.build,0,newpdTOTAL)!=0)
  {
    return 15;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Tiles...");
  if (writetiles(f,header.zelda_version,header.build,0,NEWMAXTILES)!=0)
  {
    return 16;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing MIDIs...");
  if (writemidis(f,&header)!=0)
  {
    return 17;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Cheat Codes...");
  if (writecheats(f,&header)!=0)
  {
    return 18;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Init. Data...");
  if (writeinitdata(f,&header)!=0)
  {
    return 19;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Custom Guy Data...");
  if (writeguys(f,&header)!=0)
  {
    return 20;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Custom Link Sprite Data...");
  if (writelinksprites(f,&header)!=0)
  {
    return 21;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing Custom Subscreen Data...");
  if (writesubscreens(f,&header)!=0)
  {
    return 22;
  }
  box_out("okay.");
  box_eol();

  pack_fclose(f);

  replace_extension(keyfilename, get_filename(filepath), "key", 255);
  if (header.use_keyfile&&header.dirty_password)
  {
    packfile_password(NULL);
    PACKFILE *fp = pack_fopen(keyfilename, F_WRITE);
    char msg[80];
    memset(msg,0,80);
    sprintf(msg, "ZQuest Auto-Generated Quest Password Key File.  DO NOT EDIT!");
    msg[78]=13;msg[79]=10;
    pfwrite(msg, 80, fp);
    p_iputw(header.zelda_version,fp);
    p_putc(header.build,fp);
    pfwrite(header.password, 256, fp);
    pack_fclose(fp);
    packfile_password(datapwd);
  }
  return 0;
}

int save_quest(char *filename, bool version_210)
{

  char tmpbuf[L_tmpnam];
  char *tmpfilename = temp_name(tmpbuf);

  packfile_password(datapwd);
  int ret;
  if (version_210)
  {
    ret  = save_unencoded_quest_210(tmpfilename, true);
  }
  else
  {
    ret  = save_unencoded_quest(tmpfilename, true);
  }
  


  //  int ret = save_unencoded_quest(filename, false);
  //  return ret;




  packfile_password(NULL);
  if(ret == 0)
  {
    box_out("Encrypting...");
//    ret = encode_file_007(tmpfilename, filename,((INTERNAL_VERSION + rand()) & 0xffff) + 0x413F0000, ENC_STR, ENC_METHOD_MAX-1);
    ret = encode_file_007(tmpfilename, filename,((INTERNAL_VERSION + rand()) & 0xffff) + 0x413F0000, ENC_STR, ENC_METHOD_192B185);
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

void center_zq_class_dialogs()
{
  jwin_center_dialog(pwd_dlg);
}

void zmap::prv_secrets(bool high16only)
{
  mapscr *s = &prvscr;
  mapscr *t = prvlayers;
  int ft=0;
  for(int i=0; i<176; i++)
  {
    bool putit;

    if(!high16only)
    {
      for (int iter=0; iter<2; ++iter)
      {
        putit=true;
        int checkflag=combobuf[s->data[i]].flag;
        if (iter==1)
        {
          checkflag=s->sflag[i];
        }
        switch(checkflag)
        {
          case mfBCANDLE:
          ft=sBCANDLE;
          break;
          case mfRCANDLE:
          ft=sRCANDLE;
          break;
          case mfWANDFIRE:
          ft=sWANDFIRE;
          break;
          case mfDINSFIRE:
          ft=sDINSFIRE;
          break;
          case mfARROW:
          ft=sARROW;
          break;
          case mfSARROW:
          ft=sSARROW;
          break;
          case mfGARROW:
          ft=sGARROW;
          break;
          case mfSBOMB:
          ft=sSBOMB;
          break;
          case mfBOMB:
          ft=sBOMB;
          break;
          case mfBRANG:
          ft=sBRANG;
          break;
          case mfMBRANG:
          ft=sMBRANG;
          break;
          case mfFBRANG:
          ft=sFBRANG;
          break;
          case mfWANDMAGIC:
          ft=sWANDMAGIC;
          break;
          case mfREFMAGIC:
          ft=sREFMAGIC;
          break;
          case mfREFFIREBALL:
          ft=sREFFIREBALL;
          break;
          case mfSWORD:
          ft=sSWORD;
          break;
          case mfWSWORD:
          ft=sWSWORD;
          break;
          case mfMSWORD:
          ft=sMSWORD;
          break;
          case mfXSWORD:
          ft=sXSWORD;
          break;
          case mfSWORDBEAM:
          ft=sSWORDBEAM;
          break;
          case mfWSWORDBEAM:
          ft=sWSWORDBEAM;
          break;
          case mfMSWORDBEAM:
          ft=sMSWORDBEAM;
          break;
          case mfXSWORDBEAM:
          ft=sXSWORDBEAM;
          break;
          case mfHOOKSHOT:
          ft=sHOOKSHOT;
          break;
          case mfWAND:
          ft=sWAND;
          break;
          case mfHAMMER:
          ft=sHAMMER;
          break;
          case mfSTRIKE:
          ft=sSTRIKE;
          break;
          default:
          putit = false;
          break;
        }
        if (putit)
        {
          s->data[i] = s->secretcombo[ft];
          s->cset[i] = s->secretcset[ft];
          s->sflag[i] = s->secretflag[ft];
        }
      }

      for (int j=0; j<6; j++)
      {
        for (int iter=0; iter<2; ++iter)
        {
          putit=true;
          int checkflag=combobuf[t[j].data[i]].flag;
          if (iter==1)
          {
            checkflag=t[j].sflag[i];
          }
          switch(checkflag)
          {
            case mfBCANDLE:
            ft=sBCANDLE;
            break;
            case mfRCANDLE:
            ft=sRCANDLE;
            break;
            case mfWANDFIRE:
            ft=sWANDFIRE;
            break;
            case mfDINSFIRE:
            ft=sDINSFIRE;
            break;
            case mfARROW:
            ft=sARROW;
            break;
            case mfSARROW:
            ft=sSARROW;
            break;
            case mfGARROW:
            ft=sGARROW;
            break;
            case mfSBOMB:
            ft=sSBOMB;
            break;
            case mfBOMB:
            ft=sBOMB;
            break;
            case mfBRANG:
            ft=sBRANG;
            break;
            case mfMBRANG:
            ft=sMBRANG;
            break;
            case mfFBRANG:
            ft=sFBRANG;
            break;
            case mfWANDMAGIC:
            ft=sWANDMAGIC;
            break;
            case mfREFMAGIC:
            ft=sREFMAGIC;
            break;
            case mfREFFIREBALL:
            ft=sREFFIREBALL;
            break;
            case mfSWORD:
            ft=sSWORD;
            break;
            case mfWSWORD:
            ft=sWSWORD;
            break;
            case mfMSWORD:
            ft=sMSWORD;
            break;
            case mfXSWORD:
            ft=sXSWORD;
            break;
            case mfSWORDBEAM:
            ft=sSWORDBEAM;
            break;
            case mfWSWORDBEAM:
            ft=sWSWORDBEAM;
            break;
            case mfMSWORDBEAM:
            ft=sMSWORDBEAM;
            break;
            case mfXSWORDBEAM:
            ft=sXSWORDBEAM;
            break;
            case mfHOOKSHOT:
            ft=sHOOKSHOT;
            break;
            case mfWAND:
            ft=sWAND;
            break;
            case mfHAMMER:
            ft=sHAMMER;
            break;
            case mfSTRIKE:
            ft=sSTRIKE;
            break;
            default:
            putit = false;
            break;
          }
          if (putit)
          {
            t[j].data[i] = t[j].secretcombo[ft];
            t[j].cset[i] = t[j].secretcset[ft];
            t[j].sflag[i] = t[j].secretflag[ft];
          }
        }
      }
    }

    if(!(s->flags2&fCLEARSECRET) || high16only)
    {
      if((s->sflag[i] > 15)&&(s->sflag[i] < 32))
      {
        s->data[i] = s->secretcombo[(s->sflag[i])-16+4];
        s->cset[i] = s->secretcset[(s->sflag[i])-16+4];
        s->sflag[i] = s->secretflag[(s->sflag[i])-16+4];
        //        putit = true;
      }
      for (int j=0; j<6; j++)
      {
        if((t[j].sflag[i] > 15)&&(t[j].sflag[i] < 32))
        {
          t[j].data[i] = t[j].secretcombo[(t[j].sflag[i])-16+4];
          t[j].cset[i] = t[j].secretcset[(t[j].sflag[i])-16+4];
          t[j].sflag[i] = t[j].secretflag[(t[j].sflag[i])-16+4];
          //          putit = true;
        }
      }
    }

    /*
      if(putit && refresh)
      putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cset[i]);
      */
  }
}

//foogle

int writeheader_210(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_HEADER;
  dword section_version=V_HEADER_210;
  dword section_cversion=CV_HEADER_210;
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
  char temp_pwd[31]="                              ";
  if(!pfwrite(temp_pwd,30,f))
  {
    return 8;
  }
//  if(!p_iputw(header->pwdkey,f))
  if(!p_iputw(0,f))
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
  if(!pfwrite(header->templatepath,280,f))
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


int writerules_210(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_RULES;
  dword section_version=V_RULES_210;
  dword section_cversion=CV_RULES_210;
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


int writedoorcombosets_210(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_DOORS;
  dword section_version=V_DOORS_210;
  dword section_cversion=CV_DOORS_210;
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

int writedmaps_210(PACKFILE *f, word version, word build, word start_dmap, word max_dmaps)
{
  word dmap_count=count_dmaps();
  dword section_id=ID_DMAPS;
  dword section_version=V_DMAPS_210;
  dword section_cversion=CV_DMAPS_210;
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

int writemisc_210(PACKFILE *f, zquestheader *header, miscQdata *misc)
{
  dword section_id=ID_MISC;
  dword section_version=V_MISC_210;
  dword section_cversion=CV_MISC_210;
  word shops=count_shops(misc);
  word infos=count_infos(misc);
  word warprings=count_warprings(misc);
  word windwarps=count_windwarps(misc);
  word triforces=8;
//  dword section_size=0;
  dword section_size = sizeof(word) + shops * 3 * (sizeof(char) + sizeof(word))
                     + sizeof(word) + infos * 3 * 2 * sizeof(word)
                     + sizeof(word) + warprings * (8 * sizeof(char) + sizeof(char))
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


  //infos
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

int writeitems_210(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_ITEMS;
  dword section_version=V_ITEMS_210;
  dword section_cversion=CV_ITEMS_210;
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

int writeweapons_210(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_WEAPONS;
  dword section_version=V_WEAPONS_210;
  dword section_cversion=CV_WEAPONS_210;
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

int writemaps_210(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_MAPS;
  dword section_version=V_MAPS_210;
  dword section_cversion=CV_MAPS_210;
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
      if(!p_putc(TheMaps[scr].tilewarptype[0],f))
      {
        return qe_invalid;
      }

      {
        if(!p_iputw(TheMaps[scr].door_combo_set,f))
        {
          return qe_invalid;
        }
      }

      if(!p_putc(TheMaps[scr].warpreturnx[0],f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].warpreturny[0],f))
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
      if(!p_putc(TheMaps[scr].tilewarpdmap[0],f))
      {
        return qe_invalid;

      }
      if(!p_putc(TheMaps[scr].tilewarpscr[0],f))
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
      if(!p_putc(TheMaps[scr].sidewarptype[0],f))
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
      if(!p_putc(TheMaps[scr].sidewarpscr[0],f))
      {
        return qe_invalid;
      }
      if(!p_putc(TheMaps[scr].sidewarpdmap[0],f))
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

int writecombos_210(PACKFILE *f, word version, word build, word start_combo, word max_combos)
{
  word combos_used;
  dword section_id=ID_COMBOS;
  dword section_version=V_COMBOS_210;
  dword section_cversion=CV_COMBOS_210;
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

int writecolordata_210(PACKFILE *f, miscQdata *misc, word version, word build, word start_cset, word max_csets)
{
  dword section_id=ID_CSETS;
  dword section_version=V_CSETS_210;
  dword section_cversion=CV_CSETS_210;
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

int writestrings_210(PACKFILE *f, word version, word build, word start_msgstr, word max_msgstrs)
{
  dword section_id=ID_STRINGS;
  dword section_version=V_STRINGS_210;
  dword section_cversion=CV_STRINGS_210;
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

int writetiles_210(PACKFILE *f, word version, word build, word start_tile, word max_tiles)
{
  word tiles_used;
  dword section_id=ID_TILES;
  dword section_version=V_TILES_210;
  dword section_cversion=CV_TILES_210;
//  dword section_size=0;
  tiles_used = count_tiles()-start_tile;
  tiles_used = min(tiles_used, max_tiles);
  tiles_used = min(tiles_used, NEWMAXTILES);
  dword section_size = sizeof(word)
                     + (int)tiles_used * (int)SINGLE_TILE_SIZE;

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
  tiles_used=count_tiles()-start_tile;
  tiles_used=min(tiles_used, max_tiles);
  tiles_used=min(tiles_used, NEWMAXTILES);
  if(!p_iputw(tiles_used,f))
  {
    return 5;
  }

//  if(!pfwrite(tilebuf+start_tile,dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)),f))
  for (dword i=0; i<tiles_used; ++i)
  {
    if(!pfwrite(newtilebuf[start_tile+i].data,tilesize(newtilebuf[start_tile+i].format),f))
    {
      return 6;
    }
  }
  return 0;
}

int writemidis_210(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_MIDIS;
  dword section_version=V_MIDIS_210;
  dword section_cversion=CV_MIDIS_210;
  dword section_size = 0;
  {
    int midi_count = 0;
    for(int i = 0; i < MAXCUSTOMMIDIS; i++ )
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
  for(int i=0; i<MAXCUSTOMMIDIS; i++)
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

int writecheats_210(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_CHEATS;
  dword section_version=V_CHEATS_210;
  dword section_cversion=CV_CHEATS_210;
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

int writeguys_210(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_GUYS;
  dword section_version=V_GUYS_210;
  dword section_cversion=CV_GUYS_210;
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

int writeinitdata_210(PACKFILE *f, zquestheader *header)
{
  dword section_id=ID_INITDATA;
  dword section_version=V_INITDATA_210;
  dword section_cversion=CV_INITDATA_210;
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
//  if(!p_putc(zinit.linkwalkstyle,f))
  if(!p_putc(zinit.linkanimationstyle,f))
  {
    return 60;
  }
  return 0;
}



int save_unencoded_quest_210(char *filename, bool compressed)
{
  reset_combo_animations();
  strcpy(header.id_str,QH_NEWIDSTR);
  header.zelda_version = ZELDA_VERSION_210;
  header.internal = INTERNAL_VERSION;
  // header.str_count = msg_count;
  // header.data_flags[ZQ_TILES] = usetiles;
  header.data_flags[ZQ_TILES] = true;
  header.data_flags[ZQ_CHEATS2] = 1;
  header.build=VERSION_BUILD_210;
  for(int i=0; i<MAXCUSTOMMIDIS; i++)
  {
    set_bit(midi_flags,i,int(customMIDIs[i].midi!=NULL));
  }

  char keyfilename[256];
  // word combos_used;
  // word tiles_used;
  replace_extension(keyfilename, filepath, "key", 255);




  box_start(1, "Saving Quest in 2.10 format", lfont, font, true);
  box_out("Saving Quest...");
  box_eol();
  box_eol();

  PACKFILE *f = pack_fopen(filename,compressed?F_WRITE_PACKED:F_WRITE);
  if(!f)
    return 1;

  box_out("Writing 2.10 Header...");
  if (writeheader_210(f,&header)!=0)
  {
    return 2;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing 2.10 Rules...");
  if (writerules_210(f,&header)!=0)
  {
    return 3;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing 2.10 Strings...");
  if (writestrings_210(f, ZELDA_VERSION, VERSION_BUILD, 0, MAXMSGS)!=0)
  {
    return 4;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing 2.10 Doors...");
  if (writedoorcombosets_210(f,&header)!=0)
  {
    return 5;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing 2.10 DMaps...");
  if (writedmaps_210(f,header.zelda_version,header.build,0,MAXDMAPS)!=0)
  {
    return 6;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing 2.10 Misc. Data...");
  if (writemisc_210(f,&header,&misc)!=0)
  {
    return 7;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing 2.10 Items...");
  if (writeitems_210(f,&header)!=0)
  {
    return 8;
  }

  box_out("okay.");
  box_eol();

  box_out("Writing 2.10 Weapons...");
  if (writeweapons_210(f,&header)!=0)
  {
    return 9;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing 2.10 Maps...");
  if (writemaps_210(f,&header)!=0)
  {
    return 10;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing 2.10 Combos...");
  if (writecombos_210(f,header.zelda_version,header.build,0,MAXCOMBOS)!=0)
  {
    return 11;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing 2.10 Color Data...");
  if (writecolordata_210(f,&misc,header.zelda_version,header.build,0,newpdTOTAL)!=0)
  {
    return 12;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing 2.10 Tiles...");
  if (writetiles_210(f,header.zelda_version,header.build,0,NEWMAXTILES)!=0)
  {

    return 13;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing 2.10 MIDIs...");
  if (writemidis_210(f,&header)!=0)
  {
    return 14;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing 2.10 Cheat Codes...");
  if (writecheats_210(f,&header)!=0)
  {
    return 15;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing 2.10 Init. Data...");
  if (writeinitdata_210(f,&header)!=0)
  {
    return 16;
  }
  box_out("okay.");
  box_eol();

  box_out("Writing 2.10 Enemy Data...");
  if (writeguys_210(f,&header)!=0)
  {
    return 17;
  }
  box_out("okay.");
  box_eol();

  pack_fclose(f);

  return 0;
}

