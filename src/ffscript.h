#ifndef _FFSCRIPT_H_
#define _FFSCRIPT_H_
#include "zdefs.h"
#include <utility>
#include <string>
#include <list>
#include "zelda.h" //This is probably the source of the duplication of BMP_MOUSE. -Z

#define ZS_BYTE 255
#define ZS_CHAR 255
#define ZS_WORD 65535
#define ZS_SHORT 32767
#define ZS_LONG 214747
#define ZS_INT 214747
#define ZS_FIX 214748
#define ZS_DWORD 65535

#define MAX_ZQ_LAYER 6
#define MAX_DRAW_LAYER 7
#define MIN_ZQ_LAYER 0
#define MIN_DRAW_LAYER 0
#define MAX_FLAGS 512

#define MIN_INTERNAL_BITMAP -1 //RT_SCREEN
#define MAX_INTERNAL_BITMAP 6 //RT_BITMAP6
#define FFRULES_SIZE 512

//unum FFCoreFlags[] { 
enum {
	FFCORE_SCRIPTED_MIDI_VOLUME 	= 0x0001,
	FFCORE_SCRIPTED_DIGI_VOLUME 	= 0x0002,
	FFCORE_SCRIPTED_MUSIC_VOLUME 	= 0x0004,
	FFCORE_SCRIPTED_SFX_VOLUME 	= 0x0008,
	FFCORE_SCRIPTED_PANSTYLE 	= 0x0010
};



	//Allow or forbid drawing during specific game events. 
	enum{
		scdrDRAW_WHILE_SCROLLING, scdrDRAW_DURING_SCREEN_TRANSITION, scdrDRAW_DURING_WARP,
		scdrDRAW_DURING_WIPES, scdrLAST
	};

enum
{
	UID_TYPE_NPC, UID_TYPE_WEAPON, UID_TYPE_ITEM, UID_TYPES
};

enum 
{
	vZelda, qQuestNumber, vBuild, vBeta, vHeader, vRules, vStrings, vMisc,
	vTiles, vCombos, vCSets, vMaps, vDMaps, vDoors, vItems, vWeaponSprites,
	vColours, vIcons, vGfxPack, vInitData, vGuys, vMIDIs, vCheats, vSaveformat,
	vComboAliases, vLinkSprites, vSubscreen, vItemDropsets, vFFScript, vSFX, vFavourites,
	qMapCount,
	versiontypesLAST
	
};

enum
{
	 qQuestVersion, qMinQuestVersion, qvLAST
};

class FFScript
{
	
    
public:
//FFScript();
void init();
int getQRBit(int rule);	
void setRule(int rule, bool s);
bool getRule(int rule_bit);
void setFFRules();
void setLinkTile(int t);
int getLinkTile();
void setLinkAction(int a);
int getLinkAction();

long getQuestHeaderInfo(int type);

/*
long getQuestHeaderInfo(int type)
{
    return quest_format[type];
}


*/

void do_adjustsfxvolume(const bool v);
void do_adjustvolume(const bool v);
//FFScript();
//static void init();

long quest_format[versiontypesLAST];


long coreflags;
long script_UIDs[UID_TYPES];
long usr_midi_volume, usr_digi_volume, usr_sfx_volume, usr_music_volume, usr_panstyle;
#define FFRULES_SIZE 512
byte FF_rules[FFRULES_SIZE]; //For Migration of Quest Rules, and Scritp Engine Rules
long FF_link_tile;	//Overrides for the tile used when blitting Limk to the bitmap; and a var to hold a script-set action/
byte FF_link_action; //This way, we can make safe replicas of internal Link actions to be set by script. 
bool kb_typing_mode; //script only, for disbaling key presses affecting Link, etc. 
long FF_screenbounds[4]; //edges of the screen, left, right, top, bottom used for where to scroll. 
long FF_screen_dimensions[4]; //height, width, displaywidth, displayheight
long FF_subscreen_dimensions[4];
long FF_eweapon_removal_bounds[4]; //left, right, top, bottom coordinates for automatic eweapon removal. 
long FF_lweapon_removal_bounds[4]; //left, right, top, bottom coordinates for automatic lweapon removal. 

#define FFSCRIPTCLASS_CLOCKS 10
long FF_clocks[FFSCRIPTCLASS_CLOCKS]; //Will be used for Linkaction, anims, and so forth 

#define SCRIPT_DRAWING_RULES 20
byte ScriptDrawingRules[SCRIPT_DRAWING_RULES];

#define NUM_USER_MIDI_OVERRIDES 6
long FF_UserMidis[NUM_USER_MIDI_OVERRIDES]; //MIDIs to use for Game Over, and similar to override system defaults. 
 
short passive_subscreen_offsets[2];
byte active_subscreen_scrollspeed_adjustment;

byte FF_gravity;
word FF_terminalv;
byte FF_msg_speed;
byte FF_transition_type; // Can't edit, yet.
byte FF_jump_link_layer_threshold; // Link is drawn above layer 3 if z > this.
byte FF_link_swim_speed;

byte subscreen_scroll_speed;

void setSubscreenScrollSpeed(byte n);
int getSubscreenScrollSpeed();
void do_fx_zap(const bool v);
void do_fx_wavy(const bool v);
void do_greyscale(const bool v);

int GetScriptObjectUID(int type);
    
byte item_messages_played[MAXITEMS]; //Each field is set when an item pickup message plays the first time per session
				//so that they do not play every time an item is collected, unless one of the flags is set for it.

void SetFFEngineFlag(int flag, bool v);
void SetItemMessagePlayed(int itm);
bool GetItemMessagePlayed(int itm);
	
int do_getpixel();
int GetDefaultWeaponSprite(int w);

int GetQuestVersion();
int GetQuestBuild();
int GetQuestSectionVersion(int section);

int do_get_internal_uid_npc(int i);
int do_get_internal_uid_item(int i);
int do_get_internal_uid_lweapon(int i);
int do_get_internal_uid_eweapon(int i);

void do_loadnpc_by_script_uid(const bool v);
void do_loaditem_by_script_uid(const bool v);
void do_loadlweapon_by_script_uid(const bool v);
void do_loadeweapon_by_script_uid(const bool v);
//int do_get_internal_uid

    //virtual ~FFScript();
    
	
	


	static INLINE int ZSbound_byte(int val)
	{
		return vbound(val,0,ZS_BYTE);
	}
	static INLINE int ZSbound_char(int val)
	{
		return vbound(val,0,ZS_CHAR);
	}
	static INLINE int ZSbound_word(int val)
	{
		return vbound(val,0,ZS_WORD);
	}
	static INLINE int ZSbound_short(int val)
	{
		return vbound(val,0,ZS_SHORT);
	}
	static INLINE int ZSbound_long(int val)
	{
		return vbound(val,0,ZS_LONG);
	}
	static INLINE int ZSbound_fix(int val)
	{
		return vbound(val,0,ZS_FIX);
	}
	
static void set_screenwarpReturnY(mapscr *m, int d, int value);
static void set_screendoor(mapscr *m, int d, int value);
static void set_screenenemy(mapscr *m, int index, int value);
static void set_screenlayeropacity(mapscr *m, int d, int value);
static void set_screensecretcombo(mapscr *m, int d, int value);
static void set_screensecretcset(mapscr *m, int d, int value);
static void set_screensecretflag(mapscr *m, int d, int value);
static void set_screenlayermap(mapscr *m, int d, int value);
static void set_screenlayerscreen(mapscr *m, int d, int value);
static void set_screenpath(mapscr *m, int d, int value);
static void set_screenwarpReturnX(mapscr *m, int d, int value);
static void set_screenWidth(mapscr *m, int value);
static void set_screenHeight(mapscr *m, int value);
static void set_screenViewX(mapscr *m, int value);
static void set_screenViewY(mapscr *m, int value);
static void set_screenGuy(mapscr *m, int value);
static void set_screenString(mapscr *m, int value);
static void set_screenRoomtype(mapscr *m, int value);
static void set_screenEntryX(mapscr *m, int value);
static void set_screenEntryY(mapscr *m, int value);
static void set_screenitem(mapscr *m, int value);
static void set_screenundercombo(mapscr *m, int value);
static void set_screenundercset(mapscr *m, int value);
static void set_screenatchall(mapscr *m, int value);
static long get_screenWidth(mapscr *m);
static long get_screenHeight(mapscr *m);
static void deallocateZScriptArray(const long ptrval);
static int get_screen_d(long index1, long index2);
static void set_screen_d(long index1, long index2, int val);
static int whichlayer(long scr);
static void clear_ffc_stack(const byte i);
static void clear_global_stack();
static void do_zapout();
static void do_zapin();
static void do_openscreen();
static void do_wavyin();
static void do_wavyout();
static void do_triggersecret(const bool v);
static void do_changeffcscript(const bool v);

static void setLinkDiagonal(bool v);
static bool getLinkDiagonal();
static bool getLinkBigHitbox();
static void setLinkBigHitbox(bool v);




//NPCData getters One Input, One Return
	static void getNPCData_flags(); //word
	static void getNPCData_flags2();
	static void getNPCData_width();
	static void getNPCData_height();
	static void getNPCData_s_tile();
	static void getNPCData_s_width();
	static void getNPCData_s_height();
	static void getNPCData_e_tile();
	static void getNPCData_e_width();
	static void getNPCData_hp();
	static void getNPCData_family();
	static void getNPCData_cset();
	static void getNPCData_anim();
	static void getNPCData_e_anim();
	static void getNPCData_frate();
	static void getNPCData_e_frate();
	static void getNPCData_dp();
	static void getNPCData_wdp();
	static void getNPCData_weapon();
	static void getNPCData_rate();
	static void getNPCData_hrate();
	static void getNPCData_step();
	static void getNPCData_homing();
	static void getNPCData_grumble();
	static void getNPCData_item_set();
	static void getNPCData_bgsfx();
	static void getNPCData_hitsfx();
	static void getNPCData_deadsfx();
	static void getNPCData_xofs();
	static void getNPCData_yofs();
	static void getNPCData_zofs();
	static void getNPCData_hxofs();
	static void getNPCData_hyofs();
	static void getNPCData_hxsz();
	static void getNPCData_hysz();
	static void getNPCData_hzsz();
	static void getNPCData_txsz();
	static void getNPCData_tysz();
	static void getNPCData_wpnsprite();

	//NPCData Getters, two inouts, one return



	//static void getNPCData_scriptdefence();
	static void getNPCData_misc();//switch-case
	static void getNPCData_defense(); //extra arg
	static void getNPCData_SIZEflags();


	//NPCData Setters, two inputs, no return.

	static void setNPCData_flags(); //word
	static void setNPCData_flags2();
	static void setNPCData_width();
	static void setNPCData_height();
	static void setNPCData_s_tile();
	static void setNPCData_s_width();
	static void setNPCData_s_height();
	static void setNPCData_e_tile();
	static void setNPCData_e_width();
	static void setNPCData_hp();
	static void setNPCData_family();
	static void setNPCData_cset();
	static void setNPCData_anim();
	static void setNPCData_e_anim();
	static void setNPCData_frate();
	static void setNPCData_e_frate();
	static void setNPCData_dp();
	static void setNPCData_wdp();
	static void setNPCData_weapon();
	static void setNPCData_rate();
	static void setNPCData_hrate();
	static void setNPCData_step();
	static void setNPCData_homing();
	static void setNPCData_grumble();
	static void setNPCData_item_set();
	static void setNPCData_bgsfx();
	static void setNPCData_hitsfx();
	static void setNPCData_deadsfx();
	static void setNPCData_xofs();
	static void setNPCData_yofs();
	static void setNPCData_zofs();
	static void setNPCData_hxofs();
	static void setNPCData_hyofs();
	static void setNPCData_hxsz();
	static void setNPCData_hysz();
	static void setNPCData_hzsz();
	static void setNPCData_txsz();
	static void setNPCData_tysz();
	static void setNPCData_wpnsprite();

	//NPCData Setters, three inputs, no return.
	//static void setNPCData_scriptdefence();
	static void setNPCData_defense(int v); //extra arg
	static void setNPCData_SIZEflags(int v);
	static void setNPCData_misc(int val);
	
	static void setNPCData_tile();
	static void setNPCData_e_height();
	static void getNPCData_tile();
	static void getNPCData_e_height();
	
	//one input, one return
	static void getComboData_block_enemies();
	static void getComboData_block_hole();
	static void getComboData_block_trigger();
	static void getComboData_conveyor_x_speed();
	static void getComboData_conveyor_y_speed();
	static void getComboData_create_enemy();
	static void getComboData_create_enemy_when();
	static void getComboData_create_enemy_change();
	static void getComboData_directional_change_type();
	static void getComboData_distance_change_tiles();
	static void getComboData_dive_item();
	static void getComboData_dock();
	static void getComboData_fairy();
	static void getComboData_ff_combo_attr_change();
	static void getComboData_foot_decorations_tile();
	static void getComboData_foot_decorations_type();
	static void getComboData_hookshot_grab_point();
	static void getComboData_ladder_pass();
	static void getComboData_lock_block_type();
	static void getComboData_lock_block_change();
	static void getComboData_magic_mirror_type();
	static void getComboData_modify_hp_amount();
	static void getComboData_modify_hp_delay();
	static void getComboData_modify_hp_type();
	static void getComboData_modify_mp_amount();
	static void getComboData_modify_mp_delay();
	static void getComboData_modify_mp_type();
	static void getComboData_no_push_blocks();
	static void getComboData_overhead();
	static void getComboData_place_enemy();
	static void getComboData_push_direction();
	static void getComboData_push_weight();
	static void getComboData_push_wait();
	static void getComboData_pushed();
	static void getComboData_raft();
	static void getComboData_reset_room();
	static void getComboData_save_point_type();
	static void getComboData_screen_freeze_type();

	static void getComboData_secret_combo();
	static void getComboData_singular();
	static void getComboData_slow_movement();
	static void getComboData_statue_type();
	static void getComboData_step_type();
	static void getComboData_step_change_to();
	static void getComboData_strike_remnants();
	static void getComboData_strike_remnants_type();
	static void getComboData_strike_change();
	static void getComboData_strike_item();
	static void getComboData_touch_item();
	static void getComboData_touch_stairs();
	static void getComboData_trigger_type();
	static void getComboData_trigger_sensitive();
	static void getComboData_warp_type();
	static void getComboData_warp_sensitive();
	static void getComboData_warp_direct();
	static void getComboData_warp_location();
	static void getComboData_water();
	static void getComboData_whistle();
	static void getComboData_win_game();
	static void getComboData_block_weapon_lvl();

	static void getComboData_tile();
	static void getComboData_flip();

	static void getComboData_walk();
	static void getComboData_type();
	static void getComboData_csets();
	static void getComboData_foo();
	static void getComboData_frames();
	static void getComboData_speed();
	static void getComboData_nextcombo();
	static void getComboData_nextcset();
	static void getComboData_flag();
	static void getComboData_skipanim();
	static void getComboData_nexttimer();
	static void getComboData_skipanimy();
	static void getComboData_animflags();

	//two inputs, one return
	static void getComboData_block_weapon();
	static void getComboData_expansion();
	static void getComboData_strike_weapons();

	//two inputs, no return
	static void setComboData_block_enemies();
	static void setComboData_block_hole();
	static void setComboData_block_trigger();
	static void setComboData_conveyor_x_speed();
	static void setComboData_conveyor_y_speed();
	static void setComboData_create_enemy();
	static void setComboData_create_enemy_when();
	static void setComboData_create_enemy_change();
	static void setComboData_directional_change_type();
	static void setComboData_distance_change_tiles();
	static void setComboData_dive_item();
	static void setComboData_dock();
	static void setComboData_fairy();
	static void setComboData_ff_combo_attr_change();
	static void setComboData_foot_decorations_tile();
	static void setComboData_foot_decorations_type();
	static void setComboData_hookshot_grab_point();
	static void setComboData_ladder_pass();
	static void setComboData_lock_block_type();
	static void setComboData_lock_block_change();
	static void setComboData_magic_mirror_type();
	static void setComboData_modify_hp_amount();
	static void setComboData_modify_hp_delay();
	static void setComboData_modify_hp_type();
	static void setComboData_modify_mp_amount();
	static void setComboData_modify_mp_delay();
	static void setComboData_modify_mp_type();
	static void setComboData_no_push_blocks();
	static void setComboData_overhead();
	static void setComboData_place_enemy();
	static void setComboData_push_direction();
	static void setComboData_push_weight();
	static void setComboData_push_wait();
	static void setComboData_pushed();
	static void setComboData_raft();
	static void setComboData_reset_room();
	static void setComboData_save_point_type();
	static void setComboData_screen_freeze_type();

	static void setComboData_secret_combo();
	static void setComboData_singular();
	static void setComboData_slow_movement();
	static void setComboData_statue_type();
	static void setComboData_step_type();
	static void setComboData_step_change_to();
	static void setComboData_strike_remnants();
	static void setComboData_strike_remnants_type();
	static void setComboData_strike_change();
	static void setComboData_strike_item();
	static void setComboData_touch_item();
	static void setComboData_touch_stairs();
	static void setComboData_trigger_type();
	static void setComboData_trigger_sensitive();
	static void setComboData_warp_type();
	static void setComboData_warp_sensitive();
	static void setComboData_warp_direct();
	static void setComboData_warp_location();
	static void setComboData_water();
	static void setComboData_whistle();
	static void setComboData_win_game();
	static void setComboData_block_weapon_lvl();

	static void setComboData_tile();
	static void setComboData_flip();

	static void setComboData_walk();
	static void setComboData_type();
	static void setComboData_csets();
	static void setComboData_foo();
	static void setComboData_frames();
	static void setComboData_speed();
	static void setComboData_nextcombo();
	static void setComboData_nextcset();
	static void setComboData_flag();
	static void setComboData_skipanim();
	static void setComboData_nexttimer();
	static void setComboData_skipanimy();
	static void setComboData_animflags();

	//three inputs, no return
	static void setComboData_block_weapon(int v);
	static void setComboData_expansion(int v);
	static void setComboData_strike_weapons(int v);
	
	//SpriteData
	static void getSpriteDataTile();
	static void getSpriteDataMisc();
	static void getSpriteDataCSets();
	static void getSpriteDataFrames();
	static void getSpriteDataSpeed();
	static void getSpriteDataType();
	static void getSpriteDataString();
	static void setSpriteDataTile();
	static void setSpriteDataMisc();
	static void setSpriteDataCSets();
	static void setSpriteDataFrames();
	static void setSpriteDataSpeed();
	static void setSpriteDataType();
	static void setSpriteDataString();

	//Change Save/Continue and Retry screen settings:
	static void FFSetSaveScreenSetting();
	static void FFChangeSubscreenText();
	
	static void do_typedpointer_typecast(const bool v);
	
	//New Datatypes
	static void do_loadnpcdata(const bool v);
	static void do_loadcombodata(const bool v);
	static void do_loadmapdata(const bool v);
	static long loadMapData();
	static void do_loadspritedata(const bool v);
	static void do_loadscreendata(const bool v);
	static void do_loadbitmapid(const bool v);
	static void do_loadshopdata(const bool v);
	static void do_loadinfoshopdata(const bool v);
	static void do_setMIDI_volume(int m);
	static void do_setMusic_volume(int m);
	static void do_setDIGI_volume(int m);
	static void do_setSFX_volume(int m);
	static void do_setSFX_pan(int m);
	
	static int do_getMIDI_volume();
	static int do_getMusic_volume();
	static int do_getDIGI_volume();
	static int do_getSFX_volume();
	static int do_getSFX_pan();
	
	static void do_loadmessagedata(const bool v);
	static void do_messagedata_setstring(const bool v);
	static void do_messagedata_getstring(const bool v);
	
	static void do_loaddmapdata(const bool v);
	static void do_getDMapData_dmapname(const bool v);
	static void do_setDMapData_dmapname(const bool v);
	static void do_getDMapData_dmaptitle(const bool v);
	static void do_setDMapData_dmaptitle(const bool v);
	static void do_getDMapData_dmapintro(const bool v);
	static void do_setDMapData_dmapintro(const bool v);
	static void do_getDMapData_music(const bool v);
	static void do_setDMapData_music(const bool v);

#define INVALIDARRAY localRAM[0]  //localRAM[0] is never used

enum __Error
    {
        _NoError, //OK!
        _Overflow, //script array too small
        _InvalidPointer, //passed NULL pointer or similar
        _OutOfBounds, //library array out of bounds
        _InvalidSpriteUID //bad npc, ffc, etc.
    };
    
    
    static INLINE int checkUserArrayIndex(const long index, const dword size)
    {
        if(index < 0 || index >= long(size))
        {
          //  Z_scripterrlog("Invalid index (%ld) to local array of size %ld\n", index, size);
            return _OutOfBounds;
        }
        
        return _NoError;
    }

    
    
    //only if the player is messing with their pointers...
    static ZScriptArray& InvalidError(const long ptr)
    {
        //Z_scripterrlog("Invalid pointer (%i) passed to array (don't change the values of your array pointers)\n", ptr);
        return INVALIDARRAY;
    }
    
    //Returns a reference to the correct array based on pointer passed
    static ZScriptArray& getArray(const long ptr)
    {
        if(ptr <= 0)
            return InvalidError(ptr);
            
        if(ptr >= MAX_ZCARRAY_SIZE) //Then it's a global
        {
            dword gptr = ptr - MAX_ZCARRAY_SIZE;
            
            if(gptr > game->globalRAM.size())
                return InvalidError(ptr);
                
            return game->globalRAM[gptr];
        }
        else
        {
            if(localRAM[ptr].Size() == 0)
                return InvalidError(ptr);
                
            return localRAM[ptr];
        }
    }
    
    static size_t getSize(const long ptr)
    {
        ZScriptArray& a = getArray(ptr);
        
        if(a == INVALIDARRAY)
            return size_t(-1);
            
        return a.Size();
    }
    
    //Can't you get the std::string and then check its length?
    static int strlen(const long ptr)
    {
        ZScriptArray& a = getArray(ptr);
        
        if(a == INVALIDARRAY)
            return -1;
            
        word count;
        
        for(count = 0; checkUserArrayIndex(count, a.Size()) == _NoError && a[count] != '\0'; count++) ;
        
        return count;
    }
    
    //Returns values of a zscript array as an std::string.
    static void getString(const long ptr, std::string &str, word num_chars = 256)
    {
        ZScriptArray& a = getArray(ptr);
        
        if(a == INVALIDARRAY)
        {
            str.clear();
            return;
        }
        
        str.clear();
        
        for(word i = 0; checkUserArrayIndex(i, a.Size()) == _NoError && a[i] != '\0' && num_chars != 0; i++)
        {
            str += char(a[i] / 10000);
            num_chars--;
        }
    }
    
    //Like getString but for an array of longs instead of chars. *(arrayPtr is not checked for validity)
    static void getValues(const long ptr, long* arrayPtr, word num_values)
    {
        ZScriptArray& a = getArray(ptr);
        
        if(a == INVALIDARRAY)
            return;
            
        for(word i = 0; checkUserArrayIndex(i, a.Size()) == _NoError && num_values != 0; i++)
        {
            arrayPtr[i] = (a[i] / 10000);
            num_values--;
        }
    }
    
    //Get element from array
    static INLINE long getElement(const long ptr, const long offset)
    {
        ZScriptArray& a = getArray(ptr);
        
        if(a == INVALIDARRAY)
            return -10000;
            
        if(checkUserArrayIndex(offset, a.Size()) == _NoError)
            return a[offset];
        else
            return -10000;
    }
    
    //Set element in array
    static INLINE void setElement(const long ptr, const long offset, const long value)
    {
        ZScriptArray& a = getArray(ptr);
        
        if(a == INVALIDARRAY)
            return;
            
        if(checkUserArrayIndex(offset, a.Size()) == _NoError)
            a[offset] = value;
    }
    
    //Puts values of a zscript array into a client <type> array. returns 0 on success. Overloaded
    template <typename T>
    static int getArray(const long ptr, T *refArray)
    {
        return getArray(ptr, getArray(ptr).Size(), 0, 0, 0, refArray);
    }
    
    template <typename T>
    static int getArray(const long ptr, const word size, T *refArray)
    {
        return getArray(ptr, size, 0, 0, 0, refArray);
    }
    
    template <typename T>
    static int getArray(const long ptr, const word size, word userOffset, const word userStride, const word refArrayOffset, T *refArray)
    {
        ZScriptArray& a = getArray(ptr);
        
        if(a == INVALIDARRAY)
            return _InvalidPointer;
            
        word j = 0, k = userStride;
        
        for(word i = 0; j < size; i++)
        {
            if(i >= a.Size())
                return _Overflow;
                
            if(userOffset-- > 0)
                continue;
                
            if(k > 0)
                k--;
            else if (checkUserArrayIndex(i, a.Size()) == _NoError)
            {
                refArray[j + refArrayOffset] = T(a[i]);
                k = userStride;
                j++;
            }
        }
        
        return _NoError;
    }
    
    
    static int setArray(const long ptr, const std::string s2)
    {
        ZScriptArray& a = getArray(ptr);
        
        if(a == INVALIDARRAY)
            return _InvalidPointer;
            
        word i;
        
        for(i = 0; i < s2.size(); i++)
        {
            if(i >= a.Size())
            {
                a.Back() = '\0';
                return _Overflow;
            }
            
            if(checkUserArrayIndex(i, a.Size()) == _NoError)
                a[i] = s2[i] * 10000;
        }
        
        if(checkUserArrayIndex(i, a.Size()) == _NoError)
            a[i] = '\0';
            
        return _NoError;
    }
    
    
    //Puts values of a client <type> array into a zscript array. returns 0 on success. Overloaded
    template <typename T>
    static int setArray(const long ptr, const word size, T *refArray)
    {
        return setArray(ptr, size, 0, 0, 0, refArray);
    }
    
    template <typename T>
    static int setArray(const long ptr, const word size, word userOffset, const word userStride, const word refArrayOffset, T *refArray)
    {
        ZScriptArray& a = getArray(ptr);
        
        if(a == INVALIDARRAY)
            return _InvalidPointer;
            
        word j = 0, k = userStride;
        
        for(word i = 0; j < size; i++)
        {
            if(i >= a.Size())
                return _Overflow; //Resize?
                
            if(userOffset-- > 0)
                continue;
                
            if(k > 0)
                k--;
            else if(checkUserArrayIndex(i, a.Size()) == _NoError)
            {
                a[i] = long(refArray[j + refArrayOffset]) * 10000;
                k = userStride;
                j++;
            }
        }
        
        return _NoError;
    }
    
    private:
    long sid;
  
    
};

extern long ffmisc[32][16];
extern refInfo ffcScriptData[32];

extern PALETTE tempgreypal; //script greyscale

long get_register(const long arg);
int run_script(const byte type, const word script, const byte i = -1); //Global scripts don't need 'i'
int ffscript_engine(const bool preload);

void clear_ffc_stack(const byte i);
void clear_global_stack();
void deallocateArray(const long ptrval);
void clearScriptHelperData();

void do_getscreenflags();
void do_getscreeneflags();
long get_screendoor(mapscr *m, int d);
long get_screenlayeropacity(mapscr *m, int d);
long get_screensecretcombo(mapscr *m, int d);
long get_screensecretcset(mapscr *m, int d);
long get_screensecretflag(mapscr *m, int d);
long get_screenlayermap(mapscr *m, int d);
long get_screenlayerscreen(mapscr *m, int d);
long get_screenpath(mapscr *m, int d);
long get_screenwarpReturnX(mapscr *m, int d);
long get_screenwarpReturnY(mapscr *m, int d);

long get_screenViewX(mapscr *m);
long get_screenGuy(mapscr *m);
long get_screenString(mapscr *m);
long get_screenRoomtype(mapscr *m);
long get_screenViewY(mapscr *m);
long get_screenEntryX(mapscr *m);
long get_screenEntryY(mapscr *m);
long get_screenitem(mapscr *m);
long get_screenundercombo(mapscr *m);
long get_screenundercset(mapscr *m);
long get_screenatchall(mapscr *m);
void do_getscreenLayerOpacity();
void do_getscreenSecretCombo();
void do_getscreenSecretCSet();
void do_getscreenSecretFlag();
void do_getscreenLayerMap();
void do_getscreenLayerscreen();
void do_getscreenPath();
void do_getscreenWarpReturnX();
void do_getscreenWarpReturnY();
void do_getscreenatchall();
void do_getscreenUndercombo();
void do_getscreenUnderCSet();
void do_getscreenWidth();
void do_getscreenHeight();
void do_getscreenViewX();
void do_getscreenGuy();
void do_getscreenString();
void do_getscreenRoomType();
void do_getscreenEntryX();
void do_getscreenEntryY();
void do_getscreenItem();
void do_getscreendoor();
long get_screennpc(mapscr *m, int index);
void do_getscreennpc();



struct script_command
{
    char name[64];
    byte args;
    byte arg1_type; //0=reg, 1=val;
    byte arg2_type; //0=reg, 1=val;
    bool more_stuff;
};

struct script_variable
{
    char name[64];
    long id;
    word maxcount;
    byte multiple;
};


// Defines for script flags
#define TRUEFLAG          0x0001
#define MOREFLAG          0x0002
#define TRIGGERED         0x0004

//What are these for exactly?
//#define fflong(x,y,z)       (((x[(y)][(z)])<<24)+((x[(y)][(z)+1])<<16)+((x[(y)][(z)+2])<<8)+(x[(y)][(z)+3]))
//#define ffword(x,y,z)       (((x[(y)][(z)])<<8)+(x[(y)][(z)+1]))

// Defines for the ASM operations
enum ASM_DEFINE
{
	SETV,                 //0x0000
	SETR,                 //0x0001
	ADDR,                 //0x0002
	ADDV,                 //0x0003
	SUBR,                 //0x0004
	SUBV,                 //0x0005
	MULTR,                //0x0006
	MULTV,                //0x0007
	DIVR,                 //0x0008
	DIVV,                 //0x0009
	WAITFRAME,            //0x000A
	GOTO,                 //0x000B
	CHECKTRIG,            //0x000C //NOT IMPLEMENTED
	WARP,                 //0x000D
	COMPARER,             //0x000E
	COMPAREV,             //0x000F
	GOTOTRUE,             //0x0010
	GOTOFALSE,            //0x0011
	GOTOLESS,             //0x0012
	GOTOMORE,             //0x0013
	LOAD1,                //0x0014
	LOAD2,                //0x0015
	SETA1,                //0x0016
	SETA2,                //0x0017
	QUIT,                 //0x0018
	SINR,                 //0x0019
	SINV,                 //0x001A
	COSR,                 //0x001B
	COSV,                 //0x001C
	TANR,                 //0x001D
	TANV,                 //0x001E
	MODR,                 //0x001F
	MODV,                 //0x0020
	ABSR,                 //0x0021
	MINR,                 //0x0022
	MINV,                 //0x0023
	MAXR,                 //0x0024
	MAXV,                 //0x0025
	RNDR,                 //0x0026
	RNDV,                 //0x0027
	FACTORIAL,            //0x0028
	POWERR,               //0x0029
	POWERV,               //0x002A
	IPOWERR,              //0x002B
	IPOWERV,              //0x002C
	ANDR,                 //0x002D
	ANDV,                 //0x002E
	ORR,                  //0x002F
	ORV,                  //0x0030
	XORR,                 //0x0031
	XORV,                 //0x0032
	NANDR,                //0x0033
	NANDV,                //0x0034
	NORR,                 //0x0035
	NORV,                 //0x0036
	XNORR,                //0x0037
	XNORV,                //0x0038
	NOT,                  //0x0039
	LSHIFTR,              //0x003A
	LSHIFTV,              //0x003B
	RSHIFTR,              //0x003C
	RSHIFTV,              //0x003D
	TRACER,               //0x003E
	TRACEV,               //0x003F
	TRACE3,               //0x0040
	LOOP,                 //0x0041
	PUSHR,                //0x0042
	PUSHV,                //0x0043
	POP,                  //0x0044
	ENQUEUER,             //0x0045 //NOT IMPLEMENTED
	ENQUEUEV,             //0x0046 //NOT IMPLEMENTED
	DEQUEUE,              //0x0047 //NOT IMPLEMENTED
	PLAYSOUNDR,           //0x0048
	PLAYSOUNDV,           //0x0049
	LOADLWEAPONR,          //0x004A
	LOADLWEAPONV,          //0x004B
	LOADITEMR,             //0x004C
	LOADITEMV,             //0x004D
	LOADNPCR,              //0x004E
	LOADNPCV,              //0x004F
	CREATELWEAPONR,        //0x0050
	CREATELWEAPONV,        //0x0051
	CREATEITEMR,           //0x0052
	CREATEITEMV,           //0x0053
	CREATENPCR,            //0x0054
	CREATENPCV,            //0x0055
	LOADI,                 //0x0056
	STOREI,                //0x0057
	GOTOR,                 //0x0058
	SQROOTV,               //0x0059
	SQROOTR,               //0x005A
	CREATEEWEAPONR,        //0x005B
	CREATEEWEAPONV,        //0x005C
	PITWARP,               //0x005D
	WARPR,                 //0x005E
	PITWARPR,              //0x005F
	CLEARSPRITESR,         //0x0060
	CLEARSPRITESV,         //0x0061
	RECTR,                 //0x0062
	CIRCLER,               //0x0063
	ARCR,                  //0x0064
	ELLIPSER,              //0x0065
	LINER,                 //0x0066
	PUTPIXELR,             //0x0067
	DRAWTILER,             //0x0068
	DRAWCOMBOR,            //0x0069
	ELLIPSE2,              //0x006A
	SPLINER,               //0x006B
	FLOODFILL,             //0x006C
	COMPOUNDR,             //0x006D
	COMPOUNDV,             //0x006E
	MSGSTRR,               //0x006F
	MSGSTRV,               //0x0070
	ISVALIDITEM,           //0x0071
	ISVALIDNPC,            //0x0072
	PLAYMIDIR,             //0x0073
	PLAYMIDIV,             //0x0074
	COPYTILEVV,            //0x0075
	COPYTILEVR,            //0x0076
	COPYTILERV,            //0x0077
	COPYTILERR,            //0x0078
	SWAPTILEVV,            //0x0079
	SWAPTILEVR,            //0x007A
	SWAPTILERV,            //0x007B
	SWAPTILERR,            //0x007C
	CLEARTILEV,            //0x007D
	CLEARTILER,            //0x007E
	OVERLAYTILEVV,         //0x007F
	OVERLAYTILEVR,         //0x0080
	OVERLAYTILERV,         //0x0081
	OVERLAYTILERR,         //0x0082
	FLIPROTTILEVV,         //0x0083
	FLIPROTTILEVR,         //0x0084
	FLIPROTTILERV,         //0x0085
	FLIPROTTILERR,         //0x0086
	GETTILEPIXELV,         //0x0087
	GETTILEPIXELR,         //0x0088
	SETTILEPIXELV,         //0x0089
	SETTILEPIXELR,         //0x008A
	SHIFTTILEVV,           //0x008B
	SHIFTTILEVR,           //0x008C
	SHIFTTILERV,           //0x008D
	SHIFTTILERR,           //0x008E
	ISVALIDLWPN,           //0x008F
	ISVALIDEWPN,           //0x0090
	LOADEWEAPONR,          //0x0091
	LOADEWEAPONV,          //0x0092
	ALLOCATEMEMR,          //0x0093
	ALLOCATEMEMV,          //0x0094
	ALLOCATEGMEMV,         //0x0095
	DEALLOCATEMEMR,		 //0x0096
	DEALLOCATEMEMV,		 //0x0097 //Pointless, can't deallocate a value
	WAITDRAW,				 //0x0098
	ARCTANR,				 //0x0099
	LWPNUSESPRITER,		 //0x09A
	LWPNUSESPRITEV,		 //0x09B
	EWPNUSESPRITER,		 //0x09C
	EWPNUSESPRITEV,		 //0x09D
	LOADITEMDATAR,		 //0x09E
	LOADITEMDATAV,		 //0x09F
	BITNOT,				 //0x00A0
	LOG10,				 //0x00A1
	LOGE,                  //0x00A2
	ISSOLID,               //0x00A3
	LAYERSCREEN,           //0x00A4
	LAYERMAP,              //0x00A5
	TRACE2R,		    //0x00A6
	TRACE2V,		    //0x00A7
	TRACE4,			    //0x00A8
	TRACE5,			    //0x00A9
	SECRETS,		    //0x00AA
	DRAWCHARR,		    //0x00AB
	GETSCREENFLAGS,	    //0x00AC
	QUADR,              //0X00AD
	TRIANGLER,          //0X00AE
	ARCSINR,              //0x00AF
	ARCSINV,              //0x00B0
	ARCCOSR,              //0x00B1
	ARCCOSV,              //0x00B2
	GAMEEND,              //0x00B3
	DRAWINTR,             //0x00B4
	SETTRUE,              //0x00B5
	SETFALSE,             //0x00B6
	SETMORE,              //0x00B7
	SETLESS,              //0x00B8
	FASTTILER,            //0x00B9
	FASTCOMBOR,           //0x00BA
	DRAWSTRINGR,          //0x00BB
	SETSIDEWARP,			//0x00BC
	SAVE,				    //0x00BD
	TRACE6,				//0x00BE
	WHATNO0x00BF, //PTROFF,//0x00BF
	QUAD3DR,              //0x00C0
	TRIANGLE3DR,          //0x00C1
	SETCOLORB,        	//0x00C2 for 2.6
	SETDEPTHB,        	//0x00C3 '
	GETCOLORB,        	//0x00C4 '
	GETDEPTHB,        	//0x00C5 '
	COMBOTILE,			//0x00C6
	SETTILEWARP,			//0x00C7
	GETSCREENEFLAGS,		//0x00C8
	GETSAVENAME,          //0x00C9
	ARRAYSIZE,            //0x00CA
	ITEMNAME,             //0x00CB
	SETSAVENAME,          //0x00CC
	NPCNAME,         		//0x00CD
	GETMESSAGE,			//0x00CE
	GETDMAPNAME,			//0x00CF
	GETDMAPTITLE,			//0x00D0
	GETDMAPINTRO,			//0x00D1
	ALLOCATEGMEMR,        //0x00D2
	BITMAPR,         		//0x00D3
	SETRENDERTARGET,      //0x00D4
	PLAYENHMUSIC,         //0x00D5
	GETMUSICFILE,         //0x00D6
	GETMUSICTRACK,        //0x00D7
	SETDMAPENHMUSIC,      //0x00D8
	DRAWLAYERR,           //0x00D9
	DRAWSCREENR,          //0x00DA
	BREAKSHIELD,          //0x00DB
	SAVESCREEN,           //0x00DC
	SAVEQUITSCREEN,       //0x00DD
	SELECTAWPNR,          //0x00DE
	SELECTAWPNV,          //0x00DF
	SELECTBWPNR,          //0x00E0
	SELECTBWPNV,          //0x00E1
	GETSIDEWARPDMAP,      //0x00E2
	GETSIDEWARPSCR,       //0x00E3
	GETSIDEWARPTYPE,      //0x00E4
	GETTILEWARPDMAP,      //0x00E5
	GETTILEWARPSCR,       //0x00E6
	GETTILEWARPTYPE,      //0x00E7
	GETFFCSCRIPT,         //0x00E8
	BITMAPEXR,	//0x00E9
	__RESERVED_FOR_QUAD2R, //0x00EA
	WAVYIN, //0x00EB
	WAVYOUT, //0x00EC
	ZAPIN, //0x00ED
	ZAPOUT, //0x00EF
	OPENWIPE, //0x00F0
	FREE0x00F1, //0x00F1 was SETLINKTILE
	FREE0x00F2, //0x00F2 was SETLINKEXTEND
	FREE0x00F3, //0x00F3 was GETLINKEXTEND
	SETMESSAGE,			//0x00F4
	SETDMAPNAME,			//0x00F5
	SETDMAPTITLE,			//0x00F5
	SETDMAPINTRO,			//0x00F7
	GREYSCALEON,			//0x00F8
	GREYSCALEOFF,			//0x00F9
	ENDSOUNDR,           //0x00FA
	ENDSOUNDV,           //0x00FB
	PAUSESOUNDR, 	//0x00FC
	PAUSESOUNDV,	//0x00FD
	RESUMESOUNDR,	//0x00FE
	RESUMESOUNDV,	//0x00FF
	PAUSEMUSIC,		//0x0100
	RESUMEMUSIC,	//0x0101
	LWPNARRPTR,	//0x0102
	EWPNARRPTR,	//0x0103
	ITEMARRPTR,	//0x0104
	IDATAARRPTR,	//0x0105
	FFCARRPTR,	//0x0106
	BOOLARRPTR,	//0x0107
	NPCARRPTR,	//0x0108
	LWPNARRPTR2,	//0x0109
	EWPNARRPTR2,	//0x0110
	ITEMARRPTR2,	//0x0111
	IDATAARRPTR2,	//0x0112
	FFCARRPTR2,	//0x0113
	BOOLARRPTR2,	//0x0114
	NPCARRPTR2,	//0x0115
	ARRAYSIZEB,            //0x0116
	ARRAYSIZEF,            //0x0117 
	ARRAYSIZEN,            //0x0118
	ARRAYSIZEL,            //0x0119
	ARRAYSIZEE,            //0x011A
	ARRAYSIZEI,            //0x011B
	ARRAYSIZEID,            //0x011C
	POLYGONR,		//0x011D
	__RESERVED_FOR_POLYGON3DR,		//0x011E
	__RESERVED_FOR_SETRENDERSOURCE,	//0x011F
	__RESERVED_FOR_CREATEBITMAP,	//0x0120
	__RESERVED_FOR_PIXELARRAYR,	//0x0121
	__RESERVED_FOR_TILEARRAYR,		//0x0122
	__RESERVED_FOR_COMBOARRAYR,	//0x0123
	RES0000,		//0x0124
	RES0001,		//0x0125
	RES0002,		//0x0126
	RES0003,		//0x0127
	RES0004,		//0x0128
	RES0005,		//0x0129
	RES0006,		//0x012A
	RES0007,		//0x012B
	RES0008,		//0x012C
	RES0009,		//0x012D
	RES000A,		//0x012E
	RES000B,		//0x012F
	RES000C,		//0x0130
	RES000D,		//0x0131
	RES000E,		//0x0132
	RES000F,		//0x0133
	__RESERVED_FOR_CREATELWPN2VV,            //0x0134
	__RESERVED_FOR_CREATELWPN2VR,            //0x0135
	__RESERVED_FOR_CREATELWPN2RV,            //0x0136
	__RESERVED_FOR_CREATELWPN2RR,            //0x0137
	GETSCREENDOOR, //0x0138
	GETSCREENENEMY, //0x0139
	PAUSESFX,
	RESUMESFX,
	CONTINUESFX,
	ADJUSTSFX,
	//__RESERVED_FOR_GETSCREENFLAG, //0x013A
	GETITEMSCRIPT,
	GETSCREENLAYOP,
	GETSCREENSECCMB,
	GETSCREENSECCST,
	GETSCREENSECFLG,
	GETSCREENLAYMAP,
	GETSCREENLAYSCR,
	GETSCREENPATH,
	GETSCREENWARPRX,
	GETSCREENWARPRY,
	TRIGGERSECRETR,
	TRIGGERSECRETV,
	CHANGEFFSCRIPTR,
	CHANGEFFSCRIPTV,
	
	//NPCData
	GETNPCDATAFLAGS,
	GETNPCDATAFLAGS2,
	GETNPCDATAWIDTH,
	GETNPCDATAHEIGHT,
	GETNPCDATASTILE,
	GETNPCDATASWIDTH,
	GETNPCDATASHEIGHT,
	GETNPCDATAETILE,
	GETNPCDATAEWIDTH,
	GETNPCDATAHP,
	GETNPCDATAFAMILY,
	GETNPCDATACSET,
	GETNPCDATAANIM,
	GETNPCDATAEANIM,
	GETNPCDATAFRAMERATE,
	GETNPCDATAEFRAMERATE,
	GETNPCDATATOUCHDMG,
	GETNPCDATAWPNDAMAGE,
	GETNPCDATAWEAPON,
	GETNPCDATARANDOM,
	GETNPCDATAHALT,
	GETNPCDATASTEP,
	GETNPCDATAHOMING,
	GETNPCDATAHUNGER,
	GETNPCDATADROPSET,
	GETNPCDATABGSFX,
	GETNPCDATADEATHSFX,
	GETNPCDATAXOFS,
	GETNPCDATAYOFS,
	GETNPCDATAZOFS,
	GETNPCDATAHXOFS,
	GETNPCDATAHYOFS,
	GETNPCDATAHITWIDTH,
	GETNPCDATAHITHEIGHT,
	GETNPCDATAHITZ,
	GETNPCDATATILEWIDTH,
	GETNPCDATATILEHEIGHT,
	GETNPCDATAWPNSPRITE,
	//TWO INPUTS, ONE RETURN
	GETNPCDATASCRIPTDEF,
	GETNPCDATADEFENSE,
	GETNPCDATASIZEFLAG,
	GETNPCDATAATTRIBUTE,

	//TWO INPUTS, ONE RETURN
	SETNPCDATAFLAGS,
	SETNPCDATAFLAGS2,
	SETNPCDATAWIDTH,
	SETNPCDATAHEIGHT,
	SETNPCDATASTILE,
	SETNPCDATASWIDTH,
	SETNPCDATASHEIGHT,
	SETNPCDATAETILE,
	SETNPCDATAEWIDTH,
	SETNPCDATAHP,
	SETNPCDATAFAMILY,
	SETNPCDATACSET,
	SETNPCDATAANIM,
	SETNPCDATAEANIM,
	SETNPCDATAFRAMERATE,
	SETNPCDATAEFRAMERATE,
	SETNPCDATATOUCHDMG,
	SETNPCDATAWPNDAMAGE,
	SETNPCDATAWEAPON,
	SETNPCDATARANDOM,
	SETNPCDATAHALT,
	SETNPCDATASTEP,
	SETNPCDATAHOMING,
	SETNPCDATAHUNGER,
	SETNPCDATADROPSET,
	SETNPCDATABGSFX,
	SETNPCDATADEATHSFX,
	SETNPCDATAXOFS,
	SETNPCDATAYOFS,
	SETNPCDATAZOFS,
	SETNPCDATAHXOFS,
	SETNPCDATAHYOFS,
	SETNPCDATAHITWIDTH,
	SETNPCDATAHITHEIGHT,
	SETNPCDATAHITZ,
	SETNPCDATATILEWIDTH,
	SETNPCDATATILEHEIGHT,
	SETNPCDATAWPNSPRITE,
	SETNPCDATAHITSFX,
	GETNPCDATAHITSFX,
	//ComboData
	GCDBLOCKENEM,
	GCDBLOCKHOLE,
	GCDBLOCKTRIG,
	GCDCONVEYSPDX,
	GCDCONVEYSPDY,
	GCDCREATEENEM,
	GCDCREATEENEMWH,
	GCDCREATEENEMCH,
	GCDDIRCHTYPE,
	GCDDISTCHTILES,
	GCDDIVEITEM,
	GCDDOCK,
	GCDFAIRY,
	GCDFFCOMBOATTRIB,
	GCDFOOTDECOTILE,
	GCDFOOTDECOTYPE,
	GCDHOOKSHOTGRAB,
	GCDLADDERPASS,
	GCDLOCKBLOCKTYPE,
	GCDLOCKBLOCKCHANGE,
	GCDMAGICMIRRORTYPE,
	GCDMODIFYHPAMOUNT,
	GCDMODIFYHPDELAY,
	GCDMODIFYHPTYPE,
	GCDMODIFYMPAMOUNT,
	GCDMODIFYMPDELAY,
	GCDMODIFYMPTYPE,
	GCDNOPUSHBLOCKS,
	GCDOVERHEAD,
	GCDPLACEENEMY,
	GCDPUSHDIR,
	GCDPUSHWEIGHT,
	GCDPUSHWAIT,
	GCDPUSHED,
	GCDRAFT,
	GCDRESETROOM,
	GCDSAVEPOINT,
	GCDSCREENFREEZE,
	GCDSECRETCOMBO,
	GCDSINGULAR,
	GCDSLOWMOVE,
	GCDSTATUE,
	GCDSTEPTYPE,
	GCDSTEPCHANGETO,
	GCDSTRIKEREMNANTS,
	GCDSTRIKEREMNANTSTYPE,
	GCDSTRIKECHANGE,
	GCDSTRIKECHANGEITEM,
	GCDTOUCHITEM,
	GCDTOUCHSTAIRS,
	GCDTRIGGERTYPE,
	GCDTRIGGERSENS,
	GCDWARPTYPE,
	GCDWARPSENS,
	GCDWARPDIRECT,
	GCDWARPLOCATION,
	GCDWATER,
	GCDWHISTLE,
	GCDWINGAME,
	GCDBLOCKWEAPLVL,
	GCDTILE,
	GCDFLIP,
	GCDWALK,
	GCDTYPE,
	GCDCSETS,
	GCDFOO,
	GCDFRAMES,
	GCDSPEED,
	GCDNEXTCOMBO,
	GCDNEXTCSET,
	GCDFLAG,
	GCDSKIPANIM,
	GCDNEXTTIMER,
	GCDSKIPANIMY,
	GCDANIMFLAGS,
	GCDBLOCKWEAPON,
	GCDEXPANSION,
	GCDSTRIKEWEAPONS,
	SCDBLOCKENEM,
	SCDBLOCKHOLE,
	SCDBLOCKTRIG,
	SCDCONVEYSPDX,
	SCDCONVEYSPDY,
	SCDCREATEENEM,
	SCDCREATEENEMWH,
	SCDCREATEENEMCH,
	SCDDIRCHTYPE,
	SCDDISTCHTILES,
	SCDDIVEITEM,
	SCDDOCK,
	SCDFAIRY,
	SCDFFCOMBOATTRIB,
	SCDFOOTDECOTILE,
	SCDFOOTDECOTYPE,
	SCDHOOKSHOTGRAB,
	SCDLADDERPASS,
	SCDLOCKBLOCKTYPE,
	SCDLOCKBLOCKCHANGE,
	SCDMAGICMIRRORTYPE,
	SCDMODIFYHPAMOUNT,
	SCDMODIFYHPDELAY,
	SCDMODIFYHPTYPE,
	SCDMODIFYMPAMOUNT,
	SCDMODIFYMPDELAY,
	SCDMODIFYMPTYPE,
	SCDNOPUSHBLOCKS,
	SCDOVERHEAD,
	SCDPLACEENEMY,
	SCDPUSHDIR,
	SCDPUSHWEIGHT,
	SCDPUSHWAIT,
	SCDPUSHED,
	SCDRAFT,
	SCDRESETROOM,
	SCDSAVEPOINT,
	SCDSCREENFREEZE,
	SCDSECRETCOMBO,
	SCDSINGULAR,
	SCDSLOWMOVE,
	SCDSTATUE,
	SCDSTEPTYPE,
	SCDSTEPCHANGETO,
	SCDSTRIKEREMNANTS,
	SCDSTRIKEREMNANTSTYPE,
	SCDSTRIKECHANGE,
	SCDSTRIKECHANGEITEM,
	SCDTOUCHITEM,
	SCDTOUCHSTAIRS,
	SCDTRIGGERTYPE,
	SCDTRIGGERSENS,
	SCDWARPTYPE,
	SCDWARPSENS,
	SCDWARPDIRECT,
	SCDWARPLOCATION,
	SCDWATER,
	SCDWHISTLE,
	SCDWINGAME,
	SCDBLOCKWEAPLVL,
	SCDTILE,
	SCDFLIP,
	SCDWALK,
	SCDTYPE,
	SCDCSETS,
	SCDFOO,
	SCDFRAMES,
	SCDSPEED,
	SCDNEXTCOMBO,
	SCDNEXTCSET,
	SCDFLAG,
	SCDSKIPANIM,
	SCDNEXTTIMER,
	SCDSKIPANIMY,
	SCDANIMFLAGS,
	GETNPCDATATILE,
	GETNPCDATAEHEIGHT,
	SETNPCDATATILE,
	SETNPCDATAEHEIGHT,
	//SpriteData
	GETSPRITEDATASTRING,
	GETSPRITEDATATILE,
	GETSPRITEDATAMISC,
	GETSPRITEDATACGETS,
	GETSPRITEDATAFRAMES,
	GETSPRITEDATASPEED,
	GETSPRITEDATATYPE,
	SETSPRITEDATASTRING,
	SETSPRITEDATATILE,
	SETSPRITEDATAMISC,
	SETSPRITEDATACSETS,
	SETSPRITEDATAFRAMES,
	SETSPRITEDATASPEED,
	SETSPRITEDATATYPE,
	SETCONTINUESCREEN,
	SETCONTINUESTRING,
	LOADNPCDATAR,
	LOADNPCDATAV,
	LOADCOMBODATAR,
	LOADCOMBODATAV,
	LOADMAPDATAR,
	LOADMAPDATAV,
	LOADSPRITEDATAR,
	LOADSPRITEDATAV,
	LOADSCREENDATAR,
	LOADSCREENDATAV,
	LOADBITMAPDATAR,
	LOADBITMAPDATAV,
	LOADSHOPR,
	LOADSHOPV,
	LOADINFOSHOPR,
	LOADINFOSHOPV,
	
	LOADMESSAGEDATAR, //COMMAND
	LOADMESSAGEDATAV,//COMMAND

	MESSAGEDATASETSTRINGR, //command
	MESSAGEDATASETSTRINGV, //command
	MESSAGEDATAGETSTRINGR, //command
	MESSAGEDATAGETSTRINGV, //command
	
	LOADDMAPDATAR,
	LOADDMAPDATAV,
	DMAPDATAGETNAMER,
	DMAPDATAGETNAMEV,
	DMAPDATASETNAMER,
	DMAPDATASETNAMEV,
	DMAPDATAGETTITLER,
	DMAPDATAGETTITLEV,
	DMAPDATASETTITLER,
	DMAPDATASETTITLEV,
	DMAPDATAGETINTROR,
	DMAPDATAGETINTROV,
	DMAPDATANSETITROR,
	DMAPDATASETINTROV,
	DMAPDATAGETMUSICR,
	DMAPDATAGETMUSICV,
	DMAPDATASETMUSICR,
	DMAPDATASETMUSICV,
	
	ADJUSTSFXVOLUMER,		
	ADJUSTSFXVOLUMEV,		
	ADJUSTVOLUMER,		
	ADJUSTVOLUMEV,	
	
	FXWAVYR,
	FXWAVYV,
	FXZAPR,
	FXZAPV,
	GREYSCALER,
	GREYSCALEV,
		
	NUMCOMMANDS           //0x013B
};


//ZASM registers
//When adding indexed variables the index will be loaded into ri.d[0], don't add a register for each one!
#define D(n)               ((0x0000)+(n)) //8
#define A(n)               ((0x0008)+(n)) //2
#define DATA                 0x000A
#define FCSET                0x000B
#define DELAY                0x000C
#define FX                   0x000D
#define FY                   0x000E
#define XD                   0x000F
#define YD                   0x0010
#define XD2                  0x0011
#define YD2                  0x0012
#define FLAG                 0x0013
#define WIDTH                0x0014
#define HEIGHT               0x0015
#define LINK                 0x0016
//#define COMBOD(n)          ((0x0017)+((n)*3)) ~loads spare here...
//#define COMBOC(n)          ((0x0018)+((n)*3))
//#define COMBOF(n)          ((0x0019)+((n)*3))

//n=0-175, so if n=175, then ((0x0019)+((n)*3))=0x00226
#define INPUTSTART           0x0227
#define INPUTUP              0x0228
#define INPUTDOWN            0x0229
#define INPUTLEFT            0x022A
#define INPUTRIGHT           0x022B
#define INPUTA               0x022C
#define INPUTB               0x022D
#define INPUTL               0x022E
#define INPUTR               0x022F

#define LINKX                0x0230
#define LINKY                0x0231
#define LINKDIR              0x0232
#define LINKHP               0x0233
#define LINKMP               0x0234
#define LINKMAXHP            0x0235
#define LINKMAXMP            0x0236
#define LINKACTION           0x0237
#define LINKITEMD            0x0238
#define LINKZ                0x0239
#define LINKJUMP             0x023A
#define LINKSWORDJINX        0x023B
#define LINKITEMJINX         0x023C
#define LINKDRUNK            0x023D
#define LINKHELD             0x023E
#define INPUTMAP             0x023F
#define LINKEQUIP            0x0240
#define LINKINVIS            0x0241
#define LINKINVINC           0x0242
#define LINKLADDERX          0x0243
#define LINKLADDERY          0x0244
#define LINKHITDIR           0x0245
#define LINKTILE             0x0246
#define LINKFLIP             0x0247
#define INPUTPRESSMAP        0x0248
//0x0249-0x0258 are reserved for future Link variables
#define LINKDIAG             0x0249
#define LINKBIGHITBOX             0x024A
#define __RESERVED_FOR_LINKRETSQUARE             0x024B
#define __RESERVED_FOR_LINKWARPSOUND             0x024C
//LINKWARPMUSIC
#define LINKUSINGITEM             0x024D
#define LINKUSINGITEMA             0x024E
#define LINKUSINGITEMB             0x024F
#define __RESERVED_FOR_PLAYWARPSOUND             0x0250
#define __RESERVED_FOR_WARPEFFECT             0x0251
#define __RESERVED_FOR_PLAYPITWARPSFX             0x0252
#define LINKEATEN             0x0253
#define __RESERVED_FOR_LINKEXTEND             0x0254
#define UNUSED37             0x0255
#define UNUSED38             0x0256
#define UNUSED39             0x0257
#define UNUSED40             0x0258

#define LWPNX                0x0259
#define LWPNY                0x025A
#define LWPNDIR              0x025B
#define LWPNSTEP             0x025C
#define LWPNANGULAR          0x025D
#define LWPNANGLE            0x025E
#define LWPNDRAWTYPE         0x025F
#define LWPNPOWER            0x0260
#define LWPNDEAD             0x0261
#define LWPNID               0x0262
#define LWPNTILE             0x0263
#define LWPNCSET             0x0264
#define LWPNFLASHCSET        0x0265
#define LWPNFRAMES           0x0266
#define LWPNFRAME            0x0267
#define LWPNASPEED           0x0268
#define LWPNFLASH            0x0269
#define LWPNFLIP             0x026A
#define LWPNCOUNT            0x026B
#define LWPNEXTEND           0x026C
#define LWPNOTILE            0x026D
#define LWPNOCSET            0x026E
#define LWPNZ                0x026F
#define LWPNJUMP             0x0270
#define LWPNCOLLDET          0x0271
//   0x0272-   0x028B are reserved for future weapon variables
#define LWPNRANGE             0x0272
#define LWPNPARENT             0x0273
#define LWPNLEVEL             0x0274
#define UNUSED50             0x0275
#define UNUSED51             0x0276
#define UNUSED52             0x0277
#define UNUSED53             0x0278
#define UNUSED54             0x0279
#define UNUSED55             0x027A
#define UNUSED56             0x027B
#define UNUSED57             0x027C
#define UNUSED58             0x027D
#define UNUSED59             0x027E
#define UNUSED60             0x027F
#define UNUSED61             0x0280
#define UNUSED62             0x0281
#define UNUSED63             0x0282
#define UNUSED64             0x0283
#define UNUSED65             0x0284
#define UNUSED66             0x0285
#define UNUSED67             0x0286
#define UNUSED68             0x0287
#define UNUSED69             0x0288
#define UNUSED70             0x0289
#define UNUSED71             0x028A
#define EWPNPARENT             0x028B

#define ITEMX                0x028C
#define ITEMY                0x028D
#define ITEMDRAWTYPE         0x028E
#define ITEMID               0x028F
#define ITEMTILE             0x0290
#define ITEMCSET             0x0291
#define ITEMFLASHCSET        0x0292
#define ITEMFRAMES           0x0293
#define ITEMFRAME            0x0294
#define ITEMASPEED           0x0295
#define ITEMDELAY            0x0296
#define ITEMFLASH            0x0297
#define ITEMFLIP             0x0298
#define ITEMCOUNT            0x0299
#define IDATAFAMILY          0x029A
#define IDATALEVEL           0x029B
#define IDATAKEEP            0x029C
#define IDATAAMOUNT          0x029D
#define IDATASETMAX          0x029E
#define IDATAMAX             0x029F
#define IDATACOUNTER         0x02A0
#define ITEMEXTEND           0x02A1
#define ITEMZ                0x02A2
#define ITEMJUMP             0x02A3
#define ITEMOTILE            0x02A4
#define IDATAUSESOUND        0x02A5
#define IDATAPOWER           0x02A6

//0x02A7-0x02B9 are reserved for future item variables
#define IDATAID             0x02A7
#define IDATALTM             0x02A8
#define IDATAPSCRIPT             0x02A9
#define IDATASCRIPT             0x02AA
#define IDATAMAGCOST             0x02AB
#define IDATAMINHEARTS             0x02AC
#define IDATATILE             0x02AD
#define IDATAMISC             0x02AE
#define IDATACSET             0x02AF
#define IDATAFRAMES             0x02B0
#define IDATAASPEED             0x02B1
#define IDATADELAY             0x02B2
#define IDATAWPN             0x02B3
#define IDATAFRAME             0x02B4
#define ITEMACLK            0x02B5
#define IDATAFLAGS            0x02B6
#define IDATASPRITE            0x02B7
#define IDATAATTRIB            0x02B8

#define ZELDAVERSION            0x02B9

#define NPCX                 0x02BA
#define NPCY                 0x02BB
#define NPCDIR               0x02BC
#define NPCRATE              0x02BD
#define NPCFRAMERATE         0x02BE
#define NPCHALTRATE          0x02BF
#define NPCDRAWTYPE          0x02C0
#define NPCHP                0x02C1
#define NPCID                0x02C2
#define NPCDP                0x02C3
#define NPCWDP               0x02C4
#define NPCOTILE             0x02C5
#define NPCENEMY             0x02C6
#define NPCWEAPON            0x02C7
#define NPCITEMSET           0x02C8
#define NPCCSET              0x02C9
#define NPCBOSSPAL           0x02CA
#define NPCBGSFX             0x02CB
#define NPCCOUNT             0x02CC
#define NPCEXTEND            0x02CD
#define NPCZ                 0x02CE
#define NPCJUMP              0x02CF
#define NPCSTEP              0x02D0
#define NPCTILE              0x02D1
#define NPCHOMING            0x02D2
#define NPCDD	             0x02D3
//#define NPCD(n)	           ((0x101D)+(n)) //10 spare
#define NPCTYPE              0x02DE
#define NPCMFLAGS            0x02DF
#define NPCCOLLDET           0x02E0
#define NPCSTUN              0x02E1
#define NPCHUNGER            0x02E2
//0x02E3-0x02EB are reserved for future NPC variables
//#define             0x02E2 //This was a duplicate define value in the old source. 
#define NPCSUPERMAN            0x02E3
#define NPCHASITEM            0x02E4
#define NPCRINGLEAD            0x02E5
#define NPCINVINC            0x02E6
#define NPCSHIELD            0x02E7
#define NPCFROZEN            0x02E8
#define NPCFROZENTILE            0x02E9
#define NPCFROZENCSET            0x02EA
#define UNUSED135            0x02EB
//I'm stealing this one for the stack pointer -DD
#define SP                   0x02EC

//#define SD(n)              ((0x02ED)+(n)) //Spare 8
#define GD(n)              ((0x02F5)+(n)) //256

// Defines for accessing gamedata
//#define GAMECOUNTER(n)     ((0x03F5)+((n)*3)) //All of these spare
//#define GAMEMCOUNTER(n)    ((0x03F6)+((n)*3))
//#define GAMEDCOUNTER(n)    ((0x03F7)+((n)*3))
//#define GAMEGENERIC(n)     ((0x0455)+(n))
//#define GAMEITEMS(n)       ((0x0555)+(n))
//#define GAMELITEMS(n)      ((0x0655)+(n))
//#define GAMELKEYS(n)       ((0x0755)+(n))
#define GAMEDEATHS           0x0855
#define GAMECHEAT            0x0856
#define GAMETIME             0x0857
#define GAMEHASPLAYED        0x0858
#define GAMETIMEVALID        0x0859
#define GAMEGUYCOUNTD        0x085A
#define GAMEGUYCOUNT         0x085B
#define GAMECONTSCR          0x085C
#define GAMECONTDMAP         0x085D
#define GAMECOUNTERD         0x085E
#define GAMEMCOUNTERD        0x085F
#define GAMEDCOUNTERD        0x0860
#define GAMEGENERICD         0x0861
#define GAMEITEMSD           0x0862
#define GAMELITEMSD          0x0863
#define GAMELKEYSD           0x0864
#define ROOMTYPE             0x0865
#define ROOMDATA             0x0866
//#define SCREENSTATE(n)     ((0x0867)+(n)) //spare 30
#define SCREENSTATED         0x0885

#define CURSCR               0x0886
#define CURMAP               0x0887
#define CURDMAP              0x0888
#define COMBODD	             0x0889
#define COMBOCD              0x088A
#define COMBOFD              0x088B
#define COMBOTD              0x088C
#define COMBOID              0x088D

//Defines for all the extra crap that's been added since
#define REFITEMCLASS         0x088E
#define REFITEM              0x088F
#define REFFFC               0x0890
#define REFLWPN              0x0891
#define REFNPC               0x0892
#define REFLWPNCLASS         0x0893
#define REFNPCCLASS          0x0894
//#define REFSUBSCREEN         0x0895 //Not implemented
#define REFEWPN              0x0896
#define REFEWPNCLASS         0x0897

#define EWPNX                0x0898
#define EWPNY                0x0899
#define EWPNDIR              0x089A
#define EWPNSTEP             0x089B
#define EWPNANGULAR          0x089C
#define EWPNANGLE            0x089D
#define EWPNDRAWTYPE         0x089E
#define EWPNPOWER            0x089F
#define EWPNDEAD             0x08A0
#define EWPNID               0x08A1
#define EWPNTILE             0x08A2
#define EWPNCSET             0x08A3
#define EWPNFLASHCSET        0x08A4
#define EWPNFRAMES           0x08A5
#define EWPNFRAME            0x08A6
#define EWPNASPEED           0x08A7
#define EWPNFLASH            0x08A8
#define EWPNFLIP             0x08A9
#define EWPNCOUNT            0x08AA
#define EWPNEXTEND           0x08AB
#define EWPNOTILE            0x08AC
#define EWPNOCSET            0x08AD
#define EWPNZ                0x08AE
#define EWPNJUMP             0x08AF

//#define SCRDOOR(n)         ((0x08B0)+(n)) //spare one
#define SCRDOORD             0x08B4

#define CURDSCR              0x08B5
//#define GETSTART             0x08B6 //?

#define COMBOSD	             0x08B7
#define SCREENSTATEDD        0x08B8
#define SDD                  0x08B9
#define SDDD                 0x08BA
#define GDD                  0x08BB

#define FFFLAGSD             0x08BC
#define FFCWIDTH             0x08BD
#define FFCHEIGHT            0x08BE
#define FFTWIDTH             0x08BF
#define FFTHEIGHT            0x08C0
#define FFLINK               0x08C1

//#define GS(n)              ((0x08C2)+(n)) //this never did anything anyway

#define WAVY                 0x09C2
#define QUAKE                0x09C3
#define INPUTMOUSEX          0x09C4
#define INPUTMOUSEY          0x09C5
#define INPUTMOUSEZ          0x09C6
#define INPUTMOUSEB          0x09C7

#define COMBODDM	         0x09C8
#define COMBOCDM             0x09C9
#define COMBOFDM             0x09CA
#define COMBOTDM             0x09CB
#define COMBOIDM             0x09CC
#define COMBOSDM             0x09CD

#define SCRIPTRAM            0x09CE
#define GLOBALRAM            0x09CF
#define SCRIPTRAMD           0x09D0
#define GLOBALRAMD           0x09D1
//#define SAVERAM              0x09D2 //Spare one here

#define LWPNHXOFS			 0x09D3
#define LWPNHYOFS			 0x09D4
#define LWPNXOFS			 0x09D5
#define LWPNYOFS			 0x09D6
#define LWPNZOFS			 0x09D7
#define LWPNHXSZ			 0x09D8
#define LWPNHYSZ			 0x09D9
#define LWPNHZSZ			 0x09DA

#define EWPNHXOFS			 0x09DB
#define EWPNHYOFS			 0x09DC
#define EWPNXOFS			 0x09DD
#define EWPNYOFS			 0x09DE
#define EWPNZOFS			 0x09DF
#define EWPNHXSZ			 0x09E0
#define EWPNHYSZ			 0x09E1
#define EWPNHZSZ			 0x09E2

#define NPCHXOFS			 0x09E3
#define NPCHYOFS			 0x09E4
#define NPCXOFS				 0x09E5
#define NPCYOFS				 0x09E6
#define NPCZOFS				 0x09E7
#define NPCHXSZ			     0x09E8
#define NPCHYSZ			     0x09E9
#define NPCHZSZ			     0x09EA

#define ITEMHXOFS			 0x09EB
#define ITEMHYOFS			 0x09EC
#define ITEMXOFS			 0x09ED
#define ITEMYOFS			 0x09EE
#define ITEMZOFS			 0x09EF
#define ITEMHXSZ			 0x09F0
#define ITEMHYSZ			 0x09F1
#define ITEMHZSZ			 0x09F2

#define LWPNTXSZ			 0x09F3
#define LWPNTYSZ			 0x09F4

#define EWPNTXSZ			 0x09F5
#define EWPNTYSZ			 0x09F6

#define NPCTXSZ				 0x09F7
#define NPCTYSZ				 0x09F8

#define ITEMTXSZ			 0x09F9
#define ITEMTYSZ			 0x09FA

#define LINKHXOFS			 0x09FB
#define LINKHYOFS			 0x09FC
#define LINKXOFS			 0x09FD
#define LINKYOFS			 0x09FE
#define LINKZOFS			 0x09FF
#define LINKHXSZ			 0x1000
#define LINKHYSZ			 0x1001
#define LINKHZSZ			 0x1002
#define LINKTXSZ			 0x1003
#define LINKTYSZ			 0x1004

#define LWPNBEHIND			 0x1005
#define EWPNBEHIND			 0x1006

#define SDDDD				 0x1007
#define CURLEVEL			 0x1008
#define ITEMPICKUP			 0x1009
#define LIT                  0x100A
#define INPUTEX1             0x100B
#define INPUTEX2             0x100C
#define INPUTEX3             0x100D
#define INPUTEX4             0x100E
#define INPUTPRESSSTART      0x100F
#define INPUTPRESSUP         0x1010
#define INPUTPRESSDOWN       0x1011
#define INPUTPRESSLEFT       0x1012
#define INPUTPRESSRIGHT      0x1013
#define INPUTPRESSA          0x1014
#define INPUTPRESSB          0x1015
#define INPUTPRESSL          0x1016
#define INPUTPRESSR          0x1017
#define INPUTPRESSEX1        0x1018
#define INPUTPRESSEX2        0x1019
#define INPUTPRESSEX3        0x101A
#define INPUTPRESSEX4        0x101B

#define LWPNMISCD            0x101C
//#define LWPNMISC(n)        ((0x101D)+(n)) //16 spare
#define EWPNMISCD            0x102D
//#define EWPNMISC(n)        ((0x102E)+(n)) //16 spare
#define NPCMISCD             0x103E
//#define NPCMISC(n)         ((0x103F)+(n)) //16 spare
#define ITEMMISCD            0x104F
//#define ITEMMISC(n)        ((0x1050)+(n)) //16 spare
#define FFMISCD              0x1060
//#define FFMISC(n)          ((0x1061)+(n)) //16 spare

#define GETMIDI              0x1071

#define INPUTAXISUP	 	     0x1072
#define INPUTAXISDOWN	     0x1073
#define INPUTAXISLEFT	     0x1074
#define INPUTAXISRIGHT	     0x1075
#define INPUTPRESSAXISUP	 0x1076
#define INPUTPRESSAXISDOWN   0x1077
#define INPUTPRESSAXISLEFT   0x1078
#define INPUTPRESSAXISRIGHT  0x1079

#define FFSCRIPT		     0x107A
#define SCREENFLAGSD         0x107B
//#define SCREENFLAGS(n)     ((0x107C)+(n)) //10 spare here
#define SCREENEFLAGSD        0x1086
//#define SCREENEFLAGS(n)    ((0x1087)+(n)) //3 spare here
#define FFINITDD		     0x108B
//#define FFINITD(n)		   ((0x108C)+(n)) //8 spare
#define LINKMISCD            0x1094
//#define LINKMISC(n)        ((0x1095)+(n)) //16 spare
#define DMAPFLAGSD		     0x10A5
#define EWPNCOLLDET          0x10A6

#define NPCDEFENSED	         0x10A7
#define DMAPLEVELD		     0x10A8
#define DMAPCOMPASSD         0x10A9
#define DMAPCONTINUED        0x10AA
#define DMAPMIDID            0x10AB
#define IDATAINITDD      	 0x10AC //8

#define GAMESTANDALONE       0x10AD

#define GAMEENTRSCR          0x10AE
#define GAMEENTRDMAP         0x10AF

#define GAMECLICKFREEZE      0x10B0

#define PUSHBLOCKX           0x10B1
#define PUSHBLOCKY           0x10B2
#define PUSHBLOCKCOMBO       0x10B3
#define PUSHBLOCKCSET        0x10B4
#define UNDERCOMBO           0x10B5
#define UNDERCSET            0x10B6
#define DMAPOFFSET           0x10B7
#define DMAPMAP              0x10B8

#define __RESERVED_FOR_GAMETHROTTLE	     0x10B9

//! ItemData Misc# and Wpn#

//here are all the new datatypes. I'm not adding more!!! -Z
//(I may add fonts types, one day. -Z)
#define REFMAPDATA 0x10BA
#define REFSCREENDATA 0x10BB
#define REFCOMBODATA 0x10BC
#define REFSPRITEDATA 0x10BD
#define REFBITMAP 0x10BE
#define REFDMAPDATA 0x10BF
#define REFSHOPDATA 0x10C0
#define REFMSGDATA 0x10C1
#define REFUNTYPED 0x10C2

#define REFDROPS 0x10C3
#define REFPONDS 0x10C4
#define REFWARPRINGS 0x10C5
#define REFDOORS 0x10C6
#define REFUICOLOURS 0x10C7
#define REFRGB 0x10C8
#define REFPALETTE 0x10C9
#define REFTUNES 0x10CA
#define REFPALCYCLE 0x10CB
#define REFGAMEDATA 0x10CC
#define REFCHEATS 0x10CD
//These somehow magically all fit into the space that I reserved over a year ago! -Z

//Itemdata Flags
#define IDATACOMBINE 0x10CE
#define IDATADOWNGRADE 0x10CF
#define IDATAMAGICTIMER 0x10D0
#define IDATAPSTRING 0x10D1
#define IDATAKEEPOLD 0x10D2
#define IDATARUPEECOST 0x10D3
#define IDATAEDIBLE 0x10D4
#define IDATAFLAGUNUSED 0x10D5
#define IDATAGAINLOWER 0x10D6
#define RESVD023 0x10D7
#define RESVD024 0x10D8
#define RESVD025 0x10D9

#define NPCSCRDEFENSED 0x10DA

#define __RESERVED_FOR_SETLINKEXTEND 0x10DB
#define __RESERVED_FOR_SETLINKTILE 0x10DC
#define __RESERVED_FOR_SIDEWARPSFX 0x10DD
#define __RESERVED_FOR_PITWARPSFX 0x10DE
#define __RESERVED_FOR_SIDEWARPVISUAL 0x10DF
#define __RESERVED_FOR_PITWARPVISUAL 0x10F0
#define GAMESETA 0x10F1
#define GAMESETB 0x10F2
#define SETITEMSLOT 0x10F3
#define BUTTONPRESS 0x10F4
#define BUTTONINPUT 0x10F5
#define BUTTONHELD 0x10F6
#define KEYPRESS 0x10F7
#define READKEY 0x10F8
#define JOYPADPRESS 0x10F9
#define DISABLEDITEM 0x10FA
#define RESVD026 0x10FB
#define UNUSED10FC 0x10FC
#define UNUSED10FD 0x10FD
#define UNUSED10FE 0x10FE
#define UNUSED10FF 0x10FF
#define UNUSED1100 0x1100
#define UNUSED1101 0x1101
#define UNUSED1102 0x1102
#define UNUSED1103 0x1103
#define LINKITEMB 0x1104
#define LINKITEMA 0x1105
#define __RESERVED_FOR_LINKWALKTILE 0x1106
#define 	__RESERVED_FOR_LINKFLOATTILE 0x1107
#define 	__RESERVED_FOR_LINKSWIMTILE 0x1108
#define 	__RESERVED_FOR_LINKDIVETILE 0x1109
#define 	__RESERVED_FOR_LINKSLASHTILE 0x110A
#define 	__RESERVED_FOR_LINKJUMPTILE 0x110B
#define 	__RESERVED_FOR_LINKCHARGETILE 0x110C
#define 	__RESERVED_FOR_LINKSTABTILE 0x110D
#define 	__RESERVED_FOR_LINKCASTTILE 0x110E
#define 	__RESERVED_FOR_LINKHOLD1LTILE 0x110F
#define 	__RESERVED_FOR_LINKHOLD2LTILE 0x1110
#define 	__RESERVED_FOR_LINKHOLD1WTILE 0x1111
#define 	__RESERVED_FOR_LINKHOLD2WTILE 0x1112
#define 	__RESERVED_FOR_LINKPOUNDTILE 0x1113
#define __RESERVED_FOR_LINKSWIMSPD 0x1114
#define __RESERVED_FOR_LINKWALKANMSPD 0x1115
#define __RESERVED_FOR_LINKANIMTYPE 0x1116
#define LINKINVFRAME 0x1117
#define LINKCANFLICKER 0x1118
#define LINKHURTSFX 0x1119
#define NOACTIVESUBSC 0x111A
#define UNUSED111B 0x111B
#define ZELDABUILD 0x111C
#define ZELDABETA 0x111D
#define FFCID 0x111E
#define DMAPLEVELPAL 0x111F
#define __RESERVED_FOR_ITEMINDEX 0x1120
#define __RESERVED_FOR_LWPNINDEX 0x1121
#define __RESERVED_FOR_EWPNINDEX 0x1122
#define __RESERVED_FOR_NPCINDEX 0x1123
#define __RESERVED_FOR_ITEMPTR 0x1124
#define __RESERVED_FOR_NPCPTR 0x1125
#define __RESERVED_FOR_LWPNPTR 0x1126
#define __RESERVED_FOR_EWPNPTR 0x1127


#define SETSCREENDOOR 0x1128
#define SETSCREENENEMY 0x1129
#define GAMEMAXMAPS 0x112A
#define CREATELWPNDX 0x112B
#define __RESERVED_FOR_SCREENFLAG 0x112C
#define RESVD112D 0x112D
#define RESVD112E 0x112E

//Reserved values to use for cooperative editing. 
#define RESVD112F 0x112F
#define RESVD1130 0x1130
#define RESVD1131 0x1131
#define RESVD1132 0x1132
#define RESVD1133 0x1133
#define RESVD1134 0x1134
#define RESVD1135 0x1135
#define RESVD1136 0x1136
#define RESVD1137 0x1137
#define RESVD1138 0x1138
#define RESVD1139 0x1139
#define IDATACOSTCOUNTER 0x113A

#define IDATAOVERRIDEFLWEAP 0x113B
#define IDATATILEHWEAP 0x113C
#define IDATATILEWWEAP 0x113D
#define IDATAHZSZWEAP 0x113E
#define IDATAHYSZWEAP 0x113F
#define IDATAHXSZWEAP 0x1140
#define IDATADYOFSWEAP 0x1141
#define IDATADXOFSWEAP 0x1142
#define IDATAHYOFSWEAP 0x1143
#define IDATAHXOFSWEAP 0x1144
#define IDATAOVERRIDEFL 0x1145
#define IDATAPICKUP 0x1146
#define IDATATILEH 0x1147
#define IDATATILEW 0x1148
#define IDATAHZSZ 0x1149
#define IDATAHYSZ 0x114A
#define IDATAHXSZ 0x114B
#define IDATADYOFS 0x114C
#define IDATADXOFS 0x114D
#define IDATAHYOFS 0x114E
#define IDATAHXOFS 0x114F

#define IDATAUSEWPN 0x1150
#define IDATAUSEDEF 0x1151
#define IDATAWRANGE 0x1152
#define IDATAUSEMVT 0x1153
#define IDATADURATION 0x1154

#define IDATADUPLICATES 0x1155
#define IDATADRAWLAYER 0x1156
#define IDATACOLLECTFLAGS 0x1157
#define IDATAWEAPONSCRIPT 0x1158
#define IDATAMISCD 0x1159
#define IDATAWEAPHXOFS 0x115A
#define IDATAWEAPHYOFS 0x115B
#define IDATAWEAPHYSZ 0x115C
#define IDATAWEAPHXSZ 0x115D
#define IDATAWEAPHZSZ 0x115E
#define IDATAWEAPXOFS 0x115F
#define IDATAWEAPYOFS 0x1160
#define IDATAWEAPZOFS 0x1161
#define NPCWEAPSPRITE 0x1162
#define IDATAWPNINITD 0x1163
#define DEBUGREFFFC 0x1164
#define DEBUGREFITEM 0x1165
#define DEBUGREFNPC 0x1166
#define DEBUGREFITEMDATA 0x1167
#define DEBUGREFLWEAPON 0x1168
#define DEBUGREFEWEAPON 0x1169
#define DEBUGSP 0x116A
#define DEBUGGDR 0x116B
#define SCREENWIDTH 0x116C
#define SCREENHEIGHT 0x116D
#define SCREENVIEWX 0x116E
#define SCREENVIEWY 0x116F
#define SCREENGUY 0x1170
#define SCREENSTRING 0x1171
#define SCREENROOM 0x1172
#define SCREENENTX 0x1173
#define SCREENENTY 0x1174
#define SCREENITEM 0x1175
#define SCREENUNDCMB 0x1176
#define SCREENUNDCST 0x1177
#define SCREENCATCH 0x1178
#define SETSCREENLAYOP 0x1179
#define SETSCREENSECCMB 0x117A
#define SETSCREENSECCST 0x117B
#define SETSCREENSECFLG 0x117C
#define SETSCREENLAYMAP 0x117D
#define SETSCREENLAYSCR 0x117E
#define SETSCREENPATH 0x117F
#define SETSCREENWARPRX 0x1180
#define SETSCREENWARPRY 0x1181
#define GAMENUMMESSAGES 0x1182
#define GAMESUBSCHEIGHT 0x1183
#define GAMEPLAYFIELDOFS 0x1184
#define PASSSUBOFS 0x1185

//NPCData
#define SETNPCDATASCRIPTDEF  0x1186
#define SETNPCDATADEFENSE 0x1187
#define SETNPCDATASIZEFLAG 0x118 
#define SETNPCDATAATTRIBUTE 0x1189

#define SCDBLOCKWEAPON 0x118A
#define SCDSTRIKEWEAPONS 0x118B
#define SCDEXPANSION 0x118C
#define SETGAMEOVERELEMENT 0x118D
#define SETGAMEOVERSTRING 0x118E
#define MOUSEARR 0x118F

//DataType spritedata sd->

#define SPRITEDATATILE	0x1190
#define SPRITEDATAMISC	0x1191
#define SPRITEDATACSETS 0x1192
#define SPRITEDATAFRAMES 0x1193
#define SPRITEDATASPEED 0x1194
#define SPRITEDATATYPE 0x1195

#define NPCDATATILE 0x1196
#define NPCDATAHEIGHT 0x1197
#define NPCDATAFLAGS 0x1198
#define NPCDATAFLAGS2 0x1199
#define NPCDATAWIDTH 0x119A
#define NPCDATAHITSFX 0x119B
#define NPCDATASTILE 0x119C
#define NPCDATASWIDTH 0x119D
#define NPCDATASHEIGHT 0x119E
#define NPCDATAETILE 0x119F
#define NPCDATAEWIDTH 0x11A0
#define NPCDATAEHEIGHT 0x11A1
#define NPCDATAHP 0x11A2
#define NPCDATAFAMILY 0x11A3
#define NPCDATACSET 0x11A4
#define NPCDATAANIM 0x11A5
#define NPCDATAEANIM 0x11A6
#define NPCDATAFRAMERATE 0x11A7
#define NPCDATAEFRAMERATE 0x11A8
#define NPCDATATOUCHDAMAGE 0x11A9
#define NPCDATAWEAPONDAMAGE 0x11AA
#define NPCDATAWEAPON 0x11AB
#define NPCDATARANDOM 0x11AC
#define NPCDATAHALT 0x11AD
#define NPCDATASTEP 0x11AE
#define NPCDATAHOMING 0x11AF
#define NPCDATAHUNGER 0x11B0
#define NPCDATADROPSET 0x11B1
#define NPCDATABGSFX 0x11B2
#define NPCDATADEATHSFX 0x11B3
#define NPCDATAXOFS 0x11B4
#define NPCDATAYOFS 0x11B5
#define NPCDATAZOFS 0x11B6
#define NPCDATAHXOFS 0x11B7
#define NPCDATAHYOFS 0x11B8
#define NPCDATAHITWIDTH 0x11B9
#define NPCDATAHITHEIGHT 0x11BA
#define NPCDATAHITZ 0x11BB
#define NPCDATATILEWIDTH 0x11BC
#define NPCDATATILEHEIGHT 0x11BD
#define NPCDATAWPNSPRITE 0x11BE
#define NPCDATADEFENSE 0x11BF
#define NPCDATASIZEFLAG 0x11C0
#define NPCDATAATTRIBUTE 0x11C1
#define NPCDATASHIELD 0x11C2
#define NPCDATAFROZENTILE 0x11C3
#define NPCDATAFROZENCSET 0x11C4
#define NPCDATARESERVED005 0x11C5
#define NPCDATARESERVED006 0x11C6
#define NPCDATARESERVED007 0x11C7
#define NPCDATARESERVED008 0x11C8
#define NPCDATARESERVED009 0x11C9
#define NPCDATARESERVED010 0x11CA
#define NPCDATARESERVED011 0x11CB
#define NPCDATARESERVED012 0x11CC
#define NPCDATARESERVED013 0x11CD
#define NPCDATARESERVED014 0x11CE
#define NPCDATARESERVED015 0x11CF
#define NPCDATARESERVED016 0x11D0
#define NPCDATARESERVED017 0x11D1
#define NPCDATARESERVED018 0x11D2
#define NPCDATARESERVED019 0x11D3

#define MAPDATAVALID		0x11D4	//b
#define MAPDATAGUY 		0x11D5	//b
#define MAPDATASTRING		0x11D6	//w
#define MAPDATAROOM 		0x11D7	//b
#define MAPDATAITEM 		0x11D8	//b
#define MAPDATAHASITEM 		0x11D9	//b
#define MAPDATATILEWARPTYPE 	0x11DA	//b, 4 of these
#define MAPDATATILEWARPOVFLAGS 	0x11DB	//b, tilewarpoverlayflags
#define MAPDATADOORCOMBOSET 	0x11DC	//w
#define MAPDATAWARPRETX 	0x11DD	//b, 4 of these
#define MAPDATAWARPRETY 	0x11DE	//b, 4 of these
#define MAPDATAWARPRETURNC 	0x11DF	//w
#define MAPDATASTAIRX 		0x11E0	//b
#define MAPDATASTAIRY 		0x11E1	//b
#define MAPDATACOLOUR 		0x11E2	//w
#define MAPDATAENEMYFLAGS 	0x11E3	//b
#define MAPDATADOOR 		0x11E4	//b, 4 of these
#define MAPDATATILEWARPDMAP 	0x11E5	//w, 4 of these
#define MAPDATATILEWARPSCREEN 	0x11E6	//b, 4 of these
#define MAPDATAEXITDIR 		0x11E7	//b
#define MAPDATAENEMY 		0x11E8	//w, 10 of these
#define MAPDATAPATTERN 		0x11E9	//b
#define MAPDATASIDEWARPTYPE 	0x11EA	//b, 4 of these
#define MAPDATASIDEWARPOVFLAGS 	0x11EB	//b
#define MAPDATAWARPARRIVALX 	0x11EC	//b
#define MAPDATAWARPARRIVALY 	0x11ED	//b
#define MAPDATAPATH 		0x11EE	//b, 4 of these
#define MAPDATASIDEWARPSC 	0x11EF	//b, 4 of these
#define MAPDATASIDEWARPDMAP 	0x11F0	//w, 4 of these
#define MAPDATASIDEWARPINDEX 	0x11F1	//b
#define MAPDATAUNDERCOMBO 	0x11F2	//w
#define MAPDATAUNDERCSET 	0x11F3	//b
#define MAPDATACATCHALL 	0x11F4	//W
#define MAPDATAFLAGS 		0x11F5	//B, 11 OF THESE, flags, flags2-flags10
#define MAPDATACSENSITIVE 	0x11F6	//B
#define MAPDATANORESET 		0x11F7	//W
#define MAPDATANOCARRY 		0x11F8	//W
#define MAPDATALAYERMAP 	0x11F9	//B, 6 OF THESE
#define MAPDATALAYERSCREEN 	0x11FA	//B, 6 OF THESE
#define MAPDATALAYEROPACITY 	0x11FB	//B, 6 OF THESE
#define MAPDATATIMEDWARPTICS 	0x11FC	//W
#define MAPDATANEXTMAP 		0x11FD	//B
#define MAPDATANEXTSCREEN 	0x11FE	//B
#define MAPDATASECRETCOMBO 	0x11FF	//W, 128 OF THESE
#define MAPDATASECRETCSET 	0x1200	//B, 128 OF THESE
#define MAPDATASECRETFLAG 	0x1201	//B, 128 OF THESE
#define MAPDATAVIEWX 		0x1202	//W
#define MAPDATAVIEWY 		0x1203	//W
#define MAPDATASCREENWIDTH 	0x1204	//B
#define MAPDATASCREENHEIGHT 	0x1205	//B
#define MAPDATAENTRYX 		0x1206	//B
#define MAPDATAENTRYY 		0x1207	//B
#define MAPDATANUMFF 		0x1208	//INT16
#define MAPDATAFFDATA 		0x1209	//W, 32 OF THESE
#define MAPDATAFFCSET 		0x120A	//B, 32
#define MAPDATAFFDELAY 		0x120B	//W, 32
#define MAPDATAFFX 		0x120C	//INT32, 32 OF THESE
#define MAPDATAFFY 		0x120D	//..
#define MAPDATAFFXDELTA 	0x120E	//..
#define MAPDATAFFYDELTA 	0x120F	//..
#define MAPDATAFFXDELTA2 	0x1210	//..
#define MAPDATAFFYDELTA2 	0x1211	//..
#define MAPDATAFFFLAGS 		0x1212	//INT16, 23 OF THESE
#define MAPDATAFFWIDTH 		0x1213	//B, 32 OF THESE
#define MAPDATAFFHEIGHT 	0x1214	//B, 32 OF THESE
#define MAPDATAFFLINK 		0x1215	//B, 32 OF THESE
#define MAPDATAFFSCRIPT 	0x1216	//W, 32 OF THESE
#define MAPDATAINTID 		0x1217	//INT32 , 32 OF THESE, EACH WITH 10 INDICES. 
#define MAPDATAINITA 		0x1218	//INT32, 32 OF THESE, EACH WITH 2
#define MAPDATAFFINITIALISED 	0x1219	//BOOL, 32 OF THESE
#define MAPDATASCRIPTENTRY 	0x121A	//W
#define MAPDATASCRIPTOCCUPANCY 	0x121B	//W
#define MAPDATASCRIPTEXIT 	0x121C	//W
#define MAPDATAOCEANSFX 	0x121D	//B
#define MAPDATABOSSSFX 		0x121E	//B
#define MAPDATASECRETSFX 	0x121F	//B
#define MAPDATAHOLDUPSFX 	0x1220	//B
#define MAPDATASCREENMIDI 	0x1221	//SHORT, OLD QUESTS ONLY?
#define MAPDATALENSLAYER 	0x1222	//B, OLD QUESTS ONLY?

#define SCREENDATAVALID			0x1223	//b
#define SCREENDATAGUY 			0x1224	//b
#define SCREENDATASTRING		0x1225	//w
#define SCREENDATAROOM 			0x1226	//b
#define SCREENDATAITEM 			0x1227	//b
#define SCREENDATAHASITEM 		0x1228	//b
#define SCREENDATATILEWARPTYPE 		0x1229	//b, 4 of these
#define SCREENDATATILEWARPOVFLAGS 	0x122A	//b, tilewarpoverlayflags
#define SCREENDATADOORCOMBOSET 		0x122B	//w
#define SCREENDATAWARPRETX 		0x122C	//b, 4 of these
#define SCREENDATAWARPRETY 		0x122D	//b, 4 of these
#define SCREENDATAWARPRETURNC 		0x122E	//w
#define SCREENDATASTAIRX 		0x122F	//b
#define SCREENDATASTAIRY 		0x1230	//b
#define SCREENDATACOLOUR 		0x1231	//w
#define SCREENDATAENEMYFLAGS 		0x1232	//b
#define SCREENDATADOOR 			0x1233	//b, 4 of these
#define SCREENDATATILEWARPDMAP 		0x1234	//w, 4 of these
#define SCREENDATATILEWARPSCREEN 	0x1235	//b, 4 of these
#define SCREENDATAEXITDIR 		0x1236	//b
#define SCREENDATAENEMY 		0x1237	//w, 10 of these
#define SCREENDATAPATTERN 		0x1238	//b
#define SCREENDATASIDEWARPTYPE 		0x1239	//b, 4 of these
#define SCREENDATASIDEWARPOVFLAGS 	0x123A	//b
#define SCREENDATAWARPARRIVALX 		0x123B	//b
#define SCREENDATAWARPARRIVALY 		0x123C	//b
#define SCREENDATAPATH 			0x123D	//b, 4 of these
#define SCREENDATASIDEWARPSC 		0x123E	//b, 4 of these
#define SCREENDATASIDEWARPDMAP 		0x123F	//w, 4 of these
#define SCREENDATASIDEWARPINDEX 	0x1240	//b
#define SCREENDATAUNDERCOMBO 		0x1241	//w
#define SCREENDATAUNDERCSET 		0x1242	//b
#define SCREENDATACATCHALL 		0x1243	//W
#define SCREENDATAFLAGS 		0x1244	//B, 11 OF THESE, flags, flags2-flags10
#define SCREENDATACSENSITIVE 		0x1245	//B
#define SCREENDATANORESET 		0x1246	//W
#define SCREENDATANOCARRY 		0x1247	//W
#define SCREENDATALAYERMAP 		0x1248	//B, 6 OF THESE
#define SCREENDATALAYERSCREEN 		0x1249	//B, 6 OF THESE
#define SCREENDATALAYEROPACITY 		0x124A	//B, 6 OF THESE
#define SCREENDATATIMEDWARPTICS 	0x124B	//W
#define SCREENDATANEXTMAP 		0x124C	//B
#define SCREENDATANEXTSCREEN 		0x124D	//B
#define SCREENDATASECRETCOMBO 		0x124E	//W, 128 OF THESE
#define SCREENDATASECRETCSET 		0x124F	//B, 128 OF THESE
#define SCREENDATASECRETFLAG 		0x1250	//B, 128 OF THESE
#define SCREENDATAVIEWX 		0x1251	//W
#define SCREENDATAVIEWY 		0x1252	//W
#define SCREENDATASCREENWIDTH 		0x1253	//B
#define SCREENDATASCREENHEIGHT 		0x1254	//B
#define SCREENDATAENTRYX 		0x1255	//B
#define SCREENDATAENTRYY 		0x1256	//B
#define SCREENDATANUMFF 		0x1257	//INT16
#define SCREENDATAFFDATA 		0x1258	//W, 32 OF THESE
#define SCREENDATAFFCSET 		0x1259	//B, 32
#define SCREENDATAFFDELAY 		0x125A	//W, 32
#define SCREENDATAFFX 			0x125B	//INT32, 32 OF THESE
#define SCREENDATAFFY 			0x125C	//..
#define SCREENDATAFFXDELTA 		0x125D	//..
#define SCREENDATAFFYDELTA 		0x125E	//..
#define SCREENDATAFFXDELTA2 		0x125F	//..
#define SCREENDATAFFYDELTA2 		0x1260	//..
#define SCREENDATAFFFLAGS 		0x1261	//INT16, 23 OF THESE
#define SCREENDATAFFWIDTH 		0x1262	//B, 32 OF THESE
#define SCREENDATAFFHEIGHT 		0x1263	//B, 32 OF THESE
#define SCREENDATAFFLINK 		0x1264	//B, 32 OF THESE
#define SCREENDATAFFSCRIPT 		0x1265	//W, 32 OF THESE
#define SCREENDATAINTID 		0x1266	//INT32 , 32 OF THESE, EACH WITH 10 INDICES. 
#define SCREENDATAINITA 		0x1267	//INT32, 32 OF THESE, EACH WITH 2
#define SCREENDATAFFINITIALISED 	0x1268	//BOOL, 32 OF THESE
#define SCREENDATASCRIPTENTRY 		0x1269	//W
#define SCREENDATASCRIPTOCCUPANCY 	0x126A	//W
#define SCREENDATASCRIPTEXIT 		0x126B	//W
#define SCREENDATAOCEANSFX 		0x126C	//B
#define SCREENDATABOSSSFX 		0x126D	//B
#define SCREENDATASECRETSFX 		0x126E	//B
#define SCREENDATAHOLDUPSFX 		0x126F	//B
#define SCREENDATASCREENMIDI 		0x1270	//SHORT, OLD QUESTS ONLY?
#define SCREENDATALENSLAYER 		0x1271	//B, OLD QUESTS ONLY?

#define LINKSCRIPTTILE 0x1272

#define MAPDATAITEMX 0x1273
#define MAPDATAITEMY 0x1274
#define SCREENDATAITEMX 0x1275
#define SCREENDATAITEMY 0x1276

#define MAPDATAFFEFFECTWIDTH 0x1277
#define MAPDATAFFEFFECTHEIGHT 0x1278
#define SCREENDATAFFEFFECTWIDTH 0x1279
#define SCREENDATAFFEFFECTHEIGHT 0x127A

#define MAPDATAMISCD 0x127B

#define MAPDATACOMBODD 0x127C
#define MAPDATACOMBOCD 0x127D
#define MAPDATACOMBOFD 0x127E
#define MAPDATACOMBOTD 0x127F
#define MAPDATACOMBOID 0x1280
#define MAPDATACOMBOSD 0x1281

#define MAPDATASCREENSTATED 0x1282
#define MAPDATASCREENFLAGSD 0x1283
#define MAPDATASCREENEFLAGSD 0x1284
#define LINKSCRIPFLIP 0x1285

#define LOADMAPDATA 0x1286

#define SHOPDATANAME	0x1287
#define SHOPDATAITEM	0x1288
#define SHOPDATAHASITEM	0x1289
#define SHOPDATAPRICE	0x128A
//Pay for info
#define SHOPDATASTRING  0x128B

#define AUDIOVOLUME	0x128C
#define AUDIOPAN	0x128D

#define MESSAGEDATANEXT 	0x128E	//W
#define MESSAGEDATATILE 	0x128F	//W
#define MESSAGEDATACSET 	0x1290	//b
#define MESSAGEDATATRANS 	0x1291	//BOOL
#define MESSAGEDATAFONT 	0x1292	//B
#define MESSAGEDATAX 		0x1293	//SHORT
#define MESSAGEDATAY 		0x1294	//SHORT
#define MESSAGEDATAW 		0x1295	//UNSIGNED SHORT
#define MESSAGEDATAH 		0x1296	//UNSIGNED SHORT
#define MESSAGEDATASFX 		0x1297	//BYTE
#define MESSAGEDATALISTPOS 	0x1298	//WORD
#define MESSAGEDATAVSPACE 	0x1299	//BYTE
#define MESSAGEDATAHSPACE 	0x129A	//BYTE
#define MESSAGEDATAFLAGS 	0x129B	//BYTE


#define DMAPDATAMAP 		0x129C	//byte
#define DMAPDATALEVEL 		0x129D	//word
#define DMAPDATAOFFSET 		0x129E	//char
#define DMAPDATACOMPASS 	0x129F //byte
#define DMAPDATAPALETTE 	0x12A0 //word
#define DMAPDATAMIDI 		0x12A1 //byte
#define DMAPDATACONTINUE 	0x12A2 //byte
#define DMAPDATATYPE 		0x12A3 //byte
#define DMAPDATAGRID 		0x12A4 //byte[8] --array
#define DMAPDATAMINIMAPTILE 	0x12A5 //word - two of these, so let's do MinimapTile[2]
#define DMAPDATAMINIMAPCSET 	0x12A6 //byte - two of these, so let's do MinimapCSet[2]
#define DMAPDATALARGEMAPTILE 	0x12A7 //word -- two of these, so let's to LargemapTile[2]
#define DMAPDATALARGEMAPCSET 	0x12A8 //word -- two of these, so let's to LargemaCSet[2]
#define DMAPDATAMUISCTRACK 	0x12A9 //byte
#define DMAPDATASUBSCRA 	0x12AA //byte, active subscreen
#define DMAPDATASUBSCRP 	0x12AB //byte, passive subscreen
#define DMAPDATADISABLEDITEMS 	0x12AC //byte[iMax]
#define DMAPDATAFLAGS 		0x12AD //long
//Audio->Volume()
#define AUUDIOUNUSED0001		0x12AE
#define AUUDIOUNUSED0002		0x12AF

#define ITEMPSTRING			0x12B0
#define ITEMPSTRINGFLAGS		0x12B1
#define ITEMOVERRIDEFLAGS		0x12B2
#define LINKPUSH		0x12B3
#define GAMEMISC		0x12B4 //long
#define LINKSTUN		0x12B5 //int (32b)
#define TYPINGMODE		0x12B6 //int (32b)

//NEWCOMBO STRUCT
#define COMBODTILE		0x12B7	//word
#define COMBODFLIP		0x12B8	//char
#define COMBODWALK		0x12B9	//char
#define COMBODTYPE		0x12BA	//char
#define COMBODCSET		0x12BC	//C
#define COMBODFOO		0x12BD	//W
#define COMBODFRAMES		0x12BE	//C
#define COMBODNEXTD		0x12BF	//W
#define COMBODNEXTC		0x12C0	//C
#define COMBODFLAG		0x12C1	//C
#define COMBODSKIPANIM		0x12C2	//C
#define COMBODNEXTTIMER		0x12C3	//W
#define COMBODAKIMANIMY		0x12C4	//C
#define COMBODANIMFLAGS		0x12C5	//C
#define COMBODEXPANSION		0x12C6	//C , 6 INDICES
#define COMBODATTRIBUTES 	0x12C7	//LONG, 4 INDICES, INDIVIDUAL VALUES
#define COMBODUSRFLAGS		0x12C8	//LONG
#define COMBODTRIGGERFLAGS	0x12C9	//LONG 3 INDICES AS FLAGSETS
#define COMBODTRIGGERLEVEL	0x12CA	//LONG

//COMBOCLASS STRUCT
#define COMBODNAME		0x12CB	//CHAR[64], STRING
#define COMBODBLOCKNPC		0x12CC	//C
#define COMBODBLOCKHOLE		0x12CD	//C
#define COMBODBLOCKTRIG		0x12CE	//C
#define COMBODBLOCKWEAPON	0x12CF	//C, 32 INDICES
#define COMBODCONVXSPEED	0x12D0	//SHORT
#define COMBODCONVYSPEED	0x12D1	//SHORT
#define COMBODSPAWNNPC		0x12D2	//W
#define COMBODSPAWNNPCWHEN	0x12D3	//C
#define COMBODSPAWNNPCCHANGE	0x12D4	//LONG
#define COMBODDIRCHANGETYPE	0x12D5	//C
#define COMBODDISTANCECHANGETILES	0x12D6	//LONG
#define COMBODDIVEITEM		0x12D7	//SHORT
#define COMBODDOCK		0x12D8	//C
#define COMBODFAIRY		0x12D9	//C
#define COMBODFFATTRCHANGE	0x12DA	//C
#define COMBODFOORDECOTILE	0x12DB	//LONG
#define COMBODFOORDECOTYPE	0x12DC	//C
#define COMBODHOOKSHOTPOINT	0x12DD	//C
#define COMBODLADDERPASS	0x12DE	//C
#define COMBODLOCKBLOCK		0x12DF	//C
#define COMBODLOCKBLOCKCHANGE	0x12E0	//LONG
#define COMBODMAGICMIRROR	0x12E1	//C
#define COMBODMODHPAMOUNT	0x12E2	//SHORT
#define COMBODMODHPDELAY	0x12E3	//C
#define COMBODMODHPTYPE		0x12E4	//C
#define COMBODNMODMPAMOUNT	0x12E5	//SHORT
#define COMBODMODMPDELAY	0x12E6	//C
#define COMBODMODMPTYPE		0x12E7	//C
#define COMBODNOPUSHBLOCK	0x12E8	//C
#define COMBODOVERHEAD		0x12E9	//C
#define COMBODPLACENPC		0x12EA	//C
#define COMBODPUSHDIR		0x12EB	//C
#define COMBODPUSHWAIT		0x12EC	//C
#define COMBODPUSHHEAVY		0x12ED	//C
#define COMBODPUSHED		0x12EE	//C
#define COMBODRAFT		0x12EF	//C
#define COMBODRESETROOM		0x12F0	//C
#define COMBODSAVEPOINTTYPE	0x12F1	//C
#define COMBODSCREENFREEZETYPE	0x12F2	//C
#define COMBODSECRETCOMBO	0x12F3	//C
#define COMBODSINGULAR		0x12F4	//C
#define COMBODSLOWWALK		0x12F5	//C
#define COMBODSTATUETYPE	0x12F6	//C
#define COMBODSTEPTYPE		0x12F7	//C
#define COMBODSTEPCHANGEINTO	0x12F8	//LONG
#define COMBODSTRIKEWEAPONS	0x12F9	//BYTE, 32 INDICES. 
#define COMBODSTRIKEREMNANTS	0x12FA	//LONG
#define COMBODSTRIKEREMNANTSTYPE	0x12FB	//C
#define COMBODSTRIKECHANGE	0x12FC	//LONG
#define COMBODSTRIKEITEM	0x12FD	//SHORT
#define COMBODTOUCHITEM		0x12FE	//SHORT
#define COMBODTOUCHSTAIRS	0x12FF	//C
#define COMBODTRIGGERTYPE	0x1300	//C
#define COMBODTRIGGERSENS	0x1301	//C
#define COMBODWARPTYPE		0x1302	//C
#define COMBODWARPSENS		0x1303	//C
#define COMBODWARPDIRECT	0x1304	//C
#define COMBODWARPLOCATION	0x1305	//C
#define COMBODWATER		0x1306	//C
#define COMBODWHISTLE		0x1307	//C
#define COMBODWINGAME		0x1308	//C
#define COMBODBLOCKWPNLEVEL	0x1309	//C
#define LINKHITBY		0x130A	//byte[4]
#define LINKDEFENCE		0x130B	//byte[256]
#define NPCHITBY		0x130C	//byte[4]
#define NPCISCORE		0x130D	//byte[4]
#define NPCSCRIPTUID		0x130E	//byte[4]
#define LWEAPONSCRIPTUID	0x130F	//byte[4]
#define EWEAPONSCRIPTUID	0x1310	//byte[4]
#define ITEMSCRIPTUID		0x1311	//byte[4]
#define DMAPDATASIDEVIEW	0x1312	//byte[4]

#define DONULL			0x1313	
#define DEBUGD			0x1314	
#define GETPIXEL			0x1315	
#define DOUNTYPE			0x1316	

//bytecode

//#define DMAPDATAGRAVITY 	//unimplemented
//#define DMAPDATAJUMPLAYER 	//unimplemented
//end vars

#define NUMVARIABLES         0x1317


// Script types

#define SCRIPT_GLOBAL          0
#define SCRIPT_FFC             1
#define SCRIPT_SCREEN          2
#define SCRIPT_LINK            3
#define SCRIPT_ITEM            4
#define SCRIPT_LWPN            5
#define SCRIPT_NPC             6
#define SCRIPT_SUBSCREEN       7
#define SCRIPT_EWPN            8

struct quad3Dstruct
{
    int index;
    float pos[12], uv[8];
    int size[2], color[4];
};

struct triangle3Dstruct
{
    int index;
    float pos[9], uv[6];
    int size[2], color[3];
};

#endif

//extern FFCScript FFEngine;
