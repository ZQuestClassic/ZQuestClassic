#include "allegro/file.h"
#include "core/flags.h"
#include "base/general.h"
#include "base/zapp.h"
#include "core/qrs.h"
#include "zalleg/packfile.h"
#include "core/dmap.h"
#include "core/combo.h"
#include "core/msgstr.h"
#include "core/flags.h"
#include <memory>
#include <stdio.h>
#include <cstring>
#include <string>
#include <map>
#include <vector>
#include <assert.h>
#include <fmt/format.h>
#include "dialog/info.h"

#include "fmt/core.h"
#include "zalleg/zalleg.h"
#include "core/zdefs.h"
#include "zalleg/colors.h"
#include "tiles.h"
#include "zalleg/zsys.h"
#include "core/qst.h"
#include "defdata.h"
#include "subscr.h"
#include "sfx.h"
#include "base/md5.h"
#include "zc/replay.h"
#include "zc/zelda.h"
#include "zinfo.h"
#include "zc/ffscript.h"
#include "particles.h"
#include "core/misctypes.h"
#include "core/initdata.h"
#include "advanced_music.h"
#include "zsyssimple.h"

extern sprite_list  guys, items, Ewpns, Lwpns, chainlinks, decorations;
extern void setZScriptVersion(int32_t s_version);

bool read_ext_zinfo = false, read_zinfo = false;
bool loadquest_report = false;
char const* loading_qst_name = NULL;
static std::string last_loaded_qstpath;
static byte loading_qst_num = 0;
byte subscr_mode = ssdtMAX;
// Very old quests only used a byte for combos, and each screen had a "combo page" to vary
// what combos were used. This vector just lets us convert those old quests on load.
std::vector<byte> old_combo_pages;

dword loading_tileset_flags = 0;

int32_t First[MAX_COMBO_COLS]={0},combo_alistpos[MAX_COMBO_COLS]={0},combo_pool_listpos[MAX_COMBO_COLS]={0},combo_auto_listpos[MAX_COMBO_COLS]={0};
map_and_screen map_page[MAX_MAPPAGE_BTNS]= {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}};

#ifdef _MSC_VER
	#define strncasecmp _strnicmp
#endif

#ifndef _AL_MALLOC
#define _AL_MALLOC(a) _al_malloc(a)
#define _AL_FREE(a) _al_free(a)
#endif

using std::string;
using std::pair;

extern int32_t                 hero_animation_speed; //lower is faster animation
extern byte                *colordata;
extern tiledata            *newtilebuf;
extern byte                *trashbuf;
extern comboclass          *combo_class_buf;
extern ZCHEATS             zcheats;
extern char                palnames[MAXLEVELS][17];
string				             zScript;
std::map<int32_t, script_slot_data > ffcmap;
std::map<int32_t, script_slot_data > globalmap;
std::map<int32_t, script_slot_data > genericmap;
std::map<int32_t, script_slot_data > itemmap;
std::map<int32_t, script_slot_data > npcmap;
std::map<int32_t, script_slot_data > ewpnmap;
std::map<int32_t, script_slot_data > lwpnmap;
std::map<int32_t, script_slot_data > playermap;
std::map<int32_t, script_slot_data > dmapmap;
std::map<int32_t, script_slot_data > screenmap;
std::map<int32_t, script_slot_data > itemspritemap;
std::map<int32_t, script_slot_data > comboscriptmap;
std::map<int32_t, script_slot_data > subscreenmap;
bitstring midi_bitstr;

void free_newtilebuf();
bool combosread=false;
bool mapsread=false;
bool fixffcs=false;
bool fixpolsvoice=false;

// Each section had a separate, secondary versioning number, but it was never really utilized for
// its intended purpose so it's been removed. But we still write something in this field.
bool write_deprecated_section_cversion(word section_version, PACKFILE* f)
{
	return p_iputw(section_version, f);
}

bool read_deprecated_section_cversion(PACKFILE* f)
{
	word unused;
	return p_igetw(&unused, f);
}

void script_slot_data::update()
{
	switch (format)
	{
		case SCRIPT_FORMAT_INVALID:
			output = fmt::format("{} --{}", slotname, scriptname);
			break;
		case SCRIPT_FORMAT_DEFAULT:
		default:
			output = fmt::format("{} {}", slotname, scriptname);
	}
}

char qstdat_string[2048] = { 0 };

zinfo* load_tmp_zi = NULL;

const byte clavio[9]={97,109,111,110,103,117,115,0};

//enum { qe_OK, qe_notfound, qe_invalid, qe_version, qe_obsolete,
//       qe_missing, qe_internal, qe_pwd, qe_match, qe_minver };

const char *qst_error[] =
{
    "OK","File not found","Invalid quest file",
    "Version not supported","Obsolete version",
    "Missing new data"  ,                                     /* but let it pass in ZQuest */
    "Internal error occurred", "Invalid password",
    "Quest title doesn't match saved game", "Save file is for older version of quest; please start new save",
    "Out of memory", "File Debug Mode", "Canceled", "", "No quest assigned"
};

byte deprecated_rules[QUESTRULES_NEW_SIZE];

char *ordinal(int32_t num)
{
    static const char *ending[4] = {"st","nd","rd","th"};
    static char ord_str[8];
    
    char *end;
    int32_t t=(num%100)/10;
    int32_t n=num%10;
    
    if(n>=1 && n<4 && t!=1)
        end = (char *)ending[n-1];
    else
        end = (char *)ending[3];
        
    snprintf(ord_str, sizeof(ord_str), "%d%s",num%10000,end);
    return ord_str;
}

int32_t get_version_and_build(PACKFILE *f, word *version, word *build)
{
    int32_t ret;
    *version=0;
    *build=0;
    byte temp_map_count=map_count;
    
    zquestheader tempheader{};
    
    if(!f)
    {
        return qe_invalid;
    }
    
    ret=readheader(f, &tempheader);
    
    if(ret)
    {
        return ret;
    }
    
    map_count=temp_map_count;
    *version=tempheader.zelda_version;
    *build=tempheader.build;
    return 0;
}


bool find_section(PACKFILE *f, int32_t section_id_requested)
{

    if(!f)
    {
        return false;
    }
    
    int32_t section_id_read;
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
    if(!p_mgetl(&section_id_read,f))
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
            
            if(!p_getc(&tempbyte,f))
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
            if(!p_igetw(&dummy,f))
            {
                return false;
            }
            
            if(!p_igetw(&dummy,f))
            {
                return false;
            }
            
            //section size
            if(!p_igetl(&section_size,f))
            {
                return false;
            }
            
            //pack_fseek(f, section_size);
            while(section_size>65535)
            {
                pfread(tempbuf,65535,f);
                tempbuf[65535]=0;
                section_size-=65535;
            }
            
            if(section_size>0)
            {
                pfread(tempbuf,section_size,f);
                tempbuf[section_size]=0;
            }
        }
        
        //section id
        if(!p_mgetl(&section_id_read,f))
        {
            return false;
        }
    }
    
    return false;
}





bool valid_zqt(PACKFILE *f)
{
    if(!f)
        return false;
    //for now, everything else is valid
    return true;
}

bool valid_zqt(const char *filename)
{
    PACKFILE *f=NULL;
    bool isvalid;
    int32_t error;
    f=open_quest_file(&error, filename, false);
    
    if(!f)
    {
//      setPackfilePassword(NULL);
        return false;
    }
    
    isvalid=valid_zqt(f);
	
	clear_quest_tmpfile();
	pack_fclose(f);
    
//  setPackfilePassword(NULL);
    return isvalid;
}

/*
	.qst file history

	.qst files have always been compressed using allegro's packfiles.

	At some point, an encoding layer was added. The two layers look like this:

		1) The top layer is from us. See decode_file_007.
			[0-24]    Preamble "Zelda Classic Quest File"
			[25-28]   Initial decoding seed value.
			[29-X]    Allegro-compressed payload (AKA "packed" file), but XOR'd based on seed value
			[last 4]  Checksum

		2) The bottom layer is a "compressed packed file" from Allegro 4. The entire payload
			is XOR'd with a password (datapwd). Once that is undone, the first four bytes are "slh!",
			followed by a lzss compressed representation of the payload (from allergo' packfile compression).
			The oldest quests skip the password part.

	Simply, the job of this function is to peel away the top layer.

	With this second layer of encryption, the data isn't any more secure, and adds a significant delay
	in opening and saving files. There is no version field, so they decryption key is
	found via trial-by-error (very slow!)

	There are other file types of interest:
		- .zqt: quest template files, skips top-layer encryption pass
		- .qsu: "unencoded" (and uncompressed) files; skips encryption and compression (also makes the longtan password moot)
		- .qu?: same as above. automated backup files
		- .qb?: same as above. automated backup files
		- .qt?: compressed and encrypted (or not encrypted, as of May 2023)

	May 2023: .qst files are now saved without the top layer encoding, and no allegro packfile password. The first bytes of these
	files are now "slh!.AG ZC Enhanced Quest File".
	The following command will take an existing qst file and upgrade it: `./zquest -unencrypt-qst <input> <output>`
*/
PACKFILE *open_quest_file(int32_t *open_error, const char *filename, bool show_progress)
{
	if (show_progress)
	{
		box_start(1, "Loading Quest", get_zc_font(font_lfont), font, true);
	}

	auto unencrypted_result = try_open_maybe_legacy_encoded_file(filename, ENC_STR, nullptr, QH_NEWIDSTR, QH_IDSTR);
	if (unencrypted_result.decoded_pf)
		return unencrypted_result.decoded_pf;
	if (unencrypted_result.not_found)
	{
		*open_error = qe_notfound;
		return nullptr;
	}

	// Everything below here is legacy code - recently saved quest files will have
	// returned by now.
	// The only replay qst file that is still using this legacy encoding is `link_to_the_zelda.qst`.

	// Note: although this is primarily for loading .qst files, it can also handle all of the
	// file types mentioned in the comment above. No need to be told if the file being loaded
	// is encrypted or compressed, we can do some simple and fast checks to determine how to load it.
	bool top_layer_compressed = unencrypted_result.top_layer_compressed;
	bool compressed = unencrypted_result.compressed;
	bool encrypted = unencrypted_result.encrypted;

	char tmpfilename[512];
	temp_name(tmpfilename, sizeof(tmpfilename));
	char percent_done[30];
	int32_t current_method=0;
    
	PACKFILE *f;
	const char *passwd= encrypted ? datapwd : "";
    
	// oldquest flag is set when an unencrypted qst file is suspected.
	bool oldquest = false;
	int32_t ret;

	if(strcmp(filename, "default.qst")!=0)
	{
		box_out(filename);
	}
	else
	{
		box_out("new quest"); // Or whatever
	}
	box_out("...");
	box_eol();
	box_eol();
    
	if(encrypted)
	{
		box_out("Decrypting...");
		box_save_x();
		ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_MAX-1, top_layer_compressed, passwd);
        
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
				snprintf(percent_done, sizeof(percent_done), "%d%%", (current_method*100)/ENC_METHOD_MAX);
				box_out(percent_done);
				box_load_x();
				ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_211B9, strstr(filename, ".dat#")!=NULL, passwd);
			}
            
			if(ret==5)                                              //old encryption?
			{
				current_method++;
				snprintf(percent_done, sizeof(percent_done), "%d%%", (current_method*100)/ENC_METHOD_MAX);
				box_out(percent_done);
				box_load_x();
				ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_192B185, strstr(filename, ".dat#")!=NULL, passwd);
			}
            
			if(ret==5)                                              //old encryption?
			{
				current_method++;
				snprintf(percent_done, sizeof(percent_done), "%d%%", (current_method*100)/ENC_METHOD_MAX);
				box_out(percent_done);
				box_load_x();
				ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_192B105, strstr(filename, ".dat#")!=NULL, passwd);
			}
            
			if(ret==5)                                              //old encryption?
			{
				current_method++;
				snprintf(percent_done, sizeof(percent_done), "%d%%", (current_method*100)/ENC_METHOD_MAX);
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
	f = zalleg_pack_fopen_password(oldquest ? filename : tmpfilename, compressed ? F_READ_PACKED : F_READ, passwd);
	if(!f)
	{
		if((compressed==1)&&(errno==EDOM))
		{
			f = zalleg_pack_fopen_password(oldquest ? filename : tmpfilename, F_READ, passwd);
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
		delete_file(tmpfilename);
	}
    
	box_out("okay.");
	box_eol();
    
	return f;
}

PACKFILE *open_quest_template(zquestheader *Header, const char *filename, bool validate)
{
    PACKFILE *f=NULL;
    int32_t open_error=0;
 
    if (Header->templatepath[0] != 0)
    {
        // TODO: should be safe to remove this, no one seems to use custom quest templates.
        filename=Header->templatepath;
    }
    
    f=open_quest_file(&open_error, filename, false);
    
    if(!f)
    {
        return NULL;
    }
    
    if(validate)
    {
        if(!valid_zqt(f))
        {
			InfoDialog("Error", "Invalid Quest Template").show();
            pack_fclose(f);
			clear_quest_tmpfile();
            return NULL;
        }
    }
    
    return f;
}

static bool init_section(zquestheader *Header, int32_t section_id, miscQdata *Misc, zctune *tunes, bool validate, const char* filename)
{
	// The only time the player uses this is to init tiles for some quests 1.90 or older. See readtiles.
	if (get_app_id() == App::zelda)
		assert(section_id == ID_TILES);

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
    
    int32_t ret;
    word version, build;
    PACKFILE *f=NULL;
    
    //why is this here?
    /*
      if(colordata==NULL)
      return false;
      */
    
    //setPackfilePassword(datapwd);
    f=open_quest_template(Header, filename, validate);
    
    if(!f)  //no file, nothing to delete
    {
//	  setPackfilePassword(NULL);
        return false;
    }
    
    ret=get_version_and_build(f, &version, &build);
    
    if(ret||(version==0))
    {
        pack_fclose(f);
		clear_quest_tmpfile();
        
//	setPackfilePassword(NULL);
        return false;
    }
    
    if(!find_section(f, section_id))
    {
        al_trace("Can't find section!\n");
        pack_fclose(f);
        clear_quest_tmpfile();
        
        //setPackfilePassword(NULL);
        return false;
    }
    
    switch(section_id)
    {
    case ID_RULES:
        //rules
        ret=readrules(f, Header);
        break;
        
    case ID_STRINGS:
        //strings
        ret=readstrings(f, Header);
        break;
        
    case ID_MISC:
        //misc data
        ret=readmisc(f, Header, Misc);
        break;
        
    case ID_TILES:
        //tiles
        ret=readtiles(f, newtilebuf, Header, version, build, 0, NEWMAXTILES, true);
        break;
        
    case ID_COMBOS:
        //combos
        clear_combos();
        ret=readcombos(f, Header, version, build, 0, MAXCOMBOS);
        combosread=true;
        break;
        
    case ID_COMBOALIASES:
        //combos
        ret=readcomboaliases(f, Header, version, build);
        break;
        
    case ID_CSETS:
        //color data
        ret=readcolordata(f, Misc, version, build, 0, newerpdTOTAL);
        break;
        
    case ID_MAPS:
        //maps
        ret=readmaps(f, Header);
        mapsread=true;
        break;
        
    case ID_DMAPS:
        //dmaps
        ret=readdmaps(f, Header, version, build, 0, MAXDMAPS);
        break;
        
    case ID_DOORS:
        //door combo sets
        ret=readdoorcombosets(f, Header);
        break;
        
    case ID_ITEMS:
        //items
        ret=readitems(f, version, build);
        break;
        
    case ID_WEAPONS:
        //weapons
        ret=readweapons(f, Header);
        break;
        
    case ID_COLORS:
        //misc. colors
        ret=readmisccolors(f, Header, Misc);
        break;
        
    case ID_ICONS:
        //game icons
        ret=readgameicons(f, Header, Misc);
        break;
        
    case ID_INITDATA:
        //initialization data
        ret=readinitdata(f, Header);
        break;
        
    case ID_GUYS:
        //guys
        ret=readguys(f, Header);
        break;
        
    case ID_MIDIS:
        //midis
        ret=readmidis(f, Header, tunes);
        break;
        
    case ID_CHEATS:
        //cheat codes
        ret=readcheatcodes(f, Header);
        break;
        
    case ID_ITEMDROPSETS:
        //item drop sets
        // Why is this one commented out?
        //ret=readitemdropsets(f, (int32_t)version, (word)build);
        break;
        
    case ID_FAVORITES:
        // favorite combos and aliases
        ret=readfavorites(f, version);
        break;
        
    default:
        ret=-1;
        break;
    }
    
    pack_fclose(f);
    clear_quest_tmpfile();

    //setPackfilePassword(NULL);
    if(!ret)
    {
        return true;
    }
    
    return false;
}

bool init_tiles_for_190(bool validate, zquestheader *Header)
{
    return init_section(Header, ID_TILES, NULL, NULL, validate, "assets/190_tiles.qst");
}

bool init_tiles(bool validate, zquestheader *Header)
{
    return init_section(Header, ID_TILES, NULL, NULL, validate, "modules/classic/default.qst");
}

bool init_combos(bool validate, zquestheader *Header)
{
    return init_section(Header, ID_COMBOS, NULL, NULL, validate, "modules/classic/default.qst");
}

bool init_colordata(bool validate, zquestheader *Header, miscQdata *Misc)
{
    return init_section(Header, ID_CSETS, Misc, NULL, validate, "modules/classic/default.qst");
}

void init_spritelists()
{
	if(FFCore.quest_format[vZelda] < 0x255)
	{
		guys.setMax(255);
		items.setMax(255);
		Ewpns.setMax(255);
		Lwpns.setMax(255);
		chainlinks.setMax(255);
		decorations.setMax(255);
		particles.setMax(255);
	}
	else
	{
		guys.setMax(255);
		items.setMax(255);
		Ewpns.setMax(255);
		Lwpns.setMax(255);
		chainlinks.setMax(255);
		decorations.setMax(255);
		particles.setMax(255*((255*4)+1)); //255 per sprite that can use particles; guys, items, ewpns, lwpns, +HERO
	}
}

bool reset_items(bool validate, zquestheader *Header)
{
    bool ret = true;
    if (get_app_id() == App::zquest)
        ret = init_section(Header, ID_ITEMS, NULL, NULL, validate, "modules/classic/default.qst");
    
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
	bool ret = true;
    if (get_app_id() == App::zquest)
        ret = init_section(Header, ID_WEAPONS, NULL, NULL, validate, "modules/classic/default.qst");
    
    return ret;
}

bool reset_mapstyles(miscQdata *Misc)
{
    Misc->colors.blueframe_tile = 20044;
    Misc->colors.blueframe_cset = 0;
    Misc->colors.triforce_tile = 23461;
    Misc->colors.triforce_cset = 1;
    Misc->colors.triframe_tile = 18752;
    Misc->colors.triframe_cset = 1;
    Misc->colors.overworld_map_tile = 16990;
    Misc->colors.overworld_map_cset = 2;
    Misc->colors.HCpieces_tile = 21160;
    Misc->colors.HCpieces_cset = 8;
    Misc->colors.dungeon_map_tile = 19651;
    Misc->colors.dungeon_map_cset = 8;
    return true;
}

int32_t read_weap_data(weapon_data& data, PACKFILE* f)
{
	byte tempbyte;
	
	word v_weapon_data;
	if(!p_igetw(&v_weapon_data,f))
		return qe_invalid;
	
	if (!p_igetw(&(data.flags), f))
		return qe_invalid;
	
	if (!p_igetl(&(data.moveflags), f))
		return qe_invalid;
	
	if (!p_igetw(&(data.wflags), f))
		return qe_invalid;
	
	for (int32_t q = 0; q < WPNSPR_MAX; ++q)
	{
		if (v_weapon_data < 1)
		{
			if (!p_getc(&tempbyte, f))
				return qe_invalid;
			data.burnsprs[q] = tempbyte;
		}
		else if (!p_igetw(&(data.burnsprs[q]), f))
			return qe_invalid;
		if (!p_getc(&(data.light_rads[q]), f))
			return qe_invalid;
		if (v_weapon_data >= 3)
			if (!p_igetl(&(data.light_offsets[q]), f))
				return qe_invalid;
	}
	if (!p_getc(&(data.glow_shape), f))
		return qe_invalid;
	
	if (!p_igetl(&(data.override_flags), f))
		return qe_invalid;
	if (!p_igetl(&(data.tilew), f))
		return qe_invalid;
	if (!p_igetl(&(data.tileh), f))
		return qe_invalid;
	if (!p_igetl(&(data.hxsz), f))
		return qe_invalid;
	if (!p_igetl(&(data.hysz), f))
		return qe_invalid;
	if (!p_igetl(&(data.hzsz), f))
		return qe_invalid;
	if (!p_igetl(&(data.hxofs), f))
		return qe_invalid;
	if (!p_igetl(&(data.hyofs), f))
		return qe_invalid;
	if (!p_igetl(&(data.xofs), f))
		return qe_invalid;
	if (!p_igetl(&(data.yofs), f))
		return qe_invalid;
	
	if (!p_igetzf(&(data.step), f))
		return qe_invalid;
	
	if (!p_getc(&(data.unblockable), f))
		return qe_invalid;
	
	if (!p_igetl(&(data.timeout), f))
		return qe_invalid;
	
	if (!p_getc(&(data.imitate_weapon), f))
		return qe_invalid;
	if (!p_getc(&(data.default_defense), f))
		return qe_invalid;
	
	if (!p_getc(&(data.lift_level), f))
		return qe_invalid;
	if (!p_getc(&(data.lift_time), f))
		return qe_invalid;
	if (!p_igetzf(&(data.lift_height), f))
		return qe_invalid;
	
	if (v_weapon_data < 5)
	{
		if(!p_igetw(&(data.scrconfig.script), f))
			return qe_invalid;
		for(uint q = 0; q < 8; ++q)
			if(!p_igetl(&(data.scrconfig.run_args[q]), f))
				return qe_invalid;
	}
	else if (!p_getvar(&data.scrconfig, f))
		return qe_invalid;
	if(!p_igetw(&(data.pierce_count), f))
		return qe_invalid;
	
	if (v_weapon_data >= 2)
	{
		if (!p_igetzf(&(data.bounce_mult), f))
			return qe_invalid;
		if (!p_igetzf(&(data.bounce_add), f))
			return qe_invalid;
	}
	if (v_weapon_data >= 4)
	{
		if (!p_igetl(&(data.viewport_suspend_range), f))
			return qe_invalid;
		if (!p_igetl(&(data.viewport_despawn_range), f))
			return qe_invalid;
	}
	return 0;
}

int32_t get_qst_buffers()
{
    TheMaps.resize(MAPSCRS);
	old_combo_pages.resize(MAPSCRS);
	map_infos.resize(1);

    Z_message("OK\n");
    
    // The vast majority of finished quests (and I presume this will be consistent for all time) use < 1000 strings in total.
    // (Shoelace's "Hero of Dreams" uses 1415.)
    // So let's be a bit generous and allow 4096 initially.
    // In the rare event that a quest overshoots this mark, we'll reallocate to the full 65535 later.
    // I tested it and it worked without flaw on 6/6/11. - L.
	// 2022: bumped from 4096 to 8192 to avoid a bug where the Strings menu shows (None) strings when the list passes
	//       this threshold. Possibly some bug related to `msglistcache` to being reset?
	// See https://discord.com/channels/876899628556091432/992984989073416242
    msg_strings_size = 8192;
    
	MsgStrings = new MsgStr[msg_strings_size];
        
    //memset(MsgStrings, 0, sizeof(MsgStr)*msg_strings_size);
	for(auto q = 0; q < msg_strings_size; ++q)
	{
		MsgStrings[q].clear();
	}   
    
    if((DMaps=new dmap[MAXDMAPS])==NULL)
        return 0;

    
    
	combobuf.clear();
	combobuf.resize(MAXCOMBOS);
    
    
    if((colordata=(byte*)malloc(psTOTAL255))==NULL)
        return 0;
    
    free_newtilebuf();
    if((newtilebuf=(tiledata*)malloc(NEWMAXTILES*sizeof(tiledata)))==NULL)
        return 0;
        
    memset(newtilebuf, 0, NEWMAXTILES*sizeof(tiledata));
    //Z_message("Performed memset on tiles\n"); 
    clear_tiles(newtilebuf);
    //Z_message("Performed clear_tiles()\n"); 
    
    if (get_app_id() == App::zquest)
    {
        if((grabtilebuf=(tiledata*)malloc(NEWMAXTILES*sizeof(tiledata)))==NULL)
            return 0;
            
        memset(grabtilebuf, 0, NEWMAXTILES*sizeof(tiledata));
        clear_tiles(grabtilebuf);
    }
    
    if((trashbuf=(byte*)malloc(100000))==NULL)
        return 0;
    
	itemsbuf.clear();
	sprite_data_buf.clear();
    
	for (auto& guy : guysbuf)
		guy.clear();
    
    if((combo_class_buf=(comboclass*)malloc(sizeof(comboclass)*cMAX))==NULL)
        return 0;

	for(int32_t i=0; i<OLDMAXGUYS; i++)
	{
		default_guys[i].weap_data.viewport_suspend_range = 48;
	}
        
    return 1;
}


void free_newtilebuf()
{
    if(newtilebuf)
    {
        for(int32_t i=0; i<NEWMAXTILES; i++)
            if(newtilebuf[i].data)
                free(newtilebuf[i].data);
                
        free(newtilebuf);
	newtilebuf = 0;
    }
}

void free_grabtilebuf()
{
    if (get_app_id() == App::zquest)
    {
        if(grabtilebuf)
        {
            for(int32_t i=0; i<NEWMAXTILES; i++)
                if(grabtilebuf[i].data) free(grabtilebuf[i].data);
                
            free(grabtilebuf);
	    grabtilebuf = 0;
        }
    }
}

void del_qst_buffers()
{
    if(MsgStrings) delete[] MsgStrings;
    
	if (DMaps) delete[] DMaps;
    
	combobuf.clear();
    
    if(colordata) free(colordata);
	
    free_newtilebuf();
    free_grabtilebuf();
    
    if(trashbuf) free(trashbuf);
    
    itemsbuf.clear();
	sprite_data_buf.clear();
    
    if(combo_class_buf) free(combo_class_buf);
}

// Only use for reading parts of older quests (Header->zelda_version <= 0x192)
const byte* legacy_skip_flags;

void clear_combo(int32_t i)
{
	combobuf[i].clear();
}

void clear_combos()
{
    for(int32_t tmpcounter=0; tmpcounter<MAXCOMBOS; tmpcounter++)
        clear_combo(tmpcounter);
}


void get_questpwd(char *encrypted_pwd, int16_t pwdkey, char *pwd)
{
    char temp_pwd[30];
    memset(temp_pwd,0,30);
    
    if(pwdkey!=0)
    {
        memcpy(temp_pwd,encrypted_pwd,30);
        temp_pwd[29]=0;
        
        for(int32_t i=0; i<30; i++)
        {
            temp_pwd[i] -= pwdkey;
            int32_t t=pwdkey>>15;
            pwdkey = (pwdkey<<1)+t;
        }
    }
    
    memcpy(pwd,temp_pwd,30);
}


bool devpwd()
{
	#ifdef _DEBUG
	return true;
	#endif
	#if DEVLEVEL > 3
	return true;
	#endif
	return !strcmp(zc_get_config("dev","pwd","",App::zquest), (char*)clavio) || is_ci();
}
bool check_questpwd(zquestheader *Header, char *pwd)
{
	if(devpwd())
		return true;
	if((!strcmp(pwd, (char*)clavio)))
		return true;
	cvs_MD5Context ctx;
	uint8_t md5sum[16];
	
	cvs_MD5Init(&ctx);
	cvs_MD5Update(&ctx, (const uint8_t*)pwd, (unsigned)strlen(pwd));
	cvs_MD5Final(md5sum, &ctx);
	
	return (memcmp(Header->pwd_hash,md5sum,16)==0);
}

static char const* key_exts[KEYFILE_NUMTY] = {"key", "zcheat", "zpwd"};
static bool key_hashed[KEYFILE_NUMTY] = {false, true, false};
static char const* key_namestr[KEYFILE_NUMTY] = {"Master","Cheat","ZPwd"};

static bool check_keyfile(char const* path, char const* ext, bool hashed, char const* typestr, zquestheader* Header)
{
	char keyfilename[2048];
	replace_extension(keyfilename, path, ext, 2047);
	if(!exists(keyfilename))
		return false;
	bool ret = false;
	
	char password[QSTPWD_LEN] = {0};
	PACKFILE *fp = zalleg_pack_fopen_password(keyfilename, F_READ,"");
	char msg[80] = {0};
	pfread(msg, 80, fp);
	if(strcmp(msg,"ZQuest Auto-Generated Quest Password Key File.  DO NOT EDIT!"))
	{
		zprint2("Found %s Key File '%s' (invalid header)\n", typestr, keyfilename);
		pack_fclose(fp);
		return false;
	}
	int16_t ver;
	byte bld;
	p_igetw(&ver, fp);
	p_getc(&bld, fp);
	pfread(password, QSTPWD_LEN, fp, true);
	if(hashed)
	{
		char unhashed_pw[QSTPWD_LEN] = {0};
		
		char hashmap = 'Z';
		hashmap += 'Q';
		hashmap += 'U';
		hashmap += 'E';
		hashmap += 'S';
		hashmap += 'T';
		
		for ( int32_t q = 0; q < QSTPWD_LEN; ++q )
			unhashed_pw[q] = password[q] - hashmap;
		
		ret = check_questpwd(Header, unhashed_pw);
	}
	else ret = check_questpwd(Header, password);
	pack_fclose(fp);
	zprint2("Found %s Key File '%s' (%s access)\n",
		typestr, keyfilename, ret ? "valid" : "invalid");
	return ret;
}

bool check_keyfiles(char const* path, vector<uint> types, zquestheader* Header)
{
	char exedir[PATH_MAX] = {0};
	extract_name(path, exedir, FILENAMEALL);
	char const* paths[] = {path, exedir};
	for(uint keyty : types)
	{
		if(keyty >= KEYFILE_NUMTY)
			continue;
		for(char const* p : paths)
		{
			if(check_keyfile(p, key_exts[keyty], key_hashed[keyty], key_namestr[keyty], Header))
				return true;
		}
	}
	return false;
}

void print_quest_metadata(zquestheader const& tempheader, char const* path)
{
	zprint2("\n");
	zprint2("[QUEST METADATA]\n");
	if(path)
		zprint2("Path: %s\n", path);
	if(tempheader.title[0])
		zprint2("Title: %s\n", tempheader.title);
	zprint2("ZC Version: %s\n", tempheader.getVerStr());
	if(tempheader.new_version_id_date_day)
		zprint2("ZC Build Date: %d-%d-%d %s %s\n", tempheader.new_version_id_date_year, tempheader.new_version_id_date_month, tempheader.new_version_id_date_day, tempheader.build_timestamp, tempheader.build_timezone);
	if(tempheader.version[0])
		zprint2("Qst Version: %s\n", tempheader.version);
	if(tempheader.author[0])
		zprint2("Author: %s\n", tempheader.author);
	zprint2("\n");
}

int32_t count_dmaps()
{
    int32_t i=MAXDMAPS-1;
    bool found=false;
    
    while(i>=0 && !found)
    {
        if((DMaps[i].map!=0)||(DMaps[i].level!=0)||(DMaps[i].xoff!=0)||
                (DMaps[i].compass!=0)||(DMaps[i].color!=0)||(DMaps[i].music!=0)||
                (DMaps[i].cont!=0)||(DMaps[i].type!=0))
            found=true;
            
        for(int32_t j=0; j<8; j++)
        {
            if(DMaps[i].grid[j]!=0)
            
                found=true;
        }
        
        if((DMaps[i].name[0]!=0)||(DMaps[i].title[0]!=0)||
                (DMaps[i].intro[0]!=0))
            found=true;
            
        if((DMaps[i].minimap_tile[0]!=0)||(DMaps[i].minimap_tile[1]!=0)||
                (DMaps[i].largemap_tile[0]!=0)||(DMaps[i].largemap_tile[1]!=0)||
                (DMaps[i].minimap_cset[0]!=0)||(DMaps[i].minimap_cset[1]!=0)||
                (DMaps[i].largemap_cset[0]!=0)||(DMaps[i].largemap_cset[1]!=0))
            found=true;
            
        if(!found)
        {
            i--;
        }
    }
    
    return i+1;
}


int32_t count_shops(miscQdata *Misc)
{
    int32_t i=NUM_SHOPS-1,j;
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

int32_t count_infos(miscQdata *Misc)
{
    int32_t i=255,j;
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

int32_t count_warprings(miscQdata *Misc)
{
    int32_t i=15,j;
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

int32_t count_palcycles(miscQdata *Misc)
{
    int32_t i=255,j;
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

extern const char *old_item_string[iLast];
extern const char *old_weapon_string[wLast];

static bool did_init_def_items = false;
void init_def_items()
{
	if(did_init_def_items) return;
	did_init_def_items = true;
	default_items[3].cost_counter[1] = crBOMBS;
	default_items[13].cost_counter[1] = crARROWS;
	default_items[14].cost_counter[1] = crARROWS;
	default_items[48].cost_counter[1] = crSBOMBS;
	default_items[57].cost_counter[1] = crARROWS;
}
void reset_itembuf(itemdata *item, int32_t id)
{
	init_def_items();
	if(id<iLast)
	{
		// Copy everything *EXCEPT* the tile, misc, cset, frames, speed, delay and ltm.
		word tile = item->tile;
		byte miscs = item->misc_flags, cset = item->csets, frames = item->frames, speed = item->speed, delay = item->delay;
		int32_t ltm = item->ltm;
		
		auto name = item->name;
		*item = default_items[id];
		item->name = name;
		item->tile = tile;
		item->misc_flags = miscs;
		item->csets = cset;
		item->frames = frames;
		item->speed = speed;
		item->delay = delay;
		item->ltm = ltm;
	}
}

void init_favorites()
{
    for(int32_t i=0; i<MAXFAVORITECOMBOS; i++)
    {
        favorite_combos[i]=-1;
    }
}

void reset_subscreens()
{
	subscreens_active.clear();
	subscreens_passive.clear();
	subscreens_overlay.clear();
	subscreens_map.clear();
}

int32_t setupsubscreens()
{
    reset_subscreens();
	//return 0;
	for(int q = 0; q < 2; ++q)
	{
		subscreens_active.emplace_back();
		subscreens_passive.emplace_back();
	}
    int32_t tempsubscreen=subscr_mode;
    
    if(tempsubscreen>=ssdtMAX)
        tempsubscreen=0;
    
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
			subscreens_active[0].load_old(default_subscreen_active[tempsubscreen][0]);
			subscreens_active[0].sub_type=sstACTIVE;
			subscreens_active[0].name = "Active Subscreen (Triforce)";
			subscreens_active[1].load_old(default_subscreen_active[tempsubscreen][1]);
			subscreens_active[1].sub_type=sstACTIVE;
			subscreens_active[1].name = "Active Subscreen (Dungeon Map)";
			subscreens_passive[0].load_old(default_subscreen_passive[tempsubscreen][0]);
			subscreens_passive[0].sub_type=sstPASSIVE;
			subscreens_passive[0].name = "Passive Subscreen (Magic)";
			subscreens_passive[1].load_old(default_subscreen_passive[tempsubscreen][1]);
			subscreens_passive[1].sub_type=sstPASSIVE;
			subscreens_passive[1].name = "Passive Subscreen (No Magic)";
			break;
		}
		
		case ssdtZ3:
		{
			subscreens_active[0].load_old(z3_active_a);
			subscreens_active[0].sub_type=sstACTIVE;
			subscreens_active[1].load_old(z3_active_ab);
			subscreens_active[1].sub_type=sstACTIVE;
			subscreens_passive[0].load_old(z3_passive_a);
			subscreens_passive[0].sub_type=sstPASSIVE;
			subscreens_passive[1].load_old(z3_passive_ab);
			subscreens_passive[1].sub_type=sstPASSIVE;
			break;
		}
    }
    subscr_mode = ssdtMAX;
    return 0;
}

extern script_data *ffscripts[NUMSCRIPTFFC];
extern script_data *itemscripts[NUMSCRIPTITEM];
extern script_data *guyscripts[NUMSCRIPTGUYS];
extern script_data *lwpnscripts[NUMSCRIPTWEAPONS];
extern script_data *ewpnscripts[NUMSCRIPTWEAPONS];
extern script_data *globalscripts[NUMSCRIPTGLOBAL];
extern script_data *genericscripts[NUMSCRIPTSGENERIC];
extern script_data *playerscripts[NUMSCRIPTHERO];
extern script_data *screenscripts[NUMSCRIPTSCREEN];
extern script_data *dmapscripts[NUMSCRIPTSDMAP];
extern script_data *itemspritescripts[NUMSCRIPTSITEMSPRITE];
extern script_data *comboscripts[NUMSCRIPTSCOMBODATA];
extern script_data *subscreenscripts[NUMSCRIPTSSUBSCREEN];

void(*reset_scripts_hook)();

void reset_scripts()
{
	// We can't modify the script data while jit threads are possibly compiling them.
	if (reset_scripts_hook)
		reset_scripts_hook();

	for(int32_t i=0; i<NUMSCRIPTSGENERIC; i++)
	{
		if (genericscripts[i]!=NULL) genericscripts[i]->disable();
		else genericscripts[i] = new script_data({ScriptType::Generic, i});
	}

	for(int32_t i=0; i<NUMSCRIPTFFC; i++)
	{
		if (ffscripts[i])
			ffscripts[i]->disable();
		else
			ffscripts[i] = new script_data(ScriptType::FFC, i);
	}
	
	for(int32_t i=0; i<NUMSCRIPTITEM; i++)
	{
		if (itemscripts[i])
			itemscripts[i]->disable();
		else
			itemscripts[i] = new script_data(ScriptType::Item, i);
	}
	
	for(int32_t i=0; i<NUMSCRIPTGUYS; i++)
	{
		if (guyscripts[i])
			guyscripts[i]->disable();
		else
			guyscripts[i] = new script_data(ScriptType::NPC, i);
	}
	
	for(int32_t i=0; i<NUMSCRIPTSCREEN; i++)
	{
		if (screenscripts[i])
			screenscripts[i]->disable();
		else
			screenscripts[i] = new script_data(ScriptType::Screen, i);
	}
	
	for(int32_t i=0; i<NUMSCRIPTGLOBAL; i++)
	{
		if (globalscripts[i])
			globalscripts[i]->disable();
		else
			globalscripts[i] = new script_data(ScriptType::Global, i);
	}
	
	for(int32_t i=0; i<NUMSCRIPTHERO; i++)
	{
		if (playerscripts[i])
			playerscripts[i]->disable();
		else
			playerscripts[i] = new script_data(ScriptType::Hero, i);
	}
	
	for(int32_t i=0; i<NUMSCRIPTWEAPONS; i++)
	{
		if (lwpnscripts[i])
			lwpnscripts[i]->disable();
		else
			lwpnscripts[i] = new script_data(ScriptType::Lwpn, i);
	}
	 for(int32_t i=0; i<NUMSCRIPTWEAPONS; i++)
	{
		if (ewpnscripts[i])
			ewpnscripts[i]->disable();
		else
			ewpnscripts[i] = new script_data(ScriptType::Ewpn, i);
	}
	
	for(int32_t i=0; i<NUMSCRIPTSDMAP; i++)
	{
		if (dmapscripts[i])
			dmapscripts[i]->disable();
		else
			dmapscripts[i] = new script_data(ScriptType::DMap, i);
	}
	for(int32_t i=0; i<NUMSCRIPTSITEMSPRITE; i++)
	{
		if (itemspritescripts[i])
			itemspritescripts[i]->disable();
		else
			itemspritescripts[i] = new script_data(ScriptType::ItemSprite, i);
	}
	for(int32_t i=0; i<NUMSCRIPTSCOMBODATA; i++)
	{
		if (comboscripts[i])
			comboscripts[i]->disable();
		else
			comboscripts[i] = new script_data(ScriptType::Combo, i);
	}
	for(int32_t i=0; i<NUMSCRIPTSSUBSCREEN; i++)
	{
		if (subscreenscripts[i])
			subscreenscripts[i]->disable();
		else
			subscreenscripts[i] = new script_data(ScriptType::EngineSubscreen, i);
	}
}

extern int32_t sfxdat;
extern DATAFILE *sfxdata;

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
    "skip_guys", "skip_herosprites", "skip_subscreens", "skip_ffscript",
    "skip_sfx", "skip_midis", "skip_cheats", "skip_itemdropsets",
    "skip_favorites", "skip_zinfo", "skip_adv_music"
};

static int section_id_to_enum(int id)
{
	switch (id)
	{
		case ID_HEADER: return skip_header;
		case ID_RULES: return skip_rules;
		case ID_STRINGS: return skip_strings;
		case ID_MISC: return skip_misc;
		case ID_TILES: return skip_tiles;
		case ID_COMBOS: return skip_combos;
		case ID_COMBOALIASES: return skip_comboaliases;
		case ID_CSETS: return skip_csets;
		case ID_MAPS: return skip_maps;
		case ID_DMAPS: return skip_dmaps;
		case ID_DOORS: return skip_doors;
		case ID_ITEMS: return skip_items;
		case ID_WEAPONS: return skip_weapons;
		case ID_COLORS: return skip_colors;
		case ID_ICONS: return skip_icons;
		case ID_INITDATA: return skip_initdata;
		case ID_GUYS: return skip_guys;
		case ID_HEROSPRITES: return skip_herosprites;
		case ID_SUBSCREEN: return skip_subscreens;
		case ID_FFSCRIPT: return skip_ffscript;
		case ID_SFX: return skip_sfx;
		case ID_MIDIS: return skip_midis;
		case ID_CHEATS: return skip_cheats;
		case ID_ITEMDROPSETS: return skip_itemdropsets;
		case ID_FAVORITES: return skip_favorites;
		case ID_ZINFO: return skip_zinfo;
		case ID_ADVMUSIC: return skip_adv_music;
	}

	return -1;
}

static int maybe_skip_section(PACKFILE* f, dword& section_id, const byte* skip_flags)
{
	int section_enum = section_id_to_enum(section_id);
	bool skip = section_enum >= 0 && get_bit(skip_flags, section_enum);
	if (skip)
	{
		word s_version;
		if (!p_igetw(&s_version,f))
		{
			return qe_invalid;
		}

		if (!read_deprecated_section_cversion(f))
		{
			return qe_invalid;
		}

		if (section_id == ID_RULES && s_version > 16)
		{
			dword dummy;
			if (!p_igetl(&dummy,f))
			{
				return qe_invalid;
			}
		}

		if (section_id == ID_FFSCRIPT && s_version >= 18)
		{
			word dummy;
			if (!p_igetw(&dummy,f))
			{
				return qe_invalid;
			}
		}

		dword section_length;
		if (!p_igetl(&section_length,f))
		{
			return qe_invalid;
		}

		if (pack_fseek(f, section_length))
		{
			return qe_invalid;
		}

		if (!pack_feof(f))
		{
			if (!p_mgetl(&section_id,f))
			{
				return qe_invalid;
			}
		}

		return qe_cancel;
	}

	return qe_OK;
}

// TODO: this was copied from zc/zasm_utils.cpp
static void _zasm_for_every_script(std::function<void(zasm_script*)> fn)
{
	extern std::vector<std::shared_ptr<zasm_script>> zasm_scripts;

	std::vector<zasm_script*> scripts;
	scripts.reserve(zasm_scripts.size());
	for (auto& script : zasm_scripts)
		if (script->valid())
			scripts.push_back(script.get());

	std::for_each(scripts.begin(), scripts.end(), fn);
}

static bool compat_qr_hide_bottom_pixels(const zquestheader& header)
{
	// 2.55.9 or newer?
	if (header.compareVer(2, 55, 9) >= 0)
		return false; // defer to whatever was set

	// Replays created in 2.55 for quests prior to 2.55.9 should continue hiding the bottom pixels.
	if (replay_is_replaying() && replay_get_meta_str("zc_version_created").starts_with("2.55"))
		return true;

	// Quests prior to 2.55.9 with a scripted subscreen?
	for (int i = 0; i < MAXDMAPS; i++)
	{
		int script = DMaps[i].active_sub_scrconfig.script;
		if (script && dmapscripts[script] && dmapscripts[script]->valid())
			return true;
	}

	// Only a couple quests take any time (~7ms) on my intel mac to check all the ZASM... cache those.
	std::string title = header.title;
	if (title == "Yuurand: Tales of the Labyrinth")
		return true;

	// Look for ZASM setting values of 167, 168, etc. This is a sign that the script may be drawing something
	// near the old "bottom" of the screen, or is attempting to fill the entire screen with a draw command.
	// In these cases, the compat rule must be flipped on. As of writing, 72 quests in the PZC database match
	// this query: https://gist.github.com/connorjclark/edd12f84c9aac0c924ed328d3f8efcfa
	bool found = false;
	_zasm_for_every_script([&](auto script){
		if (found) return;

		for (const auto& instr : script->zasm)
		{
			if (!(instr.command == SETV || instr.command == PUSHV)) continue;

			int value = instr.arg2;
			if (value == 167000000 || value == 168000000 || value == 167870000 || value == 167910000 || value == 168130000)
			{
				found = true;
				break;
			}
		}
	});

	return found;
}

static int32_t prev_quest_format[versiontypesLAST];
static byte prev_quest_rules[QUESTRULES_NEW_SIZE];
static byte prev_extra_rules[EXTRARULES_SIZE];
static word prev_map_count;

// When skipping any section, we are loading a qst file just to poke at a couple things.
// We should not mutate important globals in that case.
// We should also restore these globals when loading a qst fails.
// Globals that are read by usecases of `skip_flags` will have to be restored manually by the caller
// (see load_imagebuf).
static void store_prev_qstload_global_state()
{
	memcpy(prev_quest_rules, quest_rules, QUESTRULES_NEW_SIZE);
	memcpy(prev_extra_rules, extra_rules, EXTRARULES_SIZE);
	memcpy(prev_quest_format, FFCore.quest_format, versiontypesLAST);
	prev_map_count = map_count;
}

static void restore_prev_qstload_global_state()
{
	memcpy(quest_rules, prev_quest_rules, QUESTRULES_NEW_SIZE);
	memcpy(extra_rules, prev_extra_rules, EXTRARULES_SIZE);
	unpack_qrs();
	memcpy(FFCore.quest_format, prev_quest_format, versiontypesLAST);
	map_count = prev_map_count;
}

//Internal function for loadquest wrapper
// TODO: refactor to never mutate global state, to make loading partial qst files easier and less error prone. huge project.
static int32_t _lq_int(const char *filename, zquestheader *Header, miscQdata *Misc, zctune *tunes, bool show_progress, byte *skip_flags, byte printmetadata)
{
    DMapEditorLastMaptileUsed = 0;
    combosread=false;
    mapsread=false;
    fixffcs=false;

	store_prev_qstload_global_state();

	bool skipping_any = false;
	for (int i = 0; i < 4; i++)
		skipping_any |= skip_flags[i] ? true : false;

	bool do_clear_scripts = !get_bit(skip_flags,skip_ffscript);
	if(loading_tileset_flags & TILESET_CLEARSCRIPTS)
	{
		set_bit(skip_flags, skip_ffscript, 1);
		setZScriptVersion(V_FFSCRIPT);
		FFCore.quest_format[vFFScript] = V_FFSCRIPT;
		FFCore.quest_format[vLastCompile] = V_FFSCRIPT;
		do_clear_scripts = true;
	}
	if(loading_tileset_flags & TILESET_CLEARMAPS)
	{
		set_bit(skip_flags, skip_maps, 1);
	}

    char tmpfilename[512];
    temp_name(tmpfilename, sizeof(tmpfilename));
    bool catchup=false;
    byte tempbyte;

    // oldquest flag is set when an unencrypted qst file is suspected.
    bool oldquest = false;
    int32_t open_error=0;
    PACKFILE *f=open_quest_file(&open_error, filename, show_progress);
    
    if (!f)
    {
        ASSERT(open_error != 0);
        return open_error;
    }
    
    memset(quest_rules, 0, QUESTRULES_NEW_SIZE);
	memset(extra_rules, 0, EXTRARULES_SIZE);
	unpack_qrs();

	if (!get_bit(skip_flags, skip_maps))
		Regions = {};
    
	if (!(get_bit(skip_flags, skip_dmaps) &&
		get_bit(skip_flags, skip_maps) &&
		get_bit(skip_flags, skip_adv_music)))
		quest_music.clear(); // dmaps/maps of old quests will create advanced music
	
	if(do_clear_scripts)
	{
		zScript.clear();
		globalmap.clear();
		genericmap.clear();
		ffcmap.clear();
		itemmap.clear();
		npcmap.clear();
		ewpnmap.clear();
		lwpnmap.clear();
		playermap.clear();
		dmapmap.clear();
		screenmap.clear();
		itemspritemap.clear();
		comboscriptmap.clear();
		subscreenmap.clear();
		
		for(int32_t i=0; i<NUMSCRIPTFFC-1; i++)
		{
			ffcmap[i].clear();
		}
		
		globalmap[0].slotname = "Slot 1:";
		globalmap[0].scriptname = "~Init";
		globalmap[0].update();
		
		for(int32_t i=1; i<NUMSCRIPTGLOBAL; i++)
		{
			globalmap[i].clear();
		}
		
		for(int32_t i=0; i<NUMSCRIPTITEM-1; i++)
		{
			itemmap[i].clear();
		}
		
		//new script types -- prevent carrying over to a quest that you load after reading them
		//e.g., a quest has an npc script, and you make a blank quest, that now believes that it has an npc script, too!
		for(int32_t i=0; i<NUMSCRIPTGUYS-1; i++)
		{
			npcmap[i].clear();
		}
		for(int32_t i=0; i<NUMSCRIPTWEAPONS-1; i++)
		{
			lwpnmap[i].clear();
		}
		for(int32_t i=0; i<NUMSCRIPTWEAPONS-1; i++)
		{
			ewpnmap[i].clear();
		}
		for(int32_t i=0; i<NUMSCRIPTHERO-1; i++)
		{
			playermap[i].clear();
		}
		for(int32_t i=0; i<NUMSCRIPTSDMAP-1; i++)
		{
			dmapmap[i].clear();
		}
		for(int32_t i=0; i<NUMSCRIPTSCREEN-1; i++)
		{
			screenmap[i].clear();
		}
		for(int32_t i=0; i<NUMSCRIPTSITEMSPRITE-1; i++)
		{
			itemspritemap[i].clear();
		}
		for(int32_t i=0; i<NUMSCRIPTSCOMBODATA-1; i++)
		{
			comboscriptmap[i].clear();
		}
		for(int32_t i=0; i<NUMSCRIPTSGENERIC-1; i++)
		{
			genericmap[i].clear();
		}
		for(int32_t i=0; i<NUMSCRIPTSSUBSCREEN-1; i++)
		{
			subscreenmap[i].clear();
		}
		
		reset_scripts();
	}
	
    zquestheader tempheader{};
	tempheader.filename = filename;
	zinfo tempzi;
	tempzi.clear();
	load_tmp_zi = &tempzi;

	char zinfofilename[2048];
	replace_extension(zinfofilename, filename, "zinfo", 2047);
    int32_t ret=0;
    
    //header
    box_out("Reading Header...");
    ret=readheader(f, &tempheader, printmetadata);
    checkstatus(ret);
    box_out("okay.");
    box_eol();
    
	if(read_zinfo)
	{
		box_out("Reading ZInfo - ");
		box_out(read_ext_zinfo ? "External..." : "Internal...");
		if(read_ext_zinfo)
		{
			PACKFILE *inf=zalleg_pack_fopen_password(zinfofilename, F_READ, "");
			ret=readzinfo(inf, tempzi, tempheader);
			if(inf) pack_fclose(inf);
			checkstatus(ret);
		}
		else
		{
			ret=readzinfo(f, tempzi, tempheader);
			checkstatus(ret);
		}
		box_out("okay.");
		box_eol();
	}
	
    if(tempheader.zelda_version>=0x193)
    {
        dword section_id;
        
        //section id
        if(!p_mgetl(&section_id,f))
        {
            goto invalid;
        }

        std::set<dword> seen_sections;
        
        while(!pack_feof(f))
        {
            if (seen_sections.contains(section_id))
                goto invalid;
            seen_sections.insert(section_id);

			if (int retval = maybe_skip_section(f, section_id, skip_flags); retval != qe_OK)
			{
				if (retval == qe_cancel)
					continue;
				checkstatus(retval);
			}

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
                ret=readrules(f, &tempheader);
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
                ret=readstrings(f, &tempheader);
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
                ret=readmisc(f, &tempheader, Misc);
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
                ret=readtiles(f, newtilebuf, &tempheader, tempheader.zelda_version, tempheader.build, 0, NEWMAXTILES, false);
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
                ret=readcombos(f, &tempheader, tempheader.zelda_version, tempheader.build, 0, MAXCOMBOS);
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
                ret=readcomboaliases(f, &tempheader, tempheader.zelda_version, tempheader.build);
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
                ret=readcolordata(f, Misc, tempheader.zelda_version, tempheader.build, 0, newerpdTOTAL);
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
                ret=readmaps(f, &tempheader);
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
                ret=readdmaps(f, &tempheader, tempheader.zelda_version, tempheader.build, 0, MAXDMAPS);
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
                ret=readdoorcombosets(f, &tempheader);
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
                ret=readitems(f, tempheader.zelda_version, tempheader.build);
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
                ret=readweapons(f, &tempheader);
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
                ret=readmisccolors(f, &tempheader, Misc);
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
                ret=readgameicons(f, &tempheader, Misc);
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
                ret=readinitdata(f, &tempheader);
                checkstatus(ret);
                box_out("okay.");
                box_eol();
                
                if(!get_bit(skip_flags, skip_subscreens))
                {
                    if(subscr_mode!=ssdtMAX)  //not using custom subscreens
                    {
                        setupsubscreens();
                        
                        for(int32_t i=0; i<MAXDMAPS; ++i)
                        {
                            int32_t type=DMaps[i].type&dmfTYPE;
                            DMaps[i].active_subscreen=(type == dmOVERW || type == dmBSOVERW)?0:1;
                            DMaps[i].passive_subscreen=(get_qr(qr_ENABLEMAGIC))?0:1;
                        }
                    }
                }
                
                if(!get_bit(skip_flags, skip_sfx))
                {
                    setupsfx();
                }
                
                if(!get_bit(skip_flags, skip_itemdropsets))
                {
                    init_item_drop_sets();
                }
                
                if(!get_bit(skip_flags, skip_favorites))
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
                ret=readguys(f, &tempheader);
                checkstatus(ret);
                box_out("okay.");
                box_eol();
                break;
                
            case ID_HEROSPRITES:
            
                //hero sprites
                if(catchup)
                {
                    box_out("found.");
                    box_eol();
                    catchup=false;
                }
                
                box_out("Reading Custom Hero Sprite Data...");
                ret=readherosprites(f, &tempheader);
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
                ret=readsubscreens(f);
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
                ret=readffscript(f, &tempheader);
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
                ret=readsfx(f, &tempheader);
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
                
                box_out("Reading MIDIs...");
                ret=readmidis(f, &tempheader, tunes);
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
                ret=readcheatcodes(f, &tempheader);
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
                ret=readitemdropsets(f, tempheader.zelda_version);
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
                ret=readfavorites(f, tempheader.zelda_version);
                checkstatus(ret);
                box_out("okay.");
                box_eol();
                break;
			
            case ID_ADVMUSIC:
            
                // advanced music
                if(catchup)
                {
                    box_out("found.");
                    box_eol();
                    catchup=false;
                }
                
                box_out("Reading Advanced Music...");
                ret = read_adv_music(f);
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
                
                if(!p_getc(&tempbyte,f))
                {
					goto invalid;
                }
                
                section_id+=tempbyte;
            }
            
            else
            {
                //section id
                if(!pack_feof(f))
                {
                    if(!p_mgetl(&section_id,f))
                    {
                        goto invalid;
                    }
                }
            }
        }
    }
    else
    {
		std::vector<std::tuple<std::string, int32_t, std::function<int32_t()>>> hardcoded_sections = {
			{ "Rules", ID_RULES, [&](){ return readrules(f, &tempheader); }},
			{ "Strings", ID_STRINGS, [&](){ return readstrings(f, &tempheader); }},
			{ "Doors", ID_DOORS, [&](){ return readdoorcombosets(f, &tempheader); }},
			{ "DMaps", ID_DMAPS, [&](){ return readdmaps(f, &tempheader, tempheader.zelda_version, tempheader.build, 0, MAXDMAPS); }},
			{ "Misc. Data", ID_MISC, [&](){ return readmisc(f, &tempheader, Misc); }},
			{ "Items", ID_ITEMS, [&](){ return readitems(f, tempheader.zelda_version, tempheader.build); }},
			{ "Weapons", ID_WEAPONS, [&](){ return readweapons(f, &tempheader); }},
			{ "Custom Guy Data", ID_GUYS, [&](){ return readguys(f, &tempheader); }},
			{ "Maps", ID_MAPS, [&](){ return readmaps(f, &tempheader); }},
			{ "Combos", ID_COMBOS, [&](){ return readcombos(f, &tempheader, tempheader.zelda_version, tempheader.build, 0, MAXCOMBOS); }},
			{ "Color Data", ID_CSETS, [&](){ return readcolordata(f, Misc, tempheader.zelda_version, tempheader.build, 0, newerpdTOTAL); }},
			{ "Tiles", ID_TILES, [&](){ return readtiles(f, newtilebuf, &tempheader, tempheader.zelda_version, tempheader.build, 0, NEWMAXTILES, false); }},
			{ "MIDIs", ID_MIDIS, [&](){ return readmidis(f, &tempheader, tunes); }},
			{ "Cheat Codes", ID_CHEATS, [&](){ return readcheatcodes(f, &tempheader); }},
			{ "Init. Data", ID_INITDATA, [&](){ return readinitdata(f, &tempheader); }},
			{ "Custom Hero Sprite Data", ID_HEROSPRITES, [&](){ return readherosprites2(f, -1); }},
			{ "Up Default Item Drop Sets", ID_ITEMDROPSETS, [&](){ return readitemdropsets(f, -1); }},
		};

		legacy_skip_flags = skip_flags;
		for (auto& [desc, section_id, fn] : hardcoded_sections)
		{
			int section_enum = section_id_to_enum(section_id);
			bool skip = section_enum >= 0 && get_bit(skip_flags, section_enum);
			if (skip)
			{
				// Nothing to read.
				if (section_id == ID_RULES)
					continue;
				if (section_id == ID_GUYS)
					continue;

				// Haven't looked at how to skip these, because we don't need to currently: the only
				// usage of skip_flags currently is all off except: header and tiles (see `load_imagebuf`).
				if (section_id == ID_MIDIS)
					continue;
				if (section_id == ID_CHEATS)
					continue;
				if (section_id == ID_INITDATA)
					continue;
				if (section_id == ID_HEROSPRITES)
					continue;
				if (section_id == ID_ITEMDROPSETS)
					continue;
			}

			// Would be nice, but old sections mostly did not save section sizes. We could advance by
			// a specific amount, but it'd be a lot of work to get it right. So, for old quests, let's just
			// read all the sections even if requested to skip some.
			// if (int retval = maybe_skip_section(f, section_id, skip_flags); retval != qe_OK)
			// {
			// 	if (retval == qe_cancel)
			// 		continue;
			// 	checkstatus(retval);
			// }

			box_out(fmt::format("Reading {}...", desc).c_str());
			ret = fn();
			checkstatus(ret);
			box_out("okay.");
			box_eol();
		}
		legacy_skip_flags = nullptr;

        if(!get_bit(skip_flags, skip_subscreens))
        {
            setupsubscreens();
            
            for(int32_t i=0; i<MAXDMAPS; ++i)
            {
                int32_t type=DMaps[i].type&dmfTYPE;
                DMaps[i].active_subscreen=(type == dmOVERW || type == dmBSOVERW)?0:1;
                DMaps[i].passive_subscreen=(get_qr(qr_ENABLEMAGIC))?0:1;
            }
        }
        
        box_out("Setting Up Default Sound Effects...");
        
        if(!get_bit(skip_flags, skip_sfx))
            setupsfx();
		
		if(!get_bit(skip_flags, skip_favorites))
			init_favorites();
            
        box_out("okay.");
        box_eol();
    }
    
	init_spritelists();
	
    // check data
    if(f)
    {
        pack_fclose(f);
    }
	clear_quest_tmpfile();
    
    if(!oldquest)
    {
        if(exists(tmpfilename))
        {
            delete_file(tmpfilename);
        }
    }
    
    if(fixffcs && combosread && mapsread)
    {
        for(int32_t i=0; i<map_count; i++)
        {
            for(int32_t j=0; j<MAPSCRS; j++)
            {
                int c = TheMaps[(i*MAPSCRS)+j].numFFC();
                for(int32_t m=0; m<c; m++)
                {
                    // ffc data is a file-controlled word; bound it against combobuf to avoid OOB read
                    word data = TheMaps[(i*MAPSCRS)+j].ffcs[m].data;
                    if(data < MAXCOMBOS && combobuf[data].type == cCHANGE)
                        TheMaps[(i*MAPSCRS)+j].ffcs[m].flags|=ffc_changer;
                }
            }
        }
    }

	if (!get_bit(skip_flags, skip_rules))
	{
		bool should_hide = compat_qr_hide_bottom_pixels(tempheader);
		al_trace("Note: qr_HIDE_BOTTOM_8_PIXELS %s via compat rule\n", should_hide ? "enabled" : "disabled");
		if (should_hide)
			set_qr(qr_HIDE_BOTTOM_8_PIXELS, 1);

		if (FFCore.quest_format[vCompatRule] < 77 && Header->is_z3)
			set_qr(qr_LAYER6_STRINGS_OVER_SUBSCREEN, 1);
	}

    if(get_qr(qr_CONTFULL_DEP) && !get_bit(skip_flags, skip_rules) && !get_bit(skip_flags, skip_initdata))
    {
        set_qr(qr_CONTFULL_DEP, 0);
        zinit.flags.set(INIT_FL_CONTPERCENT,true);
        zinit.cont_heart=100;
        zinit.counter[crLIFE]=zinit.mcounter[crLIFE];
    }
    
    box_out("Done.");
    box_eol();
    box_end(false);
    
    if(!get_bit(skip_flags, skip_header))
    {
        *Header = tempheader;
    }
    if(!get_bit(skip_flags, skip_zinfo))
    {
		ZI.copyFrom(tempzi);
    }
    
	if(loading_tileset_flags & TILESET_CLEARMAPS)
	{
		TheMaps.clear();
		TheMaps.resize(MAPSCRS*1);
		map_count = 1;
		map_infos.clear();
		map_infos.resize(1);
	}

	if(loading_tileset_flags & TILESET_CLEARHEADER)
	{
		memset(Header->password, 0, sizeof(Header->password));
		memset(Header->minver, 0, sizeof(Header->minver));
		memset(Header->title, 0, sizeof(Header->title));
		memset(Header->author, 0, sizeof(Header->author));
		memset(Header->version, 0, sizeof(Header->version));
		Header->use_keyfile = 0;
		Header->dirty_password = false;
		cvs_MD5Context ctx;
		cvs_MD5Init(&ctx);
		cvs_MD5Update(&ctx, (const uint8_t*)"", 0);
		cvs_MD5Final(Header->pwd_hash, &ctx);
	}

	if (!get_bit(skip_flags, skip_maps))
	{
		// Not needed, so release its memory.
		old_combo_pages = {};
	}

	if (skipping_any)
		restore_prev_qstload_global_state();

    return qe_OK;
    
invalid:
    // TODO: It's too easy to forget to jump to this label, and accidentally leave the file open.
    // Should wrap PACKFILE in a std::unique_pointer with a custom deallocator.
    box_out("error.");
    box_eol();
    box_end(true);
    
    pack_fclose(f);
    
    if(!oldquest)
    {
        if(exists(tmpfilename))
        {
            delete_file(tmpfilename);
        }
    }

	restore_prev_qstload_global_state();

    return qe_invalid;
    
}

std::string get_last_loaded_qstpath()
{
	return last_loaded_qstpath;
}

int32_t loadquest(const char *filename, zquestheader *Header, miscQdata *Misc,
	zctune *tunes, bool show_progress, byte *skip_flags, byte printmetadata,
	bool report, byte qst_num, dword tilesetflags)
{
	loading_tileset_flags = tilesetflags;
	const char* basename = get_filename(filename);
	zapp_reporting_add_breadcrumb("load_quest", basename);
	zapp_reporting_set_tag("qst.filename", basename);

	last_loaded_qstpath = filename;
	loading_qst_name = filename;
	loading_qst_num = qst_num;
	// In CI, builds are cached for replay tests, which can result in their build dates being earlier than what it would be locally.
	// So to avoid a more-recently updated .qst file from hitting the "last saved in a newer version" prompt, we disable in CI.
	if (!is_ci())
		loadquest_report = report;

	auto start = std::chrono::steady_clock::now();
	zprint2("Loading qst: %s\n", filename);
	int32_t ret = _lq_int(filename, Header, Misc, tunes, show_progress, skip_flags, printmetadata);
	int32_t load_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
	zprint2("Time to load qst: %d ms\n", load_ms);
	if (ret)
		zprint2("Error: %s\n", qst_error[ret]);

	if(show_progress)
	{
		if(ret)
		{
			box_out("-- Error loading quest file! --");
			box_out(fmt::format("Error: {}", qst_error[ret]).c_str());
			box_end(true);
		}
		else box_end(false);
	}

	load_tmp_zi = NULL;
	loading_qst_name = NULL;
	loadquest_report = false;
	loading_qst_num = 0;

	zapp_reporting_set_tag("qst.author", Header->author);
	zapp_reporting_set_tag("qst.title", Header->title);
	zapp_reporting_set_tag("qst.zc_version", Header->getVerStr());

	return ret;
}
