//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  qst.cc
//
//  Code for loading '.qst' files in ZC and ZQuest.
//
//--------------------------------------------------------


#include <stdio.h>
#include <string.h>
#include <allegro.h>

#include "zdefs.h"
#include "colors.h"
#include "tiles.h"
#include "zsys.h"
#include "qst.h"
#include "zquest.h"

#include "defdata.cc"


// extern bool debug;
extern mapscr      *TheMaps;
extern MsgStr      *MsgStrings;
extern dmap        *DMaps;
extern newcombo    *combobuf;
extern byte        *colordata;
extern byte        *tilebuf;
extern itemdata    *itemsbuf;
extern wpndata     *wpnsbuf;
extern guydata     *guysbuf;
extern ZCHEATS     zcheats;
extern zinitdata   zinit;
extern char        palnames[256][17];

//enum { qe_OK, qe_notfound, qe_invalid, qe_version, qe_obsolete,
//       qe_missing, qe_internal, qe_pwd, qe_match, qe_minver };

char *qst_error[] = {"OK","File not found","Invalid quest file",
                     "Version not supported","Obsolete version",
                     "Missing new data" /* but let it pass in ZQuest */ ,
                     "Internal error occurred", "Invalid password",
                     "Doesn't match saved game", "New quest version; please restart game",
                      };

char *VerStr(int version)
{
  static char ver_str[12];
  sprintf(ver_str,"v%d.%02X",version>>8,version&0xFF);
  return ver_str;
}


char *ordinal(int num)
{
  static char *ending[4] = {"st","nd","rd","th"};
  static char ord_str[8];

  char *end;
  int t=(num%100)/10;
  int n=num%10;

  if(n>=1 && n<4 && t!=1)
    end = ending[n-1];
  else
    end = ending[3];

  sprintf(ord_str,"%d%s",num%10000,end);
  return ord_str;
}



bool get_qst_buffers()
{
  if(!(TheMaps=(mapscr*)malloc(sizeof(mapscr)*MAPSCRS*MAXMAPS)))
    goto nogo;
  if(!(MsgStrings=(MsgStr*)malloc(sizeof(MsgStr)*MAXMSGS)))
    goto nogo;
  if(!(DMaps=(dmap*)malloc(sizeof(dmap)*MAXDMAPS)))
    goto nogo;
  if(!(combobuf=(newcombo*)malloc(sizeof(newcombo)*MAXCOMBOS)))
    goto nogo;
  if(!(colordata=(byte*)malloc(newpsTOTAL)))
    goto nogo;
  if(!(tilebuf=(byte*)malloc(NEWTILE_SIZE2)))
    goto nogo;
  if(!(itemsbuf=(itemdata*)malloc(sizeof(itemdata)*MAXITEMS)))
    goto nogo;
  if(!(wpnsbuf=(wpndata*)malloc(sizeof(wpndata)*MAXWPNS)))
    goto nogo;
  if(!(guysbuf=(guydata*)malloc(sizeof(guydata)*MAXGUYS)))
    goto nogo;

  return true;

nogo:
  return false;
}


bool init_palnames()
{
  if(palnames==NULL)
    return false;

  for (int x=0; x<256; x++) {
    switch (x) {
      case 0:  sprintf(palnames[x],"Overworld"); break;
      case 10: sprintf(palnames[x],"Caves"); break;
      case 11: sprintf(palnames[x],"Passageways"); break;
      default: sprintf(palnames[x],"%c",0); break;
    }
  }

  return true;
};


static void *read_block(PACKFILE *f, int size, int alloc_size)
{
   void *p;

   p = malloc(MAX(size, alloc_size));
   if (!p)
      return NULL;

   pack_fread(p, size, f);

   if (pack_ferror(f)) {
      free(p);
      return NULL;
   }

   return p;
}



/* read_midi:
 *  Reads MIDI data from a datafile (this is not the same thing as the 
 *  standard midi file format).
 */
static MIDI *read_midi(PACKFILE *f)
{
   MIDI *m;
   int c;

   m = (MIDI*)malloc(sizeof(MIDI));
   if (!m)
     return NULL;

   for (c=0; c<MIDI_TRACKS; c++) {
      m->track[c].len = 0;
      m->track[c].data = NULL;
   }

   m->divisions = pack_mgetw(f);

   for (c=0; c<MIDI_TRACKS; c++) {
      m->track[c].len = pack_mgetl(f);
      if (m->track[c].len > 0) {
	 m->track[c].data = (byte*)read_block(f, m->track[c].len, 0);
	 if (!m->track[c].data) {
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


void clear_combo(int i)
{
  combobuf[i].tile=0;
  combobuf[i].flip=0;
  combobuf[i].walk=0;
  combobuf[i].type=0;
  combobuf[i].csets=0;
  combobuf[i].foo=0;
  combobuf[i].frames=0;
  combobuf[i].speed=0;
  combobuf[i].drawtile=0;
  for (int x=0; x<COMBO_FOO2S; x++)
    combobuf[i].foo2[x]=0;
}

void clear_combos()
{
  for(int tmpcounter=0; tmpcounter<MAXCOMBOS; tmpcounter++)
    clear_combo(tmpcounter);
}

void pack_combos()
{
  int di = 0;
  for(int si=0; si<1024; si+=2)
    combobuf[di++] = combobuf[si];

  for( ; di<1024; di++)
    clear_combo(di);
}


void fix_maps(mapscr *buf,int cnt)
{
  for(int i=0; i<cnt; i++)
    for(int j=0; j<176; j++)
    {
      int d = buf[i].data[j];
      int c = d&0xFF;
      buf[i].data[j] = ((d&0xFE00)>>1) + c;
    }
}


void reset_midi(music *m)
{
  m->title[0]=0;
  m->loop=1;
  m->volume=144;
  m->start=0;
  m->loop_start=-1;
  m->loop_end=-1;
  if(m->midi)
    destroy_midi(m->midi);
  m->midi=NULL;
}


void reset_scr(int scr)
{
  byte *di=((byte*)TheMaps)+(scr*sizeof(mapscr));
  for(unsigned i=0; i<sizeof(mapscr); i++)
    *(di++) = 0;
  TheMaps[scr].valid=mVERSION;
}

/*
void reset_items()
{
  short *tmpptr=(short*)data[DAT_ZGP].dat+1;//skip version
  byte *tmpptr2=(byte*)tmpptr+1;//skip build
  word *tilestoskip=(word*)tmpptr2; //check number of tiles
  byte *tmpptr3=(byte*)(tilestoskip+1); //skip past number of tiles
  //skip past tile section
  byte *tmpptr4=tmpptr3+((dword)(*tilestoskip)*(dword)SINGLE_TILE_SIZE);
  word *combostoread=(word*)tmpptr4; //check number of combos
  byte *tmpptr5=(byte*)(combostoread+1); //skip past number of combos
  //skip past combo section
  byte *tmpptr6=tmpptr5+((*combostoread)*sizeof(newcombo));
  tmpptr6=tmpptr6+newpsTOTAL; //skip past palette info
  tmpptr6=tmpptr6+(sizeof(palcycle)*256*3); //skip past cycling info
  tmpptr6=tmpptr6+(MAXLEVELS*PALNAMESIZE); //skip past palette names
  memcpy(itemsbuf, tmpptr6, (sizeof(itemdata)*MAXITEMS)); //read item info
}


void reset_wpns()
{
  short *tmpptr=(short*)data[DAT_ZGP].dat+1;//skip version
  byte *tmpptr2=(byte*)tmpptr+1;//skip build
  word *tilestoskip=(word*)tmpptr2; //check number of tiles
  byte *tmpptr3=(byte*)(tilestoskip+1); //skip past number of tiles
  //skip past tile section
  byte *tmpptr4=tmpptr3+((dword)(*tilestoskip)*(dword)SINGLE_TILE_SIZE);
  word *combostoread=(word*)tmpptr4; //check number of combos
  byte *tmpptr5=(byte*)(combostoread+1); //skip past number of combos
  //skip past combo section
  byte *tmpptr6=tmpptr5+((*combostoread)*sizeof(newcombo));
  tmpptr6=tmpptr6+newpsTOTAL; //skip past palette info
  tmpptr6=tmpptr6+(sizeof(palcycle)*256*3); //skip past cycling info
  tmpptr6=tmpptr6+(MAXLEVELS*PALNAMESIZE); //skip past palette names
  tmpptr6=tmpptr6+(sizeof(itemdata)*MAXITEMS); //skip past item info
  memcpy(wpnsbuf, tmpptr6, (sizeof(wpndata)*MAXWPNS)); //read weapon info
}
*/


/*  For reference:

    enum { qe_OK, qe_notfound, qe_invalid, qe_version, qe_obsolete,
       qe_missing, qe_internal, qe_pwd, qe_match, qe_minver };
*/





int loadquest(char *filename, zquestheader *header,
              miscQdata *misc, music *midis)
{
//  alert("1",NULL,NULL,"O&K",NULL,'k',0);
  int weapons_to_read=MAXWPNS;
//  alert("1b",NULL,NULL,"O&K",NULL,'k',0);
  int items_to_read=MAXITEMS;
//  alert("1c",NULL,NULL,"O&K",NULL,'k',0);
  char tmpbuf[L_tmpnam];
//  alert(tmpbuf,NULL,NULL,"O&K",NULL,'k',0);
  char *tmpfilename = tmpnam(tmpbuf);

//  alert(tmpfilename,NULL,NULL,"O&K",NULL,'k',0);
  // oldquest flag is set when an unencrypted qst file is suspected.
  bool oldquest = false;

  int ret = decode_file_007(filename, tmpfilename, ENC_STR);
  if(ret)
  {
    switch(ret)
    {
    case 1: return qe_notfound;
    case 2: return qe_internal;
    default: oldquest = true;
    // be sure not to delete tmpfilename now...
    }
  }
//  alert("3",NULL,NULL,"O&K",NULL,'k',0);
  PACKFILE *f = pack_fopen(oldquest ? filename : tmpfilename, F_READ_PACKED);
//  alert("3b",NULL,NULL,"O&K",NULL,'k',0);
  if(!f)
  {
    if(!oldquest){
      delete_file(tmpfilename);
    }
    return qe_invalid;
  }

//  alert("4",NULL,NULL,"O&K",NULL,'k',0);
  if(!pfread(header,sizeof(oldheader),f)){   // first read old header
    goto invalid;
  }


  // check header

//  alert("5",NULL,NULL,"O&K",NULL,'k',0);
  if(header->zelda_version > ZELDA_VERSION)
  {
    pack_fclose(f);
    if(!oldquest)
      delete_file(tmpfilename);
    return qe_version;
  }

//  alert("6",NULL,NULL,"O&K",NULL,'k',0);
  if(strcmp(header->id_str,QH_IDSTR))
  {
    pack_fclose(f);
    if(!oldquest)
      delete_file(tmpfilename);
    return qe_invalid;
  }

  // handle old bootleg quest files...
  /*
  if(oldquest)
  {
    if(!debug || (alert("Old quest detected", "Bootleg file?", NULL, "Kill", "Open", 13, 27) == 1))
    {
      pack_fclose(f);
      if(encode_file_007(filename, tmpfilename, 0x413F0000 + filename[0] + (filename[1] << 8), "Bootleg Quest") == 0)
      {
        copy_file(tmpfilename, filename);
        delete_file(tmpfilename);
      }
      return qe_obsolete;
    }
  }
  */

//  alert("7",NULL,NULL,"O&K",NULL,'k',0);
  if(bad_version(header))
  {
    pack_fclose(f);
    if(!oldquest)
      delete_file(tmpfilename);
    return qe_obsolete;
  }

//  alert("8",NULL,NULL,"O&K",NULL,'k',0);
  if(header->zelda_version < 0x177)      // lacks new header stuff...
  {                                      //   char minver[9], byte build
    memset(header->minver,0,20);         //   byte foo[10]
  }
  else if(!pfread(header->minver,20,f)) {  // read new header additions
    goto invalid;                        // starting at minver
  }

//  alert("9",NULL,NULL,"O&K",NULL,'k',0);
  if(header->zelda_version < 0x187)      // lacks newer header stuff...
  {                                      //   word rules3..rules10
    memset(header->rules3,0,16); //
  }
  else if(!pfread(header->rules3,16,f)) {  // read new header additions
      goto invalid;                        // starting at rules3
  }

//  alert("9b",NULL,NULL,"O&K",NULL,'k',0);
  if(header->zelda_version < 0x192)      //  lacks newer header stuff...
  {                                      //  byte foo2[50] (for expansion)
    memset(header->foo2,0,50); //
  }
  else if(!pfread(header->foo2,50,f)) {  // read new header additions
      goto invalid;                        // starting at foo2
  }

  // strings & DMaps
//  char flaf[80];
//  sprintf(flaf, "build %d", header->build);
//  alert(flaf,NULL,NULL,"O&K",NULL,'k',0);

//  alert("10",NULL,NULL,"O&K",NULL,'k',0);
  if ((header->zelda_version < 0x192)||
     ((header->zelda_version == 0x192)&&(header->build<31))) {
    if(!pfread(MsgStrings,sizeof(MsgStr)*128,f)) {
      goto invalid;
    }
  } else {
    if(!pfread(MsgStrings,sizeof(MsgStr)*MAXMSGS,f)) {
      goto invalid;
    }
  }

//  alert("11",NULL,NULL,"O&K",NULL,'k',0);
  int dmapstoread;
  if ((header->zelda_version < 0x192)||
     ((header->zelda_version == 0x192)&&(header->build<41))) {
    if ((header->zelda_version < 0x192)||
       ((header->zelda_version == 0x192)&&(header->build<5))) {
      dmapstoread=32;
    } else {
      dmapstoread=MAXDMAPS;
    }
    for (int x=0; x<dmapstoread; x++) {
        if(!pfread(&DMaps[x],16,f))
          goto invalid;
        for (int y=0; y<=21; y++) {
          DMaps[x].name[y]=0;
        }
        if (DMaps[x].level>0&&DMaps[x].level<10) {
          sprintf(DMaps[x].title,"LEVEL-%d             ", DMaps[x].level);
        } else {
          sprintf(DMaps[x].title,"                    ");
        }
        // 72 spaces
        sprintf(DMaps[x].intro,"                                                                        ");
    }
    if (dmapstoread<MAXDMAPS) {
      for (int x=dmapstoread; x<MAXDMAPS; x++) {
        for (int y=0; y<=21; y++) {
          DMaps[x].name[y]=0;
        }
        sprintf(DMaps[x].title,"                    ");
        // 72 spaces
        sprintf(DMaps[x].intro,"                                                                        ");
      }
    }
  } else {
    if(!pfread(DMaps,sizeof(dmap)*MAXDMAPS,f))
      goto invalid;
  }




  // misc data

//  alert("12",NULL,NULL,"O&K",NULL,'k',0);

  if ((header->zelda_version < 0x192)||
     ((header->zelda_version == 0x192)&&(header->build<34))) {
    miscQdata192b29 tempmisc;
    if(!pfread(&tempmisc,sizeof(miscQdata192b29),f)) {
      goto invalid;
    } else {
      for (int x=0; x<16; x++) {
        misc->shop[x]=tempmisc.shop[x];
      }

      for (int x=0; x<16; x++) {
        misc->info[x]=tempmisc.info[x];
      }

      for (int x=0; x<8; x++) {
        misc->warp[x]=tempmisc.warp[x];
      }

      palcycle tempcycle;
      tempcycle.first=0;
      tempcycle.count=0;
      tempcycle.speed=0;
      for (int x=0; x<256; x++) {
        for (int y=0; y<3; y++) {
          if (x<16) {
            misc->cycles[x][y]=tempmisc.cycles[x][y];
          } else {
            misc->cycles[x][y]=tempcycle;
          }
        }
      }

      for (int x=0; x<9; x++) {
        misc->wind[x]=tempmisc.wind[x];
      }

      for (int x=0; x<8; x++) {
        misc->triforce[x]=tempmisc.triforce[x];
      }

      misc->colors=tempmisc.colors;

      for (int x=0; x<3; x++) {
        misc->icons[x]=tempmisc.icons[x];
      }
      misc->icons[4]=0;

      for (int x=0; x<8; x++) {
        misc->warp[x]=tempmisc.warp[x];
      }

      for (int x=0; x<8; x++) {
        misc->warp[x]=tempmisc.warp[x];
      }

      for (int x=0; x<8; x++) {
        misc->warp[x]=tempmisc.warp[x];
      }

      for (int x=0; x<16; x++) {
        for (int y=0; y<3; y++) {
          misc->pond[x].olditem[y]=0;
          misc->pond[x].newitem[y]=0;
        }
        misc->pond[x].d1=0;
        misc->pond[x].fairytile=0;
        misc->pond[x].aframes=0;
        misc->pond[x].aspeed=0;
        for (int y=0; y<15; y++) {
          misc->pond[x].msg[y]=0;
        }
      }

      misc->endstring=0;
      misc->dummy=0;
      for (int x=0; x<98; x++) {
        misc->expansion[x]=0;
      }
    }
  } else if ((header->zelda_version == 0x192)&&(header->build<73)) {
    miscQdata192b72 tempmisc;
    if(!pfread(&tempmisc,sizeof(miscQdata192b72),f)) {
      goto invalid;
    } else {
      for (int x=0; x<16; x++) {
        misc->shop[x]=tempmisc.shop[x];
      }

      for (int x=0; x<16; x++) {
        misc->info[x]=tempmisc.info[x];
      }

      for (int x=0; x<8; x++) {
        misc->warp[x]=tempmisc.warp[x];
      }

      palcycle tempcycle;
      tempcycle.first=0;
      tempcycle.count=0;
      tempcycle.speed=0;
      for (int x=0; x<256; x++) {
        for (int y=0; y<3; y++) {
          if (x<16) {
            misc->cycles[x][y]=tempmisc.cycles[x][y];
          } else {
            misc->cycles[x][y]=tempcycle;
          }
        }
      }

      for (int x=0; x<9; x++) {
        misc->wind[x]=tempmisc.wind[x];
      }

      for (int x=0; x<8; x++) {
        misc->triforce[x]=tempmisc.triforce[x];
      }

      misc->colors=tempmisc.colors;

      for (int x=0; x<3; x++) {
        misc->icons[x]=tempmisc.icons[x];
      }
      misc->icons[4]=0;

      for (int x=0; x<16; x++) {
        misc->pond[x]=tempmisc.pond[x];
      }

      for (int x=0; x<8; x++) {
        misc->warp[x]=tempmisc.warp[x];
      }

      for (int x=0; x<8; x++) {
        misc->warp[x]=tempmisc.warp[x];
      }

      for (int x=0; x<8; x++) {
        misc->warp[x]=tempmisc.warp[x];
      }

      misc->endstring=tempmisc.endstring;

      misc->dummy=tempmisc.dummy;

      for (int x=0; x<98; x++) {
        misc->expansion[x]=tempmisc.expansion[x];
      }
    }
  } else {
    if(!pfread(misc,sizeof(miscQdata),f)) {
      goto invalid;
    }
  }


  // items, weapons, and guys

//  alert("13",NULL,NULL,"O&K",NULL,'k',0);
  if(header->zelda_version < 0x186) {
    items_to_read=64;
  }

  if(!pfread(itemsbuf,sizeof(itemdata)*items_to_read,f)) {
    goto invalid;
  }

//  alert("14",NULL,NULL,"O&K",NULL,'k',0);
  if(header->zelda_version < 0x186) {
    weapons_to_read=64;
  }
  if(header->zelda_version < 0x185) {
    weapons_to_read=32;
  }

//  alert("15",NULL,NULL,"O&K",NULL,'k',0);
  if(!pfread(wpnsbuf,sizeof(wpndata)*weapons_to_read,f)) {
    goto invalid;
  }

//  alert("16",NULL,NULL,"O&K",NULL,'k',0);
  if(header->zelda_version < 0x176)
  {
    wpnsbuf[iwSpawn] = *((wpndata*)(itemsbuf + iMisc1));
    wpnsbuf[iwDeath] = *((wpndata*)(itemsbuf + iMisc2));
    itemsbuf[iMisc1] = itemsbuf[iMisc2] = (itemdata){0,0,0,0,0,0,0};
  }
//  alert("17",NULL,NULL,"O&K",NULL,'k',0);
  for(int i=0; i<MAXGUYS; i++)
    guysbuf[i] = default_guys[i];  //using default data here...


  // map screens
  if ((header->zelda_version < 0x192)||
     ((header->zelda_version == 0x192)&&(header->build<10))) {

    mapscr192b9   TheOldMap;

    for(int i=0; i<header->map_count && i<MAXMAPS; i++) {
      for (int j=0; j<MAPSCRS; j++) {
        if(!pfread(&TheOldMap,sizeof(mapscr192b9),f)) {
          goto invalid;
        }
        TheMaps[i*MAPSCRS+j].valid=TheOldMap.valid;
        TheMaps[i*MAPSCRS+j].guy=TheOldMap.guy;
        TheMaps[i*MAPSCRS+j].str=TheOldMap.str;
        TheMaps[i*MAPSCRS+j].room=TheOldMap.room;
        TheMaps[i*MAPSCRS+j].item=TheOldMap.item;
        TheMaps[i*MAPSCRS+j]._FOO1_=TheOldMap._FOO1_;
        TheMaps[i*MAPSCRS+j].warptype=TheOldMap.warptype;
        TheMaps[i*MAPSCRS+j]._FOO2_=TheOldMap._FOO2_;
        TheMaps[i*MAPSCRS+j].warpx=TheOldMap.warpx;
        TheMaps[i*MAPSCRS+j].warpy=TheOldMap.warpy;
        TheMaps[i*MAPSCRS+j].stairx=TheOldMap.stairx;
        TheMaps[i*MAPSCRS+j].stairy=TheOldMap.stairy;
        TheMaps[i*MAPSCRS+j].itemx=TheOldMap.itemx;
        TheMaps[i*MAPSCRS+j].itemy=TheOldMap.itemy;
        TheMaps[i*MAPSCRS+j].color=TheOldMap.color;
        TheMaps[i*MAPSCRS+j].enemyflags=TheOldMap.enemyflags;
        for (int k=0; k<4; k++) {
          TheMaps[i*MAPSCRS+j].door[k]=TheOldMap.door[k];
        }
        TheMaps[i*MAPSCRS+j].warpdmap=TheOldMap.warpdmap;
        TheMaps[i*MAPSCRS+j].warpscr=TheOldMap.warpscr;
        TheMaps[i*MAPSCRS+j].exitdir=TheOldMap.exitdir;
        TheMaps[i*MAPSCRS+j]._FOO3_=TheOldMap._FOO3_;
        for (int k=0; k<10; k++) {
          TheMaps[i*MAPSCRS+j].enemy[k]=TheOldMap.enemy[k];
        }
        TheMaps[i*MAPSCRS+j].pattern=TheOldMap.pattern;
        TheMaps[i*MAPSCRS+j].warptype2=TheOldMap.warptype2;
        TheMaps[i*MAPSCRS+j].warpx2=TheOldMap.warpx2;
        TheMaps[i*MAPSCRS+j].warpy2=TheOldMap.warpy2;
        for (int k=0; k<4; k++) {
          TheMaps[i*MAPSCRS+j].path[k]=TheOldMap.path[k];
        }
        TheMaps[i*MAPSCRS+j].warpscr2=TheOldMap.warpscr2;
        TheMaps[i*MAPSCRS+j].warpdmap2=TheOldMap.warpdmap2;
        TheMaps[i*MAPSCRS+j].under=TheOldMap.under;
        TheMaps[i*MAPSCRS+j].cpage=TheOldMap.cpage;
        TheMaps[i*MAPSCRS+j]._FOO4_=TheOldMap._FOO4_;
        TheMaps[i*MAPSCRS+j].catchall=TheOldMap.catchall;
        TheMaps[i*MAPSCRS+j].flags=TheOldMap.flags;
        TheMaps[i*MAPSCRS+j].flags2=TheOldMap.flags2;
        TheMaps[i*MAPSCRS+j].flags3=TheOldMap.flags3;
        for (int k=0; k<6; k++) {
          TheMaps[i*MAPSCRS+j].layermap[k]=0;
          TheMaps[i*MAPSCRS+j].layerscreen[k]=0;
        }
        for (int k=0; k<32; k++) {
          TheMaps[i*MAPSCRS+j].extra[k]=(k<11)?TheOldMap.extra[k]:0;
        }
        for (int k=0; k<20; k++) {
          TheMaps[i*MAPSCRS+j].secret[k]=TheOldMap.secret[k];
        }
        for (int k=0; k<(16*11); k++) {
          TheMaps[i*MAPSCRS+j].data[k]=TheOldMap.data[k];
        }
        for (int k=0; k<176; k++) {
          TheMaps[i*MAPSCRS+j].sflag[k]=(TheOldMap.data[k]>>11);
          TheMaps[i*MAPSCRS+j].data[k]=(TheOldMap.data[k]&0x7FF);
        }
      }
    }
  } else if (((header->zelda_version == 0x192)&&(header->build<21))) {
    mapscr192b20   TheOldMap;

    for(int i=0; i<header->map_count && i<MAXMAPS; i++) {
      for (int j=0; j<MAPSCRS; j++) {
        if(!pfread(&TheOldMap,sizeof(mapscr192b20),f)) {
          goto invalid;
        }
        TheMaps[i*MAPSCRS+j].valid=TheOldMap.valid;
        TheMaps[i*MAPSCRS+j].guy=TheOldMap.guy;
        TheMaps[i*MAPSCRS+j].str=TheOldMap.str;
        TheMaps[i*MAPSCRS+j].room=TheOldMap.room;
        TheMaps[i*MAPSCRS+j].item=TheOldMap.item;
        TheMaps[i*MAPSCRS+j]._FOO1_=TheOldMap._FOO1_;
        TheMaps[i*MAPSCRS+j].warptype=TheOldMap.warptype;
        TheMaps[i*MAPSCRS+j]._FOO2_=TheOldMap._FOO2_;
        TheMaps[i*MAPSCRS+j].warpx=TheOldMap.warpx;
        TheMaps[i*MAPSCRS+j].warpy=TheOldMap.warpy;
        TheMaps[i*MAPSCRS+j].stairx=TheOldMap.stairx;
        TheMaps[i*MAPSCRS+j].stairy=TheOldMap.stairy;
        TheMaps[i*MAPSCRS+j].itemx=TheOldMap.itemx;
        TheMaps[i*MAPSCRS+j].itemy=TheOldMap.itemy;
        TheMaps[i*MAPSCRS+j].color=TheOldMap.color;
        TheMaps[i*MAPSCRS+j].enemyflags=TheOldMap.enemyflags;
        for (int k=0; k<4; k++) {
          TheMaps[i*MAPSCRS+j].door[k]=TheOldMap.door[k];
        }
        TheMaps[i*MAPSCRS+j].warpdmap=TheOldMap.warpdmap;
        TheMaps[i*MAPSCRS+j].warpscr=TheOldMap.warpscr;
        TheMaps[i*MAPSCRS+j].exitdir=TheOldMap.exitdir;
        TheMaps[i*MAPSCRS+j]._FOO3_=TheOldMap._FOO3_;
        for (int k=0; k<10; k++) {
          TheMaps[i*MAPSCRS+j].enemy[k]=TheOldMap.enemy[k];
        }
        TheMaps[i*MAPSCRS+j].pattern=TheOldMap.pattern;
        TheMaps[i*MAPSCRS+j].warptype2=TheOldMap.warptype2;
        TheMaps[i*MAPSCRS+j].warpx2=TheOldMap.warpx2;
        TheMaps[i*MAPSCRS+j].warpy2=TheOldMap.warpy2;
        for (int k=0; k<4; k++) {
          TheMaps[i*MAPSCRS+j].path[k]=TheOldMap.path[k];
        }
        TheMaps[i*MAPSCRS+j].warpscr2=TheOldMap.warpscr2;
        TheMaps[i*MAPSCRS+j].warpdmap2=TheOldMap.warpdmap2;
        TheMaps[i*MAPSCRS+j].under=TheOldMap.under;
        TheMaps[i*MAPSCRS+j].cpage=TheOldMap.cpage;
        TheMaps[i*MAPSCRS+j]._FOO4_=TheOldMap._FOO4_;
        TheMaps[i*MAPSCRS+j].catchall=TheOldMap.catchall;
        TheMaps[i*MAPSCRS+j].flags=TheOldMap.flags;
        TheMaps[i*MAPSCRS+j].flags2=TheOldMap.flags2;
        TheMaps[i*MAPSCRS+j].flags3=TheOldMap.flags3;
        for (int k=0; k<6; k++) {
          TheMaps[i*MAPSCRS+j].layermap[k]=0;
          TheMaps[i*MAPSCRS+j].layerscreen[k]=0;
        }
        for (int k=0; k<32; k++) {
          TheMaps[i*MAPSCRS+j].extra[k]=(k<11)?TheOldMap.extra[k]:0;
        }
        for (int k=0; k<20; k++) {
          TheMaps[i*MAPSCRS+j].secret[k]=TheOldMap.secret[k];
        }
        for (int k=0; k<(16*11); k++) {
          TheMaps[i*MAPSCRS+j].data[k]=TheOldMap.data[k];
        }
        for (int k=0; k<176; k++) {
          TheMaps[i*MAPSCRS+j].sflag[k]=(TheOldMap.data[k]>>11);
          TheMaps[i*MAPSCRS+j].data[k]=(TheOldMap.data[k]&0x7FF);
        }
      }
    }
  } else if (((header->zelda_version == 0x192)&&(header->build<24))) {
    mapscr192b23   TheOldMap;

    for(int i=0; i<header->map_count && i<MAXMAPS; i++) {
      for (int j=0; j<MAPSCRS; j++) {
        if(!pfread(&TheOldMap,sizeof(mapscr192b23),f)) {
          goto invalid;
        }
        TheMaps[i*MAPSCRS+j].valid=TheOldMap.valid;
        TheMaps[i*MAPSCRS+j].guy=TheOldMap.guy;
        TheMaps[i*MAPSCRS+j].str=TheOldMap.str;
        TheMaps[i*MAPSCRS+j].room=TheOldMap.room;
        TheMaps[i*MAPSCRS+j].item=TheOldMap.item;
        TheMaps[i*MAPSCRS+j]._FOO1_=TheOldMap._FOO1_;
        TheMaps[i*MAPSCRS+j].warptype=TheOldMap.warptype;
        TheMaps[i*MAPSCRS+j]._FOO2_=TheOldMap._FOO2_;
        TheMaps[i*MAPSCRS+j].warpx=TheOldMap.warpx;
        TheMaps[i*MAPSCRS+j].warpy=TheOldMap.warpy;
        TheMaps[i*MAPSCRS+j].stairx=TheOldMap.stairx;
        TheMaps[i*MAPSCRS+j].stairy=TheOldMap.stairy;
        TheMaps[i*MAPSCRS+j].itemx=TheOldMap.itemx;
        TheMaps[i*MAPSCRS+j].itemy=TheOldMap.itemy;
        TheMaps[i*MAPSCRS+j].color=TheOldMap.color;
        TheMaps[i*MAPSCRS+j].enemyflags=TheOldMap.enemyflags;
        for (int k=0; k<4; k++) {
          TheMaps[i*MAPSCRS+j].door[k]=TheOldMap.door[k];
        }
        TheMaps[i*MAPSCRS+j].warpdmap=TheOldMap.warpdmap;
        TheMaps[i*MAPSCRS+j].warpscr=TheOldMap.warpscr;
        TheMaps[i*MAPSCRS+j].exitdir=TheOldMap.exitdir;
        TheMaps[i*MAPSCRS+j]._FOO3_=TheOldMap._FOO3_;
        for (int k=0; k<10; k++) {
          TheMaps[i*MAPSCRS+j].enemy[k]=TheOldMap.enemy[k];
        }
        TheMaps[i*MAPSCRS+j].pattern=TheOldMap.pattern;
        TheMaps[i*MAPSCRS+j].warptype2=TheOldMap.warptype2;
        TheMaps[i*MAPSCRS+j].warpx2=TheOldMap.warpx2;
        TheMaps[i*MAPSCRS+j].warpy2=TheOldMap.warpy2;
        for (int k=0; k<4; k++) {
          TheMaps[i*MAPSCRS+j].path[k]=TheOldMap.path[k];
        }
        TheMaps[i*MAPSCRS+j].warpscr2=TheOldMap.warpscr2;
        TheMaps[i*MAPSCRS+j].warpdmap2=TheOldMap.warpdmap2;
        TheMaps[i*MAPSCRS+j].under=TheOldMap.under;
        TheMaps[i*MAPSCRS+j].cpage=TheOldMap.cpage;
        TheMaps[i*MAPSCRS+j]._FOO4_=TheOldMap._FOO4_;
        TheMaps[i*MAPSCRS+j].catchall=TheOldMap.catchall;
        TheMaps[i*MAPSCRS+j].flags=TheOldMap.flags;
        TheMaps[i*MAPSCRS+j].flags2=TheOldMap.flags2;
        TheMaps[i*MAPSCRS+j].flags3=TheOldMap.flags3;
        for (int k=0; k<6; k++) {
          TheMaps[i*MAPSCRS+j].layermap[k]=0;
          TheMaps[i*MAPSCRS+j].layerscreen[k]=0;
        }
        for (int k=0; k<32; k++) {
          TheMaps[i*MAPSCRS+j].extra[k]=(k<11)?TheOldMap.extra[k]:0;
        }
        for (int k=0; k<20; k++) {
          TheMaps[i*MAPSCRS+j].secret[k]=TheOldMap.secret[k];
        }
        for (int k=0; k<(16*11); k++) {
          TheMaps[i*MAPSCRS+j].data[k]=TheOldMap.data[k];
          TheMaps[i*MAPSCRS+j].sflag[k]=TheOldMap.sflag[k];
        }
      }
    }
  } else if (((header->zelda_version == 0x192)&&(header->build<98))) {
    mapscr192b97   TheOldMap;

    for(int i=0; i<header->map_count && i<MAXMAPS; i++) {
      for (int j=0; j<MAPSCRS; j++) {
        if(!pfread(&TheOldMap,sizeof(mapscr192b97),f)) {
          goto invalid;
        }
        TheMaps[i*MAPSCRS+j].valid=TheOldMap.valid;
        TheMaps[i*MAPSCRS+j].guy=TheOldMap.guy;
        TheMaps[i*MAPSCRS+j].str=TheOldMap.str;
        TheMaps[i*MAPSCRS+j].room=TheOldMap.room;
        TheMaps[i*MAPSCRS+j].item=TheOldMap.item;
        TheMaps[i*MAPSCRS+j]._FOO1_=TheOldMap._FOO1_;
        TheMaps[i*MAPSCRS+j].warptype=TheOldMap.warptype;
        TheMaps[i*MAPSCRS+j]._FOO2_=TheOldMap._FOO2_;
        TheMaps[i*MAPSCRS+j].warpx=TheOldMap.warpx;
        TheMaps[i*MAPSCRS+j].warpy=TheOldMap.warpy;
        TheMaps[i*MAPSCRS+j].stairx=TheOldMap.stairx;
        TheMaps[i*MAPSCRS+j].stairy=TheOldMap.stairy;
        TheMaps[i*MAPSCRS+j].itemx=TheOldMap.itemx;
        TheMaps[i*MAPSCRS+j].itemy=TheOldMap.itemy;
        TheMaps[i*MAPSCRS+j].color=TheOldMap.color;
        TheMaps[i*MAPSCRS+j].enemyflags=TheOldMap.enemyflags;
        for (int k=0; k<4; k++) {
          TheMaps[i*MAPSCRS+j].door[k]=TheOldMap.door[k];
        }
        TheMaps[i*MAPSCRS+j].warpdmap=TheOldMap.warpdmap;
        TheMaps[i*MAPSCRS+j].warpscr=TheOldMap.warpscr;
        TheMaps[i*MAPSCRS+j].exitdir=TheOldMap.exitdir;
        TheMaps[i*MAPSCRS+j]._FOO3_=TheOldMap._FOO3_;
        for (int k=0; k<10; k++) {
          TheMaps[i*MAPSCRS+j].enemy[k]=TheOldMap.enemy[k];
        }
        TheMaps[i*MAPSCRS+j].pattern=TheOldMap.pattern;
        TheMaps[i*MAPSCRS+j].warptype2=TheOldMap.warptype2;
        TheMaps[i*MAPSCRS+j].warpx2=TheOldMap.warpx2;
        TheMaps[i*MAPSCRS+j].warpy2=TheOldMap.warpy2;
        for (int k=0; k<4; k++) {
          TheMaps[i*MAPSCRS+j].path[k]=TheOldMap.path[k];
        }
        TheMaps[i*MAPSCRS+j].warpscr2=TheOldMap.warpscr2;
        TheMaps[i*MAPSCRS+j].warpdmap2=TheOldMap.warpdmap2;
        TheMaps[i*MAPSCRS+j].under=TheOldMap.under;
        TheMaps[i*MAPSCRS+j].cpage=TheOldMap.cpage;
        TheMaps[i*MAPSCRS+j]._FOO4_=TheOldMap._FOO4_;
        TheMaps[i*MAPSCRS+j].catchall=TheOldMap.catchall;
        TheMaps[i*MAPSCRS+j].flags=TheOldMap.flags;
        TheMaps[i*MAPSCRS+j].flags2=TheOldMap.flags2;
        TheMaps[i*MAPSCRS+j].flags3=TheOldMap.flags3;
        for (int k=0; k<6; k++) {
          TheMaps[i*MAPSCRS+j].layermap[k]=0;
          TheMaps[i*MAPSCRS+j].layerscreen[k]=0;
        }
        TheMaps[i*MAPSCRS+j].layermap[2]=TheOldMap.groundovermap;
        TheMaps[i*MAPSCRS+j].layerscreen[2]=TheOldMap.groundoverscreen;
        TheMaps[i*MAPSCRS+j].layermap[4]=TheOldMap.skyovermap;
        TheMaps[i*MAPSCRS+j].layerscreen[4]=TheOldMap.skyoverscreen;
        for (int k=0; k<32; k++) {
          TheMaps[i*MAPSCRS+j].extra[k]=(k<11)?TheOldMap.extra[k]:0;
        }
        for (int k=0; k<20; k++) {
          TheMaps[i*MAPSCRS+j].secret[k]=TheOldMap.secret[k];
        }
        for (int k=0; k<(16*11); k++) {
          TheMaps[i*MAPSCRS+j].data[k]=TheOldMap.data[k];
          TheMaps[i*MAPSCRS+j].sflag[k]=TheOldMap.sflag[k];
        }
        for (int k=0; k<(16*11); k++) {
          TheMaps[i*MAPSCRS+j].sflag2[k]=0;
        }
      }
    }
  } else {
//  alert("18b",NULL,NULL,"O&K",NULL,'k',0);
    for(int i=0; i<header->map_count && i<MAXMAPS; i++) {
      for (int j=0; j<MAPSCRS; j++) {
        if(!pfread(TheMaps+(i*MAPSCRS)+j,sizeof(mapscr),f)) {
          goto invalid;
        }
      }
    }

  if ((header->zelda_version < 0x192)||
     ((header->zelda_version == 0x192)&&(header->build<111))) {
    for(int i=0; i<header->map_count && i<MAXMAPS; i++) {
      for (int j=0; j<MAPSCRS; j++) {
        for (int k=0; k<10; k++) {
          if (TheMaps[i*MAPSCRS+j].enemy[k]>eMANHAN) {
            TheMaps[i*MAPSCRS+j].enemy[k]=TheMaps[i*MAPSCRS+j].enemy[k]+1;
          }
          if (TheMaps[i*MAPSCRS+j].enemy[k]>eDIG3) {
            TheMaps[i*MAPSCRS+j].enemy[k]=TheMaps[i*MAPSCRS+j].enemy[k]+1;
          }
          if (TheMaps[i*MAPSCRS+j].enemy[k]>eBATROBEKING) {
            TheMaps[i*MAPSCRS+j].enemy[k]=TheMaps[i*MAPSCRS+j].enemy[k]+1;
          }
        }
      }
    }
  }
/*
    for(int i=0; i<header->map_count && i<MAXMAPS; i++)
      if(!pfread(TheMaps+(i*MAPSCRS),sizeof(mapscr)*MAPSCRS,f)) {
        goto invalid;
      }
*/
  }


  // combos

//  alert("19",NULL,NULL,"O&K",NULL,'k',0);
  if(header->zelda_version < 0x174)
  {
    if(!pfread(combobuf,sizeof(newcombo)*1024,f)) {
      goto invalid;
    }

    for(int i=1024; i<2048; i++)
      clear_combo(i);
  }
  else if (header->zelda_version < 0x191) //doesn't have the
  {                                       //expanded combo space
//  alert("19b",NULL,NULL,"O&K",NULL,'k',0);
    oldcombo oldcombobuf[2048];
    if(!pfread(oldcombobuf,sizeof(oldcombo)*2048,f)) {
      goto invalid;
    }
//    for(int tmpcounter=0; tmpcounter<MAXCOMBOS; tmpcounter++)
      clear_combos();

    for (int tmpcounter=0; tmpcounter<2048; tmpcounter++) {
      combobuf[tmpcounter].tile=oldcombobuf[tmpcounter].tile;
      combobuf[tmpcounter].flip=oldcombobuf[tmpcounter].flip;
      combobuf[tmpcounter].walk=oldcombobuf[tmpcounter].walk;
      combobuf[tmpcounter].type=oldcombobuf[tmpcounter].type;
      combobuf[tmpcounter].csets=oldcombobuf[tmpcounter].csets;
      combobuf[tmpcounter].foo=oldcombobuf[tmpcounter].foo;
      combobuf[tmpcounter].frames=0;
      combobuf[tmpcounter].speed=0;
    }

  }
  else
  {
//  alert("19c",NULL,NULL,"O&K",NULL,'k',0);
    word combos_used;
//  alert("19d",NULL,NULL,"O&K",NULL,'k',0);
//    for(int tmpcounter=0; tmpcounter<MAXCOMBOS; tmpcounter++)
      clear_combos();
//  alert("19e",NULL,NULL,"O&K",NULL,'k',0);
    if(!pfread(&combos_used,sizeof(short),f))
      goto invalid;
//  alert("19f",NULL,NULL,"O&K",NULL,'k',0);
    if(!pfread(combobuf,sizeof(newcombo)*combos_used,f))
      goto invalid;
  }

  if (header->zelda_version < 0x192) {
    for(int tmpcounter=0; tmpcounter<MAXCOMBOS; tmpcounter++) {
      combobuf[tmpcounter].frames=0;
      combobuf[tmpcounter].speed=0;
    }
  }
  // color data
//  alert("20",NULL,NULL,"O&K",NULL,'k',0);
  if ((header->zelda_version < 0x192)||
     ((header->zelda_version == 0x192)&&(header->build<73))) {
    if(!pfread(colordata,oldpsTOTAL,f)) {
      goto invalid;
    }
          memcpy(colordata+(newpoSPRITE*48), colordata+(oldpoSPRITE*48), 30*16*3);
          memset(colordata+(oldpoSPRITE*48), 0, ((newpoSPRITE-oldpoSPRITE)*48));
          memcpy(colordata+((newpoSPRITE+11)*48), colordata+((newpoSPRITE+10)*48), 48);
          memcpy(colordata+((newpoSPRITE+10)*48), colordata+((newpoSPRITE+9)*48), 48);
          memcpy(colordata+((newpoSPRITE+9)*48), colordata+((newpoSPRITE+8)*48), 48);
          memset(colordata+((newpoSPRITE+8)*48), 0, 48);
/*
    for (int x=0; x<30; x++) {
      for (int y=0; y<16; y++) {
        for (int z=0; z<3; z++) {
          colordata[(newpoSPRITE*3)+(x*48)+(y*3)+z]=
          colordata[(oldpoSPRITE*3)+(x*48)+(y*3)+z];
          colordata[(oldpoSPRITE*3)+(x*48)+(y*3)+z]=0;
        }
      }
    }
*/
  } else {
    if(!pfread(colordata,newpsTOTAL,f)) {
      goto invalid;
    }
  }

  if ((header->zelda_version < 0x192)||
     ((header->zelda_version == 0x192)&&(header->build<76))) {
    init_palnames();
  } else {
    if(!pfread(palnames,MAXLEVELS*PALNAMESIZE,f)) {
      goto invalid;
    }
  }

  // tiles

//  alert("21",NULL,NULL,"O&K",NULL,'k',0);
  init_tiles();
  if(header->data_flags[ZQ_TILES])
  {
    clear_tiles();
//  alert("22",NULL,NULL,"O&K",NULL,'k',0);
    if(header->zelda_version < 0x174)
    {
      if(!pfread(tilebuf,OLDTILE_SIZE,f)) {
        goto invalid;
      }
    }
    else if(header->zelda_version < 0x191) //doesn't have the
    {                                      //expanded tile space
//  alert("22b",NULL,NULL,"O&K",NULL,'k',0);
      if(!pfread(tilebuf,OLDTILE_SIZE2,f)) {
        goto invalid;
      }
    }
    else
    {
//  alert("22c",NULL,NULL,"O&K",NULL,'k',0);
      short tiles_used;
      if(!pfread(&tiles_used,sizeof(word),f))
        goto invalid;
//  alert("22d",NULL,NULL,"O&K",NULL,'k',0);
      if(!pfread(tilebuf,dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)),f))
        goto invalid;
    }
  }


  // midis

//  alert("23",NULL,NULL,"O&K",NULL,'k',0);
  for(int i=0; i<MAXMIDIS; i++)
  {
//   alert("23b",NULL,NULL,"O&K",NULL,'k',0);
    reset_midi(midis+i);
//    alert("23c",NULL,NULL,"O&K",NULL,'k',0);
    if(get_bit(header->data_flags+ZQ_MIDIS,i))
    {
//    alert("23d",NULL,NULL,"O&K",NULL,'k',0);
      if(!pfread(midis+i,sizeof(music),f) ||
         !(midis[i].midi=read_midi(f)) ) {
        goto invalid;
      }
    }
  }

  // cheat codes
//  alert("24",NULL,NULL,"O&K",NULL,'k',0);
  memset(&zcheats, 0, sizeof(ZCHEATS));
  if(header->data_flags[ZQ_CHEATS])
  {
//  alert("24b",NULL,NULL,"O&K",NULL,'k',0);
    if(!pfread(&zcheats, sizeof(ZCHEATS), f)) {
//  alert("24c",NULL,NULL,"O&K",NULL,'k',0);
      goto invalid;
    }
  }

  if ((header->zelda_version < 0x192)||
     ((header->zelda_version == 0x192)&&(header->build<27))) {
    memset(&zinit, 0, sizeof(zinitdata));
    zinit.shield=1;
    zinit.hc=3;
    zinit.start_heart=3;
    zinit.cont_heart=3;
    zinit.max_bombs=8;
  } else {
    if(!pfread(&zinit, sizeof(zinitdata), f)) {
      goto invalid;
    }
  }

  if ((header->zelda_version < 0x192)||
     ((header->zelda_version == 0x192)&&(header->build<50))) {
    zinit.swordhearts[0]=0;
    zinit.swordhearts[1]=5;
    zinit.swordhearts[2]=12;
    zinit.swordhearts[3]=21;
  }

  if ((header->zelda_version < 0x192)||
     ((header->zelda_version == 0x192)&&(header->build<51))) {
    zinit.lastmap=0;
    zinit.lastscreen=0;
  }

  if ((header->zelda_version < 0x192)||
     ((header->zelda_version == 0x192)&&(header->build<68))) {
    zinit.maxmagic=0;
    zinit.magic=0;
    set_bit(zinit.misc,idM_DOUBLEMAGIC,0);
  }

  // check data

//  alert("25",NULL,NULL,"O&K",NULL,'k',0);
  pack_fclose(f);
  if(!oldquest)
    delete_file(tmpfilename);
  return qe_OK;

invalid:
  pack_fclose(f);
  if(!oldquest)
    delete_file(tmpfilename);
  return qe_invalid;
}


/*** end of qst.cc ***/


