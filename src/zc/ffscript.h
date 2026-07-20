#ifndef FFSCRIPT_H_
#define FFSCRIPT_H_

#include "core/dmap.h"
#include "base/general.h"
#include "ffc.h"
#include "core/mapscr.h"
#include "core/misctypes.h"
#include "parser/parserDefs.h"
#include <utility>
#include <string>
#include <deque>
#include "components/zasm/debug_data.h"
#include "components/zasm/pc.h"
#include "zc/scripting/array_manager.h"
#include "zc/scripting/context_strings.h"
#include "zc/jit.h"
#include "zc/zelda.h"
#include "zc/hero.h"
#include "sprite_data.h"

extern std::string current_zasm_context;
extern std::string current_zasm_extra_context;

void scripting_log_error_with_context(std::string text);

template <typename... Args>
static void scripting_log_error_with_context(fmt::format_string<Args...> s, Args&&... args)
{
    std::string text = fmt::format(s, std::forward<Args>(args)...);
	scripting_log_error_with_context(text);
}

#define ZS_BYTE 255
#define ZS_CHAR 255
#define ZS_WORD 65535
#define ZS_SHORT 32767
#define ZS_LONG 214747
#define ZS_INT 214747
#define ZS_FIX 214748
#define ZS_DWORD 65535

#define MAX_LWPN_SPRITES 1024
#define MAX_EWPN_SPRITES 1024
#define MAX_NPC_SPRITES 1024
#define MAX_ITEM_SPRITES 1024

#define MAX_ZQ_LAYER 6
#define MIN_ZQ_LAYER 0
#define MAX_FLAGS 512

#define MIN_INTERNAL_BITMAP -1 //RT_SCREEN
#define MAX_INTERNAL_BITMAP 6 //RT_BITMAP6
#define FFRULES_SIZE 1024

//Hero->WarpEx Flags
#define warpFlagDONTKILLSCRIPTDRAWS   0x001
#define warpFlagDONTKILLSOUNDS        0x002
#define warpFlagFORCERESETMUSIC       0x004
#define warpFlagSETENTRANCESCREEN     0x008
#define warpFlagSETENTRANCEDMAP       0x010
#define warpFlagSETCONTINUESCREEN     0x020
#define warpFlagSETCONTINUEDMAP       0x040
#define warpFlagDONTRESTARTDMAPSCRIPT 0x080
#define warpFlagDONTCLEARSPRITES      0x100
#define warpFlagFORCECONTINUEMUSIC    0x200

#define warpFlagCLEARITEMS 0x200
#define warpFlagCLEARGUYS 0x400
#define warpFlagCLEARLWEAPONS 0x800
#define warpFlagCLEAREWEAPONS 0x1000
#define warpFlagCLEARHOOKSHOT 0x2000
#define warpFlagCLEARDECORATIONS 0x4000
#define warpFlagCLEARPARTICLES 0x8000
#define warpFlagNOSTEPFORWARD 0x10000



//Visual Warp Effect Types for Hero->WarpEx()
enum { warpEffectNONE, warpEffectZap, warpEffectWave, warpEffectInstant, warpEffectOpen, warpEffectNONE2 };
void doWarpEffect(int32_t warpEffect, bool out);

int scripting_read_pal_color(int c);
int scripting_write_pal_color(int c);

void apply_qr_rule(int qr_id);

enum herospritetype
{
	LSprwalkspr, LSprstabspr, LSprslashspr, LSprfloatspr,
	LSprswimspr, LSprdivespr, LSprpoundspr, LSprjumpspr,
	LSprchargespr, LSprcastingspr, LSprholdspr1, LSprholdspr2,
	LSprholdsprw1, LSprholdsprw2, LSprdrownspr, LSprlavadrownspr, 
	LSprsideswimspr, LSprsideswimslashspr, LSprsideswimstabspr, 
	LSprsideswimpoundspr, LSprsideswimchargespr, LSprholdsprSw1, 
	LSprholdsprSw2, LSprsideswimcastingspr, LSprsidedrownspr, LSprrevslashspr, LSprlast
};

//suspend types
enum { 
	//Typical processes that we want to pause, similar to ALLOFF()
	
	//0 : Combo animation
	susptCOMBOANIM,
	
	//1->5 Main Sprite animation by type
	susptGUYS, susptLWEAPONS, susptEWEAPONS, susptITEMS, susptHERO, 
	
	//6 : FFC (e.g. movement, changers, but not scripts)
	susptUPDATEFFC, //ffcs
	
	//7->8 : Sprite subclasses
	susptDECORATIONS, susptPARTICLES, //sprite subclasses
	
	//9->10: Palette events
	susptPALCYCLE, susptLAKES, //lake dries up
	
	//11->15 : game system events
	susptCOLLISIONS, susptCONTROLSTATE, susptONEFRAMECONDS, susptSCRIPDRAWCLEAR, susptQUAKE,

	//16->27 Script Types
	susptGLOBALGAME, susptNPCSCRIPTS, susptLWEAPONSCRIPTS, susptEWEAPONSCRIPTS, susptITEMSPRITESCRIPTS,
	//21
	susptFFCSCRIPTS, susptHEROACTIVE, susptITEMSCRIPTENGINE, susptDMAPSCRIPT, susptSCREENSCRIPTS,
	susptSUBSCREENSCRIPTS, susptCOMBOSCRIPTS, //27
	
	//28->59: Reserved padding
	
	//60->42 : Moving items
	susptCONVEYORSITEMS = 60, susptDRAGGINGITEM, susptROAMINGITEM,
	//63->67 : Misc
	susptLENS, susptHOOKSHOT, susptMOVINGBLOCKS, susptMAGICCAST, susptSCREENDRAW,
	//68
	susptLAST };

//npc function enums
// npc_collision()
	//bool npc->Collision
enum { obj_type_lweapon, obj_type_eweapon, obj_type_npc, obj_type_player, 
	obj_type_ffc, obj_type_combo_pos, obj_type_item, obj_type_LAST };
	
//do_npc_simulate_hit
	//npc->SimulateHit()
enum { simulate_hit_type_weapon, simulate_hit_type_sprite };	

//Flags for SetVolume() and GetVolume() that are set in FFCoreFlags[] 
//these determine what is under script control and are used for automatic restoration
//of user volume settings on quest exit.
enum {
	FFCORE_SCRIPTED_MIDI_VOLUME 	= 0x0001,
	FFCORE_SCRIPTED_DIGI_VOLUME 	= 0x0002,
	FFCORE_SCRIPTED_MUSIC_VOLUME 	= 0x0004,
	FFCORE_SCRIPTED_SFX_VOLUME 	= 0x0008,
	FFCORE_SCRIPTED_PANSTYLE 	= 0x0010
};

//SYstem Date and Time Categories for GetSystemTime()
enum { curyear, curmonth, curday_month, curday_week, curhour, 
	curminute, cursecond, curdayyear, curDST, curTimeLAST };

	
enum { wexType, wexDMap, wexScreen, wexX, wexY, wexEffect, wexSound, wexFlags, wexDir, wexActive, wexLast };


//Script drawing: (t/b/a)
//Allow or forbid drawing during specific game events. 
enum{
	scdrDRAW_WHILE_SCROLLING, scdrDRAW_DURING_SCREEN_TRANSITION, scdrDRAW_DURING_WARP,
	scdrDRAW_DURING_WIPES, scdrLAST
};

//Quest Version Information Categories
//These reflect the version details from when the quest was last saved.
enum 
{
	vZelda, qQuestNumber, vBuild, vBeta, vHeader, vRules, vStrings, vMisc,
	vTiles, vCombos, vCSets, vMaps, vDMaps, vDoors, vItems, vWeaponSprites,
	vColours, vIcons, vGfxPack, vInitData, vGuys, vMIDIs, vCheats, vSaveformat,
	vComboAliases, vHeroSprites, vSubscreen, vItemDropsets, vFFScript, vSFX, vFavourites,
	qMapCount, vLastCompile, vCompatRule, vCompileSetting,
	versiontypesLAST
	
};

//Quest Header 'User Set Version' categories. Not in use at present.
enum
{
	 qQuestVersion, qMinQuestVersion, qvLAST
};

// go see resources/include/bindings/game.zh for documentation on these things.
enum //ScrollingData indexes
{
	SCROLLDATA_DIR,

	SCROLLDATA_NX, SCROLLDATA_NY, SCROLLDATA_OX, SCROLLDATA_OY,

	SCROLLDATA_NRX, SCROLLDATA_NRY, SCROLLDATA_ORX, SCROLLDATA_ORY,

	SCROLLDATA_NEW_REGION_DELTA_X, SCROLLDATA_NEW_REGION_DELTA_Y,

	SCROLLDATA_NEW_SCREEN_X, SCROLLDATA_NEW_SCREEN_Y, SCROLLDATA_OLD_SCREEN_X, SCROLLDATA_OLD_SCREEN_Y,

	SCROLLDATA_NEW_HERO_X, SCROLLDATA_NEW_HERO_Y, SCROLLDATA_OLD_HERO_X, SCROLLDATA_OLD_HERO_Y,

	SCROLLDATA_NEW_REGION_SCREEN_WIDTH, SCROLLDATA_NEW_REGION_SCREEN_HEIGHT,
	SCROLLDATA_OLD_REGION_SCREEN_WIDTH, SCROLLDATA_OLD_REGION_SCREEN_HEIGHT,

	SCROLLDATA_NEW_VIEWPORT_WIDTH, SCROLLDATA_NEW_VIEWPORT_HEIGHT,
	SCROLLDATA_OLD_VIEWPORT_WIDTH, SCROLLDATA_OLD_VIEWPORT_HEIGHT,

	SCROLLDATA_NEW_VIEWPORT_X, SCROLLDATA_NEW_VIEWPORT_Y,
	SCROLLDATA_OLD_VIEWPORT_X, SCROLLDATA_OLD_VIEWPORT_Y,

	SZ_SCROLLDATA
};

#define MAX_USER_RNGS 256
struct user_rng : public script_object_base
{
	zc_randgen* gen;
	
	int32_t rand()
	{
		return zc_rand(gen);
	}
	int32_t rand(int32_t upper, int32_t lower=0)
	{
		return zc_rand(upper, lower, gen);
	}
	void srand(int32_t seed)
	{
		zc_game_srand(seed, gen);
	}
	int32_t srand()
	{
		int32_t seed = time(0) + ((int64_t(rand()) * int64_t(rand())) * ((rand() % 2) ? 1 : -1));
		srand(seed);
		return seed;
	}
	void set_gen(zc_randgen* newgen)
	{
		gen = newgen;
		if(newgen) srand();
	}
};

enum class wdata_type
{
	script,
	npcdata,
	npc,
	itemdata,
	combodata_lift,
	combodata_misc,
};
#define MAX_USER_WEAPONDATAS 2048
struct user_weapondata : public script_object_base
{
	wdata_type data_type;
	dword data_index;
	
	weapon_data* inner_data = nullptr;
	
	weapon_data* get_data();
	
	~user_weapondata()
	{
		if (inner_data)
			delete inner_data;
	}
};

extern bool scripting_use_8bit_colors;
extern int scripting_max_color_val;

#define MAX_USER_PALDATAS 256
#define PALDATA_NUM_COLORS 256
#define PALDATA_BITSTREAM_SIZE 32
struct user_paldata : public script_object_base
{
	RGB colors[PALDATA_NUM_COLORS];
	byte colors_used[PALDATA_BITSTREAM_SIZE]; //A set of 256 bitflags

	enum { CSPACE_RGB, CSPACE_CMYK, CSPACE_HSV, CSPACE_HSV_CW, CSPACE_HSV_CCW, CSPACE_HSL, CSPACE_HSL_CW, CSPACE_HSL_CCW, CSPACE_LAB, CSPACE_LCH, CSPACE_LCH_CW, CSPACE_LCH_CCW };
	
	//Sets a color index on the paldata
	void set_color(int32_t ind, RGB c)
	{
		c.r = vbound(c.r, 0, scripting_max_color_val);
		c.g = vbound(c.g, 0, scripting_max_color_val);
		c.b = vbound(c.b, 0, scripting_max_color_val);
		colors[ind] = c;
		set_bit(colors_used, ind, true);
	}

	void load_cset(int32_t cset, int32_t dataset);
	void write_cset(int32_t cset, int32_t dataset);
	bool check_cset(int32_t cset, int32_t dataset);
	void load_cset_main(int32_t cset);
	void write_cset_main(int32_t cset);
	bool check_cset_main(int32_t cset);
	static RGB mix_color(RGB start, RGB end, double percent, int32_t color_space = CSPACE_RGB);
	static void RGBTo(RGB c, double arr[], int32_t color_space);
	static RGB RGBFrom(double arr[], int32_t color_space);
	static double HueToRGB(double v1, double v2, double vH);
	static double WrapLerp(double a, double b, double t, double min, double max, int32_t direction);
	void mix(user_paldata *pal_start, user_paldata *pal_end, double percent, int32_t color_space = CSPACE_RGB, int32_t start_color = 0, int32_t end_color = 240);
};

#define MAX_SCRIPT_ARRAYS 100000

script_array* create_script_array();
void register_existing_script_array(script_array* array);
std::vector<script_array*> get_script_arrays();
script_array* find_or_create_internal_script_array(script_array::internal_array_id internal_id);
script_array* checkArray(uint32_t id, bool skipError = false);

extern bool script_is_within_debugger_vm;
extern bool suppress_script_error_logging;
extern bool disable_script_error_logs;

int32_t run_script_jit_sequence(JittedScriptInstance* j_instance, pc_t pc, uint32_t sp, int32_t count);
int32_t run_script_jit_one(JittedScriptInstance* j_instance, pc_t pc, uint32_t sp);
int32_t run_script_jit_until_call_or_return(JittedScriptInstance* j_instance, pc_t pc, uint32_t sp);
int32_t run_script_int(JittedScriptInstance* j_instance = nullptr);

void clearConsole();

enum
{
	GENSCR_ST_RELOAD,
	GENSCR_ST_CONTINUE,
	GENSCR_ST_CHANGE_SCREEN,
	GENSCR_ST_CHANGE_DMAP,
	GENSCR_ST_CHANGE_LEVEL,
	GENSCR_NUMST
};
enum
{
	GENSCR_EVENT_NIL = -1,
	GENSCR_EVENT_INIT,
	GENSCR_EVENT_CONTINUE,
	GENSCR_EVENT_FFC_PRELOAD,
	GENSCR_EVENT_CHANGE_SCREEN,
	GENSCR_EVENT_CHANGE_DMAP,
	GENSCR_EVENT_CHANGE_LEVEL,
	GENSCR_EVENT_HERO_HIT_1,
	GENSCR_EVENT_HERO_HIT_2,
	GENSCR_EVENT_COLLECT_ITEM,
	GENSCR_EVENT_ENEMY_DROP_ITEM_1,
	GENSCR_EVENT_ENEMY_DROP_ITEM_2,
	GENSCR_EVENT_ENEMY_DEATH,
	GENSCR_EVENT_ENEMY_HIT1,
	GENSCR_EVENT_ENEMY_HIT2,
	GENSCR_EVENT_POST_COLLECT_ITEM,
	GENSCR_EVENT_PLAYER_FALL,
	GENSCR_EVENT_PLAYER_DROWN,
	GENSCR_NUMEVENT
};
enum
{
	GENEVT_ICTYPE_COLLECT, //Hero collected the item
	GENEVT_ICTYPE_MELEE, //Melee lweapon collected the item
	GENEVT_ICTYPE_MELEE_EW, //Melee eweapon collected the item
	GENEVT_ICTYPE_RANGED_DRAG //Ranged weapon collected, and will drag
};

extern int32_t genscript_timing;
void countGenScripts();
void timeExitAllGenscript(byte exState);
void throwGenScriptEvent(int32_t event);
void load_genscript(const gamedata& gd);
void load_genscript(const zinitdata& gd);
void save_genscript(gamedata& gd);

struct mapdata;

newcombo* checkComboFromTriggerRef(dword ref);
weapon* checkLWpn(int32_t uid);
weapon* checkEWpn(int32_t uid);
SubscrPage *checkSubPage(int32_t ref, std::set<int> const& req_tys = {});
combo_trigger* get_first_combo_trigger();
combo_trigger* get_combo_trigger(dword ref);

std::tuple<byte,int8_t,byte,word> from_subref(dword ref);
dword get_subref(int sub, byte ty, byte pg = 0, word ind = 0);

struct StackFrame
{
	const SourceFile* source_file;
	int line;
	pc_t pc;
	std::string function_name;
	std::string extra;

	std::string to_string() const;
	std::string to_short_string() const;
};

struct StackTrace
{
	std::vector<StackFrame> frames;

	std::string to_string() const;
};

class FFScript
{
	
    
public:
//FFScript();
void init(bool for_continue = false);
void shutdown();


int32_t max_ff_rules;
std::vector<mapscr*> ScrollingScreensAll;
int32_t ScrollingData[SZ_SCROLLDATA];
std::vector<int32_t> eventData;
ZCSubscreen* queued_subscreen;

int32_t getQRBit(int32_t rule);
void setHeroAction(int32_t a);
int32_t getHeroAction();
int32_t getTime(int32_t type); //get system RTC Information.
void getRTC();
int32_t getQuestHeaderInfo(int32_t type);
void do_graphics_getpixel();

void set_mapscreenflag_state(mapscr *m, int32_t flagid, bool state);
int32_t get_mapscreenflag_state(mapscr *m, int32_t flagid);

void clearRunningItemScripts();
bool itemScriptEngine();
void npcScriptEngineOnWaitdraw();
bool itemScriptEngineOnWaitdraw();
void lweaponScriptEngineOnWaitdraw();
void eweaponScriptEngine();
void eweaponScriptEngineOnWaitdraw();
void itemSpriteScriptEngine();
void itemSpriteScriptEngineOnWaitdraw();
void warpScriptCheck();
void runWarpScripts(bool waitdraw);
void runF6Engine();
void runOnDeathEngine();
void runOnLaunchEngine();
void runGenericPassiveEngine(int32_t scrtm);
bool runGenericFrozenEngine(script_config const& scrconfig);
bool runGenericFrozenEngine(const word script);
bool runScriptedActiveSubscreen();
bool runOnMapScriptEngine();
void doScriptMenuDraws();
void runOnSaveEngine();
void initIncludePaths();
void updateRunString();
void updateIncludePaths();
bool checkExtension(std::string &filename, const std::string &extension);
//String.h functions for ffscript - 2.55 Alpha 23
void do_strcmp();
void do_strncmp();
void do_strcpy(const bool a, const bool b);
void do_strlen(const bool v);
//More string.h functions, 19th May, 2019 
void do_arraycpy(const bool a, const bool b);
void AlloffLimited(int32_t flagset);
void do_xlen(const bool v);
double ln(double temp);
double Log2( double n );
int32_t numDigits(int32_t number);
int32_t Log10(double temp);
double LogToBase(double x, double base);
void do_xtoi(const bool v);
void do_ilen(const bool v);
void do_atoi(const bool v);
void do_atol(const bool v);
bool isNumber(char chr);
int32_t ilen(char *p);
int32_t atox(char *ip_str);
void do_LowerToUpper(const bool v);
void do_UpperToLower(const bool v);
void do_ConvertCase(const bool v);
void do_stricmp();
void do_strnicmp();

void do_getgenericscript();
void do_getcomboscript();
void do_getnpcscript();
void do_getlweaponscript();
void do_geteweaponscript();
void do_getheroscript();
void do_getglobalscript();
void do_getdmapscript();
void do_getscreenscript();
void do_getitemspritescript();
void do_getuntypedscript();
void do_getsubscreenscript();
void do_getnpcbyname();
void do_getitembyname();
void do_getcombobyname();
void do_getmusicdatabyname();
void do_getdmapbyname();

int32_t getHeroOTile(int32_t index1, int32_t index2);
int32_t getHeroOFlip(int32_t index1, int32_t index2);

int32_t IsBlankTile(int32_t i);
int32_t Is8BitTile(int32_t i);

defWpnSprite getDefWeaponSprite(int32_t wpnid);

int32_t Distance(double x1, double y1, double x2, double y2);
int32_t Distance(double x1, double y1, double x2, double y2, int32_t scale);
int32_t LongDistance(double x1, double y1, double x2, double y2);
int32_t LongDistance(double x1, double y1, double x2, double y2, int32_t scale);

int32_t combo_script_engine(const bool preload, const bool waitdraw = false);

void do_strstr();
void do_strcat();
void do_strspn();
void do_strcspn();
void do_strchr();
void do_strrchr();
void do_xtoi2();
void do_remchr2();
void do_atoi2();
void do_ilen2();
void do_xlen2();
void do_itoa();
void do_itoacat();
void do_xtoa();

void do_tracebool(const bool v);
void do_tracestring();
void do_printf(const bool v, const bool varg);
void do_printfarr();
void do_varg_max();
void do_varg_min();
void do_varg_choose();
void do_varg_makearray(ScriptType type, const uint32_t UID, script_object_type object_type);
void do_breakpoint();
std::optional<StackFrame> get_script_stack_frame(int pc);
void handle_trace(const std::string& s, bool is_error = false, bool no_prefix = false);
std::optional<StackTrace> create_stack_trace(const refInfo* ri);
// Whether script error/Trace output should include source stack traces. Also
// gates the JIT upkeep (ri->pc, the call stack) that only those traces consume.
bool should_display_stack_traces();
void do_trace(bool v);
void do_tracel(bool v);
void do_tracenl();
void do_cleartrace();
void do_tracetobase();
void ZScriptConsole(bool open);
template <typename ...Params>
void ZScriptConsole(int32_t attributes,const char *format, Params&&... params);
std::string GetScriptName(ScriptType script_type, int script_num);
std::string GetScriptDataName(ScriptType script_type, int script_num);
void PrintTracePrefix(bool force_show_context = false, bool is_error = false);
/*
int32_t getQuestHeaderInfo(int32_t type)
{
    return quest_format[type];
}


*/

//Script-only Warp, Hero->WarpEx(int32_t type, int32_t dmap, int32_t screen, int32_t x, int32_t y, int32_t effect, int32_t sound, int32_t flags, int32_t dir)
//Script-only Warp, Hero->WarpEx(int32_t array[])
//{int32_t type, int32_t dmap, int32_t screen, int32_t x, int32_t y, int32_t effect, int32_t sound, int32_t flags, int32_t dir}
void queueWarp(int32_t wtype, int32_t tdm, int32_t tscr, int32_t wx, int32_t wy,
	int32_t weff, int32_t wsfx, int32_t wflag, int32_t wdir);
bool warp_player(int32_t warpType, int32_t dmap, int32_t screen, int32_t warpDestX, int32_t warpDestY, int32_t warpEffect, int32_t warpSound, int32_t warpFlags, int32_t heroFacesDir);

void user_files_init();
void user_dirs_init();
void user_objects_init();
void user_stacks_init();
void user_rng_init();
void user_paldata_init();
void user_websockets_init();
void user_weapondata_init();
void script_arrays_init();

uint32_t get_new_weapondata(wdata_type data_type, dword data_index);
void do_loadstack();
void do_loadrng();
void do_create_paldata();
void do_create_paldata_clr();
void do_mix_clr();
void do_create_rgb_hex();
void do_create_rgb();
void do_convert_from_rgb();
void do_convert_to_rgb();
void do_paldata_load_level();
void do_paldata_load_sprite();
void do_paldata_load_main();
void do_paldata_load_cycle();
void do_paldata_load_bitmap();
void do_paldata_write_level();
void do_paldata_write_levelcset();
void do_paldata_write_sprite();
void do_paldata_write_spritecset();
void do_paldata_write_main();
void do_paldata_write_maincset();
void do_paldata_write_cycle();
void do_paldata_write_cyclecset();
void do_paldata_colorvalid();
void do_paldata_getcolor();
void do_paldata_setcolor();
void do_paldata_clearcolor();
void do_paldata_clearcset();
int32_t do_paldata_getrgb(user_paldata* pd, int32_t index, int32_t c);
void do_paldata_setrgb(user_paldata* pd, int32_t index, int32_t val, int32_t c);
void do_paldata_mix();
void do_paldata_mixcset();
void do_paldata_copy();
void do_paldata_copycset();

void user_bitmaps_init();

uint32_t get_free_bitmap(bool skipError = false);
void do_deallocate_bitmap();
bool isSystemBitref(int32_t ref);

uint32_t create_user_bitmap_ex(int32_t w, int32_t h);
void do_isvalidbitmap();
void do_isallocatedbitmap();

bool can_change_music_within_region();
bool can_dmap_change_music(int32_t dm);
void do_set_music_position(const bool v);
void do_get_music_position();
void do_set_music_speed(const bool v);
void do_get_music_length();
void do_set_music_loop();

bool doesResolveToScreenBitmap(int32_t bitmap_id);
bool doesResolveToDeprecatedSystemBitmap(int32_t bitmap_id);

BITMAP* GetScriptBitmap(int32_t id, BITMAP* screen_bitmap, bool skipError = false);

int32_t highest_valid_user_bitmap();
int32_t do_create_bitmap();

void do_adjustsfxvolume(const bool v);
void do_adjustvolume(const bool v);
void do_warp_ex();
void do_warp_ex_array();
//FFScript();
//static void init();

int32_t quest_format[versiontypesLAST];
byte skip_ending_credits; //checked in ending.cpp. If > 0, then we skip the game credits, but not Hero's Win script. -Z

bool system_suspend[susptLAST];

int32_t coreflags;
int32_t usr_midi_volume, usr_digi_volume, usr_sfx_volume, usr_music_volume, usr_panstyle;
byte music_update_cond, music_update_flags;

byte FF_hero_action; //This way, we can make safe replicas of internal Hero actions to be set by script. 
bool kb_typing_mode; //script only, for disbaling key presses affecting Hero, etc. 
int32_t FF_screenbounds[4]; //edges of the screen, left, right, top, bottom used for where to scroll. 
int32_t FF_screen_dimensions[4]; //height, width, displaywidth, displayheight
int32_t FF_subscreen_dimensions[4];
int32_t numscriptdraws;
bool skipscriptdraws;
int32_t FF_eweapon_removal_bounds[4]; //left, right, top, bottom coordinates for automatic eweapon removal. 
int32_t FF_lweapon_removal_bounds[4]; //left, right, top, bottom coordinates for automatic lweapon removal. 

std::vector<std::string> includePaths;
char includePathString[MAX_INCLUDE_PATH_CHARS];
int32_t warpex[wexLast];

#define FFSCRIPTCLASS_CLOCKS 10
int32_t FF_clocks[FFSCRIPTCLASS_CLOCKS]; //Will be used for Heroaction, anims, and so forth 

#define SCRIPT_DRAWING_RULES 20
byte ScriptDrawingRules[SCRIPT_DRAWING_RULES];

#define NUM_USER_MIDI_OVERRIDES 6
int32_t FF_UserMidis[NUM_USER_MIDI_OVERRIDES]; //MIDIs to use for Game Over, and similar to override system defaults. 

//Enemy removal bounds
int32_t enemy_removal_point[6];

//Disable stepforward on dungeon dmaps
byte nostepforward;
byte temp_no_stepforward; 

byte subscreen_scroll_speed;

bool show_zasm_stack_traces;

void setSubscreenScrollSpeed(byte n);
int32_t getSubscreenScrollSpeed();
void do_fx_zap(const bool v);
void do_fx_wavy(const bool v);
void do_greyscale(const bool v);
void do_monochromatic(const bool v);
void gfxmonohue();
void Tint();
void clearTint();

void initZScriptDMapScripts();
void initZScriptOnMapScript();
void initZScriptScriptedActiveSubscreen();
void initZScriptSubscreenScript();
void initZScriptHeroScripts();
void initZScriptItemScripts();

void clear_script_engine_data();
void clear_script_engine_data_for_continue();
void reset_script_engine_data(ScriptType type, int index = 0);
void clear_script_engine_data(ScriptType type, int index = 0);
void clear_script_engine_data_of_type(ScriptType type);
refInfo& ref(ScriptType type, int index);
void clear_ref(ScriptType type, int index);
byte& doscript(ScriptType type, int index = 0);
bool& waitdraw(ScriptType type, int index = 0);

//Combo Scripts
void clear_combo_scripts();
void clear_combo_script(const rpos_handle_t& rpos_handle);

void SetFFEngineFlag(int32_t flag, bool v);
void SetItemMessagePlayed(int32_t itm);
bool GetItemMessagePlayed(int32_t itm);
	
void do_bmpcollision();
int32_t GetDefaultWeaponSprite(int32_t w);
int32_t GetDefaultWeaponSFX(int32_t wpn_id);

int32_t GetQuestVersion();
int32_t GetQuestBuild();
int32_t GetQuestSectionVersion(int32_t section);

void do_loadnpc_by_script_uid(const bool v);
void do_loaditem_by_script_uid(const bool v);
void do_loadlweapon_by_script_uid(const bool v);
void do_loadeweapon_by_script_uid(const bool v);

//Deletion functions
void do_lweapon_delete();
void do_eweapon_delete();

static void deallocateArray(int32_t ptrval);
static int32_t get_screen_d(int32_t index1, int32_t index2);
static void set_screen_d(int32_t index1, int32_t index2, int32_t val);

static void do_zapout();
static void do_zapin();
static void do_openscreen();
static void do_closescreen();
static void do_openscreenshape();
static void do_closescreenshape();
static void do_wavyin();
static void do_wavyout();
static void do_triggersecret(const bool v);

	//Change Save/Continue and Retry screen settings:
	static void FFSetSaveScreenSetting();
	static void FFChangeSubscreenText();
	
	//New Datatypes
	static void do_loadnpcdata(const bool v);
	static void do_loadcombodata(const bool v);
	static void do_loadmapdata_tempscr(const bool v);
	static void do_loadmapdata_tempscr2(const bool v);
	static void do_loadmapdata_scrollscr(const bool v);
	static void do_loadmapdata_scrollscr2(const bool v);
	static int32_t loadMapData();
	static void do_loadspritedata(const bool v);
	static void do_loadbitmapid(const bool v);
	static void do_loadshopdata(const bool v);
	static void do_loadinfoshopdata(const bool v);
	static void do_setMIDI_volume(int32_t m);
	static void do_setMusic_volume(int32_t m);
	static void do_setDIGI_volume(int32_t m);
	static void do_setSFX_volume(int32_t m);
	static void do_setSFX_pan(int32_t m);
	
	static int32_t do_getMIDI_volume();
	static int32_t do_getMusic_volume();
	static int32_t do_getDIGI_volume();
	static int32_t do_getSFX_volume();
	static int32_t do_getSFX_pan();
	
	static void do_loadmessagedata(const bool v);
	static void do_messagedata_setstring(const bool v);
	static void do_messagedata_getstring(const bool v);
	
	static void do_loaddmapdata(const bool v);
	static void do_loaddropset(const bool v);
	static void do_loadbottle(const bool v);
	static void do_loadbottleshop(const bool v);
	static void do_loadgenericdata(const bool v);
	static void do_getDMapData_dmapname(const bool v);
	static void do_setDMapData_dmapname(const bool v);
	static void do_getDMapData_dmaptitle(const bool v);
	static void do_setDMapData_dmaptitle(const bool v);
	static void do_getDMapData_dmapintro(const bool v);
	static void do_setDMapData_dmapintro(const bool v);
	static void do_getDMapData_music(const bool v);
	static void do_setDMapData_music(const bool v);
	
	static void do_load_active_subscreendata(const bool v);
	static void do_load_passive_subscreendata(const bool v);
	static void do_load_overlay_subscreendata(const bool v);
	static void do_load_map_subscreendata(const bool v);
	static void do_load_subscreendata(const bool v, const bool v2);
	
	static void do_checkdir(const bool is_dir);
	static void do_fs_remove();

#define INVALIDARRAY localRAM[0]  //localRAM[0] is never used

enum __Error
    {
        _NoError, //OK!
        _Overflow, //script array too small
        _InvalidPointer, //passed NULL pointer or similar
        _OutOfBounds, //library array out of bounds
        _InvalidSpriteUID //bad npc, ffc, etc.
    };

	static void release_sprite_owned_objects(int32_t sprite_id);
	static void destroySprite(sprite* sprite);
	static void deallocateAllScriptOwned(ScriptType scriptType, const int32_t UID);
	static void deallocateAllScriptOwnedOfType(ScriptType scriptType);
	static void deallocateAllScriptOwned();
	static void deallocateAllScriptOwnedCont();
	
    private:
    int32_t sid;
};

extern PALETTE tempgreypal; //script greyscale
extern PALETTE userPALETTE[256];
extern PALETTE tempblackpal;

void script_init_name_to_slot_index_maps();

int32_t get_register(int32_t arg);
void set_register(int32_t arg, int32_t value);

// POD-array access helpers, called from the JIT backends.
int32_t jit_pod_read(int32_t arrayptr, int32_t index, int32_t pc, int32_t no_neg);
void jit_pod_write(int32_t arrayptr, int32_t index, int32_t value, int32_t type, int32_t pc, int32_t no_neg);
int32_t jit_allocatemem(int32_t size, int32_t object_type, int32_t pc);
void jit_writepodarr(int32_t id, int32_t pc);

bool is_guarded_script_register(int reg);
void do_set(int reg, int value);

int32_t run_script(ScriptType type, word script, int32_t i = -1); //Global scripts don't need 'i'
int32_t ffscript_engine(const bool preload);

sprite* get_own_sprite(ScriptType type);
sprite* get_own_sprite(refInfo* ri, ScriptType type);

void deallocateArray(const int32_t ptrval);
void clearScriptHelperData();

int32_t get_screenflags(mapscr *m, int32_t flagset);
int32_t get_screeneflags(mapscr *m, int32_t flagset);
int32_t get_ref_map_index(int32_t ref);

ffcdata* ResolveFFCWithID(ffc_id_t id);
ffcdata *ResolveFFC(int32_t ffcref);
sprite* ResolveBaseSprite(int32_t uid);
item* ResolveItemSprite(int32_t uid);
enemy* ResolveNpc(int32_t uid);

// Defines for script flags
#define TRUEFLAG          0x0001
#define MOREFLAG          0x0002
#define TRIGGERED         0x0004

struct quad3Dstruct
{
    int32_t index;
    float pos[12], uv[8];
    int32_t size[2], color[4];
};

struct triangle3Dstruct
{
    int32_t index;
    float pos[9], uv[6];
    int32_t size[2], color[3];
};

// Inline: called per-instruction in several JIT compile passes, where the
// out-of-line call showed up in compile-time profiles.
inline bool command_is_wait(int command)
{
	switch (command)
	{
	case WAITFRAME:
	case WAITDRAW:
	case WAITTO:
	case WAITEVENT:
	case WAITFRAMESR:
		return true;
	}
	return false;
}
// Waits, plus RUNGENFRZSCR: commands that suspend the script and yield
// control back to the engine (RUNGENFRZSCR runs nested engine frames, which
// can't execute inside a jitted wasm frame - see jit_wasm.cpp). Correct resume
// depends on every consult site - CFG block splits, dispatch entries,
// yielding-function detection, region eligibility - agreeing on this set, so
// it lives here rather than being spelled out at each site.
inline bool command_is_suspend(int command)
{
	return command_is_wait(command) || command == RUNGENFRZSCR;
}
bool command_is_goto(int command);
bool command_uses_comparison_result(int command);
bool command_writes_comparison_result(int command);
int command_to_cmp(int command, int arg);
bool command_could_return_not_ok(int command);
// Returns true if the command has no side effects other than
// potentially writing some value to a register pointed at by
// on of its args.
// The optimizer uses this to know if it is safe to remove a
// command, given its register output is not needed.
bool command_is_pure(int command);

int32_t get_combopos_ref(const rpos_handle_t& rpos_handle);
int32_t get_combopos_ref(rpos_t rpos, int32_t layer);
rpos_t combopos_ref_to_rpos(int32_t combopos_ref);
int32_t combopos_ref_to_layer(int32_t combopos_ref);

bool is_valid_array(int32_t ptr);
uint32_t allocatemem(int32_t size, bool local, ScriptType type, const uint32_t UID, script_object_type object_type = script_object_type::none);

struct ScriptEngineData {
	refInfo ref;
	int32_t stack[MAX_STACK_SIZE];
	int32_t ret_stack[MAX_CALL_FRAMES];
	ScriptType script_type;
	int script_num;
	std::shared_ptr<JittedScriptInstance> j_instance;
	// This is used as a boolean for all but ScriptType::Item.
	byte doscript = true;
	bool waitdraw;
	bool initialized;

	void clear_ref()
	{
		ref.Clear();
		j_instance = {};
		initialized = false;
		script_num = 0;
	}

	void reset()
	{
		// No need to zero the stack.
		ref = refInfo();
		j_instance = {};
		doscript = true;
		waitdraw = false;
		initialized = false;
		script_num = 0;
	}
};

// (type, index) => ScriptEngineData
extern std::map<std::pair<ScriptType, int>, ScriptEngineData> scriptEngineDatas;

void on_reassign_script_engine_data(ScriptType type, int index);
ScriptEngineData& get_script_engine_data(ScriptType type, int index);
bool script_engine_data_exists(ScriptType type, int index);
ScriptEngineData& get_script_engine_data(ScriptType type);
ScriptEngineData& get_ffc_script_engine_data(int index);
ScriptEngineData& get_item_script_engine_data(int index);

extern FFScript FFCore;

// RAII guard for a script engine's blocking run loop: clears the slot's engine data
// (releasing anything it owned) when it leaves scope, so an early return can't skip
// the cleanup.
struct ScopedScriptEngineDataClear
{
	ScriptType type;
	int index;
	~ScopedScriptEngineDataClear() { FFCore.clear_script_engine_data(type, index); }
};

extern byte flagpos;
extern int32_t flagval;
void clear_ornextflag();
void ornextflag(bool flag);

void log_stack_overflow_error();
void log_call_limit_error();

int32_t get_mi(int32_t);
int32_t get_mi(mapdata const&);
int32_t get_total_mi();
int get_mouse_state(int index);

bool pc_overflow(dword pc, bool print_err = true);
bool check_stack(uint32_t sp);

// Uncomment locally to run the -debug-print-register-deps command
// #define DEBUG_REGISTER_DEPS

#ifdef DEBUG_REGISTER_DEPS

#define GET_D(r) debug_get_d(r)
#define SET_D(r, v) debug_set_d(r, v)
#define GET_REF(x) debug_get_ref(#x)

int debug_get_d(int r);
int debug_set_d(int r, int v);
int debug_get_ref(std::string reg_name);
int debug_get_ref(int r);
void print_d_register_deps();

#else

#define GET_D(r) ri->d[r]
#define SET_D(r, v) ri->d[r] = (v)
#define GET_REF(x) (ri->x+0)

#endif

#endif
