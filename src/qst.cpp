//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  qst.cc
//
//  Code for loading '.qst' files in ZC and ZQuest.
//
//--------------------------------------------------------

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include <stdio.h>
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <assert.h>

#include "mem_debug.h"
#include "zc_alleg.h"
#include "zdefs.h"
#include "colors.h"
#include "tiles.h"
#include "zsys.h"
#include "qst.h"
#include "zquest.h"
#include "defdata.h"
#include "subscr.h"
#include "font.h"
#include "zc_custom.h"
#include "sfx.h"
#include "md5.h"
#include "ffscript.h"
FFScript ffcore;
//FFSCript   FFEngine;

#ifdef _MSC_VER
	#define strncasecmp _strnicmp
#endif

#ifndef _AL_MALLOC
#define _AL_MALLOC(a) _al_malloc(a)
#define _AL_FREE(a) _al_free(a)
#endif

using std::string;
using std::pair;

// extern bool                debug;
extern int                 link_animation_speed; //lower is faster animation
extern std::vector<mapscr> TheMaps;
extern zcmap               *ZCMaps;
extern MsgStr              *MsgStrings;
extern DoorComboSet        *DoorComboSets;
extern dmap                *DMaps;
extern newcombo            *combobuf;
extern byte                *colordata;
//extern byte              *tilebuf;
extern tiledata            *newtilebuf;
extern byte                *trashbuf;
extern itemdata            *itemsbuf;
extern wpndata             *wpnsbuf;
extern comboclass          *combo_class_buf;
extern guydata             *guysbuf;
extern ZCHEATS             zcheats;
extern zinitdata           zinit;
extern char                palnames[MAXLEVELS][17];
extern int                 memrequested;
extern char                *byte_conversion(int number, int format);
extern char                *byte_conversion2(int number1, int number2, int format1, int format2);
string				             zScript;
std::map<int, pair<string,string> > ffcmap;
std::map<int, pair<string,string> > globalmap;
std::map<int, pair<string,string> > itemmap;

bool combosread=false;
bool mapsread=false;
bool fixffcs=false;
bool fixpolsvoice=false;

int memDBGwatch[8]= {0,0,0,0,0,0,0,0}; //So I can monitor memory crap

//enum { qe_OK, qe_notfound, qe_invalid, qe_version, qe_obsolete,
//       qe_missing, qe_internal, qe_pwd, qe_match, qe_minver };

extern combo_alias combo_aliases[MAXCOMBOALIASES];
const char *qst_error[] =
{
    "OK","File not found","Invalid quest file",
    "Version not supported","Obsolete version",
    "Missing new data"  ,                                     /* but let it pass in ZQuest */
    "Internal error occurred", "Invalid password",
    "Doesn't match saved game", "New quest version; please restart game",
    "Out of memory", "File Debug Mode"
};

//for legacy quests -DD
enum { ssiBOMB, ssiSWORD, ssiSHIELD, ssiCANDLE, ssiLETTER, ssiPOTION, ssiLETTERPOTION, ssiBOW, ssiARROW, ssiBOWANDARROW, ssiBAIT, ssiRING, ssiBRACELET, ssiMAP,
       ssiCOMPASS, ssiBOSSKEY, ssiMAGICKEY, ssiBRANG, ssiWAND, ssiRAFT, ssiLADDER, ssiWHISTLE, ssiBOOK, ssiWALLET, ssiSBOMB, ssiHCPIECE, ssiAMULET, ssiFLIPPERS,
       ssiHOOKSHOT, ssiLENS, ssiHAMMER, ssiBOOTS, ssiDINSFIRE, ssiFARORESWIND, ssiNAYRUSLOVE, ssiQUIVER, ssiBOMBBAG, ssiCBYRNA, ssiROCS, ssiHOVERBOOTS,
       ssiSPINSCROLL, ssiCROSSSCROLL, ssiQUAKESCROLL, ssiWHISPRING, ssiCHARGERING, ssiPERILSCROLL, ssiWEALTHMEDAL, ssiHEARTRING, ssiMAGICRING, ssiSPINSCROLL2,
       ssiQUAKESCROLL2, ssiAGONY, ssiSTOMPBOOTS, ssiWHIMSICALRING, ssiPERILRING, ssiMAX
     };

static byte deprecated_rules[QUESTRULES_SIZE];


void delete_combo_aliases()
{
    for(int j(0); j<256; j++)
    {
        if(combo_aliases[j].combos != NULL)
        {
            delete[] combo_aliases[j].combos;
            combo_aliases[j].combos=NULL;
        }
        
        if(combo_aliases[j].csets != NULL)
        {
            delete[] combo_aliases[j].csets;
            combo_aliases[j].csets=NULL;
        }
    }
    
}


char *VerStr(int version)
{
    static char ver_str[12];
    sprintf(ver_str,"v%d.%02X",version>>8,version&0xFF);
    return ver_str;
}

char *byte_conversion(int number, int format)
{
    static char num_str[40];
    
    if(format==-1)                                            //auto
    {
        format=1;                                               //bytes
        
        if(number>1024)
        {
            format=2;                                             //kilobytes
        }
        
        if(number>1024*1024)
        {
            format=3;                                             //megabytes
        }
        
        if(number>1024*1024*1024)
        {
            format=4;                                             //gigabytes (dude, what are you doing?)
        }
    }
    
    switch(format)
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
    
    if(format1==-1)                                           //auto
    {
        format1=1;                                              //bytes
        
        if(number1>1024)
        {
            format1=2;                                            //kilobytes
        }
        
        if(number1>1024*1024)
        {
            format1=3;                                            //megabytes
        }
        
        if(number1>1024*1024*1024)
        {
            format1=4;                                            //gigabytes (dude, what are you doing?)
        }
    }
    
    if(format2==-1)                                           //auto
    {
        format2=1;                                              //bytes
        
        if(number2>1024)
        {
            format2=2;                                            //kilobytes
        }
        
        if(number2>1024*1024)
        {
            format2=3;                                            //megabytes
        }
        
        if(number2>1024*1024*1024)
        {
            format2=4;                                            //gigabytes (dude, what are you doing?)
        }
    }
    
    switch(format1)
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
    
    switch(format2)
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
    static const char *ending[4] = {"st","nd","rd","th"};
    static char ord_str[8];
    
    char *end;
    int t=(num%100)/10;
    int n=num%10;
    
    if(n>=1 && n<4 && t!=1)
        end = (char *)ending[n-1];
    else
        end = (char *)ending[3];
        
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
    
    if(!f)
    {
        return qe_invalid;
    }
    
    ret=readheader(f, &tempheader, true);
    
    if(ret)
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

    if(!f)
    {
        return false;
    }
    
    long section_id_read;
    bool catchup=false;
    word dummy;
    byte tempbyte;
    char tempbuf[65536];
    
    
    switch(section_id_requested)
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
        al_trace("Bad section requested!\n");
        return false;
        break;
    }
    
    dword section_size;
    
    //section id
    if(!p_mgetl(&section_id_read,f,true))
    {
        return false;
    }
    
    while(!pack_feof(f))
    {
        switch(section_id_read)
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
        
        
        while(catchup)
        {
            //section id
            section_id_read=(section_id_read<<8);
            
            if(!p_getc(&tempbyte,f,true))
            {
                return false;
            }
            
            section_id_read+=tempbyte;
        }
        
        if(section_id_read==section_id_requested)
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
            
            if(section_size>0)
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

    //word tiles_used;
    //word combos_used;
    //open the file
    //PACKFILE *f = pack_fopen(path, F_READ_PACKED);
    if(!f)
        return false;
        
    //for now, everything else is valid
    return true;
    
    /*short version;
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
      if(!pfread(trashbuf,tilesize(tf4Bit),f,true))
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
    return false;*/
}

bool valid_zqt(const char *filename)
{
    PACKFILE *f=NULL;
    bool isvalid;
    char deletefilename[1024];
    deletefilename[0]=0;
    int error;
    f=open_quest_file(&error, filename, deletefilename, true, true,false);
    
    if(!f)
    {
//      setPackfilePassword(NULL);
        return false;
    }
    
    isvalid=valid_zqt(f);
    
    if(deletefilename[0])
    {
        delete_file(deletefilename);
    }
    
//  setPackfilePassword(NULL);
    return isvalid;
}


PACKFILE *open_quest_file(int *open_error, const char *filename, char *deletefilename, bool compressed,bool encrypted, bool show_progress)
{
	char tmpfilename[32];
	temp_name(tmpfilename);
	char percent_done[30];
	int current_method=0;
    
	PACKFILE *f;
	const char *passwd= encrypted ? datapwd : "";
    
	// oldquest flag is set when an unencrypted qst file is suspected.
	bool oldquest = false;
	int ret;
    
	if(show_progress)
	{
		box_start(1, "Loading Quest", lfont, font, true);
	}
    
	box_out("Loading Quest: ");
	if(strncasecmp(filename, "qst.dat", 7)!=0)
		box_out(filename);
	else
		box_out("new quest"); // Or whatever
	box_out("...");
	box_eol();
	box_eol();
    
	if(encrypted)
	{
		box_out("Decrypting...");
		box_save_x();
		ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_MAX-1, strstr(filename, ".dat#")!=NULL, passwd);
        
		if(ret)
		{
			switch(ret)
			{
			case 1:
				box_out("error.");
				box_eol();
				box_end(true);
				*open_error=qe_notfound;
				return NULL;
                
			case 2:
				box_out("error.");
				box_eol();
				box_end(true);
				*open_error=qe_internal;
				return NULL;
				// be sure not to delete tmpfilename now...
			}
            
			if(ret==5)                                              //old encryption?
			{
				current_method++;
				sprintf(percent_done, "%d%%", (current_method*100)/ENC_METHOD_MAX);
				box_out(percent_done);
				box_load_x();
				ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_211B9, strstr(filename, ".dat#")!=NULL, passwd);
			}
            
			if(ret==5)                                              //old encryption?
			{
				current_method++;
				sprintf(percent_done, "%d%%", (current_method*100)/ENC_METHOD_MAX);
				box_out(percent_done);
				box_load_x();
				ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_192B185, strstr(filename, ".dat#")!=NULL, passwd);
			}
            
			if(ret==5)                                              //old encryption?
			{
				current_method++;
				sprintf(percent_done, "%d%%", (current_method*100)/ENC_METHOD_MAX);
				box_out(percent_done);
				box_load_x();
				ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_192B105, strstr(filename, ".dat#")!=NULL, passwd);
			}
            
			if(ret==5)                                              //old encryption?
			{
				current_method++;
				sprintf(percent_done, "%d%%", (current_method*100)/ENC_METHOD_MAX);
				box_out(percent_done);
				box_load_x();
				ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_192B104, strstr(filename, ".dat#")!=NULL, passwd);
			}
            
			if(ret)
			{
				oldquest = true;
				passwd="";
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
	f = pack_fopen_password(oldquest ? filename : tmpfilename, compressed ? F_READ_PACKED : F_READ, passwd);
    
	if(!f)
	{
		if((compressed==1)&&(errno==EDOM))
		{
			f = pack_fopen_password(oldquest ? filename : tmpfilename, F_READ, passwd);
		}
        
		if(!f)
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
		if(deletefilename)
			sprintf(deletefilename, "%s", tmpfilename);
	}
    
	box_out("okay.");
	box_eol();
    
	return f;
}

PACKFILE *open_quest_template(zquestheader *Header, char *deletefilename, bool validate)
{
    char *filename;
    PACKFILE *f=NULL;
    int open_error=0;
    deletefilename[0]=0;
    
    if(Header->templatepath[0]==0)
    {
        filename=(char *)zc_malloc(23);
        sprintf(filename, "qst.dat#NESQST_NEW_QST");
    }
    else
    {
        filename=Header->templatepath;
    }
    
    f=open_quest_file(&open_error, filename, deletefilename, true, true,false);
    
    if(Header->templatepath[0]==0)
    {
        zc_free(filename);
    }
    
    if(!f)
    {
        return NULL;
    }
    
    if(validate)
    {
        if(!valid_zqt(f))
        {
            jwin_alert("Error","Invalid Quest Template",NULL,NULL,"O&K",NULL,'k',0,lfont);
            pack_fclose(f);
            
            //setPackfilePassword(NULL);
            if(deletefilename[0])
            {
                delete_file(deletefilename);
            }
            
            return NULL;
        }
    }
    
    return f;
}

bool init_section(zquestheader *Header, long section_id, miscQdata *Misc, zctune *tunes, bool validate)
{
    combosread=false;
    mapsread=false;
    fixffcs=false;
    
    switch(section_id)
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
    case ID_ITEMDROPSETS:
    case ID_FAVORITES:
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
    
    //setPackfilePassword(datapwd);
    f=open_quest_template(Header, deletefilename, validate);
    
    if(!f)  //no file, nothing to delete
    {
//	  setPackfilePassword(NULL);
        return false;
    }
    
    ret=get_version_and_build(f, &version, &build);
    
    if(ret||(version==0))
    {
        pack_fclose(f);
        
        if(deletefilename[0])
        {
            delete_file(deletefilename);
        }
        
//	setPackfilePassword(NULL);
        return false;
    }
    
    if(!find_section(f, section_id))
    {
        al_trace("Can't find section!\n");
        pack_fclose(f);
        
        if(deletefilename[0])
        {
            delete_file(deletefilename);
        }
        
        //setPackfilePassword(NULL);
        return false;
    }
    
    switch(section_id)
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
        ret=readtiles(f, newtilebuf, Header, version, build, 0, NEWMAXTILES, true, true);
        break;
        
    case ID_COMBOS:
        //combos
        clear_combos();
        ret=readcombos(f, Header, version, build, 0, MAXCOMBOS, true);
        combosread=true;
        break;
        
    case ID_COMBOALIASES:
        //combos
        ret=readcomboaliases(f, Header, version, build, true);
        break;
        
    case ID_CSETS:
        //color data
        ret=readcolordata(f, Misc, version, build, 0, newerpdTOTAL, true);
        break;
        
    case ID_MAPS:
        //maps
        ret=readmaps(f, Header, true);
        mapsread=true;
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
        //misc. colors
        ret=readmisccolors(f, Header, Misc, true);
        break;
        
    case ID_ICONS:
        //game icons
        ret=readgameicons(f, Header, Misc, true);
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
        ret=readtunes(f, Header, tunes, true);
        break;
        
    case ID_CHEATS:
        //cheat codes
        ret=readcheatcodes(f, Header, true);
        break;
        
    case ID_ITEMDROPSETS:
        //item drop sets
        // Why is this one commented out?
        //ret=readitemdropsets(f, (int)version, (word)build, true);
        break;
        
    case ID_FAVORITES:
        // favorite combos and aliases
        ret=readfavorites(f, version, build, true);
        break;
        
    default:
        ret=-1;
        break;
    }
    
    pack_fclose(f);
    
    if(deletefilename[0])
    {
        delete_file(deletefilename);
    }
    
    //setPackfilePassword(NULL);
    if(!ret)
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
    bool ret = init_section(Header, ID_ITEMS, NULL, NULL, validate);
    
    //Ignore this, but don't remove it
    /*
    if (ret)
      for(int i=0; i<MAXITEMS; i++)
      {
        reset_itembuf(&itemsbuf[i], i);
      }
    */
    for(int i=0; i<MAXITEMS; i++) reset_itemname(i);
    
    return ret;
}

bool reset_guys()
{
    // The .dat file's guys definitions are always synchronised with defdata.cpp's - even the tile settings.
    init_guys(V_GUYS);
    return true;
}

bool reset_wpns(bool validate, zquestheader *Header)
{
    bool ret = init_section(Header, ID_WEAPONS, NULL, NULL, validate);
    
    for(int i=0; i<WPNCNT; i++)
        reset_weaponname(i);
        
    return ret;
}

bool reset_mapstyles(bool validate, miscQdata *Misc)
{
    Misc->colors.new_blueframe_tile = 20044;
    Misc->colors.blueframe_cset = 0;
    Misc->colors.new_triforce_tile = 23461;
    Misc->colors.triforce_cset = 1;
    Misc->colors.new_triframe_tile = 18752;
    Misc->colors.triframe_cset = 1;
    Misc->colors.new_overworld_map_tile = 16990;
    Misc->colors.overworld_map_cset = 2;
    Misc->colors.new_HCpieces_tile = 21160;
    Misc->colors.HCpieces_cset = 8;
    Misc->colors.new_dungeon_map_tile = 19651;
    Misc->colors.dungeon_map_cset = 8;
    return true;
}

bool reset_doorcombosets(bool validate, zquestheader *Header)
{
    return init_section(Header, ID_DOORS, NULL, NULL, validate);
}

int get_qst_buffers()
{
    memrequested+=(sizeof(mapscr)*MAPSCRS);
    Z_message("Allocating map buffer (%s)... ", byte_conversion2(sizeof(mapscr)*MAPSCRS,memrequested,-1, -1));
    TheMaps.resize(MAPSCRS);
    
    for(int i(0); i<MAPSCRS; i++)
        TheMaps[i].zero_memory();
        
    //memset(TheMaps, 0, sizeof(mapscr)*MAPSCRS); //shouldn't need this anymore
    Z_message("OK\n"); // Allocating map buffer...
    
    memrequested+=(sizeof(zcmap)*MAXMAPS2);
    Z_message("Allocating combo buffer (%s)... ", byte_conversion2(sizeof(zcmap)*MAXMAPS2,memrequested,-1,-1));
    
    if((ZCMaps=(zcmap*)zc_malloc(sizeof(zcmap)*MAXMAPS2))==NULL)
        return 0;
        
    Z_message("OK\n");
    
    // Allocating space for all 65535 strings uses up 10.62MB...
    // The vast majority of finished quests (and I presume this will be consistent for all time) use < 1000 strings in total.
    // (Shoelace's "Hero of Dreams" uses 1415.)
    // So let's be a bit generous and allow 4096 initially.
    // In the rare event that a quest overshoots this mark, we'll reallocate to the full 65535 later.
    // I tested it and it worked without flaw on 6/6/11. - L.
    msg_strings_size = 4096;
    memrequested+=(sizeof(MsgStr)*msg_strings_size);
    Z_message("Allocating string buffer (%s)... ", byte_conversion2(sizeof(MsgStr)*msg_strings_size,memrequested,-1,-1));
    
    if((MsgStrings=(MsgStr*)zc_malloc(sizeof(MsgStr)*msg_strings_size))==NULL)
        return 0;
        
    memset(MsgStrings, 0, sizeof(MsgStr)*msg_strings_size);
    Z_message("OK\n");                                        // Allocating string buffer...
    
    memrequested+=(sizeof(DoorComboSet)*MAXDOORCOMBOSETS);
    Z_message("Allocating door combo buffer (%s)... ", byte_conversion2(sizeof(DoorComboSet)*MAXDOORCOMBOSETS,memrequested,-1,-1));
    
    if((DoorComboSets=(DoorComboSet*)zc_malloc(sizeof(DoorComboSet)*MAXDOORCOMBOSETS))==NULL)
        return 0;
        
    Z_message("OK\n");                                        // Allocating door combo buffer...
    
    memrequested+=(sizeof(dmap)*MAXDMAPS);
    Z_message("Allocating dmap buffer (%s)... ", byte_conversion2(sizeof(dmap)*MAXDMAPS,memrequested,-1,-1));
    
    if((DMaps=(dmap*)zc_malloc(sizeof(dmap)*MAXDMAPS))==NULL)
        return 0;
        
    memset(DMaps, 0, sizeof(dmap)*MAXDMAPS);
    Z_message("OK\n");                                        // Allocating dmap buffer...
    
    memrequested+=(sizeof(newcombo)*MAXCOMBOS);
    Z_message("Allocating combo buffer (%s)... ", byte_conversion2(sizeof(newcombo)*MAXCOMBOS,memrequested,-1,-1));
    
    if((combobuf=(newcombo*)zc_malloc(sizeof(newcombo)*MAXCOMBOS))==NULL)
        return 0;
        
    memset(combobuf, 0, sizeof(newcombo)*MAXCOMBOS);
    Z_message("OK\n");                                        // Allocating combo buffer...
    
    memrequested+=(newerpsTOTAL);
    Z_message("Allocating color data buffer (%s)... ", byte_conversion2(newerpsTOTAL,memrequested,-1,-1));
    
    if((colordata=(byte*)zc_malloc(newerpsTOTAL))==NULL)
        return 0;
        
    Z_message("OK\n");                                        // Allocating color data buffer...
    
    memrequested+=(NEWMAXTILES*(sizeof(tiledata)+tilesize(tf4Bit)));
    Z_message("Allocating tile buffer (%s)... ", byte_conversion2(NEWMAXTILES*(sizeof(tiledata)+tilesize(tf4Bit)),memrequested,-1,-1));
    
    if((newtilebuf=(tiledata*)zc_malloc(NEWMAXTILES*sizeof(tiledata)))==NULL)
        return 0;
        
    memset(newtilebuf, 0, NEWMAXTILES*sizeof(tiledata));
    //Z_message("Performed memset on tiles\n"); 
    clear_tiles(newtilebuf);
    //Z_message("Performed clear_tiles()\n"); 
    Z_message("OK\n");                                        // Allocating tile buffer...
    
    if(is_zquest())
    {
        memrequested+=(NEWMAXTILES*(sizeof(tiledata)+tilesize(tf4Bit)));
        Z_message("Allocating tile grab buffer (%s)... ", byte_conversion2(NEWMAXTILES*sizeof(tiledata),memrequested,-1,-1));
        
        if((grabtilebuf=(tiledata*)zc_malloc(NEWMAXTILES*sizeof(tiledata)))==NULL)
            return 0;
            
        memset(grabtilebuf, 0, NEWMAXTILES*sizeof(tiledata));
        clear_tiles(grabtilebuf);
        Z_message("OK\n");                                        // Allocating tile grab buffer...
    }
    
    memrequested+=(100000);
    Z_message("Allocating trash buffer (%s)... ", byte_conversion2(100000,memrequested,-1,-1));
    
    if((trashbuf=(byte*)zc_malloc(100000))==NULL)
        return 0;
        
    Z_message("OK\n");                                        // Allocating trash buffer...
    
    // Big, ugly band-aid here. Perhaps the most common cause of random crashes
    // has been inadvertently accessing itemsbuf[-1]. All such crashes should be
    // fixed by ensuring there's actually itemdata there.
    // If you change this, be sure to update del_qst_buffers, too.
    
    memrequested+=(sizeof(itemdata)*(MAXITEMS+1));
    Z_message("Allocating item buffer (%s)... ", byte_conversion2(sizeof(itemdata)*(MAXITEMS+1),memrequested,-1,-1));
    
    if((itemsbuf=(itemdata*)zc_malloc(sizeof(itemdata)*(MAXITEMS+1)))==NULL)
        return 0;
        
    memset(itemsbuf,0,sizeof(itemdata)*(MAXITEMS+1));
    itemsbuf++;
    Z_message("OK\n");                                        // Allocating item buffer...
    
    memrequested+=(sizeof(wpndata)*MAXWPNS);
    Z_message("Allocating weapon buffer (%s)... ", byte_conversion2(sizeof(wpndata)*MAXWPNS,memrequested,-1,-1));
    
    if((wpnsbuf=(wpndata*)zc_malloc(sizeof(wpndata)*MAXWPNS))==NULL)
        return 0;
        
    memset(wpnsbuf,0,sizeof(wpndata)*MAXWPNS);
    Z_message("OK\n");                                        // Allocating weapon buffer...
    
    memrequested+=(sizeof(guydata)*MAXGUYS);
    Z_message("Allocating guy buffer (%s)... ", byte_conversion2(sizeof(guydata)*MAXGUYS,memrequested,-1,-1));
    
    if((guysbuf=(guydata*)zc_malloc(sizeof(guydata)*MAXGUYS))==NULL)
        return 0;
        
    memset(guysbuf,0,sizeof(guydata)*MAXGUYS);
    Z_message("OK\n");                                        // Allocating guy buffer...
    
    memrequested+=(sizeof(comboclass)*cMAX);
    Z_message("Allocating combo class buffer (%s)... ", byte_conversion2(sizeof(comboclass)*cMAX,memrequested,-1,-1));
    
    if((combo_class_buf=(comboclass*)zc_malloc(sizeof(comboclass)*cMAX))==NULL)
        return 0;
        
    Z_message("OK\n");										// Allocating combo class buffer...
    
    return 1;
}


void free_newtilebuf()
{
    if(newtilebuf)
    {
        for(int i=0; i<NEWMAXTILES; i++)
            if(newtilebuf[i].data)
                zc_free(newtilebuf[i].data);
                
        zc_free(newtilebuf);
    }
}

void free_grabtilebuf()
{
    if(is_zquest())
    {
        if(grabtilebuf)
        {
            for(int i=0; i<NEWMAXTILES; i++)
                if(grabtilebuf[i].data) zc_free(grabtilebuf[i].data);
                
            zc_free(grabtilebuf);
        }
    }
}

void del_qst_buffers()
{
    al_trace("Cleaning maps. \n");
    
    if(ZCMaps) zc_free(ZCMaps);
    
    if(MsgStrings) zc_free(MsgStrings);
    
    if(DoorComboSets) zc_free(DoorComboSets);
    
    if(DMaps) zc_free(DMaps);
    
    if(combobuf) zc_free(combobuf);
    
    if(colordata) zc_free(colordata);
    
    al_trace("Cleaning tile buffers. \n");
    free_newtilebuf();
    free_grabtilebuf();
    
    al_trace("Cleaning misc. \n");
    
    if(trashbuf) zc_free(trashbuf);
    
    // See get_qst_buffers
    if(itemsbuf)
    {
        itemsbuf--;
        zc_free(itemsbuf);
    }
    
    if(wpnsbuf) zc_free(wpnsbuf);
    
    if(guysbuf) zc_free(guysbuf);
    
    if(combo_class_buf) zc_free(combo_class_buf);
}

bool init_palnames()
{
    if(palnames==NULL)
        return false;
        
    for(int x=0; x<MAXLEVELS; x++)
    {
        switch(x)
        {
        case 0:
            sprintf(palnames[x],"Overworld");
            break;
            
        case 10:
            sprintf(palnames[x],"Caves");
            break;
            
        case 11:
            sprintf(palnames[x],"Passageways");
            break;
            
        default:
            sprintf(palnames[x],"%c",0);
            break;
        }
    }
    
    return true;
}

static void *read_block(PACKFILE *f, int size, int alloc_size, bool keepdata)
{
    void *p;
    
    p = _AL_MALLOC(MAX(size, alloc_size));
    
    if(!p)
    {
        return NULL;
    }
    
    if(!pfread(p,size,f,keepdata))
    {
        _AL_FREE(p);
        return NULL;
    }
    
    if(pack_ferror(f))
    {
        _AL_FREE(p);
        return NULL;
    }
    
    return p;
}

/* read_midi:
  *  Reads MIDI data from a datafile (this is not the same thing as the
  *  standard midi file format).
  */

static MIDI *read_midi(PACKFILE *f, bool)
{
    MIDI *m;
    int c;
    short divisions=0;
    long len=0;
    
    m = (MIDI*)_AL_MALLOC(sizeof(MIDI));
    
    if(!m)
    {
        return NULL;
    }
    
    for(c=0; c<MIDI_TRACKS; c++)
    {
        m->track[c].len = 0;
        m->track[c].data = NULL;
    }
    
    p_mgetw(&divisions,f,true);
    m->divisions=divisions;
    
    for(c=0; c<MIDI_TRACKS; c++)
    {
        p_mgetl(&len,f,true);
        m->track[c].len=len;
        
        if(m->track[c].len > 0)
        {
            m->track[c].data = (byte*)read_block(f, m->track[c].len, 0, true);
            
            if(!m->track[c].data)
            {
                destroy_midi(m);
                return NULL;
            }
        }
    }
    
    LOCK_DATA(m, sizeof(MIDI));
    
    for(c=0; c<MIDI_TRACKS; c++)
    {
        if(m->track[c].data)
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
        
    for(; di<1024; di++)
        clear_combo(di);
}

void reset_tunes(zctune *tune)
{
    for(int i=0; i<MAXCUSTOMTUNES; i++)
    {
        tune[i].reset();
    }
}


/*void reset_midi(zcmidi_ *m)
{
  m->title[0]=0;
  m->loop=1;
  m->volume=144;
  m->start=0;
  m->loop_start=-1;
  m->loop_end=-1;
  if(m->midi)
  {
    destroy_midi(m->midi);
  }
  m->midi=NULL;
}


void reset_midis(zcmidi_ *m)
{
  for(int i=0; i<MAXCUSTOMMIDIS; i++)
  {
      reset_midi(m+i);
  }
}
*/

void reset_scr(int scr)
{
    /*
      byte *di=((byte*)TheMaps)+(scr*sizeof(mapscr));
      for(unsigned i=0; i<sizeof(mapscr); i++)
      *(di++) = 0;
      TheMaps[scr].valid=mVERSION;
      */
    
    TheMaps[scr].zero_memory();
    //byte *di=((byte*)TheMaps)+(scr*sizeof(mapscr));
    
    for(int i=0; i<6; i++)
    {
        //these will be uncommented later
        //TheMaps[scr].layerxsize[i]=16;
        //TheMaps[scr].layerysize[i]=11;
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
    for(int i=0; i<9; i++)
    {
        for(int j=0; j<6; j++)
        {
            if(j<4)
            {
                if(a.doorcombo_u[i][j]!=b.doorcombo_u[i][j])
                {
                    return false;
                }
                
                if(a.doorcset_u[i][j]!=b.doorcset_u[i][j])
                {
                    return false;
                }
                
                if(a.doorcombo_d[i][j]!=b.doorcombo_d[i][j])
                {
                    return false;
                }
                
                if(a.doorcset_d[i][j]!=b.doorcset_d[i][j])
                {
                    return false;
                }
            }
            
            if(a.doorcombo_l[i][j]!=b.doorcombo_l[i][j])
            {
                return false;
            }
            
            if(a.doorcset_l[i][j]!=b.doorcset_l[i][j])
            {
                return false;
            }
            
            if(a.doorcombo_r[i][j]!=b.doorcombo_r[i][j])
            {
                return false;
            }
            
            if(a.doorcset_r[i][j]!=b.doorcset_r[i][j])
            {
                return false;
            }
        }
        
        if(i<2)
        {
            if(a.flags[i]!=b.flags[i])
            {
                return false;
            }
            
            if(a.bombdoorcombo_u[i]!=b.bombdoorcombo_u[i])
            {
                return false;
            }
            
            if(a.bombdoorcset_u[i]!=b.bombdoorcset_u[i])
            {
                return false;
            }
            
            if(a.bombdoorcombo_d[i]!=b.bombdoorcombo_d[i])
            {
                return false;
            }
            
            if(a.bombdoorcset_d[i]!=b.bombdoorcset_d[i])
            {
                return false;
            }
        }
        
        if(i<3)
        {
            if(a.bombdoorcombo_l[i]!=b.bombdoorcombo_l[i])
            {
                return false;
            }
            
            if(a.bombdoorcset_l[i]!=b.bombdoorcset_l[i])
            {
                return false;
            }
            
            if(a.bombdoorcombo_r[i]!=b.bombdoorcombo_r[i])
            {
                return false;
            }
            
            if(a.bombdoorcset_r[i]!=b.bombdoorcset_r[i])
            {
                return false;
            }
        }
        
        if(a.walkthroughcombo[i]!=b.walkthroughcombo[i])
        {
            return false;
        }
        
        if(a.walkthroughcset[i]!=b.walkthroughcset[i])
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
    //these are here to bypass compiler warnings about unused arguments
    map=map;
    scr=scr;
    pos=pos;
    
    //what does this function do?
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
    for(int i=0; i<9; i++)
    {
        for(int j=0; j<4; j++)
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
    for(int i=0; i<9; i++)
    {
        for(int j=0; j<4; j++)
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
    for(int i=0; i<9; i++)
    {
        for(int j=0; j<6; j++)
        {
            tempdcs.doorcombo_l[i][j]=tdcmbdat(map,scr,doortranslations_l[i][j]);
            tempdcs.doorcset_l[i][j]=tdcmbcset(map,scr,doortranslations_l[i][j]);
        }
    }
    
    for(int j=0; j>6; j++)
    {
        if((j!=2)&&(j!=3))
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
    for(int i=0; i<9; i++)
    {
        for(int j=0; j<6; j++)
        {
            tempdcs.doorcombo_r[i][j]=tdcmbdat(map,scr,doortranslations_r[i][j]);
            tempdcs.doorcset_r[i][j]=tdcmbcset(map,scr,doortranslations_r[i][j]);
        }
    }
    
    for(int j=0; j>6; j++)
    {
        if((j!=2)&&(j!=3))
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
    
    int k;
    
    for(k=0; k<door_combo_set_count; k++)
    {
        if(DoorComboSets[k]==tempdcs)
        {
            break;
        }
    }
    
    if(k==door_combo_set_count)
    {
        DoorComboSets[k]=tempdcs;
        sprintf(DoorComboSets[k].name, "Door Combo Set %d", k);
        ++door_combo_set_count;
    }
    
    return k;
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

INLINE int tcmbdat2(int map, int scr, int pos)
{
    return (TheMaps[map*MAPSCRS+TEMPLATE2].data[pos]&0xFF)+((TheMaps[map*MAPSCRS+scr].old_cpage)<<8);
}

INLINE int tcmbcset2(int map, int pos)
{

    return TheMaps[map*MAPSCRS+TEMPLATE2].cset[pos];
}

INLINE int tcmbflag2(int map, int pos)
{
    return TheMaps[map*MAPSCRS+TEMPLATE2].sflag[pos];
}


void get_questpwd(char *encrypted_pwd, short pwdkey, char *pwd)
{
    char temp_pwd[30];
    memset(temp_pwd,0,30);
    
    if(pwdkey!=0)
    {
        memcpy(temp_pwd,encrypted_pwd,30);
        temp_pwd[29]=0;
        
        for(int i=0; i<30; i++)
        {
            temp_pwd[i] -= pwdkey;
            int t=pwdkey>>15;
            pwdkey = (pwdkey<<1)+t;
        }
    }
    
    memcpy(pwd,temp_pwd,30);
}

bool check_questpwd(zquestheader *Header, char *pwd)
{
    cvs_MD5Context ctx;
    unsigned char md5sum[16];
    
    cvs_MD5Init(&ctx);
    cvs_MD5Update(&ctx, (const unsigned char*)pwd, (unsigned)strlen(pwd));
    cvs_MD5Final(md5sum, &ctx);
    
    return (memcmp(Header->pwd_hash,md5sum,16)==0);
    
}


int readheader(PACKFILE *f, zquestheader *Header, bool keepdata)
{
    int dummy;
    zquestheader tempheader;
    memcpy(&tempheader, Header, sizeof(tempheader));
    char dummybuf[80];
    byte temp_map_count;
    byte temp_midi_flags[MIDIFLAGS_SIZE];
    word version;
    char temp_pwd[30], temp_pwd2[30];
    short temp_pwdkey;
    cvs_MD5Context ctx;
    memset(temp_midi_flags, 0, MIDIFLAGS_SIZE);
    memset(&tempheader, 0, sizeof(tempheader));
	memset(ffcore.quest_format, 0, sizeof(ffcore.quest_format));
	
	
    
    if(!pfread(tempheader.id_str,sizeof(tempheader.id_str),f,true))      // first read old header
    {
        Z_message("Unable to read header string\n");
        return qe_invalid;
    }
    
    // check header
    if(strcmp(tempheader.id_str,QH_NEWIDSTR))
    {
        if(strcmp(tempheader.id_str,QH_IDSTR))
        {
            Z_message("Invalid header string:  '%s' (was expecting '%s' or '%s')\n", tempheader.id_str, QH_IDSTR, QH_NEWIDSTR);
            return qe_invalid;
        }
    }
    
    int templatepath_len=0;
    
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
	
	ffcore.quest_format[vZelda] = tempheader.zelda_version;
        
        if(tempheader.zelda_version > ZELDA_VERSION)
        {
            return qe_version;
        }
	
	ffcore.quest_format[vZelda] = tempheader.zelda_version;
        
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
	
	ffcore.quest_format[qQuestNumber] = tempheader.quest_number;
        
        if(!pfread(&quest_rules[0],2,f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&temp_map_count,f,true))
        {
            return qe_invalid;
        }
	
	ffcore.quest_format[qMapCount] = temp_map_count;
        
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
        
        if(!p_igetw(&temp_pwdkey,f,true))
        {
            return qe_invalid;
        }
        
        if(!pfread(temp_pwd,30,f,true))
        {
            return qe_invalid;
        }
        
        get_questpwd(temp_pwd, temp_pwdkey, temp_pwd2);
        cvs_MD5Init(&ctx);
        cvs_MD5Update(&ctx, (const unsigned char*)temp_pwd2, (unsigned)strlen(temp_pwd2));
        cvs_MD5Final(tempheader.pwd_hash, &ctx);
        
        if(tempheader.zelda_version < 0x177)                       // lacks new header stuff...
        {
            //memset(tempheader.minver,0,20);                          //   char minver[9], byte build, byte foo[10]
            // Not anymore...
            memset(tempheader.minver,0,9);
            tempheader.build=0;
            tempheader.use_keyfile=0;
            memset(tempheader.old_foo, 0, 9);
        }
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
	    
	    ffcore.quest_format[vBuild] = tempheader.build;
            
            if(!p_getc(&tempheader.use_keyfile,f,true))
            {
                return qe_invalid;
            }
            
            if(!pfread(dummybuf,9,f,true))
            {
                return qe_invalid;
            }
        }                                                       // starting at minver
        
        if(tempheader.zelda_version < 0x187)                    // lacks newer header stuff...
        {
            memset(&quest_rules[4],0,16);                          //   word rules3..rules10
        }
        else
        {
            if(!pfread(&quest_rules[4],16,f,true))                      // read new header additions
            {
                return qe_invalid;                                  // starting at rules3
            }
            
            if(tempheader.zelda_version <= 0x190)
            {
                set_bit(quest_rules,qr_MEANPLACEDTRAPS,0);
            }
        }
        
        if((tempheader.zelda_version < 0x192)||
                ((tempheader.zelda_version == 0x192)&&(tempheader.build<149)))
        {
            set_bit(quest_rules,qr_BRKNSHLDTILES,(get_bit(quest_rules,qr_BRKBLSHLDS_DEP)));
            set_bit(deprecated_rules,qr_BRKBLSHLDS_DEP,1);
        }
        
        if(tempheader.zelda_version >= 0x192)                       //  lacks newer header stuff...
        {
            byte *mf=temp_midi_flags;
            
            if((tempheader.zelda_version == 0x192)&&(tempheader.build<178))
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
        
        if((tempheader.zelda_version < 0x192)||
                ((tempheader.zelda_version == 0x192)&&(tempheader.build<145)))
        {
            memset(tempheader.templatepath,0,2048);
        }
        else
        {
            if(!pfread(tempheader.templatepath,280,f,true))               // read templatepath
            {
                return qe_invalid;
            }
        }
        
        if((tempheader.zelda_version < 0x192)||
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
        if(!p_igetw(&version,f,true))
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
	
	ffcore.quest_format[vZelda] = tempheader.zelda_version;
        
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
	
        ffcore.quest_format[vBuild] = tempheader.build;
	
        if(version<3)
        {
            if(!pfread(temp_pwd,30,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&temp_pwdkey,f,true))
            {
                return qe_invalid;
            }
            
            get_questpwd(temp_pwd, temp_pwdkey, temp_pwd2);
            cvs_MD5Init(&ctx);
            cvs_MD5Update(&ctx, (const unsigned char*)temp_pwd2, (unsigned)strlen(temp_pwd2));
            cvs_MD5Final(tempheader.pwd_hash, &ctx);
        }
        else
        {
            if(!pfread(tempheader.pwd_hash,sizeof(tempheader.pwd_hash),f,true))
            {
                return qe_invalid;
            }
        }
        
        if(!p_igetw(&tempheader.internal,f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&tempheader.quest_number,f,true))
        {
            return qe_invalid;
        }
	
	ffcore.quest_format[qQuestNumber] = tempheader.quest_number;
        
        if(!pfread(tempheader.version,sizeof(tempheader.version),f,true))
        {
            return qe_invalid;
        }
	
	//ffcore.quest_format[qQuestVersion] = tempheader.version;
        //needs to be copied as char[9] or stored as a s.str
        if(!pfread(tempheader.minver,sizeof(tempheader.minver),f,true))
        {
            return qe_invalid;
        }
	
	//ffcore.quest_format[qMinQuestVersion] = tempheader.minver;
        //needs to be copied as char[9] or stored as a s.str
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
        
        templatepath_len=sizeof(tempheader.templatepath);
        
        if(version==1)
        {
            templatepath_len=280;
        }
        
        if(!pfread(tempheader.templatepath,templatepath_len,f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&temp_map_count,f,true))
        {
            return qe_invalid;
        }
	
    }
    
    if(keepdata==true)
    {
        memcpy(Header, &tempheader, sizeof(tempheader));
        map_count=temp_map_count;
        memcpy(midi_flags, temp_midi_flags, MIDIFLAGS_SIZE);
    }
    
    return 0;
}

int readrules(PACKFILE *f, zquestheader *Header, bool keepdata)
{
    int dummy;
    zquestheader tempheader;
    word s_version=0;
    
    memcpy(&tempheader, Header, sizeof(tempheader));
    
    if(tempheader.zelda_version >= 0x193)
    {
        //section version info
        if(!p_igetw(&s_version,f,true))
        {
            return qe_invalid;
        }
	
	ffcore.quest_format[vRules] = s_version;
        
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
    
    //al_trace("Rules version %d\n", s_version);
    
    memcpy(deprecated_rules, quest_rules, QUESTRULES_SIZE);
    
    if(s_version<2)
    {
        set_bit(quest_rules,14,0);
        set_bit(quest_rules,27,0);
        set_bit(quest_rules,28,0);
        set_bit(quest_rules,29,0);
        set_bit(quest_rules,30,0);
        set_bit(quest_rules,32,0);
        set_bit(quest_rules,36,0);
        set_bit(quest_rules,49,0);
        set_bit(quest_rules,50,0);
        set_bit(quest_rules,51,0);
        set_bit(quest_rules,68,0);
        set_bit(quest_rules,75,0);
        set_bit(quest_rules,76,0);
        set_bit(quest_rules,98,0);
        set_bit(quest_rules,110,0);
        set_bit(quest_rules,113,0);
        set_bit(quest_rules,116,0);
        set_bit(quest_rules,102,0);
        set_bit(quest_rules,132,0);
    }
    
    //Now, do any updates...
    if((tempheader.zelda_version < 0x211)||((tempheader.zelda_version == 0x211)&&(tempheader.build<18)))
    {
        set_bit(quest_rules, qr_SMOOTHVERTICALSCROLLING,1);
        set_bit(quest_rules, qr_REPLACEOPENDOORS, 1);
        set_bit(quest_rules, qr_OLDLENSORDER, 1);
        set_bit(quest_rules, qr_NOFAIRYGUYFIRES, 1);
        set_bit(quest_rules, qr_TRIGGERSREPEAT, 1);
    }
    
    if((tempheader.zelda_version < 0x193)||((tempheader.zelda_version == 0x193)&&(tempheader.build<3)))
    {
        set_bit(quest_rules,qr_WALLFLIERS,1);
    }
    
    if((tempheader.zelda_version < 0x193)||((tempheader.zelda_version == 0x193)&&(tempheader.build<4)))
    {
        set_bit(quest_rules,qr_NOBOMBPALFLASH,1);
    }
    
    if((tempheader.zelda_version < 0x193)||((tempheader.zelda_version == 0x193)&&(tempheader.build<3)))
    {
        set_bit(quest_rules,qr_NOSCROLLCONTINUE,1);
    }
    
    if(tempheader.zelda_version == 0x210)
    {
        set_bit(quest_rules, qr_NOSCROLLCONTINUE, get_bit(quest_rules, qr_CMBCYCLELAYERS));
        set_bit(quest_rules, qr_CMBCYCLELAYERS, 0);
    }
    
    if(tempheader.zelda_version <= 0x210)
    {
        set_bit(quest_rules,qr_OLDSTYLEWARP,1);
    }
    
    //might not be correct
    if(tempheader.zelda_version < 0x210)
    {
        set_bit(deprecated_rules, qr_OLDTRIBBLES_DEP,1);
        set_bit(quest_rules, qr_OLDHOOKSHOTGRAB,1);
    }
    
    if(tempheader.zelda_version == 0x210)
    {
        set_bit(deprecated_rules, qr_OLDTRIBBLES_DEP, get_bit(quest_rules, qr_DMGCOMBOPRI));
        set_bit(quest_rules, qr_DMGCOMBOPRI, 0);
    }
    
    if(tempheader.zelda_version < 0x211 || (tempheader.zelda_version == 0x211 && tempheader.build<15))
    {
        set_bit(quest_rules, qr_OLDPICKUP,1);
    }
    
    if(tempheader.zelda_version < 0x211 || (tempheader.zelda_version == 0x211 && tempheader.build < 18))
    {
        set_bit(quest_rules,qr_NOSOLIDDAMAGECOMBOS, 1);
        set_bit(quest_rules, qr_ITEMPICKUPSETSBELOW, 1); // broke around build 400
    }
    
    if(tempheader.zelda_version < 0x250) // version<0x250 checks for beta 18; build was set to 18 prematurely
    {
        set_bit(quest_rules,qr_HOOKSHOTDOWNBUG, 1);
    }
    
    if(tempheader.zelda_version == 0x250 && tempheader.build == 24) // Annoying...
    {
        set_bit(quest_rules,qr_PEAHATCLOCKVULN, 1);
    }
    
    if(tempheader.zelda_version < 0x250 || (tempheader.zelda_version == 0x250 && tempheader.build<28))
    {
        set_bit(quest_rules, qr_OFFSCREENWEAPONS, 1);
    }
    
    if(tempheader.zelda_version < 0x250 || (tempheader.zelda_version == 0x250 && tempheader.build<29))
    {
        // qr_OFFSETEWPNCOLLISIONFIX
        // All 'official' quests need this disabled.
        // All 2.10 and lower quests need this enabled to preseve compatability.
        // All 2.11 - 2.5.1 quests should have it set also, due to a bug in about half of all the betas.

	    //~Gleeok
		set_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX, 1); //This has to be set!!!!
		
		// Broke in build 695
		if(tempheader.zelda_version>=0x211 && tempheader.build>=18)
            set_bit(quest_rules, qr_BROKENSTATUES, 1);
    }
    
    if(s_version < 3)
    {
        set_bit(quest_rules, qr_HOLDNOSTOPMUSIC, 1);
        set_bit(quest_rules, qr_CAVEEXITNOSTOPMUSIC, 1);
    }
    
    if(s_version<4)
    {
        set_bit(quest_rules,10,0);
    }
    
    if(s_version<5)
    {
        set_bit(quest_rules,27,0);
    }
    
    if(s_version<6)
    {
        set_bit(quest_rules,46,0);
    }
    
    if(s_version<7) // January 2008
    {
        set_bit(quest_rules,qr_HEARTSREQUIREDFIX,0);
        set_bit(quest_rules,qr_PUSHBLOCKCSETFIX,1);
    }
    
    if(s_version<8)
    {
        set_bit(quest_rules, 12, 0);
    }
    else
    {
        set_bit(deprecated_rules, 12, 0);
    }
    
    if(s_version<9) // October 2008
    {
        set_bit(quest_rules,qr_NOROPE2FLASH_DEP,0);
        set_bit(quest_rules,qr_NOBUBBLEFLASH_DEP,0);
        set_bit(quest_rules,qr_GHINI2BLINK_DEP,0);
        set_bit(quest_rules,qr_PHANTOMGHINI2_DEP,0);
    }
    
    if(s_version<10) // December 2008
    {
        set_bit(quest_rules,qr_NOCLOCKS_DEP,0);
        set_bit(quest_rules, qr_ALLOW10RUPEEDROPS_DEP,0);
    }
    
    if(s_version<11) // April 2009
    {
        set_bit(quest_rules,qr_SLOWENEMYANIM_DEP,0);
    }
    
    if(s_version<12)  // December 2009
    {
        set_bit(quest_rules,qr_BRKBLSHLDS_DEP,0);
        set_bit(quest_rules, qr_OLDTRIBBLES_DEP,0);
    }
    
    //if(tempheader.zelda_version < 0x250 || (tempheader.zelda_version == 0x250 && tempheader.build < 24))
    if(s_version < 13)
    {
        set_bit(quest_rules,qr_SHOPCHEAT, 1);
    }
    
    // Not entirely sure this is the best place for this...
    memset(extra_rules, 0, EXTRARULES_SIZE);
    if(tempheader.zelda_version < 0x250 || (tempheader.zelda_version == 0x250 && tempheader.build<29))
    {
        set_bit(extra_rules, er_BITMAPOFFSET, 1);
    }
    //more 2.50 fixes -Z
    if(tempheader.zelda_version < 0x250 || (tempheader.zelda_version == 0x250 && tempheader.build<31))
    {
        set_bit(quest_rules, qr_MELEEMAGICCOST, 0);
        set_bit(quest_rules, qr_NOGANONINTRO, 0);
        set_bit(quest_rules, qr_OLDMIRRORCOMBOS, 1);
        set_bit(quest_rules, qr_BROKENBOOKCOST, 1);
	set_bit(extra_rules, er_BROKENCHARINTDRAWING, 1);
	set_bit(extra_rules, er_SETENEMYWEAPONSPRITESONWPNCHANGE, 1);
    }
    if(tempheader.zelda_version == 0x254 && tempheader.build<41)
    {
	set_bit(quest_rules,qr_MELEEMAGICCOST, get_bit(extra_rules,er_MAGICCOSTSWORD));
    }
	    
    
    if(tempheader.zelda_version < 0x193)
    {
        set_bit(extra_rules, er_SHORTDGNWALK, 1);
    }
    
    if(keepdata==true)
    {
        memcpy(Header, &tempheader, sizeof(tempheader));
    }
    
    return 0;
}

void init_msgstr(MsgStr *str)
{
    memset(str->s, 0, MSGSIZE+1);
    str->nextstring=0;
    str->tile=0;
    str->cset=0;
    str->trans=false;
    str->font=font_zfont;
    str->y=32;
    str->sfx=18;
    str->listpos=0;
    str->x=24;
    str->w=24*8;
    str->h=3*8;
    str->hspace=0;
    str->vspace=0;
    str->stringflags=0;
}

void init_msgstrings(int start, int end)
{
    if(end <= start || end-start > msg_strings_size)
        return;
        
    for(int i=start; i<end; i++)
    {
        init_msgstr(&MsgStrings[i]);
        MsgStrings[i].listpos=i;
    }
    
    if(start==0)
    {
        strcpy(MsgStrings[0].s,"(None)");
        MsgStrings[0].listpos = 0;
    }
}

int readstrings(PACKFILE *f, zquestheader *Header, bool keepdata)
{

    MsgStr tempMsgString;
    init_msgstr(&tempMsgString);
    
    word temp_msg_count=0;
    word temp_expansion[16];
    memset(temp_expansion, 0, 16*sizeof(word));
    
    if(Header->zelda_version < 0x193)
    {
        byte tempbyte;
        int strings_to_read=0;
        
        if((Header->zelda_version < 0x192)||
                ((Header->zelda_version == 0x192)&&(Header->build<31)))
        {
            strings_to_read=128;
            temp_msg_count=Header->old_str_count;
            
            // Some sort of string count corruption seems to be common in old quests
            if(temp_msg_count>128)
            {
                temp_msg_count=128;
            }
        }
        else if((Header->zelda_version == 0x192)&&(Header->build<140))
        {
            strings_to_read=255;
            temp_msg_count=Header->old_str_count;
        }
        else
        {
            if(!p_igetw(&temp_msg_count,f,true))
            {
                return qe_invalid;
            }
            
            strings_to_read=temp_msg_count;
            
            if(temp_msg_count >= msg_strings_size)
            {
                Z_message("Reallocating string buffer...\n");
                
                if((MsgStrings=(MsgStr*)_al_sane_realloc(MsgStrings,sizeof(MsgStr)*MAXMSGS))==NULL)
                    return qe_nomem;
                    
                memset(MsgStrings, 0, sizeof(MsgStr)*MAXMSGS);
                msg_strings_size = MAXMSGS;
            }
        }
        
        //reset the message strings
        if(keepdata)
        {
            init_msgstrings(0,msg_strings_size);
        }
        
        for(int x=0; x<strings_to_read; x++)
        {
            memset(&tempMsgString.s, 0, MSGSIZE+1);
            tempMsgString.nextstring=0;
            tempMsgString.tile=0;
            tempMsgString.cset=0;
            tempMsgString.trans=false;
            tempMsgString.font=font_zfont;
            tempMsgString.y=32;
            tempMsgString.sfx=WAV_MSG;
            tempMsgString.listpos=x;
            tempMsgString.x=24;
            tempMsgString.w=24*8;
            tempMsgString.h=3*8;
            tempMsgString.hspace=0;
            tempMsgString.vspace=0;
            tempMsgString.stringflags=0;
            
            if(!pfread(&tempMsgString.s,73,f,true))
            {
                return qe_invalid;
            }
            
            for(int i=72; i<=MSGSIZE; i++)
                tempMsgString.s[i]='\0';
                
            if(!p_getc(&tempbyte,f,true))
            {
                return qe_invalid;
            }
            
            if((Header->zelda_version < 0x192)||
                    ((Header->zelda_version == 0x192)&&(Header->build<148)))
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
            }
            else
            {
                if(!p_igetw(&tempMsgString.nextstring,f,true))
                {
                    return qe_invalid;
                }
                
                if(!pfread(temp_expansion,32,f,true))
                {
                    return qe_invalid;
                }
            }
            
            if(keepdata==true)
            {
                memcpy(&MsgStrings[x], &tempMsgString, sizeof(tempMsgString));
            }
        }
    }
    else
    {
        int dummy_int;
        word s_version;
        word s_cversion;
        
        //section version info
        if(!p_igetw(&s_version,f,true))
        {
            return qe_invalid;
        }
	
	ffcore.quest_format[vStrings] = s_version;
        
        if(!p_igetw(&s_cversion,f,true))
        {
            return qe_invalid;
        }
        
        //al_trace("Strings version %d\n", s_version);
        //section size
        if(!p_igetl(&dummy_int,f,true))
        {
            return qe_invalid;
        }
        
        //finally...  section data
        if(!p_igetw(&temp_msg_count,f,true))
        {
            return qe_invalid;
        }
        
        if(temp_msg_count >= msg_strings_size)
        {
            Z_message("Reallocating string buffer...\n");
            
            if((MsgStrings=(MsgStr*)_al_sane_realloc(MsgStrings,sizeof(MsgStr)*MAXMSGS))==NULL)
                return qe_nomem;
                
            memset(MsgStrings, 0, sizeof(MsgStr)*MAXMSGS);
            msg_strings_size = MAXMSGS;
        }
        
        //reset the message strings
        if(keepdata)
        {
            init_msgstrings(0,msg_strings_size);
        }
        
        int string_length=(s_version<2)?73:145;
        
        for(int i=0; i<temp_msg_count; i++)
        {
            tempMsgString.nextstring=0;
            tempMsgString.tile=0;
            tempMsgString.cset=0;
            tempMsgString.trans=false;
            tempMsgString.font=font_zfont;
            tempMsgString.y=32;
            tempMsgString.sfx=WAV_MSG;
            tempMsgString.listpos=i;
            tempMsgString.x=24;
            tempMsgString.w=24*8;
            tempMsgString.h=3*8;
            tempMsgString.hspace=0;
            tempMsgString.vspace=0;
            tempMsgString.stringflags=0;
            
            if(!pfread(&tempMsgString.s,string_length,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&tempMsgString.nextstring,f,true))
            {
                return qe_invalid;
            }
            
            if(s_version<2)
            {
                for(int j=72; j<MSGSIZE; j++)
                {
                    tempMsgString.s[j]='\0';
                }
            }
            else
            {
                // June 2008: A bug corrupted the last 4 chars of a string.
                // Discard these.
                if(s_version<3)
                {
                    for(int j=MSGSIZE-4; j<MSGSIZE; j++)
                    {
                        tempMsgString.s[j]='\0';
                    }
                }
                
                tempMsgString.s[MSGSIZE]='\0';
                
		if ( s_version >= 6 )
		{
			if(!p_igetl(&tempMsgString.tile,f,true))
			{
			    return qe_invalid;
			}
		}
		else
		{
			if(!p_igetw(&tempMsgString.tile,f,true))
			{
			    return qe_invalid;
			}
		}
			
                if(!p_getc(&tempMsgString.cset,f,true))
                {
                    return qe_invalid;
                }
                
                byte dummy_char;
                
                if(!p_getc(&dummy_char,f,true)) // trans is stored as a char...
                {
                    return qe_invalid;
                }
                
                tempMsgString.trans=dummy_char!=0;
                
                if(!p_getc(&tempMsgString.font,f,true))
                {
                    return qe_invalid;
                }
                
                if(s_version < 5)
                {
                    if(!p_getc(&tempMsgString.y,f,true))
                    {
                        return qe_invalid;
                    }
                }
                else
                {
                    if(!p_igetw(&tempMsgString.x,f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_igetw(&tempMsgString.y,f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_igetw(&tempMsgString.w,f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_igetw(&tempMsgString.h,f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_getc(&tempMsgString.hspace,f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_getc(&tempMsgString.vspace,f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_getc(&tempMsgString.stringflags,f,true))
                    {
                        return qe_invalid;
                    }
                }
                
                if(!p_getc(&tempMsgString.sfx,f,true))
                {
                    return qe_invalid;
                }
                
                if(s_version>3)
                {
                    if(!p_igetw(&tempMsgString.listpos,f,true))
                    {
                        return qe_invalid;
                    }
                }
            }
            
            if(keepdata==true)
            {
                memcpy(&MsgStrings[i], &tempMsgString, sizeof(tempMsgString));
            }
        }
    }
    
    if(keepdata==true)
    {
        msg_count=temp_msg_count;
    }
    
    return 0;
}

int readdoorcombosets(PACKFILE *f, zquestheader *Header, bool keepdata)
{
    if((Header->zelda_version < 0x192)||
            ((Header->zelda_version == 0x192)&&(Header->build<158)))
    {
        return 0;
    }
    
    word temp_door_combo_set_count=0;
    DoorComboSet tempDoorComboSet;
    word dummy_word;
    long dummy_long;
    byte padding;
    int s_version = 0;
    
    if(keepdata==true)
    {
        for(int i=0; i<MAXDOORCOMBOSETS; i++)
        {
            memset(DoorComboSets+i, 0, sizeof(DoorComboSet));
        }
    }
    
    if(Header->zelda_version > 0x192)
    {
        //section version info
        if(!p_igetw(&s_version,f,true))
        {
            return qe_invalid;
        }
	
        ffcore.quest_format[vDoors] = s_version;
	
        //al_trace("Door combo sets version %d\n", dummy_word);
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
    
    for(int i=0; i<temp_door_combo_set_count; i++)
    {
        memset(&tempDoorComboSet, 0, sizeof(DoorComboSet));
        
        //name
        if(!pfread(&tempDoorComboSet.name,sizeof(tempDoorComboSet.name),f,true))
        {
            return qe_invalid;
        }
        
        if(Header->zelda_version < 0x193)
        {
            if(!p_getc(&padding,f,true))
            {
                return qe_invalid;
            }
        }
        
        //up door
        for(int j=0; j<9; j++)
        {
            for(int k=0; k<4; k++)
            {
                if(!p_igetw(&tempDoorComboSet.doorcombo_u[j][k],f,true))
                {
                    return qe_invalid;
                }
            }
        }
        
        for(int j=0; j<9; j++)
        {
            for(int k=0; k<4; k++)
            {
                if(!p_getc(&tempDoorComboSet.doorcset_u[j][k],f,true))
                {
                    return qe_invalid;
                }
            }
        }
        
        //down door
        for(int j=0; j<9; j++)
        {
            for(int k=0; k<4; k++)
            {
                if(!p_igetw(&tempDoorComboSet.doorcombo_d[j][k],f,true))
                {
                    return qe_invalid;
                }
            }
        }
        
        for(int j=0; j<9; j++)
        {
            for(int k=0; k<4; k++)
            {
                if(!p_getc(&tempDoorComboSet.doorcset_d[j][k],f,true))
                {
                    return qe_invalid;
                }
            }
        }
        
        //left door
        for(int j=0; j<9; j++)
        {
            for(int k=0; k<6; k++)
            {
                if(!p_igetw(&tempDoorComboSet.doorcombo_l[j][k],f,true))
                {
                    return qe_invalid;
                }
            }
        }
        
        for(int j=0; j<9; j++)
        {
            for(int k=0; k<6; k++)
            {
                if(!p_getc(&tempDoorComboSet.doorcset_l[j][k],f,true))
                {
                    return qe_invalid;
                }
            }
        }
        
        //right door
        for(int j=0; j<9; j++)
        {
            for(int k=0; k<6; k++)
            {
                if(!p_igetw(&tempDoorComboSet.doorcombo_r[j][k],f,true))
                {
                    return qe_invalid;
                }
            }
        }
        
        for(int j=0; j<9; j++)
        {
            for(int k=0; k<6; k++)
            {
                if(!p_getc(&tempDoorComboSet.doorcset_r[j][k],f,true))
                {
                    return qe_invalid;
                }
            }
        }
        
        //up bomb rubble
        for(int j=0; j<2; j++)
        {
            if(!p_igetw(&tempDoorComboSet.bombdoorcombo_u[j],f,true))
            {
                return qe_invalid;
            }
        }
        
        for(int j=0; j<2; j++)
        {
            if(!p_getc(&tempDoorComboSet.bombdoorcset_u[j],f,true))
            {
                return qe_invalid;
            }
        }
        
        //down bomb rubble
        for(int j=0; j<2; j++)
        {
            if(!p_igetw(&tempDoorComboSet.bombdoorcombo_d[j],f,true))
            {
                return qe_invalid;
            }
        }
        
        for(int j=0; j<2; j++)
        {
            if(!p_getc(&tempDoorComboSet.bombdoorcset_d[j],f,true))
            {
                return qe_invalid;
            }
        }
        
        //left bomb rubble
        for(int j=0; j<3; j++)
        {
            if(!p_igetw(&tempDoorComboSet.bombdoorcombo_l[j],f,true))
            {
                return qe_invalid;
            }
        }
        
        for(int j=0; j<3; j++)
        {
            if(!p_getc(&tempDoorComboSet.bombdoorcset_l[j],f,true))
            {
                return qe_invalid;
            }
        }
        
        if(Header->zelda_version < 0x193)
        {
            if(!p_getc(&padding,f,true))
            {
                return qe_invalid;
            }
            
        }
        
        //right bomb rubble
        for(int j=0; j<3; j++)
        {
            if(!p_igetw(&tempDoorComboSet.bombdoorcombo_r[j],f,true))
            {
                return qe_invalid;
            }
        }
        
        for(int j=0; j<3; j++)
        {
            if(!p_getc(&tempDoorComboSet.bombdoorcset_r[j],f,true))
            {
                return qe_invalid;
            }
        }
        
        if(Header->zelda_version < 0x193)
        {
            if(!p_getc(&padding,f,true))
            {
                return qe_invalid;
            }
        }
        
        //walkthrough stuff
        for(int j=0; j<4; j++)
        {
            if(!p_igetw(&tempDoorComboSet.walkthroughcombo[j],f,true))
            {
                return qe_invalid;
            }
        }
        
        for(int j=0; j<4; j++)
        {
            if(!p_getc(&tempDoorComboSet.walkthroughcset[j],f,true))
            {
                return qe_invalid;
            }
        }
        
        //flags
        for(int j=0; j<2; j++)
        {
            if(!p_getc(&tempDoorComboSet.flags[j],f,true))
            {
                return qe_invalid;
            }
        }
        
        if(Header->zelda_version < 0x193)
        {
            if(!pfread(&tempDoorComboSet.expansion,sizeof(tempDoorComboSet.expansion),f,true))
            {
                return qe_invalid;
            }
        }
        
        if(keepdata==true)
        {
            memcpy(&DoorComboSets[i], &tempDoorComboSet, sizeof(tempDoorComboSet));
        }
    }
    
    if(keepdata==true)
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
        if((DMaps[i].map!=0)||(DMaps[i].level!=0)||(DMaps[i].xoff!=0)||
                (DMaps[i].compass!=0)||(DMaps[i].color!=0)||(DMaps[i].midi!=0)||
                (DMaps[i].cont!=0)||(DMaps[i].type!=0))
            found=true;
            
        for(int j=0; j<8; j++)
        {
            if(DMaps[i].grid[j]!=0)
            
                found=true;
        }
        
        if((DMaps[i].name[0]!=0)||(DMaps[i].title[0]!=0)||
                (DMaps[i].intro[0]!=0)||(DMaps[i].tmusic[0]!=0))
            found=true;
            
        if((DMaps[i].minimap_1_tile!=0)||(DMaps[i].minimap_2_tile!=0)||
                (DMaps[i].largemap_1_tile!=0)||(DMaps[i].largemap_2_tile!=0)||
                (DMaps[i].minimap_1_cset!=0)||(DMaps[i].minimap_2_cset!=0)||
                (DMaps[i].largemap_1_cset!=0)||(DMaps[i].largemap_2_cset!=0))
            found=true;
            
        if(!found)
        {
            i--;
        }
    }
    
    return i+1;
}


int count_shops(miscQdata *Misc)
{
    int i=255,j;
    bool found=false;
    
    while(i>=0 && !found)
    {
        j=2;
        
        while(j>=0 && !found)
        {
            if((Misc->shop[i].hasitem[j]!=0)||(Misc->shop[i].price[j]!=0))
            {
                found=true;
            }
            else
            {
                j--;
            }
        }
        
        if(Misc->shop[i].name[0]!=0)
        {
            found=true;
        }
        
        if(!found)
        {
            i--;
        }
    }
    
    return i+1;
}

int count_infos(miscQdata *Misc)
{
    int i=255,j;
    bool found=false;
    
    while(i>=0 && !found)
    {
        j=2;
        
        while(j>=0 && !found)
        {
            if((Misc->info[i].str[j]!=0)||(Misc->info[i].price[j]!=0))
            {
                found=true;
            }
            else
            {
                j--;
            }
        }
        
        if(Misc->info[i].name[0]!=0)
        {
            found=true;
        }
        
        if(!found)
        {
            i--;
        }
    }
    
    return i+1;
}

int count_warprings(miscQdata *Misc)
{
    int i=15,j;
    bool found=false;
    
    while(i>=0 && !found)
    {
        j=7;
        
        while(j>=0 && !found)
        {
            if((Misc->warp[i].dmap[j]!=0)||(Misc->warp[i].scr[j]!=0))
            {
                found=true;
            }
            else
            {
                j--;
            }
        }
        
        if(!found)
        {
            i--;
        }
    }
    
    return i+1;
}

int count_palcycles(miscQdata *Misc)
{
    int i=255,j;
    bool found=false;
    
    while(i>=0 && !found)
    {
        j=2;
        
        while(j>=0 && !found)
        {
            if(Misc->cycles[i][j].count!=0)
            {
                found=true;
            }
            else
            {
                j--;
            }
        }
        
        if(!found)
        {
            i--;
        }
    }
    
    return i+1;
}

void clear_screen(mapscr *temp_scr)
{
    temp_scr->zero_memory();
    
    for(int j=0; j<6; j++)
    {
        temp_scr->layeropacity[j]=255;
    }
    
    for(int j=0; j<32; j++)
    {
        temp_scr->ffwidth[j] = 15;
        temp_scr->ffheight[j] = 15;
        //temp_scr->a[j][0] = 10000;
        //temp_scr->a[j][1] = 10000;
    }
}

int readdmaps(PACKFILE *f, zquestheader *Header, word, word, word start_dmap, word max_dmaps, bool keepdata)
{
    word dmapstoread=0;
    dmap tempDMap;
    
    int dummy;
    word s_version=0, s_cversion=0;
    byte padding;
    
    if(keepdata==true)
    {
        for(int i=0; i<max_dmaps; i++)
        {
            memset(&DMaps[start_dmap+i],0,sizeof(dmap));
            sprintf(DMaps[start_dmap+i].title,"                    ");
            sprintf(DMaps[start_dmap+i].intro,"                                                                        ");
            DMaps[start_dmap+i].type |= dmCAVE;
        }
    }
    
    if(!Header || Header->zelda_version > 0x192)
    {
        //section version info
        if(!p_igetw(&s_version,f,true))
        {
            return qe_invalid;
        }
	
	ffcore.quest_format[vDMaps] = s_version;
        
        //al_trace("DMaps version %d\n", s_version);
        
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
        if((Header->zelda_version < 0x192)||
                ((Header->zelda_version == 0x192)&&(Header->build<5)))
        {
            dmapstoread=32;
        }
        else if(s_version <= 4)
        {
            dmapstoread=OLDMAXDMAPS;
        }
        else
        {
            dmapstoread=MAXDMAPS;
        }
    }
    
    dmapstoread=zc_min(dmapstoread, max_dmaps);
    dmapstoread=zc_min(dmapstoread, MAXDMAPS-start_dmap);
    
    for(int i=start_dmap; i<dmapstoread+start_dmap; i++)
    {
        memset(&tempDMap,0,sizeof(dmap));
        sprintf(tempDMap.title,"                    ");
        sprintf(tempDMap.intro,"                                                                        ");
        
        if(!p_getc(&tempDMap.map,f,keepdata))
        {
            return qe_invalid;
        }
        
        if(s_version <= 4)
        {
            byte tempbyte;
            
            if(!p_getc(&tempbyte,f,keepdata))
            {
                return qe_invalid;
            }
            
            tempDMap.level=(word)tempbyte;
        }
        else
        {
            if(!p_igetw(&tempDMap.level,f,keepdata))
            {
                return qe_invalid;
            }
        }
        
        if(!p_getc(&tempDMap.xoff,f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&tempDMap.compass,f,keepdata))
        {
            return qe_invalid;
        }
        
        if(s_version > 8) // February 2009
        {
            if(!p_igetw(&tempDMap.color,f,true))
            {
                return qe_invalid;
            }
        }
        else
        {
            byte tempbyte;
            
            if(!p_getc(&tempbyte,f,true))
            {
                return qe_invalid;
            }
            
            tempDMap.color = (word)tempbyte;
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
        
        if((tempDMap.type & dmfTYPE) == dmOVERW &&
           (!Header || Header->zelda_version >= 0x210)) // Not sure exactly when this changed
            tempDMap.xoff = 0;
            
        for(int j=0; j<8; j++)
        {
            if(!p_getc(&tempDMap.grid[j],f,keepdata))
            {
                return qe_invalid;
            }
        }
        
        if(Header && ((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<41))))
        {
            if(tempDMap.level>0&&tempDMap.level<10)
            {
                sprintf(tempDMap.title,"LEVEL-%d             ", tempDMap.level);
            }
            
            if(i==0 && Header->zelda_version <= 0x190)
            {
                tempDMap.cont-=tempDMap.xoff;
                tempDMap.compass-=tempDMap.xoff;
            }
            
            //forgotten -DD
            if(tempDMap.level==0)
            {
                tempDMap.flags=dmfCAVES|dmf3STAIR|dmfWHIRLWIND|dmfGUYCAVES;
            }
        }
        else
        {
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
            
            if(Header && ((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<152))))
            {
                if(keepdata==true)
                {
                    memcpy(&DMaps[i], &tempDMap, sizeof(tempDMap));
                }
                
                continue;
            }
            
            if(Header && (Header->zelda_version < 0x193))
            {
                if(!p_getc(&padding,f,keepdata))
                {
                    return qe_invalid;
                }
            }
            if ( s_version >= 11 )
	    {
		    if(!p_igetl(&tempDMap.minimap_1_tile,f,keepdata))
		    {
			return qe_invalid;
		    }
	    }
	    else
	    {
		    if(!p_igetw(&tempDMap.minimap_1_tile,f,keepdata))
		    {
			return qe_invalid;
		    }
	    }
            
            if(!p_getc(&tempDMap.minimap_1_cset,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(Header && (Header->zelda_version < 0x193))
            {
                if(!p_getc(&padding,f,keepdata))
                {
                    return qe_invalid;
                }
            }
            
	    if ( s_version >= 11 )
	    {
		    if(!p_igetl(&tempDMap.minimap_2_tile,f,keepdata))
		    {
			return qe_invalid;
		    }
	    }
	    else
	    {
		    if(!p_igetw(&tempDMap.minimap_2_tile,f,keepdata))
		    {
			return qe_invalid;
		    }
	    }
            if(!p_getc(&tempDMap.minimap_2_cset,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(Header && (Header->zelda_version < 0x193))
            {
                if(!p_getc(&padding,f,keepdata))
                {
                    return qe_invalid;
                }
            }
            
	    if ( s_version >= 11 )
	    {
		    if(!p_igetl(&tempDMap.largemap_1_tile,f,keepdata))
		    {
			return qe_invalid;
		    }
	    }
	    else
	    {
		    if(!p_igetw(&tempDMap.largemap_1_tile,f,keepdata))
		    {
			return qe_invalid;
		    }
	    }
	    
            if(!p_getc(&tempDMap.largemap_1_cset,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(Header && (Header->zelda_version < 0x193))
            {
            
                if(!p_getc(&padding,f,keepdata))
                {
                    return qe_invalid;
                }
            }
            
	    if ( s_version >= 11 )
	    {
		    if(!p_igetl(&tempDMap.largemap_2_tile,f,keepdata))
		    {
			return qe_invalid;
		    }
	    }
	    else
	    {
		    if(!p_igetw(&tempDMap.largemap_2_tile,f,keepdata))
		    {
			return qe_invalid;
		    }
	    }
            if(!p_getc(&tempDMap.largemap_2_cset,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!pfread(&tempDMap.tmusic,sizeof(DMaps[0].tmusic),f,true))
            {
                return qe_invalid;
            }
        }
        
        if(s_version>1)
        {
            if(!p_getc(&tempDMap.tmusictrack,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&tempDMap.active_subscreen,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&tempDMap.passive_subscreen,f,keepdata))
            {
                return qe_invalid;
            }
        }
        
        if(s_version>2)
        {
            byte di[32];
            
            if(!pfread(&di, 32, f, true)) return qe_invalid;
            
            for(int j=0; j<MAXITEMS; j++)
            {
                if(di[j/8] & (1 << (j%8))) tempDMap.disableditems[j]=1;
                else tempDMap.disableditems[j]=0;
            }
        }
        
        if(s_version >= 6)
        {
            if(!p_igetl(&tempDMap.flags,f,keepdata))
            {
                return qe_invalid;
            }
        }
	
	
        else if(s_version>3)
        {
            char temp;
            
            if(!p_getc(&temp,f,keepdata))
            {
                return qe_invalid;
            }
            
            tempDMap.flags = temp;
        }
        else if(tempDMap.level==0 && ((Header->zelda_version < 0x211) || ((Header->zelda_version == 0x211) && (Header->build<18))))
        {
            tempDMap.flags=dmfCAVES|dmf3STAIR|dmfWHIRLWIND|dmfGUYCAVES;
        }
        else
            tempDMap.flags=0;
            
        if(s_version<7)
        {
            if(tempDMap.level==0 && get_bit(deprecated_rules,14))
                tempDMap.flags|= dmfVIEWMAP;
        }
        
        if(s_version<8)
        {
            if(tempDMap.level==0 && (tempDMap.type&dmfTYPE)==dmDNGN)
            {
                tempDMap.type &= ~dmDNGN;
                tempDMap.type |= dmCAVE;
            }
            else if((tempDMap.type&dmfTYPE)==dmCAVE)
            {
                tempDMap.flags |= dmfMINIMAPCOLORFIX;
            }
        }
        
        if(Header && ((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>=41)))
                && (Header->zelda_version < 0x193))
        {
            if(!p_getc(&padding,f,keepdata))
            {
                return qe_invalid;
            }
        }
	
	if(s_version >= 10)
        {
            if(!p_getc(&tempDMap.sideview,f,keepdata))
            {
                return qe_invalid;
            }
        }
	if(s_version < 10) tempDMap.sideview = 0;
        
        if(keepdata==true)
        {
            memcpy(&DMaps[i], &tempDMap, sizeof(tempDMap));
        }
    }
    
    return 0;
}

int readmisccolors(PACKFILE *f, zquestheader *Header, miscQdata *Misc, bool keepdata)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    
    miscQdata temp_misc;
    word s_version=0, s_cversion=0;
    int tempsize=0;
    
    memcpy(&temp_misc,Misc,sizeof(temp_misc));
    
    //section version info
    if(!p_igetw(&s_version,f,true))
    {
        return qe_invalid;
    }
    
    ffcore.quest_format[vColours] = s_version;
    
    al_trace("Misc Colours section version: %d\n", s_version);
    
    //al_trace("Misc. colors version %d\n", s_version);
    if(!p_igetw(&s_cversion,f,true))
    {
        return qe_invalid;
    }
    
    
    //section size
    if(!p_igetl(&tempsize,f,true))
    {
        return qe_invalid;
    }
    
    //finally...  section data
    readsize=0;
    
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
    
    if(!p_getc(&temp_misc.colors.subscr_shadow,f,true))
    {
        return qe_invalid;
    }
    
    if(s_version < 2)
    {
        temp_misc.colors.msgtext = 0x01;
    }
    else
    {
        if(!p_getc(&temp_misc.colors.msgtext, f, true))
        {
            return qe_invalid;
        }
    }
    
    if ( s_version >= 3 ) //expanded tile pages to 825
    {
	if(!p_igetl(&temp_misc.colors.new_triforce_tile,f,true))
        {
             return qe_invalid;
        }    
    }
    else
    {
	al_trace("Copying misc.colors.*tile to misc.colors.new_*tile.\n");
	temp_misc.colors.new_triforce_tile = temp_misc.colors.triforce_tile;
	    //al_trace("Old triforce tile: %d\n", temp_misc.colors.triforce_tile);
	   // al_trace("New triforce tile: %d\n", temp_misc.colors.new_triforce_tile);
    }
    if ( s_version >= 3 ) //expanded tile pages to 825
    {
	if(!p_igetl(&temp_misc.colors.new_triframe_tile,f,true))
        {
             return qe_invalid;
        }  
    }
    else
    {
	temp_misc.colors.new_triframe_tile = temp_misc.colors.triframe_tile;
	   // al_trace("Old triframe tile: %d\n", temp_misc.colors.triframe_tile);
	   // al_trace("New triframe tile: %d\n", temp_misc.colors.new_triframe_tile);
	    
    }
    if ( s_version >= 3 ) //expanded tile pages to 825
    {
	if(!p_igetl(&temp_misc.colors.new_overworld_map_tile,f,true))
        {
             return qe_invalid;
        } 
    }
    else
    {
	temp_misc.colors.new_overworld_map_tile = temp_misc.colors.overworld_map_tile;    
	  //  al_trace("Old overworld map tile: %d\n", temp_misc.colors.overworld_map_tile);
	  //  al_trace("New overworld map tile: %d\n", temp_misc.colors.new_overworld_map_tile);
	    
    }
    if ( s_version >= 3 ) //expanded tile pages to 825
    {
	if(!p_igetl(&temp_misc.colors.new_dungeon_map_tile,f,true))
        {
             return qe_invalid;
        } 
    }
    else
    {
	temp_misc.colors.new_dungeon_map_tile = temp_misc.colors.dungeon_map_tile;    
	//al_trace("Old dungeon map tile: %d\n", temp_misc.colors.dungeon_map_tile);
	  //  al_trace("New dungeon map tile: %d\n", temp_misc.colors.new_dungeon_map_tile);
    }
    if ( s_version >= 3 ) //expanded tile pages to 825
    {
	if(!p_igetl(&temp_misc.colors.new_blueframe_tile,f,true))
        {
             return qe_invalid;
        }
    }
    else
    {
	temp_misc.colors.new_blueframe_tile = temp_misc.colors.blueframe_tile;
	//al_trace("Old blueframe tile: %d\n", temp_misc.colors.blueframe_tile);
	  //  al_trace("New blueframe tile: %d\n", temp_misc.colors.new_blueframe_tile);
    }
    if ( s_version >= 3 ) //expanded tile pages to 825
    {
	if(!p_igetl(&temp_misc.colors.new_HCpieces_tile,f,true))
        {
             return qe_invalid;
        }
    }
    else
    {	    
	temp_misc.colors.new_HCpieces_tile = temp_misc.colors.HCpieces_tile;  
	//al_trace("Old HCP tile: %d\n", temp_misc.colors.HCpieces_tile);
	 //   al_trace("New HCP tile: %d\n", temp_misc.colors.new_HCpieces_tile);	    
    }
    
    
    if(keepdata==true)
    {
        memcpy(Misc, &temp_misc, sizeof(temp_misc));
    }
    
    return 0;
}

int readgameicons(PACKFILE *f, zquestheader *, miscQdata *Misc, bool keepdata)
{
    miscQdata temp_misc;
    word s_version=0, s_cversion=0;
    byte icons;
    int tempsize=0;
    
    memcpy(&temp_misc,Misc,sizeof(temp_misc));
    
    //section version info
    if(!p_igetw(&s_version,f,true))
    {
        return qe_invalid;
    }
    
    ffcore.quest_format[vIcons] = s_version;
    
    //al_trace("Game icons version %d\n", s_version);
    if(!p_igetw(&s_cversion,f,true))
    {
        return qe_invalid;
    }
    
    
    //section size
    if(!p_igetl(&tempsize,f,true))
    {
        return qe_invalid;
    }
    
    //finally...  section data
    readsize=0;
    
    icons=4;
    
    if ( s_version >= 10 )
    {
	    for(int i=0; i<icons; i++)
	    {
		if(!p_igetl(&temp_misc.icons[i],f,true))
		{
		    return qe_invalid;
		}
	    }	   
    }
    else
    {	    
	    for(int i=0; i<icons; i++)
	    {
		if(!p_igetw(&temp_misc.icons[i],f,true))
		{
		    return qe_invalid;
		}
	    }
    }
    if(keepdata==true)
    {
        memcpy(Misc, &temp_misc, sizeof(temp_misc));
    }
    
    return 0;
}

int readmisc(PACKFILE *f, zquestheader *Header, miscQdata *Misc, bool keepdata)
{
    word maxinfos=256;
    word maxshops=256;
    word shops=16, infos=16, warprings=8, palcycles=256, windwarps=9, triforces=8, icons=4;
    word ponds=16, pondsize=72, expansionsize=98*2;
    byte tempbyte, padding;
    miscQdata temp_misc;
    word s_version=0, s_cversion=0;
    word swaptmp;
    int tempsize=0;
    
    memcpy(&temp_misc,Misc,sizeof(temp_misc));
    
    for(int i=0; i<maxshops; ++i)
    {
        memset(&temp_misc.shop, 0, sizeof(shoptype)*256);
    }
    
    for(int i=0; i<maxinfos; ++i)
    {
        memset(&temp_misc.info, 0, sizeof(infotype)*256);
    }
    
    if(Header->zelda_version > 0x192)
    {
        //section version info
        if(!p_igetw(&s_version,f,true))
        {
            return qe_invalid;
        }
	
	ffcore.quest_format[vMisc] = s_version;
        
        //al_trace("Misc. data version %d\n", s_version);
        if(!p_igetw(&s_cversion,f,true))
        {
            return qe_invalid;
        }
        
        
        //section size
        if(!p_igetl(&tempsize,f,true))
        {
            return qe_invalid;
        }
    }
    
    //finally...  section data
    readsize=0;
    
    //shops
    if(Header->zelda_version > 0x192)
    {
        if(!p_igetw(&shops,f,true))
        {
            return qe_invalid;
        }
        
    }
    
    for(int i=0; i<shops; i++)
    {
        if(s_version > 6)
        {
            if(!pfread(temp_misc.shop[i].name,sizeof(temp_misc.shop[i].name),f,true))
            {
                return qe_invalid;
            }
        }
        
        for(int j=0; j<3; j++)
        {
            if(!p_getc(&temp_misc.shop[i].item[j],f,true))
            {
                return qe_invalid;
            }
            
            if(s_version < 4)
            {
                temp_misc.shop[i].hasitem[j] = (temp_misc.shop[i].item[j] == 0) ? 0 : 1;
            }
        }
        
        if(Header->zelda_version < 0x193)
        {
            if(!p_getc(&tempbyte,f,true))
            {
                return qe_invalid;
            }
        }
        
        for(int j=0; j<3; j++)
        {
            if(!p_igetw(&temp_misc.shop[i].price[j],f,true))
            {
                return qe_invalid;
            }
        }
        
        if(s_version > 3)
        {
            for(int j=0; j<3; j++)
            {
                if(!p_getc(&temp_misc.shop[i].hasitem[j],f,true))
                    return qe_invalid;
            }
        }
	
	/*
	if(s_version < 8)
        {
            for(int j=0; j<3; j++)
            {
                (&temp_misc.shop[i].str[j])=0; //initialise.
            }
        }
	*/
    }
    
    //filter all the 0 items to the end (yeah, bubble sort; sue me)
    for(int i=0; i<maxshops; ++i)
    {
        for(int j=0; j<3-1; j++)
        {
            for(int k=0; k<2-j; k++)
            {
                if(temp_misc.shop[i].hasitem[k]==0)
                {
                    swaptmp = temp_misc.shop[i].item[k];
                    temp_misc.shop[i].item[k] = temp_misc.shop[i].item[k+1];
                    temp_misc.shop[i].item[k+1] = swaptmp;
                    swaptmp = temp_misc.shop[i].price[k];
                    temp_misc.shop[i].price[k] = temp_misc.shop[i].price[k+1];
                    temp_misc.shop[i].price[k+1] = swaptmp;
                    swaptmp = temp_misc.shop[i].hasitem[k];
                    temp_misc.shop[i].hasitem[k] = temp_misc.shop[i].hasitem[k+1];
                    temp_misc.shop[i].hasitem[k+1] = swaptmp;
                }
            }
        }
    }
    
    //infos
    if(Header->zelda_version > 0x192)
    {
        if(!p_igetw(&infos,f,true))
        {
            return qe_invalid;
        }
    }
    
    for(int i=0; i<infos; i++)
    {
        if(s_version > 6)
        {
            if(!pfread(temp_misc.info[i].name,sizeof(temp_misc.info[i].name),f,true))
            {
                return qe_invalid;
            }
        }
        
        for(int j=0; j<3; j++)
        {
            if((Header->zelda_version < 0x192)||
                    ((Header->zelda_version == 0x192)&&(Header->build<146)))
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
        
        if(Header->zelda_version < 0x193)
        {
            if(!p_getc(&tempbyte,f,true))
            {
                return qe_invalid;
            }
        }
        
        if((Header->zelda_version == 0x192)&&(Header->build>145))
        {
            if(!p_getc(&padding,f,true))
            {
                return qe_invalid;
            }
        }
        
        for(int j=0; j<3; j++)
        {
            if(!p_igetw(&temp_misc.info[i].price[j],f,true))
            {
                return qe_invalid;
            }
        }
    }
    
    //filter all the 0 strings to the end (yeah, bubble sort; sue me)
    for(int i=0; i<maxinfos; ++i)
    {
        for(int j=0; j<3-1; j++)
        {
            for(int k=0; k<2-j; k++)
            {
                if(temp_misc.info[i].str[k]==0)
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
    if(s_version > 5)
        warprings++;
        
    if(Header->zelda_version > 0x192)
    {
        if(!p_igetw(&warprings,f,true))
        {
            return qe_invalid;
        }
    }
    
    for(int i=0; i<warprings; i++)
    {
        for(int j=0; j<8+((s_version > 5)?1:0); j++)
        {
            if(s_version <= 3)
            {
                if(!p_getc(&tempbyte,f,true))
                {
                    return qe_invalid;
                }
                
                temp_misc.warp[i].dmap[j]=(word)tempbyte;
            }
            else
            {
                if(!p_igetw(&temp_misc.warp[i].dmap[j],f,true))
                {
                    return qe_invalid;
                }
            }
        }
        
        for(int j=0; j<8+((s_version > 5)?1:0); j++)
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
        
        if(Header->zelda_version < 0x193)
        {
            if(!p_getc(&tempbyte,f,true))
            {
                return qe_invalid;
            }
        }
    }
    
    //palette cycles
    if(Header->zelda_version < 0x193)                         //in 1.93+, palette cycling is saved with the palettes
    {
        for(int i=0; i<256; i++)
        {
            for(int j=0; j<3; j++)
            {
                temp_misc.cycles[i][j].first=0;
                temp_misc.cycles[i][j].count=0;
                temp_misc.cycles[i][j].speed=0;
            }
        }
        
        if((Header->zelda_version < 0x192)||
                ((Header->zelda_version == 0x192)&&(Header->build<73)))
        {
            palcycles=16;
        }
        
        for(int i=0; i<palcycles; i++)
        {
            for(int j=0; j<3; j++)
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
    
    //Wind warps are now just another warp ring.
    if(s_version <= 5)
    {
        if(Header->zelda_version > 0x192)
        {
            if(!p_igetw(&windwarps,f,true))
            {
                return qe_invalid;
            }
        }
        
        for(int i=0; i<windwarps; i++)
        {
            if(s_version <= 3)
            {
                if(!p_getc(&tempbyte,f,true))
                {
                    return qe_invalid;
                }
                
                temp_misc.warp[8].dmap[i]=tempbyte;
            }
            else
            {
                if(!p_igetw(&temp_misc.warp[8].dmap[i],f,true))
                {
                    return qe_invalid;
                }
            }
            
            if(!p_getc(&temp_misc.warp[8].scr[i],f,true))
            {
                return qe_invalid;
            }
            
            temp_misc.warp[8].size = 9;
            
            if(s_version == 5)
            {
                if(!p_getc(&tempbyte,f,true))
                {
                    return qe_invalid;
                }
            }
        }
    }
    
    
    //triforce pieces
    for(int i=0; i<triforces; i++)
    {
        if(!p_getc(&temp_misc.triforce[i],f,true))
        {
            return qe_invalid;
        }
    }
    
    //misc color data
    if(s_version<3)
    {
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
        
        temp_misc.colors.msgtext = 0x01;
        
        if(Header->zelda_version < 0x193)
        {
            for(int i=0; i<7; i++)
            {
                if(!p_getc(&tempbyte,f,true))
                {
                    return qe_invalid;
                }
            }
        }
        
        if((Header->zelda_version == 0x192)&&(Header->build>145))
        {
            for(int i=0; i<256; i++)
            {
                if(!p_getc(&tempbyte,f,true))
                {
                    return qe_invalid;
                }
            }
        }
        
        if(s_version>1)
        {
            if(!p_getc(&temp_misc.colors.subscr_shadow,f,true))
            {
                return qe_invalid;
            }
        }
        
        //save game icons
        if((Header->zelda_version < 0x192)||
                ((Header->zelda_version == 0x192)&&(Header->build<73)))
        {
            icons=3;
        }
        
        for(int i=0; i<icons; i++)
        {
            if(!p_igetw(&temp_misc.icons[i],f,true))
            {
                return qe_invalid;
            }
        }
    }
    
    if((Header->zelda_version < 0x192)||
            ((Header->zelda_version == 0x192)&&(Header->build<30)))
    {
        if(keepdata==true)
        {
            memcpy(Misc, &temp_misc, sizeof(temp_misc));
        }
        
        return 0;
    }
    
    //pond information
    if(Header->zelda_version < 0x193)
    {
        if((Header->zelda_version == 0x192)&&(Header->build<146))
        {
            pondsize=25;
        }
        
        for(int i=0; i<ponds; i++)
        {
            for(int j=0; j<pondsize; j++)
            {
                if(!p_getc(&tempbyte,f,true))
                {
                    return qe_invalid;
                    
                }
            }
        }
    }
    
    //end string
    if((Header->zelda_version < 0x192)||
            ((Header->zelda_version == 0x192)&&(Header->build<146)))
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
    if(Header->zelda_version < 0x193)
    {
        if((Header->zelda_version == 0x192)&&(Header->build<73))
        {
            expansionsize=99*2;
        }
        
        for(int i=0; i<expansionsize; i++)
        {
            if(!p_getc(&tempbyte,f,true))
            {
                return qe_invalid;
            }
        }
    }
    //shops v8
    
    
    if(s_version >= 8)
    {
	for(int i=0; i<shops; i++)
        {
            for(int j=0; j<3; j++)
            {
                if(!p_igetw(&temp_misc.shop[i].str[j],f,true))
                    return qe_invalid;
            }
        }
    }
    
    memset(&temp_misc.questmisc, 0, sizeof(long)*32);
    memset(&temp_misc.questmisc_strings, 0, sizeof(char)*4096);
    
    //v9 includes quest misc[32]
    if(s_version >= 9)
    {
	for ( int q = 0; q < 32; q++ ) 
	{
		if(!p_igetl(&temp_misc.questmisc[q],f,true))
                    return qe_invalid;
	}
	for ( int q = 0; q < 32; q++ ) 
	{
		for ( int j = 0; j < 128; j++ )
		if(!p_getc(&temp_misc.questmisc_strings[q][j],f,true))
                    return qe_invalid;
	}
    }
    
    if(keepdata==true)
    {
        memcpy(Misc, &temp_misc, sizeof(temp_misc));
    }
    
    return 0;
}

extern char *item_string[ITEMCNT];
extern const char *old_item_string[iLast];
extern char *weapon_string[WPNCNT];
extern const char *old_weapon_string[wLast];

int readitems(PACKFILE *f, word version, word build, bool keepdata, bool zgpmode)
{
    byte padding;
    int  dummy;
    word items_to_read=MAXITEMS;
    itemdata tempitem;
    word s_version=0, s_cversion=0;
    word dummy_word;
    
    if(version < 0x186)
    {
        items_to_read=64;
    }
    
    if(version > 0x192)
    {
        items_to_read=0;
        
        //section version info
        if(!p_igetw(&s_version,f,true))
        {
            return qe_invalid;
        }
	
	ffcore.quest_format[vItems] = s_version;
        
        //al_trace("Items version %d\n", s_version);
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
        if(!p_igetw(&items_to_read,f,true))
        {
            return qe_invalid;
        }
    }
    
    if(s_version>1)
    {
        for(int i=0; i<items_to_read; i++)
        {
            char tempname[64];
            
            if(!pfread(tempname, 64, f, keepdata))
            {
                return qe_invalid;
            }
            
            if(keepdata)
            {
                strcpy(item_string[i], tempname);
            }
        }
    }
    else
    {
        if(keepdata)
        {
            for(int i=0; i<ITEMCNT; i++)
            {
                reset_itemname(i);
            }
        }
    }
    
    if(keepdata)
    {
        for(int i=0; i<MAXITEMS; i++)
        {
            memset(&itemsbuf[i], 0, sizeof(itemdata));
            itemsbuf[i].power=itemsbuf[i].flags=itemsbuf[i].wpn=itemsbuf[i].wpn2=itemsbuf[i].wpn3=
                                                    itemsbuf[i].wpn4=itemsbuf[i].pickup_hearts=itemsbuf[i].misc1=
                                                            itemsbuf[i].misc2=itemsbuf[i].magic=tempitem.usesound=0;
            itemsbuf[i].count=-1;
            itemsbuf[i].playsound=WAV_SCALE;
            reset_itembuf(&itemsbuf[i],i);
        }
    }
    
    for(int i=0; i<items_to_read; i++)
    {
        memset(&tempitem, 0, sizeof(itemdata));
        reset_itembuf(&tempitem,i);
        
	    
	if ( s_version > 35 ) //expanded tiles	
	{    
		if(!p_igetl(&tempitem.tile,f,true))
		{
		    return qe_invalid;
		}
	}
	else
	{
		if(!p_igetw(&tempitem.tile,f,true))
		{
		    return qe_invalid;
		}
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
        
        if(version < 0x193)
        {
            if(!p_getc(&padding,f,true))
            {
                return qe_invalid;
            }
            
            if((version < 0x192)||((version == 0x192)&&(build<186)))
            {
                switch(i)
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
                
                tempitem.count=-1;
                tempitem.flags=tempitem.wpn=tempitem.wpn2=tempitem.wpn3=tempitem.wpn3=tempitem.pickup_hearts=
                                                tempitem.misc1=tempitem.misc2=tempitem.usesound=0;
                tempitem.family=0xFF;
                tempitem.playsound=WAV_SCALE;
                reset_itembuf(&tempitem,i);
                
                if(keepdata==true)
                {
                    memcpy(&itemsbuf[i], &tempitem, sizeof(itemdata));
                }
                
                continue;
            }
        }
        
        if(!p_igetl(&tempitem.ltm,f,true))
        {
            return qe_invalid;
        }
        
        if(version < 0x193)
        {
            for(int q=0; q<12; q++)
            {
                if(!p_getc(&padding,f,true))
                {
                    return qe_invalid;
                }
            }
        }
        
        if(s_version>1)
        {
	    if ( s_version >= 31 )
	    {
		if(!p_igetl(&tempitem.family,f,true))
                {
                    return qe_invalid;
		}    
	    }
            else
	    {		    
	        if(!p_getc(&tempitem.family,f,true))
                {
                    return qe_invalid;
		}
            }
            if(s_version < 16)
                if(tempitem.family == 0xFF)
                    tempitem.family = itype_misc;
                    
            if(!p_getc(&tempitem.fam_type,f,true))
            {
                return qe_invalid;
            }
            
            if(s_version>5)
            {
		if(s_version>=31)
		{
			if(!p_igetl(&tempitem.power,f,true))
			{
				return qe_invalid;
			}
		}
		else
		{
			if(!p_getc(&tempitem.power,f,true))
			{
			return qe_invalid;
			}
		}
                
                if(!p_igetw(&tempitem.flags,f,true))
                {
                    return qe_invalid;
                }
            }
            else
            {
                //tempitem.power = tempitem.fam_type;
                char tempchar;
                
                if(!p_getc(&tempchar,f,true))
                {
                    return qe_invalid;
                }
                
                tempitem.flags |= (tempchar ? ITEM_GAMEDATA : 0);
            }
            
            if(!p_igetw(&tempitem.script,f,true))
            {
                return qe_invalid;
            }
            
            if(s_version<=3)
            {
                if(tempitem.script > NUMSCRIPTITEM)
                {
                    tempitem.script = 0;
                }
            }
            
            if(!p_getc(&tempitem.count,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&tempitem.amount,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&tempitem.collect_script,f,true))
            {
                return qe_invalid;
            }
            
            if(s_version<=3)
            {
                if(tempitem.collect_script > NUMSCRIPTITEM)
                {
                    tempitem.collect_script = 0;
                }
            }
            
            if(!p_igetw(&tempitem.setmax,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&tempitem.max,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&tempitem.playsound,f,true))
            {
                return qe_invalid;
            }
            
            for(int j=0; j<8; j++)
            {
                if(!p_igetl(&tempitem.initiald[j],f,true))
                {
                    return qe_invalid;
                }
            }
            
            for(int j=0; j<2; j++)
            {
                if(!p_getc(&tempitem.initiala[j],f,true))
                {
                    return qe_invalid;
                }
            }
            
            if(s_version>4)
            {
                if(s_version>5)
                {
                    if(!p_getc(&tempitem.wpn,f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_getc(&tempitem.wpn2,f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_getc(&tempitem.wpn3,f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_getc(&tempitem.wpn4,f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(s_version>=15)
                    {
                        if(!p_getc(&tempitem.wpn5,f,true))
                        {
                            return qe_invalid;
                        }
                        
                        if(!p_getc(&tempitem.wpn6,f,true))
                        {
                            return qe_invalid;
                        }
                        
                        if(!p_getc(&tempitem.wpn7,f,true))
                        {
                            return qe_invalid;
                        }
                        
                        if(!p_getc(&tempitem.wpn8,f,true))
                        {
                            return qe_invalid;
                        }
                        
                        if(!p_getc(&tempitem.wpn9,f,true))
                        {
                            return qe_invalid;
                        }
                        
                        if(!p_getc(&tempitem.wpn10,f,true))
                        {
                            return qe_invalid;
                        }
                    }
                    
                    if(!p_getc(&tempitem.pickup_hearts,f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(s_version<15)
                    {
                        if(!p_igetw(&dummy_word,f,true))
                        {
                            return qe_invalid;
                        }
                        
                        tempitem.misc1=dummy_word;
                        
                        if(!p_igetw(&dummy_word,f,true))
                        {
                            return qe_invalid;
                        }
                        
                        tempitem.misc2=dummy_word;
                    }
                    else
                    {
                        if(!p_igetl(&tempitem.misc1,f,true))
                        {
                            return qe_invalid;
                        }
                        
                        if(!p_igetl(&tempitem.misc2,f,true))
                        {
                            return qe_invalid;
                        }
                        
                        // Version 24: shICE -> shSCRIPT; previously, all shields could block script weapons
                        if(s_version<24)
                        {
                            if(tempitem.family==itype_shield)
                            {
                                tempitem.misc1|=shSCRIPT;
                            }
                        }
                    }
                    
                    if(!p_getc(&tempitem.magic,f,true))
                    {
                        return qe_invalid;
                    }
                }
                else
                {
                    char tempchar;
                    
                    if(!p_getc(&tempchar,f,true))
                    {
                        return qe_invalid;
                    }
                    
                    tempitem.flags |= (tempchar ? ITEM_EDIBLE : 0);
                }
                
                // June 2007: more misc. attributes
                if(s_version>=12)
                {
                    if(s_version<15)
                    {
                        if(!p_igetw(&dummy_word,f,true))
                        {
                            return qe_invalid;
                        }
                        
                        tempitem.misc3=dummy_word;
                        
                        if(!p_igetw(&dummy_word,f,true))
                        {
                            return qe_invalid;
                        }
                        
                        tempitem.misc4=dummy_word;
                    }
                    else
                    {
                        if(!p_igetl(&tempitem.misc3,f,true))
                        {
                            return qe_invalid;
                        }
                        
                        if(!p_igetl(&tempitem.misc4,f,true))
                        {
                            return qe_invalid;
                        }
                        
                        if(!p_igetl(&tempitem.misc5,f,true))
                        {
                            return qe_invalid;
                        }
                        
                        if(!p_igetl(&tempitem.misc6,f,true))
                        {
                            return qe_invalid;
                        }
                        
                        if(!p_igetl(&tempitem.misc7,f,true))
                        {
                            return qe_invalid;
                        }
                        
                        if(!p_igetl(&tempitem.misc8,f,true))
                        {
                            return qe_invalid;
                        }
                        
                        if(!p_igetl(&tempitem.misc9,f,true))
                        {
                            return qe_invalid;
                        }
                        
                        if(!p_igetl(&tempitem.misc10,f,true))
                        {
                            return qe_invalid;
                        }
                    }
                    
                    if(!p_getc(&tempitem.usesound,f,true))
                    {
                        return qe_invalid;
                    }
                }
            }
	    
	      if ( s_version >= 26 )  //! New itemdata vars for weapon editor. -Z
		{			// temp.useweapon, temp.usedefence, temp.weaprange, temp.weap_pattern[ITEM_MOVEMENT_PATTERNS]
			if(!p_getc(&tempitem.useweapon,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_getc(&tempitem.usedefence,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.weaprange,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.weapduration,f,true))
                        {
                            return qe_invalid;
                        }
			for ( int q = 0; q < ITEM_MOVEMENT_PATTERNS; q++ ) {
				
				if(!p_igetl(&tempitem.weap_pattern[q],f,true))
				{
				    return qe_invalid;
				}
			}
		
		}
		
		if ( s_version >= 27 )  //! New itemdata vars for weapon editor. -Z
		{			// temp.useweapon, temp.usedefence, temp.weaprange, temp.weap_pattern[ITEM_MOVEMENT_PATTERNS]
			if(!p_igetl(&tempitem.duplicates,f,true))
                        {
                            return qe_invalid;
                        }
			for ( int q = 0; q < INITIAL_D; q++ )
			{
				if(!p_igetl(&tempitem.weap_initiald[q],f,true))
				{
					return qe_invalid;
				}
			}
			for ( int q = 0; q < INITIAL_A; q++ )
			{
				if(!p_getc(&tempitem.weap_initiala[q],f,true))
				{
					return qe_invalid;
				}
			}
			
			if(!p_getc(&tempitem.drawlayer,f,true))
                        {
                            return qe_invalid;
                        }
			
			
			if(!p_igetl(&tempitem.hxofs,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.hyofs,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.hxsz,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.hysz,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.hzsz,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.xofs,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.yofs,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.weap_hxofs,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.weap_hyofs,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.weap_hxsz,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.weap_hysz,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.weap_hzsz,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.weap_xofs,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.weap_yofs,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetw(&tempitem.weaponscript,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.wpnsprite,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.magiccosttimer,f,true))
                        {
                            return qe_invalid;
                        }
			
		
		}
		if ( s_version >= 28 )  //! New itemdata vars for weapon editor. -Z
		{
			//Item Size FLags, TileWidth, TileHeight
			if(!p_igetl(&tempitem.overrideFLAGS,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.tilew,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.tileh,f,true))
                        {
                            return qe_invalid;
                        }
			
		}
		if ( s_version >= 29 )  //! More new vars. 
		{
			//Item Size FLags, TileWidth, TileHeight
			if(!p_igetl(&tempitem.weapoverrideFLAGS,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.weap_tilew,f,true))
                        {
                            return qe_invalid;
                        }
			if(!p_igetl(&tempitem.weap_tileh,f,true))
                        {
                            return qe_invalid;
                        }
		}
		if ( s_version >= 30 )  //! More new vars. 
		{
			//Pickup Type
			if(!p_igetl(&tempitem.pickup,f,true))
                        {
                            return qe_invalid;
                        }
		}
		if ( s_version >= 32 )  //! More new vars. 
		{
			//Pickup Type
			if(!p_igetw(&tempitem.pstring,f,true))
                        {
                            return qe_invalid;
                        }
		}
		if ( s_version >= 33 )  //! More new vars. 
		{
			//Pickup Type
			if(!p_igetw(&tempitem.pickup_string_flags,f,true))
                        {
                            return qe_invalid;
                        }
		}
		if ( s_version >= 34 )  //! cost counter
		{
			//Pickup Type
			if(!p_getc(&tempitem.cost_counter,f,true))
                        {
                            return qe_invalid;
                        }
		}

		if ( s_version < 34 )  //! set the default counter for older quests. 
		{
			if ( (tempitem.flags & ITEM_RUPEE_MAGIC) )
			{
				tempitem.cost_counter = 1;
			}
			else 
			{
				tempitem.cost_counter = 4;
			}
		}
		
		if ( s_version < 35 ) //new Lens of Truth flags		
		{
			if ( tempitem.family == itype_lens )
			{
				if ( get_bit(quest_rules,qr_RAFTLENS) ) 
				{
					tempitem.flags |= ITEM_FLAG4;
				}
				if ( get_bit(quest_rules,qr_LENSHINTS) ) 
				{
					tempitem.flags |= ITEM_FLAG1;
				}
				if ( get_bit(quest_rules,qr_LENSSEESENEMIES) ) 
				{
					tempitem.flags |= ITEM_FLAG5;
				}
				//if ( get_bit(quest_rules,qr_RAFTLENS) ) tempitem.flags &= lensflagHIDESECRETS;
				//What controlled this before? -Z
				//lensflagNOXRAY New option, not an old rule. -Z
			}
		}
		
		
        }
        else
        {
            tempitem.count=-1;
            tempitem.family=itype_misc;
            tempitem.flags=tempitem.wpn=tempitem.wpn2=tempitem.wpn3=tempitem.wpn3=tempitem.pickup_hearts=tempitem.misc1=tempitem.misc2=tempitem.usesound=0;
            tempitem.playsound=WAV_SCALE;
            reset_itembuf(&tempitem,i);
        }
        
        if(keepdata==true)
        {
            memcpy(&itemsbuf[i], &tempitem, sizeof(itemdata));
        }
        else if(zgpmode)
        {
            itemsbuf[i].tile=tempitem.tile;
            itemsbuf[i].misc=tempitem.misc;
            itemsbuf[i].csets=tempitem.csets;
            itemsbuf[i].frames=tempitem.frames;
            itemsbuf[i].speed=tempitem.speed;
            itemsbuf[i].delay=tempitem.delay;
            itemsbuf[i].ltm=tempitem.ltm;
        }
    }
    
    //////////////////////////////////////////////////////
    // Now do any updates because of new item additions
    // (These can't be done above because items_to_read
    // might be too low.)
    //////////////////////////////////////////////////////
    if(keepdata==true)
    {
        for(int i=0; i<MAXITEMS; i++)
        {
            memcpy(&tempitem, &itemsbuf[i], sizeof(itemdata));
            
            //Account for older quests that didn't have an actual item for the used letter
            if(s_version < 2 && i==iLetterUsed)
            {
                reset_itembuf(&tempitem, iLetterUsed);
                strcpy(item_string[i],old_item_string[i]);
                tempitem.tile = itemsbuf[iLetter].tile;
                tempitem.csets = itemsbuf[iLetter].csets;
                tempitem.misc = itemsbuf[iLetter].misc;
                tempitem.frames = itemsbuf[iLetter].frames;
                tempitem.speed = itemsbuf[iLetter].speed;
                tempitem.ltm = itemsbuf[iLetter].ltm;
            }
            
            if(s_version < 3)
            {
                switch(i)
                {
                case iRocsFeather:
                case iHoverBoots:
                case iSpinScroll:
                case iL2SpinScroll:
                case iCrossScroll:
                case iQuakeScroll:
                case iL2QuakeScroll:
                case iWhispRing:
                case iL2WhispRing:
                case iChargeRing:
                case iL2ChargeRing:
                case iPerilScroll:
                case iWalletL3:
                case iQuiverL4:
                case iBombBagL4:
                case iBracelet:
                case iL2Bracelet:
                case iOldGlove:
                case iL2Ladder:
                case iWealthMedal:
                case iL2WealthMedal:
                case iL3WealthMedal:
                    reset_itembuf(&tempitem, i);
                    strcpy(item_string[i],old_item_string[i]);
                    break;
                    
                case iSShield:
                    reset_itembuf(&tempitem, i);
                    strcpy(item_string[i],old_item_string[i]);
                    strcpy(item_string[iShield],old_item_string[iShield]);
                    strcpy(item_string[iMShield],old_item_string[iMShield]);
                    break;
                }
            }
            
            if(s_version < 5)
            {
                switch(i)
                {
                case iHeartRing:
                case iL2HeartRing:
                case iL3HeartRing:
                case iMagicRing:
                case iL2MagicRing:
                case iL3MagicRing:
                case iL4MagicRing:
                    reset_itembuf(&tempitem, i);
                    strcpy(item_string[i],old_item_string[i]);
                    break;
                }
            }
            
            if(s_version < 6)  // April 2007: Advanced item editing capabilities.
            {
                if(i!=iBPotion && i!=iRPotion)
                    tempitem.flags |= get_bit(deprecated_rules,32) ? ITEM_KEEPOLD : 0;
                    
                switch(i)
                {
                case iTriforce:
                    tempitem.fam_type=1;
                    break;
                    
                case iBigTri:
                    tempitem.fam_type=0;
                    break;
                    
                case iBombs:
                    tempitem.fam_type=i_bomb;
                    tempitem.power=4;
                    tempitem.wpn=wBOMB;
                    tempitem.wpn2=wBOOM;
                    tempitem.misc1 = 50;
                    
                    if(get_bit(deprecated_rules,116)) tempitem.misc1 = 200;  //qr_SLOWBOMBFUSES
                    
                    break;
                    
                case iSBomb:
                    tempitem.fam_type=i_sbomb;
                    tempitem.power=16;
                    tempitem.wpn=wSBOMB;
                    tempitem.wpn2=wSBOOM;
                    tempitem.misc1 = 50;
                    
                    if(get_bit(deprecated_rules,116)) tempitem.misc1 = 400;  //qr_SLOWBOMBFUSES
                    
                    break;
                    
                case iBook:
                    if(get_bit(deprecated_rules, 113))
                        tempitem.wpn = wFIREMAGIC; //qr_FIREMAGICSPRITE
                        
                    break;
                    
                case iSArrow:
                    tempitem.wpn2 = get_bit(deprecated_rules,27) ? wSSPARKLE : 0; //qr_SASPARKLES
                    tempitem.power=4;
                    tempitem.flags|=ITEM_GAMEDATA;
                    tempitem.wpn=wSARROW;
                    break;
                    
                case iGArrow:
                    tempitem.wpn2 = get_bit(deprecated_rules,28) ? wGSPARKLE : 0; //qr_GASPARKLES
                    tempitem.power=8;
                    tempitem.flags|=(ITEM_GAMEDATA|ITEM_FLAG1);
                    tempitem.wpn=wGARROW;
                    break;
                    
                case iBrang:
                    tempitem.power=0;
                    tempitem.wpn=wBRANG;
                    tempitem.misc1=36;
                    break;
                    
                case iMBrang:
                    tempitem.wpn2 = get_bit(deprecated_rules,29) ? wMSPARKLE : 0; //qr_MBSPARKLES
                    tempitem.power=0;
                    tempitem.wpn=wMBRANG;
                    break;
                    
                case iFBrang:
                    tempitem.wpn3 = get_bit(deprecated_rules,30) ? wFSPARKLE : 0; //qr_FBSPARKLES
                    tempitem.power=2;
                    tempitem.wpn=wFBRANG;
                    break;
                    
                case iBoots:
                    tempitem.magic = get_bit(deprecated_rules,51) ? 1 : 0;
                    tempitem.power=7;
                    break;
                    
                case iWand:
                    tempitem.magic = get_bit(deprecated_rules,49) ? 8 : 0;
                    tempitem.power=2;
                    tempitem.wpn=wWAND;
                    tempitem.wpn3=wMAGIC;
                    break;
                    
                case iBCandle:
                    tempitem.magic = get_bit(deprecated_rules,50) ? 4 : 0;
                    tempitem.power=1;
                    tempitem.flags|=(ITEM_GAMEDATA|ITEM_FLAG1);
                    tempitem.wpn3=wFIRE;
                    break;
                    
                case iRCandle:
                    tempitem.magic = get_bit(deprecated_rules,50) ? 4 : 0;
                    tempitem.power=1;
                    tempitem.wpn3=wFIRE;
                    break;
                    
                case iSword:
                    tempitem.power=1;
                    tempitem.flags|= ITEM_FLAG4 |ITEM_FLAG2;
                    tempitem.wpn=tempitem.wpn3=wSWORD;
                    tempitem.wpn2=wSWORDSLASH;
                    break;
                    
                case iWSword:
                    tempitem.power=2;
                    tempitem.flags|= ITEM_FLAG4 |ITEM_FLAG2;
                    tempitem.wpn=tempitem.wpn3=wWSWORD;
                    tempitem.wpn2=wWSWORDSLASH;
                    break;
                    
                case iMSword:
                    tempitem.power=4;
                    tempitem.flags|= ITEM_FLAG4 |ITEM_FLAG2;
                    tempitem.wpn=tempitem.wpn3=wMSWORD;
                    tempitem.wpn2=wMSWORDSLASH;
                    break;
                    
                case iXSword:
                    tempitem.power=8;
                    tempitem.flags|= ITEM_FLAG4 |ITEM_FLAG2;
                    tempitem.wpn=tempitem.wpn3=wXSWORD;
                    tempitem.wpn2=wXSWORDSLASH;
                    break;
                    
                case iNayrusLove:
                    tempitem.flags |= get_bit(deprecated_rules,76) ? ITEM_FLAG1 : 0;
                    tempitem.flags |= get_bit(deprecated_rules,75) ? ITEM_FLAG2 : 0;
                    tempitem.wpn=wNAYRUSLOVE1A;
                    tempitem.wpn2=wNAYRUSLOVE1B;
                    tempitem.wpn3=wNAYRUSLOVES1A;
                    tempitem.wpn4=wNAYRUSLOVES1B;
                    tempitem.wpn6=wNAYRUSLOVE2A;
                    tempitem.wpn7=wNAYRUSLOVE2B;
                    tempitem.wpn8=wNAYRUSLOVES2A;
                    tempitem.wpn9=wNAYRUSLOVES2B;
                    tempitem.wpn5 = iwNayrusLoveShieldFront;
                    tempitem.wpn10 = iwNayrusLoveShieldBack;
                    tempitem.misc1=512;
                    tempitem.magic=64;
                    break;
                    
                case iLens:
                    tempitem.misc1=60;
                    tempitem.flags |= get_bit(quest_rules,qr_ENABLEMAGIC) ? 0 : ITEM_RUPEE_MAGIC;
                    tempitem.magic = get_bit(quest_rules,qr_ENABLEMAGIC) ? 2 : 1;
                    break;
                    
                case iArrow:
                    tempitem.power=2;
                    tempitem.wpn=wARROW;
                    break;
                    
                case iHoverBoots:
                    tempitem.misc1=45;
                    tempitem.wpn=iwHover;
                    break;
                    
                case iDinsFire:
                    tempitem.power=8;
                    tempitem.wpn=wDINSFIRE1A;
                    tempitem.wpn2=wDINSFIRE1B;
                    tempitem.wpn3=wDINSFIRES1A;
                    tempitem.wpn4=wDINSFIRES1B;
                    tempitem.misc1 = 32;
                    tempitem.misc2 = 200;
                    tempitem.magic=32;
                    break;
                    
                case iFaroresWind:
                    tempitem.magic=32;
                    break;
                    
                case iHookshot:
                    tempitem.power=0;
                    tempitem.flags&=~ITEM_FLAG1;
                    tempitem.wpn=wHSHEAD;
                    tempitem.wpn2=wHSCHAIN_H;
                    tempitem.wpn4=wHSHANDLE;
                    tempitem.wpn3=wHSCHAIN_V;
                    tempitem.misc1=50;
                    tempitem.misc2=100;
                    break;
                    
                case iLongshot:
                    tempitem.power=0;
                    tempitem.flags&=~ITEM_FLAG1;
                    tempitem.wpn=wLSHEAD;
                    tempitem.wpn2=wLSCHAIN_H;
                    tempitem.wpn4=wLSHANDLE;
                    tempitem.wpn3=wLSCHAIN_V;
                    tempitem.misc1=99;
                    tempitem.misc2=100;
                    break;
                    
                case iHammer:
                    tempitem.power=4;
                    tempitem.wpn=wHAMMER;
                    tempitem.wpn2=iwHammerSmack;
                    break;
                    
                case iCByrna:
                    tempitem.power=1;
                    tempitem.wpn=wCBYRNA;
                    tempitem.wpn2=wCBYRNASLASH;
                    tempitem.wpn3=wCBYRNAORB;
                    tempitem.misc1=4;
                    tempitem.misc2=16;
                    tempitem.misc3=1;
                    tempitem.magic=1;
                    break;
                    
                case iWhistle:
                    tempitem.wpn=wWIND;
                    tempitem.misc1=3;
                    tempitem.flags|=ITEM_FLAG1;
                    break;
                    
                case iBRing:
                    tempitem.power=2;
                    tempitem.misc1=spBLUE;
                    break;
                    
                case iRRing:
                    tempitem.power=4;
                    tempitem.misc1=spRED;
                    break;
                    
                case iGRing:
                    tempitem.power=8;
                    tempitem.misc1=spGOLD;
                    break;
                    
                case iSpinScroll:
                    tempitem.power = 2;
                    tempitem.misc1 = 1;
                    break;
                    
                case iL2SpinScroll:
                    tempitem.family=itype_spinscroll2;
                    tempitem.fam_type=1;
                    tempitem.magic=8;
                    tempitem.power=2;
                    tempitem.misc1 = 20;
                    break;
                    
                case iQuakeScroll:
                    tempitem.misc1=0x10;
                    tempitem.misc2=64;
                    break;
                    
                case iL2QuakeScroll:
                    tempitem.family=itype_quakescroll2;
                    tempitem.fam_type=1;
                    tempitem.power = 2;
                    tempitem.misc1=0x20;
                    tempitem.misc2=192;
                    tempitem.magic=8;
                    break;
                    
                case iChargeRing:
                    tempitem.misc1=64;
                    tempitem.misc2=128;
                    break;
                    
                case iL2ChargeRing:
                    tempitem.misc1=32;
                    tempitem.misc2=64;
                    break;
                    
                case iOldGlove:
                    tempitem.flags |= ITEM_FLAG1;
                    
                    //fallthrough
                case iBombBagL4:
                case iWalletL3:
                case iQuiverL4:
                case iBracelet:
                    tempitem.power = 1;
                    break;
                    
                case iL2Bracelet:
                    tempitem.power = 2;
                    break;
                    
                case iMKey:
                    tempitem.power=0xFF;
                    tempitem.flags |= ITEM_FLAG1;
                    break;
                }
            }
            
            if(s_version < 7)
            {
                switch(i)
                {
                case iStoneAgony:
                case iStompBoots:
                case iPerilRing:
                case iWhimsicalRing:
                {
                    reset_itembuf(&tempitem, i);
                    strcpy(item_string[i],old_item_string[i]);
                    break;
                }
                }
            }
            
            if(s_version < 8) // May 2007: Some corrections.
            {
                switch(i)
                {
                case iMShield:
                    tempitem.misc1|=shFLAME;
                    tempitem.misc2|=shFIREBALL|shMAGIC;
                    
                    if(get_bit(quest_rules, qr_SWORDMIRROR))
                    {
                        tempitem.misc2 |= shSWORD;
                    }
                    
                    // fallthrough
                case iShield:
                    tempitem.misc1|=shFIREBALL|shSWORD|shMAGIC;
                    
                    // fallthrough
                case iSShield:
                    tempitem.misc1|=shROCK|shARROW|shBRANG|shSCRIPT;
                    
                    if(get_bit(deprecated_rules,102))  //qr_REFLECTROCKS
                    {
                        tempitem.misc2 |= shROCK;
                    }
                    
                    break;
                    
                case iWhispRing:
                    tempitem.power=1;
                    tempitem.flags|=ITEM_GAMEDATA|ITEM_FLAG1;
                    tempitem.misc1 = 3;
                    break;
                    
                case iL2WhispRing:
                    tempitem.power=0;
                    tempitem.flags|=ITEM_GAMEDATA|ITEM_FLAG1;
                    tempitem.misc1 = 3;
                    break;
                    
                case iL2Ladder:
                case iBow:
                case iCByrna:
                    tempitem.power = 1;
                    break;
                    break;
                }
            }
            
            if(s_version < 9 && i==iClock)
            {
                tempitem.misc1 = get_bit(deprecated_rules, qr_TEMPCLOCKS_DEP) ? 256 : 0;
            }
            
            //add the misc flag for bomb
            if(s_version < 10 && tempitem.family == itype_bomb)
            {
                tempitem.flags = (tempitem.flags & ~ITEM_FLAG1) | (get_bit(quest_rules, qr_LONGBOMBBOOM_DEP) ? ITEM_FLAG1 : 0);
            }
            
            if(s_version < 11 && tempitem.family == itype_triforcepiece)
            {
                tempitem.flags = (tempitem.fam_type ? ITEM_GAMEDATA : 0);
                tempitem.playsound = (tempitem.fam_type ? WAV_SCALE : WAV_CLEARED);
            }
            
            if(s_version < 12) // June 2007: More Misc. attributes.
            {
                switch(i)
                {
                case iFBrang:
                    tempitem.misc4 |= shFIREBALL|shSWORD|shMAGIC;
                    
                    //fallthrough
                case iMBrang:
                    tempitem.misc3 |= shSWORD|shMAGIC;
                    
                    //fallthrough
                case iHookshot:
                case iLongshot:
                    //fallthrough
                    tempitem.misc3 |= shFIREBALL;
                    
                case iBrang:
                    tempitem.misc3 |= shBRANG|shROCK|shARROW;
                    break;
                }
                
                switch(tempitem.family)
                {
                case itype_hoverboots:
                    tempitem.usesound = WAV_ZN1HOVER;
                    break;
                    
                case itype_wand:
                case itype_book:
                    tempitem.usesound = WAV_WAND;
                    break;
                    
                case itype_arrow:
                    tempitem.usesound = WAV_ARROW;
                    break;
                    
                case itype_hookshot:
                    tempitem.usesound = WAV_HOOKSHOT;
                    break;
                    
                case itype_brang:
                    tempitem.usesound = WAV_BRANG;
                    break;
                    
                case itype_shield:
                    tempitem.usesound = WAV_CHINK;
                    break;
                    
                case itype_sword:
                    tempitem.usesound = WAV_SWORD;
                    break;
                    
                case itype_whistle:
                    tempitem.usesound = WAV_WHISTLE;
                    break;
                    
                case itype_hammer:
                    tempitem.usesound = WAV_HAMMER;
                    break;
                    
                case itype_dinsfire:
                    tempitem.usesound = WAV_ZN1DINSFIRE;
                    break;
                    
                case itype_faroreswind:
                    tempitem.usesound = WAV_ZN1FARORESWIND;
                    break;
                    
                case itype_nayruslove:
                    tempitem.usesound = WAV_ZN1NAYRUSLOVE1;
                    break;
                    
                case itype_bomb:
                case itype_sbomb:
                case itype_quakescroll:
                case itype_quakescroll2:
                    tempitem.usesound = WAV_BOMB;
                    break;
                    
                case itype_spinscroll:
                case itype_spinscroll2:
                    tempitem.usesound = WAV_ZN1SPINATTACK;
                    break;
                }
            }
            
            if(s_version < 13) // July 2007
            {
                if(tempitem.family == itype_whistle)
                {
                    tempitem.misc1 = (tempitem.power==2 ? 4 : 3);
                    tempitem.power = 1;
                    tempitem.flags|=ITEM_FLAG1;
                }
                else if(tempitem.family == itype_wand)
                    tempitem.flags|=ITEM_FLAG1;
                else if(tempitem.family == itype_book)
                {
                    tempitem.flags|=ITEM_FLAG1;
                    tempitem.power = 2;
                }
            }
            
            if(s_version < 14) // August 2007
            {
                if(tempitem.family == itype_fairy)
                {
                    tempitem.usesound = WAV_SCALE;
                    
                    if(tempitem.fam_type)
                        tempitem.misc3=50;
                }
                else if(tempitem.family == itype_potion)
                {
                    tempitem.flags |= ITEM_GAINOLD;
                }
            }
            
            if(s_version < 17) // November 2007
            {
                if(tempitem.family == itype_candle && !tempitem.wpn3)
                {
                    tempitem.wpn3 = wFIRE;
                }
                else if(tempitem.family == itype_arrow && tempitem.power>4)
                {
                    tempitem.flags|=ITEM_FLAG1;
                }
            }
            
            if(s_version < 18) // New Year's Eve 2007
            {
                if(tempitem.family == itype_whistle)
                    tempitem.misc2 = 8; // Use the Whistle warp ring
                else if(tempitem.family == itype_bait)
                    tempitem.misc1 = 768; // Frames until it goes
                else if(tempitem.family == itype_triforcepiece)
                {
                    if(tempitem.flags & ITEM_GAMEDATA)
                    {
                        tempitem.misc2 = 1; // Cutscene 1
                        tempitem.flags |= ITEM_FLAG1; // Side Warp Out
                    }
                }
            }
            
            if(s_version < 19)  // January 2008
            {
                if(tempitem.family == itype_nayruslove)
                {
                    tempitem.flags |= get_bit(deprecated_rules,qr_NOBOMBPALFLASH+1)?ITEM_FLAG3:0;
                    tempitem.flags |= get_bit(deprecated_rules,qr_NOBOMBPALFLASH+2)?ITEM_FLAG4:0;
                }
            }
            
            if(s_version < 20)  // October 2008
            {
                if(tempitem.family == itype_nayruslove)
                {
                    tempitem.wpn6=wNAYRUSLOVE2A;
                    tempitem.wpn7=wNAYRUSLOVE2B;
                    tempitem.wpn8=wNAYRUSLOVES2A;
                    tempitem.wpn9=wNAYRUSLOVES2B;
                    tempitem.wpn5 = iwNayrusLoveShieldFront;
                    tempitem.wpn10 = iwNayrusLoveShieldBack;
                }
            }
            
            if(s_version < 21)  // November 2008
            {
                if(tempitem.flags & 0x0100)  // ITEM_SLASH
                {
                    tempitem.flags &= ~0x0100;
                    
                    if(tempitem.family == itype_sword ||
                            tempitem.family == itype_wand ||
                            tempitem.family == itype_candle ||
                            tempitem.family == itype_cbyrna)
                    {
                        tempitem.flags |= ITEM_FLAG4;
                    }
                }
            }
            
            if(s_version < 22)  // September 2009
            {
                if(tempitem.family == itype_sbomb || tempitem.family == itype_bomb)
                {
                    tempitem.misc3 = tempitem.power/2;
                }
            }
            
            if(s_version < 23)    // March 2011
            {
                if(tempitem.family == itype_dinsfire)
                    tempitem.wpn5 = wFIRE;
                else if(tempitem.family == itype_book)
                    tempitem.wpn2 = wFIRE;
            }
            
            // Version 25: Bomb bags were acting as though "super bombs also" was checked
            // whether it was or not, and a lot of existing quests depended on the
            // incorrect behavior.
            if(s_version < 25)    // January 2012
            {
                if(tempitem.family == itype_bombbag)
                    tempitem.flags |= 16;
                    
                if(tempitem.family == itype_dinsfire)
                    tempitem.flags |= ITEM_FLAG3; // Sideview gravity flag
            }
            
	    //Port quest rules to items
	    if( s_version <= 31) 
	    {
		if(tempitem.family == itype_candle)
		{
			if ( (!get_bit(quest_rules,qr_FIREPROOFLINK)) ) tempitem.flags |= ITEM_FLAG3;
			else tempitem.flags &= ~ ITEM_FLAG3;
			if ( (!get_bit(quest_rules,qr_TEMPCANDLELIGHT)) ) tempitem.flags |= ITEM_FLAG5;
			else tempitem.flags &= ~ ITEM_FLAG5;
			
		}
		else if(tempitem.family == itype_bomb)
		{
			if ( get_bit(quest_rules,qr_OUCHBOMBS) )  tempitem.flags |= ITEM_FLAG2;
			else tempitem.flags &= ~ ITEM_FLAG2;
		}
		else if(tempitem.family == itype_sbomb)
		{
			if ( get_bit(quest_rules,qr_OUCHBOMBS) )  tempitem.flags |= ITEM_FLAG2;
			else tempitem.flags &= ~ ITEM_FLAG2;
		}
		
		else if(tempitem.family == itype_brang)
		{
			if ( get_bit(quest_rules,qr_BRANGPICKUP) )  tempitem.flags |= ITEM_FLAG4;
			else tempitem.flags &= ~ ITEM_FLAG4;
		}	
		else if(tempitem.family == itype_wand)
		{
			if ( get_bit(quest_rules,qr_NOWANDMELEE) )  tempitem.flags |= ITEM_FLAG3;
			else tempitem.flags &= ~ ITEM_FLAG3;
		}
		
	    }
	    
	    //Port quest rules to items
	    if( s_version <= 37) 
	    {
		if(tempitem.family == itype_flippers)
		{
			if ( (get_bit(quest_rules,qr_NODIVING)) ) tempitem.flags |= ITEM_FLAG1;
			else tempitem.flags &= ~ ITEM_FLAG1;
		}
		else if(tempitem.family == itype_sword)
		{
			if ( (get_bit(quest_rules,qr_QUICKSWORD)) ) tempitem.flags |= ITEM_FLAG5;
			else tempitem.flags &= ~ ITEM_FLAG5;
		}
		else if(tempitem.family == itype_wand)
		{
			if ( (get_bit(quest_rules,qr_QUICKSWORD)) ) tempitem.flags |= ITEM_FLAG5;
			else tempitem.flags &= ~ ITEM_FLAG5;
		}
		else if(tempitem.family == itype_book)
		{
			//@VenRob: What was qrFIREPROOFLINK2 again, and does that also need to enable this?
			if ( (get_bit(quest_rules,qr_FIREPROOFLINK)) ) tempitem.flags |= ITEM_FLAG3;
			else tempitem.flags &= ~ ITEM_FLAG3;
		}
	    }
	    
	    if( s_version < 38){
		if(tempitem.family == itype_brang || tempitem.family == itype_hookshot){
			if(get_bit(quest_rules,qr_BRANGPICKUP)) tempitem.flags |= ITEM_FLAG4;
			else tempitem.flags &= ~ITEM_FLAG4;
			
			if(get_bit(quest_rules,qr_Z3BRANG_HSHOT)) tempitem.flags |= ITEM_FLAG5 | ITEM_FLAG6;
			else tempitem.flags &= ~(ITEM_FLAG5|ITEM_FLAG6);
		} else if(tempitem.family == itype_arrow){
			if(get_bit(quest_rules,qr_BRANGPICKUP)) tempitem.flags |= ITEM_FLAG4;
			else tempitem.flags &= ~ITEM_FLAG4;
			
			if(get_bit(quest_rules,qr_Z3BRANG_HSHOT)) tempitem.flags &= ~ITEM_FLAG2;
			else tempitem.flags |= ITEM_FLAG2;
		}
	    }
	    
            if(tempitem.fam_type==0)  // Always do this
                tempitem.fam_type=1;
                
            memcpy(&itemsbuf[i], &tempitem, sizeof(itemdata));
            
        }
    }
    
    return 0;
}


void reset_itembuf(itemdata *item, int id)
{
    if(id<iLast)
    {
        // Copy everything *EXCEPT* the tile, misc, cset, frames, speed, delay and ltm.
        word tile = item->tile;
        byte miscs = item->misc, cset = item->csets, frames = item->frames, speed = item->speed, delay = item->delay;
        long ltm = item->ltm;
        
        memcpy(item,&default_items[id],sizeof(itemdata));
        item->tile = tile;
        item->misc = miscs;
        item->csets = cset;
        item->frames = frames;
        item->speed = speed;
        item->delay = delay;
        item->ltm = ltm;
    }
}

void reset_itemname(int id)
{
    sprintf(item_string[id],"zz%03d",id);
    
    if(id < iLast)
        strcpy(item_string[id],old_item_string[id]);
}

int readweapons(PACKFILE *f, zquestheader *Header, bool keepdata)
{
    word weapons_to_read=MAXWPNS;
    int dummy;
    byte padding;
    wpndata tempweapon;
    word s_version=0, s_cversion=0;
    
    
    if(Header->zelda_version < 0x186)
    {
        weapons_to_read=64;
    }
    
    if(Header->zelda_version < 0x185)
    {
        weapons_to_read=32;
    }
    
    if(Header->zelda_version > 0x192)
    {
        weapons_to_read=0;
        
        //section version info
        if(!p_igetw(&s_version,f,true))
        {
            return qe_invalid;
        }
	
	ffcore.quest_format[vWeaponSprites] = s_version;
        
        //al_trace("Weapons version %d\n", s_version);
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
        if(!p_igetw(&weapons_to_read,f,true))
        {
            return qe_invalid;
        }
    }
    
    if(s_version>2)
    {
        for(int i=0; i<weapons_to_read; i++)
        {
            char tempname[64];
            
            if(!pfread(tempname, 64, f, keepdata))
            {
                return qe_invalid;
            }
            
            if(keepdata)
            {
                strcpy(weapon_string[i], tempname);
            }
        }
        
        if(s_version<4)
        {
            if(keepdata)
            {
                strcpy(weapon_string[iwHover],old_weapon_string[iwHover]);
                strcpy(weapon_string[wFIREMAGIC],old_weapon_string[wFIREMAGIC]);
            }
        }
        
        if(s_version<5)
        {
            if(keepdata)
            {
                strcpy(weapon_string[iwQuarterHearts],old_weapon_string[iwQuarterHearts]);
            }
        }
        
        /*
            if (s_version<6)
            {
              strcpy(weapon_string[iwSideRaft],old_weapon_string[iwSideRaft]);
              strcpy(weapon_string[iwSideLadder],old_weapon_string[iwSideLadder]);
            }
        */
    }
    else
    {
        if(keepdata)
            for(int i=0; i<WPNCNT; i++)
                reset_weaponname(i);
    }
    
    for(int i=0; i<weapons_to_read; i++)
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
	
	if ( s_version >= 7 )
	{
		if(!p_igetw(&tempweapon.script,f,true))
		{
		    return qe_invalid;
		}
		if(!p_igetl(&tempweapon.newtile,f,true))
		{
		    return qe_invalid;
		}	    
	}
	if ( s_version < 7 ) 
	{
		tempweapon.newtile = tempweapon.tile;
	}
        
        if(Header->zelda_version < 0x193)
        {
            if(!p_getc(&padding,f,true))
            {
                return qe_invalid;
            }
        }
        
        if(s_version < 6)
        {
            if(i==ewFIRETRAIL)
            {
                tempweapon.misc |= WF_BEHIND;
            }
            else
                tempweapon.misc &= ~WF_BEHIND;
        }
        
        if(keepdata==true)
        {
            memcpy(&wpnsbuf[i], &tempweapon, sizeof(tempweapon));
        }
    }
    
    if(keepdata==true)
    {
        if(s_version<2)
        {
            wpnsbuf[wSBOOM]=wpnsbuf[wBOOM];
        }
        
        if(s_version<5)
        {
            wpnsbuf[iwQuarterHearts].tile=1;
            wpnsbuf[iwQuarterHearts].csets=1;
        }
        
        if(Header->zelda_version < 0x176)
        {
            wpnsbuf[iwSpawn] = *((wpndata*)(itemsbuf + iMisc1));
            wpnsbuf[iwDeath] = *((wpndata*)(itemsbuf + iMisc2));
            memset(&itemsbuf[iMisc1],0,sizeof(itemdata));
            memset(&itemsbuf[iMisc2],0,sizeof(itemdata));
        }
        
        if((Header->zelda_version < 0x192)||
                ((Header->zelda_version == 0x192)&&(Header->build<129)))
        {
            wpnsbuf[wHSCHAIN_V] = wpnsbuf[wHSCHAIN_H];
        }
        
        if((Header->zelda_version < 0x210))
        {
            wpnsbuf[wLSHEAD] = wpnsbuf[wHSHEAD];
            wpnsbuf[wLSCHAIN_H] = wpnsbuf[wHSCHAIN_H];
            wpnsbuf[wLSHANDLE] = wpnsbuf[wHSHANDLE];
            wpnsbuf[wLSCHAIN_V] = wpnsbuf[wHSCHAIN_V];
        }
    }
    
    return 0;
}

void init_guys(int guyversion)
{
    for(int i=0; i<MAXGUYS; i++)
    {
        guysbuf[i] = default_guys[0];
    }
    
    for(int i=0; i<OLDMAXGUYS; i++)
    {
        guysbuf[i] = default_guys[i];
        
        // Patra fix: 2.10 BSPatras used spDIG. 2.50 Patras use CSet 7.
        if(guyversion<=3 && i==ePATRABS)
        {
            guysbuf[i].bosspal=spDIG;
            guysbuf[i].cset=14;
            guysbuf[i].misc9=14;
        }
        
        if(guyversion<=3)
        {
            // Rope/Ghini Flash rules
            if(get_bit(deprecated_rules, qr_NOROPE2FLASH_DEP))
            {
                if(i==eROPE2)
                {
                    guysbuf[i].flags2 &= ~guy_flashing;
                }
            }
            
            if(get_bit(deprecated_rules, qr_NOBUBBLEFLASH_DEP))
            {
                if(i==eBUBBLEST || i==eBUBBLESP || i==eBUBBLESR || i==eBUBBLEIT || i==eBUBBLEIP || i==eBUBBLEIR)
                {
                    guysbuf[i].flags2 &= ~guy_flashing;
                }
            }
            
            if(i==eGHINI2)
            {
                if(get_bit(deprecated_rules, qr_GHINI2BLINK_DEP))
                {
                    guysbuf[i].flags2 |= guy_blinking;
                }
                
                if(get_bit(deprecated_rules, qr_PHANTOMGHINI2_DEP))
                {
                    guysbuf[i].flags2 |= guy_transparent;
                }
            }
        }
        
        // Darknut fix
        if(i==eDKNUT1 || i==eDKNUT2 || i==eDKNUT3 || i==eDKNUT4 || i==eDKNUT5)
        {
            if(get_bit(quest_rules,qr_NEWENEMYTILES))
            {
                guysbuf[i].s_tile=guysbuf[i].e_tile+120;
                guysbuf[i].s_width=guysbuf[i].e_width;
                guysbuf[i].s_height=guysbuf[i].e_height;
            }
            else guysbuf[i].s_tile=860;
            
            if(get_bit(deprecated_rules,qr_BRKBLSHLDS_DEP))
            {
                guysbuf[i].flags |= guy_bkshield;
            }
        }
        
        if((i==eGELTRIB || i==eFGELTRIB) && get_bit(deprecated_rules,qr_OLDTRIBBLES_DEP))
        {
            guysbuf[i].misc3 = (i==eFGELTRIB ? eFZOL : eZOL);
        }
    }
}

void reset_weaponname(int i)
{
    if(i<wLast)
    {
        strcpy(weapon_string[i],old_weapon_string[i]);
    }
    else
        sprintf(weapon_string[i],"zz%03d",i);
}

void init_item_drop_sets()
{
    for(int i=0; i<MAXITEMDROPSETS; i++)
    {
//    item_drop_sets[i] = default_item_drop_sets[0];
        memset(&item_drop_sets[i], 0, sizeof(item_drop_object));
    }
    
    for(int i=0; i<isMAX; i++)
    {
        item_drop_sets[i] = default_item_drop_sets[i];
        
        // Deprecated: qr_NOCLOCKS and qr_ALLOW10RUPEEDROPS
        for(int j=0; j<10; ++j)
        {
            int it = item_drop_sets[i].item[j];
            
            if((itemsbuf[it].family == itype_rupee && ((itemsbuf[it].amount)&0xFFF) == 10)
                    && !get_bit(deprecated_rules, qr_ALLOW10RUPEEDROPS_DEP))
            {
                item_drop_sets[i].chance[j+1]=0;
            }
            else if(itemsbuf[it].family == itype_clock && get_bit(deprecated_rules, qr_NOCLOCKS_DEP))
            {
                item_drop_sets[i].chance[j+1]=0;
            }
            
            // From Sept 2007 to Dec 2008, non-gameplay items were prohibited.
            if(itemsbuf[it].family == itype_misc)
            {
                // If a non-gameplay item was selected, then item drop was aborted.
                // Reflect this by increasing the 'Nothing' chance accordingly.
                item_drop_sets[i].chance[0]+=item_drop_sets[i].chance[j+1];
                item_drop_sets[i].chance[j+1]=0;
            }
        }
    }
}

void init_favorites()
{
    for(int i=0; i<MAXFAVORITECOMBOS; i++)
    {
        favorite_combos[i]=-1;
    }
    
    for(int i=0; i<MAXFAVORITECOMBOALIASES; i++)
    {
        favorite_comboaliases[i]=-1;
    }
}

const char *ctype_name[cMAX]=
{
    "cNONE", "cSTAIR", "cCAVE", "cWATER", "cARMOS", "cGRAVE", "cDOCK",
    "cUNDEF", "cPUSH_WAIT", "cPUSH_HEAVY", "cPUSH_HW", "cL_STATUE", "cR_STATUE",
    "cWALKSLOW", "cCVUP", "cCVDOWN", "cCVLEFT", "cCVRIGHT", "cSWIMWARP", "cDIVEWARP",
    "cLADDERHOOKSHOT", "cTRIGNOFLAG", "cTRIGFLAG", "cZELDA", "cSLASH", "cSLASHITEM",
    "cPUSH_HEAVY2", "cPUSH_HW2", "cPOUND", "cHSGRAB", "cHSBRIDGE", "cDAMAGE1",
    "cDAMAGE2", "cDAMAGE3", "cDAMAGE4", "cC_STATUE", "cTRAP_H", "cTRAP_V", "cTRAP_4",
    "cTRAP_LR", "cTRAP_UD", "cPIT", "cHOOKSHOTONLY", "cOVERHEAD", "cNOFLYZONE", "cMIRROR",
    "cMIRRORSLASH", "cMIRRORBACKSLASH", "cMAGICPRISM", "cMAGICPRISM4",
    "cMAGICSPONGE", "cCAVE2", "cEYEBALL_A", "cEYEBALL_B", "cNOJUMPZONE", "cBUSH",
    "cFLOWERS", "cTALLGRASS", "cSHALLOWWATER", "cLOCKBLOCK", "cLOCKBLOCK2",
    "cBOSSLOCKBLOCK", "cBOSSLOCKBLOCK2", "cLADDERONLY", "cBSGRAVE",
    "cCHEST", "cCHEST2", "cLOCKEDCHEST", "cLOCKEDCHEST2", "cBOSSCHEST", "cBOSSCHEST2",
    "cRESET", "cSAVE", "cSAVE2", "cCAVEB", "cCAVEC", "cCAVED",
    "cSTAIRB", "cSTAIRC", "cSTAIRD", "cPITB", "cPITC", "cPITD",
    "cCAVE2B", "cCAVE2C", "cCAVE2D", "cSWIMWARPB", "cSWIMWARPC", "cSWIMWARPD",
    "cDIVEWARPB", "cDIVEWARPC", "cDIVEWARPD", "cSTAIRR", "cPITR",
    "cAWARPA", "cAWARPB", "cAWARPC", "cAWARPD", "cAWARPR",
    "cSWARPA", "cSWARPB", "cSWARPC", "cSWARPD", "cSWARPR", "cSTRIGNOFLAG", "cSTRIGFLAG",
    "cSTEP", "cSTEPSAME", "cSTEPALL", "cSTEPCOPY", "cNOENEMY", "cBLOCKARROW1", "cBLOCKARROW2",
    "cBLOCKARROW3", "cBLOCKBRANG1", "cBLOCKBRANG2", "cBLOCKBRANG3", "cBLOCKSBEAM", "cBLOCKALL",
    "cBLOCKFIREBALL", "cDAMAGE5", "cDAMAGE6", "cDAMAGE7", "cCHANGE", "cSPINTILE1", "cSPINTILE2",
    "cSCREENFREEZE", "cSCREENFREEZEFF", "cNOGROUNDENEMY", "cSLASHNEXT", "cSLASHNEXTITEM", "cBUSHNEXT"
    "cSLASHTOUCHY", "cSLASHITEMTOUCHY", "cBUSHTOUCHY", "cFLOWERSTOUCHY", "cTALLGRASSTOUCHY",
    "cSLASHNEXTTOUCHY", "cSLASHNEXTITEMTOUCHY", "cBUSHNEXTTOUCHY", "cEYEBALL_4", "cTALLGRASSNEXT"
};

int init_combo_classes()
{
    for(int i=0; i<cMAX; i++)
    {
        combo_class_buf[i] = default_combo_classes[i];
        continue;
        /*
            al_trace("===== %03d (%s)=====\n", i, ctype_name[i]);
            al_trace("name:  %s\n", combo_class_buf[i].name);
            al_trace("block_enemies:  %d\n", combo_class_buf[i].block_enemies);
            al_trace("block_hole:  %d\n", combo_class_buf[i].block_hole);
            al_trace("block_trigger:  %d\n", combo_class_buf[i].block_trigger);
            for(int j=0; j<32; j++)
            {
              al_trace("block_weapon[%d]:  %d\n", j, combo_class_buf[i].block_weapon[j]);
            }
            al_trace("conveyor_direction:  %d\n", combo_class_buf[i].conveyor_direction);
            al_trace("create_enemy:  %d\n", combo_class_buf[i].create_enemy);
            al_trace("create_enemy_when:  %d\n", combo_class_buf[i].create_enemy_when);
            al_trace("create_enemy_change:  %ld\n", combo_class_buf[i].create_enemy_change);
            al_trace("directional_change_type:  %d\n", combo_class_buf[i].directional_change_type);
            al_trace("distance_change_tiles:  %ld\n", combo_class_buf[i].distance_change_tiles);
            al_trace("dive_item:  %d\n", combo_class_buf[i].dive_item);
            al_trace("dock:  %d\n", combo_class_buf[i].dock);
            al_trace("fairy:  %d\n", combo_class_buf[i].fairy);
            al_trace("ff_combo_attr_change:  %d\n", combo_class_buf[i].ff_combo_attr_change);
            al_trace("foot_decorations_tile:  %ld\n", combo_class_buf[i].foot_decorations_tile);
            al_trace("foot_decorations_type:  %d\n", combo_class_buf[i].foot_decorations_type);
            al_trace("hookshot_grab_point:  %d\n", combo_class_buf[i].hookshot_grab_point);
            al_trace("ladder_pass:  %d\n", combo_class_buf[i].ladder_pass);
            al_trace("lock_block_type:  %d\n", combo_class_buf[i].lock_block_type);
            al_trace("lock_block_change:  %ld\n", combo_class_buf[i].lock_block_change);
            al_trace("magic_mirror_type:  %d\n", combo_class_buf[i].magic_mirror_type);
            al_trace("modify_hp_amount:  %d\n", combo_class_buf[i].modify_hp_amount);
            al_trace("modify_hp_delay:  %d\n", combo_class_buf[i].modify_hp_delay);
            al_trace("modify_hp_type:  %d\n", combo_class_buf[i].modify_hp_type);
            al_trace("modify_mp_amount:  %d\n", combo_class_buf[i].modify_mp_amount);
            al_trace("modify_mp_delay:  %d\n", combo_class_buf[i].modify_mp_delay);
            al_trace("modify_mp_type:  %d\n", combo_class_buf[i].modify_mp_type);
            al_trace("no_push_blocks:  %d\n", combo_class_buf[i].no_push_blocks);
            al_trace("overhead:  %d\n", combo_class_buf[i].overhead);
            al_trace("place_enemy:  %d\n", combo_class_buf[i].place_enemy);
            al_trace("push_direction:  %d\n", combo_class_buf[i].push_direction);
            al_trace("push_weight:  %d\n", combo_class_buf[i].push_weight);
            al_trace("push_wait:  %d\n", combo_class_buf[i].push_wait);
            al_trace("pushed:  %d\n", combo_class_buf[i].pushed);
            al_trace("raft:  %d\n", combo_class_buf[i].raft);
            al_trace("reset_room:  %d\n", combo_class_buf[i].reset_room);
            al_trace("save_point_type:  %d\n", combo_class_buf[i].save_point_type);
            al_trace("screen_freeze_type:  %d\n", combo_class_buf[i].screen_freeze_type);
            al_trace("secret_combo:  %d\n", combo_class_buf[i].secret_combo);
            al_trace("singular:  %d\n", combo_class_buf[i].singular);
            al_trace("slow_movement:  %d\n", combo_class_buf[i].slow_movement);
            al_trace("statue_type:  %d\n", combo_class_buf[i].statue_type);
            al_trace("step_type:  %d\n", combo_class_buf[i].step_type);
            al_trace("step_change_to:  %ld\n", combo_class_buf[i].step_change_to);
            for(int j=0; j<32; j++)
            {
              al_trace("strike_weapons[%d]:  %d\n", j, combo_class_buf[i].strike_weapons[j]);
            }
            al_trace("strike_remnants:  %ld\n", combo_class_buf[i].strike_remnants);
            al_trace("strike_remnants_type:  %d\n", combo_class_buf[i].strike_remnants_type);
            al_trace("strike_change:  %ld\n", combo_class_buf[i].strike_change);
            al_trace("strike_item:  %d\n", combo_class_buf[i].strike_item);
            al_trace("touch_item:  %d\n", combo_class_buf[i].touch_item);
            al_trace("touch_stairs:  %d\n", combo_class_buf[i].touch_stairs);
            al_trace("trigger_type:  %d\n", combo_class_buf[i].trigger_type);
            al_trace("trigger_sensitive:  %d\n", combo_class_buf[i].trigger_sensitive);
            al_trace("warp_type:  %d\n", combo_class_buf[i].warp_type);
            al_trace("warp_sensitive:  %d\n", combo_class_buf[i].warp_sensitive);
            al_trace("warp_direct:  %d\n", combo_class_buf[i].warp_direct);
            al_trace("warp_location:  %d\n", combo_class_buf[i].warp_location);
            al_trace("water:  %d\n", combo_class_buf[i].water);
            al_trace("whistle:  %d\n", combo_class_buf[i].whistle);
            al_trace("win_game:  %d\n", combo_class_buf[i].win_game);
            al_trace("\n\n");
        */
    }
    
    return 0;
}

int readlinksprites2(PACKFILE *f, int v_linksprites, int cv_linksprites, bool keepdata)
{
    //these are here to bypass compiler warnings about unused arguments
    cv_linksprites=cv_linksprites;
    
    if(keepdata)
    {
        zinit.link_swim_speed=67; //default
        setuplinktiles(zinit.linkanimationstyle);
    }
    
    if(v_linksprites>=0)
    {
        word tile, tile2;
        byte flip, extend, dummy_byte;
        
        for(int i=0; i<4; i++)
        {
            if(!p_igetw(&tile,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&flip,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&extend,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(keepdata)
            {
                walkspr[i][spr_tile]=(int)tile;
                walkspr[i][spr_flip]=(int)flip;
                walkspr[i][spr_extend]=(int)extend;
            }
        }
        
        for(int i=0; i<4; i++)
        {
            if(!p_igetw(&tile,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&flip,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&extend,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(keepdata)
            {
                stabspr[i][spr_tile]=(int)tile;
                stabspr[i][spr_flip]=(int)flip;
                stabspr[i][spr_extend]=(int)extend;
            }
        }
        
        for(int i=0; i<4; i++)
        {
            if(!p_igetw(&tile,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&flip,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&extend,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(keepdata)
            {
                slashspr[i][spr_tile]=(int)tile;
                slashspr[i][spr_flip]=(int)flip;
                slashspr[i][spr_extend]=(int)extend;
            }
        }
        
        for(int i=0; i<4; i++)
        {
            if(!p_igetw(&tile,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&flip,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&extend,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(keepdata)
            {
                floatspr[i][spr_tile]=(int)tile;
                floatspr[i][spr_flip]=(int)flip;
                floatspr[i][spr_extend]=(int)extend;
            }
        }
        
        if(v_linksprites>1)
        {
            for(int i=0; i<4; i++)
            {
                if(!p_igetw(&tile,f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_getc(&flip,f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_getc(&extend,f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(keepdata)
                {
                    swimspr[i][spr_tile]=(int)tile;
                    swimspr[i][spr_flip]=(int)flip;
                    swimspr[i][spr_extend]=(int)extend;
                }
            }
        }
        
        for(int i=0; i<4; i++)
        {
            if(!p_igetw(&tile,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&flip,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&extend,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(keepdata)
            {
                divespr[i][spr_tile]=(int)tile;
                divespr[i][spr_flip]=(int)flip;
                divespr[i][spr_extend]=(int)extend;
            }
        }
        
        for(int i=0; i<4; i++)
        {
            if(!p_igetw(&tile,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&flip,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&extend,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(keepdata)
            {
                poundspr[i][spr_tile]=(int)tile;
                poundspr[i][spr_flip]=(int)flip;
                poundspr[i][spr_extend]=(int)extend;
            }
        }
        
        if(!p_igetw(&tile,f,keepdata))
        {
            return qe_invalid;
        }
        
        flip=0;
        
        if(v_linksprites>0)
        {
            if(!p_getc(&flip,f,keepdata))
            {
                return qe_invalid;
            }
        }
        
        if(!p_getc(&extend,f,keepdata))
        {
            return qe_invalid;
        }
        
        if(keepdata)
        {
            castingspr[spr_tile]=(int)tile;
            castingspr[spr_flip]=(int)flip;
            castingspr[spr_extend]=(int)extend;
        }
        
        if(v_linksprites>0)
        {
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<2; j++)
                {
                    if(!p_igetw(&tile,f,keepdata))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_getc(&flip,f,keepdata))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_getc(&extend,f,keepdata))
                    {
                        return qe_invalid;
                    }
                    
                    if(keepdata)
                    {
                        holdspr[i][j][spr_tile]=(int)tile;
                        holdspr[i][j][spr_flip]=(int)flip;
                        holdspr[i][j][spr_extend]=(int)extend;
                    }
                }
            }
        }
        else
        {
            for(int i=0; i<2; i++)
            {
                if(!p_igetw(&tile,f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_igetw(&tile2,f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_getc(&extend,f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(keepdata)
                {
                    holdspr[i][spr_hold1][spr_tile]=(int)tile;
                    holdspr[i][spr_hold1][spr_flip]=(int)flip;
                    holdspr[i][spr_hold1][spr_extend]=(int)extend;
                    holdspr[i][spr_hold2][spr_tile]=(int)tile2;
                    holdspr[i][spr_hold1][spr_flip]=(int)flip;
                    holdspr[i][spr_hold2][spr_extend]=(int)extend;
                }
            }
        }
        
        if(v_linksprites>2)
        {
            for(int i=0; i<4; i++)
            {
                if(!p_igetw(&tile,f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_getc(&flip,f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_getc(&extend,f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(keepdata)
                {
                    jumpspr[i][spr_tile]=(int)tile;
                    jumpspr[i][spr_flip]=(int)flip;
                    jumpspr[i][spr_extend]=(int)extend;
                }
            }
        }
        
        if(v_linksprites>3)
        {
            for(int i=0; i<4; i++)
            {
                if(!p_igetw(&tile,f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_getc(&flip,f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_getc(&extend,f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(keepdata)
                {
                    chargespr[i][spr_tile]=(int)tile;
                    chargespr[i][spr_flip]=(int)flip;
                    chargespr[i][spr_extend]=(int)extend;
                }
            }
        }
        
        if(v_linksprites>4)
        {
            if(!p_getc(&dummy_byte,f,keepdata))
            {
                return qe_invalid;
            }
            
            if(keepdata)
            {
                zinit.link_swim_speed=(byte)dummy_byte;
            }
        }
    }
    
    return 0;
}

int readlinksprites(PACKFILE *f, zquestheader *Header, bool keepdata)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    
    dword dummy;
    word s_version=0, s_cversion=0;
    
    //section version info
    if(!p_igetw(&s_version,f,true))
    {
        return qe_invalid;
    }
    
    ffcore.quest_format[vLinkSprites] = s_version;
    
    //al_trace("Link sprites version %d\n", s_version);
    if(!p_igetw(&s_cversion,f,true))
    {
        return qe_invalid;
    }
    
    //section size
    if(!p_igetl(&dummy,f,true))
    {
        return qe_invalid;
    }
    
    return readlinksprites2(f, s_version, dummy, keepdata);
}

int readsubscreens(PACKFILE *f, zquestheader *Header, bool keepdata)
{
    int dummy;
    word s_version=0, s_cversion=0;
    
    //section version info
    if(!p_igetw(&s_version,f,true))
    {
        return qe_invalid;
    }
    
    ffcore.quest_format[vSubscreen] = s_version;
    
    //al_trace("Subscreens version %d\n", s_version);
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
    for(int i=0; i<MAXCUSTOMSUBSCREENS; i++)
    {
        int ret = read_one_subscreen(f, Header, keepdata, i, s_version, s_cversion);
        
        if(ret!=0) return ret;
    }
    
    return 0;
}

int read_one_subscreen(PACKFILE *f, zquestheader *, bool keepdata, int i, word s_version, word)
{
    GarbageCollector gc;
    
    int numsub=0;
    byte temp_ss=0;
    subscreen_object *temp_sub = gc(new subscreen_object);
    
    char tempname[64];
    
    if(!pfread(tempname,64,f,true))
    {
        return qe_invalid;
    }
    
    if(s_version > 1)
    {
        if(!p_getc(&temp_ss,f,keepdata))
        {
            return qe_invalid;
        }
    }
    
    if(s_version < 4)
    {
        unsigned char tmp=0;
        
        if(!p_getc(&tmp,f,true))
        {
            return qe_invalid;
        }
        
        numsub = (int)tmp;
    }
    else
    {
        word tmp;
        
        if(!p_igetw(&tmp, f, true))
        {
            return qe_invalid;
        }
        
        numsub = (int)tmp;
    }
    
    int j;
    
    for(j=0; (j<MAXSUBSCREENITEMS&&j<numsub); j++)
    {
        if(keepdata)
        {
            memset(temp_sub,0,sizeof(subscreen_object));
            
            switch(custom_subscreen[i].objects[j].type)
            {
            case ssoTEXT:
            case ssoTEXTBOX:
            case ssoCURRENTITEMTEXT:
            case ssoCURRENTITEMCLASSTEXT:
                if(custom_subscreen[i].objects[j].dp1 != NULL) delete [](char *)custom_subscreen[i].objects[j].dp1;
                
                //fall through
            default:
                memset(&custom_subscreen[i].objects[j],0,sizeof(subscreen_object));
                break;
            }
        }
        
        if(!p_getc(&(temp_sub->type),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&(temp_sub->pos),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(s_version < 5)
        {
            switch(temp_sub->pos)
            {
            case 0:
                temp_sub->pos = sspUP | sspDOWN | sspSCROLLING;
                break;
                
            case 1:
                temp_sub->pos = sspUP;
                break;
                
            case 2:
                temp_sub->pos = sspDOWN;
                break;
                
            default:
                temp_sub->pos = 0;
            }
        }
        
        if(!p_igetw(&(temp_sub->x),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_sub->y),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_sub->w),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_sub->h),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&(temp_sub->colortype1),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_sub->color1),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&(temp_sub->colortype2),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_sub->color2),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&(temp_sub->colortype3),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_sub->color3),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_igetd(&(temp_sub->d1),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_igetd(&(temp_sub->d2),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_igetd(&(temp_sub->d3),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_igetd(&(temp_sub->d4),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_igetd(&(temp_sub->d5),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_igetd(&(temp_sub->d6),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_igetd(&(temp_sub->d7),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_igetd(&(temp_sub->d8),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_igetd(&(temp_sub->d9),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_igetd(&(temp_sub->d10),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(s_version < 2)
        {
            if(!p_igetl(&(temp_sub->speed),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_igetl(&(temp_sub->delay),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_igetl(&(temp_sub->frame),f,keepdata))
            {
                return qe_invalid;
            }
        }
        else
        {
            if(!p_getc(&(temp_sub->speed),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&(temp_sub->delay),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&(temp_sub->frame),f,keepdata))
            {
                return qe_invalid;
            }
        }
        
        int temp_size=0;
        
        // bool deletets = false;
        switch(temp_sub->type)
        {
        case ssoTEXT:
        case ssoTEXTBOX:
        case ssoCURRENTITEMTEXT:
        case ssoCURRENTITEMCLASSTEXT:
            word temptempsize;
            /*unsigned char temp1;
            unsigned char temp2;
            temp2 = 0;
            if(!p_getc(&temp1,f,true))
                {
                  return qe_invalid;
                }
            if(temp1)
            {
            
            if(!p_getc(&temp2,f,true))
                {
                  return qe_invalid;
                }
            }*/
            
            if(!p_igetw(&temptempsize,f,true))
            {
                return qe_invalid;
            }
            
            //temptempsize = temp1 + (temp2 << 8);
            temp_size = (int)temptempsize;
            
            //if(temp_sub->dp1!=NULL) delete[] temp_sub->dp1;
            if(keepdata)
            {
                unsigned int char_length = temp_size+1;
                temp_sub->dp1 = new char[char_length]; //memory not freed
                
                //deletets = true; //obsolete
            }
            
            if(temp_size)
            {
                if(!pfread(temp_sub->dp1,temp_size+1,f,keepdata))
                {
                    return qe_invalid;
                }
            }
            
            break;
            
        case ssoLIFEMETER:
            if(get_bit(deprecated_rules, 12) != 0) // qr_24HC
                temp_sub->d3 = 1;
                
            if(!p_getc(&(temp_sub->dp1),f,keepdata))
            {
                return qe_invalid;
            }
            
            break;
            
            
        case ssoCURRENTITEM:
        
            if(s_version < 6)
            {
                switch(temp_sub->d1)
                {
                case ssiBOMB:
                    temp_sub->d1 = itype_bomb;
                    break;
                    
                case ssiSWORD:
                    temp_sub->d1 = itype_sword;
                    break;
                    
                case ssiSHIELD:
                    temp_sub->d1 = itype_shield;
                    break;
                    
                case ssiCANDLE:
                    temp_sub->d1 = itype_candle;
                    break;
                    
                case ssiLETTER:
                    temp_sub->d1 = itype_letter;
                    break;
                    
                case ssiPOTION:
                    temp_sub->d1 = itype_potion;
                    break;
                    
                case ssiLETTERPOTION:
                    temp_sub->d1 = itype_letterpotion;
                    break;
                    
                case ssiBOW:
                    temp_sub->d1 = itype_bow;
                    break;
                    
                case ssiARROW:
                    temp_sub->d1 = itype_arrow;
                    break;
                    
                case ssiBOWANDARROW:
                    temp_sub->d1 = itype_bowandarrow;
                    break;
                    
                case ssiBAIT:
                    temp_sub->d1 = itype_bait;
                    break;
                    
                case ssiRING:
                    temp_sub->d1 = itype_ring;
                    break;
                    
                case ssiBRACELET:
                    temp_sub->d1 = itype_bracelet;
                    break;
                    
                case ssiMAP:
                    temp_sub->d1 = itype_map;
                    break;
                    
                case ssiCOMPASS:
                    temp_sub->d1 = itype_compass;
                    break;
                    
                case ssiBOSSKEY:
                    temp_sub->d1 = itype_bosskey;
                    break;
                    
                case ssiMAGICKEY:
                    temp_sub->d1 = itype_magickey;
                    break;
                    
                case ssiBRANG:
                    temp_sub->d1 = itype_brang;
                    break;
                    
                case ssiWAND:
                    temp_sub->d1 = itype_wand;
                    break;
                    
                case ssiRAFT:
                    temp_sub->d1 = itype_raft;
                    break;
                    
                case ssiLADDER:
                    temp_sub->d1 = itype_ladder;
                    break;
                    
                case ssiWHISTLE:
                    temp_sub->d1 = itype_whistle;
                    break;
                    
                case ssiBOOK:
                    temp_sub->d1 = itype_book;
                    break;
                    
                case ssiWALLET:
                    temp_sub->d1 = itype_wallet;
                    break;
                    
                case ssiSBOMB:
                    temp_sub->d1 = itype_sbomb;
                    break;
                    
                case ssiHCPIECE:
                    temp_sub->d1 = itype_heartpiece;
                    break;
                    
                case ssiAMULET:
                    temp_sub->d1 = itype_amulet;
                    break;
                    
                case ssiFLIPPERS:
                    temp_sub->d1 = itype_flippers;
                    break;
                    
                case ssiHOOKSHOT:
                    temp_sub->d1 = itype_hookshot;
                    break;
                    
                case ssiLENS:
                    temp_sub->d1 = itype_lens;
                    break;
                    
                case ssiHAMMER:
                    temp_sub->d1 = itype_hammer;
                    break;
                    
                case ssiBOOTS:
                    temp_sub->d1 = itype_boots;
                    break;
                    
                case ssiDINSFIRE:
                    temp_sub->d1 = itype_dinsfire;
                    break;
                    
                case ssiFARORESWIND:
                    temp_sub->d1 = itype_faroreswind;
                    break;
                    
                case ssiNAYRUSLOVE:
                    temp_sub->d1 = itype_nayruslove;
                    break;
                    
                case ssiQUIVER:
                    temp_sub->d1 = itype_quiver;
                    break;
                    
                case ssiBOMBBAG:
                    temp_sub->d1 = itype_bombbag;
                    break;
                    
                case ssiCBYRNA:
                    temp_sub->d1 = itype_cbyrna;
                    break;
                    
                case ssiROCS:
                    temp_sub->d1 = itype_rocs;
                    break;
                    
                case ssiHOVERBOOTS:
                    temp_sub->d1 = itype_hoverboots;
                    break;
                    
                case ssiSPINSCROLL:
                    temp_sub->d1 = itype_spinscroll;
                    break;
                    
                case ssiCROSSSCROLL:
                    temp_sub->d1 = itype_crossscroll;
                    break;
                    
                case ssiQUAKESCROLL:
                    temp_sub->d1 = itype_quakescroll;
                    break;
                    
                case ssiWHISPRING:
                    temp_sub->d1 = itype_whispring;
                    break;
                    
                case ssiCHARGERING:
                    temp_sub->d1 = itype_chargering;
                    break;
                    
                case ssiPERILSCROLL:
                    temp_sub->d1 = itype_perilscroll;
                    break;
                    
                case ssiWEALTHMEDAL:
                    temp_sub->d1 = itype_wealthmedal;
                    break;
                    
                case ssiHEARTRING:
                    temp_sub->d1 = itype_heartring;
                    break;
                    
                case ssiMAGICRING:
                    temp_sub->d1 = itype_magicring;
                    break;
                    
                case ssiSPINSCROLL2:
                    temp_sub->d1 = itype_spinscroll2;
                    break;
                    
                case ssiQUAKESCROLL2:
                    temp_sub->d1 = itype_quakescroll2;
                    break;
                    
                case ssiAGONY:
                    temp_sub->d1 = itype_agony;
                    break;
                    
                case ssiSTOMPBOOTS:
                    temp_sub->d1 = itype_stompboots;
                    break;
                    
                case ssiWHIMSICALRING:
                    temp_sub->d1 = itype_whimsicalring;
                    break;
                    
                case ssiPERILRING:
                    temp_sub->d1 = itype_perilring;
                    break;
                    
                default:
                    temp_sub->d1 += itype_custom1 - ssiMAX;
                }
            }
            
            //fall-through
        default:
            if(!p_getc(&(temp_sub->dp1),f,keepdata))
            {
                return qe_invalid;
            }
            
            break;
        }
        
        if(keepdata)
        {
            switch(temp_sub->type)
            {
            case ssoTEXT:
            case ssoTEXTBOX:
            case ssoCURRENTITEMTEXT:
            case ssoCURRENTITEMCLASSTEXT:
                if(custom_subscreen[i].objects[j].dp1 != NULL) delete[](char *)custom_subscreen[i].objects[j].dp1;
                
                memcpy(&custom_subscreen[i].objects[j],temp_sub,sizeof(subscreen_object));
                custom_subscreen[i].objects[j].dp1 = NULL;
                custom_subscreen[i].objects[j].dp1 = new char[temp_size+1];
                strcpy((char*)custom_subscreen[i].objects[j].dp1,(char*)temp_sub->dp1);
                break;
                
            case ssoCOUNTER:
                if(s_version<3)
                {
                    temp_sub->d6=(temp_sub->d6?1:0)+(temp_sub->d8?2:0);
                    temp_sub->d8=0;
                }
                
            default:
                memcpy(&custom_subscreen[i].objects[j],temp_sub,sizeof(subscreen_object));
                break;
            }
            
            strcpy(custom_subscreen[i].name, tempname);
            custom_subscreen[i].ss_type = temp_ss;
        }
    }
    
    for(j=numsub; j<MAXSUBSCREENITEMS; j++)
    {
        if(keepdata)
        {
            //clear all unused object in this subscreen -DD
            switch(custom_subscreen[i].objects[j].type)
            {
            case ssoTEXT:
            case ssoTEXTBOX:
            case ssoCURRENTITEMTEXT:
            case ssoCURRENTITEMCLASSTEXT:
                if(custom_subscreen[i].objects[j].dp1 != NULL) delete [](char *)custom_subscreen[i].objects[j].dp1;
                
                //fall through
            default:
                memset(&custom_subscreen[i].objects[j],0,sizeof(subscreen_object));
                break;
            }
        }
    }
    
    //delete temp_sub; //wtf
    return 0;
}

void reset_subscreen(subscreen_group *tempss)
{
    for(int i=0; i<MAXSUBSCREENITEMS; ++i)
    {
        switch(tempss->objects[i].type)
        {
        case ssoTEXT:
        case ssoTEXTBOX:
        case ssoCURRENTITEMTEXT:
        case ssoCURRENTITEMCLASSTEXT:
            if(tempss->objects[i].dp1 != NULL) delete [](char *)tempss->objects[i].dp1;
            
            //fall through
        default:
            memset(&tempss->objects[i],0,sizeof(subscreen_object));
            break;
        }
    }
}

void reset_subscreens()
{
    for(int i=0; i<MAXCUSTOMSUBSCREENS; ++i)
    {
        reset_subscreen(&custom_subscreen[i]);
    }
}

int setupsubscreens()
{
    reset_subscreens();
    int tempsubscreen=zinit.subscreen;
    subscreen_object *tempsub;
    
    if(tempsubscreen>=ssdtMAX)
    {
        tempsubscreen=0;
    }
    
    switch(tempsubscreen)
    {
    case ssdtOLD:
    case ssdtNEWSUBSCR:
    case ssdtREV2:
    case ssdtBSZELDA:
    case ssdtBSZELDAMODIFIED:
    case ssdtBSZELDAENHANCED:
    case ssdtBSZELDACOMPLETE:
    {
        tempsub = default_subscreen_active[tempsubscreen][0];
        int i;
        
        for(i=0; (i<MAXSUBSCREENITEMS&&tempsub[i].type!=ssoNULL); i++)
        {
            switch(tempsub[i].type)
            {
            case ssoTEXT:
            case ssoTEXTBOX:
            case ssoCURRENTITEMTEXT:
            case ssoCURRENTITEMCLASSTEXT:
                if(custom_subscreen[0].objects[i].dp1 != NULL) delete [](char *)custom_subscreen[0].objects[i].dp1;
                
                memcpy(&custom_subscreen[0].objects[i],&tempsub[i],sizeof(subscreen_object));
                custom_subscreen[0].objects[i].dp1 = NULL;
                custom_subscreen[0].objects[i].dp1 = new char[strlen((char*)tempsub[i].dp1)+1];
                strcpy((char*)custom_subscreen[0].objects[i].dp1,(char*)tempsub[i].dp1);
                break;
                
            case ssoLIFEMETER:
            {
                memcpy(&custom_subscreen[0].objects[i],&tempsub[i],sizeof(subscreen_object));
                
                if(get_bit(deprecated_rules, 12) != 0)
                    custom_subscreen[0].objects[i].d3=1;
                else
                    custom_subscreen[0].objects[i].d3=0;
                    
                break;
            }
            
            default:
                memcpy(&custom_subscreen[0].objects[i],&tempsub[i],sizeof(subscreen_object));
                break;
            }
        }
        
        custom_subscreen[0].ss_type=sstACTIVE;
        sprintf(custom_subscreen[0].name, "Active Subscreen (Triforce)");
        tempsub = default_subscreen_active[tempsubscreen][1];
        
        for(i=0; (i<MAXSUBSCREENITEMS&&tempsub[i].type!=ssoNULL); i++)
        {
            switch(tempsub[i].type)
            {
            case ssoTEXT:
            case ssoTEXTBOX:
            case ssoCURRENTITEMTEXT:
            case ssoCURRENTITEMCLASSTEXT:
                if(custom_subscreen[1].objects[i].dp1 != NULL) delete [](char *)custom_subscreen[1].objects[i].dp1;
                
                memcpy(&custom_subscreen[1].objects[i],&tempsub[i],sizeof(subscreen_object));
                custom_subscreen[1].objects[i].dp1 = NULL;
                custom_subscreen[1].objects[i].dp1 = new char[strlen((char*)tempsub[i].dp1)+1];
                strcpy((char*)custom_subscreen[1].objects[i].dp1,(char*)tempsub[i].dp1);
                break;
                
            case ssoLIFEMETER:
            {
                memcpy(&custom_subscreen[1].objects[i],&tempsub[i],sizeof(subscreen_object));
                
                if(get_bit(deprecated_rules, 12) != 0)
                    custom_subscreen[1].objects[i].d3=1;
                else
                    custom_subscreen[1].objects[i].d3=0;
                    
                break;
            }
            
            default:
                memcpy(&custom_subscreen[1].objects[i],&tempsub[i],sizeof(subscreen_object));
                break;
            }
        }
        
        custom_subscreen[1].ss_type=sstACTIVE;
        sprintf(custom_subscreen[1].name, "Active Subscreen (Dungeon Map)");
//      memset(&custom_subscreen[1].objects[i],0,sizeof(subscreen_object));
        tempsub = default_subscreen_passive[tempsubscreen][0];
        
        for(i=0; (i<MAXSUBSCREENITEMS&&tempsub[i].type!=ssoNULL); i++)
        {
            switch(tempsub[i].type)
            {
            case ssoTEXT:
            case ssoTEXTBOX:
            case ssoCURRENTITEMTEXT:
            case ssoCURRENTITEMCLASSTEXT:
                if(custom_subscreen[2].objects[i].dp1 != NULL) delete [](char *)custom_subscreen[2].objects[i].dp1;
                
                memcpy(&custom_subscreen[2].objects[i],&tempsub[i],sizeof(subscreen_object));
                custom_subscreen[2].objects[i].dp1 = NULL;
                custom_subscreen[2].objects[i].dp1 = new char[strlen((char*)tempsub[i].dp1)+1];
                strcpy((char*)custom_subscreen[2].objects[i].dp1,(char*)tempsub[i].dp1);
                break;
                
            case ssoLIFEMETER:
            {
                memcpy(&custom_subscreen[2].objects[i],&tempsub[i],sizeof(subscreen_object));
                
                if(get_bit(deprecated_rules, 12) != 0)
                    custom_subscreen[2].objects[i].d3=1;
                else
                    custom_subscreen[2].objects[i].d3=0;
                    
                break;
            }
            
            default:
                memcpy(&custom_subscreen[2].objects[i],&tempsub[i],sizeof(subscreen_object));
                break;
            }
        }
        
        custom_subscreen[2].ss_type=sstPASSIVE;
        sprintf(custom_subscreen[2].name, "Passive Subscreen (Magic)");
//      memset(&custom_subscreen[2].objects[i],0,sizeof(subscreen_object));
        tempsub = default_subscreen_passive[tempsubscreen][1];
        
        for(i=0; (i<MAXSUBSCREENITEMS&&tempsub[i].type!=ssoNULL); i++)
        {
            switch(tempsub[i].type)
            {
            case ssoTEXT:
            case ssoTEXTBOX:
            case ssoCURRENTITEMTEXT:
            case ssoCURRENTITEMCLASSTEXT:
                if(custom_subscreen[3].objects[i].dp1 != NULL) delete [](char *)custom_subscreen[3].objects[i].dp1;
                
                memcpy(&custom_subscreen[3].objects[i],&tempsub[i],sizeof(subscreen_object));
                custom_subscreen[3].objects[i].dp1 = NULL;
                custom_subscreen[3].objects[i].dp1 = new char[strlen((char*)tempsub[i].dp1)+1];
                strcpy((char*)custom_subscreen[3].objects[i].dp1,(char*)tempsub[i].dp1);
                break;
                
            case ssoLIFEMETER:
            {
                memcpy(&custom_subscreen[3].objects[i],&tempsub[i],sizeof(subscreen_object));
                
                if(get_bit(deprecated_rules, 12) != 0)
                    custom_subscreen[3].objects[i].d3=1;
                else
                    custom_subscreen[3].objects[i].d3=0;
                    
                break;
            }
            
            default:
                memcpy(&custom_subscreen[3].objects[i],&tempsub[i],sizeof(subscreen_object));
                break;
            }
        }
        
        custom_subscreen[3].ss_type=sstPASSIVE;
        sprintf(custom_subscreen[3].name, "Passive Subscreen (No Magic)");
//      memset(&custom_subscreen[3].objects[i],0,sizeof(subscreen_object));
        break;
    }
    
    case ssdtZ3:
    {
        tempsub = z3_active_a;
        int i;
        
        for(i=0; (i<MAXSUBSCREENITEMS&&tempsub[i].type!=ssoNULL); i++)
        {
            switch(tempsub[i].type)
            {
            case ssoTEXT:
            case ssoTEXTBOX:
            case ssoCURRENTITEMTEXT:
            case ssoCURRENTITEMCLASSTEXT:
                if(custom_subscreen[0].objects[i].dp1 != NULL) delete [](char *)custom_subscreen[0].objects[i].dp1;
                
                memcpy(&custom_subscreen[0].objects[i],&tempsub[i],sizeof(subscreen_object));
                custom_subscreen[0].objects[i].dp1 = NULL;
                custom_subscreen[0].objects[i].dp1 = new char[strlen((char*)tempsub[i].dp1)+1];
                strcpy((char*)custom_subscreen[0].objects[i].dp1,(char*)tempsub[i].dp1);
                break;
                
            case ssoLIFEMETER:
            {
                memcpy(&custom_subscreen[0].objects[i],&tempsub[i],sizeof(subscreen_object));
                
                if(get_bit(deprecated_rules, 12) != 0)
                    custom_subscreen[0].objects[i].d3=1;
                else
                    custom_subscreen[0].objects[i].d3=0;
                    
                break;
            }
            
            default:
                memcpy(&custom_subscreen[0].objects[i],&tempsub[i],sizeof(subscreen_object));
                break;
            }
        }
        
        custom_subscreen[0].ss_type=sstACTIVE;
//      memset(&custom_subscreen[0].objects[i],0,sizeof(subscreen_object));
        tempsub = z3_active_ab;
        
        for(i=0; (i<MAXSUBSCREENITEMS&&tempsub[i].type!=ssoNULL); i++)
        {
            switch(tempsub[i].type)
            {
            case ssoTEXT:
            case ssoTEXTBOX:
            case ssoCURRENTITEMTEXT:
            case ssoCURRENTITEMCLASSTEXT:
                if(custom_subscreen[1].objects[i].dp1 != NULL) delete [](char *)custom_subscreen[1].objects[i].dp1;
                
                memcpy(&custom_subscreen[1].objects[i],&tempsub[i],sizeof(subscreen_object));
                custom_subscreen[1].objects[i].dp1 = NULL;
                custom_subscreen[1].objects[i].dp1 = new char[strlen((char*)tempsub[i].dp1)+1];
                strcpy((char*)custom_subscreen[1].objects[i].dp1,(char*)tempsub[i].dp1);
                break;
                
            case ssoLIFEMETER:
            {
                memcpy(&custom_subscreen[1].objects[i],&tempsub[i],sizeof(subscreen_object));
                
                if(get_bit(deprecated_rules, 12) != 0)
                    custom_subscreen[1].objects[i].d3=1;
                else
                    custom_subscreen[1].objects[i].d3=0;
                    
                break;
            }
            
            default:
                memcpy(&custom_subscreen[1].objects[i],&tempsub[i],sizeof(subscreen_object));
                break;
            }
        }
        
        custom_subscreen[1].ss_type=sstACTIVE;
//      memset(&custom_subscreen[1].objects[i],0,sizeof(subscreen_object));
        tempsub = z3_passive_a;
        
        for(i=0; (i<MAXSUBSCREENITEMS&&tempsub[i].type!=ssoNULL); i++)
        {
            switch(tempsub[i].type)
            {
            case ssoTEXT:
            case ssoTEXTBOX:
            case ssoCURRENTITEMTEXT:
            case ssoCURRENTITEMCLASSTEXT:
                if(custom_subscreen[2].objects[i].dp1 != NULL) delete [](char *)custom_subscreen[2].objects[i].dp1;
                
                memcpy(&custom_subscreen[2].objects[i],&tempsub[i],sizeof(subscreen_object));
                custom_subscreen[2].objects[i].dp1 = NULL;
                custom_subscreen[2].objects[i].dp1 = new char[strlen((char*)tempsub[i].dp1)+1];
                strcpy((char*)custom_subscreen[2].objects[i].dp1,(char*)tempsub[i].dp1);
                break;
                
            case ssoLIFEMETER:
            {
                memcpy(&custom_subscreen[2].objects[i],&tempsub[i],sizeof(subscreen_object));
                
                if(get_bit(deprecated_rules, 12) != 0)
                    custom_subscreen[2].objects[i].d3=1;
                else
                    custom_subscreen[2].objects[i].d3=0;
                    
                break;
            }
            
            default:
                memcpy(&custom_subscreen[2].objects[i],&tempsub[i],sizeof(subscreen_object));
                break;
            }
        }
        
        custom_subscreen[2].ss_type=sstPASSIVE;
//      memset(&custom_subscreen[2].objects[i],0,sizeof(subscreen_object));
        tempsub = z3_passive_ab;
        
        for(i=0; (i<MAXSUBSCREENITEMS&&tempsub[i].type!=ssoNULL); i++)
        {
            switch(tempsub[i].type)
            {
            case ssoTEXT:
            case ssoTEXTBOX:
            case ssoCURRENTITEMTEXT:
            case ssoCURRENTITEMCLASSTEXT:
                if(custom_subscreen[3].objects[i].dp1 != NULL) delete [](char *)custom_subscreen[3].objects[i].dp1;
                
                memcpy(&custom_subscreen[3].objects[i],&tempsub[i],sizeof(subscreen_object));
                custom_subscreen[3].objects[i].dp1 = NULL;
                custom_subscreen[3].objects[i].dp1 = new char[strlen((char*)tempsub[i].dp1)+1];
                strcpy((char*)custom_subscreen[3].objects[i].dp1,(char*)tempsub[i].dp1);
                break;
                
            case ssoLIFEMETER:
            {
                memcpy(&custom_subscreen[3].objects[i],&tempsub[i],sizeof(subscreen_object));
                
                if(get_bit(deprecated_rules, 12) != 0)
                    custom_subscreen[3].objects[i].d3=1;
                else
                    custom_subscreen[3].objects[i].d3=0;
                    
                break;
            }
            
            default:
                memcpy(&custom_subscreen[3].objects[i],&tempsub[i],sizeof(subscreen_object));
                break;
            }
        }
        
        custom_subscreen[3].ss_type=sstPASSIVE;
//      memset(&custom_subscreen[3].objects[i],0,sizeof(subscreen_object));
        break;
    }
    }
    
    for(int i=0; i<4; ++i)
    {
        purge_blank_subscreen_objects(&custom_subscreen[i]);
    }
    
    return 0;
}

extern ffscript *ffscripts[512];
extern ffscript *itemscripts[256];
extern ffscript *guyscripts[256];
extern ffscript *wpnscripts[256];
extern ffscript *globalscripts[NUMSCRIPTGLOBAL];
extern ffscript *linkscripts[3];
extern ffscript *screenscripts[256];

int readffscript(PACKFILE *f, zquestheader *Header, bool keepdata)
{
    int dummy;
    word s_version=0, s_cversion=0;
    byte numscripts=0;
    numscripts=numscripts; //to avoid unused variables warnings
    int ret;
    
    //section version info
    if(!p_igetw(&s_version,f,true))
    {
        return qe_invalid;
    }
    
    ffcore.quest_format[vFFScript] = s_version;
    
    if(!p_igetw(&s_cversion,f,true))
    {
        return qe_invalid;
    }
    
    //al_trace("Scripts version %d\n", s_version);
    //section size
    if(!p_igetl(&dummy,f,true))
    {
        return qe_invalid;
    }
    
    //ZScriptVersion::setVersion(s_version); ~this ideally, but there's no ZC/ZQuest defines...
    setZScriptVersion(s_version); //Lumped in zelda.cpp and in zquest.cpp as zquest can't link ZScriptVersion
    
    //finally...  section data
    for(int i = 0; i < ((s_version < 2) ? NUMSCRIPTFFCOLD : NUMSCRIPTFFC); i++)
    {
        ret = read_one_ffscript(f, Header, keepdata, i, s_version, s_cversion, &ffscripts[i]);
        
        if(ret != 0) return qe_invalid;
    }
    
    if(s_version > 1)
    {
        for(int i = 0; i < NUMSCRIPTITEM; i++)
        {
            ret = read_one_ffscript(f, Header, keepdata, i, s_version, s_cversion, &itemscripts[i]);
            
            if(ret != 0) return qe_invalid;
        }
        
        for(int i = 0; i < NUMSCRIPTGUYS; i++)
        {
            ret = read_one_ffscript(f, Header, keepdata, i, s_version, s_cversion, &guyscripts[i]);
            
            if(ret != 0) return qe_invalid;
        }
        
        for(int i = 0; i < NUMSCRIPTWEAPONS; i++)
        {
            ret = read_one_ffscript(f, Header, keepdata, i, s_version, s_cversion, &wpnscripts[i]);
            
            if(ret != 0) return qe_invalid;
        }
        
        for(int i = 0; i < NUMSCRIPTSCREEN; i++)
        {
            ret = read_one_ffscript(f, Header, keepdata, i, s_version, s_cversion, &screenscripts[i]);
            
            if(ret != 0) return qe_invalid;
        }
        
        if(s_version > 4)
        {
            for(int i = 0; i < NUMSCRIPTGLOBAL; i++)
            {
                ret = read_one_ffscript(f, Header, keepdata, i, s_version, s_cversion, &globalscripts[i]);
                
                if(ret != 0) return qe_invalid;
            }
        }
        else
        {
            for(int i = 0; i < NUMSCRIPTGLOBALOLD; i++)
            {
                ret = read_one_ffscript(f, Header, keepdata, i, s_version, s_cversion, &globalscripts[i]);
                
                if(ret != 0) return qe_invalid;
            }
            
            if(globalscripts[GLOBAL_SCRIPT_CONTINUE] != NULL)
                delete [] globalscripts[GLOBAL_SCRIPT_CONTINUE];
                
            globalscripts[GLOBAL_SCRIPT_CONTINUE] = new ffscript[1];
            globalscripts[GLOBAL_SCRIPT_CONTINUE][0].command = 0xFFFF;
        }
        
        for(int i = 0; i < NUMSCRIPTLINK; i++)
        {
            ret = read_one_ffscript(f, Header, keepdata, i, s_version, s_cversion, &linkscripts[i]);
            
            if(ret != 0) return qe_invalid;
        }
    }
    
    if(s_version > 2)
    {
        long bufsize;
        p_igetl(&bufsize, f, true);
        char * buf = new char[bufsize+1];
        pfread(buf, bufsize, f, true);
        buf[bufsize]=0;
        
        if(keepdata)
            zScript = string(buf);
            
        delete[] buf;
        word numffcbindings;
        p_igetw(&numffcbindings, f, true);
        
        for(int i=0; i<numffcbindings; i++)
        {
            word id;
            p_igetw(&id, f, true);
            p_igetl(&bufsize, f, true);
            buf = new char[bufsize+1];
            pfread(buf, bufsize, f, true);
            buf[bufsize]=0;
            
            //fix for buggy older saved quests -DD
            if(keepdata && id < NUMSCRIPTFFC-1)
                ffcmap[id].second = buf;
                
            delete[] buf;
        }
        
        word numglobalbindings;
        p_igetw(&numglobalbindings, f, true);
        
        for(int i=0; i<numglobalbindings; i++)
        {
            word id;
            p_igetw(&id, f, true);
            p_igetl(&bufsize, f, true);
            buf = new char[bufsize+1];
            pfread(buf, bufsize, f, true);
            buf[bufsize]=0;
            
			// id in principle should be valid, since slot assignment cannot assign a global script to a bogus slot.
			// However, because of a corruption bug, some 2.50.x quests contain bogus entries in the global bindings table.
			// Ignore these. -DD
            if(keepdata && id >= 0 && id < NUMSCRIPTGLOBAL)
            {
                //Disable old '~Continue's, they'd wreak havoc. Bit messy, apologies ~Joe
                if(strcmp(buf,"~Continue") == 0)
                {
                    globalmap[id].second = "";
                    
                    if(globalscripts[GLOBAL_SCRIPT_CONTINUE] != NULL)
                        globalscripts[GLOBAL_SCRIPT_CONTINUE][0].command = 0xFFFF;
                }
                else
                {
                    globalmap[id].second = buf;
                }
            }
            
            delete[] buf;
        }
        
        if(s_version > 3)
        {
            word numitembindings;
            p_igetw(&numitembindings, f, true);
            
            for(int i=0; i<numitembindings; i++)
            {
                word id;
                p_igetw(&id, f, true);
                p_igetl(&bufsize, f, true);
                buf = new char[bufsize+1];
                pfread(buf, bufsize, f, true);
                buf[bufsize]=0;
                
                //fix this too
                if(keepdata && id <NUMSCRIPTITEM-1)
                    itemmap[id].second = buf;
                    
                delete[] buf;
            }
        }
    }
    
    return 0;
}

//Eh?
bool is_string_command(int command)
{
    command = command;
    return false;
}

void reset_scripts()
{
    //OK, who spaced this? ;)
    for(int i=0; i<NUMSCRIPTFFC; i++)
    {
        if(ffscripts[i]!=NULL) delete [] ffscripts[i];
    }
    
    for(int i=0; i<NUMSCRIPTITEM; i++)
    {
        if(itemscripts[i]!=NULL) delete [] itemscripts[i];
    }
    
    for(int i=0; i<NUMSCRIPTGUYS; i++)
    {
        if(guyscripts[i]!=NULL) delete [] guyscripts[i];
    }
    
    for(int i=0; i<NUMSCRIPTWEAPONS; i++)
    {
        if(wpnscripts[i]!=NULL) delete [] wpnscripts[i];
    }
    
    for(int i=0; i<NUMSCRIPTSCREEN; i++)
    {
        if(screenscripts[i]!=NULL) delete [] screenscripts[i];
    }
    
    for(int i=0; i<NUMSCRIPTGLOBAL; i++)
    {
        if(globalscripts[i]!=NULL) delete [] globalscripts[i];
    }
    
    for(int i=0; i<NUMSCRIPTLINK; i++)
    {
        if(linkscripts[i]!=NULL) delete [] linkscripts[i];
    }
    
    
    for(int i=0; i<NUMSCRIPTFFC; i++)
    {
        ffscripts[i] = new ffscript[1];
        ffscripts[i][0].command = 0xFFFF;
    }
    
    for(int i=0; i<NUMSCRIPTITEM; i++)
    {
        itemscripts[i] = new ffscript[1];
        itemscripts[i][0].command = 0xFFFF;
    }
    
    for(int i=0; i<NUMSCRIPTGUYS; i++)
    {
        guyscripts[i] = new ffscript[1];
        guyscripts[i][0].command = 0xFFFF;
    }
    
    for(int i=0; i<NUMSCRIPTWEAPONS; i++)
    {
        wpnscripts[i] = new ffscript[1];
        wpnscripts[i][0].command = 0xFFFF;
    }
    
    for(int i=0; i<NUMSCRIPTSCREEN; i++)
    {
        screenscripts[i] = new ffscript[1];
        screenscripts[i][0].command = 0xFFFF;
    }
    
    for(int i=0; i<NUMSCRIPTGLOBAL; i++)
    {
        globalscripts[i] = new ffscript[1];
        globalscripts[i][0].command = 0xFFFF;
    }
    
    for(int i=0; i<NUMSCRIPTLINK; i++)
    {
        linkscripts[i] = new ffscript[1];
        linkscripts[i][0].command = 0xFFFF;
    }
}

int read_one_ffscript(PACKFILE *f, zquestheader *, bool keepdata, int , word s_version, word , ffscript **script)
{

    //Please also update loadquest() when modifying this method -DD
    ffscript temp_script;
    temp_script.ptr=NULL;
    long num_commands=1000;
    
    if(s_version>=2)
    {
        if(!p_igetl(&num_commands,f,true))
        {
            return qe_invalid;
        }
    }
    
    if(keepdata)
    {
        //FIXME:
        if((*script) != NULL) //Surely we want to do this regardless of keepdata?
            delete [](*script);
            
        (*script) = new ffscript[num_commands]; //memory leak
    }
    
    for(int j=0; j<num_commands; j++)
    {
        if(!p_igetw(&(temp_script.command),f,true))
        {
            return qe_invalid;
        }
        
        if(temp_script.command == 0xFFFF)
        {
            if(keepdata)
                (*script)[j].command = 0xFFFF;
                
            break;
        }
        else
        {
            if(is_string_command(temp_script.command))
            {
            }
            else
            {
                if(!p_igetl(&(temp_script.arg1),f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_igetl(&(temp_script.arg2),f,keepdata))
                {
                    return qe_invalid;
                }
            }
            
            if(keepdata)
            {
                (*script)[j].command = temp_script.command;
                (*script)[j].arg1 = temp_script.arg1;
                (*script)[j].arg2 = temp_script.arg2;
                // I'll comment this out until the whole routine is finished using ptr
                //if(is_string_command(temp_script.command))
                //{
                //( *script)[j].ptr=(char *)zc_malloc(256);
                //memcpy((*script)[j].ptr, temp_script.ptr, 256);
                //}
            }
        }
    }
    
    return 0;
}

extern SAMPLE customsfxdata[WAV_COUNT];
extern unsigned char customsfxflag[WAV_COUNT>>3];
extern int sfxdat;
extern DATAFILE *sfxdata;
const char *old_sfx_string[Z35] =
{
    "Arrow", "Sword beam", "Bomb blast", "Boomerang",  "Subscreen cursor",
    "Shield is hit", "Item chime",  "Roar (Dodongo, Gohma)", "Shutter", "Enemy dies",
    "Enemy is hit", "Low hearts warning", "Fire", "Ganon's fanfare", "Boss is hit", "Hammer",
    "Hookshot", "Message", "Link is hit", "Item fanfare", "Bomb placed", "Item pickup",
    "Refill", "Roar (Aquamentus, Gleeok, Ganon)", "Item pickup 2", "Ocean ambience",
    "Secret chime", "Link dies", "Stairs", "Sword", "Roar (Manhandla, Digdogger, Patra)",
    "Wand magic", "Whistle", "Zelda's fanfare", "Charging weapon", "Charging weapon 2",
    "Din's Fire", "Enemy falls from ceiling", "Farore's Wind", "Fireball", "Tall Grass slashed",
    "Pound pounded", "Hover Boots", "Ice magic", "Jump", "Lens of Truth off", "Lens of Truth on",
    "Nayru's Love shield", "Nayru's Love shield 2", "Push block", "Rock", "Spell rocket down",
    "Spell rocket up", "Sword spin attack", "Splash", "Summon magic", "Sword tapping",
    "Sword tapping (secret)", "Whistle whirlwind", "Cane of Byrna orbit"
};
char *sfx_string[WAV_COUNT];

int readsfx(PACKFILE *f, zquestheader *Header, bool keepdata)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    
    long dummy;
    word s_version=0, s_cversion=0;
    //int ret;
    SAMPLE temp_sample;
    temp_sample.loop_start=0;
    temp_sample.loop_end=0;
    temp_sample.param=0;
    
    //section version info
    if(!p_igetw(&s_version,f,true))
    {
        return qe_invalid;
    }
    
    ffcore.quest_format[vSFX] = s_version;
    
    //al_trace("SFX version %d\n", s_version);
    if(!p_igetw(&s_cversion,f,true))
    {
        return qe_invalid;
    }
    
    //section size
    if(!p_igetl(&dummy,f,true))
    {
        return qe_invalid;
    }
    
    /* HIGHLY UNORTHODOX UPDATING THING, by L
     * This fixes quests made before revision 411 (such as the 'Lost Isle Build'),
     * where the meaning of GOTOLESS changed. It also coincided with V_SFX
     * changing from 1 to 2.
     */
    if(s_version < 2 && keepdata)
        set_bit(quest_rules,qr_GOTOLESSNOTEQUAL,1);
        
    /* End highly unorthodox updating thing */
    
    int wavcount = WAV_COUNT;
    
    if(s_version < 6)
        wavcount = 128;
        
    unsigned char tempflag[WAV_COUNT>>3];
    
    if(s_version < 4)
    {
        memset(tempflag, 0xFF, WAV_COUNT>>3);
    }
    else
    {
        if(s_version < 6)
            memset(tempflag, 0, WAV_COUNT>>3);
            
        for(int i=0; i<(wavcount>>3); i++)
        {
            p_getc(&tempflag[i], f, true);
        }
        
    }
    
    if(keepdata)
        memcpy(customsfxflag, tempflag, WAV_COUNT>>3);
        
    if(s_version>4)
    {
        for(int i=1; i<WAV_COUNT; i++)
        {
            if(keepdata)
            {
                sprintf(sfx_string[i],"s%03d",i);
                
                if((i<Z35))
                    strcpy(sfx_string[i], old_sfx_string[i-1]);
            }
            
            if((get_bit(tempflag, i-1) == 0) || i>=wavcount)
                continue;
                
            char tempname[36];
            
            if(!pfread(tempname, 36, f, keepdata))
            {
                return qe_invalid;
            }
            
            if(keepdata)
            {
                strcpy(sfx_string[i], tempname);
            }
        }
    }
    else
    {
        if(keepdata)
        {
            for(int i=1; i<WAV_COUNT; i++)
            {
                sprintf(sfx_string[i],"s%03d",i);
                
                if(i<Z35)
                    strcpy(sfx_string[i], old_sfx_string[i-1]);
            }
        }
    }
    
    //finally...  section data
    for(int i=1; i<wavcount; i++)
    {
        if(get_bit(tempflag, i-1) == 0)
            continue;
            
        if(!p_igetl(&dummy,f,true))
        {
            return qe_invalid;
        }
        
        (temp_sample.bits) = dummy;
        
        if(!p_igetl(&dummy,f,true))
        {
            return qe_invalid;
        }
        
        (temp_sample.stereo) = dummy;
        
        if(!p_igetl(&dummy,f,keepdata))
        {
            return qe_invalid;
        }
        
        (temp_sample.freq) = dummy;
        
        if(!p_igetl(&dummy,f,keepdata))
        {
            return qe_invalid;
        }
        
        (temp_sample.priority) = dummy;
        
        if(!p_igetl(&(temp_sample.len),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetl(&(temp_sample.loop_start),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_igetl(&(temp_sample.loop_end),f,keepdata))
        {
            return qe_invalid;
        }
        
        if(!p_igetl(&(temp_sample.param),f,keepdata))
        {
            return qe_invalid;
        }
        
        // al_trace("F%i: L%i\n",i,temp_sample.len);
//    temp_sample.data = new byte[(temp_sample.bits==8?1:2)*temp_sample.len];
        int len = (temp_sample.bits==8?1:2)*(temp_sample.stereo==0?1:2)*temp_sample.len;
        temp_sample.data = calloc(len,1);
        
        if(s_version < 3)
            len = (temp_sample.bits==8?1:2)*temp_sample.len;
            
        //old-style, non-portable loading (Bad Allegro! Bad!!) -DD
        if(s_version < 2)
        {
            if(!pfread(temp_sample.data, len,f,keepdata))
            {
                return qe_invalid;
            }
        }
        else
        {
            //re-endianfy the data
            int wordstoread = len / sizeof(word);
            
            for(int j=0; j<wordstoread; j++)
            {
                word temp;
                
                if(!p_igetw(&temp, f, keepdata))
                {
                    return qe_invalid;
                }
                
                if(keepdata)
                    ((word *)temp_sample.data)[j] = temp;
            }
        }
        
        if(keepdata)
        {
            if(customsfxdata[i].data!=NULL)
            {
//        delete [] customsfxdata[i].data;
                zc_free(customsfxdata[i].data);
            }
            
//      customsfxdata[i].data = new byte[(temp_sample.bits==8?1:2)*temp_sample.len];
            int len2 = (temp_sample.bits==8?1:2)*(temp_sample.stereo==0?1:2)*temp_sample.len;
            customsfxdata[i].data = calloc(len2,1);
            customsfxdata[i].bits = temp_sample.bits;
            customsfxdata[i].stereo = temp_sample.stereo;
            customsfxdata[i].freq = temp_sample.freq;
            customsfxdata[i].priority = temp_sample.priority;
            customsfxdata[i].len = temp_sample.len;
            customsfxdata[i].loop_start = temp_sample.loop_start;
            customsfxdata[i].loop_end = temp_sample.loop_end;
            customsfxdata[i].param = temp_sample.param;
            int cpylen = len2;
            
            if(s_version<3)
            {
                cpylen = (temp_sample.bits==8?1:2)*temp_sample.len;
                al_trace("WARNING: Quest SFX %d is in stereo, and may be corrupt.\n",i);
            }
            
            memcpy(customsfxdata[i].data,temp_sample.data,cpylen);
            
            
        }
        
        zc_free(temp_sample.data);
    }
    
    sfxdat=0;
    return 0;
}

void setupsfx()
{
    for(int i=1; i<WAV_COUNT; i++)
    {
        sprintf(sfx_string[i],"s%03d",i);
        
        if(i<Z35)
        {
            strcpy(sfx_string[i], old_sfx_string[i-1]);
        }
        
        memset(customsfxflag, 0, WAV_COUNT>>3);
        
        int j=i;
        
        if(i>Z35)
        {
            i=Z35;
        }
        
        SAMPLE *temp_sample = (SAMPLE *)sfxdata[i].dat;
        
        if(customsfxdata[j].data!=NULL)
        {
//    delete [] customsfxdata[j].data;
            zc_free(customsfxdata[j].data);
        }
        
//    customsfxdata[j].data = new byte[(temp_sample->bits==8?1:2)*temp_sample->len];
        customsfxdata[j].data = calloc((temp_sample->bits==8?1:2)*(temp_sample->stereo == 0 ? 1 : 2)*temp_sample->len,1);
        customsfxdata[j].bits = temp_sample->bits;
        customsfxdata[j].stereo = temp_sample->stereo;
        customsfxdata[j].freq = temp_sample->freq;
        customsfxdata[j].priority = temp_sample->priority;
        customsfxdata[j].len = temp_sample->len;
        customsfxdata[j].loop_start = temp_sample->loop_start;
        customsfxdata[j].loop_end = temp_sample->loop_end;
        customsfxdata[j].param = temp_sample->param;
        memcpy(customsfxdata[j].data, (temp_sample->data), (temp_sample->bits==8?1:2)*(temp_sample->stereo==0 ? 1 : 2)*temp_sample->len);
        i=j;
    }
}

extern char *guy_string[eMAXGUYS];
extern const char *old_guy_string[OLDMAXGUYS];

int readguys(PACKFILE *f, zquestheader *Header, bool keepdata)
{
    dword dummy;
    word dummy2;
    word guyversion=0;
    
    if(Header->zelda_version >= 0x193)
    {
        //section version info
        if(!p_igetw(&guyversion,f,true))
        {
            return qe_invalid;
        }
        
	ffcore.quest_format[vGuys] = guyversion;
	
        //al_trace("Guys version %d\n", guyversion);
        if(!p_igetw(&dummy2,f,true))
        {
            return qe_invalid;
        }
        
        //section size
        if(!p_igetl(&dummy,f,true))
        {
            return qe_invalid;
        }
    }
    
    if(guyversion > 3)
    {
        for(int i=0; i<MAXGUYS; i++)
        {
            char tempname[64];
            
            // rev. 1511 : guyversion = 23. upped to 512 editable enemies. -Gleeok
            // if guyversion < 23 then there is only 256 enemies in the packfile, so default the rest.
            if(guyversion < 23 && i >= OLDBETAMAXGUYS && keepdata)
            {
                memset(tempname, 0, sizeof(char)*64);
                sprintf(tempname, "e%03d", i);
                strcpy(guy_string[i], tempname);
                
                continue;
            }
            
            if(!pfread(tempname, 64, f, keepdata))
            {
                return qe_invalid;
            }
            
            // Don't retain names of uneditable enemy entries!
            if(keepdata)
            {
                // for version upgrade to 2.5
                if(guyversion < 23 && i >= 177)
                {
                    // some of the older builds have names such as 'zz123',
                    // (this order gets messed up with some eXXX and some zzXXX)
                    // so let's update to the newer naming convection. -Gleeok
                    char tmpbuf[64];
                    memset(tmpbuf, 0, sizeof(char)*64);
                    sprintf(tmpbuf, "zz%03d", i);
                    
                    if(memcmp(tempname, tmpbuf, size_t(5)) == 0)
                    {
                        memset(tempname, 0, sizeof(char)*64);
                        sprintf(tempname, "e%03d", i);
                    }
                }
                
                if(i >= OLDMAXGUYS || strlen(tempname)<1 || tempname[strlen(tempname)-1]!=' ')
                {
                    strcpy(guy_string[i], tempname);
                }
                else
                {
                    strcpy(guy_string[i],old_guy_string[i]);
                }
            }
        }
    }
    else
    {
        if(keepdata)
        {
            for(int i=0; i<eMAXGUYS; i++)
            {
                sprintf(guy_string[i],"zz%03d",i);
            }
            
            for(int i=0; i<OLDMAXGUYS; i++)
            {
                strcpy(guy_string[i],old_guy_string[i]);
            }
        }
    }
    
    
    //finally...  section data
    if(keepdata)
    {
        init_guys(guyversion);                            //using default data for now...
        
        // Goriya guy fix
        if((Header->zelda_version < 0x211)||((Header->zelda_version == 0x211)&&(Header->build<7)))
        {
            if(get_bit(quest_rules,qr_NEWENEMYTILES))
            {
                guysbuf[gGORIYA].tile=130;
                guysbuf[gGORIYA].e_tile=130;
            }
        }
    }
    
    if(Header->zelda_version < 0x193)
    {
        if(get_bit(deprecated_rules,46))
        {
            guysbuf[eDODONGO].cset=14;
            guysbuf[eDODONGO].bosspal=spDIG;
        }
    }
    
    // Not sure when this first changed, but it's necessary for 2.10, at least
    if(Header->zelda_version <= 0x210)
    {
        guysbuf[eGLEEOK1F].misc6 = 16;
        guysbuf[eGLEEOK2F].misc6 = 16;
        guysbuf[eGLEEOK3F].misc6 = 16;
        guysbuf[eGLEEOK4F].misc6 = 16;
        
        guysbuf[eWIZ1].misc4 = 1;
        guysbuf[eBATROBE].misc4 = 1;
        guysbuf[eSUMMONER].misc4 = 1;
        guysbuf[eWWIZ].misc4 = 1;
    }
    
    // The versions here may not be correct
    // zelda_version>=0x211 handled at guyversion<24
    if(Header->zelda_version <= 0x190)
    {
        guysbuf[eCENT1].misc3 = 0;
        guysbuf[eCENT2].misc3 = 0;
        guysbuf[eMOLDORM].misc2 = 0;
    }
    else if(Header->zelda_version <= 0x210)
    {
        guysbuf[eCENT1].misc3 = 1;
        guysbuf[eCENT2].misc3 = 1;
        guysbuf[eMOLDORM].misc2 = 0;
    }
    
    if(guyversion<=2)
    {
        return readlinksprites2(f, guyversion==2?0:-1, 0, keepdata);
    }
    
    if(guyversion > 3)
    {
        guydata tempguy;
        
        for(int i=0; i<MAXGUYS; i++)
        {
            if(guyversion < 23 && keepdata)   // May 2012 : 512 max enemies
            {
                if(i >= OLDBETAMAXGUYS)
                {
                    memset(&guysbuf[i], 0, sizeof(guydata));
                    continue;
                }
            }
            
            memset(&tempguy, 0, sizeof(guydata));
            
            if(!p_igetl(&(tempguy.flags),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_igetl(&(tempguy.flags2),f,keepdata))
            {
                return qe_invalid;
            }
            
	    if ( guyversion >= 36 ) //expanded tiles
	    {
		    if(!p_igetl(&(tempguy.tile),f,keepdata))
		    {
			return qe_invalid;
		    }
	    }
	    else
	    {
		    if(!p_igetw(&(tempguy.tile),f,keepdata))
		    {
			return qe_invalid;
		    }
	    }    
            if(!p_getc(&(tempguy.width),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&(tempguy.height),f,keepdata))
            {
                return qe_invalid;
            }
            
	    if ( guyversion >= 36 ) //expanded tiles
	    {
		    if(!p_igetl(&(tempguy.s_tile),f,keepdata))
		    {
			return qe_invalid;
		    }
	    }
	    else
	    {
		    if(!p_igetw(&(tempguy.s_tile),f,keepdata))
		    {
			return qe_invalid;
		    }
	    }
	    
            if(!p_getc(&(tempguy.s_width),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&(tempguy.s_height),f,keepdata))
            {
                return qe_invalid;
            }
            
	    if ( guyversion >= 36 ) //expanded tiles
	    {
		    if(!p_igetl(&(tempguy.e_tile),f,keepdata))
		    {
			return qe_invalid;
		    }
	    }
	    else
	    {
		    if(!p_igetw(&(tempguy.e_tile),f,keepdata))
		    {
			return qe_invalid;
		    }
	    }
	    
            if(!p_getc(&(tempguy.e_width),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&(tempguy.e_height),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&(tempguy.hp),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&(tempguy.family),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(guyversion < 9 && (i==eDKNUT1 || i==eDKNUT2 || i==eDKNUT3 || i==eDKNUT4 || i==eDKNUT5)) // Whoops, forgot about Darknuts...
            {
                if(get_bit(quest_rules,qr_NEWENEMYTILES))
                {
                    tempguy.s_tile=tempguy.e_tile+120;
                    tempguy.s_width=tempguy.e_width;
                    tempguy.s_height=tempguy.e_height;
                }
                else tempguy.s_tile=860;
            }
            
            if(!p_igetw(&(tempguy.cset),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&(tempguy.anim),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&(tempguy.e_anim),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&(tempguy.frate),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&(tempguy.e_frate),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(guyversion < 13)  // April 2009
            {
                if(get_bit(deprecated_rules, qr_SLOWENEMYANIM_DEP))
                {
                    tempguy.frate *= 2;
                    tempguy.e_frate *= 2;
                }
            }
            
            if(guyversion < 14)  // May 1 2009
            {
                if(tempguy.anim==a2FRMSLOW)
                {
                    tempguy.anim=a2FRM;
                    tempguy.frate *= 2;
                }
                
                if(tempguy.e_anim==a2FRMSLOW)
                {
                    tempguy.e_anim=a2FRM;
                    tempguy.e_frate *= 2;
                }
                
                if(tempguy.anim==aFLIPSLOW)
                {
                    tempguy.anim=aFLIP;
                    tempguy.frate *= 2;
                }
                
                if(tempguy.e_anim==aFLIPSLOW)
                {
                    tempguy.e_anim=aFLIP;
                    tempguy.e_frate *= 2;
                }
                
                if(tempguy.anim == aNEWDWALK) tempguy.anim = a4FRM4DIR;
                
                if(tempguy.e_anim == aNEWDWALK) tempguy.e_anim = a4FRM4DIR;
                
                if(tempguy.anim == aNEWPOLV || tempguy.anim == a4FRM3TRAP)
                {
                    tempguy.anim=a4FRM4DIR;
                    tempguy.s_tile=(get_bit(quest_rules,qr_NEWENEMYTILES) ? tempguy.e_tile : tempguy.tile)+20;
                }
                
                if(tempguy.e_anim == aNEWPOLV || tempguy.e_anim == a4FRM3TRAP)
                {
                    tempguy.e_anim=a4FRM4DIR;
                    tempguy.s_tile=(get_bit(quest_rules,qr_NEWENEMYTILES) ? tempguy.e_tile : tempguy.tile)+20;
                }
            }
            
            if(!p_igetw(&(tempguy.dp),f,keepdata))
            {
                return qe_invalid;
            }
            
            //correction for guy fire
            if(guyversion < 6)
            {
                if(i == gFIRE)
                    tempguy.dp = 2;
            }
            
            if(!p_igetw(&(tempguy.wdp),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&(tempguy.weapon),f,keepdata))
            {
                return qe_invalid;
            }
            
            //correction for bosses using triple, "rising" fireballs
            if(guyversion < 5)
            {
                if(i == eLAQUAM || i == eRAQUAM || i == eGOHMA1 || i == eGOHMA2 ||
                        i == eGOHMA3 || i == eGOHMA4)
                {
                    if(tempguy.weapon == ewFireball)
                        tempguy.weapon = ewFireball2;
                }
            }
            
            if(!p_igetw(&(tempguy.rate),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&(tempguy.hrate),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&(tempguy.step),f,keepdata))
            {
                return qe_invalid;
            }
            
            // HIGHLY UNORTHODOX UPDATING THING, part 2
            if(fixpolsvoice && tempguy.family==eePOLSV)
            {
                tempguy.step /= 2;
            }
            
            if(!p_igetw(&(tempguy.homing),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&(tempguy.grumble),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&(tempguy.item_set),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(guyversion>=22) // Version 22: Expand misc attributes to 32 bits
            {
                if(!p_igetl(&(tempguy.misc1),f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_igetl(&(tempguy.misc2),f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_igetl(&(tempguy.misc3),f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_igetl(&(tempguy.misc4),f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_igetl(&(tempguy.misc5),f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_igetl(&(tempguy.misc6),f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_igetl(&(tempguy.misc7),f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_igetl(&(tempguy.misc8),f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_igetl(&(tempguy.misc9),f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_igetl(&(tempguy.misc10),f,keepdata))
                {
                    return qe_invalid;
                }
            }
            else
            {
                short tempMisc;
                
                if(!p_igetw(&tempMisc,f,keepdata))
                {
                    return qe_invalid;
                }
                
                tempguy.misc1=tempMisc;
                
                if(!p_igetw(&tempMisc,f,keepdata))
                {
                    return qe_invalid;
                }
                
                tempguy.misc2=tempMisc;
                
                if(!p_igetw(&tempMisc,f,keepdata))
                {
                    return qe_invalid;
                }
                
                tempguy.misc3=tempMisc;
                
                if(!p_igetw(&tempMisc,f,keepdata))
                {
                    return qe_invalid;
                }
                
                tempguy.misc4=tempMisc;
                
                if(!p_igetw(&tempMisc,f,keepdata))
                {
                    return qe_invalid;
                }
                
                tempguy.misc5=tempMisc;
                
                if(guyversion < 13)  // April 2009 - a tiny Wizzrobe update
                {
                    if(tempguy.family == eeWIZZ && !(tempguy.misc1))
                        tempguy.misc5 = 74;
                }
                
                if(!p_igetw(&tempMisc,f,keepdata))
                {
                    return qe_invalid;
                }
                
                tempguy.misc6=tempMisc;
                
                if(!p_igetw(&tempMisc,f,keepdata))
                {
                    return qe_invalid;
                }
                
                tempguy.misc7=tempMisc;
                
                if(!p_igetw(&tempMisc,f,keepdata))
                {
                    return qe_invalid;
                }
                
                tempguy.misc8=tempMisc;
                
                if(!p_igetw(&tempMisc,f,keepdata))
                {
                    return qe_invalid;
                }
                
                tempguy.misc9=tempMisc;
                
                if(!p_igetw(&tempMisc,f,keepdata))
                {
                    return qe_invalid;
                }
                
                tempguy.misc10=tempMisc;
            }
            
            if(!p_igetw(&(tempguy.bgsfx),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&(tempguy.bosspal),f,keepdata))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&(tempguy.extend),f,keepdata))
            {
                return qe_invalid;
            }
            
	    //! Enemy Defences
	    
	    //If a 2.50 quest, use only the 2.5 defences. 
            if(guyversion >= 16 )  // November 2009 - Super Enemy Editor
            {
                for(int j=0; j<edefLAST; j++)
                {
                    if(!p_getc(&(tempguy.defense[j]),f,keepdata))
                    {
                        return qe_invalid;
                    }
                }
		//then copy the generic script defence to all the new script defences
		
            }
	    
	    
	    
            
            if(guyversion >= 18)
            {
                if(!p_getc(&(tempguy.hitsfx),f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_getc(&(tempguy.deadsfx),f,keepdata))
                {
                    return qe_invalid;
                }
            }
            
            if(guyversion >= 22)
            {
                if(!p_igetl(&(tempguy.misc11),f,keepdata))
                {
                    return qe_invalid;
                }
                
                if(!p_igetl(&(tempguy.misc12),f,keepdata))
                {
                    return qe_invalid;
                }
            }
            else if(guyversion >= 19)
            {
                short tempMisc;
                
                if(!p_igetw(&tempMisc,f,keepdata))
                {
                    return qe_invalid;
                }
                
                tempguy.misc11=tempMisc;
                
                if(!p_igetw(&tempMisc,f,keepdata))
                {
                    return qe_invalid;
                }
                
                tempguy.misc12=tempMisc;
            }
	    
	    //If a 2.54 or later quest, use all of the defences. 
	    if(guyversion > 24) // Add new guyversion conditional statement 
            {
		for(int j=edefLAST; j<edefLAST255; j++)
                {
                    if(!p_getc(&(tempguy.defense[j]),f,keepdata))
                    {
                        return qe_invalid;
                    }
                }
            }
	    
	    if(guyversion <= 24) // Port over generic script settings from old quests in the new editor. 
            {
		for(int j=edefSCRIPT01; j<=edefSCRIPT10; j++)
                {
                    tempguy.defense[j] = tempguy.defense[edefSCRIPT] ;
                }
            }
	    
	    //tilewidth, tileheight, hitwidth, hitheight, hitzheight, hitxofs, hityofs, hitzofs
	    if(guyversion > 25)
	    {
		    if(!p_igetl(&(tempguy.txsz),f,keepdata))
                    {
                        return qe_invalid;
                    }
		    if(!p_igetl(&(tempguy.tysz),f,keepdata))
                    {
                        return qe_invalid;
                    }
		    if(!p_igetl(&(tempguy.hxsz),f,keepdata))
                    {
                        return qe_invalid;
                    }
		    if(!p_igetl(&(tempguy.hysz),f,keepdata))
                    {
                        return qe_invalid;
                    }
		    if(!p_igetl(&(tempguy.hzsz),f,keepdata))
                    {
                        return qe_invalid;
                    }
		    /* Is it safe to read a fixed with getl, or do I need to typecast it? -Z
		   
		    */
	    }
	    //More Enemy Editor vars for 2.60
	    if(guyversion > 26)
	    {
		    if(!p_igetl(&(tempguy.hxofs),f,keepdata))
                    {
                        return qe_invalid;
                    }
		    if(!p_igetl(&(tempguy.hyofs),f,keepdata))
                    {
                        return qe_invalid;
                    }
		    if(!p_igetl(&(tempguy.xofs),f,keepdata))
                    {
                        return qe_invalid;
                    }
		    if(!p_igetl(&(tempguy.yofs),f,keepdata))
                    {
                        return qe_invalid;
                    }
		    if(!p_igetl(&(tempguy.zofs),f,keepdata))
                    {
                        return qe_invalid;
                    }
	    }
	    
	    if(guyversion <= 27) // Port over generic script settings from old quests in the new editor. 
            {
		tempguy.wpnsprite = 0;
            }
	    
	    if(guyversion > 27)
	    {
	        if(!p_igetl(&(tempguy.wpnsprite),f,keepdata))
                    {
                        return qe_invalid;
                    }
	    }
	    if(guyversion <= 28) // Port over generic script settings from old quests in the new editor. 
            {
		tempguy.SIZEflags = 0;
            }
	    if(guyversion > 28)
	    {
		if(!p_igetl(&(tempguy.SIZEflags),f,keepdata))
		    {
			return qe_invalid;
		    }
		
	    }
	    if(guyversion < 30) // Port over generic script settings from old quests in the new editor. 
            {
		tempguy.frozentile = 0;
		tempguy.frozencset = 0;
		tempguy.frozenclock = 0;
		for ( int q = 0; q < 10; q++ ) tempguy.frozenmisc[q] = 0;
            }
	    if(guyversion >= 30)
	    {
		if(!p_igetl(&(tempguy.frozentile),f,keepdata))
		{
			return qe_invalid;
		}  
		if(!p_igetl(&(tempguy.frozencset),f,keepdata))
		{
			return qe_invalid;
		}  
		if(!p_igetl(&(tempguy.frozenclock),f,keepdata))
		{
			return qe_invalid;
		}  
		for ( int q = 0; q < 10; q++ ) {
			if(!p_igetw(&(tempguy.frozenmisc[q]),f,keepdata))
			{
				return qe_invalid;
			}
		}
		
	    }
	    
	    if(guyversion >= 34)
	    {
		if(!p_igetw(&(tempguy.firesfx),f,keepdata))
		{
			return qe_invalid;
		}  
		if(!p_igetl(&(tempguy.misc16),f,keepdata))
		{
			return qe_invalid;
		}  
		if(!p_igetl(&(tempguy.misc17),f,keepdata))
		{
			return qe_invalid;
		} 
		if(!p_igetl(&(tempguy.misc18),f,keepdata))
		{
			return qe_invalid;
		} 
		if(!p_igetl(&(tempguy.misc19),f,keepdata))
		{
			return qe_invalid;
		} 
		if(!p_igetl(&(tempguy.misc20),f,keepdata))
		{
			return qe_invalid;
		} 
		if(!p_igetl(&(tempguy.misc21),f,keepdata))
		{
			return qe_invalid;
		} 
		if(!p_igetl(&(tempguy.misc22),f,keepdata))
		{
			return qe_invalid;
		} 
		if(!p_igetl(&(tempguy.misc23),f,keepdata))
		{
			return qe_invalid;
		} 
		if(!p_igetl(&(tempguy.misc24),f,keepdata))
		{
			return qe_invalid;
		} 
		if(!p_igetl(&(tempguy.misc25),f,keepdata))
		{
			return qe_invalid;
		} 
		if(!p_igetl(&(tempguy.misc26),f,keepdata))
		{
			return qe_invalid;
		} 
		if(!p_igetl(&(tempguy.misc27),f,keepdata))
		{
			return qe_invalid;
		} 
		if(!p_igetl(&(tempguy.misc28),f,keepdata))
		{
			return qe_invalid;
		} 
		if(!p_igetl(&(tempguy.misc29),f,keepdata))
		{
			return qe_invalid;
		} 
		if(!p_igetl(&(tempguy.misc30),f,keepdata))
		{
			return qe_invalid;
		} 
		if(!p_igetl(&(tempguy.misc31),f,keepdata))
		{
			return qe_invalid;
		} 
		if(!p_igetl(&(tempguy.misc32),f,keepdata))
		{
			return qe_invalid;
		} 
		
		for ( int q = 0; q < 32; q++ ) {
			if(!p_igetl(&(tempguy.movement[q]),f,keepdata))
			{
				return qe_invalid;
			}
		}
		for ( int q = 0; q < 32; q++ ) {
			if(!p_igetl(&(tempguy.new_weapon[q]),f,keepdata))
			{
				return qe_invalid;
			}
		}
		if(!p_igetw(&(tempguy.npcscript),f,keepdata))
		{
			return qe_invalid;
		} 
		for ( int q = 0; q < 8; q++ )
		{
			if(!p_igetl(&(tempguy.initD[q]),f,keepdata))
			{
				return qe_invalid;
			} 			
		}
		for ( int q = 0; q < 2; q++ )
		{
			if(!p_igetl(&(tempguy.initA[q]),f,keepdata))
			{
				return qe_invalid;
			} 			
		}
		
	    }
	    
	    
	    //default weapon sprites (quest version < 2.54)
	    //port over old defaults -Z
	    if(guyversion < 32)
	    {
		if ( tempguy.wpnsprite <= 0 )
		{
			switch(tempguy.weapon)
			{
				case wNone:
					tempguy.wpnsprite = 0; break;
				
				case wSword:
				case wBeam:
				case wBrang:
				case wBomb:
				case wSBomb:
				case wLitBomb:
				case wLitSBomb:
				case wArrow:
				case wFire:
				case wWhistle:
				case wBait:
				case wWand:
				case wMagic:
				case wCatching:
				case wWind:
				case wRefMagic:
				case wRefFireball:
				case wRefRock:
				case wHammer:
				case wHookshot:
				case wHSHandle:
				case wHSChain:
				case wSSparkle:
				case wFSparkle:
				case wSmack:
				case wPhantom:
				case wCByrna:
				case wRefBeam:
				case wStomp:
				case lwMax:
				case wScript1:
				case wScript2:
				case wScript3:
				case wScript4:
				case wScript5:
				case wScript6:
				case wScript7:
				case wScript8:
				case wScript9:
				case wScript10:
				case wIce:
					//Cannot use any of these weapons yet. 
					tempguy.wpnsprite = -1;
					break;
				
				case wEnemyWeapons:
				case ewFireball: tempguy.wpnsprite = 17; break;
				
				case ewArrow: tempguy.wpnsprite = 19; break;
				case ewBrang: tempguy.wpnsprite = 4; break;
				case ewSword: tempguy.wpnsprite = 20; break;
				case ewRock: tempguy.wpnsprite = 18; break;
				case ewMagic: tempguy.wpnsprite = 21; break;
				case ewBomb: tempguy.wpnsprite = 78; break;
				case ewSBomb: tempguy.wpnsprite = 79; break;
				case ewLitBomb: tempguy.wpnsprite = 76; break;
				case ewLitSBomb: tempguy.wpnsprite = 77; break;
				case ewFireTrail: tempguy.wpnsprite = 80; break;
				case ewFlame: tempguy.wpnsprite = 35; break;
				case ewWind: tempguy.wpnsprite = 36; break;
				case ewFlame2: tempguy.wpnsprite = 81; break;
				case ewFlame2Trail: tempguy.wpnsprite = 82; break;
				case ewIce: tempguy.wpnsprite = 83; break;
				case ewFireball2: tempguy.wpnsprite = 17; break; //fireball (rising)
				
					
				default: break; //No assign.
			}
		}
	    }
	    
	    //default weapon fire sound (quest version < 2.54)
	    //port over old defaults and zero new data. -Z
	    if(guyversion < 34)
	    {
		for ( int q = 0; q < 32; q++ )
		{
			tempguy.movement[q] = 0;
			tempguy.new_weapon[q] = 0;
			
		}
		
		//NPC Script attributes.
		tempguy.npcscript = 0; //No scripted enemies existed. -Z
		for ( int q = 0; q < 8; q++ ) tempguy.initD[q] = 0; //Script Data
		for ( int q = 0; q < 2; q++ ) tempguy.initA[q] = 0; //Script Data
		
		tempguy.misc16 = 0;
		tempguy.misc17 = 0;
		tempguy.misc18 = 0;
		tempguy.misc19 = 0;
		tempguy.misc20 = 0;
		tempguy.misc21 = 0;
		tempguy.misc22 = 0;
		tempguy.misc23 = 0;
		tempguy.misc24 = 0;
		tempguy.misc25 = 0;
		tempguy.misc26 = 0;
		tempguy.misc27 = 0;
		tempguy.misc28 = 0;
		tempguy.misc29 = 0;
		tempguy.misc30 = 0;
		tempguy.misc31 = 0;
		tempguy.misc32 = 0;

		//old default sounds
		if ( tempguy.firesfx <= 0 )
		{
			switch(tempguy.weapon)
			{
				case wNone:
					tempguy.firesfx = 0; break;
				
				case wSword:
				case wBeam:
				case wBrang:
				case wBomb:
				case wSBomb:
				case wLitBomb:
				case wLitSBomb:
				case wArrow:
				case wFire:
				case wWhistle:
				case wBait:
				case wWand:
				case wMagic:
				case wCatching:
				case wWind:
				case wRefMagic:
				case wRefFireball:
				case wRefRock:
				case wHammer:
				case wHookshot:
				case wHSHandle:
				case wHSChain:
				case wSSparkle:
				case wFSparkle:
				case wSmack:
				case wPhantom:
				case wCByrna:
				case wRefBeam:
				case wStomp:
				case lwMax:
				case wScript1:
				case wScript2:
				case wScript3:
				case wScript4:
				case wScript5:
				case wScript6:
				case wScript7:
				case wScript8:
				case wScript9:
				case wScript10:
				case wIce:
					//Cannot use any of these weapons yet. 
					tempguy.firesfx = -1;
					break;
				
				case wEnemyWeapons:
				case ewFireball: tempguy.firesfx = 40; break;
				
				case ewArrow: tempguy.firesfx = 1; break; //Ghost.zh has 0?
				case ewBrang: tempguy.firesfx = 4; break; //Ghost.zh has 0?
				case ewSword: tempguy.firesfx = 20; break; //Ghost.zh has 0?
				case ewRock: tempguy.firesfx = 51; break;
				case ewMagic: tempguy.firesfx = 32; break;
				case ewBomb: tempguy.firesfx = 3; break; //Ghost.zh has 0?
				case ewSBomb: tempguy.firesfx = 3; break; //Ghost.zh has 0?
				case ewLitBomb: tempguy.firesfx = 21; break; //Ghost.zh has 0?
				case ewLitSBomb: tempguy.firesfx = 21; break; //Ghost.zh has 0?
				case ewFireTrail: tempguy.firesfx = 13; break;
				case ewFlame: tempguy.firesfx = 13; break;
				case ewWind: tempguy.firesfx = 32; break;
				case ewFlame2: tempguy.firesfx = 13; break;
				case ewFlame2Trail: tempguy.firesfx = 13; break;
				case ewIce: tempguy.firesfx = 44; break;
				case ewFireball2: tempguy.firesfx = 40; break; //fireball (rising)
				
				//what about special attacks (e.g. summoning == 56)
				default: break; //No assign.
			}
		}
	    }
	    
	    //Port hardcoded hit sound to the enemy hitsfx defaults for older quests. 
	    if(Header->zelda_version <= 0x250 || ( Header->zelda_version > 0x250 && guyversion < 35 ))
	    {
		    if ( tempguy.hitsfx == 0 ) tempguy.hitsfx = 11;
	    }
	 
		
	    
            //miscellaneous other corrections
            //fix the mirror wizzrobe -DD
            if(guyversion < 7)
            {
                if(i == eMWIZ)
                {
                    tempguy.misc2 = 0;
                    tempguy.misc4 = 1;
                }
            }
            
            if(guyversion < 8)
            {
                if(i == eGLEEOK1 || i == eGLEEOK2 || i == eGLEEOK3 || i == eGLEEOK4 || i == eGLEEOK1F || i == eGLEEOK2F || i == eGLEEOK3F || i == eGLEEOK4F)
                {
                    // Some of these are deliberately different to NewDefault/defdata.cpp, by the way. -L
                    tempguy.misc5 = 4; //neck length in segments
                    tempguy.misc6 = 8; //neck offset from first body tile
                    tempguy.misc7 = 40; //offset for each subsequent neck tile from the first neck tile
                    tempguy.misc8 = 168; //head offset from first body tile
                    tempguy.misc9 = 228; //flying head offset from first body tile
                    
                    if(i == eGLEEOK1F || i == eGLEEOK2F || i == eGLEEOK3F || i == eGLEEOK4F)
                    {
                        tempguy.misc6 += 10; //neck offset from first body tile
                        tempguy.misc8 -= 12; //head offset from first body tile
                    }
                }
            }
            
            if(guyversion < 10) // December 2007 - Dodongo CSet fix
            {
                if(get_bit(deprecated_rules,46) && tempguy.family==eeDONGO && tempguy.misc1==0)
                    tempguy.bosspal = spDIG;
            }
            
            if(guyversion < 11) // December 2007 - Spinning Tile fix
            {
                if(tempguy.family==eeSPINTILE)
                {
                    tempguy.flags |= guy_superman;
                    tempguy.item_set = 0; // Don't drop items
                    tempguy.step = 300;
                }
            }
            
            if(guyversion < 12) // October 2008 - Flashing Bubble, Rope 2, and Ghini 2 fix
            {
                if(get_bit(deprecated_rules, qr_NOROPE2FLASH_DEP))
                {
                    if(tempguy.family==eeROPE)
                    {
                        tempguy.flags2 &= ~guy_flashing;
                    }
                }
                
                if(get_bit(deprecated_rules, qr_NOBUBBLEFLASH_DEP))
                {
                    if(tempguy.family==eeBUBBLE)
                    {
                        tempguy.flags2 &= ~guy_flashing;
                    }
                }
                
                if((tempguy.family==eeGHINI)&&(tempguy.misc1))
                {
                    if(get_bit(deprecated_rules, qr_GHINI2BLINK_DEP))
                    {
                        tempguy.flags2 |= guy_blinking;
                    }
                    
                    if(get_bit(deprecated_rules, qr_PHANTOMGHINI2_DEP))
                    {
                        tempguy.flags2 |= guy_transparent;
                    }
                }
            }
            
            if(guyversion < 15) // July 2009 - Guy Fire and Fairy fix
            {
                if(i==gFIRE)
                {
                    tempguy.e_anim = aFLIP;
                    tempguy.e_frate = 24;
                }
                
                if(i==gFAIRY)
                {
                    tempguy.e_anim = a2FRM;
                    tempguy.e_frate = 16;
                }
            }
            
            if(guyversion < 16)  // November 2009 - Super Enemy Editor part 1
            {
                if(i==0) Z_message("Updating guys to version 16...\n");
                
                update_guy_1(&tempguy);
                
                if(i==eMPOLSV)
                {
                    tempguy.defense[edefARROW] = edCHINK;
                    tempguy.defense[edefMAGIC] = ed1HKO;
                    tempguy.defense[edefREFMAGIC] = ed1HKO;
                }
            }
            
            if(guyversion < 17)  // December 2009
            {
                if(tempguy.family==eePROJECTILE)
                {
                    tempguy.misc1 = 0;
                }
            }
            
            if(guyversion < 18)  // January 2010
            {
                bool boss = (tempguy.family == eeAQUA || tempguy.family==eeDONGO || tempguy.family == eeMANHAN || tempguy.family == eeGHOMA || tempguy.family==eeDIG
                             || tempguy.family == eeGLEEOK || tempguy.family==eePATRA || tempguy.family == eeGANON || tempguy.family==eeMOLD);
                             
                tempguy.hitsfx = (boss && tempguy.family != eeMOLD && tempguy.family != eeDONGO && tempguy.family != eeDIG) ? WAV_GASP : 0;
                tempguy.deadsfx = (boss && (tempguy.family != eeDIG || tempguy.misc10 == 0)) ? WAV_GASP : WAV_EDEAD;
                
                if(tempguy.family == eeAQUA)
                    for(int j=0; j<edefLAST; j++) tempguy.defense[j] = default_guys[eRAQUAM].defense[j];
                else if(tempguy.family == eeMANHAN)
                    for(int j=0; j<edefLAST; j++) tempguy.defense[j] = default_guys[eMANHAN].defense[j];
                else if(tempguy.family==eePATRA)
                    for(int j=0; j<edefLAST; j++) tempguy.defense[j] = default_guys[eGLEEOK1].defense[j];
                else if(tempguy.family==eeGHOMA)
                {
                    for(int j=0; j<edefLAST; j++)
                        tempguy.defense[j] = default_guys[eGOHMA1].defense[j];
                        
                    tempguy.defense[edefARROW] = ((tempguy.misc1==3) ? edCHINKL8 : (tempguy.misc1==2) ? edCHINKL4 : 0);
                    
                    if(tempguy.misc1==3 && !tempguy.weapon) tempguy.weapon = ewFlame;
                    
                    tempguy.misc1--;
                }
                else if(tempguy.family == eeGLEEOK)
                {
                    for(int j=0; j<edefLAST; j++)
                        tempguy.defense[j] = default_guys[eGLEEOK1].defense[j];
                        
                    if(tempguy.misc3==1 && !tempguy.weapon) tempguy.weapon = ewFlame;
                }
                else if(tempguy.family == eeARMOS)
                {
                    tempguy.family=eeWALK;
                    tempguy.hrate = 0;
                    tempguy.misc10 = tempguy.misc1;
                    tempguy.misc1 = tempguy.misc2 = tempguy.misc3 = tempguy.misc4 = tempguy.misc5 = tempguy.misc6 = tempguy.misc7 = tempguy.misc8 = 0;
                    tempguy.misc9 = e9tARMOS;
                }
                else if(tempguy.family == eeGHINI && !tempguy.misc1)
                {
                    tempguy.family=eeWALK;
                    tempguy.hrate = 0;
                    tempguy.misc1 = tempguy.misc2 = tempguy.misc3 = tempguy.misc4 = tempguy.misc5 = tempguy.misc6 =
                                                        tempguy.misc7 = tempguy.misc8 = tempguy.misc9 = tempguy.misc10 = 0;
                }
                
                // Spawn animation flags
                if(tempguy.family == eeWALK && (tempguy.flags2&cmbflag_armos || tempguy.flags2&cmbflag_ghini))
                    tempguy.flags |= guy_fadeflicker;
                else
                    tempguy.flags &= 0x0F00000F; // Get rid of the unused flags!
            }
            
            if(guyversion < 20)  // April 2010
            {
                if(tempguy.family == eeTRAP)
                {
                    tempguy.misc2 = tempguy.misc10;
                    
                    if(tempguy.misc10>=1)
                    {
                        tempguy.misc1++;
                    }
                    
                    tempguy.misc10 = 0;
                }
                
                // Bomb Blast fix
                if(tempguy.weapon==ewBomb && tempguy.family!=eeROPE && (tempguy.family!=eeWALK || tempguy.misc2 != e2tBOMBCHU))
                    tempguy.weapon = ewLitBomb;
                else if(tempguy.weapon==ewSBomb && tempguy.family!=eeROPE && (tempguy.family!=eeWALK || tempguy.misc2 != e2tBOMBCHU))
                    tempguy.weapon = ewLitSBomb;
            }
            
            if(guyversion < 21)  // September 2011
            {
                if(tempguy.family == eeKEESE || tempguy.family == eeKEESETRIB)
                {
                    if(tempguy.family == eeKEESETRIB)
                    {
                        tempguy.family = eeKEESE;
                        tempguy.misc2 = e2tKEESETRIB;
                        tempguy.misc1 = 0;
                    }
                    
                    tempguy.rate = 2;
                    tempguy.hrate = 8;
                    tempguy.homing = 0;
                    tempguy.step= (tempguy.family == eeKEESE && tempguy.misc1 ? 100:62);
                }
                else if(tempguy.family == eePEAHAT || tempguy.family==eePATRA)
                {
                    if(tempguy.family == eePEAHAT)
                    {
                        tempguy.rate = 4;
                        tempguy.step = 62;
                    }
                    else
                        tempguy.step = 25;
                        
                    tempguy.hrate = 8;
                    tempguy.homing = 0;
                }
                else if(tempguy.family == eeDIG || tempguy.family == eeMANHAN)
                {
                    if(tempguy.family == eeMANHAN)
                        tempguy.step=50;
                        
                    tempguy.hrate = 16;
                    tempguy.homing = 0;
                }
                else if(tempguy.family == eeGLEEOK)
                {
                    tempguy.rate = 2;
                    tempguy.homing = 0;
                    tempguy.hrate = 9;
                    tempguy.step=89;
                }
                else if(tempguy.family == eeGHINI)
                {
                    tempguy.rate = 4;
                    tempguy.hrate = 12;
                    tempguy.step=62;
                    tempguy.homing = 0;
                }
                
                // Bigdig random rate fix
                if(tempguy.family==eeDIG && tempguy.misc10==1)
                {
                    tempguy.rate = 2;
                }
            }
            
            if(guyversion < 24) // November 2012
            {
                if(tempguy.family==eeLANM)
                    tempguy.misc3 = 1;
                else if(tempguy.family==eeMOLD)
                    tempguy.misc2 = 0;
            }
	    
	    if(guyversion < 33) //Whistle defence did not exist before this version of 2.54. -Z
	    {
		if(tempguy.family!=eeDIG)
		{
			tempguy.defense[edefWhistle] = edIGNORE; //Might need to be ignore, universally. 
		}
			
	    }
            
            if(keepdata)
            {
                guysbuf[i] = tempguy;
            }
        }
    }
    
    return 0;
}

void update_guy_1(guydata *tempguy) // November 2009
{
    bool doesntcount = false;
    tempguy->flags &= ~weak_arrow; // Formerly 'weak to arrow' which wasn't implemented
    
    switch(tempguy->family)
    {
    case 1: //eeWALK
        switch(tempguy->misc10)
        {
        case 0: //Stalfos
            if(tempguy->misc1==1)  // Fires four projectiles at once
                tempguy->misc1=4;
                
            break;
            
        case 1: //Darknut
            goto darknuts;
            break;
        }
        
        tempguy->misc10 = 0;
        break;
        
    case 2: //eeSHOOT
        tempguy->family = eeWALK;
        
        switch(tempguy->misc10)
        {
        case 0: //Octorok
            if(tempguy->misc1==1||tempguy->misc1==2)
            {
                tempguy->misc1=e1tFIREOCTO;
                tempguy->misc2=e2tFIREOCTO;
            }
            else tempguy->misc1 = 0;
            
            tempguy->misc6=tempguy->misc4;
            tempguy->misc4=tempguy->misc3;
            tempguy->misc3=0;
            break;
            
        case 1: // Moblin
            tempguy->misc1 = 0;
            break;
            
        case 2: //Lynel
            tempguy->misc6=tempguy->misc1+1;
            tempguy->misc1=0;
            break;
            
        case 3: //Stalfos 2
            if(tempguy->misc1==1)  // Fires four projectiles at once
                tempguy->misc1=e1t4SHOTS;
            else tempguy->misc1 = 0;
            
            break;
            
        case 4: //Darknut 5
darknuts:
            tempguy->defense[edefFIRE] = edIGNORE;
            tempguy->defense[edefBRANG] = edSTUNORCHINK;
            tempguy->defense[edefHOOKSHOT] = 0;
            tempguy->defense[edefARROW] = tempguy->defense[edefBYRNA] = tempguy->defense[edefREFROCK] =
                                              tempguy->defense[edefMAGIC] = tempguy->defense[edefSTOMP] = edCHINK;
                                              
            if(tempguy->misc1==1)
                tempguy->misc1=2;
            else if(tempguy->misc1==2)
            {
                tempguy->misc4=tempguy->misc3;
                tempguy->misc3=tempguy->misc2;
                tempguy->misc2=e2tSPLIT;
                tempguy->misc1 = 0;
            }
            else tempguy->misc1 = 0;
            
            tempguy->flags |= inv_front;
            
            if(get_bit(deprecated_rules,qr_BRKBLSHLDS_DEP))
                tempguy->flags |= guy_bkshield;
                
            break;
        }
        
        tempguy->misc10 = 0;
        break;
        
        /*
        		case 9: //eeARMOS
        		tempguy->family = eeWALK;
        		break;
        */
    case 11: //eeGEL
    case 33: //eeGELTRIB
        if(tempguy->family==33)
        {
            tempguy->misc4 = 1;
            
            if(get_bit(deprecated_rules, qr_OLDTRIBBLES_DEP))  //Old Tribbles
                tempguy->misc3 = tempguy->misc2;
                
            tempguy->misc2 = e2tTRIBBLE;
        }
        else
        {
            tempguy->misc4 = 0;
            tempguy->misc3 = 0;
            tempguy->misc2 = 0;
        }
        
        tempguy->family = eeWALK;
        
        if(tempguy->misc1)
        {
            tempguy->misc1=1;
            tempguy->weapon = ewFireTrail;
        }
        
        break;
        
    case 34: //eeZOLTRIB
    case 12: //eeZOL
        tempguy->misc4=tempguy->misc3;
        tempguy->misc3=tempguy->misc2;
        tempguy->family = eeWALK;
        tempguy->misc2=e2tSPLITHIT;
        
        if(tempguy->misc1)
        {
            tempguy->misc1=1;
            tempguy->weapon = ewFireTrail;
        }
        
        break;
        
    case 13: //eeROPE
        tempguy->family = eeWALK;
        tempguy->misc9 = e9tROPE;
        
        if(tempguy->misc1)
        {
            tempguy->misc4 = tempguy->misc3;
            tempguy->misc3 = tempguy->misc2;
            tempguy->misc2 = e2tBOMBCHU;
        }
        
        tempguy->misc1 = 0;
        break;
        
    case 14: //eeGORIYA
        tempguy->family = eeWALK;
        
        if(tempguy->misc1!=2) tempguy->misc1 = 0;
        
        break;
        
    case 17: //eeBUBBLE
        tempguy->family = eeWALK;
        tempguy->misc8 = tempguy->misc2;
        tempguy->misc7 = tempguy->misc1 + 1;
        tempguy->misc1 = tempguy->misc2 = 0;
        
        //fallthrogh
    case eeTRAP:
    case eeROCK:
        doesntcount = true;
        break;
        
    case 35: //eeVIRETRIB
    case 18: //eeVIRE
        tempguy->family = eeWALK;
        tempguy->misc4=tempguy->misc3;
        tempguy->misc3=tempguy->misc2;
        tempguy->misc2=e2tSPLITHIT;
        tempguy->misc9=e9tVIRE;
        break;
        
    case 19: //eeLIKE
        tempguy->family = eeWALK;
        tempguy->misc7 = e7tEATITEMS;
        tempguy->misc8=95;
        break;
        
    case 20: //eePOLSV
        tempguy->defense[edefBRANG] = edSTUNORCHINK;
        tempguy->defense[edefBOMB] = tempguy->defense[edefSBOMB] = tempguy->defense[edefFIRE] = edIGNORE;
        tempguy->defense[edefMAGIC] = tempguy->defense[edefBYRNA] = edCHINK;
        tempguy->defense[edefARROW] = ed1HKO;
        tempguy->defense[edefHOOKSHOT] = edSTUNONLY;
        tempguy->family = eeWALK;
        tempguy->misc9 = e9tPOLSVOICE;
        tempguy->rate = 4;
        tempguy->homing = 32;
        tempguy->hrate = 10;
        tempguy->grumble = 0;
        break;
        
    case eeWIZZ:
        if(tempguy->misc4)
        {
            for(int i=0; i < edefLAST; i++)
                tempguy->defense[i] = (i != edefREFBEAM && i != edefREFMAGIC && i != edefQUAKE) ? edIGNORE : 0;
        }
        else
        {
            tempguy->defense[edefBRANG] = edSTUNORCHINK;
            tempguy->defense[edefMAGIC] = edCHINK;
            tempguy->defense[edefHOOKSHOT] = edSTUNONLY;
            tempguy->defense[edefARROW] = tempguy->defense[edefFIRE] =
                                              tempguy->defense[edefWAND] = tempguy->defense[edefBYRNA] = edIGNORE;
        }
        
        break;
        
    case eePEAHAT:
        tempguy->flags &= ~(guy_superman|guy_sbombonly);
        
        if(!(tempguy->flags & guy_bhit))
            tempguy->defense[edefBRANG] = edSTUNONLY;
            
        break;
        
    case eeLEV:
        tempguy->defense[edefSTOMP] = edCHINK;
        break;
    }
    
    // Old flags
    if(tempguy->flags & guy_superman)
    {
        for(int i = 0; i < edefLAST; i++)
            if(!(i==edefSBOMB && (tempguy->flags & guy_sbombonly)))
                tempguy->defense[i] = (i==edefBRANG && tempguy->defense[i] != edIGNORE
                                       && tempguy->family != eeROCK && tempguy->family != eeTRAP
                                       && tempguy->family != eePROJECTILE) ? edSTUNORIGNORE : edIGNORE;
    }
    
    tempguy->flags &= ~(guy_superman|guy_sbombonly);
    
    if(doesntcount)
        tempguy->flags |= (guy_doesntcount);
}


int readmapscreen(PACKFILE *f, zquestheader *Header, mapscr *temp_mapscr, zcmap *temp_map, word version)
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
    
    if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<146)))
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
    
    if(Header->zelda_version < 0x211 || (Header->zelda_version == 0x211 && Header->build < 14))
    {
        temp_mapscr->hasitem = (temp_mapscr->item != 0) ? 1 : 0;
    }
    else
    {
        if(!p_getc(&(temp_mapscr->hasitem),f,true))
            return qe_invalid;
    }
    
    if((Header->zelda_version < 0x192)||
            ((Header->zelda_version == 0x192)&&(Header->build<154)))
    {
        if(!p_getc(&tempbyte,f,true))
        {
            return qe_invalid;
        }
    }
    
    if(!p_getc(&(temp_mapscr->tilewarptype[0]),f,true))
    {
        return qe_invalid;
    }
    
    if(Header->zelda_version < 0x193)
    {
        if(!p_getc(&tempbyte,f,true))
        {
            return qe_invalid;
        }
    }
    
    if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
    {
        for(int i=1; i<4; i++)
        {
            if(!p_getc(&(temp_mapscr->tilewarptype[i]),f,true))
            {
                return qe_invalid;
            }
        }
    }
    else
    {
        temp_mapscr->tilewarptype[1]=0;
        temp_mapscr->tilewarptype[2]=0;
        temp_mapscr->tilewarptype[3]=0;
    }
    
    if((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>153)))
    {
        if(!p_igetw(&(temp_mapscr->door_combo_set),f,true))
        {
            return qe_invalid;
        }
    }
    
    if(!p_getc(&(temp_mapscr->warpreturnx[0]),f,true))
    {
        return qe_invalid;
    }
    
    temp_mapscr->warpreturnx[1]=0;
    temp_mapscr->warpreturnx[2]=0;
    temp_mapscr->warpreturnx[3]=0;
    
    if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
    {
        for(int i=1; i<4; i++)
        {
            if(!p_getc(&(temp_mapscr->warpreturnx[i]),f,true))
            {
                return qe_invalid;
            }
        }
    }
    
    if(!p_getc(&(temp_mapscr->warpreturny[0]),f,true))
    {
        return qe_invalid;
    }
    
    temp_mapscr->warpreturny[1]=0;
    temp_mapscr->warpreturny[2]=0;
    temp_mapscr->warpreturny[3]=0;
    
    if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
    {
        for(int i=1; i<4; i++)
        {
            if(!p_getc(&(temp_mapscr->warpreturny[i]),f,true))
            {
                return qe_invalid;
            }
        }
        
        if(version>=18)
        {
            if(!p_igetw(&temp_mapscr->warpreturnc,f,true))
            {
                return qe_invalid;
            }
        }
        else
        {
            byte temp;
            
            if(!p_getc(&temp,f,true))
            {
                return qe_invalid;
            }
            
            temp_mapscr->warpreturnc=temp<<8|temp;
        }
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
    
    if(version > 15) // February 2009
    {
        if(!p_igetw(&(temp_mapscr->color),f,true))
        {
            return qe_invalid;
        }
    }
    else
    {
        if(!p_getc(& tempbyte,f,true))
        {
            return qe_invalid;
        }
        
        temp_mapscr->color = (word) tempbyte;
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
    
    if(version <= 11)
    {
        if(!p_getc(&(tempbyte),f,true))
        {
            return qe_invalid;
        }
        
        temp_mapscr->tilewarpdmap[0]=(word)tempbyte;
        
        if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
        {
            for(int i=1; i<4; i++)
            {
                if(!p_getc(&(tempbyte),f,true))
                {
                    return qe_invalid;
                }
                
                temp_mapscr->tilewarpdmap[i]=(word)tempbyte;
            }
        }
        else
        {
            temp_mapscr->tilewarpdmap[1]=0;
            temp_mapscr->tilewarpdmap[2]=0;
            temp_mapscr->tilewarpdmap[3]=0;
        }
    }
    else
    {
        for(int i=0; i<4; i++)
        {
            if(!p_igetw(&(temp_mapscr->tilewarpdmap[i]),f,true))
            {
                return qe_invalid;
            }
        }
    }
    
    if(!p_getc(&(temp_mapscr->tilewarpscr[0]),f,true))
    {
        return qe_invalid;
    }
    
    if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
    {
        for(int i=1; i<4; i++)
        {
            if(!p_getc(&(temp_mapscr->tilewarpscr[i]),f,true))
            {
                return qe_invalid;
            }
        }
    }
    else
    {
        temp_mapscr->tilewarpscr[1]=0;
        temp_mapscr->tilewarpscr[2]=0;
        temp_mapscr->tilewarpscr[3]=0;
    }
    
    if(version >= 15)
    {
        if(!p_getc(&(temp_mapscr->tilewarpoverlayflags),f,true))
        {
            return qe_invalid;
        }
    }
    else
    {
        temp_mapscr->tilewarpoverlayflags=0;
    }
    
    if(!p_getc(&(temp_mapscr->exitdir),f,true))
    {
        return qe_invalid;
    }
    
    if(Header->zelda_version < 0x193)
    {
        if(!p_getc(&tempbyte,f,true))
        {
            return qe_invalid;
        }
        
    }
    
    if((Header->zelda_version == 0x192)&&(Header->build>145)&&(Header->build<154))
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
        if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<10)))
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
        
        if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<108)))
        {
            //using enumerations here is dangerous
            //very easy to break old quests -DD
            if(temp_mapscr->enemy[k]>=57)  //old eGOHMA1
            {
                temp_mapscr->enemy[k]+=5;
            }
            else if(temp_mapscr->enemy[k]>=52)  //old eGLEEOK2
            {
                temp_mapscr->enemy[k]+=1;
            }
        }
        
        if(version < 9)
        {
            if(temp_mapscr->enemy[k]>0)
            {
                temp_mapscr->enemy[k]+=10;
            }
        }
    }
    
    if(!p_getc(&(temp_mapscr->pattern),f,true))
    {
        return qe_invalid;
    }
    
    if(!p_getc(&(temp_mapscr->sidewarptype[0]),f,true))
    {
        return qe_invalid;
    }
    
    if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
    {
        for(int i=1; i<4; i++)
        {
            if(!p_getc(&(temp_mapscr->sidewarptype[i]),f,true))
            {
                return qe_invalid;
            }
        }
    }
    else
    {
        temp_mapscr->sidewarptype[1]=0;
        temp_mapscr->sidewarptype[2]=0;
        temp_mapscr->sidewarptype[3]=0;
    }
    
    if(version >= 15)
    {
        if(!p_getc(&(temp_mapscr->sidewarpoverlayflags),f,true))
        {
            return qe_invalid;
        }
    }
    else
    {
        temp_mapscr->sidewarpoverlayflags=0;
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
    
    if(!p_getc(&(temp_mapscr->sidewarpscr[0]),f,true))
    {
        return qe_invalid;
    }
    
    if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
    {
        for(int i=1; i<4; i++)
        {
            if(!p_getc(&(temp_mapscr->sidewarpscr[i]),f,true))
            {
                return qe_invalid;
            }
        }
    }
    else
    {
        temp_mapscr->sidewarpscr[1]=0;
        temp_mapscr->sidewarpscr[2]=0;
        temp_mapscr->sidewarpscr[3]=0;
    }
    
    if(version <= 11)
    {
        if(!p_getc(&(tempbyte),f,true))
        {
            return qe_invalid;
        }
        
        temp_mapscr->sidewarpdmap[0]=(word)tempbyte;
        
        if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
        {
            for(int i=1; i<4; i++)
            {
                if(!p_getc(&(tempbyte),f,true))
                {
                    return qe_invalid;
                }
                
                temp_mapscr->sidewarpdmap[i]=(word)tempbyte;
            }
        }
        else
        {
            temp_mapscr->sidewarpdmap[1]=0;
            temp_mapscr->sidewarpdmap[2]=0;
            temp_mapscr->sidewarpdmap[3]=0;
        }
    }
    else
    {
        for(int i=0; i<4; i++)
        {
            if(!p_igetw(&(temp_mapscr->sidewarpdmap[i]),f,true))
            {
                return qe_invalid;
            }
        }
    }
    
    if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
    {
        if(!p_getc(&(temp_mapscr->sidewarpindex),f,true))
        {
            return qe_invalid;
        }
    }
    else temp_mapscr->sidewarpindex = 0;
    
    if(!p_igetw(&(temp_mapscr->undercombo),f,true))
    {
        return qe_invalid;
    }
    
    if(Header->zelda_version < 0x193)
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
    
    if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>1)))
        //if (version>2)
    {
        if(!p_getc(&(temp_mapscr->flags4),f,true))
        {
            return qe_invalid;
        }
    }
    
    if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
    {
        if(!p_getc(&(temp_mapscr->flags5),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_mapscr->noreset),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_mapscr->nocarry),f,true))
        {
            return qe_invalid;
        }
        
        if(temp_mapscr->flags5&32)
        {
            temp_mapscr->flags5 &= ~32;
            temp_mapscr->noreset |= 48;
        }
        
        if(version<8)
        {
            if(temp_mapscr->noreset&1)
            {
                temp_mapscr->noreset|=8192;
            }
            
            if(temp_mapscr->nocarry&1)
            {
                temp_mapscr->nocarry|=8192;
                temp_mapscr->nocarry&=~1;
            }
        }
    }
    else
    {
        temp_mapscr->flags5 = 0;
        temp_mapscr->noreset = 0;
        temp_mapscr->nocarry = 0;
    }
    
    if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>9)))
    {
        if(!p_getc(&(temp_mapscr->flags6),f,true))
        {
            return qe_invalid;
        }
    }
    
    if(version>5)
    {
        if(!p_getc(&(temp_mapscr->flags7),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&(temp_mapscr->flags8),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&(temp_mapscr->flags9),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&(temp_mapscr->flags10),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&(temp_mapscr->csensitive),f,true))
        {
            return qe_invalid;
        }
    }
    else
    {
        temp_mapscr->csensitive=1;
    }
    
    if(version<14) // August 2007: screen SFX added
    {
        if(temp_mapscr->flags&8) //fROAR
        {
            temp_mapscr->bosssfx=
                (temp_mapscr->flags3&2) ? WAV_DODONGO :  // fDODONGO
                (temp_mapscr->flags2&32) ? WAV_VADER : // fVADER
                WAV_ROAR;
        }
        
        if(temp_mapscr->flags&128) //fSEA
        {
            temp_mapscr->oceansfx=WAV_SEA;
        }
        
        if(!(temp_mapscr->flags3&64)) //fNOSECRETSOUND
        {
            temp_mapscr->secretsfx=WAV_SECRET;
        }
        
        temp_mapscr->flags3 &= ~66; //64|2
        temp_mapscr->flags2 &= ~32;
        temp_mapscr->flags &= ~136; // 128|8
    }
    else
    {
        if(!p_getc(&(temp_mapscr->oceansfx),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&(temp_mapscr->bosssfx),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&(temp_mapscr->secretsfx),f,true))
        {
            return qe_invalid;
        }
    }
    
    if(version<15) // October 2007: another SFX
    {
        temp_mapscr->holdupsfx=WAV_PICKUP;
    }
    else
    {
        if(!p_getc(&(temp_mapscr->holdupsfx),f,true))
        {
            return qe_invalid;
        }
    }
    
    
    if((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>97)))
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
    else if((Header->zelda_version == 0x192)&&(Header->build>23)&&(Header->build<98))
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
    
    if((Header->zelda_version == 0x192)&&(Header->build>149))
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
    
    if((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>149)))
    {
        for(int k=0; k<6; k++)
        {
            if(!p_getc(&(temp_mapscr->layeropacity[k]),f,true))
            {
                return qe_invalid;
            }
        }
    }
    
    if((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>153)))
    {
        if((Header->zelda_version == 0x192)&&(Header->build>153))
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
    
    if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<24)))
    {
        extras=15;
    }
    else if(((Header->zelda_version == 0x192)&&(Header->build<98)))
    {
        extras=11;
    }
    else if((Header->zelda_version == 0x192)&&(Header->build<150))
    {
        extras=32;
    }
    else if((Header->zelda_version == 0x192)&&(Header->build<154))
    {
        extras=64;
    }
    else if(Header->zelda_version < 0x193)
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
    
    if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>2)))
        //if (version>3)
    {
        if(!p_getc(&(temp_mapscr->nextmap),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&(temp_mapscr->nextscr),f,true))
        {
            return qe_invalid;
        }
    }
    else
    {
        temp_mapscr->nextmap=0;
        temp_mapscr->nextscr=0;
    }
    
    if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<137)))
    {
        secretcombos=20;
    }
    else if((Header->zelda_version == 0x192)&&(Header->build<154))
    {
        secretcombos=256;
    }
    else
    {
        secretcombos=128;
    }
    
    if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<154)))
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
    
    if((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>153)))
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
    
    if((Header->zelda_version == 0x192)&&(Header->build>97)&&(Header->build<154))
    {
        if(!p_getc(&padding,f,true))
        {
            return qe_invalid;
        }
    }
    
    const int _mapsSize = (temp_map->tileWidth*temp_map->tileHeight);
    
    temp_mapscr->data.resize(_mapsSize, 0);
    temp_mapscr->sflag.resize(_mapsSize, 0);
    temp_mapscr->cset.resize(_mapsSize, 0);
    
    for(int k=0; k<(temp_map->tileWidth*temp_map->tileHeight); k++)
    {
        if(!p_igetw(&(temp_mapscr->data[k]),f,true))
        {
            return qe_invalid;
        }
    }
    
    if((Header->zelda_version == 0x192)&&(Header->build>20)&&(Header->build<24))
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
    
    if((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>20)))
    {
        for(int k=0; k<(temp_map->tileWidth*temp_map->tileHeight); k++)
        {
            if(!p_getc(&(temp_mapscr->sflag[k]),f,true))
            {
                return qe_invalid;
            }
            
            if((Header->zelda_version == 0x192)&&(Header->build<24))
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
    
    if((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>97)))
    {
        for(int k=0; k<(temp_map->tileWidth*temp_map->tileHeight); k++)
        {
        
            if(!p_getc(&(temp_mapscr->cset[k]),f,true))
            {
                return qe_invalid;
            }
        }
    }
    
    if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<154)))
    {
        temp_mapscr->undercset=(temp_mapscr->undercombo>>8)&7;
        temp_mapscr->undercombo=(temp_mapscr->undercombo&0xFF)+(temp_mapscr->old_cpage<<8);
    }
    
    if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<137)))
    {
        temp_mapscr->secretcombo[sSBOMB]=temp_mapscr->secretcombo[sBOMB];
        temp_mapscr->secretcombo[sRCANDLE]=temp_mapscr->secretcombo[sBCANDLE];
        temp_mapscr->secretcombo[sWANDFIRE]=temp_mapscr->secretcombo[sBCANDLE];
        temp_mapscr->secretcombo[sDINSFIRE]=temp_mapscr->secretcombo[sBCANDLE];
        temp_mapscr->secretcombo[sSARROW]=temp_mapscr->secretcombo[sARROW];
        temp_mapscr->secretcombo[sGARROW]=temp_mapscr->secretcombo[sARROW];
    }
    
    if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<154)))
    {
        for(int k=0; k<(temp_map->tileWidth*temp_map->tileHeight); k++)
        {
            if((Header->zelda_version == 0x192)&&(Header->build>149))
            {
                if((Header->zelda_version == 0x192)&&(Header->build!=153))
                {
                    temp_mapscr->cset[k]=((temp_mapscr->data[k]>>8)&7);
                }
            }
            else
            {
                if((Header->zelda_version < 0x192)||
                        ((Header->zelda_version == 0x192)&&(Header->build<21)))
                {
                    temp_mapscr->sflag[k]=(temp_mapscr->data[k]>>11);
                }
                
                temp_mapscr->cset[k]=((temp_mapscr->data[k]>>8)&7);
            }
            
            temp_mapscr->data[k]=(temp_mapscr->data[k]&0xFF)+(temp_mapscr->old_cpage<<8);
        }
    }
    
    /*if(version>12)
    {
      if(!p_getc(&(temp_mapscr->scrWidth),f,true))
      {
        return qe_invalid;
      }
    if(!p_getc(&(temp_mapscr->scrHeight),f,true))
      {
        return qe_invalid;
      }
    }*/
    
    if(version>4)
    {
        if(!p_igetw(&(temp_mapscr->screen_midi),f,true))
        {
            return qe_invalid;
        }
    }
    else
    {
        temp_mapscr->screen_midi = -1;
    }
    
    if(version>=17)
    {
        if(!p_getc(&(temp_mapscr->lens_layer),f,true))
        {
            return qe_invalid;
        }
    }
    else
    {
        temp_mapscr->lens_layer = llNORMAL;
    }
    
    if(version>6)
    {
        if(!p_igetl(&(temp_mapscr->numff),f,true))
        {
            return qe_invalid;
        }
        
        int m;
        float tempfloat;
        
        for(m=0; m<MAXFFCS; m++)
        {
            if((temp_mapscr->numff>>m)&1)
            {
                if(!p_igetw(&(temp_mapscr->ffdata[m]),f,true))
                {
                    return qe_invalid;
                }
                
                if(!p_getc(&(temp_mapscr->ffcset[m]),f,true))
                {
                    return qe_invalid;
                }
                
                if(!p_igetw(&(temp_mapscr->ffdelay[m]),f,true))
                {
                    return qe_invalid;
                }
                
                if(version < 9)
                {
                    if(!p_igetf(&tempfloat,f,true))
                    {
                        return qe_invalid;
                    }
                    
                    temp_mapscr->ffx[m]=int(tempfloat*10000);
                    
                    if(!p_igetf(&tempfloat,f,true))
                    {
                        return qe_invalid;
                    }
                    
                    temp_mapscr->ffy[m]=int(tempfloat*10000);
                    
                    if(!p_igetf(&tempfloat,f,true))
                    {
                        return qe_invalid;
                    }
                    
                    temp_mapscr->ffxdelta[m]=int(tempfloat*10000);
                    
                    if(!p_igetf(&tempfloat,f,true))
                    {
                        return qe_invalid;
                    }
                    
                    temp_mapscr->ffydelta[m]=int(tempfloat*10000);
                    
                    if(!p_igetf(&tempfloat,f,true))
                    {
                        return qe_invalid;
                    }
                    
                    temp_mapscr->ffxdelta2[m]=int(tempfloat*10000);
                    
                    if(!p_igetf(&tempfloat,f,true))
                    {
                        return qe_invalid;
                    }
                    
                    temp_mapscr->ffydelta2[m]=int(tempfloat*10000);
                }
                else
                {
                    if(!p_igetl(&(temp_mapscr->ffx[m]),f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_igetl(&(temp_mapscr->ffy[m]),f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_igetl(&(temp_mapscr->ffxdelta[m]),f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_igetl(&(temp_mapscr->ffydelta[m]),f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_igetl(&(temp_mapscr->ffxdelta2[m]),f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_igetl(&(temp_mapscr->ffydelta2[m]),f,true))
                    {
                        return qe_invalid;
                    }
                }
                
                if(!p_getc(&(temp_mapscr->fflink[m]),f,true))
                {
                    return qe_invalid;
                }
                
                if(version>7)
                {
                    if(!p_getc(&(temp_mapscr->ffwidth[m]),f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_getc(&(temp_mapscr->ffheight[m]),f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_igetl(&(temp_mapscr->ffflags[m]),f,true))
                    {
                        return qe_invalid;
                    }
                }
                else
                {
                    temp_mapscr->ffwidth[m]=15;
                    temp_mapscr->ffheight[m]=15;
                    temp_mapscr->ffflags[m]=0;
                }
                
                if(Header->zelda_version == 0x211 || (Header->zelda_version == 0x250 && Header->build<20))
                {
                    temp_mapscr->ffflags[m]|=ffIGNOREHOLDUP;
                }
                
                if(version>9)
                {
                    if(!p_igetw(&(temp_mapscr->ffscript[m]),f,true))
                    {
                        return qe_invalid;
                    }
                }
                else
                {
                    temp_mapscr->ffscript[m]=0;
                }
                
                if(version>10)
                {
                    if(!p_igetl(&(temp_mapscr->initd[m][0]),f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_igetl(&(temp_mapscr->initd[m][1]),f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_igetl(&(temp_mapscr->initd[m][2]),f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_igetl(&(temp_mapscr->initd[m][3]),f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_igetl(&(temp_mapscr->initd[m][4]),f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_igetl(&(temp_mapscr->initd[m][5]),f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_igetl(&(temp_mapscr->initd[m][6]),f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_igetl(&(temp_mapscr->initd[m][7]),f,true))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_getc(&(tempbyte),f,true))
                    {
                        return qe_invalid;
                    }
                    
                    temp_mapscr->inita[m][0]=tempbyte*10000;
                    
                    if(!p_getc(&(tempbyte),f,true))
                    {
                        return qe_invalid;
                    }
                    
                    temp_mapscr->inita[m][1]=tempbyte*10000;
                }
                else
                {
                    temp_mapscr->inita[m][0] = 10000;
                    temp_mapscr->inita[m][1] = 10000;
                }
                
                temp_mapscr->initialized[m] = false;
                
                if(version <= 11)
                {
                    fixffcs=true;
                }
            }
            else
            {
                temp_mapscr->ffdata[m]=0;
                temp_mapscr->ffcset[m]=0;
                temp_mapscr->ffdelay[m]=0;
                temp_mapscr->ffx[m]=0;
                temp_mapscr->ffy[m]=0;
                temp_mapscr->ffxdelta[m]=0;
                temp_mapscr->ffydelta[m]=0;
                temp_mapscr->ffxdelta2[m]=0;
                temp_mapscr->ffydelta2[m]=0;
                temp_mapscr->ffdata[m]=0;
                temp_mapscr->ffwidth[m]=15;
                temp_mapscr->ffheight[m]=15;
                temp_mapscr->ffflags[m]=0;
                temp_mapscr->ffscript[m]=0;
                //temp_mapscr->a[m][0] = 10000;
                //temp_mapscr->a[m][1] = 10000;
                temp_mapscr->initialized[m] = false;
            }
        }
    }
    
    //add in the new whistle flags
    if(version<13)
    {
        if(temp_mapscr->flags & fWHISTLE)
        {
            temp_mapscr->flags7 |= (fWHISTLEPAL | fWHISTLEWATER);
        }
    }
    
    for(int m=0; m<MAXFFCS; m++)
    {
        // ffcScriptData used to be part of mapscr, and this was handled just above
        ffcScriptData[m].a[0] = 10000;
        ffcScriptData[m].a[1] = 10000;
    }
    
    if ( version >= 19 )
    {
	for ( int q = 0; q < 10; q++ ) 
	{
		if(!p_igetl(&(temp_mapscr->npcstrings[q]),f,true))
		{
                        return qe_invalid;
		} 
	}
	for ( int q = 0; q < 10; q++ ) 
	{
		if(!p_igetw(&(temp_mapscr->new_items[q]),f,true))
		{
                        return qe_invalid;
		} 
	}
	for ( int q = 0; q < 10; q++ ) 
	{
		if(!p_igetw(&(temp_mapscr->new_item_x[q]),f,true))
		{
                        return qe_invalid;
		} 
	}
	for ( int q = 0; q < 10; q++ ) 
	{
		if(!p_igetw(&(temp_mapscr->new_item_y[q]),f,true))
		{
                        return qe_invalid;
		} 
	}
    }
    
    return 0;
}



int readmaps(PACKFILE *f, zquestheader *Header, bool keepdata)
{

    int scr=0;
    
    word version=0;
    dword dummy;
    int screens_to_read;
    
    mapscr temp_mapscr;
    zcmap temp_map;
    word temp_map_count;
    dword section_size;
    
    if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<137)))
    {
        screens_to_read=MAPSCRS192b136;
    }
    else
    {
        screens_to_read=MAPSCRS;
    }
    
    if(Header->zelda_version > 0x192)
    {
        //section version info
        if(!p_igetw(&version,f,true))
        {
            return qe_invalid;
        }
	
	ffcore.quest_format[vMaps] = version;
        
        //al_trace("Maps version %d\n", version);
        if(!p_igetw(&dummy,f,true))
        {
            return qe_invalid;
        }
        
        //section size
        if(!p_igetl(&section_size,f,true))
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
    
    
    if(keepdata)
    {
        const int _mapsSize = MAPSCRS*temp_map_count;
        TheMaps.resize(_mapsSize);
        
        for(int i(0); i<_mapsSize; i++)
            TheMaps[i].zero_memory();
        
        // Used to be done for each screen
        for(int i=0; i<32; i++)
        {
            ffcScriptData[i].a[0] = 10000;
            ffcScriptData[i].a[1] = 10000;
        }
        
        memset(ZCMaps, 0, sizeof(zcmap)*MAXMAPS2);
    }
    
    temp_mapscr.zero_memory();
    
    for(int i=0; i<temp_map_count && i<MAXMAPS2; i++)
    {
        memset(&temp_map, 0, sizeof(zcmap));
        /*if(version>12)
        {
            if(!p_getc(&(temp_map.tileWidth),f,true))
            {
              return qe_invalid;
            }
          if(!p_getc(&(temp_map.tileHeight),f,true))
            {
              return qe_invalid;
            }
          if(!p_igetw(&(temp_map.subaWidth),f,true))
            {
              return qe_invalid;
            }
          if(!p_igetw(&(temp_map.subaHeight),f,true))
            {
              return qe_invalid;
            }
          if(!p_igetw(&(temp_map.subpWidth),f,true))
            {
              return qe_invalid;
            }
          if(!p_igetw(&(temp_map.subpHeight),f,true))
            {
              return qe_invalid;
            }
          if(!p_igetw(&(temp_map.scrResWidth),f,true))
            {
              return qe_invalid;
            }
          if(!p_igetw(&(temp_map.scrResHeight),f,true))
            {
              return qe_invalid;
            }
          if(!p_igetw(&(temp_map.viewWidth),f,true))
            {
              return qe_invalid;
            }
          if(!p_igetw(&(temp_map.viewHeight),f,true))
            {
              return qe_invalid;
            }
          if(!p_igetw(&(temp_map.viewX),f,true))
            {
              return qe_invalid;
            }
          if(!p_igetw(&(temp_map.viewY),f,true))
            {
              return qe_invalid;
            }
          if(!p_getc(&(temp_map.subaTrans),f,true))
            {
              return qe_invalid;
            }
          if(!p_getc(&(temp_map.subpTrans),f,true))
            {
              return qe_invalid;
            }
        }
        else
        {*/
        temp_map.scrResWidth = 256;
        temp_map.scrResHeight = 224;
        temp_map.tileWidth = 16;
        temp_map.tileHeight = 11;
        temp_map.viewWidth = 256;
        temp_map.viewHeight = 176;
        temp_map.viewX = 0;
        temp_map.viewY = 64;
        temp_map.subaWidth = 256;
        temp_map.subaHeight = 168;
        temp_map.subaTrans = false;
        temp_map.subpWidth = 256;
        temp_map.subpHeight = 56;
        temp_map.subpTrans = false;
        
        //}
        for(int j=0; j<screens_to_read; j++)
        {
            scr=i*MAPSCRS+j;
            clear_screen(&temp_mapscr);
            readmapscreen(f, Header, &temp_mapscr, &temp_map, version);
            
            if(keepdata==true)
            {
            
//		delete_theMaps_data( scr );
                TheMaps[scr] = temp_mapscr;
                
            }
        }
        
        if(keepdata==true)
        {
            map_count = temp_map_count;
            
            if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<137)))
            {
                int index = (i*MAPSCRS+132);
                //.....hmmm. todo: test this. >_>
                
//		delete_theMaps_data(index);
                TheMaps[index]=TheMaps[index-1];
                
                const int _mapsSize = (temp_map.tileWidth)*(temp_map.tileHeight);
                
                //err...what? @_@
                TheMaps[index].data.resize(_mapsSize, 0);
                TheMaps[index].sflag.resize(_mapsSize, 0);
                TheMaps[index].cset.resize(_mapsSize, 0);
                
                TheMaps[i*MAPSCRS+132].data = TheMaps[i*MAPSCRS+131].data;
                TheMaps[i*MAPSCRS+132].sflag = TheMaps[i*MAPSCRS+131].sflag;
                TheMaps[i*MAPSCRS+132].cset = TheMaps[i*MAPSCRS+131].cset;
                
                for(int j=133; j<MAPSCRS; j++)
                {
                    scr=i*MAPSCRS+j;
                    
                    TheMaps[scr].zero_memory();
                    TheMaps[scr].valid = mVERSION;
                    TheMaps[scr].screen_midi = -1;
                    TheMaps[scr].csensitive = 1;
                }
            }
            
            if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<154)))
            {
                for(int j=0; j<MAPSCRS; j++)
                {
                    scr=i*MAPSCRS+j;
                    TheMaps[scr].door_combo_set=MakeDoors(i, j);
                    
                    for(int k=0; k<128; k++)
                    {
                        TheMaps[scr].secretcset[k]=tcmbcset2(i, TheMaps[scr].secretcombo[k]);
                        TheMaps[scr].secretflag[k]=tcmbflag2(i, TheMaps[scr].secretcombo[k]);
                        TheMaps[scr].secretcombo[k]=tcmbdat2(i, j, TheMaps[scr].secretcombo[k]);
                    }
                }
            }
        }
        
        if(keepdata==true)
        {
            memcpy(&ZCMaps[i], &temp_map, sizeof(zcmap));
        }
    }
    
    clear_screen(&temp_mapscr);
    return 0;
}


int readcombos(PACKFILE *f, zquestheader *Header, word version, word build, word start_combo, word max_combos, bool keepdata)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    
    reset_combo_animations();
    reset_combo_animations2();
    
    init_combo_classes();
    
    // combos
    word combos_used=0;
    int dummy;
    byte padding;
    newcombo temp_combo;
    word section_version=0;
    word section_cversion=0;
    
    if(keepdata==true)
    {
        memset(combobuf+start_combo,0,sizeof(newcombo)*max_combos);
    }
    
    if(version > 0x192)
    {
        //section version info
        if(!p_igetw(&section_version,f,true))
        {
            return qe_invalid;
        }
	
	ffcore.quest_format[vCombos] = section_version;
        
        //al_trace("Combos version %d\n", section_version);
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
    for(int i=0; i<combos_used; i++)
    {
        memset(&temp_combo,0,sizeof(temp_combo));
        
	if ( section_version >= 11 )
	{
		if(!p_igetl(&temp_combo.tile,f,true))
		{
		    return qe_invalid;
		}
	}
	else
	{
		if(!p_igetw(&temp_combo.tile,f,true))
		{
		    return qe_invalid;
		}
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
        
        if(version < 0x193)
        {
            if(!p_getc(&padding,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&padding,f,true))
            {
                return qe_invalid;
            }
            
            if(version < 0x192)
            {
                if(version == 0x191)
                {
                    for(int tmpcounter=0; tmpcounter<16; tmpcounter++)
                    {
                        if(!p_getc(&padding,f,true))
                        {
                            return qe_invalid;
                        }
                    }
                }
                
                if(keepdata==true)
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
        
        //Base flag
        if(section_version>=3)
        {
            if(!p_getc(&temp_combo.flag,f,true))
            {
                return qe_invalid;
            }
        }
        
        if(section_version>=4)
        {
            if(!p_getc(&temp_combo.skipanim,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&temp_combo.nexttimer,f,true))
            {
                return qe_invalid;
            }
        }
        
        if(section_version>=5)
        {
            if(!p_getc(&temp_combo.skipanimy,f,true))
            {
                return qe_invalid;
            }
        }
        
        if(section_version>=6)
        {
            if(!p_getc(&temp_combo.animflags,f,true))
            {
                return qe_invalid;
            }
            
            if(section_version == 6)
                temp_combo.animflags = temp_combo.animflags ? AF_FRESH : 0;
        }
        
	if(section_version>=8) //combo Attributes[4] and userflags.
	{
		for ( int q = 0; q < NUM_COMBO_ATTRIBUTES; q++ )
		{
		    if(!p_igetl(&temp_combo.attributes[q],f,true))
		    {
			return qe_invalid;
		    }
		}
		if(!p_igetl(&temp_combo.usrflags,f,true))
		{
			return qe_invalid;
		}
	}
	if(section_version==9) //combo trigger flags, V9 only had two indices of triggerflags[]
	{
		for ( int q = 0; q < 2; q++ )
		{
		    if(!p_igetl(&temp_combo.triggerflags[q],f,true))
		    {
			return qe_invalid;
		    }
		}
		if(!p_igetl(&temp_combo.triggerlevel,f,true))
		{
			return qe_invalid;
		}
	}
	if(section_version>=10) //combo trigger flags
	{
	    for ( int q = 0; q < 3; q++ )
		{
		    if(!p_igetl(&temp_combo.triggerflags[q],f,true))
		    {
			return qe_invalid;
		    }
		}
		if(!p_igetl(&temp_combo.triggerlevel,f,true))
		{
			return qe_invalid;
		}
	}
        if(version < 0x193)
        {
            for(int q=0; q<11; q++)
            {
                if(!p_getc(&dummy,f,true))
                {
                    return qe_invalid;
                }
            }
        }
        
        //Goriya tiles were flipped around in 2.11 build 7. Compensate for the flip here. -DD
        if((version < 0x211)||((version == 0x211)&&(build<7)))
        {
            if(!get_bit(quest_rules,qr_NEWENEMYTILES))
            {
                switch(temp_combo.tile)
                {
                case 130:
                    temp_combo.tile = 132;
                    break;
                    
                case 131:
                    temp_combo.tile = 133;
                    break;
                    
                case 132:
                    temp_combo.tile = 130;
                    break;
                    
                case 133:
                    temp_combo.tile = 131;
                    break;
                }
            }
        }
        
        if(keepdata==true && i>=start_combo)
        {
            memcpy(&combobuf[i], &temp_combo, sizeof(temp_combo));
        }
    }
    
    if(keepdata==true)
    {
        if((version < 0x192)|| ((version == 0x192)&&(build<185)))
        {
            for(int tmpcounter=0; tmpcounter<MAXCOMBOS; tmpcounter++)
            {
                if(combobuf[tmpcounter].type==cHOOKSHOTONLY)
                {
                    combobuf[tmpcounter].type=cLADDERHOOKSHOT;
                }
            }
        }
        
        //June 3 2012; ladder only is broken in 2.10 and allows the hookshot also. -Gleeok
        if(version == 0x210 && !is_zquest())
        {
            for(int tmpcounter=0; tmpcounter<MAXCOMBOS; tmpcounter++)
                if(combobuf[tmpcounter].type == cLADDERONLY)
                    combobuf[tmpcounter].type = cLADDERHOOKSHOT;
        }
        
        if(section_version<7)
        {
            for(int tmpcounter=0; tmpcounter<MAXCOMBOS; tmpcounter++)
            {
                switch(combobuf[tmpcounter].type)
                {
                case cSLASH:
                    combobuf[tmpcounter].type=cSLASHTOUCHY;
                    break;
                    
                case cSLASHITEM:
                    combobuf[tmpcounter].type=cSLASHITEMTOUCHY;
                    break;
                    
                case cBUSH:
                    combobuf[tmpcounter].type=cBUSHTOUCHY;
                    break;
                    
                case cFLOWERS:
                    combobuf[tmpcounter].type=cFLOWERSTOUCHY;
                    break;
                    
                case cTALLGRASS:
                    combobuf[tmpcounter].type=cTALLGRASSTOUCHY;
                    break;
                    
                case cSLASHNEXT:
                    combobuf[tmpcounter].type=cSLASHNEXTTOUCHY;
                    break;
                    
                case cSLASHNEXTITEM:
                    combobuf[tmpcounter].type=cSLASHNEXTITEMTOUCHY;
                    break;
                    
                case cBUSHNEXT:
                    combobuf[tmpcounter].type=cBUSHNEXTTOUCHY;
                    break;
                }
            }
        }
        
    }
    
    //Now for the new combo alias reset
    if(section_version<2 && keepdata)
    {
        for(int j=0; j<MAXCOMBOALIASES; j++)
        {
            combo_aliases[j].width = 0;
            combo_aliases[j].height = 0;
            combo_aliases[j].layermask = 0;
            
            if(combo_aliases[j].combos != NULL)
            {
                delete[] combo_aliases[j].combos;
            }
            
            if(combo_aliases[j].csets != NULL)
            {
                delete[] combo_aliases[j].csets;
            }
            
            combo_aliases[j].combos = new word[1];
            combo_aliases[j].csets = new byte[1];
            combo_aliases[j].combos[0] = 0;
            combo_aliases[j].csets[0] = 0;
        }
    }
    
    
    setup_combo_animations();
    setup_combo_animations2();
    return 0;
}

int readcomboaliases(PACKFILE *f, zquestheader *Header, word version, word build, bool keepdata)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    version=version;
    build=build;
    
    int dummy;
    word sversion=0, c_sversion;
    
    //section version info
    if(!p_igetw(&sversion,f,true))
    {
        return qe_invalid;
    }
    
    ffcore.quest_format[vComboAliases] = sversion;
    
    //al_trace("Combo aliases version %d\n", sversion);
    if(!p_igetw(&c_sversion,f,true))
    {
        return qe_invalid;
    }
    
    //section size
    if(!p_igetl(&dummy,f,true))
    {
        return qe_invalid;
    }
    
    int max_num_combo_aliases = MAXCOMBOALIASES;
    
    if(sversion < 3) // max saved combo alias' upped from 256 to 2048.
    {
        max_num_combo_aliases = MAX250COMBOALIASES;
    }
    if(sversion < 2) // max saved combo alias' upped from 256 to 2048.
    {
        max_num_combo_aliases = OLDMAXCOMBOALIASES;
    }
    
    for(int j=0; j<max_num_combo_aliases; j++)
    {
        byte width,height,mask,tempcset;
        int count;
        word tempword;
        byte tempbyte;
        
        if(!p_igetw(&tempword,f,true))
        {
            return qe_invalid;
        }
        
        if(keepdata)
        {
            combo_aliases[j].combo = tempword;
        }
        
        if(!p_getc(&tempbyte,f,true))
        {
            return qe_invalid;
        }
        
        if(keepdata)
        {
            combo_aliases[j].cset = tempbyte;
        }
        
        if(!p_getc(&width,f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&height,f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&mask,f,true))
        {
            return qe_invalid;
        }
        
        count=(width+1)*(height+1)*(comboa_lmasktotal(mask)+1);
        
        if(keepdata)
        {
            if(combo_aliases[j].combos != NULL)
            {
                delete[] combo_aliases[j].combos;
            }
            
            if(combo_aliases[j].csets != NULL)
            {
                delete[] combo_aliases[j].csets;
            }
            
            combo_aliases[j].width = width;
            combo_aliases[j].height = height;
            combo_aliases[j].layermask = mask;
            combo_aliases[j].combos = new word[count];
            combo_aliases[j].csets = new byte[count];
        }
        
        for(int k=0; k<count; k++)
        {
            if(!p_igetw(&tempword,f,true))
            {
                return qe_invalid;
            }
            
            if(keepdata)
            {
                combo_aliases[j].combos[k] = tempword;
            }
        }
        
        for(int k=0; k<count; k++)
        {
            if(!p_getc(&tempcset,f,true))
            {
                return qe_invalid;
            }
            
            if(keepdata)
            {
                combo_aliases[j].csets[k] = tempcset;
            }
        }
    }
    
    return 0;
}

int readcolordata(PACKFILE *f, miscQdata *Misc, word version, word build, word start_cset, word max_csets, bool keepdata)
{
    //these are here to bypass compiler warnings about unused arguments
    start_cset=start_cset;
    max_csets=max_csets;
    word s_version=0;
    
    miscQdata temp_misc;
    memcpy(&temp_misc, Misc, sizeof(temp_misc));
    
    byte temp_colordata[48];
    char temp_palname[PALNAMESIZE];
    
    int dummy;
    word palcycles;
    
    if(version > 0x192)
    {
        //section version info
        if(!p_igetw(&s_version,f,true))
        {
            return qe_invalid;
        }
	
	ffcore.quest_format[vColours] = s_version;
        
        //al_trace("Color data version %d\n", s_version);
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
    for(int i=0; i<oldpdTOTAL; ++i)
    {
        memset(temp_colordata, 0, 48);
        
        if(!pfread(temp_colordata,48,f,true))
        {
            return qe_invalid;
        }
        
        if(keepdata==true)
        {
            memcpy(&colordata[i*48], temp_colordata, 48);
        }
    }
    
    if((version < 0x192)||((version == 0x192)&&(build<73)))
    {
        if(keepdata==true)
        {
            memcpy(colordata+(newerpoSPRITE*48), colordata+(oldpoSPRITE*48), 30*16*3);
            memset(colordata+(oldpoSPRITE*48), 0, ((newerpoSPRITE-oldpoSPRITE)*48));
            memcpy(colordata+((newerpoSPRITE+11)*48), colordata+((newerpoSPRITE+10)*48), 48);
            memcpy(colordata+((newerpoSPRITE+10)*48), colordata+((newerpoSPRITE+9)*48), 48);
            memcpy(colordata+((newerpoSPRITE+9)*48), colordata+((newerpoSPRITE+8)*48), 48);
            memset(colordata+((newerpoSPRITE+8)*48), 0, 48);
        }
    }
    else
    {
        memset(temp_colordata, 0, 48);
        
        for(int i=0; i<newpdTOTAL-oldpdTOTAL; ++i)
        {
            if(!pfread(temp_colordata,48,f,true))
            {
                return qe_invalid;
            }
            
            if(keepdata==true)
            {
                memcpy(&colordata[(oldpdTOTAL+i)*48], temp_colordata, 48);
            }
        }
        
        if(s_version < 4)
        {
            if(keepdata==true)
            {
                memcpy(colordata+(newerpoSPRITE*48), colordata+(newpoSPRITE*48), 30*16*3);
                memset(colordata+(newpoSPRITE*48), 0, ((newerpoSPRITE-newpoSPRITE)*48));
            }
        }
        else
        {
            for(int i=0; i<newerpdTOTAL-newpdTOTAL; ++i)
            {
                if(!pfread(temp_colordata,48,f,true))
                {
                    return qe_invalid;
                }
                
                if(keepdata==true)
                {
                    memcpy(&colordata[(newpdTOTAL+i)*48], temp_colordata, 48);
                }
            }
        }
    }
    
    if((version < 0x192)||((version == 0x192)&&(build<76)))
    {
        if(keepdata==true)
        {
            init_palnames();
        }
    }
    else
    {
        int palnamestoread = 0;
        
        if(s_version < 3)
            palnamestoread = OLDMAXLEVELS;
        else
            palnamestoread = 512;
            
        for(int i=0; i<palnamestoread; ++i)
        {
            memset(temp_palname, 0, PALNAMESIZE);
            
            if(!pfread(temp_palname,PALNAMESIZE,f,true))
            {
                return qe_invalid;
            }
            
            if(keepdata==true)
            {
                memcpy(palnames[i], temp_palname, PALNAMESIZE);
            }
        }
        
        if(keepdata)
        {
            for(int i=palnamestoread; i<MAXLEVELS; i++)
            {
                memset(palnames[i], 0, PALNAMESIZE);
            }
        }
    }
    
    if(version > 0x192)
    {
        for(int i=0; i<256; i++)
        {
            for(int j=0; j<3; j++)
            {
                temp_misc.cycles[i][j].first=0;
                temp_misc.cycles[i][j].count=0;
                temp_misc.cycles[i][j].speed=0;
            }
        }
        
        if(!p_igetw(&palcycles,f,true))
        {
            return qe_invalid;
        }
        
        for(int i=0; i<palcycles; i++)
        {
            for(int j=0; j<3; j++)
            {
                if(!p_getc(&temp_misc.cycles[i][j].first,f,true))
                {
                    return qe_invalid;
                }
            }
            
            for(int j=0; j<3; j++)
            {
                if(!p_getc(&temp_misc.cycles[i][j].count,f,true))
                {
                    return qe_invalid;
                }
            }
            
            for(int j=0; j<3; j++)
            {
                if(!p_getc(&temp_misc.cycles[i][j].speed,f,true))
                {
                    return qe_invalid;
                }
            }
        }
        
        if(keepdata==true)
        {
            memcpy(Misc, &temp_misc, sizeof(temp_misc));
        }
    }
    
    return 0;
}

int readtiles(PACKFILE *f, tiledata *buf, zquestheader *Header, word version, word build, word start_tile, int max_tiles, bool from_init, bool keepdata)
{
    int dummy;
    int tiles_used=0;
	word section_version = 0;
	word section_cversion = 0;
	int section_size= 0;
    byte *temp_tile = new byte[tilesize(tf32Bit)];
	
    //Tile Expansion
    if ( build < 41 ) 
    {
	    //al_trace("Build was < 41 when reading tiles\n");
	    max_tiles = ZC250MAXTILES;
    }
    
	//al_trace("Max Tiles: %d\n", max_tiles);
	
    if(Header!=NULL&&(!Header->data_flags[ZQ_TILES]&&!from_init))         //keep for old quests
    {
        if(keepdata==true)
        {
            if(!init_tiles(true, Header))
            {
                al_trace("Unable to initialize tiles\n");
            }
        }
        
        delete[] temp_tile;
        temp_tile=NULL;
        return 0;
    }
    else
    {
        if(version > 0x192)
        {
            //section version info
            if(!p_igetw(&section_version,f,true))
            {
                delete[] temp_tile;
                return qe_invalid;
            }
            
	    ffcore.quest_format[vTiles] = section_version;
	    
            if(!p_igetw(&section_cversion,f,true))
            {
                delete[] temp_tile;
                return qe_invalid;
            }
            
            //section size
            if(!p_igetl(&section_size,f,true))
            {
                delete[] temp_tile;
                return qe_invalid;
            }
        }
        
	//if ( build < 41 ) 
	//{
	//	tiles_used = ZC250MAXTILES;
	//}
	
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
            if ( version >= 0x254 && build >= 41 ) //read and write the size of tiles_used properly
	    { 
		    if(!p_igetl(&tiles_used,f,true))
		    {
			delete[] temp_tile;
			return qe_invalid;
		    }
	    }
	    else
	    {
		 if(!p_igetw(&tiles_used,f,true))
		    {
			delete[] temp_tile;
			return qe_invalid;
		    }
	    }
	    
        }
        
        tiles_used=zc_min(tiles_used, max_tiles);
        
	if ( version < 0x254 && build < 41 )
	{
		tiles_used=zc_min(tiles_used, ZC250MAXTILES-start_tile);
	}
	else 
	{
		tiles_used = zc_min(tiles_used,NEWMAXTILES-start_tile); 
	}
	
	//if ( section_version > 1 ) tiles_used = NEWMAXTILES;
        
	//al_trace("tiles_used = %d\n", tiles_used);
	
        for(int i=0; i<tiles_used; ++i)
        {
            byte format=tf4Bit;
            memset(temp_tile, 0, tilesize(tf32Bit));
            
            if((version>0x211)||((version==0x211)&&(build>4)))
            {
                if(!p_getc(&format,f,true))
                {
                    delete[] temp_tile;
                    return qe_invalid;
                }
            }
            
            if(!pfread(temp_tile,tilesize(format),f,true))
            {
                delete[] temp_tile;
                return qe_invalid;
            }
            
            if(keepdata==true)
            {
                buf[start_tile+i].format=format;
                
                if(buf[start_tile+i].data)
                {
                    zc_free(buf[start_tile+i].data);
                    buf[start_tile+i].data=NULL;
                }
                
                buf[start_tile+i].data=(byte *)zc_malloc(tilesize(buf[start_tile+i].format));
                memcpy(buf[start_tile+i].data,temp_tile,tilesize(buf[start_tile+i].format));
            }
        }
    }
    
	if ( section_version < 2 ) //write blank tile data --check s_version with this again instead?
	{
		//al_trace("Writing blank tile data to new tiles for build < 41\n");
		for ( int q = ZC250MAXTILES; q < NEWMAXTILES; ++q )
		{
			
			buf[q].data=(byte *)zc_malloc(tilesize(tf4Bit));
			//memcpy(buf[q].data,temp_tile,tilesize(buf[q].format));
			reset_tile(buf,q,tf4Bit);
			
			
			/*
			
			byte tempbyte;
			for(int i=0; i<tilesize(tf4Bit); i++)
			{
				tempbyte=buf[ZC250MAXTILES-1].data[i];
				buf[q].data[i] = tempbyte;
			}
			//int temp = tempbyte=buf[130].data[i];
			//buf[q].data = buf[ZC250MAXTILES-1].data;
			*/
			//reset_tile(buf,q,tf4Bit);
		}
		
	}
    
    if(keepdata==true)
    {
	    //al_trace("calling reset_tile()");
	if ( build < 41 ) 
	{
		for(int i=start_tile+tiles_used; i<max_tiles; ++i)
		{
			//al_trace("Resetting tiles for ZC250MAXTILES, iteration: %d\n", i);
		    reset_tile(buf,i,tf4Bit);
		}
	}

	else
	{
		for(int i=start_tile+tiles_used; i<max_tiles; ++i)
		{
			//al_trace("Resetting tiles for build 41+\n");
		    reset_tile(buf,i,tf4Bit);
		}
	}
	
        
        if((version < 0x192)|| ((version == 0x192)&&(build<186)))
        {
            if(get_bit(quest_rules,qr_BSZELDA))   //
            {
                byte tempbyte;
                int floattile=wpnsbuf[iwSwim].tile;
                
                for(int i=0; i<tilesize(tf4Bit); i++)  //BSZelda tiles are out of order
                {
                    tempbyte=buf[23].data[i];
                    buf[23].data[i]=buf[24].data[i];
                    buf[24].data[i]=buf[25].data[i];
                    buf[25].data[i]=buf[26].data[i];
                    buf[26].data[i]=tempbyte;
                }
                
                for(int i=0; i<tilesize(tf4Bit); i++)
                {
                    tempbyte=buf[floattile+11].data[i];
                    buf[floattile+11].data[i]=buf[floattile+12].data[i];
                    buf[floattile+12].data[i]=tempbyte;
                }
            }
        }
        
        if((version < 0x211)||((version == 0x211)&&(build<7)))   //Goriya tiles are out of order
        {
            if(!get_bit(quest_rules,qr_NEWENEMYTILES))
            {
                byte tempbyte;
                
                for(int i=0; i<tilesize(tf4Bit); i++)
                {
                    tempbyte=buf[130].data[i];
                    buf[130].data[i]=buf[132].data[i];
                    buf[132].data[i]=tempbyte;
                    
                    tempbyte=buf[131].data[i];
                    buf[131].data[i]=buf[133].data[i];
                    buf[133].data[i]=tempbyte;
                }
            }
        }
        
	
	
	al_trace("Registering blank tiles\n");
        register_blank_tiles(max_tiles);
    }
    
    //memset(temp_tile, 0, tilesize(tf32Bit));
    delete[] temp_tile;
    temp_tile=NULL;
    return 0;
}

int readtunes(PACKFILE *f, zquestheader *Header, zctune *tunes /*zcmidi_ *midis*/, bool keepdata)
{
    byte *mf=midi_flags;
    long dummy;
    word dummy2;
    // zcmidi_ temp_midi;
    int tunes_to_read;
    int tune_count=0;
    word section_version=0;
    zctune temp;
    
    if(Header->zelda_version < 0x193)
    {
        //    mf=Header->data_flags+ZQ_MIDIS2;
        if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<178)))
        {
            tunes_to_read=MAXCUSTOMMIDIS192b177;
        }
        else
        {
            tunes_to_read=MAXCUSTOMTUNES;
        }
    }
    else
    {
        //section version info
        if(!p_igetw(&section_version,f,true))
        {
            return qe_invalid;
        }
	
	ffcore.quest_format[vMIDIs] = section_version;
        
        //al_trace("Tunes version %d\n", section_version);
        if(!p_igetw(&dummy2,f,true))
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
        
        tunes_to_read=MAXCUSTOMTUNES;
    }
    
    for(int i=0; i<MAXCUSTOMTUNES; ++i)
    {
        if(get_bit(mf, i))
        {
            ++tune_count;
        }
    }
    
    if(keepdata==true)
    {
        reset_tunes(tunes); //reset_midis(midis);
    }
    
    for(int i=0; i<tunes_to_read; i++)
    {
        temp.clear(); //memset(&temp_midi,0,sizeof(zcmidi_));
        
        if(keepdata==true)
        {
            tunes[i].reset(); // reset_midi(midis+i);
        }
        
        if(get_bit(mf,i))
        {
            if(section_version < 4)
            {
                if(!pfread(&temp.title,sizeof(char)*20,f,true))
                {
                    return qe_invalid;
                }
            }
            else
            {
                if(!pfread(&temp.title,sizeof(temp.title),f,true))
                {
                    return qe_invalid;
                }
            }
            
            if(!p_igetl(&temp.start,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetl(&temp.loop_start,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetl(&temp.loop_end,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&temp.loop,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&temp.volume,f,true))
            {
                return qe_invalid;
            }
            
            if(Header->zelda_version < 0x193)
            {
                if(!p_igetl(&dummy,f,true))
                {
                    return qe_invalid;
                }
            }
            
            if(section_version >= 3)
            {
                if(!pfread(&temp.flags,sizeof(temp.flags),f,true))
                {
                    return qe_invalid;
                }
            }
            
            if(keepdata==true)
            {
                tunes[i].copyfrom(temp); // memcpy(&midis[i], &temp_midi, sizeof(zcmidi_));
            }
            
            if(section_version < 2) //= 1 || (Header->zelda_version < 0x211) || (Header->zelda_version == 0x211 && Header->build < 18))
            {
                // old format - a midi is a midi
                if(((keepdata==true?tunes[i].data:temp.data)=read_midi(f, true))==NULL)
                {
                    return qe_invalid;
                }
                
                //yes you can do this. Isn't the ? operator awesome? :)
                (keepdata ? tunes[i] : temp).format = MFORMAT_MIDI;
            }
            else
            {
                // 'midi' could be midi or nes, gb, ... music
                if(!pfread(&(keepdata ? tunes[i] : temp).format,sizeof((keepdata ? tunes[i] : temp).format),f,true))
                {
                    return qe_invalid;
                }
                
                zctune *ptr = (keepdata==true)?&(tunes[i]):&temp;
                
                switch(temp.format)
                {
                case MFORMAT_MIDI:
                    if((ptr->data=read_midi(f, true))==NULL)
                    {
                        return qe_invalid;
                    }
                    
                    break;
                    
                default:
                    return qe_invalid;
                    break;
                }
            }
        }
    }
    
    return 0;
}

int readcheatcodes(PACKFILE *f, zquestheader *Header, bool keepdata)
{
    int dummy;
    ZCHEATS tempzcheats;
    char temp_use_cheats=1;
    memset(&tempzcheats, 0, sizeof(tempzcheats));
    word s_version = 0;
    
    if(Header->zelda_version > 0x192)
    {
        //section version info
        if(!p_igetw(&s_version,f,true))
        {
            return qe_invalid;
        }
        
        //al_trace("Cheats version %d\n", dummy);
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
    
    if(Header->data_flags[ZQ_CHEATS2])
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
    
    if(keepdata==true)
    {
        memcpy(&zcheats, &tempzcheats, sizeof(tempzcheats));
        Header->data_flags[ZQ_CHEATS2]=temp_use_cheats;
    }
    
    return 0;
}

int readinitdata(PACKFILE *f, zquestheader *Header, bool keepdata)
{
    int dummy;
    word s_version=0, s_cversion=0;
    byte padding;
    
    zinitdata temp_zinit;
    memset(&temp_zinit, 0, sizeof(zinitdata));
    
    
    // Legacy item properties (now integrated into itemdata)
    byte sword_hearts[4];
    byte beam_hearts[4];
    byte beam_percent=0;
    word beam_power[4];
    byte hookshot_length=99;
    byte hookshot_links=100;
    byte longshot_length=99;
    byte longshot_links=100;
    byte moving_fairy_hearts=3;
    byte moving_fairy_heart_percent=0;
    byte stationary_fairy_hearts=3;
    byte stationary_fairy_heart_percent=0;
    byte moving_fairy_magic=0;
    byte moving_fairy_magic_percent=0;
    byte stationary_fairy_magic=0;
    byte stationary_fairy_magic_percent=0;
    byte blue_potion_hearts=100;
    byte blue_potion_heart_percent=1;
    byte red_potion_hearts=100;
    byte red_potion_heart_percent=1;
    byte blue_potion_magic=100;
    byte blue_potion_magic_percent=1;
    byte red_potion_magic=100;
    byte red_potion_magic_percent=1;
    
    temp_zinit.subscreen_style=get_bit(quest_rules,qr_COOLSCROLL)?1:0;
    temp_zinit.max_rupees=255;
    temp_zinit.max_keys=255;
    temp_zinit.hcp_per_hc=4;
    temp_zinit.bomb_ratio=4;
    
    for(int i=0; i<MAXITEMS; i++)
    {
        temp_zinit.items[i]=false;
    }
    
    if(Header->zelda_version > 0x192)
    {
        //section version info
        if(!p_igetw(&s_version,f,true))
        {
            return qe_invalid;
        }
        
	ffcore.quest_format[vInitData] = s_version;
	
        //al_trace("Init data version %d\n", s_version);
        if(!p_igetw(&s_cversion,f,true))
        {
            return qe_invalid;
        }
        
        //section size
        if(!p_igetl(&dummy,f,true))
        {
            return qe_invalid;
        }
    }
    
    /* HIGHLY UNORTHODOX UPDATING THING, by L
     * This fixes quests made before revision 277 (such as the 'Lost Isle Build'),
     * where the speed of Pols Voice changed. It also coincided with V_INITDATA
     * changing from 13 to 14.
     */
    if(keepdata && s_version < 14)
        fixpolsvoice=true;
        
    /* End highly unorthodox updating thing */
    
    temp_zinit.ss_grid_x=8;
    temp_zinit.ss_grid_y=8;
    temp_zinit.ss_grid_xofs=0;
    temp_zinit.ss_grid_yofs=0;
    temp_zinit.ss_grid_color=8;
    temp_zinit.ss_bbox_1_color=15;
    temp_zinit.ss_bbox_2_color=7;
    temp_zinit.ss_flags=0;
    temp_zinit.gravity=16;
    temp_zinit.terminalv=320;
    temp_zinit.msg_speed=5;
    temp_zinit.transition_type=0;
    temp_zinit.jump_link_layer_threshold=255;
    
    if(s_version >= 15 && get_bit(deprecated_rules, 27)) // The short-lived rule, qr_JUMPLINKLAYER3
        temp_zinit.jump_link_layer_threshold=0;
        
    if(s_version >= 10)
    {
        char temp;
        
        //new-style items
        for(int j=0; j<256; j++)
        {
            if(!p_getc(&temp,f,true))
                return qe_invalid;
                
            temp_zinit.items[j] = (temp != 0);
        }
    }
    
    if((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>26)))
    {
        char temp;
        
        //finally...  section data
        if((Header->zelda_version > 0x192)||
                //new only
                ((Header->zelda_version == 0x192)&&(Header->build>173)))
        {
            //OLD-style items... sigh
            if(s_version < 10)
            {
                if(!p_getc(&temp,f,true))
                {
                    return qe_invalid;
                }
                
                temp_zinit.items[iRaft]=(temp != 0);
                
                if(!p_getc(&temp,f,true))
                {
                    return qe_invalid;
                }
                
                temp_zinit.items[iLadder]=(temp != 0);
                
                if(!p_getc(&temp,f,true))
                {
                    return qe_invalid;
                }
                
                temp_zinit.items[iBook]=(temp != 0);
                
                if(!p_getc(&temp,f,true))
                {
                    return qe_invalid;
                }
                
                temp_zinit.items[iMKey]=(temp!=0);
                
                if(!p_getc(&temp,f,true))
                {
                    return qe_invalid;
                }
                
                temp_zinit.items[iFlippers]=(temp != 0);
                
                if(!p_getc(&temp,f,true))
                {
                    return qe_invalid;
                }
                
                temp_zinit.items[iBoots]=(temp!=0);
            }
        }
        
        if(s_version < 10)
        {
            char tempring, tempsword, tempshield, tempwallet, tempbracelet, tempamulet, tempbow;
            
            if(!p_getc(&tempring,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&tempsword,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&tempshield,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&tempwallet,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&tempbracelet,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&tempamulet,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&tempbow,f,true))
            {
                return qe_invalid;
            }
            
            //old only
            if((Header->zelda_version == 0x192)&&(Header->build<174))
            {
                tempring=(tempring)?(1<<(tempring-1)):0;
                tempsword=(tempsword)?(1<<(tempsword-1)):0;
                tempshield=(tempshield)?(1<<(tempshield-1)):0;
                tempwallet=(tempwallet)?(1<<(tempwallet-1)):0;
                tempbracelet=(tempbracelet)?(1<<(tempbracelet-1)):0;
                tempamulet=(tempamulet)?(1<<(tempamulet-1)):0;
                tempbow=(tempbow)?(1<<(tempbow-1)):0;
            }
            
            //rings start at level 2... wtf
            //account for this -DD
            tempring <<= 1;
            addOldStyleFamily(&temp_zinit, itemsbuf, itype_ring, tempring);
            addOldStyleFamily(&temp_zinit, itemsbuf, itype_sword, tempsword);
            addOldStyleFamily(&temp_zinit, itemsbuf, itype_shield, tempshield);
            addOldStyleFamily(&temp_zinit, itemsbuf, itype_wallet, tempwallet);
            //bracelet ALSO starts at level 2 :-( -DD
            tempbracelet<<=1;
            addOldStyleFamily(&temp_zinit, itemsbuf, itype_bracelet, tempbracelet);
            addOldStyleFamily(&temp_zinit, itemsbuf, itype_amulet, tempamulet);
            addOldStyleFamily(&temp_zinit, itemsbuf, itype_bow, tempbow);
            
            //new only
            if((Header->zelda_version == 0x192)&&(Header->build>173))
            {
                for(int q=0; q<32; q++)
                {
                    if(!p_getc(&padding,f,true))
                    {
                        return qe_invalid;
                    }
                }
            }
            
            char tempcandle, tempboomerang, temparrow, tempwhistle;
            
            if(!p_getc(&tempcandle,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&tempboomerang,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&temparrow,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&temp,f,true))
            {
                return qe_invalid;
            }
            
            addOldStyleFamily(&temp_zinit, itemsbuf, itype_potion, temp);
            
            if(!p_getc(&tempwhistle,f,true))
            {
                return qe_invalid;
            }
            
            //old only
            if((Header->zelda_version == 0x192)&&(Header->build<174))
            {
                tempcandle=(tempcandle)?(1<<(tempcandle-1)):0;
                tempboomerang=(tempboomerang)?(1<<(tempboomerang-1)):0;
                temparrow=(temparrow)?(1<<(temparrow-1)):0;
                tempwhistle=(tempwhistle)?(1<<(tempwhistle-1)):0;
            }
            
            addOldStyleFamily(&temp_zinit, itemsbuf, itype_candle, tempcandle);
            addOldStyleFamily(&temp_zinit, itemsbuf, itype_brang, tempboomerang);
            addOldStyleFamily(&temp_zinit, itemsbuf, itype_arrow, temparrow);
            addOldStyleFamily(&temp_zinit, itemsbuf, itype_whistle, tempwhistle);
            //What about the potion...?
            
        }
        
        //Oh sure, stick these IN THE MIDDLE OF THE ITEMS, just to make me want
        //to jab out my eye...
        if(!p_getc(&temp_zinit.bombs,f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&temp_zinit.super_bombs,f,true))
        {
            return qe_invalid;
        }
        
        //Back to more OLD item code
        if(s_version < 10)
        {
            if((Header->zelda_version > 0x192)||
                    //new only
                    ((Header->zelda_version == 0x192)&&(Header->build>173)))
            {
                if(!p_getc(&temp,f,true))
                {
                    return qe_invalid;
                }
                
                addOldStyleFamily(&temp_zinit, itemsbuf, itype_wand, temp);
                
                if(!p_getc(&temp,f,true))
                {
                    return qe_invalid;
                }
                
                addOldStyleFamily(&temp_zinit, itemsbuf, itype_letter, temp);
                
                if(!p_getc(&temp,f,true))
                {
                    return qe_invalid;
                }
                
                addOldStyleFamily(&temp_zinit, itemsbuf, itype_lens, temp);
                
                if(!p_getc(&temp,f,true))
                {
                    return qe_invalid;
                }
                
                addOldStyleFamily(&temp_zinit, itemsbuf, itype_hookshot, temp);
                
                if(!p_getc(&temp,f,true))
                {
                    return qe_invalid;
                }
                
                addOldStyleFamily(&temp_zinit, itemsbuf, itype_bait, temp);
                
                if(!p_getc(&temp,f,true))
                {
                    return qe_invalid;
                }
                
                addOldStyleFamily(&temp_zinit, itemsbuf, itype_hammer, temp);
                
                if(!p_getc(&temp,f,true))
                {
                    return qe_invalid;
                }
                
                addOldStyleFamily(&temp_zinit, itemsbuf, itype_dinsfire, temp);
                
                if(!p_getc(&temp,f,true))
                {
                    return qe_invalid;
                }
                
                addOldStyleFamily(&temp_zinit, itemsbuf, itype_faroreswind, temp);
                
                if(!p_getc(&temp,f,true))
                {
                    return qe_invalid;
                }
                
                addOldStyleFamily(&temp_zinit, itemsbuf, itype_nayruslove, temp);
                
                if(!p_getc(&temp,f,true))
                {
                    return qe_invalid;
                }
                
                if(Header->zelda_version == 0x192)
                {
                    for(int q=0; q<32; q++)
                    {
                        if(!p_getc(&padding,f,true))
                        {
                            return qe_invalid;
                        }
                    }
                }
            }
        }
        
        //old only
        if((Header->zelda_version == 0x192)&&(Header->build<174))
        {
            byte equipment, items;                                //bit flags
            
            if(!p_getc(&equipment,f,true))
            {
                return qe_invalid;
            }
            
            temp_zinit.items[iRaft]=(get_bit(&equipment, idE_RAFT)!=0);
            temp_zinit.items[iLadder]=(get_bit(&equipment, idE_LADDER)!=0);
            temp_zinit.items[iBook]=(get_bit(&equipment, idE_BOOK)!=0);
            temp_zinit.items[iMKey]=(get_bit(&equipment, idE_KEY)!=0);
            temp_zinit.items[iFlippers]=(get_bit(&equipment, idE_FLIPPERS)!=0);
            temp_zinit.items[iBoots]=(get_bit(&equipment, idE_BOOTS)!=0);
            
            
            if(!p_getc(&items,f,true))
            {
                return qe_invalid;
            }
            
            temp_zinit.items[iWand]=(get_bit(&items, idI_WAND)!=0);
            temp_zinit.items[iLetter]=(get_bit(&items, idI_LETTER)!=0);
            temp_zinit.items[iLens]=(get_bit(&items, idI_LENS)!=0);
            temp_zinit.items[iHookshot]=(get_bit(&items, idI_HOOKSHOT)!=0);
            temp_zinit.items[iBait]=(get_bit(&items, idI_BAIT)!=0);
            temp_zinit.items[iHammer]=(get_bit(&items, idI_HAMMER)!=0);
        }
        
        if(!p_getc(&temp_zinit.hc,f,true))
        {
            return qe_invalid;
        }
        
        if(s_version < 14)
        {
            byte temphp;
            
            if(!p_getc(&temphp,f,true))
            {
                return qe_invalid;
            }
            
            temp_zinit.start_heart=temphp;
            
            if(!p_getc(&temphp,f,true))
            {
                return qe_invalid;
            }
            
            temp_zinit.cont_heart=temphp;
        }
        else
        {
            if(!p_igetw(&temp_zinit.start_heart,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&temp_zinit.cont_heart,f,true))
            {
                return qe_invalid;
            }
        }
        
        if(!p_getc(&temp_zinit.hcp,f,true))
        {
            return qe_invalid;
        }
        
        if(s_version >= 14)
        {
            if(!p_getc(&temp_zinit.hcp_per_hc,f,true))
            {
                return qe_invalid;
            }
            
            if(s_version<16)  // July 2007
            {
                if(get_bit(quest_rules,qr_BRANGPICKUP+1))
                    temp_zinit.hcp_per_hc = 0xFF;
                    
                //Dispose of legacy rule
                set_bit(quest_rules,qr_BRANGPICKUP+1, 0);
            }
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
        
        if(s_version>12 || (Header->zelda_version == 0x211 && Header->build == 18))
        {
            for(int i=0; i<64; i++)
            {
                if(!p_getc(&temp_zinit.map[i],f,true))
                {
                    return qe_invalid;
                }
            }
            
            for(int i=0; i<64; i++)
            {
                if(!p_getc(&temp_zinit.compass[i],f,true))
                {
                    return qe_invalid;
                }
            }
        }
        else
        {
            for(int i=0; i<32; i++)
            {
                if(!p_getc(&temp_zinit.map[i],f,true))
                {
                    return qe_invalid;
                }
            }
            
            for(int i=0; i<32; i++)
            {
                if(!p_getc(&temp_zinit.compass[i],f,true))
                {
                    return qe_invalid;
                }
            }
        }
        
        if((Header->zelda_version > 0x192)||
                //new only
                ((Header->zelda_version == 0x192)&&(Header->build>173)))
        {
            if(s_version>12 || (Header->zelda_version == 0x211 && Header->build == 18))
            {
                for(int i=0; i<64; i++)
                {
                    if(!p_getc(&temp_zinit.boss_key[i],f,true))
                    {
                        return qe_invalid;
                    }
                }
            }
            else
            {
                for(int i=0; i<32; i++)
                {
                    if(!p_getc(&temp_zinit.boss_key[i],f,true))
                    {
                        return qe_invalid;
                    }
                }
            }
        }
        
        for(int i=0; i<16; i++)
        {
            if(!p_getc(&temp_zinit.misc[i],f,true))
            {
                return qe_invalid;
            }
        }
        
        if(s_version < 15) for(int i=0; i<4; i++)
            {
                if(!p_getc(&sword_hearts[i],f,true))
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
        
        if(s_version < 14)
        {
            byte tempmp;
            
            if(!p_getc(&tempmp,f,true))
            {
                return qe_invalid;
            }
            
            temp_zinit.max_magic=tempmp;
            
            if(!p_getc(&tempmp,f,true))
            {
                return qe_invalid;
            }
            
            temp_zinit.magic=tempmp;
        }
        else
        {
            if(!p_igetw(&temp_zinit.max_magic,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&temp_zinit.magic,f,true))
            {
                return qe_invalid;
            }
        }
        
        if(s_version < 15)
        {
            if(s_version < 12)
            {
                temp_zinit.max_magic*=MAGICPERBLOCK;
                temp_zinit.magic*=MAGICPERBLOCK;
            }
            
            for(int i=0; i<4; i++)
            {
                if(!p_getc(&beam_hearts[i],f,true))
                {
                    return qe_invalid;
                }
            }
            
            if(!p_getc(&beam_percent,f,true))
            {
                return qe_invalid;
            }
        }
        else
        {
            if(!p_getc(&temp_zinit.bomb_ratio,f,true))
            {
                return qe_invalid;
            }
        }
        
        if(s_version < 15)
        {
            byte tempbp;
            
            for(int i=0; i<4; i++)
            {
                if(!(s_version < 14 ? p_getc(&tempbp,f,true) : p_igetw(&tempbp,f,true)))
                {
                    return qe_invalid;
                }
                
                beam_power[i]=tempbp;
            }
            
            if(!p_getc(&hookshot_links,f,true))
            {
                return qe_invalid;
            }
            
            if(s_version>6)
            {
                if(!p_getc(&hookshot_length,f,true))
                {
                    return qe_invalid;
                }
                
                if(!p_getc(&longshot_links,f,true))
                {
                    return qe_invalid;
                }
                
                if(!p_getc(&longshot_length,f,true))
                {
                    return qe_invalid;
                }
            }
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
        if((Header->zelda_version == 0x192)&&(Header->build<174))
        {
            for(int i=0; i<32; i++)
            {
                if(!p_getc(&temp_zinit.boss_key[i],f,true))
                {
                    return qe_invalid;
                }
            }
        }
        
        if((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>173)))  //new only
        {
            if(s_version <= 10)
            {
                byte tempbyte;
                
                if(!p_getc(&tempbyte,f,true))
                {
                    return qe_invalid;
                }
                
                temp_zinit.start_dmap = (word)tempbyte;
            }
            else
            {
                if(!p_igetw(&temp_zinit.start_dmap,f,true))
                {
                    return qe_invalid;
                }
            }
            
            if(!p_getc(&temp_zinit.linkanimationstyle,f,true))
            {
                return qe_invalid;
            }
        }
        
        if(s_version>1)
        {
            if(!p_getc(&temp_zinit.arrows,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&temp_zinit.max_arrows,f,true))
            {
                return qe_invalid;
            }
        }
        
        if(s_version>2)
        {
            if(s_version <= 10)
            {
                for(int i=0; i<OLDMAXLEVELS; i++)
                {
                    if(!p_getc(&(temp_zinit.level_keys[i]),f,true))
                    {
                        return qe_invalid;
                    }
                }
            }
            else
            {
                for(int i=0; i<MAXLEVELS; i++)
                {
                    if(!p_getc(&(temp_zinit.level_keys[i]),f,true))
                    {
                        return qe_invalid;
                    }
                }
            }
        }
        
        if(s_version>3)
        {
            if(!p_igetw(&temp_zinit.ss_grid_x,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&temp_zinit.ss_grid_y,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&temp_zinit.ss_grid_xofs,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&temp_zinit.ss_grid_yofs,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&temp_zinit.ss_grid_color,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&temp_zinit.ss_bbox_1_color,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&temp_zinit.ss_bbox_2_color,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&temp_zinit.ss_flags,f,true))
            {
                return qe_invalid;
            }
            
            temp_zinit.ss_grid_x=zc_max(temp_zinit.ss_grid_x,1);
            temp_zinit.ss_grid_y=zc_max(temp_zinit.ss_grid_y,1);
        }
        
        if(s_version>4 && s_version<15)
        {
            if(!p_getc(&moving_fairy_hearts,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&moving_fairy_heart_percent,f,true))
            {
                return qe_invalid;
            }
        }
        
        if(s_version>5 && s_version < 10)
        {
            if(!p_getc(&temp,f,true))
            {
                return qe_invalid;
            }
            
            addOldStyleFamily(&temp_zinit, itemsbuf, itype_quiver, temp);
        }
        
        if(s_version>6 && s_version<15)
        {
            if(!p_getc(&stationary_fairy_hearts,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&stationary_fairy_heart_percent,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&moving_fairy_magic,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&moving_fairy_magic_percent,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&stationary_fairy_magic,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&stationary_fairy_magic_percent,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&blue_potion_hearts,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&blue_potion_heart_percent,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&red_potion_hearts,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&red_potion_heart_percent,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&blue_potion_magic,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&blue_potion_magic_percent,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&red_potion_magic,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&red_potion_magic_percent,f,true))
            {
                return qe_invalid;
            }
        }
        
        if(s_version>6)
        {
            if(!p_getc(&temp_zinit.subscreen_style,f,true))
            {
                return qe_invalid;
            }
        }
        
        if(s_version>7)
        {
            if(!p_getc(&temp_zinit.usecustomsfx,f,true))
            {
                return qe_invalid;
            }
        }
        
        if(s_version>8)
        {
            if(!p_igetw(&temp_zinit.max_rupees,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&temp_zinit.max_keys,f,true))
            {
                return qe_invalid;
            }
        }
        
        if(s_version>16)
        {
            if(!p_getc(&temp_zinit.gravity,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_igetw(&temp_zinit.terminalv,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&temp_zinit.msg_speed,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&temp_zinit.transition_type,f,true))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&temp_zinit.jump_link_layer_threshold,f,true))
            {
                return qe_invalid;
            }
        }
        
        if(s_version>17)
        {
            if(!p_getc(&temp_zinit.msg_more_is_offset,f,true))
            {
                return qe_invalid;
            }
        }
        
        //old only
        if((Header->zelda_version == 0x192)&&(Header->build<174))
        {
            byte items2;
            
            if(!p_getc(&items2,f,true))
            {
                return qe_invalid;
            }
            
            temp_zinit.items[iDinsFire]=(get_bit(&items2, idI_DFIRE)!=0);
            temp_zinit.items[iFaroresWind]=(get_bit(&items2, idI_FWIND)!=0);
            temp_zinit.items[iNayrusLove]=(get_bit(&items2, idI_NLOVE)!=0);
        }
        
        if(Header->zelda_version < 0x193)
        {
            for(int q=0; q<96; q++)
            {
                if(!p_getc(&padding,f,true))
                {
                    return qe_invalid;
                }
            }
            
            //new only
            if((Header->zelda_version == 0x192)&&(Header->build>173))
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
    
    if((Header->zelda_version < 0x211)||((Header->zelda_version == 0x211)&&(Header->build<15)))
    {
        //temp_zinit.shield=i_smallshield;
        int sshieldid = getItemID(itemsbuf, itype_shield, i_smallshield);
        
        if(sshieldid != -1)
            temp_zinit.items[sshieldid] = true;
    }
    
    if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<27)))
    {
        temp_zinit.hc=3;
        temp_zinit.start_heart=3;
        temp_zinit.cont_heart=3;
        temp_zinit.max_bombs=8;
    }
    
    if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<50)))
    {
        sword_hearts[0]=0;
        sword_hearts[1]=5;
        sword_hearts[2]=12;
        sword_hearts[3]=21;
    }
    
    if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<51)))
    {
        temp_zinit.last_map=0;
        temp_zinit.last_screen=0;
    }
    
    if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<68)))
    {
        temp_zinit.max_magic=0;
        temp_zinit.magic=0;
        set_bit(temp_zinit.misc,idM_DOUBLEMAGIC,0);
    }
    
    if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<129)))
    {
    
        for(int x=0; x<4; x++)
        {
            beam_hearts[x]=100;
        }
        
        for(int i=0; i<idBP_MAX; i++)
        {
            set_bit(&beam_percent,i,!get_bit(quest_rules,qr_LENSHINTS+i));
            set_bit(quest_rules,qr_LENSHINTS+i,0);
        }
        
        for(int x=0; x<4; x++)
        {
            beam_power[x]=get_bit(quest_rules,qr_HIDECARRIEDITEMS)?50:100;
        }
        
        set_bit(quest_rules,qr_HIDECARRIEDITEMS,0);
        hookshot_links=100;
        temp_zinit.msg_more_x=224;
        temp_zinit.msg_more_y=64;
    }
    
    // Okay,  let's put these legacy values into itemsbuf.
    if(s_version < 15) for(int i=0; i<MAXITEMS; i++)
        {
            switch(i)
            {
            case iFairyStill:
                itemsbuf[i].misc1 = stationary_fairy_hearts;
                itemsbuf[i].misc2 = stationary_fairy_magic;
                itemsbuf[i].misc3 = 0;
                itemsbuf[i].flags |= stationary_fairy_heart_percent ? ITEM_FLAG1 : 0;
                itemsbuf[i].flags |= stationary_fairy_magic_percent ? ITEM_FLAG2 : 0;
                break;
                
            case iFairyMoving:
                itemsbuf[i].misc1 = moving_fairy_hearts;
                itemsbuf[i].misc2 = moving_fairy_magic;
                itemsbuf[i].misc3 = 50;
                itemsbuf[i].flags |= moving_fairy_heart_percent ? ITEM_FLAG1 : 0;
                itemsbuf[i].flags |= moving_fairy_magic_percent ? ITEM_FLAG2 : 0;
                break;
                
            case iRPotion:
                itemsbuf[i].misc1 = red_potion_hearts;
                itemsbuf[i].misc2 = red_potion_magic;
                itemsbuf[i].flags |= red_potion_heart_percent ? ITEM_FLAG1 : 0;
                itemsbuf[i].flags |= red_potion_magic_percent ? ITEM_FLAG2 : 0;
                break;
                
            case iBPotion:
                itemsbuf[i].misc1 = blue_potion_hearts;
                itemsbuf[i].misc2 = blue_potion_magic;
                itemsbuf[i].flags |= blue_potion_heart_percent ? ITEM_FLAG1 : 0;
                itemsbuf[i].flags |= blue_potion_magic_percent ? ITEM_FLAG2 : 0;
                break;
                
            case iSword:
                itemsbuf[i].pickup_hearts = sword_hearts[0];
                itemsbuf[i].misc1 = beam_hearts[0];
                itemsbuf[i].misc2 = beam_power[0];
                // It seems that ITEM_FLAG1 was already added by reset_itembuf()...
                itemsbuf[i].flags &= (!get_bit(&beam_percent,0)) ? ~ITEM_FLAG1 : ~0;
                break;
                
            case iWSword:
                itemsbuf[i].pickup_hearts = sword_hearts[1];
                itemsbuf[i].misc1 = beam_hearts[1];
                itemsbuf[i].misc2 = beam_power[1];
                itemsbuf[i].flags &= (!get_bit(&beam_percent,1)) ? ~ITEM_FLAG1 : ~0;
                break;
                
            case iMSword:
                itemsbuf[i].pickup_hearts = sword_hearts[2];
                itemsbuf[i].misc1 = beam_hearts[2];
                itemsbuf[i].misc2 = beam_power[2];
                itemsbuf[i].flags &= (!get_bit(&beam_percent,2)) ? ~ITEM_FLAG1 : ~0;
                break;
                
            case iXSword:
                itemsbuf[i].pickup_hearts = sword_hearts[3];
                itemsbuf[i].misc1 = beam_hearts[3];
                itemsbuf[i].misc2 = beam_power[3];
                itemsbuf[i].flags &= (!get_bit(&beam_percent,3)) ? ~ITEM_FLAG1 : ~0;
                break;
                
            case iHookshot:
                itemsbuf[i].misc1 = hookshot_length;
                itemsbuf[i].misc2 = hookshot_links;
                break;
                
            case iLongshot:
                itemsbuf[i].misc1 = longshot_length;
                itemsbuf[i].misc2 = longshot_links;
                break;
            }
        }
        
    if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<168)))
    {
        //was new subscreen rule
        temp_zinit.subscreen=get_bit(quest_rules,qr_FREEFORM)?1:0;
        set_bit(quest_rules,qr_FREEFORM,0);
    }
    
    if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<185)))
    {
        temp_zinit.start_dmap=0;
    }
    
    if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<186)))
    {
        temp_zinit.linkanimationstyle=get_bit(quest_rules,qr_BSZELDA)?1:0;
    }
    
    if(s_version < 16 && get_bit(deprecated_rules, qr_COOLSCROLL+1))
    {
        //addOldStyleFamily(&temp_zinit, itemsbuf, itype_wallet, 4);   //is this needed?
        temp_zinit.max_rupees=999;
        temp_zinit.rupies=999;
    }
    
    if(keepdata==true)
    {
        memcpy(&zinit, &temp_zinit, sizeof(zinitdata));
        
        if(zinit.linkanimationstyle==las_zelda3slow)
        {
            link_animation_speed=2;
        }
        else
        {
            link_animation_speed=1;
        }
    }
    
    return 0;
}

/*
void setupitemdropsets()
{
  for(int i=0; i<isMAX; i++)
  {
    memcpy(&item_drop_sets[i], &default_item_drop_sets[i], sizeof(item_drop_object));
  }
}
*/

int readitemdropsets(PACKFILE *f, int version, word build, bool keepdata)
{
    build=build; // here to prevent compiler warnings
    dword dummy_dword;
    word item_drop_sets_to_read=0;
    item_drop_object tempitemdrop;
    word s_version=0, s_cversion=0;
    
    if(keepdata)
    {
        for(int i=0; i<MAXITEMDROPSETS; i++)
        {
            memset(&item_drop_sets[i], 0, sizeof(item_drop_object));
        }
    }
    
    if(version > 0x192)
    {
        item_drop_sets_to_read=0;
        
        //section version info
        if(!p_igetw(&s_version,f,true))
        {
            return qe_invalid;
        }
	
	ffcore.quest_format[vItemDropsets] = s_version;
        
        //al_trace("Item drop sets version %d\n", s_version);
        if(!p_igetw(&s_cversion,f,true))
        {
            return qe_invalid;
        }
        
        //section size
        if(!p_igetl(&dummy_dword,f,true))
        {
            return qe_invalid;
        }
        
        //finally...  section data
        if(!p_igetw(&item_drop_sets_to_read,f,true))
        {
            return qe_invalid;
        }
    }
    else
    {
        if(keepdata==true)
        {
            init_item_drop_sets();
        }
    }
    
    if(s_version>=1)
    {
        for(int i=0; i<item_drop_sets_to_read; i++)
        {
            if(!pfread(tempitemdrop.name,sizeof(tempitemdrop.name),f,true))
            {
                return qe_invalid;
            }
            
            for(int j=0; j<10; ++j)
            {
                if(!p_igetw(&tempitemdrop.item[j],f,true))
                {
                    return qe_invalid;
                }
            }
            
            for(int j=0; j<11; ++j)
            {
                if(!p_igetw(&tempitemdrop.chance[j],f,true))
                {
                    return qe_invalid;
                }
            }
            
            // Dec 2008: Addition of the 'Tall Grass' set, #12,
            // overrides the quest's set #12.
            if(s_version<2 && i==12)
                continue;
                
            // Deprecated: qr_NOCLOCKS and qr_ALLOW10RUPEEDROPS
            if(s_version<2) for(int j=0; j<10; ++j)
                {
                    int it = tempitemdrop.item[j];
                    
                    if((itemsbuf[it].family == itype_rupee
                            && ((itemsbuf[it].amount)&0xFFF) == 10)
                            && !get_bit(deprecated_rules, qr_ALLOW10RUPEEDROPS_DEP))
                    {
                        tempitemdrop.chance[j+1]=0;
                    }
                    else if(itemsbuf[it].family == itype_clock && get_bit(deprecated_rules, qr_NOCLOCKS_DEP))
                    {
                        tempitemdrop.chance[j+1]=0;
                    }
                    
                    // From Sept 2007 to Dec 2008, non-gameplay items were prohibited.
                    if(itemsbuf[it].family == itype_misc)
                    {
                        // If a non-gameplay item was selected, then item drop was aborted.
                        // Reflect this by increasing the 'Nothing' chance accordingly.
                        tempitemdrop.chance[0]+=tempitemdrop.chance[j+1];
                        tempitemdrop.chance[j+1]=0;
                    }
                }
                
            if(keepdata)
            {
                memcpy(&item_drop_sets[i], &tempitemdrop, sizeof(item_drop_object));
            }
        }
    }
    
    return 0;
}

int readfavorites(PACKFILE *f, int, word, bool keepdata)
{
    int temp_num;
    dword dummy_dword;
    word num_favorite_combos;
    word num_favorite_combo_aliases;
    word s_version=0, s_cversion=0;
    
    //section version info
    if(!p_igetw(&s_version,f,true))
    {
        return qe_invalid;
    }
    
    ffcore.quest_format[vFavourites] = s_version;
    
    if(!p_igetw(&s_cversion,f,true))
    {
        return qe_invalid;
    }
    
    //section size
    if(!p_igetl(&dummy_dword,f,true))
    {
        return qe_invalid;
    }
    
    //finally...  section data
    if(!p_igetw(&num_favorite_combos,f,true))
    {
        return qe_invalid;
    }
    
    for(int i=0; i<num_favorite_combos; i++)
    {
        if(!p_igetl(&temp_num,f,true))
        {
            return qe_invalid;
        }
        
        if(keepdata)
        {
            favorite_combos[i]=temp_num;
        }
    }
    
    if(!p_igetw(&num_favorite_combo_aliases,f,true))
    {
        return qe_invalid;
    }
    
    for(int i=0; i<num_favorite_combo_aliases; i++)
    {
        if(!p_igetl(&temp_num,f,true))
        {
            return qe_invalid;
        }
        
        if(keepdata)
        {
            favorite_comboaliases[i]=temp_num;
        }
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

const char *skip_text[skip_max]=
{
    "skip_header", "skip_rules", "skip_strings", "skip_misc",
    "skip_tiles", "skip_combos", "skip_comboaliases", "skip_csets",
    "skip_maps", "skip_dmaps", "skip_doors", "skip_items",
    "skip_weapons", "skip_colors", "skip_icons", "skip_initdata",
    "skip_guys", "skip_linksprites", "skip_subscreens", "skip_ffscript",
    "skip_sfx", "skip_midis", "skip_cheats", "skip_itemdropsets",
    "skip_favorites"
};


void port250QuestRules(){
	
	portCandleRules(); //Candle
	portBombRules();
	//FFScript::setFFRules();

}

void portCandleRules()
{
	bool hurtslink = get_bit(quest_rules,qr_FIREPROOFLINK);
	//itemdata itemsbuf;
	for ( int q = 0; q < MAXITEMS; q++ ) 
	{
		if ( itemsbuf[q].family == itype_candle )
		{
			if ( hurtslink ) itemsbuf[q].flags |= ITEM_FLAG2;
			else itemsbuf[q].flags &= ~ ITEM_FLAG2;
		}
	}
}

void portBombRules()
{
	bool hurtslink = get_bit(quest_rules,qr_OUCHBOMBS);
	//itemdata itemsbuf;
	for ( int q = 0; q < MAXITEMS; q++ ) 
	{
		if ( itemsbuf[q].family == itype_bomb )
		{
			if ( hurtslink ) itemsbuf[q].flags |= ITEM_FLAG2;
			else itemsbuf[q].flags &= ~ ITEM_FLAG2;
		}
	}
	
}

int loadquest(const char *filename, zquestheader *Header, miscQdata *Misc, zctune *tunes, bool show_progress, bool compressed, bool encrypted, bool keepall, byte *skip_flags)
{
    combosread=false;
    mapsread=false;
    fixffcs=false;
    
    if(get_debug()&&(key[KEY_LSHIFT]||key[KEY_RSHIFT]))
    {
        keepall=false;
        jwin_alert("Load Quest","Data retention disabled.",NULL,NULL,"OK",NULL,13,27,lfont);
    }
    
    //  show_progress=true;
    char tmpfilename[32];
    temp_name(tmpfilename);
//  char percent_done[30];
    bool catchup=false;
    byte tempbyte;
    word old_map_count=map_count;
    
    byte old_quest_rules[QUESTRULES_SIZE];
    byte old_extra_rules[EXTRARULES_SIZE];
    byte old_midi_flags[MIDIFLAGS_SIZE];
    
    if(keepall==false||get_bit(skip_flags, skip_rules))
    {
        memcpy(old_quest_rules, quest_rules, QUESTRULES_SIZE);
        memcpy(old_extra_rules, extra_rules, EXTRARULES_SIZE);
    }
    
    if(keepall==false||get_bit(skip_flags, skip_midis))
    {
        memcpy(old_midi_flags, midi_flags, MIDIFLAGS_SIZE);
    }
    
    
    if(keepall&&!get_bit(skip_flags, skip_ffscript))
    {
        zScript.clear();
        globalmap.clear();
        ffcmap.clear();
        itemmap.clear();
        
        for(int i=0; i<NUMSCRIPTFFC-1; i++)
        {
            ffcmap[i] = pair<string,string>("","");
        }
        
        globalmap[0] = pair<string,string>("Slot 1: ~Init", "~Init");
        
        for(int i=1; i<NUMSCRIPTGLOBAL; i++)
        {
            globalmap[i] = pair<string,string>("","");
        }
        
        //globalmap[3] = pair<string,string>("Slot 4: ~Continue", "~Continue");
        for(int i=0; i<NUMSCRIPTITEM-1; i++)
        {
            itemmap[i] = pair<string,string>("","");
        }
        
        reset_scripts();
    }
    
    zquestheader tempheader;
    memset(&tempheader, 0, sizeof(zquestheader));
    
    // oldquest flag is set when an unencrypted qst file is suspected.
    bool oldquest = false;
    int open_error=0;
    char deletefilename[1024];
    PACKFILE *f=open_quest_file(&open_error, filename, deletefilename, compressed, encrypted, show_progress);
    
    if(!f)
        return open_error;
        
    int ret=0;
    
    //header
    box_out("Reading Header...");
    ret=readheader(f, &tempheader, true);
    checkstatus(ret);
    box_out("okay.");
    box_eol();
    al_trace("Made in ZQuest %x Beta %d\n",tempheader.zelda_version, tempheader.build);
    
    if(tempheader.zelda_version>=0x193)
    {
        dword section_id;
        
        //section id
        if(!p_mgetl(&section_id,f,true))
        {
            return qe_invalid;
        }
        
        while(!pack_feof(f))
        {
            switch(section_id)
            {
            case ID_RULES:
            
                //rules
                if(catchup)
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
                if(catchup)
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
                if(catchup)
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
                if(catchup)
                {
                    box_out("found.");
                    box_eol();
                    catchup=false;
                }
                
                box_out("Reading Tiles...");
                ret=readtiles(f, newtilebuf, &tempheader, tempheader.zelda_version, tempheader.build, 0, NEWMAXTILES, false, keepall&&!get_bit(skip_flags, skip_tiles));
                checkstatus(ret);
                box_out("okay.");
                box_eol();
                break;
                
            case ID_COMBOS:
            
                //combos
                if(catchup)
                {
                    box_out("found.");
                    box_eol();
                    catchup=false;
                }
                
                box_out("Reading Combos...");
                ret=readcombos(f, &tempheader, tempheader.zelda_version, tempheader.build, 0, MAXCOMBOS, keepall&&!get_bit(skip_flags, skip_combos));
                combosread=true;
                checkstatus(ret);
                box_out("okay.");
                box_eol();
                break;
                
            case ID_COMBOALIASES:
            
                //combo aliases
                if(catchup)
                {
                    box_out("found.");
                    box_eol();
                    catchup=false;
                }
                
                box_out("Reading Combo Aliases...");
                ret=readcomboaliases(f, &tempheader, tempheader.zelda_version, tempheader.build, keepall&&!get_bit(skip_flags, skip_comboaliases));
                checkstatus(ret);
                box_out("okay.");
                box_eol();
                break;
                
            case ID_CSETS:
            
                //color data
                if(catchup)
                {
                    box_out("found.");
                    box_eol();
                    catchup=false;
                }
                
                box_out("Reading Color Data...");
                ret=readcolordata(f, Misc, tempheader.zelda_version, tempheader.build, 0, newerpdTOTAL, keepall&&!get_bit(skip_flags, skip_csets));
                checkstatus(ret);
                box_out("okay.");
                box_eol();
                break;
                
            case ID_MAPS:
            
                //maps
                if(catchup)
                {
                    box_out("found.");
                    box_eol();
                    catchup=false;
                }
                
                box_out("Reading Maps...");
                ret=readmaps(f, &tempheader, keepall&&!get_bit(skip_flags, skip_maps));
                mapsread=true;
                checkstatus(ret);
                box_out("okay.");
                box_eol();
                break;
                
            case ID_DMAPS:
            
                //dmaps
                if(catchup)
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
                if(catchup)
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
                if(catchup)
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
                if(catchup)
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
            
                //misc. colors
                if(catchup)
                {
                    box_out("found.");
                    box_eol();
                    catchup=false;
                }
                
                box_out("Reading Misc. Colors...");
                ret=readmisccolors(f, &tempheader, Misc, keepall&&!get_bit(skip_flags, skip_colors));
                checkstatus(ret);
                box_out("okay.");
                box_eol();
                break;
                
            case ID_ICONS:
            
                //game icons
                if(catchup)
                {
                    box_out("found.");
                    box_eol();
                    catchup=false;
                }
                
                box_out("Reading Game Icons...");
                ret=readgameicons(f, &tempheader, Misc, keepall&&!get_bit(skip_flags, skip_icons));
                checkstatus(ret);
                box_out("okay.");
                box_eol();
                break;
                
            case ID_INITDATA:
            
                //initialization data
                if(catchup)
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
                
                if(keepall&&!get_bit(skip_flags, skip_subscreens))
                {
                    if(zinit.subscreen!=ssdtMAX)  //not using custom subscreens
                    {
                        setupsubscreens();
                        
                        for(int i=0; i<MAXDMAPS; ++i)
                        {
                            int type=DMaps[i].type&dmfTYPE;
                            DMaps[i].active_subscreen=(type == dmOVERW || type == dmBSOVERW)?0:1;
                            DMaps[i].passive_subscreen=(get_bit(quest_rules,qr_ENABLEMAGIC))?0:1;
                        }
                    }
                }
                
                if(keepall&&!get_bit(skip_flags, skip_sfx))
                {
                    setupsfx();
                }
                
                if(keepall&&!get_bit(skip_flags, skip_itemdropsets))
                {
                    init_item_drop_sets();
                }
                
                if(keepall&&!get_bit(skip_flags, skip_favorites))
                {
                    init_favorites();
                }
                
                break;
                
            case ID_GUYS:
            
                //guys
                if(catchup)
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
                
            case ID_LINKSPRITES:
            
                //link sprites
                if(catchup)
                {
                    box_out("found.");
                    box_eol();
                    catchup=false;
                }
                
                box_out("Reading Custom Link Sprite Data...");
                ret=readlinksprites(f, &tempheader, keepall&&!get_bit(skip_flags, skip_linksprites));
                checkstatus(ret);
                box_out("okay.");
                box_eol();
                break;
                
            case ID_SUBSCREEN:
            
                //custom subscreens
                if(catchup)
                {
                    box_out("found.");
                    box_eol();
                    catchup=false;
                }
                
                box_out("Reading Custom Subscreen Data...");
                ret=readsubscreens(f, &tempheader, keepall&&!get_bit(skip_flags, skip_subscreens));
                checkstatus(ret);
                box_out("okay.");
                box_eol();
                break;
                
            case ID_FFSCRIPT:
            
                //Freeform combo scripts
                if(catchup)
                {
                    box_out("found.");
                    box_eol();
                    catchup=false;
                }
                
                box_out("Reading FF Script Data...");
                ret=readffscript(f, &tempheader, keepall&&!get_bit(skip_flags, skip_ffscript));
                checkstatus(ret);
                box_out("okay.");
                box_eol();
                break;
                
            case ID_SFX:
            
                //SFX data
                if(catchup)
                {
                    box_out("found.");
                    box_eol();
                    catchup=false;
                }
                
                box_out("Reading SFX Data...");
                ret=readsfx(f, &tempheader, keepall&&!get_bit(skip_flags, skip_sfx));
                checkstatus(ret);
                box_out("okay.");
                box_eol();
                break;
                
            case ID_MIDIS:
            
                //midis
                if(catchup)
                {
                    box_out("found.");
                    box_eol();
                    catchup=false;
                }
                
                box_out("Reading Tunes...");
                ret=readtunes(f, &tempheader, tunes, keepall&&!get_bit(skip_flags, skip_midis));
                checkstatus(ret);
                box_out("okay.");
                box_eol();
                break;
                
            case ID_CHEATS:
            
                //cheat codes
                if(catchup)
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
                
            case ID_ITEMDROPSETS:
            
                //item drop sets
                if(catchup)
                {
                    box_out("found.");
                    box_eol();
                    catchup=false;
                }
                
                box_out("Reading Item Drop Sets...");
                ret=readitemdropsets(f, tempheader.zelda_version, tempheader.build, keepall&&!get_bit(skip_flags, skip_itemdropsets));
                checkstatus(ret);
                box_out("okay.");
                box_eol();
                break;
                
            case ID_FAVORITES:
            
                //favorite combos and combo aliases
                if(catchup)
                {
                    box_out("found.");
                    box_eol();
                    catchup=false;
                }
                
                box_out("Reading Favorite Combos...");
                ret=readfavorites(f, tempheader.zelda_version, tempheader.build, keepall&&!get_bit(skip_flags, skip_favorites));
                checkstatus(ret);
                box_out("okay.");
                box_eol();
                break;
                
            default:
                if(!catchup)
                {
                    box_out("Bad token!  Searching...");
                    box_eol();
                }
                
                catchup=true;
                break;
            }
            
	    
            if(catchup)
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
                if(!pack_feof(f))
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
        mapsread=true;
        checkstatus(ret);
        box_out("okay.");
        box_eol();
        
        //combos
        box_out("Reading Combos...");
        ret=readcombos(f, &tempheader, tempheader.zelda_version, tempheader.build, 0, MAXCOMBOS, keepall&&!get_bit(skip_flags, skip_combos));
        combosread=true;
        checkstatus(ret);
        box_out("okay.");
        box_eol();
        
        //color data
        box_out("Reading Color Data...");
        ret=readcolordata(f, Misc, tempheader.zelda_version, tempheader.build, 0, newerpdTOTAL, keepall&&!get_bit(skip_flags, skip_csets));
        checkstatus(ret);
        box_out("okay.");
        box_eol();
        
        //tiles
        box_out("Reading Tiles...");
        ret=readtiles(f, newtilebuf, &tempheader, tempheader.zelda_version, tempheader.build, 0, NEWMAXTILES, false, keepall&&!get_bit(skip_flags, skip_tiles));
        checkstatus(ret);
        box_out("okay.");
        box_eol();
        
        //midis
        box_out("Reading Tunes...");
        ret=readtunes(f, &tempheader, tunes, keepall&&!get_bit(skip_flags, skip_midis));
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
        
        if(keepall&&!get_bit(skip_flags, skip_subscreens))
        {
            setupsubscreens();
            
            for(int i=0; i<MAXDMAPS; ++i)
            {
                int type=DMaps[i].type&dmfTYPE;
                DMaps[i].active_subscreen=(type == dmOVERW || type == dmBSOVERW)?0:1;
                DMaps[i].passive_subscreen=(get_bit(quest_rules,qr_ENABLEMAGIC))?0:1;
            }
        }
        
        box_out("Setting Up Default Sound Effects...");
        
        if(keepall&&!get_bit(skip_flags, skip_sfx))
            setupsfx();
            
        box_out("okay.");
        box_eol();
        
        //link sprites
        box_out("Reading Custom Link Sprite Data...");
        ret=readlinksprites2(f, -1, 0, keepall&&!get_bit(skip_flags, skip_linksprites));
        checkstatus(ret);
        box_out("okay.");
        box_eol();
        
        box_out("Setting Up Default Item Drop Sets...");
        ret=readitemdropsets(f, -1, 0, keepall&&!get_bit(skip_flags, skip_itemdropsets));
        box_out("okay.");
        box_eol();
    }
    
    // check data
    if(f)
    {
        pack_fclose(f);
    }
    
    if(!oldquest)
    {
        if(exists(tmpfilename))
        {
            delete_file(tmpfilename);
        }
    }
    
    if(fixffcs && combosread && mapsread)
    {
        for(int i=0; i<map_count; i++)
        {
            for(int j=0; j<MAPSCRS; j++)
            {
                for(int m=0; m<32; m++)
                {
                    if(combobuf[TheMaps[(i*MAPSCRS)+j].ffdata[m]].type == cCHANGE)
                        TheMaps[(i*MAPSCRS)+j].ffflags[m]|=ffCHANGER;
                }
            }
        }
    }
    
    if(get_bit(quest_rules, qr_CONTFULL_DEP))
    {
        set_bit(quest_rules, qr_CONTFULL_DEP, 0);
        set_bit(zinit.misc, idM_CONTPERCENT, 1);
        zinit.cont_heart=100;
        zinit.start_heart=zinit.hc;
    }
    
    box_out("Done.");
    box_eol();
    box_end(false);
    
//  if (keepall==true||!get_bit(skip_flags, skip_header))
    if(keepall&&!get_bit(skip_flags, skip_header))
    {
        memcpy(Header, &tempheader, sizeof(tempheader));
    }
    
    if(!keepall||get_bit(skip_flags, skip_maps))
    {
        map_count=old_map_count;
    }
    
    if(!keepall||get_bit(skip_flags, skip_rules))
    {
        memcpy(quest_rules, old_quest_rules, QUESTRULES_SIZE);
        memcpy(extra_rules, old_extra_rules, EXTRARULES_SIZE);
    }
    
    if(!keepall||get_bit(skip_flags, skip_midis))
    {
        memcpy(midi_flags, old_midi_flags, MIDIFLAGS_SIZE);
    }
    
    if(deletefilename[0] && exists(deletefilename))
    {
        delete_file(deletefilename);
    }
    
    //Debug ffcore.quest_format[]
	al_trace("Quest made in ZC Version: %d\n", ffcore.quest_format[vZelda]);
	al_trace("Quest made in ZC Build: %d\n", ffcore.quest_format[vBuild]);
	al_trace("Quest Section 'Header' is Version: %d\n", ffcore.quest_format[vHeader]);
	al_trace("Quest Section 'Rules' is Version: %d\n", ffcore.quest_format[vRules]);
	al_trace("Quest Section 'Strings' is Version: %d\n", ffcore.quest_format[vStrings]);
	al_trace("Quest Section 'Misc' is Version: %d\n", ffcore.quest_format[vMisc]);
	al_trace("Quest Section 'Tiles' is Version: %d\n", ffcore.quest_format[vTiles]);
	al_trace("Quest Section 'Combos' is Version: %d\n", ffcore.quest_format[vCombos]);
	al_trace("Quest Section 'CSets' is Version: %d\n", ffcore.quest_format[vCSets]);
	al_trace("Quest Section 'Maps' is Version: %d\n", ffcore.quest_format[vMaps]);
	al_trace("Quest Section 'DMaps' is Version: %d\n", ffcore.quest_format[vDMaps]);
	al_trace("Quest Section 'Doors' is Version: %d\n", ffcore.quest_format[vDoors]);
	al_trace("Quest Section 'Items' is Version: %d\n", ffcore.quest_format[vItems]);
	al_trace("Quest Section 'Weapons' is Version: %d\n", ffcore.quest_format[vWeaponSprites]);
	al_trace("Quest Section 'Colors' is Version: %d\n", ffcore.quest_format[vColours]);
	al_trace("Quest Section 'Icons' is Version: %d\n", ffcore.quest_format[vIcons]);
	al_trace("Quest Section 'Gfx Pack' is Version: %d\n", ffcore.quest_format[vGfxPack]);
	al_trace("Quest Section 'InitData' is Version: %d\n", ffcore.quest_format[vInitData]);
	al_trace("Quest Section 'Guys' is Version: %d\n", ffcore.quest_format[vGuys]);
	al_trace("Quest Section 'MIDIs' is Version: %d\n", ffcore.quest_format[vMIDIs]);
	al_trace("Quest Section 'Cheats' is Version: %d\n", ffcore.quest_format[vCheats]);
	al_trace("Quest Section 'Save Format' is Version: %d\n", ffcore.quest_format[vSaveformat]);
	al_trace("Quest Section 'Combo Aliases' is Version: %d\n", ffcore.quest_format[vComboAliases]);
	al_trace("Quest Section 'Link Sprites' is Version: %d\n", ffcore.quest_format[vLinkSprites]);
	al_trace("Quest Section 'Subscreen' is Version: %d\n", ffcore.quest_format[vSubscreen]);
	al_trace("Quest Section 'Dropsets' is Version: %d\n", ffcore.quest_format[vItemDropsets]);
	al_trace("Quest Section 'FFScript' is Version: %d\n", ffcore.quest_format[vFFScript]);
	al_trace("Quest Section 'SFX' is Version: %d\n", ffcore.quest_format[vSFX]);
	al_trace("Quest Section 'Favorites' is Version: %d\n", ffcore.quest_format[vFavourites]);
    
    return qe_OK;
    
invalid:
    box_out("error.");
    box_eol();
    box_end(true);
    
    if(f)
    {
        pack_fclose(f);
    }
    
    if(!oldquest)
    {
        if(exists(tmpfilename))
        {
            delete_file(tmpfilename);
        }
        
        if(deletefilename[0] && exists(deletefilename))
        {
            delete_file(deletefilename);
        }
    }
    
    return qe_invalid;
    
}

/*** end of qst.cc ***/

