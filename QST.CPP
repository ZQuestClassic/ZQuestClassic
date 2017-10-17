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
#include "zc_alleg.h"

#include "zdefs.h"
#include "colors.h"
#include "tiles.h"
#include "zsys.h"
#include "qst.h"
#include "zquest.h"
#include "defdata.h"
#include "font.h"

// extern bool debug;
extern mapscr       *TheMaps;
extern MsgStr       *MsgStrings;
extern DoorComboSet *DoorComboSets;
extern dmap         *DMaps;
extern newcombo     *combobuf;
extern byte         *colordata;
extern byte         *tilebuf;
extern byte         *trashbuf;
extern itemdata     *itemsbuf;
extern wpndata      *wpnsbuf;
extern guydata      *guysbuf;
extern ZCHEATS      zcheats;
extern zinitdata    zinit;
extern char         palnames[256][17];
extern int          memrequested;
extern char         *byte_conversion(int number, int format);
extern char         *byte_conversion2(int number1, int number2, int format1, int format2);
//enum { qe_OK, qe_notfound, qe_invalid, qe_version, qe_obsolete,
//       qe_missing, qe_internal, qe_pwd, qe_match, qe_minver };

bool keepit=true;
char *qst_error[] =
{
  "OK","File not found","Invalid quest file",
  "Version not supported","Obsolete version",
  "Missing new data"  ,                                     /* but let it pass in ZQuest */
  "Internal error occurred", "Invalid password",
  "Doesn't match saved game", "New quest version; please restart game",
  "Out of memory", "File Debug Mode"
};

char *VerStr(int version)
{
  static char ver_str[12];
  sprintf(ver_str,"v%d.%02X",version>>8,version&0xFF);
  return ver_str;
}

char *byte_conversion(int number, int format)
{
  static char num_str[40];
  if (format==-1)                                           //auto
  {
    format=1;                                               //bytes
    if (number>1024)
    {
      format=2;                                             //kilobytes
    }
    if (number>1024*1024)
    {
      format=3;                                             //megabytes
    }
    if (number>1024*1024*1024)
    {
      format=4;                                             //gigabytes (dude, what are you doing?)
    }
  }
  switch (format)
  {
    case 1:                                                 //bytes
      sprintf(num_str,"%db",number);
      break;
    case 2:                                                 //kilobytes
      sprintf(num_str,"%.2fk",float(number)/1024);
      break;
    case 3:                                                 //megabytes
      sprintf(num_str,"%.2fM",float(number)/(1024*1024));
      break;
    case 4:                                                 //gigabytes
      sprintf(num_str,"%.2fG",float(number)/(1024*1024*1024));
      break;
    default:
      exit(1);
      break;
  }
  return num_str;
}

char *byte_conversion2(int number1, int number2, int format1, int format2)
{
  static char num_str1[40];
  static char num_str2[40];
  static char num_str[80];
  if (format1==-1)                                          //auto
  {
    format1=1;                                              //bytes
    if (number1>1024)
    {
      format1=2;                                            //kilobytes
    }
    if (number1>1024*1024)
    {
      format1=3;                                            //megabytes
    }
    if (number1>1024*1024*1024)
    {
      format1=4;                                            //gigabytes (dude, what are you doing?)
    }
  }
  if (format2==-1)                                          //auto
  {
    format2=1;                                              //bytes
    if (number2>1024)
    {
      format2=2;                                            //kilobytes
    }
    if (number2>1024*1024)
    {
      format2=3;                                            //megabytes
    }
    if (number2>1024*1024*1024)
    {
      format2=4;                                            //gigabytes (dude, what are you doing?)
    }
  }
  switch (format1)
  {
    case 1:                                                 //bytes
      sprintf(num_str1,"%db",number1);
      break;
    case 2:                                                 //kilobytes
      sprintf(num_str1,"%.2fk",float(number1)/1024);
      break;
    case 3:                                                 //megabytes
      sprintf(num_str1,"%.2fM",float(number1)/(1024*1024));
      break;
    case 4:                                                 //gigabytes
      sprintf(num_str1,"%.2fG",float(number1)/(1024*1024*1024));
      break;
    default:
      exit(1);
      break;
  }
  switch (format2)
  {
    case 1:                                                 //bytes
      sprintf(num_str2,"%db",number2);
      break;
    case 2:                                                 //kilobytes
      sprintf(num_str2,"%.2fk",float(number2)/1024);
      break;
    case 3:                                                 //megabytes
      sprintf(num_str2,"%.2fM",float(number2)/(1024*1024));
      break;
    case 4:                                                 //gigabytes
      sprintf(num_str2,"%.2fG",float(number2)/(1024*1024*1024));
      break;
    default:
      exit(1);
      break;
  }
  sprintf(num_str, "%s/%s", num_str1, num_str2);
  return num_str;
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

int get_version_and_build(PACKFILE *f, word *version, word *build)
{
  int ret;
  *version=0;
  *build=0;
  byte temp_map_count=map_count;
  byte temp_midi_flags[MIDIFLAGS_SIZE];
  memcpy(temp_midi_flags, midi_flags, MIDIFLAGS_SIZE);

  zquestheader tempheader;

  if (!f)
  {
    return qe_invalid;
  }

  ret=readheader(f, &tempheader, true);
  if (ret)
  {
    return ret;
  }

  map_count=temp_map_count;
  memcpy(midi_flags, temp_midi_flags, MIDIFLAGS_SIZE);
  *version=tempheader.zelda_version;
  *build=tempheader.build;
  return 0;
}


bool find_section(PACKFILE *f, long section_id_requested)
{

  if (!f)
  {
    return false;
  }

  long section_id_read;
  bool catchup=false;
  word dummy;
  byte tempbyte;
  char tempbuf[65536];


  switch (section_id_requested)
  {
    case ID_RULES:
    case ID_STRINGS:
    case ID_MISC:
    case ID_TILES:
    case ID_COMBOS:
    case ID_CSETS:
    case ID_MAPS:
    case ID_DMAPS:
    case ID_DOORS:
    case ID_ITEMS:
    case ID_WEAPONS:
    case ID_COLORS:
    case ID_ICONS:
    case ID_INITDATA:
    case ID_GUYS:
    case ID_MIDIS:
    case ID_CHEATS:
      break;
    default:
	  return false;
      break;
  }

  dword section_size;

  //section id
  if(!p_mgetl(&section_id_read,f,true))
  {
    return false;
  }

  while (!pack_feof(f))
  {
    switch (section_id_read)
    {
      case ID_RULES:
      case ID_STRINGS:
      case ID_MISC:
      case ID_TILES:
      case ID_COMBOS:
      case ID_CSETS:
      case ID_MAPS:
      case ID_DMAPS:
      case ID_DOORS:
      case ID_ITEMS:
      case ID_WEAPONS:
      case ID_COLORS:
      case ID_ICONS:
      case ID_INITDATA:
      case ID_GUYS:
      case ID_MIDIS:
      case ID_CHEATS:
        catchup=false;
        break;
      default:
        break;
    }


    while (catchup)
    {
      //section id
      section_id_read=(section_id_read<<8);
      if(!p_getc(&tempbyte,f,true))
      {
        return false;
      }
      section_id_read+=tempbyte;
    }

    if (section_id_read==section_id_requested)
    {
      return true;
    }
    else
    {
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
      if(!p_igetl(&section_size,f,true))
      {
        return false;
      }
      //pack_fseek(f, section_size);
      while(section_size>65535)
      {
        pfread(tempbuf,65535,f,true);
        tempbuf[65535]=0;
        section_size-=65535;
      }
      if (section_size>0)
      {
        pfread(tempbuf,section_size,f,true);
        tempbuf[section_size]=0;
      }
    }
    //section id
    if(!p_mgetl(&section_id_read,f,true))
    {
      return false;
    }
  }
  return false;
}





bool valid_zqt(PACKFILE *f)
{

  word tiles_used;
  word combos_used;
  //open the file
  //PACKFILE *f = pack_fopen(path, F_READ_PACKED);
  if(!f)
    return false;

  //for now, everything else is valid
  return true;

  short version;
  byte build;

  //read the version and make sure it worked
  if(!p_igetw(&version,f,true))
  {
    goto error;
  }

  //read the build and make sure it worked
  if(!p_getc(&build,f,true))
    goto error;

  //read the tile info and make sure it worked
  if(!p_igetw(&tiles_used,f,true))
  {
    goto error;
  }

  for (int i=0; i<tiles_used; i++)
  {
    if(!pfread(trashbuf,SINGLE_TILE_SIZE,f,true))
    {
      goto error;
    }
  }

  //read the combo info and make sure it worked
  if(!p_igetw(&combos_used,f,true))
  {
    goto error;
  }
  for (int i=0; i<combos_used; i++)
  {
    if(!pfread(trashbuf,sizeof(newcombo),f,true))
    {
      goto error;
    }
  }

  //read the palette info and make sure it worked
  for (int i=0; i<48; i++)
  {
	  if(!pfread(trashbuf,newpdTOTAL,f,true))
	  {
	    goto error;
	  }
  }
  if(!pfread(trashbuf,sizeof(palcycle)*256*3,f,true))
  {
    goto error;
  }
  for (int i=0; i<MAXLEVELS; i++)
  {
    if(!pfread(trashbuf,PALNAMESIZE,f,true))
    {
      goto error;
    }
  }

  //read the sprite info and make sure it worked
  for (int i=0; i<MAXITEMS; i++)
  {
    if(!pfread(trashbuf,sizeof(itemdata),f,true))
    {
      goto error;
    }
  }

  for (int i=0; i<MAXWPNS; i++)
  {
    if(!pfread(trashbuf,sizeof(wpndata),f,true))
    {
      goto error;
    }
  }

  //read the triforce pieces info and make sure it worked
  for (int i=0; i<8; ++i)
  {
    if(!p_getc(&trashbuf,f,true))
    {
      goto error;
    }
  }



  //read the game icons info and make sure it worked
  for (int i=0; i<4; ++i)
  {
    if(!p_igetw(&trashbuf,f,true))
    {
      goto error;
    }
  }

  //read the misc colors info and map styles info and make sure it worked
  if(!pfread(trashbuf,sizeof(zcolors),f,true))
  {
    goto error;
  }

  //read the template screens and make sure it worked
  byte num_maps;
  if(!p_getc(&num_maps,f,true))
  {
    goto error;
  }
  for (int i=0; i<TEMPLATES; i++)
  {
    if(!pfread(trashbuf,sizeof(mapscr),f,true))
    {
      goto error;
    }
  }
  if (num_maps>1)                                           //dungeon templates
  {
    for (int i=0; i<TEMPLATES; i++)
    {
      if(!pfread(trashbuf,sizeof(mapscr),f,true))
      {
        goto error;
      }
    }
  }

  //yay!  it worked!  close the file and say everything was ok.
  pack_fclose(f);
  return true;

  error:
  pack_fclose(f);
  return false;
}

bool valid_zqt(char *filename)
{
  PACKFILE *f=NULL;
  bool isvalid;
  char deletefilename[1024];
  deletefilename[0]=0;
  int error;
  f=open_quest_file(&error, filename, deletefilename, true, false);
  if (!f)
  {
    packfile_password(NULL);
    return false;
  }

  isvalid=valid_zqt(f);
  if (deletefilename[0])
  {
    delete_file(deletefilename);
  }
  packfile_password(NULL);
  return isvalid;
}


PACKFILE *open_quest_file(int *open_error, char *filename, char *deletefilename, bool compressed, bool show_progress)
{
  char tmpbuf[L_tmpnam];
  char *tmpfilename = tmpnam(tmpbuf);
  char percent_done[30];
  int current_method=0;

  PACKFILE *f;

  // oldquest flag is set when an unencrypted qst file is suspected.
  bool oldquest = false;
  int ret;

  if (show_progress)
  {
    box_start(1, "Loading Quest", lfont, font, true);
  }
  box_out("Loading Quest...");
  box_eol();
  box_eol();
  if(compressed)
  {
    box_out("Decrypting ...");
    box_save_x();

    ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_MAX-1, strstr(filename, ".dat#")!=NULL);
    if(ret)
    {
      switch(ret)
      {
        case 1: box_out("error."); box_eol(); box_end(true); *open_error=qe_notfound;  return NULL;
        case 2: box_out("error."); box_eol(); box_end(true); *open_error=qe_internal;  return NULL;
        // be sure not to delete tmpfilename now...
      }
      if (ret==5)                                             //old encryption?
      {
        current_method++;
        sprintf(percent_done, "%d%%", (current_method*100)/ENC_METHOD_MAX);
        box_out(percent_done);
        box_load_x();
        ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_192B105, strstr(filename, ".dat#")!=NULL);
      }
      if (ret==5)                                             //old encryption?
      {
        current_method++;
        sprintf(percent_done, "%d%%", (current_method*100)/ENC_METHOD_MAX);
        box_out(percent_done);
        box_load_x();
        ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_192B104, strstr(filename, ".dat#")!=NULL);
      }
      if (ret)
      {
        oldquest = true;
      }
    }
    box_out("okay.");
    box_eol();
  }
  else
  {
    oldquest = true;
  }
  box_out("Opening...");
  f = pack_fopen(oldquest ? filename : tmpfilename, compressed ? F_READ_PACKED : F_READ);
  if(!f)
  {
    if((compressed==1)&&(errno==EDOM))
    {
      f = pack_fopen(oldquest ? filename : tmpfilename, F_READ);
    }
    if (!f)
    {
      if(!oldquest)
      {
        delete_file(tmpfilename);
      }
      box_out("error.");
      box_eol();
      box_end(true);
      *open_error=qe_invalid;
      return NULL;
    }
  }

  if(!oldquest)
  {
    sprintf(deletefilename, "%s", tmpfilename);
  }

  box_out("okay.");
  box_eol();

  return f;
}

PACKFILE *open_quest_template(zquestheader *header, char *deletefilename, bool validate)
{
  char *filename;
  PACKFILE *f=NULL;
  int open_error=0;
  deletefilename[0]=0;

  if (header->templatepath[0]==0)
  {
    filename=(char *)malloc(20);
    sprintf(filename, "qst.dat#DAT_NESQST");
  }
  else
  {
    filename=header->templatepath;
  }

  f=open_quest_file(&open_error, filename, deletefilename, true, false);
  if (!f)
  {
    return false;
  }
  if (validate&&0)
  {
    if (!valid_zqt(f))
    {
      jwin_alert("Error","Invalid Quest Template",NULL,NULL,"O&K",NULL,'k',0,lfont);
      pack_fclose(f);
      packfile_password(NULL);
      if (deletefilename[0])
      {
        delete_file(deletefilename);
      }
      return false;
    }
  }
  return f;
}

bool init_section(zquestheader *Header, long section_id, miscQdata *Misc, music *midis, bool validate)
{
  switch (section_id)
  {
    case ID_RULES:
    case ID_STRINGS:
    case ID_MISC:
    case ID_TILES:
    case ID_COMBOS:
    case ID_CSETS:
    case ID_MAPS:
    case ID_DMAPS:
    case ID_DOORS:
    case ID_ITEMS:
    case ID_WEAPONS:
    case ID_COLORS:
    case ID_ICONS:
    case ID_INITDATA:
    case ID_GUYS:
    case ID_MIDIS:
    case ID_CHEATS:
    break;
    default:
    return false;
    break;
  }

  int ret;
  word version, build;
  PACKFILE *f=NULL;

  char deletefilename[1024];
  deletefilename[0]=0;

  //why is this here?
  /*
    if(colordata==NULL)
    return false;
    */

  packfile_password(datapwd);
  f=open_quest_template(Header, deletefilename, validate);
  if (!f) //no file, nothing to delete
  {
    packfile_password(NULL);
    return false;
  }
  ret=get_version_and_build(f, &version, &build);
  if (ret||(version==0))
  {
    pack_fclose(f);
    if (deletefilename[0])
    {
      delete_file(deletefilename);
    }
    packfile_password(NULL);
    return false;
  }

  if (!find_section(f, section_id))
  {
    pack_fclose(f);
    if (deletefilename[0])
    {
      delete_file(deletefilename);
    }
    packfile_password(NULL);
    return false;
  }
  switch (section_id)
  {
    case ID_RULES:
      //rules
      ret=readrules(f, Header, true);
      break;
    case ID_STRINGS:
      //strings
      ret=readstrings(f, Header, true);
      break;
    case ID_MISC:
      //misc data
      ret=readmisc(f, Header, Misc, true);
      break;
    case ID_TILES:
      //tiles
      clear_tiles(tilebuf);
      ret=readtiles(f, tilebuf, Header, version, build, 0, NEWMAXTILES, true, true);
      break;
    case ID_COMBOS:
      //combos
      clear_combos();
      ret=readcombos(f, Header, version, build, 0, MAXCOMBOS, true);
      break;
    case ID_CSETS:
      //color data
      ret=readcolordata(f, Misc, version, build, 0, newpdTOTAL, true);
      break;
    case ID_MAPS:
      //maps
      ret=readmaps(f, Header, true);
      break;
    case ID_DMAPS:
      //dmaps
      ret=readdmaps(f, Header, version, build, 0, MAXDMAPS, true);
      break;
    case ID_DOORS:
      //door combo sets
      ret=readdoorcombosets(f, Header, true);
      break;
    case ID_ITEMS:
      //items
      ret=readitems(f, version, build, true);
      break;
    case ID_WEAPONS:
      //weapons
      ret=readweapons(f, Header, true);
      break;
    case ID_COLORS:
      break;
    case ID_ICONS:
      break;
    case ID_INITDATA:
      //initialization data
      ret=readinitdata(f, Header, true);
      break;
    case ID_GUYS:
      //guys
      ret=readguys(f, Header, true);
      break;
    case ID_MIDIS:
      //midis
      ret=readmidis(f, Header, midis, true);
      break;
    case ID_CHEATS:
      //cheat codes
      ret=readcheatcodes(f, Header, true);
      break;
    default:
      ret=-1;
      break;
  }

  pack_fclose(f);
  if (deletefilename[0])
  {
    delete_file(deletefilename);
  }
  packfile_password(NULL);
  if (!ret)
  {
    return true;
  }
  return false;
}

bool init_tiles(bool validate, zquestheader *Header)
{
  return init_section(Header, ID_TILES, NULL, NULL, validate);
}

bool init_combos(bool validate, zquestheader *Header)
{
  return init_section(Header, ID_COMBOS, NULL, NULL, validate);
}

bool init_colordata(bool validate, zquestheader *Header, miscQdata *Misc)
{
  return init_section(Header, ID_CSETS, Misc, NULL, validate);
}

bool reset_items(bool validate, zquestheader *Header)
{
  return init_section(Header, ID_ITEMS, NULL, NULL, validate);
}

bool reset_wpns(bool validate, zquestheader *Header)
{
  return init_section(Header, ID_WEAPONS, NULL, NULL, validate);
}

bool reset_mapstyles(bool validate, zquestheader *Header, miscQdata *Misc)
{
  return init_section(Header, ID_TILES, Misc, NULL, validate);
}

bool reset_doorcombosets(bool validate, zquestheader *Header)
{
  return init_section(Header, ID_DOORS, NULL, NULL, validate);
}

void get_qst_buffers()
{
  memrequested+=(sizeof(mapscr)*MAPSCRS);
  Z_message("Allocating map buffer (%s)... ", byte_conversion2(sizeof(mapscr)*MAPSCRS,memrequested,-1, -1));
  if(!(TheMaps=(mapscr*)malloc(sizeof(mapscr)*MAPSCRS)))
    Z_error("Error");
  Z_message("OK\n");                                        // Allocating map buffer...

  memrequested+=(sizeof(MsgStr)*MAXMSGS);
  Z_message("Allocating string buffer (%s)... ", byte_conversion2(sizeof(MsgStr)*MAXMSGS,memrequested,-1,-1));
  if(!(MsgStrings=(MsgStr*)malloc(sizeof(MsgStr)*MAXMSGS)))
    Z_error("Error");
  Z_message("OK\n");                                        // Allocating string buffer...

  memrequested+=(sizeof(DoorComboSet)*MAXDOORCOMBOSETS);
  Z_message("Allocating door combo buffer (%s)... ", byte_conversion2(sizeof(DoorComboSet)*MAXDOORCOMBOSETS,memrequested,-1,-1));
  if(!(DoorComboSets=(DoorComboSet*)malloc(sizeof(DoorComboSet)*MAXDOORCOMBOSETS)))
    Z_error("Error");
  Z_message("OK\n");                                        // Allocating door combo buffer...

  memrequested+=(sizeof(dmap)*MAXDMAPS);
  Z_message("Allocating dmap buffer (%s)... ", byte_conversion2(sizeof(dmap)*MAXDMAPS,memrequested,-1,-1));
  if(!(DMaps=(dmap*)malloc(sizeof(dmap)*MAXDMAPS)))
    Z_error("Error");
  Z_message("OK\n");                                        // Allocating dmap buffer...

  memrequested+=(sizeof(newcombo)*MAXCOMBOS);
  Z_message("Allocating combo buffer (%s)... ", byte_conversion2(sizeof(newcombo)*MAXCOMBOS,memrequested,-1,-1));
  if(!(combobuf=(newcombo*)malloc(sizeof(newcombo)*MAXCOMBOS)))
    Z_error("Error");
  Z_message("OK\n");                                        // Allocating combo buffer...

  memrequested+=(newpsTOTAL);
  Z_message("Allocating color data buffer (%s)... ", byte_conversion2(newpsTOTAL,memrequested,-1,-1));
  if(!(colordata=(byte*)malloc(newpsTOTAL)))
    Z_error("Error");
  Z_message("OK\n");                                        // Allocating color data buffer...

  memrequested+=(NEWTILE_SIZE2);
  Z_message("Allocating tile buffer (%s)... ", byte_conversion2(NEWTILE_SIZE2,memrequested,-1,-1));
  if(!(tilebuf=(byte*)malloc(NEWTILE_SIZE2)))
    Z_error("Error");
  Z_message("OK\n");                                        // Allocating tile buffer...

  memrequested+=(100000);
  Z_message("Allocating trash buffer (%s)... ", byte_conversion2(100000,memrequested,-1,-1));
  if(!(trashbuf=(byte*)malloc(100000)))
    Z_error("Error");
  Z_message("OK\n");                                        // Allocating trash buffer...

  memrequested+=(sizeof(itemdata)*MAXITEMS);
  Z_message("Allocating item buffer (%s)... ", byte_conversion2(sizeof(itemdata)*MAXITEMS,memrequested,-1,-1));
  if(!(itemsbuf=(itemdata*)malloc(sizeof(itemdata)*MAXITEMS)))
    Z_error("Error");
  Z_message("OK\n");                                        // Allocating item buffer...

  memrequested+=(sizeof(wpndata)*MAXWPNS);
  Z_message("Allocating weapon buffer (%s)... ", byte_conversion2(sizeof(wpndata)*MAXWPNS,memrequested,-1,-1));
  if(!(wpnsbuf=(wpndata*)malloc(sizeof(wpndata)*MAXWPNS)))
    Z_error("Error");
  Z_message("OK\n");                                        // Allocating weapon buffer...

  memrequested+=(sizeof(guydata)*MAXGUYS);
  Z_message("Allocating guy buffer (%s)... ", byte_conversion2(sizeof(guydata)*MAXGUYS,memrequested,-1,-1));
  if(!(guysbuf=(guydata*)malloc(sizeof(guydata)*MAXGUYS)))
    Z_error("Error");
  Z_message("OK\n");                                        // Allocating guy buffer...

}

bool init_palnames()
{
  if(palnames==NULL)
    return false;

  for (int x=0; x<256; x++)
  {
    switch (x)
    {
      case 0:  sprintf(palnames[x],"Overworld"); break;
      case 10: sprintf(palnames[x],"Caves"); break;
      case 11: sprintf(palnames[x],"Passageways"); break;
      default: sprintf(palnames[x],"%c",0); break;
    }
  }

  return true;
}

static void *read_block(PACKFILE *f, int size, int alloc_size, bool keepdata)
{
  void *p;

  p = malloc(MAX(size, alloc_size));
  if (!p)
  {
    return NULL;
  }

  if(!pfread(p,size,f,keepdata))
  {
    free(p);
    return NULL;
  }

  if (pack_ferror(f))
  {
    free(p);
    return NULL;
  }

  return p;
}

/* read_midi:
 *  Reads MIDI data from a datafile (this is not the same thing as the
 *  standard midi file format).
 */

static MIDI *read_midi(PACKFILE *f, bool keepdata)
{
  MIDI *m;
  int c;
  short divisions;
  int len=0;

  m = (MIDI*)malloc(sizeof(MIDI));
  if (!m)
  {
    return NULL;
  }

  for (c=0; c<MIDI_TRACKS; c++)
  {
    m->track[c].len = 0;
    m->track[c].data = NULL;
  }

  p_mgetw(&divisions,f,true);
  m->divisions=divisions;

  for (c=0; c<MIDI_TRACKS; c++)
  {
    p_mgetl(&len,f,true);
    m->track[c].len=len;
    if (m->track[c].len > 0)
    {
      m->track[c].data = (byte*)read_block(f, m->track[c].len, 0, true);
      if (!m->track[c].data)
      {
        destroy_midi(m);
        return NULL;
      }
    }
  }

  LOCK_DATA(m, sizeof(MIDI));
  for (c=0; c<MIDI_TRACKS; c++)
  {
    if (m->track[c].data)
    {
      LOCK_DATA(m->track[c].data, m->track[c].len);
    }
  }

  return m;
}

void clear_combo(int i)
{
  memset(combobuf+i,0,sizeof(newcombo));
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

void reset_midis(music *m)
{
  for(int i=0; i<MAXMIDIS; i++)
    reset_midi(m+i);
}

void reset_scr(int scr)
{
  /*
    byte *di=((byte*)TheMaps)+(scr*sizeof(mapscr));
    for(unsigned i=0; i<sizeof(mapscr); i++)
      *(di++) = 0;
    TheMaps[scr].valid=mVERSION;
  */

  byte *di=((byte*)TheMaps)+(scr*sizeof(mapscr));
  memset(di, 0, sizeof(mapscr));
  for (int i=0; i<6; i++)
  {
    TheMaps[scr].layerxsize[i]=16;
    TheMaps[scr].layerysize[i]=11;
    TheMaps[scr].layeropacity[i]=255;
  }

  TheMaps[scr].valid=mVERSION;

}

/*  For reference:

    enum { qe_OK, qe_notfound, qe_invalid, qe_version, qe_obsolete,
       qe_missing, qe_internal, qe_pwd, qe_match, qe_minver };
*/

int operator ==(DoorComboSet a, DoorComboSet b)
{
  for (int i=0; i<9; i++)
  {
    for (int j=0; j<6; j++)
    {
      if (j<4)
      {
        if (a.doorcombo_u[i][j]!=b.doorcombo_u[i][j])
        {
          return false;
        }
        if (a.doorcset_u[i][j]!=b.doorcset_u[i][j])
        {
          return false;
        }
        if (a.doorcombo_d[i][j]!=b.doorcombo_d[i][j])
        {
          return false;
        }
        if (a.doorcset_d[i][j]!=b.doorcset_d[i][j])
        {
          return false;
        }
      }
      if (a.doorcombo_l[i][j]!=b.doorcombo_l[i][j])
      {
        return false;
      }
      if (a.doorcset_l[i][j]!=b.doorcset_l[i][j])
      {
        return false;
      }
      if (a.doorcombo_r[i][j]!=b.doorcombo_r[i][j])
      {
        return false;
      }
      if (a.doorcset_r[i][j]!=b.doorcset_r[i][j])
      {
        return false;
      }
    }
    if (i<2)
    {
      if (a.flags[i]!=b.flags[i])
      {
        return false;
      }
      if (a.bombdoorcombo_u[i]!=b.bombdoorcombo_u[i])
      {
        return false;
      }
      if (a.bombdoorcset_u[i]!=b.bombdoorcset_u[i])
      {
        return false;
      }
      if (a.bombdoorcombo_d[i]!=b.bombdoorcombo_d[i])
      {
        return false;
      }
      if (a.bombdoorcset_d[i]!=b.bombdoorcset_d[i])
      {
        return false;
      }
    }
    if (i<3)
    {
      if (a.bombdoorcombo_l[i]!=b.bombdoorcombo_l[i])
      {
        return false;
      }
      if (a.bombdoorcset_l[i]!=b.bombdoorcset_l[i])
      {
        return false;
      }
      if (a.bombdoorcombo_r[i]!=b.bombdoorcombo_r[i])
      {
        return false;
      }
      if (a.bombdoorcset_r[i]!=b.bombdoorcset_r[i])
      {
        return false;
      }
    }
    if (a.walkthroughcombo[i]!=b.walkthroughcombo[i])
    {
      return false;
    }
    if (a.walkthroughcset[i]!=b.walkthroughcset[i])
    {
      return false;
    }
  }
  return true;
}

int doortranslations_u[9][4]=
{
  {37,38,53,54},
  {37,38,39,40},
  {37,38,55,56},
  {37,38,39,40},
  {37,38,53,54},
  {37,38,53,54},
  {37,38,53,54},
  {7,8,23,24},
  {7,8,41,42}
};

int doortranslations_d[9][4]=
{
  {117,118,133,134},
  {135,136,133,134},
  {119,120,133,134},
  {135,136,133,134},
  {117,118,133,134},
  {117,118,133,134},
  {117,118,133,134},
  {151,152,167,168},
  {137,138,167,168},
};

//enum {dt_pass=0, dt_lock, dt_shut, dt_boss, dt_olck, dt_osht, dt_obos, dt_wall, dt_bomb, dt_walk, dt_max};
int doortranslations_l[9][6]=
{
  {66,67,82,83,98,99},
  {66,68,82,84,98,100},
  {66,69,82,85,98,101},
  {66,68,82,84,98,100},
  {66,67,82,83,98,99},
  {66,67,82,83,98,99},
  {66,67,82,83,98,99},
  {64,65,80,81,96,97},
  {64,65,80,114,96,97},
};

int doortranslations_r[9][6]=
{

  {76,77,92,93,108,109},
  {75,77,91,93,107,109},
  {74,77,90,93,106,109},
  {75,77,91,93,107,109},
  {76,77,92,93,108,109},
  {76,77,92,93,108,109},
  {76,77,92,93,108,109},
  {78,79,94,95,110,111},
  {78,79,125,95,110,111},
};

int tdcmbdat(int map, int scr, int pos)
{
  return (TheMaps[map*MAPSCRS+TEMPLATE].data[pos]&0xFF)+((TheMaps[map*MAPSCRS+scr].old_cpage)<<8);
}

int tdcmbcset(int map, int scr, int pos)
{
  //  return TheMaps[map*MAPSCRS+TEMPLATE].cset[pos];
  return 2;
}

int MakeDoors(int map, int scr)
{
  if(!(TheMaps[map*MAPSCRS+scr].valid&mVALID))
  {
    return 0;
  }
  DoorComboSet tempdcs;
  memset(&tempdcs, 0, sizeof(DoorComboSet));
  //up
  for (int i=0; i<9; i++)
  {
    for (int j=0; j<4; j++)
    {
      tempdcs.doorcombo_u[i][j]=tdcmbdat(map,scr,doortranslations_u[i][j]);
      tempdcs.doorcset_u[i][j]=tdcmbcset(map,scr,doortranslations_u[i][j]);
    }
  }
  tempdcs.bombdoorcombo_u[0]=tdcmbdat(map,scr,57);
  tempdcs.bombdoorcset_u[0]=tdcmbcset(map,scr,57);
  tempdcs.bombdoorcombo_u[1]=tdcmbdat(map,scr,58);
  tempdcs.bombdoorcset_u[1]=tdcmbcset(map,scr,58);
  tempdcs.walkthroughcombo[0]=tdcmbdat(map,scr,34);
  tempdcs.walkthroughcset[0]=tdcmbdat(map,scr,34);

  //down
  for (int i=0; i<9; i++)
  {
    for (int j=0; j<4; j++)
    {
      tempdcs.doorcombo_d[i][j]=tdcmbdat(map,scr,doortranslations_d[i][j]);
      tempdcs.doorcset_d[i][j]=tdcmbcset(map,scr,doortranslations_d[i][j]);
    }
  }
  tempdcs.bombdoorcombo_d[0]=tdcmbdat(map,scr,121);

  tempdcs.bombdoorcset_d[0]=tdcmbcset(map,scr,121);
  tempdcs.bombdoorcombo_d[1]=tdcmbdat(map,scr,122);
  tempdcs.bombdoorcset_d[1]=tdcmbcset(map,scr,122);
  tempdcs.walkthroughcombo[1]=tdcmbdat(map,scr,34);
  tempdcs.walkthroughcset[1]=tdcmbdat(map,scr,34);

  //left
  //        TheMaps[i*MAPSCRS+j].warpdmap=TheOldMap.warpdmap;
  for (int i=0; i<9; i++)
  {
    for (int j=0; j<6; j++)
    {
      tempdcs.doorcombo_l[i][j]=tdcmbdat(map,scr,doortranslations_l[i][j]);
      tempdcs.doorcset_l[i][j]=tdcmbcset(map,scr,doortranslations_l[i][j]);
    }
  }

  for (int j=0; j>6; j++)
  {
    if ((j!=2)&&(j!=3))
    {
      tempdcs.doorcombo_l[dt_bomb][j]=TheMaps[map*MAPSCRS+scr].data[doortranslations_l[dt_bomb][j]];
      tempdcs.doorcset_l[dt_bomb][j]=TheMaps[map*MAPSCRS+scr].cset[doortranslations_l[dt_bomb][j]];
    }
  }

  tempdcs.bombdoorcombo_l[0]=0;
  tempdcs.bombdoorcset_l[0]=tdcmbcset(map,scr,115);
  tempdcs.bombdoorcombo_l[1]=tdcmbdat(map,scr,115);
  tempdcs.bombdoorcset_l[1]=tdcmbcset(map,scr,115);
  tempdcs.bombdoorcombo_l[2]=0;
  tempdcs.bombdoorcset_l[2]=tdcmbcset(map,scr,115);
  tempdcs.walkthroughcombo[2]=tdcmbdat(map,scr,34);
  tempdcs.walkthroughcset[2]=tdcmbdat(map,scr,34);

  //right
  for (int i=0; i<9; i++)
  {
    for (int j=0; j<6; j++)
    {
      tempdcs.doorcombo_r[i][j]=tdcmbdat(map,scr,doortranslations_r[i][j]);
      tempdcs.doorcset_r[i][j]=tdcmbcset(map,scr,doortranslations_r[i][j]);
    }
  }

  for (int j=0; j>6; j++)
  {
    if ((j!=2)&&(j!=3))
    {
      tempdcs.doorcombo_r[dt_bomb][j]=TheMaps[map*MAPSCRS+scr].data[doortranslations_r[dt_bomb][j]];
      tempdcs.doorcset_r[dt_bomb][j]=TheMaps[map*MAPSCRS+scr].cset[doortranslations_r[dt_bomb][j]];
    }
  }

  tempdcs.bombdoorcombo_r[0]=0;
  tempdcs.bombdoorcset_r[0]=tdcmbcset(map,scr,124);
  tempdcs.bombdoorcombo_r[1]=tdcmbdat(map,scr,124);
  tempdcs.bombdoorcset_r[1]=tdcmbcset(map,scr,124);
  tempdcs.bombdoorcombo_r[2]=0;
  tempdcs.bombdoorcset_r[2]=tdcmbcset(map,scr,124);
  tempdcs.walkthroughcombo[3]=tdcmbdat(map,scr,34);
  tempdcs.walkthroughcset[3]=tdcmbdat(map,scr,34);

  int i;
  for (i=0; i<door_combo_set_count; i++)
  {
    if (DoorComboSets[i]==tempdcs)
    {
      break;
    }
  }
  if (i==door_combo_set_count)
  {
    DoorComboSets[i]=tempdcs;
    sprintf(DoorComboSets[i].name, "Door Combo Set %d", i);
    ++door_combo_set_count;
  }
  return i;
  /*
  doorcombo_u[9][4];
  doorcset_u[9][4];
  doorcombo_d[9][4];
  doorcset_d[9][4];
  doorcombo_l[9][6];
  doorcset_l[9][6];
  doorcombo_r[9][6];
  doorcset_r[9][6];
  bombdoorcombo_u[2];
  bombdoorcset_u[2];
  bombdoorcombo_d[2];
  bombdoorcset_d[2];
  bombdoorcombo_l[3];
  bombdoorcset_l[3];
  bombdoorcombo_r[3];
  bombdoorcset_r[3];
  walkthroughcombo[4];
  walkthroughcset[4];
  */
}

inline int tcmbdat2(int map, int scr, int pos)
{
  return (TheMaps[map*MAPSCRS+TEMPLATE2].data[pos]&0xFF)+((TheMaps[map*MAPSCRS+scr].old_cpage)<<8);
}

inline int tcmbcset2(int map, int pos)
{

  return TheMaps[map*MAPSCRS+TEMPLATE2].cset[pos];
}

inline int tcmbflag2(int map, int pos)
{
  return TheMaps[map*MAPSCRS+TEMPLATE2].sflag[pos];
}

//int readheader(PACKFILE *f, zquestheader *header, char *filename)
int readheader(PACKFILE *f, zquestheader *header, bool keepdata)
{
  int dummy;
  zquestheader tempheader;
  memcpy(&tempheader, header, sizeof(tempheader));
  char dummybuf[80];
  byte temp_map_count;
  byte temp_midi_flags[MIDIFLAGS_SIZE];

  memset(temp_midi_flags, 0, MIDIFLAGS_SIZE);

  if(!pfread(tempheader.id_str,sizeof(tempheader.id_str),f,true))      // first read old header
  {
    return qe_invalid;
  }
  // check header
  if(strcmp(tempheader.id_str,QH_NEWIDSTR))
  {
    if(strcmp(tempheader.id_str,QH_IDSTR))
    {
      return qe_invalid;
    }
  }

  if(!strcmp(tempheader.id_str,QH_IDSTR))                      //pre-1.93 version
  {
    byte padding;
    if(!p_getc(&padding,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&tempheader.zelda_version,f,true))
    {
      return qe_invalid;
    }
    if(tempheader.zelda_version > ZELDA_VERSION)
    {
      return qe_version;
    }
    if(strcmp(tempheader.id_str,QH_IDSTR))
    {
      return qe_invalid;
    }
    if(bad_version(tempheader.zelda_version))
    {
      return qe_obsolete;
    }
    if(!p_igetw(&tempheader.internal,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempheader.quest_number,f,true))
    {
      return qe_invalid;
    }
    if(!pfread(&quest_rules[0],2,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_map_count,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempheader.old_str_count,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempheader.data_flags[ZQ_TILES],f,true))
    {
      return qe_invalid;
    }
    if(!pfread(temp_midi_flags,4,f,true))
    {
     return qe_invalid;
    }
    if(!p_getc(&tempheader.data_flags[ZQ_CHEATS2],f,true))
    {
      return qe_invalid;
    }
    if(!pfread(dummybuf,14,f,true))
    {
      return qe_invalid;
    }
    if(!pfread(&quest_rules[2],2,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&dummybuf,f,true))
    {
      return qe_invalid;
    }
    if(!pfread(tempheader.version,sizeof(tempheader.version),f,true))
    {
      return qe_invalid;
    }
    if(!pfread(tempheader.title,sizeof(tempheader.title),f,true))
    {
      return qe_invalid;
    }
    if(!pfread(tempheader.author,sizeof(tempheader.author),f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&padding,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&tempheader.pwdkey,f,true))
    {
      return qe_invalid;
    }
    if(!pfread(tempheader.password,sizeof(tempheader.password),f,true))
    {
      return qe_invalid;
    }
    if(tempheader.zelda_version < 0x177)                       // lacks new header stuff...
    {
      memset(tempheader.minver,0,20);                          //   char minver[9], byte build
    }                                                       //   byte foo[10]
    else
    {
      if(!pfread(tempheader.minver,sizeof(tempheader.minver),f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&tempheader.build,f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&tempheader.use_keyfile,f,true))
      {
        return qe_invalid;
      }
      if(!pfread(dummybuf,9,f,true))
      {
        return qe_invalid;
      }
    }                                                       // starting at minver
    if(tempheader.zelda_version < 0x187)                       // lacks newer header stuff...
    {
      memset(&quest_rules[4],0,16);                          //   word rules3..rules10
    }
    else
    {
      if(!pfread(&quest_rules[4],16,f,true))                      // read new header additions
      {
        return qe_invalid;                                  // starting at rules3
      }
    }
    if ((tempheader.zelda_version < 0x192)||
      ((tempheader.zelda_version == 0x192)&&(tempheader.build<149)))
    {
      set_bit(quest_rules,qr_BRKNSHLDTILES,(get_bit(quest_rules,qr_BRKBLSHLDS)));
      set_bit(quest_rules,qr_BRKBLSHLDS,1);
    }

    if(tempheader.zelda_version >= 0x192)                       //  lacks newer header stuff...
    {
      byte *mf=temp_midi_flags;
      if ((tempheader.zelda_version == 0x192)&&(tempheader.build<178))
      {
        mf=(byte*)dummybuf;
      }
      if(!pfread(mf,32,f,true))                  // read new header additions
      {
        return qe_invalid;                                  // starting at foo2
      }
      if(!pfread(dummybuf,18,f,true))                        // read new header additions
      {
        return qe_invalid;                                  // starting at foo2
      }
    }
    if ((tempheader.zelda_version < 0x192)||
      ((tempheader.zelda_version == 0x192)&&(tempheader.build<145)))
    {
      memset(tempheader.templatepath,0,280);
    }
    else
    {
      if(!pfread(tempheader.templatepath,280,f,true))               // read templatepath
      {
        return qe_invalid;
      }
    }
    if ((tempheader.zelda_version < 0x192)||
      ((tempheader.zelda_version == 0x192)&&(tempheader.build<186)))
    {
      tempheader.use_keyfile=0;
    }
  }
  else
  {
    //section id
    if(!p_mgetl(&dummy,f,true))
    {
      return qe_invalid;
    }

    //section version info
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }

    //section size
    if(!p_igetl(&dummy,f,true))
    {
      return qe_invalid;
    }

    //finally...  section data
    if(!p_igetw(&tempheader.zelda_version,f,true))
    {
      return qe_invalid;
    }

    //do some quick checking...
    if(tempheader.zelda_version > ZELDA_VERSION)
    {
      return qe_version;
    }
    if(strcmp(tempheader.id_str,QH_NEWIDSTR))
    {
      return qe_invalid;
    }
    if(bad_version(tempheader.zelda_version))
    {
      return qe_obsolete;
    }

    if(!p_getc(&tempheader.build,f,true))
    {

      return qe_invalid;
    }
    if(!pfread(tempheader.password,sizeof(tempheader.password),f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&tempheader.pwdkey,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&tempheader.internal,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempheader.quest_number,f,true))
    {
      return qe_invalid;
    }
    if(!pfread(tempheader.version,sizeof(tempheader.version),f,true))
    {
      return qe_invalid;
    }
    if(!pfread(tempheader.minver,sizeof(tempheader.minver),f,true))
    {
      return qe_invalid;
    }
    if(!pfread(tempheader.title,sizeof(tempheader.title),f,true))
    {
      return qe_invalid;
    }
    if(!pfread(tempheader.author,sizeof(tempheader.author),f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempheader.use_keyfile,f,true))
    {
      return qe_invalid;
    }
/*
    if(!pfread(tempheader.data_flags,sizeof(tempheader.data_flags),f,true))
    {
      return qe_invalid;
    }
*/
    if(!p_getc(&tempheader.data_flags[ZQ_TILES],f,true))
    {
      return qe_invalid;
    }

    if(!pfread(&dummybuf,4,f,true))
    {
      return qe_invalid;
    }

    if(!p_getc(&tempheader.data_flags[ZQ_CHEATS2],f,true))
    {
      return qe_invalid;
    }
    if(!pfread(dummybuf,14,f,true))
    {
      return qe_invalid;
    }
    if(!pfread(tempheader.templatepath,sizeof(tempheader.templatepath),f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_map_count,f,true))
    {
      return qe_invalid;
    }
  }
  if (keepdata==true)
  {
    memcpy(header, &tempheader, sizeof(tempheader));
    map_count=temp_map_count;
    memcpy(midi_flags, temp_midi_flags, MIDIFLAGS_SIZE);
  }
  return 0;
}

int readrules(PACKFILE *f, zquestheader *header, bool keepdata)
{
  int dummy;
  zquestheader tempheader;

  memcpy(&tempheader, header, sizeof(tempheader));

  if (tempheader.zelda_version >= 0x193)
  {
    //section version info
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }

    //section size
    if(!p_igetl(&dummy,f,true))
    {
      return qe_invalid;
    }

    //finally...  section data
    if(!pfread(quest_rules,QUESTRULES_SIZE,f,true))
    {
      return qe_invalid;
    }
  }

  //Now, do any updates...
  if ((tempheader.zelda_version < 0x193)||((tempheader.zelda_version == 0x193)&&(tempheader.build<3)))
  {
    set_bit(quest_rules,qr_WALLFLIERS,1);
  }
  if ((tempheader.zelda_version < 0x193)||((tempheader.zelda_version == 0x193)&&(tempheader.build<4)))
  {
    set_bit(quest_rules,qr_NOBOMBPALFLASH,1);
  }
  //might not be correct
  if(tempheader.zelda_version < 0x210)
  {
    set_bit(quest_rules,qr_NOSCROLLCONTINUE,1);
  }
  if(tempheader.zelda_version < 0x210)
  {
	  set_bit(quest_rules, qr_OLDTRIBBLES,1);
  }
  if (keepdata==true)
  {
    memcpy(header, &tempheader, sizeof(tempheader));
  }

  return 0;
}

int readstrings(PACKFILE *f, zquestheader *header, bool keepdata)
{
  //reset the message strings
  if (keepdata==true)
  {
    for(int i=0; i<MAXMSGS; i++)
    {
      memset(MsgStrings[i].s, 0, 73);
      MsgStrings[i].nextstring=0;
      memset(MsgStrings[i].expansion, 0, 32);
    }
    strcpy(MsgStrings[0].s,"(None)");
  }

  MsgStr tempMsgString;
  word temp_msg_count;

  if (header->zelda_version < 0x193)
  {
    byte tempbyte;
    int strings_to_read=0;

    if ((header->zelda_version < 0x192)||
      ((header->zelda_version == 0x192)&&(header->build<31)))
    {
      strings_to_read=128;
      temp_msg_count=header->old_str_count;
    }
    else if ((header->zelda_version == 0x192)&&(header->build<140))
    {
      strings_to_read=255;
      temp_msg_count=header->old_str_count;
    }
    else
    {
      if(!p_igetw(&temp_msg_count,f,true))
      {
        return qe_invalid;
      }
      strings_to_read=temp_msg_count;
    }

    for (int x=0; x<strings_to_read; x++)
    {
/*
      if(!pfread(&MsgStrings[x].s,sizeof(tempMsgString.s),f,true))
      {
        return qe_invalid;
      }
*/
      memset(&tempMsgString.s, 0, 73);
      tempMsgString.nextstring=0;
      memset(&tempMsgString.expansion, 0, 32);

      if(!pfread(&tempMsgString.s,sizeof(tempMsgString.s),f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&tempbyte,f,true))
      {
        return qe_invalid;
      }
      if ((header->zelda_version < 0x192)||
        ((header->zelda_version == 0x192)&&(header->build<148)))
      {
        tempMsgString.nextstring=tempbyte?x+1:0;
        if(!p_getc(&tempbyte,f,true))
        {
          return qe_invalid;
        }
        if(!p_getc(&tempbyte,f,true))
        {
          return qe_invalid;
        }
        memset(tempMsgString.expansion, 0, 32);
      }
      else
      {
        if(!p_igetw(&tempMsgString.nextstring,f,true))
        {
          return qe_invalid;
        }
        if(!pfread(&tempMsgString.expansion,32,f,true))
        {
          return qe_invalid;
        }
      }
      if (keepdata==true)
      {
        memcpy(&MsgStrings[x], &tempMsgString, sizeof(tempMsgString));
      }
    }
  }
  else
  {

    int dummy;
    //section version info
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }
    //section size
    if(!p_igetl(&dummy,f,true))
    {
      return qe_invalid;
    }
    //finally...  section data
    if(!p_igetw(&temp_msg_count,f,true))
    {
      return qe_invalid;
    }
    for (int i=0; i<temp_msg_count; i++)
    {
      if(!pfread(&tempMsgString.s,sizeof(tempMsgString.s),f,true))
      {
        return qe_invalid;
      }
      if(!p_igetw(&tempMsgString.nextstring,f,true))
      {
        return qe_invalid;
      }
      if (keepdata==true)
      {
        memcpy(&MsgStrings[i], &tempMsgString, sizeof(tempMsgString));
      }
    }
  }
  if (keepdata==true)
  {
    msg_count=temp_msg_count;
  }
  return 0;
}

int readdoorcombosets(PACKFILE *f, zquestheader *header, bool keepdata)
{
  if ((header->zelda_version < 0x192)||
    ((header->zelda_version == 0x192)&&(header->build<158)))
  {
    return 0;
  }

  word temp_door_combo_set_count=0;
  DoorComboSet tempDoorComboSet;
  word dummy_word;
  long dummy_long;
  byte padding;

  if (keepdata==true)
  {
    for(int i=0; i<MAXDOORCOMBOSETS; i++)
    {
      memset(DoorComboSets+i, 0, sizeof(DoorComboSet));
    }
  }
  if (header->zelda_version > 0x192)
  {
    //section version info
    if(!p_igetw(&dummy_word,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&dummy_word,f,true))
    {
      return qe_invalid;
    }
    //section size
    if(!p_igetl(&dummy_long,f,true))
    {
      return qe_invalid;
    }
  }

  //finally...  section data
  if(!p_igetw(&temp_door_combo_set_count,f,true))
  {
    return qe_invalid;
  }
  for (int i=0; i<temp_door_combo_set_count; i++)
  {
    memset(&tempDoorComboSet, 0, sizeof(DoorComboSet));
    //name
    if(!pfread(&tempDoorComboSet.name,sizeof(tempDoorComboSet.name),f,true))
    {
      return qe_invalid;
    }
    if (header->zelda_version < 0x193)
    {
      if(!p_getc(&padding,f,true))
      {
        return qe_invalid;
      }
    }
    //up door
    for (int j=0; j<9; j++)
    {
      for (int k=0; k<4; k++)
      {
        if(!p_igetw(&tempDoorComboSet.doorcombo_u[j][k],f,true))
        {
          return qe_invalid;
        }
      }
    }
    for (int j=0; j<9; j++)
    {
      for (int k=0; k<4; k++)
      {
        if(!p_getc(&tempDoorComboSet.doorcset_u[j][k],f,true))
        {
          return qe_invalid;
        }
      }
    }

    //down door
    for (int j=0; j<9; j++)
    {
      for (int k=0; k<4; k++)
      {
        if(!p_igetw(&tempDoorComboSet.doorcombo_d[j][k],f,true))
        {
          return qe_invalid;
        }
      }
    }
    for (int j=0; j<9; j++)
    {
      for (int k=0; k<4; k++)
      {
        if(!p_getc(&tempDoorComboSet.doorcset_d[j][k],f,true))
        {
          return qe_invalid;
        }
      }
    }

    //left door
    for (int j=0; j<9; j++)
    {
      for (int k=0; k<6; k++)
      {
        if(!p_igetw(&tempDoorComboSet.doorcombo_l[j][k],f,true))
        {
          return qe_invalid;
        }
      }
    }
    for (int j=0; j<9; j++)
    {
      for (int k=0; k<6; k++)
      {
        if(!p_getc(&tempDoorComboSet.doorcset_l[j][k],f,true))
        {
          return qe_invalid;
        }
      }
    }

    //right door
    for (int j=0; j<9; j++)
    {
      for (int k=0; k<6; k++)
      {
        if(!p_igetw(&tempDoorComboSet.doorcombo_r[j][k],f,true))
        {
          return qe_invalid;
        }
      }
    }
    for (int j=0; j<9; j++)
    {
      for (int k=0; k<6; k++)
      {
        if(!p_getc(&tempDoorComboSet.doorcset_r[j][k],f,true))
        {
          return qe_invalid;
        }
      }
    }

    //up bomb rubble
    for (int j=0; j<2; j++)
    {
      if(!p_igetw(&tempDoorComboSet.bombdoorcombo_u[j],f,true))
      {
        return qe_invalid;
      }
    }
    for (int j=0; j<2; j++)
    {
      if(!p_getc(&tempDoorComboSet.bombdoorcset_u[j],f,true))
      {
        return qe_invalid;
      }
    }

    //down bomb rubble
    for (int j=0; j<2; j++)
    {
      if(!p_igetw(&tempDoorComboSet.bombdoorcombo_d[j],f,true))
      {
        return qe_invalid;
      }
    }
    for (int j=0; j<2; j++)
    {
      if(!p_getc(&tempDoorComboSet.bombdoorcset_d[j],f,true))
      {
        return qe_invalid;
      }
    }

    //left bomb rubble
    for (int j=0; j<3; j++)
    {
      if(!p_igetw(&tempDoorComboSet.bombdoorcombo_l[j],f,true))
      {
        return qe_invalid;
      }
    }
    for (int j=0; j<3; j++)
    {
      if(!p_getc(&tempDoorComboSet.bombdoorcset_l[j],f,true))
      {
        return qe_invalid;
      }
    }
    if (header->zelda_version < 0x193)
    {
      if(!p_getc(&padding,f,true))
      {
        return qe_invalid;
      }

    }

    //right bomb rubble
    for (int j=0; j<3; j++)
    {
      if(!p_igetw(&tempDoorComboSet.bombdoorcombo_r[j],f,true))
      {
        return qe_invalid;
      }
    }
    for (int j=0; j<3; j++)
    {
      if(!p_getc(&tempDoorComboSet.bombdoorcset_r[j],f,true))
      {
        return qe_invalid;
      }
    }
    if (header->zelda_version < 0x193)
    {
      if(!p_getc(&padding,f,true))
      {
        return qe_invalid;
      }
    }

    //walkthrough stuff
    for (int j=0; j<4; j++)
    {
      if(!p_igetw(&tempDoorComboSet.walkthroughcombo[j],f,true))
      {
        return qe_invalid;
      }
    }
    for (int j=0; j<4; j++)
    {
      if(!p_getc(&tempDoorComboSet.walkthroughcset[j],f,true))
      {
        return qe_invalid;
      }
    }
    //flags
    for (int j=0; j<2; j++)
    {
      if(!p_getc(&tempDoorComboSet.flags[j],f,true))
      {
        return qe_invalid;
      }
    }
    if (header->zelda_version < 0x193)
    {
      if(!pfread(&tempDoorComboSet.expansion,sizeof(tempDoorComboSet.expansion),f,true))
      {
        return qe_invalid;
      }
    }
    if (keepdata==true)
    {
      memcpy(&DoorComboSets[i], &tempDoorComboSet, sizeof(tempDoorComboSet));
    }
  }
  if (keepdata==true)
  {
    door_combo_set_count=temp_door_combo_set_count;
  }
  return 0;
}

int count_dmaps()
{
  int i=MAXDMAPS-1;
  bool found=false;
  while(i>=0 && !found)
  {
    if ((DMaps[i].map!=0)||(DMaps[i].level!=0)||(DMaps[i].xoff!=0)||
      (DMaps[i].compass!=0)||(DMaps[i].color!=0)||(DMaps[i].midi!=0)||
      (DMaps[i].cont!=0)||(DMaps[i].type!=0))
      found=true;
    for (int j=0; j<8; j++)
    {
      if (DMaps[i].grid[j]!=0)

        found=true;
    }
    if ((DMaps[i].name[0]!=0)||(DMaps[i].title[0]!=0)||
      (DMaps[i].intro[0]!=0)||(DMaps[i].tmusic[0]!=0))
      found=true;
    if ((DMaps[i].minimap_1_tile!=0)||(DMaps[i].minimap_2_tile!=0)||
      (DMaps[i].largemap_1_tile!=0)||(DMaps[i].largemap_2_tile!=0)||
      (DMaps[i].minimap_1_cset!=0)||(DMaps[i].minimap_2_cset!=0)||
      (DMaps[i].largemap_1_cset!=0)||(DMaps[i].largemap_2_cset!=0))
      found=true;
    if (!found)
    {
      i--;
    }
  }
  return i+1;
}


int count_shops(miscQdata *misc)
{
  int i=15,j;
  bool found=false;
  while(i>=0 && !found)
  {
    j=2;
    while(j>=0 && !found)
    {
      if ((misc->shop[i].item[j]!=0)||(misc->shop[i].price[j]!=0))
      {
        found=true;
      }
      else
      {
        j--;
      }
    }
    if (!found)
    {
      i--;
    }
  }
  return i+1;
}

int count_infos(miscQdata *misc)
{
  int i=15,j;
  bool found=false;
  while(i>=0 && !found)
  {
    j=2;
    while(j>=0 && !found)
    {
      if ((misc->info[i].str[j]!=0)||(misc->info[i].price[j]!=0))
      {
        found=true;
      }
      else
      {
        j--;
      }
    }
    if (!found)
    {
      i--;
    }
  }
  return i+1;
}

int count_warprings(miscQdata *misc)
{
  int i=15,j;
  bool found=false;
  while(i>=0 && !found)
  {
    j=7;
    while(j>=0 && !found)
    {
      if ((misc->warp[i].dmap[j]!=0)||(misc->warp[i].scr[j]!=0))
      {
        found=true;
      }
      else
      {
        j--;
      }
    }
    if (!found)
    {
      i--;
    }
  }
  return i+1;
}

int count_palcycles(miscQdata *misc)
{
  int i=255,j;
  bool found=false;
  while(i>=0 && !found)
  {
    j=2;
    while(j>=0 && !found)
    {
      if (misc->cycles[i][j].count!=0)
      {
        found=true;
      }
      else
      {
        j--;
      }
    }
    if (!found)
    {
      i--;
    }
  }
  return i+1;
}

int count_windwarps(miscQdata *misc)
{
  int i=8;
  bool found=false;
  while(i>=0 && !found)
  {
    if ((misc->wind[i].dmap!=0)||(misc->wind[i].scr!=0))
    {
      found=true;
    }
    else
    {
      i--;
    }
  }
  return i+1;
}

void clear_screen(mapscr *temp_scr)
{
  memset(temp_scr,0,sizeof(mapscr));
  for (int j=0; j<6; j++)
  {
    temp_scr->layeropacity[j]=255;
  }
}

int readdmaps(PACKFILE *f, zquestheader *Header, word version, word build, word start_dmap, word max_dmaps, bool keepdata)
{
  //these are here to bypass compiler warnings about unused arguments
  version=version;
  build=build;

  word dmapstoread=0;
  dmap tempDMap;

  int dummy;
  word s_version=0, s_cversion=0;
  byte padding;
  if (keepdata==true)
  {
    for (int i=0; i<max_dmaps; i++)
    {
      memset(&DMaps[start_dmap+i],0,sizeof(dmap));
      sprintf(DMaps[start_dmap+i].title,"                    ");
      sprintf(DMaps[start_dmap+i].intro,"                                                                        ");
    }
  }

  if (!Header || Header->zelda_version > 0x192)
  {
    //section version info
    if(!p_igetw(&s_version,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&s_cversion,f,true))
    {
      return qe_invalid;
    }

    //section size
    if(!p_igetl(&dummy,f,true))
    {
      return qe_invalid;
    }

    //finally...  section data
    if(!p_igetw(&dmapstoread,f,true))
    {
      return qe_invalid;
    }
  }
  else
  {
    if ((Header->zelda_version < 0x192)||
        ((Header->zelda_version == 0x192)&&(Header->build<5)))
    {
      dmapstoread=32;
    }
    else
    {
      dmapstoread=MAXDMAPS;
    }
  }

  dmapstoread=min(dmapstoread, max_dmaps);
  dmapstoread=min(dmapstoread, MAXDMAPS-start_dmap);

  for (int i=start_dmap; i<dmapstoread+start_dmap; i++)
  {
    memset(&tempDMap,0,sizeof(dmap));
    sprintf(tempDMap.title,"                    ");
    sprintf(tempDMap.intro,"                                                                        ");

    if(!p_getc(&tempDMap.map,f,keepdata))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempDMap.level,f,keepdata))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempDMap.xoff,f,keepdata))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempDMap.compass,f,keepdata))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempDMap.color,f,keepdata))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempDMap.midi,f,keepdata))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempDMap.cont,f,keepdata))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempDMap.type,f,keepdata))
    {
      return qe_invalid;
    }
    for (int j=0; j<8; j++)
    {
      if(!p_getc(&tempDMap.grid[j],f,keepdata))
      {
        return qe_invalid;
      }
    }

    if (Header && ((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<41))))
    {
      if (tempDMap.level>0&&tempDMap.level<10)
      {
        sprintf(tempDMap.title,"LEVEL-%d             ", tempDMap.level);
      }
      if (i==0 && Header->zelda_version <= 0x190)
      {
        tempDMap.cont-=tempDMap.xoff;
        tempDMap.compass-=tempDMap.xoff;
      }
      if (keepdata==true)
      {
        memcpy(&DMaps[i], &tempDMap, sizeof(tempDMap));
      }
      continue;
    }
    if(!pfread(&tempDMap.name,sizeof(DMaps[0].name),f,true))
    {
      return qe_invalid;
    }
    if(!pfread(&tempDMap.title,sizeof(DMaps[0].title),f,true))
    {
      return qe_invalid;
    }
    if(!pfread(&tempDMap.intro,sizeof(DMaps[0].intro),f,true))
    {
      return qe_invalid;
    }
    if (Header && ((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<152))))
    {
      if (keepdata==true)
      {
        memcpy(&DMaps[i], &tempDMap, sizeof(tempDMap));
      }
      continue;
    }
    if (Header && (Header->zelda_version < 0x193))
    {
      if(!p_getc(&padding,f,keepdata))
      {
        return qe_invalid;
      }
    }
    if(!p_igetw(&tempDMap.minimap_1_tile,f,keepdata))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempDMap.minimap_1_cset,f,keepdata))
    {
      return qe_invalid;
    }
    if (Header && (Header->zelda_version < 0x193))
    {
      if(!p_getc(&padding,f,keepdata))
      {
        return qe_invalid;
      }
    }
    if(!p_igetw(&tempDMap.minimap_2_tile,f,keepdata))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempDMap.minimap_2_cset,f,keepdata))
    {
      return qe_invalid;
    }
    if (Header && (Header->zelda_version < 0x193))
    {
      if(!p_getc(&padding,f,keepdata))
      {
        return qe_invalid;
      }
    }
    if(!p_igetw(&tempDMap.largemap_1_tile,f,keepdata))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempDMap.largemap_1_cset,f,keepdata))
    {
      return qe_invalid;
    }
    if (Header && (Header->zelda_version < 0x193))
    {
      if(!p_getc(&padding,f,keepdata))
      {
        return qe_invalid;
      }
    }
    if(!p_igetw(&tempDMap.largemap_2_tile,f,keepdata))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempDMap.largemap_2_cset,f,keepdata))
    {
      return qe_invalid;
    }
    if(!pfread(&tempDMap.tmusic,sizeof(DMaps[0].tmusic),f,true))
    {
      return qe_invalid;
    }
    if (Header && (Header->zelda_version < 0x193))
    {
      if(!p_getc(&padding,f,keepdata))
      {
        return qe_invalid;
      }
    }
    if (keepdata==true)
    {
      memcpy(&DMaps[i], &tempDMap, sizeof(tempDMap));
    }
  }
  return 0;
}


int readmisc(PACKFILE *f, zquestheader *header, miscQdata *misc, bool keepdata)
{
  word maxinfos=16; word maxshops=16;
  word shops=16, infos=16, warprings=8, palcycles=256, windwarps=9, triforces=8, icons=4;
  word ponds=16, pondsize=72, expansionsize=98*2;
  byte tempbyte, padding;
  miscQdata temp_misc;
  word swaptmp;

  memset(&temp_misc,0,sizeof(temp_misc));

  if (header->zelda_version > 0x192)
  {
    int dummy;
    //section version info
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }


    //section size
    if(!p_igetl(&dummy,f,true))
    {
      return qe_invalid;
    }
  }

  //finally...  section data

  //shops
  if (header->zelda_version > 0x192)
  {
    if(!p_igetw(&shops,f,true))
    {
      return qe_invalid;
    }
  }
  for (int i=0; i<shops; i++)
  {
    for (int j=0; j<3; j++)
    {
      if(!p_getc(&temp_misc.shop[i].item[j],f,true))
      {
        return qe_invalid;
      }
    }
    if (header->zelda_version < 0x193)
    {
      if(!p_getc(&tempbyte,f,true))
      {
        return qe_invalid;
      }
    }
    for (int j=0; j<3; j++)
    {
      if(!p_igetw(&temp_misc.shop[i].price[j],f,true))
      {
        return qe_invalid;
      }
    }
  }
  //filter all the 0 items to the end (yeah, bubble sort; sue me)
  for (int i=0; i<maxshops; ++i)
  {
    for (int j=0; j<3-1; j++)
    {
      for (int k=0; k<2-j; k++)
      {
        if (temp_misc.shop[i].item[k]==0)
        {
          swaptmp = temp_misc.shop[i].item[k];
          temp_misc.shop[i].item[k] = temp_misc.shop[i].item[k+1];
          temp_misc.shop[i].item[k+1] = swaptmp;
          swaptmp = temp_misc.shop[i].price[k];
          temp_misc.shop[i].price[k] = temp_misc.shop[i].price[k+1];
          temp_misc.shop[i].price[k+1] = swaptmp;
        }
      }
    }
  }

  //infos
  if (header->zelda_version > 0x192)
  {
    if(!p_igetw(&infos,f,true))
    {
      return qe_invalid;
    }
  }
  for (int i=0; i<infos; i++)
  {
    for (int j=0; j<3; j++)
    {
      if ((header->zelda_version < 0x192)||
        ((header->zelda_version == 0x192)&&(header->build<146)))
      {
        if(!p_getc(&tempbyte,f,true))
        {
          return qe_invalid;
        }
        temp_misc.info[i].str[j]=tempbyte;
      }
      else
      {
        if(!p_igetw(&temp_misc.info[i].str[j],f,true))
        {
          return qe_invalid;
        }
      }
    }
    if (header->zelda_version < 0x193)
    {
      if(!p_getc(&tempbyte,f,true))
      {
        return qe_invalid;
      }
    }
    if ((header->zelda_version == 0x192)&&(header->build>145))
    {
      if(!p_getc(&padding,f,true))
      {
        return qe_invalid;
      }
    }
    for (int j=0; j<3; j++)
    {
      if(!p_igetw(&temp_misc.info[i].price[j],f,true))
      {
        return qe_invalid;
      }
    }
  }
  //filter all the 0 strings to the end (yeah, bubble sort; sue me)
  for (int i=0; i<maxinfos; ++i)
  {
    for (int j=0; j<3-1; j++)
    {
      for (int k=0; k<2-j; k++)
      {
        if (temp_misc.info[i].str[k]==0)
        {
          swaptmp = temp_misc.info[i].str[k];
          temp_misc.info[i].str[k] = temp_misc.info[i].str[k+1];
          temp_misc.info[i].str[k+1] = swaptmp;
          swaptmp = temp_misc.info[i].price[k];
          temp_misc.info[i].price[k] = temp_misc.info[i].price[k+1];
          temp_misc.info[i].price[k+1] = swaptmp;
        }
      }
    }
  }

  //warp rings
  if (header->zelda_version > 0x192)
  {
    if(!p_igetw(&warprings,f,true))
    {
      return qe_invalid;
    }
  }
  for (int i=0; i<warprings; i++)
  {
    for (int j=0; j<8; j++)
    {
      if(!p_getc(&temp_misc.warp[i].dmap[j],f,true))
      {
        return qe_invalid;
      }
    }
    for (int j=0; j<8; j++)
    {
      if(!p_getc(&temp_misc.warp[i].scr[j],f,true))
      {
        return qe_invalid;
      }
    }
    if(!p_getc(&temp_misc.warp[i].size,f,true))
    {
      return qe_invalid;
    }
    if (header->zelda_version < 0x193)
    {
      if(!p_getc(&tempbyte,f,true))
      {
        return qe_invalid;
      }
    }
  }

  //palette cycles
  if (header->zelda_version < 0x193)                        //in 1.93+, palette cycling is saved with the palettes
  {
    if ((header->zelda_version < 0x192)||
      ((header->zelda_version == 0x192)&&(header->build<73)))
    {
      palcycles=16;
    }
    for (int i=0; i<palcycles; i++)
    {
      for (int j=0; j<3; j++)
      {
        if(!p_getc(&temp_misc.cycles[i][j].first,f,true))
        {
          return qe_invalid;
        }
        if(!p_getc(&temp_misc.cycles[i][j].count,f,true))
        {
          return qe_invalid;
        }
        if(!p_getc(&temp_misc.cycles[i][j].speed,f,true))
        {
          return qe_invalid;
        }
      }
    }
  }

  //wind warps
  if (header->zelda_version > 0x192)
  {
    if(!p_igetw(&windwarps,f,true))
    {
      return qe_invalid;
    }
  }
  for (int i=0; i<windwarps; i++)
  {
    if(!p_getc(&temp_misc.wind[i].dmap,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.wind[i].scr,f,true))
    {
      return qe_invalid;
    }
  }
  //triforce pieces
  for (int i=0; i<triforces; i++)
  {
    if(!p_getc(&temp_misc.triforce[i],f,true))
    {
      return qe_invalid;
    }

  }
  //misc color data

  {                                                         //this brace is here to make it easier to fold all the color data code
    if(!p_getc(&temp_misc.colors.text,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.caption,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.overw_bg,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.dngn_bg,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.dngn_fg,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.cave_fg,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.bs_dk,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.bs_goal,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.compass_lt,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.compass_dk,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.subscr_bg,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.triframe_color,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.link_dot,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.bmap_bg,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.bmap_fg,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.triforce_cset,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.triframe_cset,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.overworld_map_cset,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.dungeon_map_cset,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.blueframe_cset,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&temp_misc.colors.triforce_tile,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&temp_misc.colors.triframe_tile,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&temp_misc.colors.overworld_map_tile,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&temp_misc.colors.dungeon_map_tile,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&temp_misc.colors.blueframe_tile,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&temp_misc.colors.HCpieces_tile,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_misc.colors.HCpieces_cset,f,true))
    {
      return qe_invalid;
    }
    if (header->zelda_version < 0x193)
    {
      for (int i=0; i<7; i++)

      {
        if(!p_getc(&tempbyte,f,true))
        {
          return qe_invalid;
        }
      }
    }
    if ((header->zelda_version == 0x192)&&(header->build>145))
    {
      for (int i=0; i<256; i++)
      {
        if(!p_getc(&tempbyte,f,true))
        {
          return qe_invalid;
        }
      }
    }
  }

  //save game icons
  if ((header->zelda_version < 0x192)||
    ((header->zelda_version == 0x192)&&(header->build<73)))
  {
    icons=3;
  }
  for (int i=0; i<icons; i++)
  {
    if(!p_igetw(&temp_misc.icons[i],f,true))
    {
      return qe_invalid;
    }
  }
  if ((header->zelda_version < 0x192)||
    ((header->zelda_version == 0x192)&&(header->build<30)))
  {
    if (keepdata==true)
    {
      memcpy(misc, &temp_misc, sizeof(temp_misc));
    }
    return 0;
  }

  //pond information
  if (header->zelda_version < 0x193)
  {
    if ((header->zelda_version == 0x192)&&(header->build<146))
    {
      pondsize=25;
    }
    for (int i=0; i<ponds; i++)
    {
      for (int j=0; j<pondsize; j++)
      {
        if(!p_getc(&tempbyte,f,true))
        {
          return qe_invalid;

        }
      }
    }
  }

  //end string
  if ((header->zelda_version < 0x192)||
    ((header->zelda_version == 0x192)&&(header->build<146)))
  {
    if(!p_getc(&tempbyte,f,true))
    {
      return qe_invalid;
    }
    temp_misc.endstring=tempbyte;
    if(!p_getc(&tempbyte,f,true))
    {
      return qe_invalid;
    }
  }
  else
  {
    if(!p_igetw(&temp_misc.endstring,f,true))
    {
      return qe_invalid;
    }
  }

  //expansion
  if (header->zelda_version < 0x193)
  {
    if ((header->zelda_version == 0x192)&&(header->build<73))
    {
      expansionsize=99*2;
    }
    for (int i=0; i<expansionsize; i++)
    {
      if(!p_getc(&tempbyte,f,true))
      {
        return qe_invalid;
      }
    }
  }
  if (keepdata==true)
  {
    memcpy(misc, &temp_misc, sizeof(temp_misc));
  }
  return 0;
}

int readitems(PACKFILE *f, word version, word build, bool keepdata)
{
  byte padding;
  int  dummy;
  word items_to_read=MAXITEMS;
  itemdata tempitem;
  if(version < 0x186)
  {
    items_to_read=64;
  }

  if (version > 0x192)
  {
    items_to_read=0;
    //section version info
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }

    //section size
    if(!p_igetl(&dummy,f,true))
    {
      return qe_invalid;
    }

    //finally...  section data
    if(!p_igetw(&items_to_read,f,true))
    {
      return qe_invalid;
    }

  }

  for (int i=0; i<items_to_read; i++)
  {
    if(!p_igetw(&tempitem.tile,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempitem.misc,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempitem.csets,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempitem.frames,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempitem.speed,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempitem.delay,f,true))
    {
      return qe_invalid;
    }
    if (version < 0x193)
    {
      if(!p_getc(&padding,f,true))
      {
        return qe_invalid;
      }
      if ((version < 0x192)||((version == 0x192)&&(build<186)))
      {
        switch (i)
        {
          case iShield:
            tempitem.ltm=get_bit(quest_rules,qr_BSZELDA)?-12:10;
            break;
          case iMShield:
            tempitem.ltm=get_bit(quest_rules,qr_BSZELDA)?-6:-10;
            break;
          default:
            tempitem.ltm=0;
            break;
        }
        if (keepdata==true)
        {
          memcpy(&itemsbuf[i], &tempitem, sizeof(tempitem));
        }
        continue;
      }
    }
    if(!p_igetl(&tempitem.ltm,f,true))
    {
      return qe_invalid;
    }

    if (version < 0x193)
    {
      for (int q=0; q<12; q++)
      {
        if(!p_getc(&padding,f,true))
        {
          return qe_invalid;
        }
      }
    }
    if (keepdata==true)
    {
      memcpy(&itemsbuf[i], &tempitem, sizeof(tempitem));
    }
  }
  return 0;
}

int readweapons(PACKFILE *f, zquestheader *header, bool keepdata)
{
  word weapons_to_read=MAXWPNS;
  int dummy;
  byte padding;
  wpndata tempweapon;

  if(header->zelda_version < 0x186)
  {
    weapons_to_read=64;
  }
  if(header->zelda_version < 0x185)
  {
    weapons_to_read=32;
  }

  if (header->zelda_version > 0x192)
  {
    weapons_to_read=0;
    //section version info
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }

    //section size
    if(!p_igetl(&dummy,f,true))
    {
      return qe_invalid;
    }

    //finally...  section data
    if(!p_igetw(&weapons_to_read,f,true))
    {
      return qe_invalid;
    }
  }

  for (int i=0; i<weapons_to_read; i++)
  {
    if(!p_igetw(&tempweapon.tile,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempweapon.misc,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempweapon.csets,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempweapon.frames,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempweapon.speed,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&tempweapon.type,f,true))
    {
      return qe_invalid;
    }
    if (header->zelda_version < 0x193)
    {
      if(!p_getc(&padding,f,true))
      {
        return qe_invalid;
      }
    }
    if (keepdata==true)
    {
      memcpy(&wpnsbuf[i], &tempweapon, sizeof(tempweapon));
    }
  }
  if (keepdata==true)
  {
    if(header->zelda_version < 0x176)
    {
      wpnsbuf[iwSpawn] = *((wpndata*)(itemsbuf + iMisc1));
      wpnsbuf[iwDeath] = *((wpndata*)(itemsbuf + iMisc2));
      memset(&itemsbuf[iMisc1],0,sizeof(itemdata));
      memset(&itemsbuf[iMisc2],0,sizeof(itemdata));
    }
    if ((header->zelda_version < 0x192)||
      ((header->zelda_version == 0x192)&&(header->build<129)))
    {
      wpnsbuf[wHSCHAIN_V] = wpnsbuf[wHSCHAIN_H];
    }
  }
  return 0;
}

int init_guys()
{
  for(int i=0; i<MAXGUYS; i++)
    guysbuf[i] = default_guys[i];
  return 0;
}

int readguys(PACKFILE *f, zquestheader *header, bool keepdata)
{
  dword dummy;

  if (header->zelda_version >= 0x193)
  {
    //section version info
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }

    //section size
    if(!p_igetl(&dummy,f,true))
    {
      return qe_invalid;
    }
  }

  //finally...  section data
  if (keepdata==true)
  {
    init_guys();                            //using default data for now...
  }
  return 0;
}


int readmapscreen(PACKFILE *f, zquestheader *header, mapscr *temp_mapscr)
{
  byte tempbyte, padding;
  int extras, secretcombos;

  if(!p_getc(&(temp_mapscr->valid),f,true))
  {
    return qe_invalid;
  }
  if(!p_getc(&(temp_mapscr->guy),f,true))
  {
    return qe_invalid;
  }

  if ((header->zelda_version < 0x192)||((header->zelda_version == 0x192)&&(header->build<146)))
  {
    if(!p_getc(&tempbyte,f,true))
    {
      return qe_invalid;
    }
    temp_mapscr->str=tempbyte;
  }
  else
  {
    if(!p_igetw(&(temp_mapscr->str),f,true))
    {
      return qe_invalid;
    }
  }

  if(!p_getc(&(temp_mapscr->room),f,true))
  {
    return qe_invalid;
  }
  if(!p_getc(&(temp_mapscr->item),f,true))
  {
    return qe_invalid;
  }
  if ((header->zelda_version < 0x192)||
    ((header->zelda_version == 0x192)&&(header->build<154)))
  {
    if(!p_getc(&tempbyte,f,true))
    {
      return qe_invalid;
    }
  }

  if(!p_getc(&(temp_mapscr->tilewarptype),f,true))
  {
    return qe_invalid;
  }
  if (header->zelda_version < 0x193)
  {
    if(!p_getc(&tempbyte,f,true))
    {
      return qe_invalid;
    }
  }

  if ((header->zelda_version > 0x192)||((header->zelda_version == 0x192)&&(header->build>153)))
  {
    if(!p_igetw(&(temp_mapscr->door_combo_set),f,true))
    {
      return qe_invalid;
    }
  }

  if(!p_getc(&(temp_mapscr->warpreturnx),f,true))
  {
    return qe_invalid;
  }
  if(!p_getc(&(temp_mapscr->warpreturny),f,true))
  {
    return qe_invalid;
  }
  if(!p_getc(&(temp_mapscr->stairx),f,true))

  {
    return qe_invalid;
  }
  if(!p_getc(&(temp_mapscr->stairy),f,true))
  {
    return qe_invalid;
  }
  if(!p_getc(&(temp_mapscr->itemx),f,true))
  {
    return qe_invalid;
  }
  if(!p_getc(&(temp_mapscr->itemy),f,true))
  {
    return qe_invalid;
  }
  if(!p_getc(&(temp_mapscr->color),f,true))
  {
    return qe_invalid;
  }
  if(!p_getc(&(temp_mapscr->enemyflags),f,true))
  {
    return qe_invalid;
  }
  for(int k=0; k<4; k++)
  {
    if(!p_getc(&(temp_mapscr->door[k]),f,true))
    {
      return qe_invalid;

    }
  }
  if(!p_getc(&(temp_mapscr->tilewarpdmap),f,true))
  {
    return qe_invalid;
  }
  if(!p_getc(&(temp_mapscr->tilewarpscr),f,true))
  {
    return qe_invalid;
  }
  if(!p_getc(&(temp_mapscr->exitdir),f,true))
  {
    return qe_invalid;
  }
  if (header->zelda_version < 0x193) {
    if(!p_getc(&tempbyte,f,true))
    {
      return qe_invalid;
    }

  }

  if ((header->zelda_version == 0x192)&&(header->build>145)&&(header->build<154))
  {
    if(!p_getc(&padding,f,true))
    {
      return qe_invalid;
    }
  }

  for(int k=0; k<10; k++)
  {
/*
    if (!temp_mapscr->enemy[k])
    {
      continue;
    }
*/
    if ((header->zelda_version < 0x192)||((header->zelda_version == 0x192)&&(header->build<10)))
    {
      if(!p_getc(&tempbyte,f,true))
      {
        return qe_invalid;
      }
      temp_mapscr->enemy[k]=tempbyte;
    }
    else
    {
      if(!p_igetw(&(temp_mapscr->enemy[k]),f,true))
      {
        return qe_invalid;
      }
    }
    if ((header->zelda_version < 0x192)||((header->zelda_version == 0x192)&&(header->build<108)))
    {
      if (temp_mapscr->enemy[k]>=eDIG3) //old eGOHMA1
      {
        temp_mapscr->enemy[k]+=5;
      }
      else if (temp_mapscr->enemy[k]>=eGLEEOK1) //old eGLEEOK2
      {
        temp_mapscr->enemy[k]+=1;
      }
    }
  }

  if(!p_getc(&(temp_mapscr->pattern),f,true))
  {
    return qe_invalid;
  }
  if(!p_getc(&(temp_mapscr->sidewarptype),f,true))
  {
    return qe_invalid;
  }
  if(!p_getc(&(temp_mapscr->warparrivalx),f,true))
  {
    return qe_invalid;
  }
  if(!p_getc(&(temp_mapscr->warparrivaly),f,true))
  {
    return qe_invalid;
  }
  for(int k=0; k<4; k++)
  {
    if(!p_getc(&(temp_mapscr->path[k]),f,true))
    {
      return qe_invalid;
    }
  }
  if(!p_getc(&(temp_mapscr->sidewarpscr),f,true))
  {
    return qe_invalid;
  }
  if(!p_getc(&(temp_mapscr->sidewarpdmap),f,true))
  {
    return qe_invalid;
  }
  if(!p_igetw(&(temp_mapscr->undercombo),f,true))
  {
    return qe_invalid;
  }
  if (header->zelda_version < 0x193)
  {
    if(!p_getc(&(temp_mapscr->old_cpage),f,true))
    {
      return qe_invalid;
    }
  }
  if(!p_getc(&(temp_mapscr->undercset),f,true))                //recalculated for older quests
  {
    return qe_invalid;
  }
  if(!p_igetw(&(temp_mapscr->catchall),f,true))
  {
    return qe_invalid;
  }
  if(!p_getc(&(temp_mapscr->flags),f,true))
  {
    return qe_invalid;
  }
  if(!p_getc(&(temp_mapscr->flags2),f,true))
  {
    return qe_invalid;
  }
  if(!p_getc(&(temp_mapscr->flags3),f,true))
  {
    return qe_invalid;
  }

  if ((header->zelda_version > 0x192)||((header->zelda_version == 0x192)&&(header->build>97)))
  {
    for(int k=0; k<6; k++)
    {
      if(!p_getc(&(temp_mapscr->layermap[k]),f,true))
      {
        return qe_invalid;
      }
    }
    for(int k=0; k<6; k++)
    {
      if(!p_getc(&(temp_mapscr->layerscreen[k]),f,true))
      {
        return qe_invalid;
      }
    }
  }
  else if ((header->zelda_version == 0x192)&&(header->build>23)&&(header->build<98))
  {
    if(!p_getc(&(temp_mapscr->layermap[2]),f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&(temp_mapscr->layerscreen[2]),f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&(temp_mapscr->layermap[4]),f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&(temp_mapscr->layerscreen[4]),f,true))

    {
      return qe_invalid;
    }
  }

  if ((header->zelda_version == 0x192)&&(header->build>149))
  {
    for(int k=0; k<6; k++)
    {
      if(!p_getc(&tempbyte,f,true))                          //layerxsize
      {
        return qe_invalid;
      }
    }
    for(int k=0; k<6; k++)
    {
      if(!p_getc(&tempbyte,f,true))                          //layerxspeed
      {
        return qe_invalid;
      }
    }
    for(int k=0; k<6; k++)
    {
      if(!p_getc(&tempbyte,f,true))                          //layerxdelay
      {
        return qe_invalid;
      }
    }
    for(int k=0; k<6; k++)
    {
      if(!p_getc(&tempbyte,f,true))                          //layerysize
      {
        return qe_invalid;
      }
    }
    for(int k=0; k<6; k++)
    {
      if(!p_getc(&tempbyte,f,true))                          //layeryspeed
      {
        return qe_invalid;
      }
    }
    for(int k=0; k<6; k++)
    {
      if(!p_getc(&tempbyte,f,true))                          //layerydelay
      {
        return qe_invalid;
      }
    }
  }

  if ((header->zelda_version > 0x192)||((header->zelda_version == 0x192)&&(header->build>149)))
  {
    for(int k=0; k<6; k++)
    {
      if(!p_getc(&(temp_mapscr->layeropacity[k]),f,true))
      {
        return qe_invalid;
      }
    }
  }

  if ((header->zelda_version > 0x192)||((header->zelda_version == 0x192)&&(header->build>153)))
  {
    if ((header->zelda_version == 0x192)&&(header->build>153))
    {
      if(!p_getc(&padding,f,true))
      {
        return qe_invalid;
      }
    }
    if(!p_igetw(&(temp_mapscr->timedwarptics),f,true))
    {
      return qe_invalid;
    }
  }

  if ((header->zelda_version < 0x192)||((header->zelda_version == 0x192)&&(header->build<24)))
  {
    extras=15;
  }
  else if (((header->zelda_version == 0x192)&&(header->build<98)))
  {
    extras=11;
  }
  else if ((header->zelda_version == 0x192)&&(header->build<150))
  {
    extras=32;
  }
  else if ((header->zelda_version == 0x192)&&(header->build<154))
  {
    extras=64;
  }
  else if (header->zelda_version < 0x193)
  {
    extras=62;
  }
  else

  {
    extras=0;
  }

  for(int k=0; k<extras; k++)
  {
    if(!p_getc(&tempbyte,f,true))                            //extra[k]
    {
      return qe_invalid;
    }
  }

  if ((header->zelda_version < 0x192)||((header->zelda_version == 0x192)&&(header->build<137)))
  {
    secretcombos=20;
  }
  else if ((header->zelda_version == 0x192)&&(header->build<154))
  {
    secretcombos=256;
  }
  else
  {
    secretcombos=128;
  }

  if ((header->zelda_version < 0x192)||((header->zelda_version == 0x192)&&(header->build<154)))
  {
    for(int k=0; k<secretcombos; k++)
    {
      if(!p_getc(&tempbyte,f,true))
      {
        return qe_invalid;
      }
      if(k<128)
      {
        temp_mapscr->secretcombo[k]=tempbyte;
      }
    }
  }
  else
  {
    for(int k=0; k<128; k++)
    {
      if(!p_igetw(&(temp_mapscr->secretcombo[k]),f,true))
      {
        return qe_invalid;
      }

    }
  }

  if ((header->zelda_version > 0x192)||((header->zelda_version == 0x192)&&(header->build>153)))
  {
    for(int k=0; k<128; k++)
    {
      if(!p_getc(&(temp_mapscr->secretcset[k]),f,true))
      {
        return qe_invalid;
      }
    }
    for(int k=0; k<128; k++)
    {
      if(!p_getc(&(temp_mapscr->secretflag[k]),f,true))
      {
        return qe_invalid;
      }
    }
  }

  if ((header->zelda_version == 0x192)&&(header->build>97)&&(header->build<154))
  {
    if(!p_getc(&padding,f,true))
    {
      return qe_invalid;
    }
  }

  for(int k=0; k<16*11; k++)
  {
    if(!p_igetw(&(temp_mapscr->data[k]),f,true))
    {
      return qe_invalid;
    }
  }

  if ((header->zelda_version == 0x192)&&(header->build>20)&&(header->build<24))
  {
    if(!p_getc(&padding,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&padding,f,true))
    {
      return qe_invalid;
    }
  }

  if ((header->zelda_version > 0x192)||((header->zelda_version == 0x192)&&(header->build>20)))
  {
    for(int k=0; k<16*11; k++)
    {
      if(!p_getc(&(temp_mapscr->sflag[k]),f,true))
      {
        return qe_invalid;
      }
      if ((header->zelda_version == 0x192)&&(header->build<24))
      {
        if(!p_getc(&tempbyte,f,true))
        {
          return qe_invalid;
        }
        if(!p_getc(&tempbyte,f,true))
        {
          return qe_invalid;
        }
        if(!p_getc(&tempbyte,f,true))
        {
          return qe_invalid;
        }
      }
    }
  }

  if ((header->zelda_version > 0x192)||((header->zelda_version == 0x192)&&(header->build>97)))
  {
    for(int k=0; k<16*11; k++)
    {

      if(!p_getc(&(temp_mapscr->cset[k]),f,true))
      {
        return qe_invalid;
      }
    }
  }

  if ((header->zelda_version < 0x192)||((header->zelda_version == 0x192)&&(header->build<154)))
  {
    temp_mapscr->undercset=(temp_mapscr->undercombo>>8)&7;
    temp_mapscr->undercombo=(temp_mapscr->undercombo&0xFF)+(temp_mapscr->old_cpage<<8);
  }

  if ((header->zelda_version < 0x192)||((header->zelda_version == 0x192)&&(header->build<137)))
  {
    temp_mapscr->secretcombo[sSBOMB]=temp_mapscr->secretcombo[sBOMB];
    temp_mapscr->secretcombo[sRCANDLE]=temp_mapscr->secretcombo[sBCANDLE];
    temp_mapscr->secretcombo[sWANDFIRE]=temp_mapscr->secretcombo[sBCANDLE];
    temp_mapscr->secretcombo[sDINSFIRE]=temp_mapscr->secretcombo[sBCANDLE];
    temp_mapscr->secretcombo[sSARROW]=temp_mapscr->secretcombo[sARROW];
    temp_mapscr->secretcombo[sGARROW]=temp_mapscr->secretcombo[sARROW];
  }

  if ((header->zelda_version < 0x192)||((header->zelda_version == 0x192)&&(header->build<154)))
  {
    for (int k=0; k<176; k++)
    {
      if ((header->zelda_version == 0x192)&&(header->build>149))
      {
        if ((header->zelda_version == 0x192)&&(header->build!=153))
        {
          temp_mapscr->cset[k]=((temp_mapscr->data[k]>>8)&7);
        }
      }
      else
      {
        if ((header->zelda_version < 0x192)||
          ((header->zelda_version == 0x192)&&(header->build<21)))
        {
          temp_mapscr->sflag[k]=(temp_mapscr->data[k]>>11);
        }
        temp_mapscr->cset[k]=((temp_mapscr->data[k]>>8)&7);
      }
      temp_mapscr->data[k]=(temp_mapscr->data[k]&0xFF)+(temp_mapscr->old_cpage<<8);
    }
  }
  return 0;
}

int readmaps(PACKFILE *f, zquestheader *header, bool keepdata)
{
  int scr=0;

  dword dummy;
  int screens_to_read;

  mapscr temp_mapscr;
  word temp_map_count;

  if ((header->zelda_version < 0x192)||((header->zelda_version == 0x192)&&(header->build<137)))
  {
    screens_to_read=MAPSCRS192b136;
  }
  else
  {
    screens_to_read=MAPSCRS;
  }

  if (header->zelda_version > 0x192)
  {
    //section version info
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }

    //section size
    if(!p_igetl(&dummy,f,true))
    {
      return qe_invalid;
    }

    //finally...  section data
    if(!p_igetw(&temp_map_count,f,true))
    {
      return 5;
    }
  }
  else
  {
    temp_map_count=map_count;
  }

  if (keepdata==true)
  {
    free(TheMaps);
    if(!(TheMaps=(mapscr*)malloc(sizeof(mapscr)*MAPSCRS*temp_map_count)))
    {
      return qe_nomem;
    }
  }

  for(int i=0; i<temp_map_count && i<MAXMAPS2; i++)
  {
    for (int j=0; j<screens_to_read; j++)
    {
      scr=i*MAPSCRS+j;
      clear_screen(&temp_mapscr);
      readmapscreen(f, header, &temp_mapscr);
      if (keepdata==true)
      {
        memcpy(&TheMaps[scr], &temp_mapscr, sizeof(mapscr));
      }
    }
    if (keepdata==true)
    {
      map_count=temp_map_count;
      if ((header->zelda_version < 0x192)||((header->zelda_version == 0x192)&&(header->build<137)))
      {
        memcpy(TheMaps+(i*MAPSCRS+132),TheMaps+(i*MAPSCRS+131),sizeof(mapscr));
        for (int j=0; j<MAPSCRS-MAPSCRS192b136-1; j++)
        {
          scr=i*MAPSCRS+j;
          memset(TheMaps+(i*MAPSCRS+j+133), 0, sizeof(mapscr));
        }
      }

      if ((header->zelda_version < 0x192)||((header->zelda_version == 0x192)&&(header->build<154)))
      {
        for (int j=0; j<MAPSCRS; j++)
        {
          scr=i*MAPSCRS+j;
          TheMaps[scr].door_combo_set=MakeDoors(i, j);
          for (int k=0; k<128; k++)
          {
            TheMaps[scr].secretcset[k]=tcmbcset2(i, TheMaps[scr].secretcombo[k]);
            TheMaps[scr].secretflag[k]=tcmbflag2(i, TheMaps[scr].secretcombo[k]);
            TheMaps[scr].secretcombo[k]=tcmbdat2(i, j, TheMaps[scr].secretcombo[k]);
          }
        }
      }
    }
  }
  return 0;
}

int readcombos(PACKFILE *f, zquestheader *Header, word version, word build, word start_combo, word max_combos, bool keepdata)
{
  //these are here to bypass compiler warnings about unused arguments
  Header=Header;

  reset_combo_animations();

  // combos
  word combos_used=0;
  int dummy;
  byte padding;
  newcombo temp_combo;
  word section_version=0;
  word section_cversion=0;
  if (keepdata==true)
  {
    memset(combobuf+start_combo,0,sizeof(newcombo)*max_combos);
  }

  if (version > 0x192)
  {
    //section version info
    if(!p_igetw(&section_version,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&section_cversion,f,true))
    {
      return qe_invalid;
    }

    //section size
    if(!p_igetl(&dummy,f,true))
    {
      return qe_invalid;
    }
  }

  if(version < 0x174)
  {
    combos_used=1024;
  }
  else if(version < 0x191)
  {
    combos_used=2048;
  }
  else
  {
    if(!p_igetw(&combos_used,f,true))
    {
      return qe_invalid;
    }
  }

  //finally...  section data
  for (int i=0; i<combos_used; i++)
  {
    memset(&temp_combo,0,sizeof(temp_combo));
    if(!p_igetw(&temp_combo.tile,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_combo.flip,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_combo.walk,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_combo.type,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_combo.csets,f,true))
    {
      return qe_invalid;
    }
    if (version < 0x193)
    {
      if(!p_getc(&padding,f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&padding,f,true))
      {
        return qe_invalid;
      }
      if (version < 0x192)
      {
        if (version == 0x191)
        {
          for(int tmpcounter=0; tmpcounter<16; tmpcounter++)
          {
            if(!p_getc(&padding,f,true))
            {
              return qe_invalid;
            }
          }
        }
        if (keepdata==true)
        {
          memcpy(&combobuf[i], &temp_combo, sizeof(temp_combo));
        }
        continue;
      }
    }

    if(!p_getc(&temp_combo.frames,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_combo.speed,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&temp_combo.nextcombo,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_combo.nextcset,f,true))
    {
      return qe_invalid;
    }

    if (version < 0x193)
    {
      for (int q=0; q<11; q++)
      {
        if(!p_getc(&dummy,f,true))
        {
          return qe_invalid;
        }
      }
    }
    if (keepdata==true)
    {
      memcpy(&combobuf[i], &temp_combo, sizeof(temp_combo));
    }
  }

  if (keepdata==true)
  {
    if ((version < 0x192)|| ((version == 0x192)&&(build<185)))
    {
      for(int tmpcounter=0; tmpcounter<MAXCOMBOS; tmpcounter++)
      {
        if (combobuf[tmpcounter].type==cHOOKSHOTONLY)
        {
          combobuf[tmpcounter].type=cLADDERHOOKSHOT;
        }
      }
    }
  }

  setup_combo_animations();
  return 0;
}

int readcolordata(PACKFILE *f, miscQdata *misc, word version, word build, word start_cset, word max_csets, bool keepdata)
{
  //these are here to bypass compiler warnings about unused arguments
  start_cset=start_cset;
  max_csets=max_csets;

  miscQdata temp_misc;
  memcpy(&temp_misc, misc, sizeof(temp_misc));

  byte temp_colordata[48];
  char temp_palname[PALNAMESIZE];

  int dummy;
  word palcycles;
  if (version > 0x192)
  {
    //section version info
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }

    //section size
    if(!p_igetl(&dummy,f,true))
    {
      return qe_invalid;
    }
  }

  //finally...  section data
  for (int i=0; i<oldpdTOTAL; ++i)
  {
    memset(temp_colordata, 0, 48);
    if(!pfread(temp_colordata,48,f,true))
    {
      return qe_invalid;
    }
    if (keepdata==true)
    {
      memcpy(&colordata[i*48], temp_colordata, 48);
    }
  }

  if ((version < 0x192)||((version == 0x192)&&(build<73)))
  {
    if (keepdata==true)
    {
      memcpy(colordata+(newpoSPRITE*48), colordata+(oldpoSPRITE*48), 30*16*3);
      memset(colordata+(oldpoSPRITE*48), 0, ((newpoSPRITE-oldpoSPRITE)*48));
      memcpy(colordata+((newpoSPRITE+11)*48), colordata+((newpoSPRITE+10)*48), 48);
      memcpy(colordata+((newpoSPRITE+10)*48), colordata+((newpoSPRITE+9)*48), 48);
      memcpy(colordata+((newpoSPRITE+9)*48), colordata+((newpoSPRITE+8)*48), 48);
      memset(colordata+((newpoSPRITE+8)*48), 0, 48);
    }
  }
  else
  {
    memset(temp_colordata, 0, 48);
    for (int i=0; i<newpdTOTAL-oldpdTOTAL; ++i)
    {
      if(!pfread(temp_colordata,48,f,true))
      {
        return qe_invalid;
      }
      if (keepdata==true)
      {
        memcpy(&colordata[(oldpdTOTAL+i)*48], temp_colordata, 48);
      }
    }
    if ((version < 0x192)||((version == 0x192)&&(build<76)))
    {
      if (keepdata==true)
      {
        init_palnames();
      }
    }
    else
    {
      for (int i=0; i<MAXLEVELS; ++i)
      {
        memset(temp_palname, 0, PALNAMESIZE);
        if(!pfread(temp_palname,PALNAMESIZE,f,true))
        {
          return qe_invalid;
        }
        if (keepdata==true)
        {
          memcpy(palnames[i], temp_palname, PALNAMESIZE);
        }
      }
    }
  }

  if (version > 0x192)
  {
    if(!p_igetw(&palcycles,f,true))
    {
      return qe_invalid;
    }
    for (int i=0; i<palcycles; i++)
    {
      for (int j=0; j<3; j++)
      {
        if(!p_getc(&temp_misc.cycles[i][j].first,f,true))
        {
          return qe_invalid;
        }
      }
      for (int j=0; j<3; j++)
      {
        if(!p_getc(&temp_misc.cycles[i][j].count,f,true))
        {
          return qe_invalid;
        }
      }
      for (int j=0; j<3; j++)
      {
        if(!p_getc(&temp_misc.cycles[i][j].speed,f,true))
        {
          return qe_invalid;
        }
      }
    }
    if (keepdata==true)
    {
      memcpy(misc, &temp_misc, sizeof(temp_misc));
    }
  }
  return 0;
}

int readtiles(PACKFILE *f, byte *buf, zquestheader *header, word version, word build, word start_tile, word max_tiles, bool from_init, bool keepdata)
{
  int dummy;
  word tiles_used=0;
  byte temp_tile[SINGLE_TILE_SIZE];

  if(header!=NULL&&(!header->data_flags[ZQ_TILES]&&!from_init))         //keep for old quests
  {
    if (keepdata==true)
    {
      packfile_password(NULL);
      init_tiles(true, header);
      packfile_password(datapwd);
    }
  }
  else
  {
    if (keepdata==true)
    {
      clear_tiles(tilebuf);
    }
    if (version > 0x192)
    {
      //section version info
      if(!p_igetw(&dummy,f,true))
      {
        return qe_invalid;
      }
      if(!p_igetw(&dummy,f,true))
      {
        return qe_invalid;
      }

      //section size
      if(!p_igetl(&dummy,f,true))
      {
        return qe_invalid;
      }
    }

    if(version < 0x174)
    {
      tiles_used=TILES_PER_PAGE*4;
    }                                                       //no expanded tile space
    else if(version < 0x191)
    {
      tiles_used=OLDMAXTILES;
    }
    else
    {
      //finally...  section data
      if(!p_igetw(&tiles_used,f,true))
      {
        return qe_invalid;
      }
    }

    tiles_used=min(tiles_used, max_tiles);

    tiles_used=min(tiles_used, NEWMAXTILES-start_tile);
    for (dword i=0; i<tiles_used; ++i)
    {
      memset(&temp_tile, 0, SINGLE_TILE_SIZE);
      if(!pfread(temp_tile,SINGLE_TILE_SIZE,f,true))
      {
        return qe_invalid;
      }
      if (keepdata==true)
      {
        memcpy(&buf[start_tile+(i*128)],temp_tile,SINGLE_TILE_SIZE);
      }
    }
  }

  if (keepdata==true)
  {
    if ((version < 0x192)|| ((version == 0x192)&&(build<186)))
    {
      if (get_bit(quest_rules,qr_BSZELDA))
      {
        byte tempbyte;
        int swimtile=wpnsbuf[iwSwim].tile;
        for (int i=0; i<SINGLE_TILE_SIZE; i++)
        {
          tempbyte=buf[(SINGLE_TILE_SIZE*23)+i];
          buf[(SINGLE_TILE_SIZE*23)+i]=buf[(SINGLE_TILE_SIZE*24)+i];
          buf[(SINGLE_TILE_SIZE*24)+i]=buf[(SINGLE_TILE_SIZE*25)+i];
          buf[(SINGLE_TILE_SIZE*25)+i]=buf[(SINGLE_TILE_SIZE*26)+i];
          buf[(SINGLE_TILE_SIZE*26)+i]=tempbyte;
        }

        for (int i=0; i<SINGLE_TILE_SIZE; i++)
        {
          tempbyte=buf[(SINGLE_TILE_SIZE*(swimtile+11))+i];
          buf[(SINGLE_TILE_SIZE*(swimtile+11))+i]=buf[(SINGLE_TILE_SIZE*(swimtile+12))+i];
          buf[(SINGLE_TILE_SIZE*(swimtile+12))+i]=tempbyte;
        }
      }
    }
    register_blank_tiles();
  }
  return 0;
}

int readmidis(PACKFILE *f, zquestheader *header, music *midis, bool keepdata)
{
  byte *mf;
  long dummy;
  music temp_midi;
  int midis_to_read;
  int midi_count=0;
  if (header->zelda_version < 0x193)
  {
//    mf=header->data_flags+ZQ_MIDIS2;
    mf=midi_flags;
    if ((header->zelda_version < 0x192)||((header->zelda_version == 0x192)&&(header->build<178)))
    {
      midis_to_read=MAXMIDIS192b177;
    }
    else
    {
      midis_to_read=MAXMIDIS;
    }
  }
  else
  {
    //section version info
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }

    //section size
    if(!p_igetl(&dummy,f,true))
    {
      return qe_invalid;
    }

    //finally...  section data
    if(!pfread(midi_flags,sizeof(midi_flags),f,true))
    {
      return qe_invalid;
    }
    mf=midi_flags;
    midis_to_read=MAXMIDIS;
  }
  for (int i=0; i<MAXMIDIS; ++i)
  {
    if (get_bit(mf, i))
    {
      ++midi_count;
    }
  }
  if (keepdata==true)
  {
    reset_midis(midis);
  }
  for(int i=0; i<midis_to_read; i++)
  {
    memset(&temp_midi,0,sizeof(temp_midi));
    if (keepdata==true)
    {
      reset_midi(midis+i);
    }
    if(get_bit(mf,i))
    {
      if(!pfread(&temp_midi.title,sizeof(temp_midi.title),f,true))
      {
        return qe_invalid;
      }
      if(!p_igetl(&temp_midi.start,f,true))
      {
        return qe_invalid;
      }
      if(!p_igetl(&temp_midi.loop_start,f,true))
      {
        return qe_invalid;
      }
      if(!p_igetl(&temp_midi.loop_end,f,true))
      {
        return qe_invalid;
      }
      if(!p_igetw(&temp_midi.loop,f,true))
      {
        return qe_invalid;
      }
      if(!p_igetw(&temp_midi.volume,f,true))
      {
        return qe_invalid;
      }
      if (header->zelda_version < 0x193)
      {
        if(!p_igetl(&dummy,f,true))
        {
          return qe_invalid;
        }
      }
      if (keepdata==true)
      {
        memcpy(&midis[i], &temp_midi, sizeof(temp_midi));
      }
      if(!((keepdata==true?midis[i].midi:temp_midi.midi)=read_midi(f, true)))
      {
        return qe_invalid;
      }
    }
  }
/*
  if ((header->zelda_version < 0x192)||((header->zelda_version == 0x192)&&(header->build<178)))
  {
    memset(header->data_flags+ZQ_MIDIS2,0,4);
  }
*/
  return 0;
}

int readcheatcodes(PACKFILE *f, zquestheader *header, bool keepdata)
{
  int dummy;
  ZCHEATS tempzcheats;
  char temp_use_cheats;
  memset(&tempzcheats, 0, sizeof(tempzcheats));
  if (header->zelda_version > 0x192)
  {
    //section version info
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }

    //section size
    if(!p_igetl(&dummy,f,true))
    {
      return qe_invalid;
    }

    //finally...  section data
    if(!p_getc(&temp_use_cheats,f,true))
    {
      return qe_invalid;
    }
  }

  if(header->data_flags[ZQ_CHEATS2])
  {
    if(!p_igetl(&tempzcheats.flags,f,true))
    {
      return qe_invalid;
    }
    if(!pfread(&tempzcheats.codes, sizeof(tempzcheats.codes), f,true))
    {
      return qe_invalid;
    }
  }

  if (keepdata==true)
  {
    memcpy(&zcheats, &tempzcheats, sizeof(tempzcheats));
    header->data_flags[ZQ_CHEATS2]=temp_use_cheats;
  }

  return 0;
}

int readinitdata(PACKFILE *f, zquestheader *header, bool keepdata)
{
  int dummy;
  byte padding;

  zinitdata temp_zinit;
  memset(&temp_zinit, 0, sizeof(zinitdata));

  if (header->zelda_version > 0x192)
  {
    //section version info
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&dummy,f,true))
    {
      return qe_invalid;
    }

    //section size
    if(!p_igetl(&dummy,f,true))
    {
      return qe_invalid;
    }
  }

  if ((header->zelda_version > 0x192)||((header->zelda_version == 0x192)&&(header->build>26)))
  {

    //finally...  section data
    if ((header->zelda_version > 0x192)||
                                                            //new only
      ((header->zelda_version == 0x192)&&(header->build>173)))
    {
      if(!p_getc(&temp_zinit.raft,f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&temp_zinit.ladder,f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&temp_zinit.book,f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&temp_zinit.key,f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&temp_zinit.flippers,f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&temp_zinit.boots,f,true))
      {
        return qe_invalid;
      }
    }

    if(!p_getc(&temp_zinit.ring,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.sword,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.shield,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.wallet,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.bracelet,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.amulet,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.bow,f,true))
    {
      return qe_invalid;
    }

                                                            //old only
    if ((header->zelda_version == 0x192)&&(header->build<174))
    {
      temp_zinit.ring=(temp_zinit.ring)?(1<<(temp_zinit.ring-1)):0;
      temp_zinit.sword=(temp_zinit.sword)?(1<<(temp_zinit.sword-1)):0;
      temp_zinit.shield=(temp_zinit.shield)?(1<<(temp_zinit.shield-1)):0;
      temp_zinit.wallet=(temp_zinit.wallet)?(1<<(temp_zinit.wallet-1)):0;
      temp_zinit.bracelet=(temp_zinit.bracelet)?(1<<(temp_zinit.bracelet-1)):0;
      temp_zinit.amulet=(temp_zinit.amulet)?(1<<(temp_zinit.amulet-1)):0;
      temp_zinit.bow=(temp_zinit.bow)?(1<<(temp_zinit.bow-1)):0;
    }

                                                            //new only
    if ((header->zelda_version == 0x192)&&(header->build>173))
    {
      for (int q=0; q<32; q++)
      {
        if(!p_getc(&padding,f,true))
        {
          return qe_invalid;
        }
      }
    }

    if(!p_getc(&temp_zinit.candle,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.boomerang,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.arrow,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.potion,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.whistle,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.bombs,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.super_bombs,f,true))
    {
      return qe_invalid;
    }

                                                            //old only
    if ((header->zelda_version == 0x192)&&(header->build<174))
    {
      temp_zinit.candle=(temp_zinit.candle)?(1<<(temp_zinit.candle-1)):0;
      temp_zinit.boomerang=(temp_zinit.boomerang)?(1<<(temp_zinit.boomerang-1)):0;
      temp_zinit.arrow=(temp_zinit.arrow)?(1<<(temp_zinit.arrow-1)):0;
      temp_zinit.whistle=(temp_zinit.whistle)?(1<<(temp_zinit.whistle-1)):0;
    }

    if ((header->zelda_version > 0x192)||
                                                            //new only
      ((header->zelda_version == 0x192)&&(header->build>173)))
    {
      if(!p_getc(&temp_zinit.wand,f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&temp_zinit.letter,f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&temp_zinit.lens,f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&temp_zinit.hookshot,f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&temp_zinit.bait,f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&temp_zinit.hammer,f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&temp_zinit.dins_fire,f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&temp_zinit.farores_wind,f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&temp_zinit.nayrus_love,f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&temp_zinit.cloak,f,true))
      {
        return qe_invalid;
      }

      if (header->zelda_version == 0x192)
      {
        for (int q=0; q<32; q++)
        {
          if(!p_getc(&padding,f,true))
          {
            return qe_invalid;
          }
        }
      }
    }

                                                            //old only
    if ((header->zelda_version == 0x192)&&(header->build<174))
    {
      byte equipment, items;                                //bit flags
      if(!p_getc(&equipment,f,true))
      {
        return qe_invalid;
      }
      temp_zinit.raft=get_bit(&equipment, idE_RAFT);
      temp_zinit.ladder=get_bit(&equipment, idE_LADDER);
      temp_zinit.book=get_bit(&equipment, idE_BOOK);
      temp_zinit.key=get_bit(&equipment, idE_KEY);
      temp_zinit.flippers=get_bit(&equipment, idE_FLIPPERS);
      temp_zinit.boots=get_bit(&equipment, idE_BOOTS);


      if(!p_getc(&items,f,true))
      {
        return qe_invalid;
      }
      temp_zinit.wand=get_bit(&items, idI_WAND);
      temp_zinit.letter=get_bit(&items, idI_LETTER);
      temp_zinit.lens=get_bit(&items, idI_LENS);
      temp_zinit.hookshot=get_bit(&items, idI_HOOKSHOT);
      temp_zinit.bait=get_bit(&items, idI_BAIT);
      temp_zinit.hammer=get_bit(&items, idI_HAMMER);
    }

    if(!p_getc(&temp_zinit.hc,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.start_heart,f,true))
    {
      return qe_invalid;

    }
    if(!p_getc(&temp_zinit.cont_heart,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.hcp,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.max_bombs,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.keys,f,true))
    {
      return qe_invalid;
    }
    if(!p_igetw(&temp_zinit.rupies,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.triforce,f,true))
    {
      return qe_invalid;
    }
    for (int i=0; i<32; i++)

    {
      if(!p_getc(&temp_zinit.map[i],f,true))
      {
        return qe_invalid;
      }
    }
    for (int i=0; i<32; i++)
    {
      if(!p_getc(&temp_zinit.compass[i],f,true))
      {
        return qe_invalid;
      }
    }

    if ((header->zelda_version > 0x192)||
                                                            //new only
      ((header->zelda_version == 0x192)&&(header->build>173)))
    {
      for (int i=0; i<32; i++)
      {
        if(!p_getc(&temp_zinit.boss_key[i],f,true))
        {
          return qe_invalid;
        }
      }
    }

    for (int i=0; i<16; i++)
    {
      if(!p_getc(&temp_zinit.misc[i],f,true))
      {
        return qe_invalid;
      }
    }
    for (int i=0; i<4; i++)
    {
      if(!p_getc(&temp_zinit.sword_hearts[i],f,true))
      {
        return qe_invalid;
      }
    }
    if(!p_getc(&temp_zinit.last_map,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.last_screen,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.max_magic,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.magic,f,true))
    {
      return qe_invalid;
    }
    for (int i=0; i<4; i++)
    {
      if(!p_getc(&temp_zinit.beam_hearts[i],f,true))
      {
        return qe_invalid;
      }
    }
    if(!p_getc(&temp_zinit.beam_percent,f,true))
    {
      return qe_invalid;
    }
    for (int i=0; i<4; i++)
    {
      if(!p_getc(&temp_zinit.beam_power[i],f,true))
      {
        return qe_invalid;
      }
    }
    if(!p_getc(&temp_zinit.hookshot_links,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.msg_more_x,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.msg_more_y,f,true))
    {
      return qe_invalid;
    }
    if(!p_getc(&temp_zinit.subscreen,f,true))
    {
      return qe_invalid;
    }

                                                            //old only
    if ((header->zelda_version == 0x192)&&(header->build<174))
    {
      for (int i=0; i<32; i++)
      {
        if(!p_getc(&temp_zinit.boss_key[i],f,true))
        {
          return qe_invalid;
        }
      }
    }

    if ((header->zelda_version > 0x192)||
                                                            //new only
      ((header->zelda_version == 0x192)&&(header->build>173)))
    {
      if(!p_getc(&temp_zinit.start_dmap,f,true))
      {
        return qe_invalid;
      }
      if(!p_getc(&temp_zinit.linkwalkstyle,f,true))
      {
        return qe_invalid;
      }
    }

                                                            //old only
    if ((header->zelda_version == 0x192)&&(header->build<174))
    {
      byte items2;
      if(!p_getc(&items2,f,true))
      {
        return qe_invalid;
      }
      temp_zinit.dins_fire=get_bit(&items2, idI_DFIRE);
      temp_zinit.farores_wind=get_bit(&items2, idI_FWIND);
      temp_zinit.nayrus_love=get_bit(&items2, idI_NLOVE);
    }

    if (header->zelda_version < 0x193)
    {
      for (int q=0; q<96; q++)
      {
        if(!p_getc(&padding,f,true))
        {
          return qe_invalid;
        }
      }

                                                            //new only
      if ((header->zelda_version == 0x192)&&(header->build>173))
      {
        if(!p_getc(&padding,f,true))
        {
          return qe_invalid;
        }
        if(!p_getc(&padding,f,true))
        {
          return qe_invalid;
        }
      }
    }
  }

  if ((header->zelda_version < 0x192)||
    ((header->zelda_version == 0x192)&&(header->build<27)))
  {
    temp_zinit.shield=i_smallshield;
    temp_zinit.hc=3;
    temp_zinit.start_heart=3;
    temp_zinit.cont_heart=3;
    temp_zinit.max_bombs=8;
  }
  if ((header->zelda_version < 0x192)||
    ((header->zelda_version == 0x192)&&(header->build<50)))
  {
    temp_zinit.sword_hearts[0]=0;
    temp_zinit.sword_hearts[1]=5;
    temp_zinit.sword_hearts[2]=12;
    temp_zinit.sword_hearts[3]=21;
  }

  if ((header->zelda_version < 0x192)||
    ((header->zelda_version == 0x192)&&(header->build<51)))
  {
    temp_zinit.last_map=0;
    temp_zinit.last_screen=0;
  }

  if ((header->zelda_version < 0x192)||
    ((header->zelda_version == 0x192)&&(header->build<68)))
  {
    temp_zinit.max_magic=0;
    temp_zinit.magic=0;
    set_bit(temp_zinit.misc,idM_DOUBLEMAGIC,0);
  }

  if ((header->zelda_version < 0x192)||
    ((header->zelda_version == 0x192)&&(header->build<129)))
  {

    for (int x=0; x<4; x++)
    {
      temp_zinit.beam_hearts[x]=100;
    }
    for(int i=0; i<idBP_MAX; i++)
    {
      set_bit(&(temp_zinit.beam_percent),i,!get_bit(quest_rules,qr_LENSHINTS+i));
      set_bit(quest_rules,qr_LENSHINTS+i,0);
    }
    for (int x=0; x<4; x++)
    {
      temp_zinit.beam_power[x]=get_bit(quest_rules,qr_HIDECARRIEDITEMS)?50:100;
    }
    set_bit(quest_rules,qr_HIDECARRIEDITEMS,0);
    temp_zinit.hookshot_links=100;
    temp_zinit.msg_more_x=224;
    temp_zinit.msg_more_y=64;
  }
  if ((header->zelda_version < 0x192)||
    ((header->zelda_version == 0x192)&&(header->build<168)))
  {
                                                            //was new subscreen rule
    temp_zinit.subscreen=get_bit(quest_rules,qr_FREEFORM)?1:0;
    set_bit(quest_rules,qr_FREEFORM,0);
  }
  if ((header->zelda_version < 0x192)||
    ((header->zelda_version == 0x192)&&(header->build<185)))
  {
    temp_zinit.start_dmap=0;
  }

  if ((header->zelda_version < 0x192)||
    ((header->zelda_version == 0x192)&&(header->build<186)))
  {
    temp_zinit.linkwalkstyle=get_bit(quest_rules,qr_BSZELDA)?1:0;
  }

  if (keepdata==true)
  {
    memcpy(&zinit, &temp_zinit, sizeof(zinitdata));
  }
  return 0;
}

/*
switch (ret) {
  case 0:
    break;

  case qe_invalid:
    goto invalid;
    break;
  default:
    pack_fclose(f);
    if(!oldquest)
      delete_file(tmpfilename);
return ret;
break;
}
*/

char *skip_text[skip_max]=
{
  "skip_header", "skip_rules", "skip_strings", "skip_misc", "skip_tiles", "skip_combos", "skip_comboaliases", "skip_csets", "skip_maps", "skip_dmaps", "skip_doors", "skip_items", "skip_weapons", "skip_colors", "skip_icons", "skip_initdata", "skip_guys", "skip_linksprites", "skip_subscreens", "skip_ffscript", "skip_sfx", "skip_midis", "skip_cheats"
};

int loadquest(char *filename, zquestheader *Header, miscQdata *Misc, music *midis, bool show_progress, bool compressed, bool encrypted, bool keepall, byte *skip_flags)
{
  if (debug&&(key[KEY_LSHIFT]||key[KEY_RSHIFT]))
  {
    keepall=false;
    jwin_alert("Load Quest","Data retention disabled.",NULL,NULL,"OK",NULL,13,27,lfont);
  }

  //  show_progress=true;
  char tmpbuf[L_tmpnam];
  char *tmpfilename = temp_name(tmpbuf);
  char percent_done[30];
  int current_method=0;
  bool catchup=false;
  byte tempbyte;
  word old_map_count=map_count;
  byte encryption_method=0;

  byte old_quest_rules[QUESTRULES_SIZE];
  byte old_midi_flags[MIDIFLAGS_SIZE];

  if (keepall==false||get_bit(skip_flags, skip_rules))
  {
    memcpy(old_quest_rules, quest_rules, QUESTRULES_SIZE);
  }
  if (keepall==false||get_bit(skip_flags, skip_midis))
  {
    memcpy(old_midi_flags, midi_flags, MIDIFLAGS_SIZE);
  }

  zquestheader tempheader;

  // oldquest flag is set when an unencrypted qst file is suspected.
  bool oldquest = false;
  PACKFILE *f=NULL;
  int ret;

  /*
    sprintf(error_message, "Bad value! - %c%c%c%c",
    (section_id>>24)&0xFF,(section_id>>16)&0xFF,
    (section_id>>8)&0xFF,section_id&0xFF);
    */
  if (show_progress)
  {
    box_start(1, "Loading Quest", lfont, font, true);
  }
  box_out("Loading Quest...");
  box_eol();
  box_eol();
  if(encrypted)
  {
    box_out("Decrypting...");
    box_save_x();

    encryption_method=ENC_METHOD_MAX-1;
    ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_MAX-1, strstr(filename, ".dat#")!=NULL);
    if(ret)
    {
      switch(ret)
      {
        case 1: box_out("error."); box_eol(); box_end(true); return qe_notfound;
        case 2: box_out("error."); box_eol(); box_end(true); return qe_internal;
        // be sure not to delete tmpfilename now...
      }
      if (ret==5)                                             //old encryption?
      {
        current_method++;
        sprintf(percent_done, "%d%%", (current_method*100)/ENC_METHOD_MAX);
        box_out(percent_done);
        box_load_x();
        encryption_method=ENC_METHOD_192B185;
        ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_192B185, strstr(filename, ".dat#")!=NULL);
      }
      if (ret==5)                                             //old encryption?
      {
        current_method++;
        sprintf(percent_done, "%d%%", (current_method*100)/ENC_METHOD_MAX);
        box_out(percent_done);
        box_load_x();
        encryption_method=ENC_METHOD_192B105;
        ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_192B105, strstr(filename, ".dat#")!=NULL);
      }
      if (ret==5)                                             //old encryption?
      {
        current_method++;
        sprintf(percent_done, "%d%%", (current_method*100)/ENC_METHOD_MAX);
        box_out(percent_done);
        box_load_x();
        encryption_method=ENC_METHOD_192B104;
        ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_192B104, strstr(filename, ".dat#")!=NULL);
      }
      if (ret)
      {
        oldquest = true;
      }
    }
    box_out("okay.");
    box_eol();
  }
  else
  {
    oldquest = true;
  }
  box_out("Opening...");
  f = pack_fopen(oldquest ? filename : tmpfilename, compressed ? F_READ_PACKED : F_READ);
  if(!f)
  {
    if((compressed==1)&&(errno==EDOM))
    {
      f = pack_fopen(oldquest ? filename : tmpfilename, F_READ);
    }
    if (!f)
    {
      if(!oldquest)
      {
        delete_file(tmpfilename);
      }
      box_out("error.");
      box_eol();
      box_end(true);
      return qe_invalid;
    }
  }

  box_out("okay.");
  box_eol();

 /* box_out("Copying to File Buffer...");
  ret = copyquest(f);
  checkstatus(ret);
  box_out("okay.");
  box_eol();*/
  //header
  box_out("Reading Header...");
  ret=readheader(f, &tempheader, true);
  checkstatus(ret);
  box_out("okay.");
  box_eol();

  if (tempheader.zelda_version>=0x193)
  {
    dword section_id;
    //section id
    if(!p_mgetl(&section_id,f,true))
    {
      return qe_invalid;
    }
    while (!pack_feof(f))
    {
      switch (section_id)
      {
        case ID_RULES:
          //rules
          if (catchup)
          {
            box_out("found.");
            box_eol();
            catchup=false;
          }
          box_out("Reading Rules...");
          ret=readrules(f, &tempheader, keepall&&!get_bit(skip_flags, skip_rules));
          checkstatus(ret);
          box_out("okay.");
          box_eol();
          break;
        case ID_STRINGS:
          //strings
          if (catchup)
          {
            box_out("found.");
            box_eol();
            catchup=false;
          }
          box_out("Reading Strings...");
          ret=readstrings(f, &tempheader, keepall&&!get_bit(skip_flags, skip_strings));
          checkstatus(ret);
          box_out("okay.");
          box_eol();
          break;
        case ID_MISC:
          //misc data
          if (catchup)
          {
            box_out("found.");
            box_eol();
            catchup=false;
          }
          box_out("Reading Misc. Data...");
          ret=readmisc(f, &tempheader, Misc, keepall&&!get_bit(skip_flags, skip_misc));
          checkstatus(ret);
          box_out("okay.");
          box_eol();
          break;
        case ID_TILES:
          //tiles
          if (catchup)
          {
            box_out("found.");
            box_eol();
            catchup=false;
          }
          box_out("Reading Tiles...");
          ret=readtiles(f, tilebuf, &tempheader, tempheader.zelda_version, tempheader.build, 0, NEWMAXTILES, false, keepall&&!get_bit(skip_flags, skip_tiles));
          checkstatus(ret);
          box_out("okay.");
          box_eol();
          break;
        case ID_COMBOS:
          //combos
          if (catchup)
          {
            box_out("found.");
            box_eol();
            catchup=false;
          }
          box_out("Reading Combos...");
          ret=readcombos(f, &tempheader, tempheader.zelda_version, tempheader.build, 0, MAXCOMBOS, keepall&&!get_bit(skip_flags, skip_combos));
          checkstatus(ret);
          box_out("okay.");
          box_eol();
          break;
        case ID_CSETS:
          //color data
          if (catchup)
          {
            box_out("found.");
            box_eol();
            catchup=false;
          }
          box_out("Reading Color Data...");
          ret=readcolordata(f, Misc, tempheader.zelda_version, tempheader.build, 0, newpdTOTAL, keepall&&!get_bit(skip_flags, skip_csets));
          checkstatus(ret);
          box_out("okay.");
          box_eol();
          break;
        case ID_MAPS:
          //maps
          if (catchup)
          {
            box_out("found.");
            box_eol();
            catchup=false;
          }
          box_out("Reading Maps...");
          ret=readmaps(f, &tempheader, keepall&&!get_bit(skip_flags, skip_maps));
          checkstatus(ret);
          box_out("okay.");
          box_eol();
          break;
        case ID_DMAPS:
          //dmaps
          if (catchup)
          {
            box_out("found.");
            box_eol();
            catchup=false;
          }
          box_out("Reading DMaps...");
          ret=readdmaps(f, &tempheader, tempheader.zelda_version, tempheader.build, 0, MAXDMAPS, keepall&&!get_bit(skip_flags, skip_dmaps));
          checkstatus(ret);
          box_out("okay.");
          box_eol();
          break;
        case ID_DOORS:
          //door combo sets
          if (catchup)
          {
            box_out("found.");
            box_eol();
            catchup=false;
          }
          box_out("Reading Doors...");
          ret=readdoorcombosets(f, &tempheader, keepall&&!get_bit(skip_flags, skip_doors));
          checkstatus(ret);
          box_out("okay.");
          box_eol();
          break;
        case ID_ITEMS:
          //items
          if (catchup)
          {
            box_out("found.");
            box_eol();
            catchup=false;
          }
          box_out("Reading Items...");
          ret=readitems(f, tempheader.zelda_version, tempheader.build, keepall&&!get_bit(skip_flags, skip_items));
          checkstatus(ret);
          box_out("okay.");
          box_eol();
          break;
        case ID_WEAPONS:
          //weapons
          if (catchup)
          {
            box_out("found.");
            box_eol();
            catchup=false;
          }
          box_out("Reading Weapons...");
          ret=readweapons(f, &tempheader, keepall&&!get_bit(skip_flags, skip_weapons));
          checkstatus(ret);
          box_out("okay.");
          box_eol();
          break;
        case ID_COLORS:
          break;
        case ID_ICONS:
          break;
        case ID_INITDATA:
          //initialization data
          if (catchup)
          {
            box_out("found.");
            box_eol();
            catchup=false;
          }
          box_out("Reading Init. Data...");
          ret=readinitdata(f, &tempheader, keepall&&!get_bit(skip_flags, skip_initdata));
          checkstatus(ret);
          box_out("okay.");
          box_eol();
          break;
        case ID_GUYS:
          //guys
          if (catchup)
          {
            box_out("found.");
            box_eol();
            catchup=false;
          }
          box_out("Reading Custom Guy Data...");
          ret=readguys(f, &tempheader, keepall&&!get_bit(skip_flags, skip_guys));
          checkstatus(ret);
          box_out("okay.");
          box_eol();
          break;
        case ID_MIDIS:
          //midis
          if (catchup)
          {
            box_out("found.");
            box_eol();
            catchup=false;
          }
          box_out("Reading MIDIs...");
          ret=readmidis(f, &tempheader, midis, keepall&&!get_bit(skip_flags, skip_midis));
          checkstatus(ret);
          box_out("okay.");
          box_eol();
          break;
        case ID_CHEATS:
          //cheat codes
          if (catchup)
          {
            box_out("found.");
            box_eol();
            catchup=false;
          }
          box_out("Reading Cheat Codes...");
          ret=readcheatcodes(f, &tempheader, keepall&&!get_bit(skip_flags, skip_cheats));
          checkstatus(ret);
          box_out("okay.");
          box_eol();
          break;
        default:
          if (!catchup)
          {
            box_out("Bad token!  Searching...");
            box_eol();
          }
          catchup=true;
          break;
      }
      if (catchup)
      {
        //section id
        section_id=(section_id<<8);
        if(!p_getc(&tempbyte,f,true))
        {
          return qe_invalid;
        }
        section_id+=tempbyte;
      }

      else
      {
        //section id
        if (!pack_feof(f))
        {
          if(!p_mgetl(&section_id,f,true))
          {
            return qe_invalid;
          }
        }
      }
    }
  }
  else
  {
    //rules
    box_out("Reading Rules...");
    ret=readrules(f, &tempheader, keepall&&!get_bit(skip_flags, skip_rules));
    checkstatus(ret);
    box_out("okay.");
    box_eol();

    //strings
    box_out("Reading Strings...");
    ret=readstrings(f, &tempheader, keepall&&!get_bit(skip_flags, skip_strings));
    checkstatus(ret);
    box_out("okay.");
    box_eol();

    //door combo sets
    box_out("Reading Doors...");
    ret=readdoorcombosets(f, &tempheader, keepall&&!get_bit(skip_flags, skip_doors));
    checkstatus(ret);
    box_out("okay.");
    box_eol();

    //dmaps
    box_out("Reading DMaps...");
    ret=readdmaps(f, &tempheader, tempheader.zelda_version, tempheader.build, 0, MAXDMAPS, keepall&&!get_bit(skip_flags, skip_dmaps));
    checkstatus(ret);
    box_out("okay.");
    box_eol();

    // misc data
    box_out("Reading Misc. Data...");
    ret=readmisc(f, &tempheader, Misc, keepall&&!get_bit(skip_flags, skip_misc));
    checkstatus(ret);
    box_out("okay.");
    box_eol();

    //items
    box_out("Reading Items...");
    ret=readitems(f, tempheader.zelda_version, tempheader.build, keepall&&!get_bit(skip_flags, skip_items));
    checkstatus(ret);
    box_out("okay.");
    box_eol();

    //weapons
    box_out("Reading Weapons...");
    ret=readweapons(f, &tempheader, keepall&&!get_bit(skip_flags, skip_weapons));
    checkstatus(ret);
    box_out("okay.");
    box_eol();

    //guys
    box_out("Reading Custom Guy Data...");
    ret=readguys(f, &tempheader, keepall&&!get_bit(skip_flags, skip_guys));
    checkstatus(ret);
    box_out("okay.");
    box_eol();

    //maps
    box_out("Reading Maps...");
    ret=readmaps(f, &tempheader, keepall&&!get_bit(skip_flags, skip_maps));
    checkstatus(ret);
    box_out("okay.");
    box_eol();

    //combos
    box_out("Reading Combos...");
    ret=readcombos(f, &tempheader, tempheader.zelda_version, tempheader.build, 0, MAXCOMBOS, keepall&&!get_bit(skip_flags, skip_combos));
    checkstatus(ret);
    box_out("okay.");
    box_eol();

    //color data
    box_out("Reading Color Data...");
    ret=readcolordata(f, Misc, tempheader.zelda_version, tempheader.build, 0, newpdTOTAL, keepall&&!get_bit(skip_flags, skip_csets));
    checkstatus(ret);
    box_out("okay.");
    box_eol();

    //tiles
    box_out("Reading Tiles...");
    ret=readtiles(f, tilebuf, &tempheader, tempheader.zelda_version, tempheader.build, 0, NEWMAXTILES, false, keepall&&!get_bit(skip_flags, skip_tiles));
    checkstatus(ret);
    box_out("okay.");
    box_eol();

    //midis
    box_out("Reading MIDIs...");
    ret=readmidis(f, &tempheader, midis, keepall&&!get_bit(skip_flags, skip_midis));
    checkstatus(ret);
    box_out("okay.");
    box_eol();

    //cheat codes
    box_out("Reading Cheat Codes...");
    ret=readcheatcodes(f, &tempheader, keepall&&!get_bit(skip_flags, skip_cheats));
    checkstatus(ret);
    box_out("okay.");
    box_eol();

    //initialization data
    box_out("Reading Init. Data...");
    ret=readinitdata(f, &tempheader, keepall&&!get_bit(skip_flags, skip_initdata));
    checkstatus(ret);
    box_out("okay.");
    box_eol();

  }

  // check data
  if (f)
  {
    pack_fclose(f);
  }
  if(!oldquest)
  {
    if (exists(tmpfilename))
    {
      delete_file(tmpfilename);
    }
  }
  box_out("Done."); box_eol(); box_end(false);
//  if (keepall==true||!get_bit(skip_flags, skip_header))
  if (keepall&&!get_bit(skip_flags, skip_header))
  {
    memcpy(Header, &tempheader, sizeof(tempheader));
  }
  if (!keepall||get_bit(skip_flags, skip_maps))
  {
    map_count=old_map_count;
  }
  if (!keepall||get_bit(skip_flags, skip_rules))
  {
    memcpy(quest_rules, old_quest_rules, QUESTRULES_SIZE);
  }
  if (!keepall||get_bit(skip_flags, skip_midis))
  {
    memcpy(midi_flags, old_midi_flags, MIDIFLAGS_SIZE);
  }
  return qe_OK;

invalid:
  box_out("error."); box_eol(); box_end(true);
  if (f)
  {
    pack_fclose(f);
  }
  if(!oldquest)
  {
    if (exists(tmpfilename))
    {
      delete_file(tmpfilename);
    }
  }
  return qe_invalid;

}

/*** end of qst.cc ***/
