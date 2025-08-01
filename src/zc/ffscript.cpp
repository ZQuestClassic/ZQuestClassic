#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <sstream>
#include <math.h>
#include <cstdio>
#include <algorithm>
#include <ranges>
//
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <filesystem>
#include <fmt/format.h>
#include <fmt/ranges.h>
//

#include "base/check.h"
#include "base/expected.h"
#include "base/handles.h"
#include "base/general.h"
#include "base/mapscr.h"
#include "base/qrs.h"
#include "base/dmap.h"
#include "base/msgstr.h"
#include "base/packfile.h"
#include "base/misctypes.h"
#include "base/initdata.h"
#include "base/version.h"
#include "new_subscr.h"
#include "zc/maps.h"
#include "zasm/serialize.h"
#include "zasm/table.h"
#include "zc/replay.h"
#include "zc/scripting/arrays.h"
#include "zc/scripting/script_object.h"
#include "zc/scripting/types.h"
#include "zc/scripting/types/websocket.h"
#include "zc/zasm_optimize.h"
#include "zc/zasm_utils.h"
#include "zc/zc_ffc.h"
#include "zc/zc_sys.h"
#include "zc/jit.h"
#include "zc/script_debug.h"
#include "base/zc_alleg.h"
#include "base/zc_math.h"
#include "base/zc_array.h"
#include "zc/ffscript.h"
#include "zc/render.h"
#include "zc/zc_subscr.h"
#include <time.h>
#include "zc/script_drawing.h"
#include "base/util.h"
#include "zc/ending.h"
#include "zc/combos.h"
#include "drawing.h"
#include "base/colors.h"
#include "pal.h"
#include "zinfo.h"
#include "subscr.h"
#include "zc_list_data.h"
#include "music_playback.h"
#include "iter.h"
#include <sstream>

#include "zc/zelda.h"
#include "particles.h"
#include "zc/hero.h"
#include "zc/guys.h"
#include "gamedata.h"
#include "zc/zc_init.h"
#include "base/zsys.h"
#include "base/misctypes.h"
#include "zc/title.h"
#include "zscriptversion.h"

#include "pal.h"
#include "base/zdefs.h"
#include "zc/rendertarget.h"

#include "hero_tiles.h"
#include "base/qst.h"

using namespace util;

#ifdef _WIN32
#define SCRIPT_FILE_MODE	(_S_IREAD | _S_IWRITE)
#else
	#include <fcntl.h>
	#include <unistd.h>
	#include <iostream>
	#define SCRIPT_FILE_MODE	(S_ISVTX | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#endif

//Define this register, so it can be treated specially
#define NUL		5
#define MAX_ZC_ARRAY_SIZE 214748

using namespace util;
using std::ostringstream;

static ASM_DEFINE current_zasm_command;
static uint32_t current_zasm_register;
// If set, the next call to scripting_log_error_with_context will use this string in addition to whatever
// current_zasm_command and current_zasm_register refer to. Must unset after manually.
std::string current_zasm_extra_context;
// If set, the next call to scripting_log_error_with_context will use this string instead of whatever
// current_zasm_command and current_zasm_register refer to. Must unset after manually.
std::string current_zasm_context;

void scripting_log_error_with_context(std::string text)
{
	if (current_zasm_context.empty())
	{
		std::vector<const char*> context;

		const char* register_string = scripting_get_zasm_register_context_string(current_zasm_register);
		if (register_string)
			context.push_back(register_string);

		const char* command_string = scripting_get_zasm_command_context_string(current_zasm_command);
		if (command_string)
			context.push_back(command_string);

		if (!current_zasm_extra_context.empty())
			context.push_back(current_zasm_extra_context.c_str());

		if (context.size())
			current_zasm_context = fmt::format("{}", fmt::join(context, ", "));
		else
		{
			Z_scripterrlog("%s\n", text.c_str());
			return;
		}
	}

	Z_scripterrlog("%s | %s\n", current_zasm_context.c_str(), text.c_str());
	current_zasm_context = "";
	current_zasm_extra_context = "";
}

// (type, index) => ScriptEngineData
std::map<std::pair<ScriptType, int>, ScriptEngineData> scriptEngineDatas;

extern byte use_dwm_flush;
uint8_t using_SRAM = 0;

int32_t hangcount = 0;
bool can_neg_array = true;

extern byte monochrome_console;

static std::map<script_id, ScriptDebugHandle> script_debug_handles;
ScriptDebugHandle* runtime_script_debug_handle;
int32_t jitted_uncompiled_command_count;

CScriptDrawingCommands scriptdraws;
FFScript FFCore;

static expected<std::string, std::string> parse_user_path(const std::string& user_path, bool is_file);

static UserDataContainer<script_array, 1000000> script_arrays = {script_object_type::array, "array"};
static UserDataContainer<user_dir, MAX_USER_DIRS> user_dirs = {script_object_type::dir, "directory"};
static UserDataContainer<user_file, MAX_USER_FILES> user_files = {script_object_type::file, "file"};
static UserDataContainer<user_paldata, MAX_USER_PALDATAS> user_paldatas = {script_object_type::paldata, "paldata"};
static UserDataContainer<user_rng, MAX_USER_RNGS> user_rngs = {script_object_type::rng, "rng"};
static UserDataContainer<user_stack, MAX_USER_STACKS> user_stacks = {script_object_type::stack, "stack"};
static UserDataContainer<user_bitmap, MAX_USER_BITMAPS> user_bitmaps = {script_object_type::bitmap, "bitmap"};

script_array* create_script_array()
{
	return script_arrays.create();
}

void register_existing_script_array(script_array* array)
{
	script_arrays.register_existing(array);
}

std::vector<script_array*> get_script_arrays()
{
	std::vector<script_array*> result;
	for (auto id : script_object_ids_by_type[script_arrays.type])
	{
		result.push_back(&script_arrays[id]);
	}
	return result;
}

static script_array* find_or_create_internal_script_array(script_array::internal_array_id internal_id)
{
	if (!zasm_array_supports(internal_id.zasm_var))
	{
		scripting_log_error_with_context("Invalid internal array id: {}", internal_id.zasm_var);
		return nullptr;
	}

	for (auto id : script_object_ids_by_type[script_arrays.type])
	{
		auto object = static_cast<script_array*>(get_script_object_checked(id));
		if (!object->internal_expired && object->internal_id.has_value() && object->internal_id.value() == internal_id)
			return object;
	}

	auto array = script_arrays.create();
	if (array)
	{
		array->arr.setValid(true);
		array->internal_id = internal_id;
	}
	return array;
}

static void expire_internal_script_arrays(ScriptType scriptType, int ref)
{
	if (!ZScriptVersion::gc_arrays())
		return;

	// Expire internal arrays referring to this script object.
	for (auto& script_object : script_objects | std::views::values)
	{
		if (script_object->type != script_object_type::array)
			continue;

		auto array = static_cast<script_array*>(script_object.get());
		if (!array->internal_id.has_value())
			continue;

		if (array->internal_id->matches(scriptType, ref))
			array->internal_expired = true;
	}
}

static void expire_internal_script_arrays(ScriptType scriptType)
{
	if (!ZScriptVersion::gc_arrays())
		return;

	// Expire internal arrays referring to this script object.
	for (auto& script_object : script_objects | std::views::values)
	{
		if (script_object->type != script_object_type::array)
			continue;

		auto array = static_cast<script_array*>(script_object.get());
		if (!array->internal_id.has_value())
			continue;

		if (array->internal_id->matches(scriptType))
			array->internal_expired = true;
	}
}

script_array* checkArray(uint32_t id, bool skipError)
{
	return script_arrays.check(id, skipError);
}

void script_bitmaps::update()
{
	auto ids = script_object_ids_by_type[user_bitmaps.type];
	for (auto id : ids)
	{
		auto& bitmap = user_bitmaps[id];
		if (bitmap.is_freeing())
		{
			bitmap.mark_can_del();
			delete_script_object(id);
		}
	}
}

user_bitmap& script_bitmaps::get(int32_t id)
{
	static user_bitmap fake;

	current_zasm_context = "script drawing";
	if (auto bitmap = user_bitmaps.check(id))
	{
		current_zasm_context = "";
		return *bitmap;
	}

	return fake;
}

script_bitmaps scb;
user_rng nulrng;
zc_randgen script_rnggens[MAX_USER_RNGS];

FONT *get_zc_font(int index);

int32_t combopos_modified = -1;
static std::vector<word> combo_id_cache;

void user_dir::setPath(const char* buf)
{
	if(!list)
	{
		list = (FLIST *) calloc(1, sizeof(FLIST));
	}
	filepath = std::string(buf) + "/";
	regulate_path(filepath);
	list->load(filepath.c_str());
}

int32_t CScriptDrawingCommands::GetCount()
{
	al_trace("current number of draws is: %d\n", count);
	return count;
}

// Decodes a `mapref` (reference number) for a temporary screen.
//
// A mapref can refer to:
//
// - the canonical mapscr data, loaded via `Game->LoadMapData(int map, int screen)`
// - a temporary mapscr, loaded via `Game->LoadTempScreen(int layer, int? screen)`
// - a temporary mapscr, loaded via `Game->LoadScrollingScreen(int layer, int? screen)`
//
// The canonical maprefs are >=0, and temporary ones are all negative.
//
// If temporary, and loaded without specifiying a screen index, we allow combo array variables (like
// `ComboX[pos]`) to address any rpos in the region. Otherwise, only positions in the exact screen
// referenced by `mapref` can be used (0-175). See ResolveMapdataPos.
mapdata decode_mapdata_ref(int ref)
{
	if (ref >= 0)
	{
		if (ref >= TheMaps.size())
			return mapdata{};

		int screen = ref % MAPSCRS;
		return mapdata{mapdata_type::CanonicalScreen, &TheMaps[ref], screen, 0};
	}

	// Negative values are for temporary screens.

	ref = -(ref + 1);
	bool is_scrolling = ref & 1;
	bool is_region = ref & 2;
	int screen = (ref & 0x0000FF00) >> 8;
	int layer  = (ref & 0x00FF0000) >> 16;

	if (is_region)
	{
		if (is_scrolling)
			screen = scrolling_region.origin_screen;
		else
			screen = cur_screen;
	}

	mapscr* scr = nullptr;
	if (is_scrolling)
	{
		int index = screen * 7 + layer;
		if (index >= 0 && index < FFCore.ScrollingScreensAll.size())
			scr = FFCore.ScrollingScreensAll[index];
	}
	else
	{
		if (layer >= 0 && layer <= 6 && is_in_current_region(screen))
			scr = get_scr_layer(screen, layer);
	}

	if (!scr)
		return mapdata{};

	auto type = mapdata_type::None;
	if (is_region && is_scrolling)
		type = mapdata_type::TemporaryScrollingRegion;
	else if (is_region && !is_scrolling)
		type = mapdata_type::TemporaryCurrentRegion;
	else if (!is_region && is_scrolling)
		type = mapdata_type::TemporaryScrollingScreen;
	else if (!is_region && !is_scrolling)
		type = mapdata_type::TemporaryCurrentScreen;

	return mapdata{type, scr, screen, layer};
}

static int create_mapdata_temp_ref(mapdata_type type, int screen, int layer)
{
	bool is_scrolling = type == mapdata_type::TemporaryScrollingScreen || type == mapdata_type::TemporaryScrollingRegion;
	bool is_region = type == mapdata_type::TemporaryScrollingRegion || type == mapdata_type::TemporaryCurrentRegion;

	int ref = 0;
	ref |= is_scrolling ? 1 : 0;
	ref |= is_region ? 2 : 0;
	if (!is_region)
		ref |= ((screen & 0xFF) << 8);
	ref |= ((layer & 0xFF) << 16);
	return -ref-1;
}

mapscr* GetScrollingMapscr(int layer, int x, int y)
{
	if (!screenscrolling)
		return nullptr;

	int screen = scrolling_region.origin_screen + map_scr_xy_to_index(x / 256, y / 176);
	mapscr* m = FFCore.ScrollingScreensAll[screen * 7 + layer];
	if (!m || !m->is_valid())
		return nullptr;

	return m;
}

int32_t getMap(int32_t ref)
{
	switch(ref)
	{
		case MAPSCR_TEMP0:
			return cur_map+1;
		case MAPSCR_TEMP1:
			return origin_scr->layermap[0];
		case MAPSCR_TEMP2:
			return origin_scr->layermap[1];
		case MAPSCR_TEMP3:
			return origin_scr->layermap[2];
		case MAPSCR_TEMP4:
			return origin_scr->layermap[3];
		case MAPSCR_TEMP5:
			return origin_scr->layermap[4];
		case MAPSCR_TEMP6:
			return origin_scr->layermap[5];
		case MAPSCR_SCROLL0:
			return scrolling_map+1;
		case MAPSCR_SCROLL1:
			return special_warp_return_scr->layermap[0];
		case MAPSCR_SCROLL2:
			return special_warp_return_scr->layermap[1];
		case MAPSCR_SCROLL3:
			return special_warp_return_scr->layermap[2];
		case MAPSCR_SCROLL4:
			return special_warp_return_scr->layermap[3];
		case MAPSCR_SCROLL5:
			return special_warp_return_scr->layermap[4];
		case MAPSCR_SCROLL6:
			return special_warp_return_scr->layermap[5];
		default:
			return (ref / MAPSCRS + 1);
	}
}
int32_t getScreen(int32_t ref)
{
	switch(ref)
	{
		case MAPSCR_TEMP0:
			return cur_screen;
		case MAPSCR_TEMP1:
			return origin_scr->layerscreen[0];
		case MAPSCR_TEMP2:
			return origin_scr->layerscreen[1];
		case MAPSCR_TEMP3:
			return origin_scr->layerscreen[2];
		case MAPSCR_TEMP4:
			return origin_scr->layerscreen[3];
		case MAPSCR_TEMP5:
			return origin_scr->layerscreen[4];
		case MAPSCR_TEMP6:
			return origin_scr->layerscreen[5];
		case MAPSCR_SCROLL0:
			return scrolling_hero_screen;
		case MAPSCR_SCROLL1:
			return special_warp_return_scr->layerscreen[0];
		case MAPSCR_SCROLL2:
			return special_warp_return_scr->layerscreen[1];
		case MAPSCR_SCROLL3:
			return special_warp_return_scr->layerscreen[2];
		case MAPSCR_SCROLL4:
			return special_warp_return_scr->layerscreen[3];
		case MAPSCR_SCROLL5:
			return special_warp_return_scr->layerscreen[4];
		case MAPSCR_SCROLL6:
			return special_warp_return_scr->layerscreen[5];
		default:
			return (ref % MAPSCRS);
	}
}

static ffcdata* get_ffc(ffc_id_t ffc_id)
{
	return &get_scr_for_region_index_offset(ffc_id / MAXFFCS)->getFFC(ffc_id % MAXFFCS);
}

dword get_subref(int sub, byte ty, byte pg, word ind)
{
	byte s;
	if(sub == -1) //special; load current
	{
		if (new_sub_indexes[ty] < 0) return 0;
		s = new_sub_indexes[ty];
	}
	else if(unsigned(sub) < 256)
		s = sub;
	else return 0;
	++ty; //type is offset by 1
	return (s<<24)|(pg<<16)|((ty&0x7)<<13)|(ind&0x1FFF);
}
std::tuple<byte,int8_t,byte,word> from_subref(dword ref)
{
	byte type = (ref>>13)&0x07;
	if(!type)
		return { 0, -1, 0, 0 };
	
	byte sub = (ref>>24)&0xFF;
	byte pg = (ref>>16)&0xFF;
	word ind = (ref)&0x1FFF;
	return { sub, type-1, pg, ind };
}

std::tuple<ZCSubscreen*,SubscrPage*,SubscrWidget*,byte> load_subscreen_ref(dword ref)
{
	auto [sub,ty,pg,ind] = from_subref(ref);
	ZCSubscreen* sbscr = nullptr;
	SubscrPage* sbpg = nullptr;
	SubscrWidget* sbwidg = nullptr;
	switch(ty)
	{
		case sstACTIVE:
			if(sub < subscreens_active.size())
				sbscr = &subscreens_active[sub];
			break;
		case sstPASSIVE:
			if(sub < subscreens_passive.size())
				sbscr = &subscreens_passive[sub];
			break;
		case sstOVERLAY:
			if(sub < subscreens_overlay.size())
				sbscr = &subscreens_overlay[sub];
			break;
	}
	if(sbscr)
	{
		if(pg < sbscr->pages.size())
			sbpg = &sbscr->pages[pg];
	}
	else return { nullptr, nullptr, nullptr, -1 }; //no subscreen
	if(sbpg)
	{
		if(ind < sbpg->size())
			sbwidg = sbpg->at(ind);
	}
	return { sbscr, sbpg, sbwidg, ty };
}
std::pair<ZCSubscreen*,byte> load_subdata(dword ref)
{
	auto [sub,_pg,_widg,ty] = load_subscreen_ref(ref);
	return { sub, ty };
}
std::pair<SubscrPage*,byte> load_subpage(dword ref)
{
	auto [_sub,pg,_widg,ty] = load_subscreen_ref(ref);
	return { pg, ty };
}
std::pair<SubscrWidget*,byte> load_subwidg(dword ref)
{
	auto [_sub,_pg,widg,ty] = load_subscreen_ref(ref);
	return { widg, ty };
}

#include "zconsole/ConsoleLogger.h"

//no ifdef here
extern CConsoleLoggerEx zscript_coloured_console;

bool FFScript::isNumber(char chr)
{
	if ( chr >= '0' )
	{
		if ( chr <= '9' ) return true;
	}
	return false;
}

int32_t FFScript::ilen(char *p)
{
	int32_t ret = 0; int32_t pos = 0;
	if(p[pos] == '-')
		ret++;
	for(; FFCore.isNumber(p[pos + ret]); ++ret);
	return ret;
}

int32_t FFScript::atox(char *ip_str)
{
	char tmp[2]={'2','\0'};
	int32_t op_val=0, i=0, ip_len = strlen(ip_str);

	if(strncmp(ip_str, "0x", 2) == 0)
	{
		ip_str +=2;
		ip_len -=2;
	}

	for(i=0;i<ip_len;i++)
	{
		op_val *= 0x10;
		switch(ip_str[i])
		{
			case 'a':
			op_val += 0xa;
			break;
			case 'b':
			op_val += 0xb;
			break;
			case 'c':
			op_val += 0xc;
			break;
			case 'd':
			op_val += 0xd;
			break;
			case 'e':
			op_val += 0xe;
			break;
			case 'f':
			op_val += 0xf;
			break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			tmp[0] = ip_str[i];
			op_val += atoi(tmp);
			break;
			default :
			op_val += 0x0;
			break;
		}
	}
	return op_val;
}

char runningItemScripts[256] = {0};
 
//item *FFCore.temp_ff_item = NULL;
//enemy *FFCore.temp_ff_enemy = NULL;
//weapon *FFCore.temp_ff_lweapon = NULL;
//weapon *FFCore.temp_ff_eweapon = NULL;

extern int32_t directItemA;
extern int32_t directItemB;
extern int32_t directItemX;
extern int32_t directItemY;


#ifdef _MSC_VER
#pragma warning ( disable : 4800 ) //int32_t to bool town. population: lots.
#endif

//! New datatype vars for 2.54:

//spritedata sp->member


using std::string;

extern char *guy_string[];
extern int32_t skipcont;

PALETTE tempgreypal; //Palettes go here. This is used for Greyscale() / Monochrome()
PALETTE userPALETTE[256]; //Palettes go here. This is used for Greyscale() / Monochrome()
PALETTE tempblackpal; //Used for storing the palette while fading to black

byte FF_hero_action; //This way, we can make safe replicas of internal Hero actions to be set by script. 
	
int32_t FF_screenbounds[4]; //edges of the screen, left, right, top, bottom used for where to scroll. 
int32_t FF_screen_dimensions[4]; //height, width, displaywidth, displayheight
int32_t FF_subscreen_dimensions[4];
int32_t FF_eweapon_removal_bounds[4]; //left, right, top, bottom coordinates for automatic eweapon removal. 
int32_t FF_lweapon_removal_bounds[4]; //left, right, top, bottom coordinates for automatic lweapon removal. 
int32_t FF_clocks[FFSCRIPTCLASS_CLOCKS]; //Will be used for Heroaction, anims, and so forth 
byte ScriptDrawingRules[SCRIPT_DRAWING_RULES];
int32_t FF_UserMidis[NUM_USER_MIDI_OVERRIDES]; //MIDIs to use for Game Over, and similar to override system defaults. 

int32_t legacy_get_int_arr(const int32_t ptr, int32_t indx);
void legacy_set_int_arr(const int32_t ptr, int32_t indx, int32_t val);
int32_t legacy_sz_int_arr(const int32_t ptr);

//We gain some speed by not passing as arguments
int32_t sarg1;
int32_t sarg2;
int32_t sarg3;
vector<int32_t> *sargvec;
string *sargstr;
refInfo *ri;
script_data *curscript;
int32_t(*stack)[MAX_STACK_SIZE];
bounded_vec<word, int32_t>* ret_stack;
vector<int32_t> zs_vargs;
ScriptType curScriptType;
word curScriptNum;
int32_t curScriptIndex;
bool script_funcrun;
string* destructstr;
size_t gen_frozen_index;

static vector<ScriptType> curScriptType_cache;
static vector<int32_t> curScriptNum_cache;
static vector<int32_t> curScriptIndex_cache;
static vector<int32_t> sarg1cache;
static vector<int32_t> sarg2cache;
static vector<int32_t> sarg3cache;
static vector<vector<int32_t>*> sargvec_cache;
static vector<string*> sargstr_cache;
static vector<refInfo*> ricache;
static vector<script_data*> sdcache;
static vector<int32_t(*)[MAX_STACK_SIZE]> stackcache;
static vector<bounded_vec<word, int32_t>*> ret_stack_cache;
void push_ri()
{
	sarg1cache.push_back(sarg1);
	sarg2cache.push_back(sarg2);
	sarg3cache.push_back(sarg3);
	curScriptType_cache.push_back(curScriptType);
	curScriptNum_cache.push_back(curScriptNum);
	curScriptIndex_cache.push_back(curScriptIndex);
	sargvec_cache.push_back(sargvec);
	sargstr_cache.push_back(sargstr);
	ricache.push_back(ri);
	sdcache.push_back(curscript);
	stackcache.push_back(stack);
	ret_stack_cache.push_back(ret_stack);
}
void pop_ri()
{
	sarg1 = sarg1cache.back(); sarg1cache.pop_back();
	sarg2 = sarg2cache.back(); sarg2cache.pop_back();
	sarg3 = sarg3cache.back(); sarg3cache.pop_back();
	curScriptType = curScriptType_cache.back(); curScriptType_cache.pop_back();
	curScriptNum = curScriptNum_cache.back(); curScriptNum_cache.pop_back();
	curScriptIndex = curScriptIndex_cache.back(); curScriptIndex_cache.pop_back();
	sargvec = sargvec_cache.back(); sargvec_cache.pop_back();
	sargstr = sargstr_cache.back(); sargstr_cache.pop_back();
	ri = ricache.back(); ricache.pop_back();
	curscript = sdcache.back(); sdcache.pop_back();
	stack = stackcache.back(); stackcache.pop_back();
	ret_stack = ret_stack_cache.back(); ret_stack_cache.pop_back();
}

//START HELPER FUNCTIONS
///-------------------------------------//
//           Helper Functions           //
///-------------------------------------//

static void log_stack_overflow_error()
{
	scripting_log_error_with_context("Stack overflow!");
}

static void log_call_limit_error()
{
	scripting_log_error_with_context("Function call limit reached! Too much recursion. Max nested function calls is {}", MAX_CALL_FRAMES);
}

void SH::write_stack(const uint32_t sp, const int32_t value)
{
	if (sp >= MAX_STACK_SIZE)
	{
		log_stack_overflow_error();
		ri->overflow = true;
		return;
	}
	
	(*stack)[sp] = value;
}

int32_t SH::read_stack(const uint32_t sp)
{
	if (sp >= MAX_STACK_SIZE)
	{
		log_stack_overflow_error();
		ri->overflow = true;
		return -10000;
	}
	
	return (*stack)[sp];
}

///----------------------------//
//           Misc.             //
///----------------------------//

byte flagpos;
int32_t flagval;
void clear_ornextflag()
{
	flagpos = 0;
	flagval = 0;
}
void ornextflag(bool flag)
{
	if(flag) flagval |= 1<<flagpos;
	++flagpos;
}

int32_t get_screenflags(mapscr *m, int32_t flagset)
{
	clear_ornextflag();
	
	switch(flagset)
	{
		case 0: // Room Type
			ornextflag(m->flags6&1);
			ornextflag(m->flags6&2);
			ornextflag(m->flags7&8);
			break;
			
		case 1: // View
			ornextflag(m->flags3&8);
			ornextflag(m->flags7&16);
			ornextflag(m->flags3&16);
			ornextflag(m->flags3&64);
			ornextflag(m->flags7&2);
			ornextflag(m->flags7&1);
			ornextflag(m->flags&fDARK);
			ornextflag(m->flags9&fDARK_DITHER);
			ornextflag(m->flags9&fDARK_TRANS);
			break;
			
		case 2: // Secrets
			ornextflag(m->flags&1);
			ornextflag(m->flags5&16);
			ornextflag(m->flags6&4);
			ornextflag(m->flags6&32);
			break;
			
		case 3: // Warp
			ornextflag(m->flags5&4);
			ornextflag(m->flags5&8);
			ornextflag(m->flags&64);
			ornextflag(m->flags8&64);
			ornextflag(m->flags3&32);
			ornextflag(m->flags9&fDISABLE_MIRROR);
			ornextflag(m->flags10&fMAZE_CAN_GET_LOST);
			ornextflag(m->flags10&fMAZE_LOOPY);
			break;
			
		case 4: // Item
			ornextflag(m->flags3&1);
			ornextflag(m->flags7&4);
			ornextflag(m->flags8&0x40);
			ornextflag(m->flags8&0x80);
			ornextflag(m->flags9&0x01);
			ornextflag(m->flags9&0x02);
			ornextflag(m->flags9&fBELOWRETURN);
			break;
			
		case 5: // Combo
			ornextflag((m->flags2>>4)&2);
			ornextflag(m->flags3&2);
			ornextflag(m->flags5&2);
			ornextflag(m->flags6&64);
			break;
			
		case 6: // Save
			ornextflag(m->flags4&64);
			ornextflag(m->flags4&128);
			ornextflag(m->flags6&8);
			ornextflag(m->flags6&16);
			break;
			
		case 7: // FFC
			ornextflag(m->flags6&128);
			ornextflag(m->flags5&128);
			break;
			
		case 8: // Whistle
			ornextflag(m->flags&16);
			ornextflag(m->flags7&64);
			ornextflag(m->flags7&128);
			break;
			
		case 9: // Misc
			ornextflag(m->flags&32);
			ornextflag(m->flags5&64);
			flagval |= m->flags8<<2;
			break;
	}
	
	return flagval;
}

int32_t get_screeneflags(mapscr *m, int32_t flagset)
{
	clear_ornextflag();
	
	switch(flagset)
	{
		case 0:
			flagval |= m->flags11&0x1F;
			break;
			
		case 1:
			ornextflag(m->flags11&32);
			ornextflag(m->flags11&64);
			ornextflag(m->flags3&4);
			ornextflag(m->flags11&128);
			ornextflag((m->flags2>>4)&4);
			break;
			
		case 2:
			ornextflag(m->flags3&128);
			ornextflag(m->flags&2);
			ornextflag((m->flags2>>4)&8);
			ornextflag(m->flags4&16);
			ornextflag(m->flags9&fENEMY_WAVES);
			break;
	}
	
	return flagval;
}

int32_t get_mi(int32_t ref)
{
	auto result = decode_mapdata_ref(ref);
	if (result.canonical())
	{
		if (result.screen >= MAPSCRSNORMAL) return -1;
		return mapind(result.scr->map, result.screen);
	}
	else if (result.current())
	{
		if (result.screen >= MAPSCRSNORMAL) return -1;
		return mapind(cur_map, result.screen);
	}
	else if (result.scrolling())
	{
		if (result.screen >= MAPSCRSNORMAL) return -1;
		return mapind(scrolling_map, result.screen);
	}

	return -1;
}

int32_t get_ref_map_index(int32_t ref)
{
	if (ref >= 0)
		return ref;

	auto result = decode_mapdata_ref(ref);
	if (result.current())
	{
		return map_screen_index(cur_map, result.screen);
	}
	else if (result.scrolling())
	{
		return map_screen_index(scrolling_map, result.screen);
	}

	return -1;
}

template <typename T>
static T* ResolveSprite(int32_t uid, const char* name)
{
	if (!uid)
	{
		scripting_log_error_with_context("Invalid sprite: null pointer");
		return nullptr;
	}

	if (auto s = sprite::getByUID(uid))
	{
		if (auto s2 = dynamic_cast<T*>(s))
			return s2;

		scripting_log_error_with_context("Invalid sprite using UID = {} - but that sprite is not a {}", uid, name);
		return nullptr;
	}

	scripting_log_error_with_context("Invalid sprite using UID = {} - but that sprite does not exist", uid);
	return nullptr;
}

sprite* ResolveBaseSprite(int32_t uid)
{
	return ResolveSprite<sprite>(uid, "sprite");
}

item* ResolveItemSprite(int32_t uid)
{
	return ResolveSprite<item>(uid, "item");
}

enemy* ResolveNpc(int32_t uid)
{
	return ResolveSprite<enemy>(uid, "npc");
}

static weapon* ResolveEWeapon_checkSpriteList(int32_t uid)
{
	// Check here first (for the error logging.)
	const char* name = "eweapon";
	auto spr = ResolveSprite<weapon>(uid, name);

	// Double check this is from the right sprite list.
	if (spr && !Ewpns.getByUID(uid))
	{
		scripting_log_error_with_context("Invalid sprite using UID = {} - but that sprite is not a {}", uid, name);
		return nullptr;
	}

	return spr;
}

static weapon* ResolveLWeapon_checkSpriteList(int32_t uid)
{
	// Check here first (for the error logging.)
	const char* name = "lweapon";
	auto spr = ResolveSprite<weapon>(uid, name);

	// Double check this is from the right sprite list.
	if (spr && !Lwpns.getByUID(uid))
	{
		scripting_log_error_with_context("Invalid sprite using UID = {} - but that sprite is not a {}", uid, name);
		return nullptr;
	}

	return spr;
}

// For compat, get the first `combo_trigger` of the current `ri->combosref`
combo_trigger* get_first_combo_trigger()
{
	if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
		return nullptr;
	if(combobuf[ri->combosref].triggers.empty())
		return &(combobuf[ri->combosref].triggers.emplace_back());
	return &(combobuf[ri->combosref].triggers[0]);
}
// Get the combo trigger pointed to by `ref` (usually ri->combotrigref)
combo_trigger* get_combo_trigger(dword ref)
{
	byte idx = (ref >> 24) & 0xFF;
	dword cid = ref & 0x00FFFFFF;
	if(cid >= MAXCOMBOS)
	{
		scripting_log_error_with_context("Invalid combotrigger ID: {}-{}", idx, cid);
		return nullptr;
	}
	newcombo& cmb = combobuf[cid];
	if(idx >= cmb.triggers.size())
	{
		scripting_log_error_with_context("Invalid combotrigger ID: {}-{}", idx, cid);
		return nullptr;
	}
	return &(cmb.triggers[idx]);
}
// Get the combo ID of the trigger pointed to by `ref` (usually ri->combotrigref)
dword get_combo_from_trigger_ref(dword ref)
{
	dword cid = ref & 0x00FFFFFF;
	DCHECK(cid < MAXCOMBOS);
	return cid;
}

///------------------------------------------------//
//           Pointer Handling Functions          //
///------------------------------------------------//

//LWeapon Helper
class LwpnH : public SH
{

public:
	

	static defWpnSprite getDefWeaponSprite(weapon *wp)
	{
		switch(wp->id)
		{
			case wNone: return ws_0;
			case wSword: return ws_0;
			case wBeam: return wsBeam;
			case wBrang : return wsBrang;
			case wBomb: return wsBomb;
			case wSBomb: return wsSBomb;
			case wLitBomb: return wsBombblast;
			case wLitSBomb: return wsBombblast;
			case wArrow: return wsArrow;
			case wRefArrow: return wsArrow;
			case wFire: return wsFire;
			case wRefFire: return wsFire;
			case wRefFire2: return wsFire;
			case wWhistle: return wsUnused45;
			case wBait: return wsBait;
			case wWand: return wsWandHandle;
			case wMagic: return wsMagic;
			case wCatching: return wsUnused45;
			case wWind: return wsWind;
			case wRefMagic: return wsRefMagic;
			case wRefFireball: return wsRefFireball;
			case wRefRock: return wsRock;
			case wHammer: return wsHammer;
			case wHookshot: return wsHookshotHead;
			case wHSHandle: return wsHookshotHandle;
			case wHSChain: return wsHookshotChainH;
			case wSSparkle: return wsSilverSparkle;
			case wFSparkle: return wsGoldSparkle;
			case wSmack: return wsHammerSmack;
			case wPhantom: return wsUnused45;
			case wCByrna: return wsByrnaCane;
			case wRefBeam: return wsRefBeam;
			case wStomp: return wsUnused45;
			case lwMax: return wsUnused45;
			case wScript1: 
			case wScript2:
			case wScript3:
			case wScript4:
			case wScript5:
			case wScript6:
			case wScript7:
			case wScript8:
			case wScript9:
			case wScript10: return ws_0;
			case wIce: return wsIce; //new
			case wFlame: return wsEFire2; //new
			//not implemented; t/b/a
			case wSound:
			case wThrown: 
			case wPot:
			case wLit:
			case wBombos:
			case wEther:
			case wQuake:
			case wSword180:
			case wSwordLA:  return wsUnused45;
		
			case ewFireball: return wsFireball2;
			case ewArrow: return wsEArrow;
			case ewBrang: return wsBrang;
			case ewSword: return wsEBeam;
			case ewRock: return wsRock;
			case ewMagic: return wsEMagic;
			case ewBomb: return wsEBomb;
			case ewSBomb: return wsESbomb;
			case ewLitBomb: return wsEBombblast;
			case ewLitSBomb: return wsESbombblast;
			case ewFireTrail: return wsEFiretrail;
			case ewFlame: return wsEFire;
			case ewWind: return wsEWind;
			case ewFlame2: return wsEFire2;
			case ewFlame2Trail: return wsEFiretrail2;
			case ewIce: return wsIce;
			case ewFireball2: return wsFireball2;
			default: return wsUnused45;
		}
	};
	
	static int32_t loadWeapon(const int32_t uid)
	{
		tempweapon = ResolveSprite<weapon>(uid, "lweapon");
		if (!tempweapon)
			return _InvalidSpriteUID;
		
		return _NoError;
	}
	
	static INLINE weapon *getWeapon()
	{
		return tempweapon;
	}
	
	static INLINE void clearTemp()
	{
		tempweapon = NULL;
	}
	
private:

	static weapon *tempweapon;
};

weapon *LwpnH::tempweapon = NULL;

//EWeapon Helper
class EwpnH : public SH
{

public:
	
	defWpnSprite getDefWeaponSprite(weapon *wp)
	{
		switch(wp->id)
		{
			case wNone: return ws_0;
			case wSword: return ws_0;
			case wBeam: return wsBeam;
			case wBrang : return wsBrang;
			case wBomb: return wsBomb;
			case wSBomb: return wsSBomb;
			case wLitBomb: return wsBombblast;
			case wLitSBomb: return wsBombblast;
			case wArrow: return wsArrow;
			case wRefArrow: return wsArrow;
			case wFire: return wsFire;
			case wRefFire: return wsFire;
			case wRefFire2: return wsFire;
			case wWhistle: return wsUnused45;
			case wBait: return wsBait;
			case wWand: return wsWandHandle;
			case wMagic: return wsMagic;
			case wCatching: return wsUnused45;
			case wWind: return wsWind;
			case wRefMagic: return wsRefMagic;
			case wRefFireball: return wsRefFireball;
			case wRefRock: return wsRock;
			case wHammer: return wsHammer;
			case wHookshot: return wsHookshotHead;
			case wHSHandle: return wsHookshotHandle;
			case wHSChain: return wsHookshotChainH;
			case wSSparkle: return wsSilverSparkle;
			case wFSparkle: return wsGoldSparkle;
			case wSmack: return wsHammerSmack;
			case wPhantom: return wsUnused45;
			case wCByrna: return wsByrnaCane;
			case wRefBeam: return wsRefBeam;
			case wStomp: return wsUnused45;
			case lwMax: return wsUnused45;
			case wScript1: 
			case wScript2:
			case wScript3:
			case wScript4:
			case wScript5:
			case wScript6:
			case wScript7:
			case wScript8:
			case wScript9:
			case wScript10: return ws_0;
			case wIce: return wsIce; //new
			case wFlame: return wsEFire2; //new
			//not implemented; t/b/a
			case wSound:
			case wThrown: 
			case wPot:
			case wLit:
			case wBombos:
			case wEther:
			case wQuake:
			case wSword180:
			case wSwordLA:  return wsUnused45;
		
			case ewFireball: return wsFireball2;
			case ewArrow: return wsEArrow;
			case ewBrang: return wsBrang;
			case ewSword: return wsEBeam;
			case ewRock: return wsRock;
			case ewMagic: return wsEMagic;
			case ewBomb: return wsEBomb;
			case ewSBomb: return wsESbomb;
			case ewLitBomb: return wsEBombblast;
			case ewLitSBomb: return wsESbombblast;
			case ewFireTrail: return wsEFiretrail;
			case ewFlame: return wsEFire;
			case ewWind: return wsEWind;
			case ewFlame2: return wsEFire2;
			case ewFlame2Trail: return wsEFiretrail2;
			case ewIce: return wsIce;
			case ewFireball2: return wsFireball2;
			default: return wsUnused45;
		}
	};

	static int32_t loadWeapon(const int32_t uid)
	{
		tempweapon = ResolveSprite<weapon>(uid, "eweapon");
		if (!tempweapon)
			return _InvalidSpriteUID;
		
		return _NoError;
	}
	
	static INLINE weapon *getWeapon()
	{
		return tempweapon;
	}
	
	static INLINE void clearTemp()
	{
		tempweapon = NULL;
	}
	
private:

	static weapon *tempweapon;
};

weapon *EwpnH::tempweapon = NULL;

void clearScriptHelperData()
{
	GuyH::clearTemp();
	LwpnH::clearTemp();
	EwpnH::clearTemp();
	ItemH::clearTemp();
}
////END HELPER FUNCTIONS

static int32_t numInstructions = 0; // Used to detect hangs
static bool scriptCanSave = true;

static ScriptEngineData& get_script_engine_data(ScriptType type, int index)
{
	if (type == ScriptType::DMap || type == ScriptType::OnMap || type == ScriptType::ScriptedPassiveSubscreen || type == ScriptType::ScriptedActiveSubscreen)
	{
		// `index` is used for dmapref, not for different script engine data.
		index = 0;
	}
	if (type == ScriptType::EngineSubscreen)
	{
		// `index` is used for subdataref, not for different script engine data.
		index = 0;
	}
	
	return scriptEngineDatas[{type, index}];
}

static bool script_engine_data_exists(ScriptType type, int index)
{
	if (type == ScriptType::DMap || type == ScriptType::OnMap || type == ScriptType::ScriptedPassiveSubscreen || type == ScriptType::ScriptedActiveSubscreen)
	{
		// `index` is used for dmapref, not for different script engine data.
		index = 0;
	}
	if (type == ScriptType::EngineSubscreen)
	{
		// `index` is used for subdataref, not for different script engine data.
		index = 0;
	}

	return scriptEngineDatas.contains({type, index});
}

static ScriptEngineData& get_script_engine_data(ScriptType type)
{
	return get_script_engine_data(type, 0);
}

void FFScript::clear_script_engine_data()
{
	scriptEngineDatas.clear();
}

void FFScript::reset_script_engine_data(ScriptType type, int index)
{
	get_script_engine_data(type, index).reset();
}

void on_reassign_script_engine_data(ScriptType type, int index)
{
	auto& data = get_script_engine_data(type, index);
	data.clear_ref();
	FFScript::deallocateAllScriptOwned(type, index);
}

void FFScript::clear_script_engine_data(ScriptType type, int index)
{
	if (type == ScriptType::DMap || type == ScriptType::OnMap || type == ScriptType::ScriptedPassiveSubscreen || type == ScriptType::ScriptedActiveSubscreen)
	{
		// `index` is used for dmapref, not for different script engine data.
		index = 0;
	}
	if (type == ScriptType::EngineSubscreen)
	{
		// `index` is used for subdataref, not for different script engine data.
		index = 0;
	}

	auto it = scriptEngineDatas.find({type, index});
	if (it != scriptEngineDatas.end())
	{
		scriptEngineDatas.erase(it);
	}
}

void FFScript::clear_script_engine_data_of_type(ScriptType type)
{
	std::erase_if(scriptEngineDatas, [&](auto& kv) { return kv.first.first == type; });
}

refInfo& FFScript::ref(ScriptType type, int index)
{
	return get_script_engine_data(type, index).ref;
}

void FFScript::clear_ref(ScriptType type, int index)
{
	get_script_engine_data(type, index).clear_ref();
}

byte& FFScript::doscript(ScriptType type, int index)
{
	if (type == ScriptType::Generic && unsigned(index) < NUMSCRIPTSGENERIC)
		return user_genscript::get(index).doscript();
	return get_script_engine_data(type, index).doscript;
}

bool& FFScript::waitdraw(ScriptType type, int index)
{
	return get_script_engine_data(type, index).waitdraw;
}

static bool set_current_script_engine_data(ScriptEngineData& data, ScriptType type, int script, int index)
{
	bool got_initialized = false;

	ri = &data.ref;
	stack = &data.stack;
	ret_stack = &data.ret_stack;

	// By default, make `Screen->` refer to the top-left screen.
	// Will be set to something more specific for relevant script types.
	ri->screenref = cur_screen;

	switch (type)
	{
		case ScriptType::FFC:
		{
			curscript = ffscripts[script];
			ffcdata* ffc = get_ffc(index);

			if (!data.initialized)
			{
				got_initialized = true;
				mapscr* scr = get_scr(ffc->screen_spawned);
				memcpy(ri->d, scr->ffcs[index % 128].initd, 8 * sizeof(int32_t));
				data.initialized = true;
			}

			ri->ffcref = ZScriptVersion::ffcRefIsSpriteId() ? ffc->getUID() : index;
			ri->screenref = ffc->screen_spawned;
		}
		break;
		
		case ScriptType::NPC:
		{
			enemy *spr = (enemy*)guys.getByUID(index);
			curscript = guyscripts[script];
			
			if (!data.initialized)
			{
				got_initialized = true;
				memcpy(ri->d, spr->initD, 8 * sizeof(int32_t));
				data.initialized = 1;
			}
			
			ri->guyref = index;
			ri->screenref = spr->screen_spawned;
		}
		break;
		
		case ScriptType::Lwpn:
		{
			weapon *spr = (weapon*)Lwpns.getByUID(index);
			curscript = lwpnscripts[script];
			
			if (!data.initialized)
			{
				got_initialized = true;
				memcpy(ri->d, spr->initD, 8 * sizeof(int32_t));
				data.initialized = 1;
			}
			
			ri->lwpn = index;
			ri->screenref = spr->screen_spawned;
		}
		break;
		
		case ScriptType::Ewpn:
		{
			weapon *spr = (weapon*)Ewpns.getByUID(index);
			curscript = ewpnscripts[script];
			
			if (!data.initialized)
			{
				got_initialized = true;
				memcpy(ri->d, spr->initD, 8 * sizeof(int32_t));
				data.initialized = 1;
			}
			
			ri->ewpn = index;
			ri->screenref = spr->screen_spawned;
		}
		break;
		
		case ScriptType::ItemSprite:
		{
			item *spr = (item*)items.getByUID(index);
			curscript = itemspritescripts[script];
			
			if (!data.initialized)
			{
				got_initialized = true;
				memcpy(ri->d, spr->initD, 8 * sizeof(int32_t));
				data.initialized = 1;
			}
			
			ri->itemref = index;
			ri->screenref = spr->screen_spawned;
		}
		break;
		
		case ScriptType::Item:
		{
			int32_t i = index;
			int32_t new_i = 0;
			bool collect = ( ( i < 1 ) || (i == COLLECT_SCRIPT_ITEM_ZERO) );
			new_i = ( collect ) ? (( i != COLLECT_SCRIPT_ITEM_ZERO ) ? (i * -1) : 0) : i;

			curscript = itemscripts[script];
			
			if (!data.initialized)
			{
				got_initialized = true;
				memcpy(ri->d, ( collect ) ? itemsbuf[new_i].initiald : itemsbuf[i].initiald, 8 * sizeof(int32_t));
				data.initialized = true;
			}			
			ri->idata = ( collect ) ? new_i : i; //'this' pointer
		}
		break;
		
		case ScriptType::Global:
		{
			curscript = globalscripts[script];
			if (!data.initialized)
			{
				got_initialized = true;
				data.initialized = 1;

				// If this compat QR is on, scripts can run before ~Init and set global variables.
				// Before overwriting them with 0, get rid of object references held by global variables.
				if (get_qr(qr_OLD_INIT_SCRIPT_TIMING) && ZScriptVersion::gc() && script == GLOBAL_SCRIPT_INIT)
				{
					for (int i = 0; i < MAX_SCRIPT_REGISTERS; i++)
						script_object_ref_dec(game->global_d[i]);
				}
			}
		}
		break;
		
		case ScriptType::Generic:
		{
			user_genscript& scr = user_genscript::get(script);
			curscript = genericscripts[script];
			scr.waitevent = false;
			if(!data.initialized)
			{
				got_initialized = true;
				scr.initd.copy_to(ri->d, 8);
				data.initialized = true;
			}
			ri->genericdataref = script;
		}
		break;
		
		case ScriptType::GenericFrozen:
		{
			user_genscript& scr = user_genscript::get(script);
			curscript = genericscripts[script];
			if(!data.initialized)
			{
				got_initialized = true;
				scr.initd.copy_to(ri->d, 8);
				data.initialized = true;
			}
			ri->genericdataref = script;
		}
		break;
		
		case ScriptType::Hero:
		{
			curscript = playerscripts[script];
			ri->screenref = hero_screen;
			if (!data.initialized)
			{
				got_initialized = true;
				data.initialized = 1;
			}
		}
		break;
		
		case ScriptType::DMap:
		{
			curscript = dmapscripts[script];
			ri->dmapsref = index;
			//how do we clear initialised on dmap change?
			if ( !data.initialized )
			{
				got_initialized = true;
				for ( int32_t q = 0; q < 8; q++ ) 
				{
					ri->d[q] = DMaps[ri->dmapsref].initD[q];// * 10000;
				}
				data.initialized = true;
			}
		}
		break;
		
		case ScriptType::OnMap:
		{
			curscript = dmapscripts[script];
			ri->dmapsref = index;
			if (!data.initialized)
			{
				got_initialized = true;
				for ( int32_t q = 0; q < 8; q++ ) 
				{
					ri->d[q] = DMaps[ri->dmapsref].onmap_initD[q];
				}
				data.initialized = true;
			}
		}
		break;
		
		case ScriptType::ScriptedActiveSubscreen:
		{
			curscript = dmapscripts[script];
			ri->dmapsref = index;
			if (!data.initialized)
			{
				got_initialized = true;
				for ( int32_t q = 0; q < 8; q++ ) 
				{
					ri->d[q] = DMaps[ri->dmapsref].sub_initD[q];
				}
				data.initialized = true;
			}
		}
		break;
		
		case ScriptType::ScriptedPassiveSubscreen:
		{
			curscript = dmapscripts[script];
			ri->dmapsref = index;
			if (!data.initialized)
			{
				got_initialized = true;
				for ( int32_t q = 0; q < 8; q++ ) 
				{
					ri->d[q] = DMaps[ri->dmapsref].sub_initD[q];
				}
				data.initialized = true;
			}
		}
		break;
		case ScriptType::EngineSubscreen:
		{
			curscript = subscreenscripts[script];
			ri->subdataref = get_subref(-1, sstACTIVE);
			auto [ptr,_ty] = load_subdata(ri->subdataref);
			
			if (ptr && !data.initialized)
			{
				got_initialized = true;
				for ( int32_t q = 0; q < 8; q++ ) 
				{
					ri->d[q] = ptr->initd[q];
				}
				data.initialized = true;
			}
		}
		break;
		
		case ScriptType::Screen:
		{
			curscript = screenscripts[script];

			if (!data.initialized)
			{
				got_initialized = true;
				mapscr* scr = get_scr(index);
				for ( int32_t q = 0; q < 8; q++ ) 
				{
					ri->d[q] = scr->screeninitd[q];// * 10000;
				}
				data.initialized = true;
			}

			ri->screenref = index;
		}
		break;
		
		case ScriptType::Combo:
		{
			curscript = comboscripts[script];

			rpos_t rpos = combopos_ref_to_rpos(index);
			int32_t lyr = combopos_ref_to_layer(index);
			auto rpos_handle = get_rpos_handle(rpos, lyr);
			int32_t id = rpos_handle.data();
			if (!data.initialized)
			{
				got_initialized = true;
				memset(ri->d, 0, 8 * sizeof(int32_t));
				for ( int32_t q = 0; q < 8; q++ )
					ri->d[q] = combobuf[id].initd[q];
				data.initialized = true;
			}

			ri->combosref = id; //'this' pointer
			ri->comboposref = index; //used for X(), Y(), Layer(), and so forth.
			ri->screenref = rpos_handle.screen;
			break;
		}
	}
	
	if (got_initialized)
		ri->pc = curscript->pc;

	return got_initialized;
}

static ffcdata *ResolveFFCWithID(ffc_id_t id)
{
	if (BC::checkFFC(id) != SH::_NoError)
		return nullptr;

	ffcdata* ffc = get_ffc(id);
	if (!ffc)
		scripting_log_error_with_context("Invalid ffc using ID = {}", id);

	return ffc;
}

static ffcdata *ResolveFFC(int32_t ffcref)
{
	if (ZScriptVersion::ffcRefIsSpriteId())
		return ResolveSprite<ffcdata>(ffcref, "ffc");

	return ResolveFFCWithID(ffcref);
}

static mapscr* ResolveMapdataScr(int32_t mapref)
{
	auto mapdata = decode_mapdata_ref(mapref);
	if (!mapdata.scr)
		scripting_log_error_with_context("mapdata id is invalid: {}", mapref);
	return mapdata.scr;
}

static rpos_handle_t ResolveMapdataPos(int32_t mapref, int pos)
{
	auto mapdata = decode_mapdata_ref(mapref);
	if (!mapdata.scr)
	{
		scripting_log_error_with_context("mapdata id is invalid: {}", mapref);
		return rpos_handle_t{};
	}

	return mapdata.resolve_pos(pos);
}

int mapdata::max_pos()
{
	if (type == mapdata_type::TemporaryCurrentRegion)
		return (int)region_max_rpos;

	if (type == mapdata_type::TemporaryScrollingRegion)
		return (int)scrolling_region.screen_count * 176 - 1;

	return 175;
}

rpos_handle_t mapdata::resolve_pos(int pos)
{
	if (!screenscrolling && scrolling())
	{
		int32_t mapref = create_mapdata_temp_ref(type, screen, layer);
		scripting_log_error_with_context("mapdata id is invalid: {} - screen is not scrolling right now", mapref);
		return rpos_handle_t{};
	}

	// mapdata loaded via `Game->LoadTempScreen(layer)` have access to the entire region.
	if (type == mapdata_type::TemporaryCurrentRegion)
	{
		rpos_t rpos = (rpos_t)pos;
		if (BC::checkComboRpos(rpos) != SH::_NoError)
			return rpos_handle_t{};

		return get_rpos_handle(rpos, layer);
	}

	// mapdata loaded via `Game->LoadScrollingScreen(layer)` have access to the entire scrolling region.
	if (type == mapdata_type::TemporaryScrollingRegion)
	{
		rpos_t rpos = (rpos_t)pos;
		rpos_t max = (rpos_t)(scrolling_region.screen_count * 176 - 1);
		if (BC::checkBoundsRpos(rpos, (rpos_t)0, max) != SH::_NoError)
			return rpos_handle_t{};

		int origin_screen = scrolling_region.origin_screen;
		int origin_screen_x = origin_screen % 16;
		int origin_screen_y = origin_screen / 16;
		int scr_index = static_cast<int32_t>(rpos) / 176;
		int scr_x = origin_screen_x + scr_index%cur_region.screen_width;
		int scr_y = origin_screen_y + scr_index/cur_region.screen_width;
		int screen = map_scr_xy_to_index(scr_x, scr_y);
		mapscr* scr = FFCore.ScrollingScreensAll[screen * 7 + layer];

		return {scr, screen, layer, rpos, RPOS_TO_POS(rpos)};
	}

	// Otherwise, access is limited to just one screen.
	if (BC::checkComboPos(pos) != SH::_NoError)
		return rpos_handle_t{};

	if (type == mapdata_type::CanonicalScreen)
		return {scr, screen, 0, (rpos_t)pos, pos};

	if (scrolling())
	{
		if (!scr->is_valid())
			return rpos_handle_t{};

		return {scr, screen, layer, (rpos_t)pos, pos};
	}

	rpos_t rpos = POS_TO_RPOS(pos, screen);
	if (BC::checkComboRpos(rpos) != SH::_NoError)
		return rpos_handle_t{};

	return {scr, screen, layer, rpos, pos};
}

ffc_handle_t mapdata::resolve_ffc_handle(int index)
{
	index -= 1;
	if (BC::checkMapdataFFC(index) != SH::_NoError)
		return ffc_handle_t{};

	int screen_index_offset = 0;
	if (current() && layer == 0)
		screen_index_offset = get_region_screen_offset(screen);

	return *scr->getFFCHandle(index, screen_index_offset);
}

ffcdata* mapdata::resolve_ffc(int index)
{
	return resolve_ffc_handle(index).ffc;
}

static ffc_handle_t ResolveMapdataFFC(int32_t mapref, int index)
{
	index -= 1;
	if (BC::checkMapdataFFC(index) != SH::_NoError)
		return ffc_handle_t{};

	auto result = decode_mapdata_ref(mapref);
	if (!result.scr)
	{
		scripting_log_error_with_context("mapdata id is invalid: {}", mapref);
		return ffc_handle_t{};
	}

	int screen_index_offset = 0;
	if (result.current() && result.layer == 0)
		screen_index_offset = get_region_screen_offset(result.screen);

	return *result.scr->getFFCHandle(index, screen_index_offset);
}

int32_t genscript_timing = SCR_TIMING_START_FRAME;
static word max_valid_genscript;

void user_genscript::clear()
{
	wait_atleast = true;
	waituntil = SCR_TIMING_START_FRAME;
	waitevent = false;
	exitState = 0;
	reloadState = 0;
	eventstate = 0;
	initd.clear();
	data.clear();
	quit();
}
void user_genscript::launch()
{
	quit();
	doscript() = true;
	wait_atleast = true;
	waituntil = SCR_TIMING_START_FRAME;
	waitevent = false;
}
void user_genscript::quit()
{
	if(indx > -1)
	{
		FFCore.destroyScriptableObject(ScriptType::Generic, indx);
	}
	_doscript = false;
}
byte& user_genscript::doscript()
{
	return _doscript;
}
byte const& user_genscript::doscript() const
{
	return _doscript;
}


user_genscript& user_genscript::get(int ind)
{
	if(ind < 1 || ind >= NUMSCRIPTSGENERIC)
		ind = 0;
	user_scripts[ind].indx = ind;
	return user_scripts[ind];
}
user_genscript user_genscript::user_scripts[NUMSCRIPTSGENERIC];

void countGenScripts()
{
	max_valid_genscript = 0;
	for(auto q = 1; q < NUMSCRIPTSGENERIC; ++q)
	{
		if(genericscripts[q] && genericscripts[q]->valid())
			max_valid_genscript = q;
	}
}
void timeExitAllGenscript(byte exState)
{
	for(auto q = 1; q <= max_valid_genscript; ++q)
		user_genscript::get(q).timeExit(exState);
}
void throwGenScriptEvent(int32_t event)
{
	for(auto q = 1; q <= max_valid_genscript; ++q)
	{
		user_genscript& scr = user_genscript::get(q);
		if(!scr.doscript()) continue;
		if(!genericscripts[q]->valid()) continue;
		if(!scr.waitevent) continue;
		if(scr.eventstate & (1<<event))
		{
			auto& data = get_script_engine_data(ScriptType::Generic, q);
			data.ref.d[rEXP1] = event*10000;
			scr.waitevent = false;
			
			//Run the script!
			ZScriptVersion::RunScript(ScriptType::Generic, q, q);
		}
	}
}

void load_genscript(const gamedata& gd)
{
	for(size_t q = 0; q < NUMSCRIPTSGENERIC; ++q)
	{
		user_genscript& gen = user_genscript::get(q);
		gen.clear();
		gen.doscript() = gd.gen_doscript.get(q);
		gen.exitState = gd.gen_exitState[q];
		gen.reloadState = gd.gen_reloadState[q];
		gen.eventstate = gd.gen_eventstate[q];
		gen.initd = gd.gen_initd[q];
		gen.data = gd.gen_data[q];
	}
}
void load_genscript(const zinitdata& zd)
{
	for(size_t q = 0; q < NUMSCRIPTSGENERIC; ++q)
	{
		user_genscript& gen = user_genscript::get(q);
		gen.clear();
		gen.doscript() = zd.gen_doscript.get(q);
		gen.exitState = zd.gen_exitState[q];
		gen.reloadState = zd.gen_reloadState[q];
		gen.eventstate = zd.gen_eventstate[q];
		gen.initd = zd.gen_initd[q];
		gen.data = zd.gen_data[q];
	}
}

void save_genscript(gamedata& gd)
{
	for(size_t q = 0; q < NUMSCRIPTSGENERIC; ++q)
	{
		user_genscript const& gen = user_genscript::get(q);
		gd.gen_doscript.set(q, gen.doscript());
		gd.gen_exitState[q] = gen.exitState;
		gd.gen_reloadState[q] = gen.reloadState;
		gd.gen_eventstate[q] = gen.eventstate;
		gd.gen_initd[q] = gen.initd;
		gd.gen_data[q] = gen.data;
	}
}

void FFScript::runGenericPassiveEngine(int32_t scrtm)
{
	if(!max_valid_genscript) return; //No generic scripts in the quest!
	bool init = (scrtm == SCR_TIMING_INIT);
	if(!init)
	{
		if(genscript_timing != scrtm)
		{
			while(genscript_timing != scrtm)
				runGenericPassiveEngine(genscript_timing);
		}
	}
	for(auto q = 1; q <= max_valid_genscript; ++q)
	{
		user_genscript& scr = user_genscript::get(q);
		if(!scr.doscript()) continue;
		if(!genericscripts[q]->valid()) continue;
		if(scr.waitevent) continue;
		if(!init && (scr.waituntil > scrtm || (!scr.wait_atleast && scr.waituntil != scrtm)))
			continue;
		
		//Run the script!
		ZScriptVersion::RunScript(ScriptType::Generic, q, q);
	}
	if(init || genscript_timing >= SCR_TIMING_END_FRAME)
		genscript_timing = SCR_TIMING_START_FRAME;
	else ++genscript_timing;
}

void FFScript::initZScriptDMapScripts()
{
	scriptEngineDatas[{ScriptType::DMap, 0}] = ScriptEngineData();
	scriptEngineDatas[{ScriptType::ScriptedPassiveSubscreen, 0}] = ScriptEngineData();
}

void FFScript::initZScriptSubscreenScript()
{
	scriptEngineDatas[{ScriptType::EngineSubscreen, 0}] = ScriptEngineData();
}
void FFScript::initZScriptScriptedActiveSubscreen()
{
	scriptEngineDatas[{ScriptType::ScriptedActiveSubscreen, 0}] = ScriptEngineData();
}

void FFScript::initZScriptOnMapScript()
{
	scriptEngineDatas[{ScriptType::OnMap, 0}] = ScriptEngineData();
}

void FFScript::initZScriptHeroScripts()
{
	scriptEngineDatas[{ScriptType::Hero, 0}] = ScriptEngineData();
}

void FFScript::initZScriptItemScripts()
{
	for ( int32_t q = 0; q < 256; q++ )
	{
		auto& data = get_script_engine_data(ScriptType::Item, q);
		data.reset();
		data.doscript = (itemsbuf[q].flags&item_passive_script) && game->item[q];
	}

	for ( int32_t q = -256; q < 0; q++ )
	{
		auto& data = get_script_engine_data(ScriptType::Item, q);
		data.reset();
		data.doscript = 0;
	}
}

int get_mouse_state(int index)
{
	int value = 0;
	if (replay_is_replaying())
	{
		value = replay_get_mouse(index);
	}
	else if (index == 0)
	{
		value = script_mouse_x;
	}
	else if (index == 1)
	{
		value = script_mouse_y;
	}
	else if (index == 2)
	{
		value = script_mouse_z;
	}
	else if (index == 3)
	{
		value = script_mouse_b;
	}

	if (replay_is_recording())
	{
		replay_set_mouse(index, value);
	}

	return value;
}

///---------------------------------------------//
//           Array Helper Functions           //
///---------------------------------------------//

#define ZCARRAY_MAX_SIZE 214748

size_t ArrayH::getSize(const int32_t ptr)
{
	ArrayManager am(ptr);
	return am.size();
}

//Can't you get the std::string and then check its length?
int32_t ArrayH::strlen(const int32_t ptr)
{
	ArrayManager am(ptr);
	if (am.invalid() || am.size() == 0)
		return -1;
		
	word count;
	size_t sz = am.size();
	for(count = 0; BC::checkUserArrayIndex(count, sz) == _NoError
		&& am.get(count) != '\0'; count++);
	
	return count;
}

//Returns values of a zscript array as an std::string.
void ArrayH::getString(const int32_t ptr, string &str, dword num_chars, dword offset)
{
	ArrayManager am(ptr);
	
	if(am.invalid())
	{
		str.clear();
		return;
	}
	
	str.clear();
	size_t sz = am.size();
	for(word i = offset; BC::checkUserArrayIndex(i, sz) == _NoError && am.get(i) != '\0' && num_chars != 0; i++)
	{
		int32_t c = am.get(i) / 10000;
		if(byte(c) != c)
		{
			Z_scripterrlog("Illegal char value (%d) at position [%d] in string pointer %d\n", c, i, ptr);
			Z_scripterrlog("Value of invalid char will overflow.\n");
		}
		str += byte(c);
		--num_chars;
	}
}

//Used for issues where reading the ZScript array floods the console with errors 'Accessing array index [12] size of 12.
//Happens with Quad3D and some other functions, and I have no clue why. -Z ( 28th April, 2019 )
//Like getString but for an array of longs instead of chars. *(arrayPtr is not checked for validity)
void ArrayH::getValues2(const int32_t ptr, int32_t* arrayPtr, dword num_values, dword offset) //a hack -Z
{
	ArrayManager am(ptr);
	
	if(am.invalid())
		return;
	
	size_t sz = am.size();
	for(word i = offset; BC::checkUserArrayIndex(i, sz+1) == _NoError && num_values != 0; i++)
	{
		arrayPtr[i] = (am.get(i) / 10000);
		num_values--;
	}
}

//Like getString but for an array of longs instead of chars. *(arrayPtr is not checked for validity)
void ArrayH::getValues(const int32_t ptr, int32_t* arrayPtr, dword num_values, dword offset)
{
	ArrayManager am(ptr);
	
	if (am.invalid())
		return;
	size_t sz = am.size();
	for(word i = offset; num_values != 0 && BC::checkUserArrayIndex(i, sz) == _NoError; i++)
	{
		arrayPtr[i] = (am.get(i) / 10000);
		num_values--;
	}
}

void ArrayH::copyValues(const int32_t ptr, const int32_t ptr2)
{
	ArrayManager am1(ptr), am2(ptr2);
	if(am1.invalid() || am2.invalid())
		return;

	int sz = std::min(am1.size(),am2.size());
	for (int i = 0; i < sz; i++)
	{
		am1.set(i,am2.get(i));
	}
}
//Get element from array
INLINE int32_t ArrayH::getElement(const int32_t ptr, int32_t offset, const bool neg)
{
	ArrayManager am(ptr,neg);
	return am.get(offset);
}

//Set element in array
INLINE void ArrayH::setElement(const int32_t ptr, int32_t offset, const int32_t value, const bool neg)
{
	ArrayManager am(ptr,neg);
	am.set(offset,value);
}

int32_t ArrayH::setArray(const int32_t ptr, string const& s2, bool resize)
{
	ArrayManager am(ptr);
	
	if (am.invalid())
		return _InvalidPointer;
	
	size_t i;
	
	if(am.can_resize() && resize)
		am.resize_min(s2.size()+1);
	
	size_t sz = am.size();
	for(i = 0; i < s2.size(); i++)
	{
		if(i >= sz)
		{
			am.set(sz-1,'\0');
			return _Overflow;
		}
		
		if(BC::checkUserArrayIndex(i, sz) == _NoError)
			am.set(i,s2[i] * 10000);
	}
	
	if(BC::checkUserArrayIndex(i, sz) == _NoError)
		am.set(i,'\0');
		
	return _NoError;
}

ArrayManager::ArrayManager(int32_t ptr, bool neg) : negAccess(neg)
{
	_invalid = false;
	internal_array_id = {};
	legacy_internal_id = 0;

	if (ZScriptVersion::gc_arrays())
	{
		if (auto* array = checkArray(ptr))
		{
			if (array->internal_id.has_value())
			{
				if (array->internal_expired)
				{
					current_zasm_extra_context = scripting_get_zasm_register_context_string(array->internal_id->zasm_var);
					scripting_log_error_with_context("Invalid internal array: the object this array refers to has expired");
					_invalid = true;
					return;
				}

				aptr = nullptr;
				internal_array_id = array->internal_id.value();
				if (!zasm_array_supports(internal_array_id.zasm_var))
				{
					scripting_log_error_with_context("Invalid internal array id: {}", internal_array_id.zasm_var);
					_invalid = true;
					return;
				}
			}
			else
			{
				aptr = &array->arr;
			}
		}
		else
		{
			aptr = &INVALIDARRAY;
			_invalid = true;
		}

		if (_invalid)
			scripting_log_error_with_context("Invalid pointer used as array: {}", ptr);
		return;
	}

	ptr /= 10000;

	if(ptr >= INTARR_OFFS)
	{
		aptr = nullptr;
		legacy_internal_id = ptr;
		if(legacy_sz_int_arr(ptr) < 0)
			_invalid = true;
	}
	else if(ptr == 0)
	{
		aptr = &INVALIDARRAY;
		_invalid = true;
	}
	else if(ptr < 0) //An object array?
	{
		int32_t objptr = -ptr;
		auto it = objectRAM.find(objptr);
		if(it == objectRAM.end())
		{
			aptr = &INVALIDARRAY;
			_invalid = true;
		}
		else aptr = &(it->second);
	}
	else if(ptr >= NUM_ZSCRIPT_ARRAYS) //Then it's a global
	{
		dword gptr = ptr - NUM_ZSCRIPT_ARRAYS;
		
		if(gptr > game->globalRAM.size())
		{
			aptr = &INVALIDARRAY;
			_invalid = true;
		}
		else aptr = &(game->globalRAM[gptr]);
	}
	else
	{
		if(!localRAM[ptr].Valid())
		{
			aptr = &INVALIDARRAY;
			_invalid = true;
		}
		else aptr = &(localRAM[ptr]);
	}
	if (_invalid)
	{
		scripting_log_error_with_context("Invalid pointer used as array: {}", ptr);
	}
}
ArrayManager::ArrayManager(int32_t ptr) : ArrayManager(ptr,can_neg_array){}

int32_t ArrayManager::get(int32_t indx) const
{
	if(_invalid) return -10000;
	if(aptr)
	{
		int32_t sz = size();
		if(BC::checkUserArrayIndex(indx, sz, negAccess) == SH::_NoError)
		{
			if(indx < 0)
				indx += sz; //[-1] becomes [size-1] -Em
			return (*aptr)[indx];
		}
	}
	else //internal special array
	{
		if (ZScriptVersion::gc_arrays())
			return zasm_array_get(internal_array_id.zasm_var, internal_array_id.ref, indx);

		int32_t sz = size();
		if(sz >= 0 && BC::checkUserArrayIndex(indx, sz, negAccess) == SH::_NoError)
		{
			if(indx < 0)
				indx += sz; //[-1] becomes [size-1] -Em
			return legacy_get_int_arr(legacy_internal_id, indx);
		}
	}
	return -10000;
}

void ArrayManager::set(int32_t indx, int32_t val)
{
	if(_invalid) return;

	if(aptr)
	{
		int32_t sz = size();
		if(BC::checkUserArrayIndex(indx, sz, negAccess) == SH::_NoError)
		{
			if(indx < 0)
				indx += sz; //[-1] becomes [size-1] -Em
			if (aptr->HoldsObjects())
			{
				int id = (*aptr)[indx];
				script_object_ref_dec(id);
			}
			(*aptr)[indx] = val;
			if (aptr->HoldsObjects())
				script_object_ref_inc(val);
		}
	}
	else //internal special array
	{
		if (ZScriptVersion::gc_arrays())
		{
			zasm_array_set(internal_array_id.zasm_var, internal_array_id.ref, indx, val);
			return;
		}

		int32_t sz = size();
		if(sz >= 0 && BC::checkUserArrayIndex(indx, sz, negAccess) == SH::_NoError)
		{
			if(indx < 0)
				indx += sz; //[-1] becomes [size-1] -Em
			legacy_set_int_arr(legacy_internal_id, indx, val);
		}
	}
}

int32_t ArrayManager::size() const
{
	if(_invalid) return -1;
	if(aptr)
		return aptr->Size();
	else // Internal special
	{
		if (ZScriptVersion::gc_arrays())
			return zasm_array_size(internal_array_id.zasm_var, internal_array_id.ref);

		int32_t sz = legacy_sz_int_arr(legacy_internal_id);
		if(sz < 0)
			return -1;
		return sz;
	}
}

bool ArrayManager::resize(size_t newsize)
{
	if(_invalid) return false;
	if(!aptr)
	{
		log_invalid_operation();
		return false;
	}
	if (aptr->HoldsObjects())
	{
		for (int i = newsize; i < aptr->Size(); i++)
		{
			auto id = (*aptr)[i];
			script_object_ref_dec(id);
		}
	}
	aptr->Resize(newsize);
	return true;
}

bool ArrayManager::resize_min(size_t newsize)
{
	if(size() >= newsize)
		return true;
	return resize(newsize);
}

bool ArrayManager::can_resize()
{
	if(_invalid || !aptr)
		return false;
	return true;
}

bool ArrayManager::push(int32_t val, int indx)
{
	if(_invalid) return false;
	if(!aptr)
	{
		log_invalid_operation();
		return false;
	}
	if(aptr->Size() == ZCARRAY_MAX_SIZE)
		return false;
	aptr->Push(val,indx);
	if (aptr->HoldsObjects())
		script_object_ref_inc(val);
	return true;
}

int32_t ArrayManager::pop(int indx)
{
	if(_invalid) return -10000;
	if(!aptr)
	{
		log_invalid_operation();
		return -10000;
	}

	if (aptr->Empty())
	{
		scripting_log_error_with_context("Array had nothing to Pop!");
		return -10000;
	}

	int32_t val = aptr->Pop(indx);
	if (aptr->HoldsObjects())
		script_object_ref_dec(val);
	return val;
}

std::string ArrayManager::asString(std::function<char const*(int32_t)> formatter, const size_t& limit) const
{
	if(_invalid) return "{ INVALID ARRAY }";
	std::ostringstream oss;
	oss << "{ ";
	size_t s = size();
	bool overflow = limit < s;
	if(overflow)
		s = limit;
	
	for(auto q = 0; q < s; ++q)
	{
		oss << formatter(get(q));
		if (q + 1 < s)
			oss << ", ";
	}
	if (overflow)
		oss << ", ...";
	oss << " }";
	return oss.str();
}

void ArrayManager::log_invalid_operation() const
{
	if (internal_array_id.zasm_var)
	{
		current_zasm_extra_context = scripting_get_zasm_register_context_string(internal_array_id.zasm_var);
		scripting_log_error_with_context("Internal array not valid for this operation");
	}
	else
	{
		scripting_log_error_with_context("Internal array '{}' not valid for this operation", legacy_internal_id);
	}
}

// Call only when the underlying engine object is being deleted. This deallocs script data, and
// invalidates any internal array references that may remain.
// Any script type given to this function must also be handled in
// script_array::internal_array_id::matches.
void FFScript::destroyScriptableObject(ScriptType scriptType, const int32_t UID)
{
	FFCore.deallocateAllScriptOwned(scriptType, UID);
	FFCore.reset_script_engine_data(scriptType, UID);
	expire_internal_script_arrays(scriptType, UID);
}

void FFScript::destroyScriptableObjectsOfType(ScriptType scriptType)
{
	FFCore.deallocateAllScriptOwnedOfType(scriptType);
	FFCore.clear_script_engine_data_of_type(scriptType);
	expire_internal_script_arrays(scriptType);
}

void FFScript::deallocateAllScriptOwned(ScriptType scriptType, const int32_t UID)
{
	std::vector<uint32_t> ids_to_clear;
	for (auto& script_object : script_objects | std::views::values)
	{
		if (script_object->own_clear(scriptType, UID))
		{
			ids_to_clear.push_back(script_object->id);
			script_object->owned_type = ScriptType::None;
			script_object->owned_i = 0;
		}
	}

	if (ZScriptVersion::gc() && script_engine_data_exists(scriptType, UID))
	{
		auto& data = get_script_engine_data(scriptType, UID);
		for (uint32_t offset : data.ref.stack_pos_is_object)
		{
			uint32_t id = data.stack[offset];
			ids_to_clear.push_back(id);
		}
		data.ref.stack_pos_is_object.clear();
	}

	if (ZScriptVersion::gc())
	{
		for (auto id : ids_to_clear)
			script_object_ref_dec(id);
	}
	else
	{
		for (auto id : ids_to_clear)
			delete_script_object(id);
	}

	if (!ZScriptVersion::gc_arrays())
	{
		for(int32_t i = 1; i < NUM_ZSCRIPT_ARRAYS; i++)
		{
			if(arrayOwner[i].own_clear(scriptType,UID))
				deallocateArray(i);
		}
	}
}

void FFScript::deallocateAllScriptOwnedOfType(ScriptType scriptType)
{
	std::vector<uint32_t> ids_to_clear;
	for (auto& script_object : script_objects | std::views::values)
	{
		if (script_object->owned_type == scriptType)
		{
			ids_to_clear.push_back(script_object->id);
			script_object->owned_type = ScriptType::None;
			script_object->owned_i = 0;
		}
	}

	if (ZScriptVersion::gc())
	{
		for (auto& [key, data] : scriptEngineDatas)
		{
			if (key.first != scriptType)
				continue;

			for (uint32_t offset : data.ref.stack_pos_is_object)
			{
				uint32_t id = data.stack[offset];
				ids_to_clear.push_back(id);
			}
			data.ref.stack_pos_is_object.clear();
		}

		for (auto id : ids_to_clear)
			script_object_ref_dec(id);
	}
	else
	{
		for (auto id : ids_to_clear)
			delete_script_object(id);
	}

	if (!ZScriptVersion::gc_arrays())
	{
		for(int32_t i = 1; i < NUM_ZSCRIPT_ARRAYS; i++)
		{
			if(arrayOwner[i].owned_type == scriptType)
				deallocateArray(i);
		}
	}
}

// Only called when resetting the engine. Don't keep anything.
void FFScript::deallocateAllScriptOwned()
{
	script_object_ids_by_type.clear();
	script_objects.clear();
	next_script_object_id_freelist.clear();

	if (!ZScriptVersion::gc_arrays())
	{
		for(int32_t i = 1; i < NUM_ZSCRIPT_ARRAYS; i++)
		{
			if(localRAM[i].Valid())
			{
				// Unowned arrays are ALSO deallocated!
				arrayOwner[i].clear();
				localRAM[i].Clear();
			}
		}
	}
}

void FFScript::deallocateAllScriptOwnedCont()
{
	std::vector<uint32_t> ids_to_clear;
	for (auto& script_object : script_objects | std::views::values)
	{
		if (script_object->own_clear_cont())
		{
			ids_to_clear.push_back(script_object->id);
			script_object->owned_type = ScriptType::None;
			script_object->owned_i = 0;
		}
	}

	if (ZScriptVersion::gc())
	{
		for (auto& [key, data] : scriptEngineDatas)
		{
			for (uint32_t offset : data.ref.stack_pos_is_object)
			{
				uint32_t id = data.stack[offset];
				ids_to_clear.push_back(id);
			}
			data.ref.stack_pos_is_object.clear();
		}

		for (auto id : ids_to_clear)
			script_object_ref_dec(id);
	}
	else
	{
		for (auto id : ids_to_clear)
			delete_script_object(id);
	}

	if (!ZScriptVersion::gc_arrays())
	{
		//No QR check here- always deallocate on quest exit.
		for(int32_t i = 1; i < NUM_ZSCRIPT_ARRAYS; i++)
		{
			if(localRAM[i].Valid())
			{
				if(arrayOwner[i].own_clear_cont())
					deallocateArray(i);
			}
		}
	}
}

weapon *checkLWpn(int32_t uid)
{
	return ResolveSprite<weapon>(uid, "lweapon");
}

weapon *checkEWpn(int32_t uid)
{
	return ResolveSprite<weapon>(uid, "eweapon");
}

weapon *checkWpn(int32_t uid)
{
	return ResolveSprite<weapon>(uid, "weapon");
}

user_file *checkFile(int32_t ref, bool req_file = false, bool skipError = false)
{
	user_file* file = user_files.check(ref, skipError);
	if (file && req_file && !file->file)
	{
		if (skipError) return NULL;

		scripting_log_error_with_context("Script attempted to reference an invalid file!");
		Z_scripterrlog("File with UID = %d does not have an open file connection!\n", ref);
		Z_scripterrlog("Use '->Open()' or '->Create()' to hook to a system file.\n");
		return NULL;
	}
	return file;
}

user_genscript *checkGenericScr(int32_t ref)
{
	if (BC::checkBounds(ref, 1, NUMSCRIPTSGENERIC-1) != SH::_NoError)
		return NULL;

	return &user_genscript::get(ref);
}
extern portal mirror_portal;
portal *checkPortal(int32_t ref, bool skiperr = false)
{
	if(ref == -1)
		return &mirror_portal;

	portal* p = (portal*)portals.getByUID(ref);
	if(!p)
	{
		if(!skiperr)
			scripting_log_error_with_context("Invalid portal pointer: {}", ref);
		return nullptr;
	}
	return p;
}

savedportal *checkSavedPortal(int32_t ref, bool skiperr = false)
{
	savedportal* sp = game->getSavedPortal(ref);
	if(!sp)
	{
		if(!skiperr)
			scripting_log_error_with_context("Invalid savedportal pointer: {}", ref);
		return nullptr;
	}
	return sp;
}
int32_t getPortalFromSaved(savedportal* p)
{
	if(p == &(game->saved_mirror_portal))
		return -1;
	portal* prtl = nullptr;
	portals.forEach([&](sprite& spr)
	{
		portal* tmp = (portal*)&spr;
		if(p->getUID() == tmp->saved_data)
		{
			prtl = tmp;
			return true;
		}
		return false;
	});
	return prtl ? prtl->getUID() : 0;
}

static user_dir *checkDir(uint32_t id,  bool skipError = false)
{
	return user_dirs.check(id, skipError);
}

static user_stack *checkStack(uint32_t id, bool skipError = false)
{
	return user_stacks.check(id, skipError);
}

static user_rng *checkRNG(uint32_t id, bool skipError = false)
{
	// A null RNG pointer is special-case, access engine rng.
	if (id == 0) return &nulrng;
	return user_rngs.check(id, skipError);
}

user_paldata* checkPalData(int32_t ref, bool skipError)
{
	return user_paldatas.check(ref, skipError);
}

newcombo* checkCombo(int32_t ref, bool skipError)
{
	if (ref < 0 || ref > (MAXCOMBOS-1) )
	{
		scripting_log_error_with_context("Invalid combodata ID: {}", ref);
		return nullptr;
	}

	return &combobuf[ref];
}

newcombo* checkComboFromTriggerRef(dword ref)
{
	ref = get_combo_from_trigger_ref(ref);
	return checkCombo(ref);
}

dmap* checkDmap(int32_t ref)
{
	if (BC::checkDMapID(ref) != SH::_NoError)
		return nullptr;

	return &DMaps[ref];
}

ffcdata* checkFFC(int32_t ref)
{
	return ResolveFFC(ref);
}

enemy* checkNPC(int32_t ref)
{
	return ResolveNpc(ref);
}

guydata* checkNPCData(int32_t ref)
{
	if (ref >= 0 && ref < MAXNPCS)
		return &guysbuf[ref];

	scripting_log_error_with_context("Invalid {} using UID = {}", "npcdata", ref);
	return nullptr;
}

item* checkItem(int32_t ref)
{
	return ResolveItemSprite(ref);
}

itemdata* checkItemData(int32_t ref)
{
	if (ref >= 0 && ref < MAXITEMS)
		return &itemsbuf[ref];

	scripting_log_error_with_context("Invalid {} using UID = {}", "itemdata", ref);
	return nullptr;
}

mapdata* checkMapData(int32_t ref)
{
	static mapdata last_result;

	last_result = decode_mapdata_ref(ref);
	if (!last_result.scr)
	{
		scripting_log_error_with_context("Invalid {} using UID = {}", "mapdata", ref);
		return nullptr;
	}

	return &last_result;
}

mapscr* checkMapDataScr(int32_t ref)
{
	return decode_mapdata_ref(ref).scr;
}

screendata* checkScreen(int32_t ref)
{
	return (screendata*)get_scr_maybe(cur_map, ref);
}

bottletype* checkBottleData(int32_t ref, bool skipError)
{
	if(ref > 0 && ref <= 64)
	{
		return &QMisc.bottle_types[ref-1];
	}
	if(skipError) return NULL;

	scripting_log_error_with_context("Invalid {} using UID = {}", "bottledata", ref);
	return NULL;
}

bottleshoptype *checkBottleShopData(int32_t ref, bool skipError)
{
	if(ref > 0 && ref <= 256)
	{
		return &QMisc.bottle_shop_types[ref-1];
	}
	if(skipError) return NULL;

	scripting_log_error_with_context("Invalid {} using UID = {}", "bottleshopdata", ref);
	return NULL;
}

item_drop_object *checkDropSetData(int32_t ref)
{
	if(ref > 0 && ref < MAXITEMDROPSETS)
		return &item_drop_sets[ref];

	scripting_log_error_with_context("Invalid {} using UID = {}", "dropsetdata", ref);
	return NULL;
}

wpndata *checkSpriteData(int32_t ref)
{
	if(ref > 0 && ref < MAXWPNS)
		return &wpnsbuf[ref];

	scripting_log_error_with_context("Invalid {} using UID = {}", "spritedata", ref);
	return NULL;
}

MsgStr *checkMessageData(int32_t ref)
{
	if(ref > 0 && ref < msg_strings_size)
		return &MsgStrings[ref];

	scripting_log_error_with_context("Invalid {} using UID = {}", "messagedata", ref);
	return NULL;
}

combo_trigger* checkComboTrigger(dword ref)
{
	return get_combo_trigger(ref);
}

user_bitmap *checkBitmap(int32_t ref, bool req_valid = false, bool skipError = false)
{
	switch (ref - 10)
	{
		case rtSCREEN:
		case rtBMP0:
		case rtBMP1:
		case rtBMP2:
		case rtBMP3:
		case rtBMP4:
		case rtBMP5:
		case rtBMP6:
			zprint2("Internal error: 'checkBitmap()' recieved ref pointing to system bitmap!\n");
			zprint2("Please report this as a bug!\n");

			if(skipError) return NULL;

			scripting_log_error_with_context("Tried to reference a non-existent bitmap with UID = {}", ref);
			return NULL;

		default:
		{
			user_bitmap* b = user_bitmaps.check(ref, skipError);
			if (req_valid && (!b || !b->u_bmp))
			{
				if (skipError) return NULL;

				scripting_log_error_with_context("Tried to reference an invalid user bitmap with UID = {}.", ref);
				Z_scripterrlog("Did you forget to create the bitmap with `new bitmap()` or `->Create()`?.\n");
				return NULL;
			}
			return b;
		}
	}
}

extern const std::string subscr_names[sstMAX];
ZCSubscreen *checkSubData(int32_t ref, int req_ty)
{
	auto [ptr,ty] = load_subdata(ref);
	if(ptr)
	{
		if(req_ty < 0 || req_ty == ty)
			return ptr;
		else
		{
			scripting_log_error_with_context("Wrong type of SubscreenData accessed! Expecting type '{}', but found '{}'",
				subscr_names[req_ty], subscr_names[ty]);
		}
	}
	else scripting_log_error_with_context("Script attempted to reference a nonexistent SubscreenData!");
	
	scripting_log_error_with_context("You were trying to reference an invalid SubscreenData with UID = {}", ref);
	return NULL;
}

SubscrPage *checkSubPage(int32_t ref, int req_ty)
{
	auto [ptr,ty] = load_subpage(ref);
	if(ptr)
	{
		if(req_ty < 0 || req_ty == ty)
			return ptr;
		else
		{
			scripting_log_error_with_context("Wrong type of Subscreen accessed! Expecting type '{}', but found '{}'",
				subscr_names[req_ty], subscr_names[ty]);
		}
	}
	else scripting_log_error_with_context("Script attempted to reference a nonexistent SubscreenPage!");
	
	scripting_log_error_with_context("You were trying to reference an invalid SubscreenPage with UID = {}", ref);
	return NULL;
}

SubscrWidget *checkSubWidg(int32_t ref, int req_widg_ty, int req_sub_ty)
{
	auto [ptr,ty] = load_subwidg(ref);
	if(ptr)
	{
		if(req_sub_ty < 0 || req_sub_ty == ty)
		{
			if(req_widg_ty < 0 || req_widg_ty == ptr->getType())
				return ptr;
			else
			{
				auto listdata = GUI::ZCListData::subscr_widgets();
				scripting_log_error_with_context("Wrong type of SubscreenWidget accessed! Expecting type '{}', but found '{}'",
					listdata.findText(req_widg_ty), listdata.findText(ptr->getType()));
			}
		}
		else
		{
			scripting_log_error_with_context("Wrong type of Subscreen accessed! Expecting subscreen type '{}', but found '{}'",
				subscr_names[req_sub_ty], subscr_names[ty]);
		}
	}
	else scripting_log_error_with_context("Script attempted to reference a nonexistent SubscreenWidget!");

	scripting_log_error_with_context("You were trying to reference an invalid SubscreenWidget with UID = {}", ref);
	return NULL;
}

static void bad_subwidg_type(bool func, byte type)
{
	auto tyname = type < widgMAX ? subwidg_internal_names[type].c_str() : "";
	scripting_log_error_with_context("Widget type {} '{}' does not have this {}!",
		type, tyname, func ? "function" : "value");
}

// TODO: Remove this.
sprite *s;

int32_t item_flag(item_flags flag)
{
	if(unsigned(ri->idata) >= MAXITEMS)
	{
		scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
		return 0;
	}
	return (itemsbuf[ri->idata].flags & flag) ? 10000 : 0;
}
void item_flag(item_flags flag, bool val)
{
	if(unsigned(ri->idata) >= MAXITEMS)
	{
		scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
		return;
	}
	SETFLAG(itemsbuf[ri->idata].flags, flag, val);
}

bool scripting_use_8bit_colors;
int scripting_max_color_val;

static int scripting_read_pal_color(int c)
{
	return scripting_use_8bit_colors ? c : c / 4;
}

static int scripting_write_pal_color(int c)
{
	return scripting_use_8bit_colors ? c : _rgb_scale_6[c];
}

void apply_qr_rule(int qr_id)
{
	bool value = get_qr(qr_id);
	switch (qr_id)
	{
		case qr_LTTPWALK:
			Hero.setDiagMove(value?1:0);
			break;
		case qr_LTTPCOLLISION:
			Hero.setBigHitbox(value?1:0);
			break;
		case qr_ZS_NO_NEG_ARRAY:
			can_neg_array = !value;
			break;
		case qr_SCRIPTS_6_BIT_COLOR:
		{
			if (value)
			{
				scripting_use_8bit_colors = false;
				scripting_max_color_val = 63;
			}
			else
			{
				scripting_use_8bit_colors = true;
				scripting_max_color_val = 255;
			}
			break;
		}
		case qr_HIDE_BOTTOM_8_PIXELS:
		{
			updateShowBottomPixels();
			break;
		}
	}
}

static void apply_qr_rules()
{
	apply_qr_rule(qr_HIDE_BOTTOM_8_PIXELS);
	apply_qr_rule(qr_LTTPCOLLISION);
	apply_qr_rule(qr_LTTPWALK);
	apply_qr_rule(qr_SCRIPTS_6_BIT_COLOR);
	apply_qr_rule(qr_ZS_NO_NEG_ARRAY);
}

//Forward decl
int32_t do_msgheight(int32_t msg);
int32_t do_msgwidth(int32_t msg);
//

template <typename T, size_t N>
static int read_array(const T(&arr)[N], int index)
{
	if (BC::checkIndex(index, 0, N - 1) != SH::_NoError)
		return 0;

	return arr[index];
}

template <typename T, size_t N>
static bool write_array(T(&arr)[N], int index, T value)
{
	if (BC::checkIndex(index, 0, N - 1) != SH::_NoError)
		return false;

	arr[index] = value;
	return true;
}

static int get_ref(int arg)
{
	switch (arg)
	{
		case REFBITMAP: return ri->bitmapref;
		case REFBOTTLESHOP: return ri->bottleshopref;
		case REFBOTTLETYPE: return ri->bottletyperef;
		case REFCOMBODATA: return ri->combosref;
		case REFCOMBOTRIGGER: return ri->combotrigref;
		case REFDMAPDATA: return ri->dmapsref;
		case REFDROPS: return ri->dropsetref;
		case REFEWPN: return ri->ewpn;
		case REFFFC: return ri->ffcref;
		case REFGENERICDATA: return ri->genericdataref;
		case REFITEM: return ri->itemref;
		case REFITEMCLASS: return ri->idata;
		case REFLWPN: return ri->lwpn;
		case REFMAPDATA: return ri->mapsref;
		case REFMSGDATA: return ri->zmsgref;
		case REFNPC: return ri->guyref;
		case REFNPCCLASS: return ri->npcdataref;
		case REFPALDATA: return ri->paldataref;
		case REFSCREENDATA: return ri->screenref;
		case REFSHOPDATA: return ri->shopsref;
		case REFSPRITE: return ri->spriteref;
		case REFSPRITEDATA: return ri->spritedataref;
		case REFSUBSCREEN: return ri->subdataref;
		case REFSUBSCREENPAGE: return ri->subpageref;
		case REFSUBSCREENWIDG: return ri->subwidgref;

		default: NOTREACHED();
	}
}

int32_t earlyretval = -1;
int32_t get_register(int32_t arg)
{
	if (arg >= D(0) && arg <= D(7))
		return ri->d[arg - D(0)];

	if (arg >= GD(0) && arg <= GD(MAX_SCRIPT_REGISTERS))
		return game->global_d[arg - GD(0)];

	int32_t ret = 0;

	#define GET_SPRITEDATA_VAR_INT(member) \
	{ \
		if(unsigned(ri->spritedataref) > (MAXWPNS-1) )    \
		{ \
			ret = -10000; \
			scripting_log_error_with_context("Invalid Sprite ID: {}", ri->spritedataref*10000); \
		} \
		else \
			ret = (wpnsbuf[ri->spritedataref].member * 10000); \
	}

	current_zasm_register = arg;

	// Do not ever use `return` in these cases!
	switch(arg)
	{
		case MAX_FFC_ID:
		{
			ret = (MAX_FFCID + 1) * 10000;
			break;
		}

		case INCQST:
		{
			int32_t newqst = 0;
			if ( game->get_quest() < 255 )  //255 is a custom quest
			{
				newqst = (game->get_quest()+1);
			}
			else
			{
				newqst = 1;
			}
			if ( newqst < 11 ) 
			{
			
				ret = newqst * 10000;
				Quit = qINCQST;
				//ending();
			
			}
			else ret = -10000;
			break;
		}
		case DEBUGTESTING:
			ret = use_testingst_start ? 10000 : 0;
			break;
		
		///----------------------------------------------------------------------------------------------------//
		//FFC Variables
		case DATA:
			if (auto ffc = ResolveFFC(ri->ffcref))
				ret = ffc->data * 10000;
			break;
			
		case FFSCRIPT:
			if(auto ffc = ResolveFFC(ri->ffcref))
				ret = ffc->script * 10000;
			break;
			
		case FCSET:
			if(auto ffc = ResolveFFC(ri->ffcref))
				ret = ffc->cset * 10000;
			break;
			
		case DELAY:
			if(auto ffc = ResolveFFC(ri->ffcref))
				ret = ffc->delay * 10000;
			break;
			
		case FX:
			if(auto ffc = ResolveFFC(ri->ffcref))
				ret = ffc->x.getZLong();
			break;
			
		case FY:
			if(auto ffc = ResolveFFC(ri->ffcref))
				ret = ffc->y.getZLong();
			break;
			
		case XD:
			if(auto ffc = ResolveFFC(ri->ffcref))
				ret = ffc->vx.getZLong();
			break;
			
		case YD:
			if(auto ffc = ResolveFFC(ri->ffcref))
				ret = ffc->vy.getZLong();
			break;
		case FFCID:
			if (auto ffc = ResolveFFC(ri->ffcref))
				ret = (get_region_screen_offset(ffc->screen_spawned) * MAXFFCS + ffc->index + 1) * 10000;
			break;
			
		case XD2:
			if(auto ffc = ResolveFFC(ri->ffcref))
				ret = ffc->ax.getZLong();
			break;
			
		case YD2:
			if(auto ffc = ResolveFFC(ri->ffcref))
				ret = ffc->ay.getZLong();
			break;
			
		case FFCWIDTH:
			if(auto ffc = ResolveFFC(ri->ffcref))
				ret = ffc->hit_width * 10000;
			break;
			
		case FFCHEIGHT:
			if(auto ffc = ResolveFFC(ri->ffcref))
				ret = ffc->hit_height * 10000;
			break;
			
		case FFTWIDTH:
			if(auto ffc = ResolveFFC(ri->ffcref))
				ret = ffc->txsz * 10000;
			break;
			
		case FFTHEIGHT:
			if(auto ffc = ResolveFFC(ri->ffcref))
				ret = ffc->tysz * 10000;
			break;
			
		case FFCLAYER:
			if (auto ffc = ResolveFFC(ri->ffcref))
				ret = ffc->layer * 10000;
			break;
			
		case FFLINK:
			if(auto ffc = ResolveFFC(ri->ffcref))
				ret = ffc->link * 10000;
			break;
		
		///----------------------------------------------------------------------------------------------------//
		//Hero's Variables
		case LINKX:
		{
			if (get_qr(qr_SPRITEXY_IS_FLOAT))
			{
				ret = Hero.getX().getZLong();
			}
			else ret = int32_t(Hero.getX()) * 10000;

			break;
		}
		
		case LINKCSET:
		{
			ret = Hero.cs * 10000;
			break;
		}		
		case LINKY:
		{
			if (get_qr(qr_SPRITEXY_IS_FLOAT))
			{
				ret = Hero.getY().getZLong();
			}
			else ret = int32_t(Hero.getY()) * 10000;

			break;
		}    
		case LINKZ:
		{
			if (get_qr(qr_SPRITEXY_IS_FLOAT))
			{
				ret = Hero.getZ().getZLong();
			}
			else ret = int32_t(Hero.getZ()) * 10000;

			break;
		} 
		case LINKJUMP:
			ret = Hero.getJump().getZLong();
			break;
			
		case HEROFAKEJUMP:
			ret = Hero.getFakeJump().getZLong() / -100;
			break;
			
		case LINKDIR:
			ret=(int32_t)(Hero.dir)*10000;
			break;
			
		case LINKHITDIR:
			ret=(int32_t)(Hero.getHitDir())*10000;
			break;
			
		case LINKHP:
			ret=(int32_t)(game->get_life())*10000;
			break;
		
		case LINKGRAVITY:
			ret = ( (Hero.moveflags & move_obeys_grav) ? 10000 : 0 );
			break;
		
		case HERONOSTEPFORWARD:
			ret = ( (FFCore.nostepforward) ? 10000 : 0 );
			break;
			
		case LINKMP:
			ret=(int32_t)(game->get_magic())*10000;
			break;
			
		case LINKMAXHP:
			ret=(int32_t)(game->get_maxlife())*10000;
			break;
			
		case LINKMAXMP:
			ret=(int32_t)(game->get_maxmagic())*10000;
			break;
			
		case LINKACTION:
		{
			ret = FFCore.getHeroAction() * 10000;
			break;
		}
		
		case HEROHEALTHBEEP:
		{
			ret = heart_beep ? ( heart_beep_timer * 10000 ) : 0;
			break;
		}
			
		case LINKHELD:
			ret = (int32_t)(Hero.getHeldItem())*10000;
			break;
			
		case HEROSTEPRATE:
			ret = Hero.getStepRate() * 10000;
			break;
		case HEROSHOVEOFFSET:
			ret = Hero.shove_offset.getZLong();
			break;
			
		case LINKEQUIP:
			ret = ((Awpn&0xFF)|((Bwpn&0xFF)<<8))*10000;
			break;
			
		case LINKINVIS:
			ret = (((int32_t)(Hero.getDontDraw())) ? 10000 : 0);
			break;
			
		case LINKINVINC:
			ret = (int32_t)(Hero.scriptcoldet)*10000;
			break;
		
		case LINKENGINEANIMATE:
			ret = (int32_t)(Hero.do_animation)*10000;
			break;
			
		case LINKLADDERX:
			ret=(int32_t)(Hero.getLadderX())*10000;
			break;
			
		case LINKLADDERY:
			ret=(int32_t)(Hero.getLadderY())*10000;
			break;
			
		case LINKSWORDJINX:
			ret = (int32_t)(Hero.getSwordClk())*10000;
			break;
			
		case LINKITEMJINX:
			ret = (int32_t)(Hero.getItemClk())*10000;
			break;
			
		case LINKDRUNK:
			ret = (int32_t)(Hero.DrunkClock())*10000;
			break;
			
		case LINKROTATION:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				ret = -1; break;
			}
			ret = (int32_t)(Hero.rotation)*10000;
			break;
		
		case LINKSCALE:
		{
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				ret = -1; break;
			}
			ret = (int32_t)(Hero.scale*100.0);
			break;
		}
		

		case LINKHXOFS:
			ret = (int32_t)(Hero.hxofs)*10000;
			break;
			
		case LINKHYOFS:
			ret = (int32_t)(Hero.hyofs)*10000;
			break;
			
		case LINKXOFS:
			ret = (int32_t)(Hero.xofs)*10000;
			break;
			
		case LINKYOFS:
			ret = (int32_t)(Hero.yofs-(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset))*10000;
			break;
			
		case HEROSHADOWXOFS:
			ret = (int32_t)(Hero.shadowxofs)*10000;
			break;
			
		case HEROSHADOWYOFS:
			ret = (int32_t)(Hero.shadowyofs)*10000;
			break;
			
		case HEROTOTALDYOFFS:
			ret = 10000*(((int32_t)(Hero.yofs-(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)))
				+ ((Hero.switch_hooked && Hero.switchhookstyle == swRISE)
					? -(8-(abs(Hero.switchhookclk-32)/4)) : 0));
			break;
			
		case LINKZOFS:
			ret = (int32_t)(Hero.zofs)*10000;
			break;
			
		case LINKHXSZ:
			ret = (int32_t)(Hero.hit_width)*10000;
			break;
			
		case LINKHYSZ:
			ret = (int32_t)(Hero.hit_height)*10000;
			break;
			
		case LINKHZSZ:
			ret = (int32_t)(Hero.hzsz)*10000;
			break;
			
		case LINKTXSZ:
			ret = (int32_t)(Hero.txsz)*10000;
			break;
			
		case LINKTYSZ:
			ret = (int32_t)(Hero.tysz)*10000;
			break;
			
		case LINKTILE:
			ret = (int32_t)(Hero.tile)*10000;
			break;
			
		case LINKFLIP:
			ret = (int32_t)(Hero.flip)*10000;
			break;
		
		case LINKINVFRAME:
			ret = (int32_t)Hero.getHClk()*10000;
			break;
		
		case LINKCANFLICKER:
			ret= Hero.getCanFlicker()?10000:0;
			break;
		case LINKHURTSFX:
			ret = (int32_t)Hero.getHurtSFX()*10000;
			break;
		
		/*
		case LINKUSINGITEM:
			ret = (int32_t)Hero.getDirectItem()*10000;
			break;
		
		case LINKUSINGITEMA:
			ret = (int32_t)Hero.getDirectItemA()*10000;
			break;
		
		case LINKUSINGITEMB:
			ret = (int32_t)Hero.getDirectItemB()*10000;
			break;
		*/
			
		case LINKEATEN:
			ret=(int32_t)Hero.getEaten()*10000;
			break;
		case LINKGRABBED:
			ret = Hero.inwallm ? 10000 : 0;
			break;
		case HEROBUNNY:
			ret = Hero.BunnyClock()*10000;
			break;
		case LINKPUSH:
			ret=(int32_t)Hero.getPushing()*10000;
			break;
		case LINKSTUN:
			ret=(int32_t)Hero.StunClock()*10000;
			break;
		case LINKSCRIPTTILE:
			ret=script_hero_sprite*10000;
			break;
		
		case HEROSCRIPTCSET:
			ret=script_hero_cset*10000;
			break;
		case LINKSCRIPFLIP:
			ret=script_hero_flip*10000;
			break;
			
			
		case LINKITEMB:
			//Hero->setBButtonItem(vbound((value/10000),0,(MAXITEMS-1)));
			ret = Bwpn*10000;
			break;
		
		case LINKITEMA:
			//Hero->setBButtonItem(vbound((value/10000),0,(MAXITEMS-1)));
			ret = Awpn *10000;
			break;
		
		case LINKITEMX:
			//Hero->setBButtonItem(vbound((value/10000),0,(MAXITEMS-1)));
			ret = Xwpn *10000;
			break;
		
		case LINKITEMY:
			//Hero->setBButtonItem(vbound((value/10000),0,(MAXITEMS-1)));
			ret = Ywpn *10000;
			break;
		
		case LINKTILEMOD:
			ret = Hero.getTileModifier() * 10000;
			break;
		
		case LINKDIAG:
			ret=Hero.getDiagMove()?10000:0;
			break;
		
		case LINKBIGHITBOX:
			ret=Hero.getBigHitbox()?10000:0;
			break;
		
		case LINKCLIMBING:
			ret = Hero.getOnSideviewLadder()?10000:0;
			break;
			
		case HEROJUMPCOUNT:
			ret = Hero.extra_jump_count * 10000;
			break;
		
		case HEROPULLDIR:
			ret = Hero.pit_pulldir * 10000;
			break;
		
		case HEROPULLCLK:
			ret = Hero.pit_pullclk * 10000;
			break;
		
		case HEROFALLCLK:
			ret = Hero.fallclk * 10000;
			break;
		
		case HEROFALLCMB:
			ret = Hero.fallCombo * 10000;
			break;
			
		case HERODROWNCLK:
			ret = Hero.drownclk * 10000;
			break;
		
		case HERODROWNCMB:
			ret = Hero.drownCombo * 10000;
			break;
			
		case HEROFAKEZ:
		{
			if (get_qr(qr_SPRITEXY_IS_FLOAT))
			{
				ret = Hero.getFakeZ().getZLong();
			}
			else ret = int32_t(Hero.getFakeZ()) * 10000;

			break;
		} 
		
		case HEROSHIELDJINX:
			ret = Hero.shieldjinxclk * 10000;
			break;
		
		case HEROISWARPING:
			ret = Hero.is_warping ? 10000L : 0L;
			break;
		
		case CLOCKACTIVE:
			ret=watch?10000:0;
			break;
		
		case CLOCKCLK:
			ret=clockclk*10000;
			break;
			
		case HERORESPAWNX:
		{
			ret = Hero.respawn_x.getZLong();
			break;
		}
		
		case HERORESPAWNY:
		{
			ret = Hero.respawn_y.getZLong();
			break;
		}
		
		case HERORESPAWNDMAP:
		{
			ret = Hero.respawn_dmap * 10000;
			break;
		}
		
		case HERORESPAWNSCR:
		{
			ret = Hero.respawn_scr * 10000;
			break;
		}
		
		case HEROSWITCHTIMER:
		{
			ret = Hero.switchhookclk * 10000;
			break;
		}
		
		case HEROSWITCHMAXTIMER:
		{
			ret = Hero.switchhookmaxtime * 10000;
			break;
		}
		
		case HEROIMMORTAL:
		{
			ret = Hero.immortal * 10000;
			break;
		}
		
		case HEROSTANDING:
		{
			ret = Hero.isStanding(true) ? 10000 : 0;
			break;
		}
		
		case HEROCOYOTETIME:
		{
			ret = Hero.coyotetime*10000;
			break;
		}
		
		case HEROLIFTEDWPN:
		{
			ret = Hero.lift_wpn ? Hero.lift_wpn->getUID() : 0;
			break;
		}
		case HEROLIFTTIMER:
		{
			ret = Hero.liftclk * 10000;
			break;
		}
		case HEROLIFTMAXTIMER:
		{
			ret = Hero.tliftclk * 10000;
			break;
		}
		case HEROLIFTHEIGHT:
		{
			ret = Hero.liftheight.getZLong();
			break;
		}
		case HEROHAMMERSTATE:
		{
			ret = Hero.getHammerState() * 10000;
			break;
		}
		case HEROFLICKERCOLOR:
			ret = (int32_t)(Hero.flickercolor) * 10000; break;
		case HEROFLASHINGCSET:
			ret = (int32_t)(Hero.getFlashingCSet()) * 10000; break;
		case HEROFLICKERTRANSP:
			ret = (int32_t)(Hero.flickertransp) * 10000; break;
		
		case HEROSLIDING:
			ret = Hero.sliding*10000; break;
		case HEROICECMB:
			ret = Hero.ice_combo*10000; break;
		case HEROSCRICECMB:
			ret = Hero.script_ice_combo*10000; break;
		case HEROICEVX:
			ret = Hero.ice_vx.getZLong(); break;
		case HEROICEVY:
			ret = Hero.ice_vy.getZLong(); break;
		case HEROICEENTRYFRAMES:
			ret = Hero.ice_entry_count*10000; break;
		case HEROICEENTRYMAXFRAMES:
			ret = Hero.ice_entry_mcount*10000; break;
		
		///----------------------------------------------------------------------------------------------------//
		//Input States
		case INPUTSTART:
			ret=control_state[6]?10000:0;
			break;
			
		case INPUTMAP:
			ret=control_state[9]?10000:0;
			break;
			
		case INPUTUP:
			ret=control_state[0]?10000:0;
			break;
			
		case INPUTDOWN:
			ret=control_state[1]?10000:0;
			break;
			
		case INPUTLEFT:
			ret=control_state[2]?10000:0;
			break;
			
		case INPUTRIGHT:
			ret=control_state[3]?10000:0;
			break;
			
		case INPUTA:
			ret=control_state[4]?10000:0;
			break;
			
		case INPUTB:
			ret=control_state[5]?10000:0;
			break;
			
		case INPUTL:
			ret=control_state[7]?10000:0;
			break;
			
		case INPUTR:
			ret=control_state[8]?10000:0;
			break;
			
		case INPUTEX1:
			ret=control_state[10]?10000:0;
			break;
			
		case INPUTEX2:
			ret=control_state[11]?10000:0;
			break;
			
		case INPUTEX3:
			ret=control_state[12]?10000:0;
			break;
			
		case INPUTEX4:
			ret=control_state[13]?10000:0;
			break;
			
		case INPUTAXISUP:
			ret=control_state[14]?10000:0;
			break;
			
		case INPUTAXISDOWN:
			ret=control_state[15]?10000:0;
			break;
			
		case INPUTAXISLEFT:
			ret=control_state[16]?10000:0;
			break;
			
		case INPUTAXISRIGHT:
			ret=control_state[17]?10000:0;
			break;
			
		case INPUTMOUSEX:
		{
			ret=get_mouse_state(0)*10000;
			break;
		}
		
		case INPUTMOUSEY:
		{
			int32_t mousequakeoffset = 56+((int32_t)(zc::math::Sin((double)(quakeclk*int64_t(2)-frame))*4));
			int32_t tempoffset = (quakeclk > 0) ? mousequakeoffset : (get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
			ret=((get_mouse_state(1)-tempoffset))*10000;
			break;
		}
		
		case INPUTMOUSEZ:
			ret=(get_mouse_state(2))*10000;
			break;
			
		case INPUTMOUSEB:
			ret=(get_mouse_state(3))*10000;
			break;
		
		case INPUTPRESSSTART:
			ret=button_press[6]?10000:0;
			break;
			
		case INPUTPRESSMAP:
			ret=button_press[9]?10000:0;
			break;
			
		case INPUTPRESSUP:
			ret=button_press[0]?10000:0;
			break;
			
		case INPUTPRESSDOWN:
			ret=button_press[1]?10000:0;
			break;
			
		case INPUTPRESSLEFT:
			ret=button_press[2]?10000:0;
			break;
			
		case INPUTPRESSRIGHT:
			ret=button_press[3]?10000:0;
			break;
			
		case INPUTPRESSA:
			ret=button_press[4]?10000:0;
			break;
			
		case INPUTPRESSB:
			ret=button_press[5]?10000:0;
			break;
			
		case INPUTPRESSL:
			ret=button_press[7]?10000:0;
			break;
			
		case INPUTPRESSR:
			ret=button_press[8]?10000:0;
			break;
			
		case INPUTPRESSEX1:
			ret=button_press[10]?10000:0;
			break;
			
		case INPUTPRESSEX2:
			ret=button_press[11]?10000:0;
			break;
			
		case INPUTPRESSEX3:
			ret=button_press[12]?10000:0;
			break;
			
		case INPUTPRESSEX4:
			ret=button_press[13]?10000:0;
			break;
			
		case PRESSAXISUP:
			ret=button_press[14]?10000:0;
			break;
			
		case PRESSAXISDOWN:
			ret=button_press[15]?10000:0;
			break;
			
		case PRESSAXISLEFT:
			ret=button_press[16]?10000:0;
			break;
			
		case PRESSAXISRIGHT:
			ret=button_press[17]?10000:0;
			break;
		
		case KEYMODIFIERS:
		{
			ret = (key_shifts*10000);
			break;
		}
		
		///----------------------------------------------------------------------------------------------------//
		//Itemdata Variables
		
		
		case IDATAUSEWPN:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].weap_data.imitate_weapon)*10000;
			break;
		case IDATAUSEDEF:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].weap_data.default_defense)*10000;
			break;
		case IDATAWRANGE:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].weaprange)*10000;
			break;
		case IDATAMAGICTIMER:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].magiccosttimer[0])*10000;
			break;
		case IDATAMAGICTIMER2:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].magiccosttimer[1])*10000;
			break;
		// Note: never used?
		case IDATAUSEMVT:
		{
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			int32_t a = vbound((ri->d[rINDEX] / 10000),0,(ITEM_MOVEMENT_PATTERNS-1));
			ret=(itemsbuf[ri->idata].weap_pattern[a])*10000;
		}
		break;
		
		case IDATADURATION:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].weapduration)*10000;
			break;
		
		case IDATADUPLICATES:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].duplicates)*10000;
			break;
		case IDATADRAWLAYER:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].drawlayer)*10000;
			break;
		case IDATACOLLECTFLAGS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = 0;
				break;
			}
			ret=(itemsbuf[ri->idata].collectflags)*10000;
			break;
		case IDATAWEAPONSCRIPT:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].weap_data.script)*10000;
			break;
		case IDATAWEAPHXOFS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].weap_data.hxofs)*10000;
			break;
		case IDATAWEAPHYOFS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].weap_data.hyofs)*10000;
			break;
		case IDATAWEAPHXSZ:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].weap_data.hxsz)*10000;
			break;
		case IDATAWEAPHYSZ:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].weap_data.hysz)*10000;
			break;
		case IDATAWEAPHZSZ:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].weap_data.hzsz)*10000;
			break;
		case IDATAWEAPXOFS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].weap_data.xofs)*10000;
			break;
		case IDATAWEAPYOFS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].weap_data.yofs)*10000;
			break;
		case IDATAHXOFS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].hxofs)*10000;
			break;
		case IDATAHYOFS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].hyofs)*10000;
			break;
		case IDATAHXSZ:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].hxsz)*10000;
			break;
		case IDATAHYSZ:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].hysz)*10000;
			break;
		case IDATAHZSZ:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].hzsz)*10000;
			break;
		case IDATADXOFS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].xofs)*10000;
			break;
		case IDATADYOFS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].yofs)*10000;
			break;
		case IDATATILEW:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].tilew)*10000;
			break;
		case IDATATILEH:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].tileh)*10000;
			break;
		case IDATAPICKUP:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].pickup)*10000;
			break;
		case IDATAOVERRIDEFL:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = 0;
				break;
			}
			ret=(itemsbuf[ri->idata].overrideFLAGS)*10000;
			break;

		case IDATATILEWWEAP:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].weap_data.tilew)*10000;
			break;
		case IDATATILEHWEAP:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].weap_data.tileh)*10000;
			break;
		case IDATAOVERRIDEFLWEAP:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = 0;
				break;
			}
			ret=(itemsbuf[ri->idata].weap_data.override_flags)*10000;
			break;
		
		case IDATAFAMILY:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].family)*10000;
			break;
			
		case IDATALEVEL:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].fam_type)*10000;
			break;
			
		case IDATAKEEP:
			ret = item_flag(item_gamedata);
			break;
			
		case IDATAAMOUNT:
		{
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			int32_t v = itemsbuf[ri->idata].amount;
			ret = ((v&0x4000)?-1:1)*(v & 0x3FFF)*10000;
			break;
		}
		case IDATAGRADUAL:
		{
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret = (itemsbuf[ri->idata].amount&0x8000) ? 10000 : 0;
			break;
		}
		case IDATACONSTSCRIPT:
			ret = item_flag(item_passive_script);
			break;
		case IDATASSWIMDISABLED:
			ret = item_flag(item_sideswim_disabled);
			break;
		case IDATABUNNYABLE:
			ret = item_flag(item_bunny_enabled);
			break;
		case IDATAJINXIMMUNE:
			ret = item_flag(item_jinx_immune);
			break;
		case IDATAJINXSWAP:
			ret = item_flag(item_flip_jinx);
			break;
		case IDATAUSEBURNSPR:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = 0;
			}
			else ret = (itemsbuf[ri->idata].weap_data.wflags & WFLAG_UPDATE_IGNITE_SPRITE) ? 10000 : 0;
			break;
			
		case IDATASETMAX:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].setmax)*10000;
			break;
			
		case IDATAMAX:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].max)*10000;
			break;
			
		case IDATACOUNTER:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].count)*10000;
			break;
			
		case IDATAPSOUND:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].playsound)*10000;
			break;
		case IDATAUSESOUND:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].usesound)*10000;
			break;
			
		case IDATAUSESOUND2:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].usesound2)*10000;
			break;
			
		case IDATAPOWER:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].power)*10000;
			break;
		
		//Get the ID of an item.
		case IDATAID:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				//Don't error here //scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=ri->idata*10000;
			break;
		
		//Get the script assigned to an item (active)
		case IDATASCRIPT:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].script)*10000;
			break;
		case IDATASPRSCRIPT:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].sprite_script)*10000;
			break;
		//Hero TIle modifier
		case IDATALTM:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = 0;
				break;
			}
			ret=(itemsbuf[ri->idata].ltm)*10000;
			break;
		//Pickup script
		case IDATAPSCRIPT:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].collect_script)*10000;
			break;
		//Pickup string
		case IDATAPSTRING:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].pstring)*10000;
			break;
		case IDATAPFLAGS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = 0;
				break;
			}
			ret = (itemsbuf[ri->idata].pickup_string_flags)*10000;
			break;
		case IDATAPICKUPLITEMS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = 0;
				break;
			}
			ret = (itemsbuf[ri->idata].pickup_litems)*10000;
			break;
		case IDATAPICKUPLITEMLEVEL:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = 0;
				break;
			}
			ret = (itemsbuf[ri->idata].pickup_litem_level)*10000;
			break;
		//Magic cost
		case IDATAMAGCOST:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].cost_amount[0])*10000;
			break;
		case IDATACOST2:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].cost_amount[1])*10000;
			break;
		//cost counter ref
		case IDATACOSTCOUNTER:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].cost_counter[0])*10000;
			break;
		case IDATACOSTCOUNTER2:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].cost_counter[1])*10000;
			break;
		//Min Hearts to Pick Up
		case IDATAMINHEARTS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].pickup_hearts)*10000;
			break;
		//Tile used by the item
		case IDATATILE:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].tile)*10000;
			break;
		//itemdata->Flash
		case IDATAMISC:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].misc_flags)*10000;
			break;
		//->CSet
		case IDATACSET:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}

			ret = (itemsbuf[ri->idata].csets&15)*10000;

			// If we find quests that broke, use this code.
			// if (QHeader.compareVer(2, 55, 9) >= 0)
			// 	ret = (itemsbuf[ri->idata].csets&15)*10000;
			// else
			// 	ret = itemsbuf[ri->idata].csets*10000;
			break;
		case IDATAFLASHCSET:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].csets>>4)*10000;
			break;
		//->A.Frames
		case IDATAFRAMES:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].frames)*10000;
			break;
		/*
		case IDATAFRAME:
			ret=(itemsbuf[ri->idata].frame)*10000;
			break;
		*/ 
		//->A.Speed
		case IDATAASPEED:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].speed)*10000;
			break;
		//->Delay
		case IDATADELAY:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				ret = -10000;
				break;
			}
			ret=(itemsbuf[ri->idata].delay)*10000;
			break;
		// teo of this item upgrades
		case IDATACOMBINE:
			ret = item_flag(item_combine);
			break;
		//Use item, and get the lower level one
		case IDATADOWNGRADE:
			ret = item_flag(item_downgrade);
			break;
		//Only validate the cost, don't charge it
		case IDATAVALIDATE:
			ret = item_flag(item_validate_only);
			break;
		case IDATAVALIDATE2:
			ret = item_flag(item_validate_only_2);
			break;
		//->Keep Old
		case IDATAKEEPOLD:
			ret = item_flag(item_keep_old);
			break;
		//Use rupees instead of magic
		case IDATARUPEECOST:
			ret = item_flag(item_rupee_magic);
			break;
		//Can be eaten
		case IDATAEDIBLE:
			ret = item_flag(item_edible);
			break;
		//currently unused
		case IDATAFLAGUNUSED:
			ret = item_flag(item_unused);
			break;
		//Gain lower level items when collected
		case IDATAGAINLOWER:
			ret = item_flag(item_gain_old);
			break;

		///----------------------------------------------------------------------------------------------------//
		//LWeapon Variables
		case LWPNSPECIAL:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((int32_t)((weapon*)(s))->specialinfo)*10000;
			
				
			break;
			
		case LWPNSCALE:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				ret = -1; break;
			}
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((int32_t)((weapon*)(s))->scale)*100.0;
				
			break;
		
		case LWPNX:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret=(((weapon*)(s))->x).getZLong();  
				}
				else 
					ret=((int32_t)((weapon*)(s))->x)*10000;
			}
				
			break;
		
		case SPRITEMAXLWPN:
		{
			//No bounds check, as this is a universal function and works from NULL pointers!
			ret = Lwpns.getMax() * 10000;
			break;
		}
	
		case LWPNY:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret=(((weapon*)(s))->y).getZLong();  
				}
				else 
					ret=((int32_t)((weapon*)(s))->y)*10000;
			}
			break;
			
		case LWPNZ:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret=(((weapon*)(s))->z).getZLong();  
				}
				else 
					ret=((int32_t)((weapon*)(s))->z)*10000;
			}
				
			break;
			
		case LWPNJUMP:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->fall.getZLong() / -100;
				if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED)) ret = trunc(ret / 10000) * 10000;
			}
				
			break;
		
		case LWPNFAKEJUMP:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->fakefall.getZLong() / -100;
				if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED)) ret = trunc(ret / 10000) * 10000;
			}
				
			break;
			
		case LWPNDIR:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->dir*10000;
				
			break;
		 
		case LWPNGRAVITY:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret= (((weapon*)(s))->moveflags & move_obeys_grav) ? 10000 : 0;
				
			break;
			
		case LWPNSTEP:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				if ( get_qr(qr_STEP_IS_FLOAT) || replay_is_active() )
				{
					ret=((weapon*)s)->step.getZLong() * 100;
				}
				//old, buggy code replication, round two: Go! -Z
				//else ret = ( ( ( ((weapon*)s)->step ) * 100.0 ).getZLong() );
				
				//else 
				//{
					//old, buggy code replication, round THREE: Go! -Z
				//	double tmp = ( ((weapon*)s)->step.getFloat() ) * 1000000.0;
				//	ret = (int32_t)tmp;
				//}
				
				//old, buggy code replication, round FOUR: Go! -Z
				else ret = (int32_t)((float)((weapon*)s)->step * 1000000.0);
			}
			break;
			
		case LWPNANGLE:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=(int32_t)(((weapon*)(s))->angle*10000);
				
			break;
		
		case LWPNDEGANGLE:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret=(int32_t)(((weapon*)(s))->angle*(180.0 / PI)*10000);
			}
				
			break;
			
		case LWPNVX:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				if (((weapon*)(s))->angular)
					ret = int32_t(zc::math::Cos(((weapon*)s)->angle)*10000.0*((weapon*)s)->step);
				else
				{
					switch(NORMAL_DIR(((weapon*)(s))->dir))
					{
						case l_up:
						case l_down:
						case left:
							ret = int32_t(-10000.0*((weapon*)s)->step);
							break;
							
						case r_down:
						case r_up:
						case right:
							ret = int32_t(10000.0*((weapon*)s)->step);
							break;
						
						default:
							ret = 0;
							break;
					}
				}
			}
				
			break;
		
		case LWPNVY:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				if (((weapon*)(s))->angular)
					ret = int32_t(zc::math::Sin(((weapon*)s)->angle)*10000.0*((weapon*)s)->step);
				else
				{
					switch(NORMAL_DIR(((weapon*)(s))->dir))
					{
						case l_up:
						case r_up:
						case up:
							ret = int32_t(-10000.0*((weapon*)s)->step);
							break;
						case l_down:
						case r_down:
						case down:
							ret = int32_t(10000.0*((weapon*)s)->step);
							break;
							
						default:
							ret = 0;
							break;
					}
				}
			}
				
			break;
				
		case LWPNANGULAR:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->angular*10000;
				
			break;
			
		case LWPNAUTOROTATE:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->autorotate*10000;
				
			break;
			
		case LWPNBEHIND:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->behind*10000;
				
			break;
			
		case LWPNDRAWTYPE:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->drawstyle*10000;
				
			break;
			
		case LWPNPOWER:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->power*10000;
				
			break;
		/*
		case LWPNRANGE:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->scriptrange*10000;
				
			break;
		*/        
		case LWPNDEAD:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->dead*10000;
				
			break;
			
		case LWPNID:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->id*10000;
				
			break;
			
		case LWPNTILE:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->tile*10000;
				
			break;
		
		case LWPNSCRIPTTILE:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->scripttile*10000;
				
			break;
		
		case LWPNSCRIPTFLIP:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->scriptflip*10000;
				
			break;
			
		case LWPNCSET:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->cs*10000;
				
			break;
			
		case LWPNFLASHCSET:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=(((weapon*)(s))->o_cset>>4)*10000;
				
			break;
			
		case LWPNFRAMES:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->frames*10000;
				
			break;
			
		case LWPNFRAME:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->aframe*10000;
				
			break;
			
		case LWPNASPEED:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->o_speed*10000;
				
			break;
			
		case LWPNFLASH:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->flash*10000;
				
			break;
			
		case LWPNFLIP:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->flip*10000;
				
			break;
			
		case LWPNCOUNT:
			ret=Lwpns.Count()*10000;
			break;
			
		case LWPNEXTEND:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->extend*10000;
				
			break;
			
		case LWPNOTILE:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->o_tile*10000;
				
			break;
			
		case LWPNOCSET:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=(((weapon*)(s))->o_cset&15)*10000;
				
			break;
			
		case LWPNHXOFS:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=(((weapon*)(s))->hxofs)*10000;
				
			break;
			
		case LWPNHYOFS:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=(((weapon*)(s))->hyofs)*10000;
				
			break;
			
		case LWPNXOFS:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((int32_t)(((weapon*)(s))->xofs))*10000;
				
			break;
			
		case LWPNYOFS:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((int32_t)(((weapon*)(s))->yofs-(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)))*10000;
				
			break;
			
		case LWPNSHADOWXOFS:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((int32_t)(((weapon*)(s))->shadowxofs))*10000;
				
			break;
			
		case LWPNSHADOWYOFS:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((int32_t)(((weapon*)(s))->shadowyofs))*10000;
				
			break;
			
		case LWPNTOTALDYOFFS:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret = ((int32_t)(((weapon*)(s))->yofs-(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset))
					+ ((((weapon*)(s))->switch_hooked && Hero.switchhookstyle == swRISE)
						? -(8-(abs(Hero.switchhookclk-32)/4)) : 0)) * 10000;
			break;
			
		case LWPNZOFS:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((int32_t)(((weapon*)(s))->zofs))*10000;
				
			break;
			
		case LWPNHXSZ:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=(((weapon*)(s))->hit_width)*10000;
				
			break;
			
		case LWPNHYSZ:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=(((weapon*)(s))->hit_height)*10000;
				
			break;
			
		case LWPNHZSZ:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=(((weapon*)(s))->hzsz)*10000;
				
			break;
			
		case LWPNTXSZ:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=(((weapon*)(s))->txsz)*10000;
				
			break;
			
		case LWPNTYSZ:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=(((weapon*)(s))->tysz)*10000;
				
			break;
			
		case LWPNCOLLDET:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=(((weapon*)(s))->scriptcoldet)*10000;
				
			break;
		
		case LWPNENGINEANIMATE:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=(((weapon*)(s))->do_animation)*10000;
				
			break;
		
		case LWPNPARENT:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=(((weapon*)(s))->parentitem)*10000;
				
			break;

		case LWPNLEVEL:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=(((weapon*)(s))->type)*10000;
				
			break;
		
		case LWPNSCRIPT:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=(((weapon*)(s))->script)*10000;
				
			break;
		
		case LWPNUSEWEAPON:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=(((weapon*)(s))->useweapon)*10000;
				
			break;
		
		case LWPNUSEDEFENCE:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=(((weapon*)(s))->usedefense)*10000;
				
			break;
		
		case LWEAPONSCRIPTUID:
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=(((weapon*)(s))->getUID());
				
			break;

		case LWPNROTATION:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				ret = -1; break;
			}
			if(0!=(s=checkLWpn(ri->lwpn)))
				ret=((weapon*)(s))->rotation*10000;
				
			break;
		
		case LWPNFALLCLK:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->fallclk * 10000;
			}
			break;
		
		case LWPNFALLCMB:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->fallCombo * 10000;
			}
			break;
		
		case LWPNDROWNCLK:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->drownclk * 10000;
			}
			break;
		
		case LWPNDROWNCMB:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->drownCombo * 10000;
			}
			break;
			
		case LWPNFAKEZ:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret=(((weapon*)(s))->fakez).getZLong();  
				}
				else 
					ret=((int32_t)((weapon*)(s))->fakez)*10000;
			}
			break;

		case LWPNGLOWRAD:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->glowRad * 10000;
			}
			break;
			
		case LWPNGLOWSHP:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->glowShape * 10000;
			}
			break;
			
		case LWPNUNBL:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->unblockable * 10000;
			}
			break;
			
		case LWPNSHADOWSPR:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->spr_shadow * 10000;
			}
			break;
		case LWSWHOOKED:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = s->switch_hooked ? 10000 : 0;
			}
			break;
		case LWPNTIMEOUT:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->weap_timeout * 10000;
			}
			break;
		case LWPNDEATHITEM:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->death_spawnitem * 10000;
			}
			break;
		case LWPNDEATHDROPSET:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->death_spawndropset * 10000;
			}
			break;
		case LWPNDEATHIPICKUP:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->death_item_pflags * 10000;
			}
			break;
		case LWPNDEATHSPRITE:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->death_sprite * 10000;
			}
			break;
		case LWPNDEATHSFX:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->death_sfx * 10000;
			}
			break;
		case LWPNLIFTLEVEL:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->lift_level * 10000;
			}
			break;
		case LWPNLIFTTIME:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->lift_time * 10000;
			}
			break;
		case LWPNLIFTHEIGHT:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				ret = ((weapon*)(s))->lift_height.getZLong();
			}
			break;
			
		///----------------------------------------------------------------------------------------------------//
		//EWeapon Variables
		case EWPNSCALE:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				ret = -1; break;
			}
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((int32_t)((weapon*)(s))->scale)*100.0;
				
			break;

		case EWPNX:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret=(((weapon*)(s))->x).getZLong();
				}
				else 
					ret=((int32_t)((weapon*)(s))->x)*10000;
			}
			break;
			
		case SPRITEMAXEWPN:
		{
			//No bounds check, as this is a universal function and works from NULL pointers!
			ret = Ewpns.getMax() * 10000;
			break;
		}
	
		case EWPNY:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret=(((weapon*)(s))->y).getZLong();
				}
				else 
					 ret=((int32_t)((weapon*)(s))->y)*10000;
			}
			break;
			
		case EWPNZ:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret=(((weapon*)(s))->z).getZLong();
				}
				else 
					ret=((int32_t)((weapon*)(s))->z)*10000;
			}
			break;
			
		case EWPNJUMP:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->fall.getZLong() / -100;
				if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED)) ret = trunc(ret / 10000) * 10000;
			}
				
			break;
		
		case EWPNFAKEJUMP:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->fakefall.getZLong() / -100;
				if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED)) ret = trunc(ret / 10000) * 10000;
			}
				
			break;
			
		case EWPNDIR:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->dir*10000;
				
			break;
			
		case EWPNLEVEL:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->type*10000;
				
			break;
			
		case EWPNGRAVITY:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((((weapon*)(s))->moveflags & move_obeys_grav) ? 10000 : 0);
				
			break;
			
		case EWPNSTEP:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				if ( get_qr(qr_STEP_IS_FLOAT) || replay_is_active() )
				{
					ret=((weapon*)s)->step.getZLong() * 100;
				}
				//old, buggy code replication, round two: Go! -Z
				//else ret = ( ( ( ((weapon*)s)->step ) * 100.0 ).getZLong() );
				//old, buggy code replication, round FOUR: Go! -Z
				else ret = (int32_t)((float)((weapon*)s)->step * 1000000.0);
			}
			//else 
			//{
				//old, buggy code replication, round THREE: Go! -Z
			//	double tmp = ( ((weapon*)s)->step.getFloat() ) * 1000000.0;
			//	ret = int32_t(tmp);
			//}
			break;
			
		case EWPNANGLE:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=(int32_t)(((weapon*)(s))->angle*10000);
				
			break;
			
		case EWPNDEGANGLE:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret=(int32_t)(((weapon*)(s))->angle*(180.0 / PI)*10000);
			}
				
			break;
			
		case EWPNVX:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				if (((weapon*)(s))->angular)
					ret = int32_t(zc::math::Cos(((weapon*)s)->angle)*10000.0*((weapon*)s)->step);
				else
				{
					switch(NORMAL_DIR(((weapon*)(s))->dir))
					{
						case l_up:
						case l_down:
						case left:
							ret = int32_t(-10000.0*((weapon*)s)->step);
							break;
						case r_up:
						case r_down:
						case right:
							ret = int32_t(10000.0*((weapon*)s)->step);
							break;
							
						default:
							ret = 0;
							break;
					}
				}
			}
				
			break;
		
		case EWPNVY:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				if (((weapon*)(s))->angular)
					ret = int32_t(zc::math::Sin(((weapon*)s)->angle)*10000.0*((weapon*)s)->step);
				else
				{
					switch(NORMAL_DIR(((weapon*)(s))->dir))
					{
						case l_up:
						case r_up:
						case up:
							ret = int32_t(-10000.0*((weapon*)s)->step);
							break;
						case l_down:
						case r_down:
						case down:
							ret = int32_t(10000.0*((weapon*)s)->step);
							break;
							
						default:
							ret = 0;
							break;
					}
				}
			}
				
			break;
			
			
		case EWPNANGULAR:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->angular*10000;
				
			break;
			
		case EWPNAUTOROTATE:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->autorotate*10000;
				
			break;
			
		case EWPNBEHIND:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->behind*10000;
				
			break;
			
		case EWPNDRAWTYPE:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->drawstyle*10000;
				
			break;
			
		case EWPNPOWER:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->power*10000;
				
			break;
			
		case EWPNDEAD:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->dead*10000;
				
			break;
			
		case EWPNID:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->id*10000;
				
			break;
			
		case EWPNTILE:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->tile*10000;
				
			break;
		
		case EWPNSCRIPTTILE:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->scripttile*10000;
				
			break;
		
		case EWPNSCRIPTFLIP:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->scriptflip*10000;
				
			break;
			
		case EWPNCSET:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->cs*10000;
				
			break;
			
		case EWPNFLASHCSET:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=(((weapon*)(s))->o_cset>>4)*10000;
				
			break;
			
		case EWPNFRAMES:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->frames*10000;
				
			break;
			
		case EWPNFRAME:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->aframe*10000;
				
			break;
			
		case EWPNASPEED:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->o_speed*10000;
				
			break;
			
		case EWPNFLASH:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->flash*10000;
				
			break;
			
		case EWPNFLIP:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->flip*10000;
				
			break;

		case EWPNROTATION:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'");
				break;
			}
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->rotation*10000;
				
			break;

		case EWPNCOUNT:
			ret=Ewpns.Count()*10000;
			break;
			
		case EWPNEXTEND:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->extend*10000;
				
			break;
			
		case EWPNOTILE:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((weapon*)(s))->o_tile*10000;
				
			break;
			
		case EWPNOCSET:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=(((weapon*)(s))->o_cset&15)*10000;
				
			break;
			
		case EWPNHXOFS:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=(((weapon*)(s))->hxofs)*10000;
				
			break;
			
		case EWPNHYOFS:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=(((weapon*)(s))->hyofs)*10000;
				
			break;
			
		case EWPNXOFS:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((int32_t)(((weapon*)(s))->xofs))*10000;
				
			break;
			
		case EWPNYOFS:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((int32_t)(((weapon*)(s))->yofs-(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)))*10000;
				
			break;
			
		case EWPNSHADOWXOFS:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((int32_t)(((weapon*)(s))->shadowxofs))*10000;
				
			break;
			
		case EWPNSHADOWYOFS:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((int32_t)(((weapon*)(s))->shadowyofs))*10000;
				
			break;
		case EWPNTOTALDYOFFS:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret = ((int32_t)(((weapon*)(s))->yofs-(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset))
					+ ((((weapon*)(s))->switch_hooked && Hero.switchhookstyle == swRISE)
						? -(8-(abs(Hero.switchhookclk-32)/4)) : 0) * 10000);
			break;
			
		case EWPNZOFS:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=((int32_t)(((weapon*)(s))->zofs))*10000;
				
			break;
			
		case EWPNHXSZ:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=(((weapon*)(s))->hit_width)*10000;
				
			break;
			
		case EWPNHYSZ:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=(((weapon*)(s))->hit_height)*10000;
				
			break;
			
		case EWPNHZSZ:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=(((weapon*)(s))->hzsz)*10000;
				
			break;
			
		case EWPNTXSZ:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=(((weapon*)(s))->txsz)*10000;
				
			break;
			
		case EWPNTYSZ:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=(((weapon*)(s))->tysz)*10000;
				
			break;
			
		case EWPNCOLLDET:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=(((weapon*)(s))->scriptcoldet)*10000;
				
			break;
		
		case EWPNENGINEANIMATE:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=(((weapon*)(s))->do_animation)*10000;
				
			break;
		
		case EWPNPARENT:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret= ((get_qr(qr_OLDEWPNPARENT)) ? (((weapon*)(s))->parentid)*10000 : (((weapon*)(s))->parentid));
		
			break;
		
		case EWEAPONSCRIPTUID:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=(((weapon*)(s))->getUID());
				
			break;
		
		case EWPNPARENTUID:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret = s->parent ? s->parent->getUID() : 0;
				
			break;
		
		case EWPNSCRIPT:
			if(0!=(s=checkEWpn(ri->ewpn)))
				ret=(((weapon*)(s))->script)*10000;
				
			break;
		
		case EWPNFALLCLK:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->fallclk * 10000;
			}
			break;
		
		case EWPNFALLCMB:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->fallCombo * 10000;
			}
			break;
		
		case EWPNDROWNCLK:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->drownclk * 10000;
			}
			break;
		
		case EWPNDROWNCMB:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->drownCombo * 10000;
			}
			break;
		case EWPNFAKEZ:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret=(((weapon*)(s))->fakez).getZLong();
				}
				else 
					ret=((int32_t)((weapon*)(s))->fakez)*10000;
			}
			break;
		
		case EWPNGLOWRAD:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->glowRad * 10000;
			}
			break;
			
		case EWPNGLOWSHP:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->glowShape * 10000;
			}
			break;
			
		case EWPNUNBL:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->unblockable * 10000;
			}
			break;
			
		case EWPNSHADOWSPR:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->spr_shadow * 10000;
			}
			break;
		case EWSWHOOKED:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = s->switch_hooked ? 10000 : 0;
			}
			break;
		case EWPNTIMEOUT:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->weap_timeout * 10000;
			}
			break;
		case EWPNDEATHITEM:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->death_spawnitem * 10000;
			}
			break;
		case EWPNDEATHDROPSET:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->death_spawndropset * 10000;
			}
			break;
		case EWPNDEATHIPICKUP:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->death_item_pflags * 10000;
			}
			break;
		case EWPNDEATHSPRITE:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->death_sprite * 10000;
			}
			break;
		case EWPNDEATHSFX:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->death_sfx * 10000;
			}
			break;
		case EWPNLIFTLEVEL:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->lift_level * 10000;
			}
			break;
		case EWPNLIFTTIME:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->lift_time * 10000;
			}
			break;
		case EWPNLIFTHEIGHT:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				ret = ((weapon*)(s))->lift_height.getZLong();
			}
			break;
		
		case GETPIXEL:
			ret=FFCore.do_getpixel();
			break;

		case SCREENSCRDATASIZE:
		{
			int index = map_screen_index(cur_map, ri->screenref);
			if (index < 0) break;
			ret = 10000*game->scriptDataSize(index);
			break;
		}
		
		case DISTANCE: 
		{
			double x1 = double(ri->d[rSFTEMP] / 10000.0);
			double y1 = double(ri->d[rINDEX] / 10000.0);
			double x2 = double(ri->d[rINDEX2] / 10000.0);
			double y2 = double(ri->d[rEXP1] / 10000.0);
			
			
			
			int32_t result = FFCore.Distance(x1, y1, x2, y2);
			ret = (result);
		
			break;
		}
		case LONGDISTANCE: 
		{
			double x1 = double(ri->d[rSFTEMP]);
			double y1 = double(ri->d[rINDEX]);
			double x2 = double(ri->d[rINDEX2]);
			double y2 = double(ri->d[rEXP1]);
			
			
			
			int32_t result = FFCore.LongDistance(x1, y1, x2, y2);
			ret = (result);
		
			break;
		}
		
		case DISTANCESCALE: 
		{
			double x1 = (double)(ri->d[rSFTEMP] / 10000.0);
			double y1 = (double)(ri->d[rINDEX] / 10000.0);
			double x2 = (double)(ri->d[rINDEX2] / 10000.0);
			double y2 = (double)(ri->d[rEXP1] / 10000.0);
			
			int32_t scale = (ri->d[rWHAT_NO_7]/10000);
			
			if ( !scale ) scale = 10000;
			int32_t result = FFCore.Distance(x1, y1, x2, y2, scale);
			ret = (result);
			
			break;
		}
		case LONGDISTANCESCALE: 
		{
			double x1 = (double)(ri->d[rSFTEMP]);
			double y1 = (double)(ri->d[rINDEX]);
			double x2 = (double)(ri->d[rINDEX2]);
			double y2 = (double)(ri->d[rEXP1]);
			
			int32_t scale = (ri->d[rWHAT_NO_7]);
			
			if ( !scale ) scale = 1;
			int32_t result = FFCore.LongDistance(x1, y1, x2, y2, scale);
			ret = (result);
			
			break;
		}

		// Note: never used?
		case GAMEGUYCOUNTD:
		{
			int mi = mapind(cur_map, ri->d[rINDEX] / 10000);
			ret = game->guys[mi] * 10000;
			break;
		}

		case ALLOCATEBITMAPR:
			ret=FFCore.get_free_bitmap();
			break;
			
		case GETMIDI:
			ret=(currmidi-(ZC_MIDI_COUNT-1))*10000;
			break;
			
		case CURDSCR:
		{
			int32_t di = (get_currscr()-DMaps[get_currdmap()].xoff);
			ret=(DMaps[get_currdmap()].type==dmOVERW ? cur_screen : di)*10000;
		}
		break;
		
		case GAMEMAXMAPS:
			ret = (map_count)*10000;
			break;
		case GAMENUMMESSAGES:
			ret = (msg_count-1) * 10000; 
			break;
		
		case CURDMAP:
			ret=cur_dmap*10000;
			break;
			
		case CURLEVEL:
			ret=DMaps[get_currdmap()].level*10000;
			break;
			
		case GAMECLICKFREEZE:
			ret=disableClickToFreeze?0:10000;
			break;
			
		
		case NOACTIVESUBSC:
			ret=Hero.stopSubscreenFalling()?10000:0;
			break;///----------------------------------------------------------------------------------------------------//
	//BottleTypes
		
		case BOTTLENEXT:
		{
			if(bottletype* ptr = checkBottleData(ri->bottletyperef))
			{
				ret = 10000L * ptr->next_type;
			}
			else ret = -10000L;
		}
		break;

		///----------------------------------------------------------------------------------------------------//
		//Region

		case REGION_WIDTH:
		{
			ret = world_w * 10000;
		}
		break;

		case REGION_HEIGHT:
		{
			ret = world_h * 10000;
		}
		break;

		case REGION_SCREEN_WIDTH:
		{
			ret = cur_region.screen_width * 10000;
		}
		break;

		case REGION_SCREEN_HEIGHT:
		{
			ret = cur_region.screen_height * 10000;
		}
		break;

		case REGION_NUM_COMBOS:
		{
			ret = region_num_rpos * 10000;
		}
		break;

		case REGION_ID:
		{
			ret = get_current_region_id() * 10000;
		}
		break;

		case REGION_ORIGIN_SCREEN:
		{
			ret = cur_screen;
		}
		break;

		///----------------------------------------------------------------------------------------------------//
		//Viewport

		case VIEWPORT_TARGET:
		{
			ret = get_viewport_sprite()->uid;
		}
		break;

		case VIEWPORT_MODE:
		{
			ret = (int)viewport_mode;
		}
		break;

		case VIEWPORT_X:
		{
			ret = viewport.x * 10000;
		}
		break;

		case VIEWPORT_Y:
		{
			ret = viewport.y * 10000;
		}
		break;

		case VIEWPORT_WIDTH:
		{
			ret = viewport.w * 10000;
		}
		break;

		case VIEWPORT_HEIGHT:
		{
			ret = viewport.h * 10000;
		}
		break;
		
		///----------------------------------------------------------------------------------------------------//
		//Screen Information
		
			#define	GET_SCREENDATA_VAR_INT32(member) \
		{ \
			ret = (get_scr(ri->screenref)->member *10000); \
		} \

		#define	GET_SCREENDATA_VAR_INT16(member) \
		{ \
			ret = (get_scr(ri->screenref)->member *10000); \
		} \

		#define	GET_SCREENDATA_VAR_BYTE(member) \
		{ \
			ret = (get_scr(ri->screenref)->member *10000); \
		} \
		
		#define GET_SCREENDATA_BYTE_INDEX(member, indexbound) \
		{ \
			int32_t indx = ri->d[rINDEX] / 10000; \
			ret = (get_scr(ri->screenref)->member[indx] *10000); \
		} \
		
		//byte
		#define GET_SCREENDATA_LAYER_INDEX(member, indexbound) \
		{ \
			int32_t indx = ri->d[rINDEX] / 10000; \
			if (BC::checkIndex(indx, 1, indexbound) != SH::_NoError) \
			{ \
				ret = -10000; \
			} \
			else \
			{ \
				ret = (get_scr(ri->screenref)->member[indx-1] *10000); \
			} \
		} \
		
		#define GET_SCREENDATA_BOOL_INDEX(member, indexbound) \
		{ \
			int32_t indx = ri->d[rINDEX] / 10000; \
			if (BC::checkIndex(indx, 0, indexbound) != SH::_NoError) \
			{ \
				ret = -10000; \
			} \
			else \
			{ \
				ret = (get_scr(ri->screenref)->member[indx]?10000:0); \
			} \
		} \
		
		
		#define GET_SCREENDATA_FLAG(member, str, indexbound) \
		{ \
			int32_t flag =  (value/10000);  \
			ret = (get_scr(ri->screenref)->member&flag) ? 10000 : 0); \
		} \
		
		case SCREENDATAVALID:		GET_SCREENDATA_VAR_BYTE(valid); break;		//b
		case SCREENDATAGUY: 		GET_SCREENDATA_VAR_BYTE(guy); break;		//b
		case SCREENDATASTRING:		GET_SCREENDATA_VAR_INT32(str); break;		//w
		case SCREENDATAROOM: 		GET_SCREENDATA_VAR_BYTE(room);	break;		//b
		case SCREENDATAITEM:
		{
			mapscr* scr = get_scr(ri->screenref);
			if(scr->hasitem)
				ret = (scr->item *10000);
			else ret = -10000;
			break;
		}
		case SCREENDATAHASITEM: 		GET_SCREENDATA_VAR_BYTE(hasitem); break;	//b
		case SCREENDATADOORCOMBOSET: 	GET_SCREENDATA_VAR_INT32(door_combo_set); break;	//w
		case SCREENDATAWARPRETURNC: 	GET_SCREENDATA_VAR_INT32(warpreturnc); break;	//w
		case SCREENDATASTAIRX: 		GET_SCREENDATA_VAR_BYTE(stairx); break;	//b
		case SCREENDATASTAIRY: 		GET_SCREENDATA_VAR_BYTE(stairy); break;	//b
		case SCREENDATAITEMX:		GET_SCREENDATA_VAR_BYTE(itemx); break; //itemx
		case SCREENDATAITEMY:		GET_SCREENDATA_VAR_BYTE(itemy); break;	//itemy
		case SCREENDATACOLOUR: 		GET_SCREENDATA_VAR_INT32(color); break;	//w
		case SCREENDATAENEMYFLAGS: 	GET_SCREENDATA_VAR_BYTE(flags11);	break;	//b
		// Note: never used?
		case SCREENDATADOOR: 		GET_SCREENDATA_BYTE_INDEX(door, 3); break;	//b, 4 of these
		case SCREENDATAEXITDIR: 		GET_SCREENDATA_VAR_BYTE(exitdir); break;	//b
		case SCREENDATAPATTERN: 		GET_SCREENDATA_VAR_BYTE(pattern); break;	//b
		case SCREENDATAWARPARRIVALX: 	GET_SCREENDATA_VAR_BYTE(warparrivalx); break;	//b
		case SCREENDATAWARPARRIVALY: 	GET_SCREENDATA_VAR_BYTE(warparrivaly); break;	//b
		case SCREENDATASIDEWARPINDEX: 	GET_SCREENDATA_VAR_BYTE(sidewarpindex); break;	//b
		case SCREENDATAUNDERCOMBO: 	GET_SCREENDATA_VAR_INT32(undercombo); break;	//w
		case SCREENDATAUNDERCSET:	 	GET_SCREENDATA_VAR_BYTE(undercset); break; //b
		case SCREENDATACATCHALL:	 	GET_SCREENDATA_VAR_INT32(catchall); break; //W

		case SCREENDATACSENSITIVE: 	GET_SCREENDATA_VAR_BYTE(csensitive); break;	//B
		case SCREENDATANORESET: 		GET_SCREENDATA_VAR_INT32(noreset); break;	//W
		case SCREENDATANOCARRY: 		GET_SCREENDATA_VAR_INT32(nocarry); break;	//W
		case SCREENDATATIMEDWARPTICS: 	GET_SCREENDATA_VAR_INT32(timedwarptics); break;	//W
		case SCREENDATANEXTMAP: 		GET_SCREENDATA_VAR_BYTE(nextmap); break;	//B
		case SCREENDATANEXTSCREEN: 	GET_SCREENDATA_VAR_BYTE(nextscr); break;	//B
		case SCREENDATAVIEWX: 		break;//GET_SCREENDATA_VAR_INT32(viewX, "ViewX"); break;	//W
		case SCREENDATAVIEWY: 		break;//GET_SCREENDATA_VAR_INT32(viewY, "ViewY"); break; //W
		case SCREENDATASCREENWIDTH: 	break;//GET_SCREENDATA_VAR_BYTE(scrWidth, "Width"); break;	//B
		case SCREENDATASCREENHEIGHT: 	break;//GET_SCREENDATA_VAR_BYTE(scrHeight,	"Height"); break;	//B
		case SCREENDATAENTRYX: 		GET_SCREENDATA_VAR_BYTE(entry_x); break;	//B
		case SCREENDATAENTRYY: 		GET_SCREENDATA_VAR_BYTE(entry_y); break;	//B
		//Number of ffcs that are in use (have valid data
		// Note: that is totally not what its doing.
		case SCREENDATANUMFF: 	
		{
			int id = ri->d[rINDEX] / 10000;
			if (auto ffc = ResolveFFCWithID(id))
				ret = ffc->data != 0 ? 10000 : 0;
			break;
		}

		// Note: never used?
		case SCREENDATAFFINITIALISED: 	{
			int32_t indx = ri->d[rINDEX] / 10000;
			if (indx < 0 || indx > MAX_FFCID)
			{
				scripting_log_error_with_context("Invalid index: %d", (indx));
				ret = -10000;
			}
			else
			{
				ret = get_script_engine_data(ScriptType::FFC, indx).initialized ? 10000 : 0;
			}
		}
		break;

		case SCREENDATASCRIPTENTRY:
		{
			Z_scripterrlog("Unimplemented: %s\n", "ScriptEntry");
			ret = -10000;
		}
		break;
		case SCREENDATASCRIPTOCCUPANCY:
		{
			Z_scripterrlog("Unimplemented: %s\n", "ScriptOccupancy");
			ret = -10000;
		}
		break;
		case SCREENDATASCRIPTEXIT:
		{
			Z_scripterrlog("Unimplemented: %s\n", "ExitScript");
			ret = -10000;
		}
		break;

		case SCREENDATAOCEANSFX:	 	GET_SCREENDATA_VAR_BYTE(oceansfx); break;	//B
		case SCREENDATABOSSSFX: 		GET_SCREENDATA_VAR_BYTE(bosssfx); break;	//B
		case SCREENDATASECRETSFX:	 	GET_SCREENDATA_VAR_BYTE(secretsfx); break;	//B
		case SCREENDATAHOLDUPSFX:	 	GET_SCREENDATA_VAR_BYTE(holdupsfx); break; //B
		case SCREENDATASCREENMIDI:
		{
			ret = ((get_scr(ri->screenref)->screen_midi+(MIDIOFFSET_MAPSCR-MIDIOFFSET_ZSCRIPT)) *10000);
			break;
		}
		case SCREENDATALENSLAYER:	 	GET_SCREENDATA_VAR_BYTE(lens_layer); break;	//B, OLD QUESTS ONLY?

		case SCREENSECRETSTRIGGERED:
		{
			ret = get_screen_state(ri->screenref).triggered_secrets ? 10000L : 0L;
			break;
		}
		
		case SCREENDATAGUYCOUNT:
		{
			int mi = mapind(cur_map, ri->screenref);
			if(mi < 0)
				ret = -10000;
			else ret = game->guys[mi] * 10000;
			break;
		}
		case SCREENDATAEXDOOR:
		{
			ret = 0;
			int mi = mapind(cur_map, ri->screenref);
			if(mi < 0) break;
			int dir = SH::read_stack(ri->sp+1) / 10000;
			int ind = SH::read_stack(ri->sp+0) / 10000;
			if(unsigned(dir) > 3)
				Z_scripterrlog("Invalid dir '%d' passed to 'Screen->GetExDoor()'; must be 0-3\n", dir);
			else if(unsigned(ind) > 7)
				Z_scripterrlog("Invalid index '%d' passed to 'Screen->GetExDoor()'; must be 0-7\n", ind);
			else
			{
				int bit = 1<<ind;
				ret = (game->xdoors[mi][dir]&bit) ? 10000 : 0;
			}
			break;
		}
		
		case SHOWNMSG:
		{
			ret = ((msg_active || msg_onscreen) ? msgstr : 0) * 10000L;
			break;
		}
		
		case SDDD:
			ret=FFScript::get_screen_d((ri->d[rINDEX])/10000 + ((get_currdmap())<<7), ri->d[rINDEX2] / 10000);
			break;
		
		case LINKOTILE:
			ret=FFCore.getHeroOTile(ri->d[rINDEX]/10000, ri->d[rINDEX2] / 10000);
			break;
			
		case SDDDD:
			ret=FFScript::get_screen_d(ri->d[rINDEX2] / 10000 + ((ri->d[rINDEX]/10000)<<7), ri->d[rEXP1] / 10000);
			break;

		case SCREENSCRIPT:
			ret=get_scr(ri->screenref)->script*10000;
			break;

		//These use the same method as GetScreenD -Z
		case SCREENWIDTH:
			// ret=FFScript::get_screenWidth(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)]);
			break;

		case SCREENHEIGHT:
			// ret=FFScript::get_screenHeight(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)]);
			break;

		case SCREENVIEWX:
			// ret=get_screenViewX(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)]);
			break;

		case SCREENVIEWY:
			// ret=get_screenViewY(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)]);
			break;
			
		case LIT:
			ret= get_lights() ? 10000 : 0;
			break;
			
		case WAVY:
			ret = wavy*10000;
			break;
			
		case QUAKE:
			ret = quakeclk*10000;
			break;

		case NPCCOUNT:
			ret = guys.Count()*10000;
			break;
			
		case ROOMDATA:
			ret = get_scr(ri->screenref)->catchall*10000;
			break;
			
		case ROOMTYPE:
			ret = get_scr(ri->screenref)->room*10000;
			break;
			
		case PUSHBLOCKX:
			ret = mblock2.active() ? int32_t(mblock2.x)*10000 : -10000;
			break;
			
		case PUSHBLOCKY:
			ret = mblock2.active() ? int32_t(mblock2.y)*10000 : -10000;
			break;
		
		case PUSHBLOCKLAYER:
			ret = mblock2.active() ? int32_t(mblock2.blockLayer)*10000 : -10000;
			break;
			
		case PUSHBLOCKCOMBO:
			ret = mblock2.bcombo*10000;
			break;
			
		case PUSHBLOCKCSET:
			ret = mblock2.cs*10000;
			break;
			
		case UNDERCOMBO:
			ret = get_scr(ri->screenref)->undercombo*10000;
			break;
			
		case UNDERCSET:
			ret = get_scr(ri->screenref)->undercset*10000;
			break;

		case SCREEN_INDEX:
			ret = ri->screenref*10000;
			break;

		case SCREEN_DRAW_ORIGIN:
			ret = (int)ri->screen_draw_origin;
			break;
		
		case SCREEN_DRAW_ORIGIN_TARGET:
			ret = ri->screen_draw_origin_target;
			break;

		//Creates an lweapon using an iemdata struct values to generate its properties.
		//Useful in conjunction with the new weapon editor. 
		case CREATELWPNDX:
		{
			//Z_message("Trying to get Hero->SetExtend().\n");
			int32_t ID = (ri->d[rINDEX] / 10000);
			int32_t itemid = (ri->d[rINDEX2]/10000);
			itemid = vbound(itemid,0,(MAXITEMS-1));
			
			// TODO: use has_space()
			if ( Lwpns.Count() < 256 )
			{
				
				(void)Lwpns.add
				(
					new weapon
					(
						(zfix)0, /*X*/
						(zfix)0, /*Y*/
						(zfix)0, /*Z*/
						ID,	 /*id*/
						0,	 /*type*/
						0,	 /*power*/
						0,	 /*dir*/
						-1,	 /*Parentid*/
						Hero.getUID(), /*prntid*/
						false,	 /*isdummy*/
						1,	 /*script_gen*/
						1,  /*islwpn*/
						(ID==wWind?1:0)  /*special*/
					)
				);
				ri->lwpn = Lwpns.spr(Lwpns.Count() - 1)->getUID();
				
				weapon *w = (weapon*)Lwpns.spr(Lwpns.Count()-1); //last created
				//w->LOADGFX(FFCore.getDefWeaponSprite(ID)); //What the fuck Zoria, this broke old quests...
				w->ScriptGenerated = 1;
				w->isLWeapon = 1;
				if(ID == wWind) w->specialinfo = 1;
				//weapon *w = (weapon*)Lwpns.spr(Lwpns.Count()-1); //last created
				//w->LOADGFX(FFCore.getDefWeaponSprite(ID)); //not needed here because this has access to wpn->prent
			}
			else
			{
				Z_scripterrlog("Tried to create too many LWeapons on the screen. The current LWeapon count is: %d\n", Lwpns.Count());
				ri->lwpn = 0;
			}
			
			ret = ri->lwpn; 
		}
		break;
		
		///----------------------------------------------------------------------------------------------------//
		//New Datatype Variables
			
		///----------------------------------------------------------------------------------------------------//
		//spritedata sp-> Variables
			
		case SPRITEDATATILE: GET_SPRITEDATA_VAR_INT(tile) break;
		case SPRITEDATAMISC: GET_SPRITEDATA_VAR_INT(misc) break;
		case SPRITEDATACSETS:
		{
			if (auto sd = checkSpriteData(ri->spritedataref); !sd)
				ret = -10000;
			else
				ret = ((sd->csets & 0xF) * 10000);
			break;
		}
		case SPRITEDATAFLCSET:
		{
			if (auto sd = checkSpriteData(ri->spritedataref); !sd)
				ret = -10000;
			else
				ret = (((sd->csets & 0xF0)>>4) * 10000);
			break;
		}
		case SPRITEDATAFRAMES: GET_SPRITEDATA_VAR_INT(frames) break;
		case SPRITEDATASPEED: GET_SPRITEDATA_VAR_INT(speed) break;
		case SPRITEDATATYPE: GET_SPRITEDATA_VAR_INT(type) break;
		case SPRITEDATAID:
		{
			if(unsigned(ri->spritedataref) > (MAXWPNS-1) )
			{
				ret = -10000;
				Z_scripterrlog("Invalid Sprite ID passed to spritedata->ID: %d\n", (ri->spritedataref*10000));
				break;
			}
			ret = ri->spritedataref*10000;
			break;
		}
		
		///----------------------------------------------------------------------------------------------------//
		//mapdata m-> variables
		#define	GET_MAPDATA_VAR_INT32(member) \
		{ \
			if ( mapscr *m = ResolveMapdataScr(ri->mapsref) ) \
			{ \
				ret = (m->member *10000); \
			} \
			else \
			{ \
				ret = -10000; \
			} \
		} \

		#define	GET_MAPDATA_VAR_INT16(member) \
		{ \
			if ( mapscr *m = ResolveMapdataScr(ri->mapsref) ) \
			{ \
				ret = (m->member *10000); \
			} \
			else \
			{ \
				ret = -10000; \
			} \
		} \

		#define	GET_MAPDATA_VAR_BYTE(member) \
		{ \
			if ( mapscr *m = ResolveMapdataScr(ri->mapsref) ) \
			{ \
				ret = (m->member *10000); \
			} \
			else \
			{ \
				ret = -10000; \
			} \
		} \
		
		#define GET_MAPDATA_FLAG(member) \
		{ \
			int32_t flag =  (value/10000);  \
			if (mapscr *m = ResolveMapdataScr(ri->mapsref)) \
			{ \
				ret = (m->member&flag) ? 10000 : 0); \
			} \
			else \
			{ \
				ret = -10000; \
			} \
		} \
		
		#define GET_MAPDATA_FFCPOS_INDEX32(member, indexbound) \
		{ \
			int32_t index = (ri->d[rINDEX] / 10000); \
			if (auto handle = ResolveMapdataFFC(ri->mapsref, index)) \
			{ \
				ret = (handle.ffc->member).getZLong(); \
			} \
			else \
			{ \
				ret = -10000; \
			} \
		} \
		
		#define GET_MAPDATA_FFC_INDEX32(member, indexbound) \
		{ \
			int32_t index = (ri->d[rINDEX] / 10000); \
			if (auto handle = ResolveMapdataFFC(ri->mapsref, index)) \
			{ \
				ret = (handle.ffc->member)*10000; \
			} \
			else \
			{ \
				ret = -10000; \
			} \
		} \

		#define GET_MAPDATA_FFC_INDEX32(member, indexbound) \
		{ \
			int32_t index = (ri->d[rINDEX] / 10000); \
			if (auto handle = ResolveMapdataFFC(ri->mapsref, index)) \
			{ \
				ret = (handle.ffc->member)*10000; \
			} \
			else \
			{ \
				ret = -10000; \
			} \
		} \
			
		case LOADMAPDATA:
				ret=FFScript::loadMapData();
				break;

		case CREATEBITMAP:
		{
			ret=FFCore.do_create_bitmap();
			break;
		}

		case MAPDATAVALID:		GET_MAPDATA_VAR_BYTE(valid); break;		//b
		case MAPDATAGUY: 		GET_MAPDATA_VAR_BYTE(guy); break;		//b
		case MAPDATASTRING:		GET_MAPDATA_VAR_INT32(str); break;		//w
		case MAPDATAROOM: 		GET_MAPDATA_VAR_BYTE(room);	break;		//b
		case MAPDATAITEM:
		{
			if ( mapscr *m = ResolveMapdataScr(ri->mapsref) )
			{
				if(m->hasitem)
					ret = (m->item *10000);
				else ret = -10000;
			}
			else
			{
				ret = -10000;
			}
			break;
		}
		case MAPDATAREGIONID:
		{
			if (auto scr = ResolveMapdataScr(ri->mapsref))
				ret = get_region_id(scr->map, scr->screen) * 10000;
			break;
		}
		case MAPDATAHASITEM: 		GET_MAPDATA_VAR_BYTE(hasitem); break;	//b
		case MAPDATADOORCOMBOSET: 	GET_MAPDATA_VAR_INT32(door_combo_set); break;	//w
		case MAPDATAWARPRETURNC: 	GET_MAPDATA_VAR_INT32(warpreturnc); break;	//w
		case MAPDATASTAIRX: 		GET_MAPDATA_VAR_BYTE(stairx); break;	//b
		case MAPDATASTAIRY: 		GET_MAPDATA_VAR_BYTE(stairy); break;	//b
		case MAPDATAITEMX:		GET_MAPDATA_VAR_BYTE(itemx); break; //itemx
		case MAPDATAITEMY:		GET_MAPDATA_VAR_BYTE(itemy); break;	//itemy
		case MAPDATACOLOUR: 		GET_MAPDATA_VAR_INT32(color); break;	//w
		case MAPDATAENEMYFLAGS: 	GET_MAPDATA_VAR_BYTE(flags11);	break;	//b
		case MAPDATAEXITDIR: 		GET_MAPDATA_VAR_BYTE(exitdir); break;	//b
		case MAPDATAPATTERN: 		GET_MAPDATA_VAR_BYTE(pattern); break;	//b
		case MAPDATAWARPARRIVALX: 	GET_MAPDATA_VAR_BYTE(warparrivalx); break;	//b
		case MAPDATAWARPARRIVALY: 	GET_MAPDATA_VAR_BYTE(warparrivaly); break;	//b
		case MAPDATASIDEWARPINDEX: 	GET_MAPDATA_VAR_BYTE(sidewarpindex); break;	//b
		case MAPDATAUNDERCOMBO: 	GET_MAPDATA_VAR_INT32(undercombo); break;	//w
		case MAPDATAUNDERCSET:	 	GET_MAPDATA_VAR_BYTE(undercset); break; //b
		case MAPDATACATCHALL:	 	GET_MAPDATA_VAR_INT32(catchall); break; //W

		case MAPDATACSENSITIVE: 	GET_MAPDATA_VAR_BYTE(csensitive); break;	//B
		case MAPDATANORESET: 		GET_MAPDATA_VAR_INT32(noreset); break;	//W
		case MAPDATANOCARRY: 		GET_MAPDATA_VAR_INT32(nocarry); break;	//W
		case MAPDATATIMEDWARPTICS: 	GET_MAPDATA_VAR_INT32(timedwarptics); break;	//W
		case MAPDATANEXTMAP: 		GET_MAPDATA_VAR_BYTE(nextmap); break;	//B
		case MAPDATANEXTSCREEN: 	GET_MAPDATA_VAR_BYTE(nextscr); break;	//B
		
		case MAPDATAVIEWX: 		break;//GET_MAPDATA_VAR_INT32(viewX, "ViewX"); break;	//W
		case MAPDATASCRIPT: 		GET_MAPDATA_VAR_INT32(script); break;	//W
		case MAPDATAVIEWY: 		break;//GET_MAPDATA_VAR_INT32(viewY, "ViewY"); break; //W
		case MAPDATASCREENWIDTH: 	break;//GET_MAPDATA_VAR_BYTE(scrWidth, "Width"); break;	//B
		case MAPDATASCREENHEIGHT: 	break;//GET_MAPDATA_VAR_BYTE(scrHeight,	"Height"); break;	//B
		case MAPDATAENTRYX: 		GET_MAPDATA_VAR_BYTE(entry_x); break;	//B
		case MAPDATAENTRYY: 		GET_MAPDATA_VAR_BYTE(entry_y); break;	//B
		
		//Number of ffcs that are in use (have valid data
		// NOTE: defunct. Never implemented correctly.
		case MAPDATANUMFF: 	
		{
			int index = ri->d[rINDEX] / 10000;

			if (auto handle = ResolveMapdataFFC(ri->mapsref, index))
			{
				ret = (handle.data() != 0) ? 10000 : 0;
			}
			else
			{
				ret = 0;
			}
			break;
		}

		case MAPDATAINTID: 	 //Same form as SetScreenD()
			//SetFFCInitD(ffindex, d, value)
		{
			int32_t index = (ri->d[rINDEX]/10000);
			int32_t d_index = ri->d[rINDEX2]/10000;

			if (BC::checkBounds(d_index, 0, 7) != SH::_NoError)
				break;

			if (auto handle = ResolveMapdataFFC(ri->mapsref, index))
				ret = handle.ffc->initd[d_index];
			else
			{
				ret = -10000;
			}
			break;
		}

		case MAPDATASCRIPTENTRY:
		{
			Z_scripterrlog("Unimplemented: %s\n", "ScriptEntry");
			ret = -10000;
		}
		break;
		case MAPDATASCRIPTOCCUPANCY:
		{
			Z_scripterrlog("Unimplemented: %s\n", "ScriptOccupancy");
			ret = -10000;
		}
		break;
		case MAPDATASCRIPTEXIT:
		{
			Z_scripterrlog("Unimplemented: %s\n", "ExitScript");
			ret = -10000;
		}
		break;

		case MAPDATAOCEANSFX:	 	GET_MAPDATA_VAR_BYTE(oceansfx); break;	//B
		case MAPDATABOSSSFX: 		GET_MAPDATA_VAR_BYTE(bosssfx); break;	//B
		case MAPDATASECRETSFX:	 	GET_MAPDATA_VAR_BYTE(secretsfx); break;	//B
		case MAPDATAHOLDUPSFX:	 	GET_MAPDATA_VAR_BYTE(holdupsfx); break; //B
		case MAPDATASCREENMIDI:
		{
			if (mapscr *m = ResolveMapdataScr(ri->mapsref))
			{
				ret = ((m->screen_midi+(MIDIOFFSET_MAPSCR-MIDIOFFSET_ZSCRIPT)) *10000);
			}
			else
			{
				ret = -10000;
			}
			break;
		}
		case MAPDATALENSLAYER:	 	GET_MAPDATA_VAR_BYTE(lens_layer); break;	//B, OLD QUESTS ONLY?
		case MAPDATAMAP:
		{
			if (mapscr *m = ResolveMapdataScr(ri->mapsref))
			{
				ret = getMap(ri->mapsref) * 10000;
			}
			else
			{
				ret = -10000;
			}
			break;
		}
		case MAPDATASCREEN:
		{
			if (mapscr *m = ResolveMapdataScr(ri->mapsref))
			{
				ret = getScreen(ri->mapsref) * 10000;
			}
			else
			{
				ret = -10000;
			}
			break;
		}
		case MAPDATASCRDATASIZE:
		{
			ret = -10000;
			if (mapscr *m = ResolveMapdataScr(ri->mapsref))
			{
				int index = get_ref_map_index(ri->mapsref);
				if (index < 0) break;

				ret = 10000*game->scriptDataSize(index);
			}
			break;
		}
		case MAPDATAGUYCOUNT:
		{
			if (mapscr *m = ResolveMapdataScr(ri->mapsref))
			{
				int mi = get_mi(ri->mapsref);
				if(mi > -1)
				{
					ret = game->guys[mi] * 10000;
					break;
				}
			}
			ret = -10000;
			break;
		}
		case MAPDATAEXDOOR:
		{
			ret = 0;
			if (mapscr *m = ResolveMapdataScr(ri->mapsref))
			{
				int mi = get_mi(ri->mapsref);
				if(mi < 0) break;
				int dir = SH::read_stack(ri->sp+1) / 10000;
				int ind = SH::read_stack(ri->sp+0) / 10000;
				if(unsigned(dir) > 3)
					Z_scripterrlog("Invalid dir '%d' passed to 'mapdata->GetExDoor()'; must be 0-3\n", dir);
				else if(unsigned(ind) > 7)
					Z_scripterrlog("Invalid index '%d' passed to 'mapdata->GetExDoor()'; must be 0-7\n", ind);
				else
				{
					int bit = 1<<ind;
					ret = (game->xdoors[mi][dir]&bit) ? 10000 : 0;
				}
			}
			break;
		}

		///----------------------------------------------------------------------------------------------------//
		//shopdata sd-> variables
	
		case SHOPDATATYPE:
		{
			int32_t ref = ri->shopsref; 
			if ( ref > NUMINFOSHOPS || ref < 0 ) ret = 0;
			else ret = ( ( ref <= NUMSHOPS ) ? 10000 : 20000 ); 
			break;
		}

		///----------------------------------------------------------------------------------------------------//
		//dmapdata dmd-> variables

		//getter
		case DMAPDATAID: ret = ri->dmapsref*10000; break; //read-only, equal to CurrentDMap
			
		case DMAPDATAMAP: 	//byte
		{
			ret = ((byte)DMaps[ri->dmapsref].map + 1) * 10000; break;
		}
		case DMAPDATALEVEL:	//word
		{
			ret = ((word)DMaps[ri->dmapsref].level) * 10000; break;
		}
		case DMAPDATAOFFSET:	//char
		{
			ret = ((char)DMaps[ri->dmapsref].xoff) * 10000; break;
		}
		case DMAPDATACOMPASS:	//byte
		{
			ret = ((byte)DMaps[ri->dmapsref].compass) * 10000; break;
		}
		case DMAPDATAPALETTE:	//word
		{
			ret = ((word)DMaps[ri->dmapsref].color) * 10000; break;
		}
		case DMAPSCRIPT:	//word
		{
			ret = (DMaps[ri->dmapsref].script) * 10000; break;
		}
		case DMAPDATAMIDI:	//byte
		{
			ret = (DMaps[ri->dmapsref].midi-MIDIOFFSET_DMAP) * 10000; break;
		}
		case DMAPDATACONTINUE:	//byte
		{
			ret = ((byte)DMaps[ri->dmapsref].cont) * 10000; break;
		}
		case DMAPDATATYPE:	//byte
		{
			ret = ((byte)DMaps[ri->dmapsref].type&dmfTYPE) * 10000; break;
		}
		case DMAPDATASIDEVIEW:	//byte
		{
			ret = ((DMaps[ri->dmapsref].sideview) ? 10000 : 0); break;
		}
		case DMAPDATAMUISCTRACK:	//byte
		{
			ret = ((byte)DMaps[ri->dmapsref].tmusictrack) * 10000; break;
		}
		case DMAPDATASUBSCRA:
		{
			ret = ((byte)DMaps[ri->dmapsref].active_subscreen) * 10000; break;
		}
		case DMAPDATASUBSCRP:
		{
			ret = ((byte)DMaps[ri->dmapsref].passive_subscreen) * 10000; break;
		}
		case DMAPDATASUBSCRO:
		{
			ret = ((byte)DMaps[ri->dmapsref].overlay_subscreen) * 10000; break;
		}
		case DMAPDATAFLAGS:	 //int32_t
		{
			ret = (DMaps[ri->dmapsref].flags) * 10000; break;
		}
		case DMAPDATAMIRRDMAP:
		{
			ret = (DMaps[ri->dmapsref].mirrorDMap) * 10000; break;
		}
		case DMAPDATALOOPSTART:
		{
			ret = (DMaps[ri->dmapsref].tmusic_loop_start); break;
		}
		case DMAPDATALOOPEND:
		{
			ret = (DMaps[ri->dmapsref].tmusic_loop_end); break;
		}
		case DMAPDATAXFADEIN:
		{
			ret = (DMaps[ri->dmapsref].tmusic_xfade_in * 10000); break;
		}
		case DMAPDATAXFADEOUT:
		{
			ret = (DMaps[ri->dmapsref].tmusic_xfade_out * 10000); break;
		}
		case DMAPDATAINTROSTRINGID:
		{
			ret = (DMaps[ri->dmapsref].intro_string_id * 10000); break;
		}
		case MUSICUPDATECOND:
		{
			ret = ((byte)FFCore.music_update_cond) * 10000; break;
		}
		case DMAPDATAASUBSCRIPT:	//word
		{
			ret = (DMaps[ri->dmapsref].active_sub_script) * 10000; break;
		}
		case DMAPDATAMAPSCRIPT:	//byte
		{
			ret = (DMaps[ri->dmapsref].onmap_script) * 10000; break;
		}
		case DMAPDATAPSUBSCRIPT:	//word
		{
			ret = (DMaps[ri->dmapsref].passive_sub_script) * 10000; break;
		}
			
		///----------------------------------------------------------------------------------------------------//
		//messagedata msgd-> variables
		case MESSAGEDATANEXT: //W
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
			{
				ret = -10000; break;
			}
			else 
			{
				ret = ((int32_t)MsgStrings[ID].nextstring) * 10000;
				break;
			}
		}	

		case MESSAGEDATATILE: //W
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else 
				ret = ((int32_t)MsgStrings[ID].tile) * 10000;
			break;
		}	

		case MESSAGEDATACSET: //b
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else 
				ret = ((int32_t)MsgStrings[ID].cset) * 10000;
			break;
		}	
		case MESSAGEDATATRANS: //BOOL
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else 
				ret = ((MsgStrings[ID].trans)?10000:0);
			break;
		}	
		case MESSAGEDATAFONT: //B
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else 
				ret = (int32_t)MsgStrings[ID].font * 10000;
			break;
		}	
		case MESSAGEDATAX: //SHORT
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else 
				ret = ((int32_t)MsgStrings[ID].x) * 10000;
			break;
		}	
		case MESSAGEDATAY: //SHORT
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else 
				ret = ((int32_t)MsgStrings[ID].y) * 10000;
			break;
		}	
		case MESSAGEDATAW: //UNSIGNED SHORT
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else 
				ret = ((int32_t)MsgStrings[ID].w) * 10000;
			break;
		}	
		case MESSAGEDATAH: //UNSIGNED SHORT
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else 
				ret = ((int32_t)MsgStrings[ID].h) * 10000;
			break;
		}	
		case MESSAGEDATASFX: //BYTE
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else 
				ret = ((int32_t)MsgStrings[ID].sfx) * 10000;
			break;
		}	
		case MESSAGEDATALISTPOS: //WORD
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else 
				ret = ((int32_t)MsgStrings[ID].listpos) * 10000;
			break;
		}	
		case MESSAGEDATAVSPACE: //BYTE
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else 
				ret = ((int32_t)MsgStrings[ID].vspace) * 10000;
			break;
		}	
		case MESSAGEDATAHSPACE: //BYTE
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else 
				ret = ((int32_t)MsgStrings[ID].hspace) * 10000;
			break;
		}	
		case MESSAGEDATAFLAGS: //BYTE
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else 
				ret = ((int32_t)MsgStrings[ID].stringflags) * 10000;
			break;
		}
		case MESSAGEDATAPORTTILE: //INT
		{
			int32_t ID = ri->zmsgref;
			
			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].portrait_tile) * 10000;
			break;
		}
		case MESSAGEDATAPORTCSET: //BYTE
		{
			int32_t ID = ri->zmsgref;
			
			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].portrait_cset) * 10000;
			break;
		}
		case MESSAGEDATAPORTX: //BYTE
		{
			int32_t ID = ri->zmsgref;
			
			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].portrait_x) * 10000;
			break;
		}
		case MESSAGEDATAPORTY: //BYTE
		{
			int32_t ID = ri->zmsgref;
			
			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].portrait_y) * 10000;
			break;
		}
		case MESSAGEDATAPORTWID: //BYTE
		{
			int32_t ID = ri->zmsgref;
			
			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].portrait_tw) * 10000;
			break;
		}
		case MESSAGEDATAPORTHEI: //BYTE
		{
			int32_t ID = ri->zmsgref;
			
			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].portrait_th) * 10000;
			break;
		}
		case MESSAGEDATATEXTLEN: //BYTE
		{
			int32_t ID = ri->zmsgref;
			
			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = int32_t(MsgStrings[ID].s.size()) * 10000;
			break;
		}
		case MESSAGEDATATEXTWID:
		{
			ret = do_msgwidth(ri->zmsgref)*10000;
			break;
		}
		case MESSAGEDATATEXTHEI:
		{
			ret = do_msgheight(ri->zmsgref)*10000;
			break;
		}

		///----------------------------------------------------------------------------------------------------//
		//combodata cd-> Getter variables
		#define	GET_COMBO_VAR_INT(member) \
		{ \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
				ret = -10000; \
			} \
			else \
			{ \
				ret = (combobuf[ri->combosref].member *10000); \
			} \
		} \

		#define	GET_COMBO_VAR_BYTE(member) \
		{ \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
				ret = -10000; \
			} \
			else \
			{ \
				ret = (combobuf[ri->combosref].member *10000); \
			} \
		} \
		
		#define	GET_COMBO_VAR_DWORD(member) \
		{ \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
				ret = -10000; \
			} \
			else \
			{ \
				ret = (combobuf[ri->combosref].member *10000); \
			} \
		} \
		
		#define GET_COMBO_VAR_INDEX(member, indexbound) \
		{ \
				int32_t indx = ri->d[rINDEX] / 10000; \
				if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
				{ \
					scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
					ret = -10000; \
				} \
				else if ( indx < 0 || indx >= indexbound ) \
				{ \
					scripting_log_error_with_context("Invalid Array Index: {}", indx); \
					ret = -10000; \
				} \
				else \
				{ \
					ret = (combobuf[ri->combosref].member[indx] * 10000); \
				} \
		}

		#define GET_COMBO_BYTE_INDEX(member, indexbound) \
		{ \
				int32_t indx = ri->d[rINDEX] / 10000; \
				if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
				{ \
					scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
					ret = -10000; \
				} \
				else if ( indx < 0 || indx >= indexbound ) \
				{ \
					scripting_log_error_with_context("Invalid Array Index: {}", indx); \
					ret = -10000; \
				} \
				else \
				{ \
					ret = (combobuf[ri->combosref].member[indx] * 100000); \
				} \
		}
		
		#define GET_COMBO_FLAG(member, indexbound) \
		{ \
			int32_t flag =  (value/10000);  \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
			} \
			else \
			{ \
				ret = (combobuf[ri->combosref].member&flag) ? 10000 : 0); \
			} \
		} \

		//comboclass macros

		#define	GET_COMBOCLASS_VAR_INT(member) \
		{ \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
				ret = -10000; \
			} \
			else \
			{ \
				ret = (combo_class_buf[combobuf[ri->combosref].type].member *10000); \
			} \
		} \

		#define	GET_COMBOCLASS_VAR_BYTE(member) \
		{ \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
				ret = -10000; \
			} \
			else \
			{ \
				ret = (combo_class_buf[combobuf[ri->combosref].type].member *10000); \
			} \
		} \
		
		#define	GET_COMBOCLASS_VAR_DWORD(member) \
		{ \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
				ret = -10000; \
			} \
			else \
			{ \
				ret = (combo_class_buf[combobuf[ri->combosref].type].member *10000); \
			} \
		} \
		
		#define GET_COMBOCLASS_VAR_INDEX(member, indexbound) \
		{ \
				int32_t indx = ri->d[rINDEX] / 10000; \
				if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
				{ \
					scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
					ret = -10000; \
				} \
				else if ( indx < 0 || indx > indexbound ) \
				{ \
					scripting_log_error_with_context("Invalid Array Index: {}", indx); \
					ret = -10000; \
				} \
				else \
				{ \
					ret = (combo_class_buf[combobuf[ri->combosref].type].member[indx] * 10000); \
				} \
		}

		#define GET_COMBOCLASS_BYTE_INDEX(member, indexbound) \
		{ \
				int32_t indx = ri->d[rINDEX] / 10000; \
				if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
				{ \
					scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
					ret = -10000; \
				} \
				else if ( indx < 0 || indx > indexbound ) \
				{ \
					scripting_log_error_with_context("Invalid Array Index: {}", indx); \
					ret = -10000; \
				} \
				else \
				{ \
					ret = (combo_class_buf[combobuf[ri->combosref].type].member[indx] * 100000); \
				} \
		}
		
		#define GET_COMBOCLASS_FLAG(member, indexbound) \
		{ \
			int32_t flag =  (value/10000);  \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
			} \
			else \
			{ \
				ret = (combo_class_buf[combobuf[ri->combosref].type].member&flag) ? 10000 : 0); \
			} \
		} \
		
		case COMBOXR:
		{
			//ri->combosref = id; //'this' pointer
			//ri->comboposref = i; //used for X(), Y(), Layer(), and so forth.
			if ( curScriptType == ScriptType::Combo )
			{
				rpos_t rpos = combopos_ref_to_rpos(ri->comboposref);
				ret = (( COMBOX_REGION((rpos)) ) * 10000); //comboscriptstack[i]
				//this may be wrong...may need a special new var for this, storing the exact combopos
				//i is the current script number
			}
			else
			{
				scripting_log_error_with_context("Can only be called by combodata scripts, but you tried to use it from script type {}, script token {}", ScriptTypeToString(curScriptType), comboscriptmap[ri->combosref].scriptname);
				ret = -10000;
			}
			break;
		}

		case COMBOYR:
		{
			if ( curScriptType == ScriptType::Combo )
			{
				rpos_t rpos = combopos_ref_to_rpos(ri->comboposref);
				ret = (( COMBOY_REGION((rpos)) ) * 10000); //comboscriptstack[i]
			}
			else
			{
				scripting_log_error_with_context("Can only be called by combodata scripts, but you tried to use it from script type {}, script token {}", ScriptTypeToString(curScriptType), comboscriptmap[ri->combosref].scriptname);				ret = -10000;
			}
			break;
		}
		case COMBOPOSR:
		{
			if ( curScriptType == ScriptType::Combo )
			{
				rpos_t rpos = combopos_ref_to_rpos(ri->comboposref);
				ret = (int)rpos * 10000; //comboscriptstack[i]
			}
			else
			{
				scripting_log_error_with_context("Can only be called by combodata scripts, but you tried to use it from script type {}, script token {}", ScriptTypeToString(curScriptType), comboscriptmap[ri->combosref].scriptname);				ret = -10000;
			}
			break;
		}
		case COMBOLAYERR:
		{
			if ( curScriptType == ScriptType::Combo )
			{
				int32_t layer = combopos_ref_to_layer(ri->comboposref);
				ret = layer * 10000; //comboscriptstack[i]
			}
			else
			{
				scripting_log_error_with_context("Can only be called by combodata scripts, but you tried to use it from script type {}, script token {}", ScriptTypeToString(curScriptType), comboscriptmap[ri->combosref].scriptname);				ret = -10000;
			}
			break;
		}
		
		//NEWCOMBO STRUCT
		case COMBODTILE:		GET_COMBO_VAR_DWORD(tile); break;					//word
		case COMBODOTILE:		GET_COMBO_VAR_DWORD(o_tile); break;			//word
		case COMBODFRAME:		GET_COMBO_VAR_BYTE(cur_frame); break;				//char
		case COMBODACLK:		GET_COMBO_VAR_BYTE(aclk); break;				//char
		case COMBODASPEED:		GET_COMBO_VAR_BYTE(speed); break;					//char
		case COMBODFLIP:		GET_COMBO_VAR_BYTE(flip); break;					//char
		case COMBODWALK:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
				ret = -10000;
			}
			else
			{
				ret = ((combobuf[ri->combosref].walk&0x0F) *10000);
			}
			break;
		}
		case COMBODEFFECT:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
				ret = -10000;
			}
			else
			{
				ret = (((combobuf[ri->combosref].walk&0xF0)>>4) *10000);
			}
			break;
		}
		case COMBODTYPE:		GET_COMBO_VAR_BYTE(type); break;					//char
		case COMBODCSET:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
				ret = -10000;
			}
			else
			{
				bool neg = combobuf[ri->combosref].csets&0x8;
				ret = ((combobuf[ri->combosref].csets&0x7) * (neg ? -10000 : 10000));
			}
			break;
		}
		case COMBODCSET2FLAGS:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else
			{
				ret = ((combobuf[ri->combosref].csets & 0xF0) >> 4) * 10000;
			}
			break;
		}
		case COMBODFOO:			break;						//W
		case COMBODATASCRIPT:			GET_COMBO_VAR_DWORD(script); break;						//W
		case COMBODFRAMES:		GET_COMBO_VAR_BYTE(frames); break;					//C
		case COMBODNEXTD:		GET_COMBO_VAR_INT(nextcombo); break;					//W
		case COMBODNEXTC:		GET_COMBO_VAR_BYTE(nextcset); break;				//C
		case COMBODFLAG:		GET_COMBO_VAR_BYTE(flag); break;					//C
		case COMBODSKIPANIM:		GET_COMBO_VAR_BYTE(skipanim); break;				//C
		case COMBODNEXTTIMER:		GET_COMBO_VAR_DWORD(nexttimer); break;				//W
		case COMBODAKIMANIMY:		GET_COMBO_VAR_BYTE(skipanimy); break;				//C
		case COMBODANIMFLAGS:		GET_COMBO_VAR_BYTE(animflags); break;				//C
		case COMBODUSRFLAGS:		GET_COMBO_VAR_INT(usrflags); break;				//LONG
		case COMBODTRIGGERITEM:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->triggeritem * 10000;
			break;
		}
		case COMBODTRIGGERTIMER:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trigtimer * 10000;
			break;
		}
		case COMBODTRIGGERSFX:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trigsfx * 10000;
			break;
		}
		case COMBODTRIGGERCHANGECMB:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trigchange * 10000;
			break;
		}
		case COMBODTRIGGERPROX:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trigprox * 10000;
			break;
		}
		case COMBODTRIGGERLIGHTBEAM:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->triglbeam * 10000;
			break;
		}
		case COMBODTRIGGERCTR:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trigctr * 10000;
			break;
		}
		case COMBODTRIGGERCTRAMNT:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trigctramnt * 10000;
			break;
		}
		case COMBODTRIGGERCOOLDOWN:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trigcooldown * 10000;
			break;
		}
		case COMBODTRIGGERCOPYCAT:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trigcopycat * 10000;
			break;
		}
		case COMBODTRIGITEMPICKUP:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->spawnip * 10000;
			break;
		}
		case COMBODTRIGEXSTATE:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->exstate * 10000;
			break;
		}
		case COMBODTRIGEXDOORDIR:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->exdoor_dir * 10000;
			break;
		}
		case COMBODTRIGEXDOORIND:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->exdoor_ind * 10000;
			break;
		}
		case COMBODTRIGSPAWNENEMY:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->spawnenemy * 10000;
			break;
		}
		case COMBODTRIGSPAWNITEM:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->spawnitem * 10000;
			break;
		}
		case COMBODTRIGCSETCHANGE:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trigcschange * 10000;
			break;
		}
		case COMBODTRIGLITEMS:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_levelitems * 10000;
			break;
		}
		case COMBODTRIGDMAPLVL:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trigdmlevel * 10000;
			break;
		}
		case COMBODTRIGTINTR:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
				break;
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trigtint[0] * 10000;
			break;
		}
		case COMBODTRIGTINTG:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trigtint[1] * 10000;
			break;
		}
		case COMBODTRIGTINTB:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trigtint[2] * 10000;
			break;
		}
		case COMBODTRIGLVLPAL:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->triglvlpalette * 10000;
			break;
		}
		case COMBODTRIGBOSSPAL:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trigbosspalette * 10000;
			break;
		}
		case COMBODTRIGQUAKETIME:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trigquaketime * 10000;
			break;
		}
		case COMBODTRIGWAVYTIME:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trigwavytime * 10000;
			break;
		}
		case COMBODTRIGSWORDJINX:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_swjinxtime * 10000;
			break;
		}
		case COMBODTRIGITEMJINX:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_itmjinxtime * 10000;
			break;
		}
		case COMBODTRIGSHIELDJINX:
		{
			ret = -10000;
			if (ri->combosref < 0 || ri->combosref >(MAXCOMBOS - 1))
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_shieldjinxtime * 10000;
			break;
		}
		case COMBODTRIGSTUN:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_stuntime * 10000;
			break;
		}
		case COMBODTRIGBUNNY:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_bunnytime * 10000;
			break;
		}
		case COMBODTRIGPUSHTIME:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_pushtime * 10000;
			break;
		}
		case COMBODLIFTGFXCOMBO:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = (combobuf[ri->combosref].liftcmb) * 10000;
			break;
		}
		case COMBODLIFTGFXCCSET:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = (combobuf[ri->combosref].liftcs) * 10000;
			break;
		}
		case COMBODLIFTUNDERCMB:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = (combobuf[ri->combosref].liftundercmb) * 10000;
			break;
		}
		case COMBODLIFTUNDERCS:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = (combobuf[ri->combosref].liftundercs) * 10000;
			break;
		}
		case COMBODLIFTDAMAGE:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = (combobuf[ri->combosref].liftdmg) * 10000;
			break;
		}
		case COMBODLIFTLEVEL:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = (combobuf[ri->combosref].liftlvl) * 10000;
			break;
		}
		case COMBODLIFTITEM:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = (combobuf[ri->combosref].liftitm) * 10000;
			break;
		}
		case COMBODLIFTGFXTYPE:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = (combobuf[ri->combosref].liftgfx) * 10000;
			break;
		}
		case COMBODLIFTGFXSPRITE:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = (combobuf[ri->combosref].liftsprite) * 10000;
			break;
		}
		case COMBODLIFTSFX:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = (combobuf[ri->combosref].liftsfx) * 10000;
			break;
		}
		case COMBODLIFTBREAKSPRITE:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = (combobuf[ri->combosref].liftbreaksprite) * 10000;
			break;
		}
		case COMBODLIFTBREAKSFX:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = (combobuf[ri->combosref].liftbreaksfx) * 10000;
			break;
		}
		case COMBODLIFTHEIGHT:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = (combobuf[ri->combosref].lifthei) * 10000;
			break;
		}
		case COMBODLIFTTIME:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = (combobuf[ri->combosref].lifttime) * 10000;
			break;
		}
		case COMBODLIFTLIGHTRAD:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = (combobuf[ri->combosref].lift_weap_data.light_rads[WPNSPR_BASE]) * 10000;
			break;
		}
		case COMBODLIFTLIGHTSHAPE:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = (combobuf[ri->combosref].lift_weap_data.glow_shape) * 10000;
			break;
		}
		case COMBODLIFTWEAPONITEM:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = (combobuf[ri->combosref].lift_parent_item) * 10000;
			break;
		}
		case COMBODTRIGGERLSTATE:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_lstate * 10000;
			break;
		}
		case COMBODTRIGGERGSTATE:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_gstate * 10000;
			break;
		}
		case COMBODTRIGGERGROUP:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_group * 10000;
			break;
		}
		case COMBODTRIGGERGROUPVAL:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_group_val * 10000;
			break;
		}
		case COMBODTRIGGERGTIMER:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_statetime * 10000;
			break;
		}
		case COMBODTRIGGERGENSCRIPT:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_genscr * 10000;
			break;
		}
		
		case COMBODTRIGGERLEVEL:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				ret = trig->triggerlevel * 10000;
			break;
		}
		case COMBODATAID: 		ret = (ri->combosref*10000); break;
		case COMBODNUMTRIGGERS:
		{
			ret = -10000;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = combobuf[ri->combosref].triggers.size() * 10000;
			break;
		}
		case COMBODONLYGEN:
		{
			ret = 0;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = combobuf[ri->combosref].only_gentrig ? 10000 : 0;
			break;
		}
		case COMBOD_Z_HEIGHT:
		{
			ret = 0;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = combobuf[ri->combosref].z_height.getZLong();
			break;
		}
		case COMBOD_Z_STEP_HEIGHT:
		{
			ret = 0;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else ret = combobuf[ri->combosref].z_step_height.getZLong();
			break;
		}
		//COMBOCLASS STRUCT
		//case COMBODNAME:		//CHAR[64], STRING
		case COMBODBLOCKNPC:		GET_COMBOCLASS_VAR_BYTE(block_enemies); break;			//C
		case COMBODBLOCKHOLE:		GET_COMBOCLASS_VAR_BYTE(block_hole); break;			//C
		case COMBODBLOCKTRIG:		GET_COMBOCLASS_VAR_BYTE(block_trigger); break; 		//C
		case COMBODBLOCKWEAPON:		GET_COMBOCLASS_BYTE_INDEX(block_weapon, 32); break;		//C, 32 INDICES
		case COMBODCONVXSPEED:		GET_COMBOCLASS_VAR_DWORD(conveyor_x_speed); break;		//SHORT
		case COMBODCONVYSPEED:		GET_COMBOCLASS_VAR_DWORD(conveyor_y_speed); break;		//SHORT
		case COMBODSPAWNNPC:		GET_COMBOCLASS_VAR_DWORD(create_enemy); break;			//W
		case COMBODSPAWNNPCWHEN:	GET_COMBOCLASS_VAR_BYTE(create_enemy_when); break;		//C
		case COMBODSPAWNNPCCHANGE:	GET_COMBOCLASS_VAR_INT(create_enemy_change); break;		//LONG
		case COMBODDIRCHANGETYPE:	GET_COMBOCLASS_VAR_BYTE(directional_change_type); break;		//C
		case COMBODDISTANCECHANGETILES:	GET_COMBOCLASS_VAR_INT(distance_change_tiles); break; 	//LONG
		case COMBODDIVEITEM:		GET_COMBOCLASS_VAR_DWORD(dive_item); break;				//SHORT
		case COMBODDOCK:		GET_COMBOCLASS_VAR_BYTE(dock); break;					//C
		case COMBODFAIRY:		GET_COMBOCLASS_VAR_BYTE(fairy); break;					//C
		case COMBODFFATTRCHANGE:	GET_COMBOCLASS_VAR_BYTE(ff_combo_attr_change); break;	//C
		case COMBODFOORDECOTILE:	GET_COMBOCLASS_VAR_INT(foot_decorations_tile); break;		//LONG
		case COMBODFOORDECOTYPE:	GET_COMBOCLASS_VAR_BYTE(foot_decorations_type); break;	//C
		case COMBODHOOKSHOTPOINT:	GET_COMBOCLASS_VAR_BYTE(hookshot_grab_point); break;		//C
		case COMBODLADDERPASS:		GET_COMBOCLASS_VAR_BYTE(ladder_pass); break;				//C
		case COMBODLOCKBLOCK:		GET_COMBOCLASS_VAR_BYTE(lock_block_type); break;			//C
		case COMBODLOCKBLOCKCHANGE:	GET_COMBOCLASS_VAR_INT(lock_block_change); break;		//LONG
		case COMBODMAGICMIRROR:		GET_COMBOCLASS_VAR_BYTE(magic_mirror_type); break;			//C
		case COMBODMODHPAMOUNT:		GET_COMBOCLASS_VAR_DWORD(modify_hp_amount); break;		//SHORT
		case COMBODMODHPDELAY:		GET_COMBOCLASS_VAR_BYTE(modify_hp_delay); break;			//C
		case COMBODMODHPTYPE:		GET_COMBOCLASS_VAR_BYTE(modify_hp_type); break; 			//C
		case COMBODNMODMPAMOUNT:	GET_COMBOCLASS_VAR_DWORD(modify_mp_amount); break;		//SHORT
		case COMBODMODMPDELAY:		GET_COMBOCLASS_VAR_BYTE(modify_mp_delay); break;			//C
		case COMBODMODMPTYPE:		GET_COMBOCLASS_VAR_BYTE(modify_mp_type);	break;			//C
		case COMBODNOPUSHBLOCK:		GET_COMBOCLASS_VAR_BYTE(no_push_blocks); break;			//C
		case COMBODOVERHEAD:		GET_COMBOCLASS_VAR_BYTE(overhead); break;				//C
		case COMBODPLACENPC:		GET_COMBOCLASS_VAR_BYTE(place_enemy); break;			//C
		case COMBODPUSHDIR:		GET_COMBOCLASS_VAR_BYTE(push_direction); break; 			//C
		case COMBODPUSHWAIT:		GET_COMBOCLASS_VAR_BYTE(push_wait); break;				//C
		case COMBODPUSHHEAVY:		GET_COMBOCLASS_VAR_BYTE(push_weight); break;				//C
		case COMBODPUSHED:		GET_COMBOCLASS_VAR_BYTE(pushed); break;				//C
		case COMBODRAFT:		GET_COMBOCLASS_VAR_BYTE(raft); break;					//C
		case COMBODRESETROOM:		GET_COMBOCLASS_VAR_BYTE(reset_room); break;			//C
		case COMBODSAVEPOINTTYPE:	GET_COMBOCLASS_VAR_BYTE(save_point_type); break;			//C
		case COMBODSCREENFREEZETYPE:	GET_COMBOCLASS_VAR_BYTE(screen_freeze_type); break;		//C
		case COMBODSECRETCOMBO:		GET_COMBOCLASS_VAR_BYTE(secret_combo); break;			//C
		case COMBODSINGULAR:		GET_COMBOCLASS_VAR_BYTE(singular); break;				//C
		case COMBODSLOWWALK:		GET_COMBOCLASS_VAR_BYTE(slow_movement); break;			//C
		case COMBODSTATUETYPE:		GET_COMBOCLASS_VAR_BYTE(statue_type); break;				//C
		case COMBODSTEPTYPE:		GET_COMBOCLASS_VAR_BYTE(step_type); break;				//C
		case COMBODSTEPCHANGEINTO:	GET_COMBOCLASS_VAR_INT(step_change_to); break;			//LONG
		case COMBODSTRIKEWEAPONS:	GET_COMBOCLASS_BYTE_INDEX(strike_weapons, 32); break;			//BYTE, 32 INDICES. 
		case COMBODSTRIKEREMNANTS:	GET_COMBOCLASS_VAR_INT(strike_remnants); break;		//LONG
		case COMBODSTRIKEREMNANTSTYPE:	GET_COMBOCLASS_VAR_BYTE(strike_remnants_type); break;	//C
		case COMBODSTRIKECHANGE:	GET_COMBOCLASS_VAR_INT(strike_change); break;			//LONG
		case COMBODSTRIKEITEM:		GET_COMBOCLASS_VAR_DWORD(strike_item); break;			//SHORT
		case COMBODTOUCHITEM:		GET_COMBOCLASS_VAR_DWORD(touch_item); break;			//SHORT
		case COMBODTOUCHSTAIRS:		GET_COMBOCLASS_VAR_BYTE(touch_stairs); break;			//C
		case COMBODTRIGGERTYPE:		GET_COMBOCLASS_VAR_BYTE(trigger_type); break;			//C
		case COMBODTRIGGERSENS:		GET_COMBOCLASS_VAR_BYTE(trigger_sensitive); break;	//C
		case COMBODWARPTYPE:		GET_COMBOCLASS_VAR_BYTE(warp_type); break;				//C
		case COMBODWARPSENS:		GET_COMBOCLASS_VAR_BYTE(warp_sensitive); break; 		//C
		case COMBODWARPDIRECT:		GET_COMBOCLASS_VAR_BYTE(warp_direct); break;			//C
		case COMBODWARPLOCATION:	GET_COMBOCLASS_VAR_BYTE(warp_location); break;			//C
		case COMBODWATER:		GET_COMBOCLASS_VAR_BYTE(water); break;					//C
		case COMBODWHISTLE:		GET_COMBOCLASS_VAR_BYTE(whistle); break;				//C
		case COMBODWINGAME:		GET_COMBOCLASS_VAR_BYTE(win_game); break; 				//C
		case COMBODBLOCKWPNLEVEL:	GET_COMBOCLASS_VAR_BYTE(block_weapon_lvl); break;		//C


		
		///----------------------------------------------------------------------------------------------------//
		case CMBTRIGWPNLEVEL:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->triggerlevel * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGREQITEM:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->triggeritem * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGTIMER:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trigtimer * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGSFX:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trigsfx * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGCHANGECMB:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trigchange * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGCSETCHANGE:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trigcschange * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGPROX:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trigprox * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGLIGHTBEAM:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->triglbeam * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGCTR:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trigctr * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGCTRAMNT:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trigctramnt * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGCOOLDOWN:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trigcooldown * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGCOPYCAT:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trigcopycat * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGITEMPICKUP:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->spawnip * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGEXSTATE:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->exstate * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGEXDOORDIR:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->exdoor_dir * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGEXDOORIND:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->exdoor_ind * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGSPAWNENEMY:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->spawnenemy * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGSPAWNITEM:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->spawnitem * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGLSTATE:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trig_lstate * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGGSTATE:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trig_gstate * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGGTIMER:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trig_statetime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGGENSCRIPT:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trig_genscr * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGGROUP:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trig_group * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGGROUPVAL:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trig_group_val * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGLITEMS:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trig_levelitems * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGDMAPLVL:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trigdmlevel * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGTINTR:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trigtint[0] * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGTINTG:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trigtint[1] * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGTINTB:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trigtint[2] * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGLVLPAL:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->triglvlpalette * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGBOSSPAL:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trigbosspalette * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGQUAKETIME:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trigquaketime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGWAVYTIME:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trigwavytime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGSWORDJINX:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trig_swjinxtime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGITEMJINX:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trig_itmjinxtime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGSHIELDJINX:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trig_shieldjinxtime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGSTUN:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trig_stuntime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGBUNNY:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trig_bunnytime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGPUSHTIME:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				ret = trig->trig_pushtime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGGERPROMPTCID:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->prompt_cid * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERPROMPTCS:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->prompt_cs * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERFAILPROMPTCID:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->fail_prompt_cid * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERFAILPROMPTCS:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->fail_prompt_cs * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERPROMPTX:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->prompt_x * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERPROMPTY:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->prompt_y * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERTRIGSTR:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->trig_msgstr * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERFAILSTR:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->fail_msgstr * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERPLAYERBOUNCE:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->player_bounce;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERREQPLAYERZ:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->req_player_z;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERDESTHEROX:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->dest_player_x;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERDESTHEROY:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->dest_player_y;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERDESTHEROZ:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->dest_player_z;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERREQPLAYERJUMP:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->req_player_jump;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERREQPLAYERX:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->req_player_x;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERREQPLAYERY:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->req_player_y;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERFORCEPLAYERDIR:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->dest_player_dir * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERICECOMBO:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->force_ice_combo * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERICEVX:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->force_ice_vx;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERICEVY:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				ret = trig->force_ice_vy;
			else ret = -10000;
			break;
		}
		///----------------------------------------------------------------------------------------------------//
		//npcdata nd-> variables
			
		//npcdata nd->member variable
		#define	GET_NPCDATA_VAR_INT32(member, str) \
		{ \
			if( (unsigned) ri->npcdataref > (MAXNPCS-1) ) \
			{ \
				Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", str, (ri->npcdataref*10000)); \
				ret = -10000; \
			} \
			else \
			{ \
				ret = (guysbuf[ri->npcdataref].member *10000); \
			} \
		} \

		#define	GET_NPCDATA_VAR_BYTE(member, str) \
		{ \
			if( (unsigned) ri->npcdataref > (MAXNPCS-1) ) \
			{ \
				Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", str, (ri->npcdataref*10000)); \
				ret = -10000; \
			} \
			else \
			{ \
				ret = (guysbuf[ri->npcdataref].member *10000); \
			} \
		} \
		
		#define	GET_NPCDATA_VAR_INT16(member, str) \
		{ \
			if( (unsigned) ri->npcdataref > (MAXNPCS-1) ) \
			{ \
				Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", str, (ri->npcdataref*10000)); \
				ret = -10000; \
			} \
			else \
			{ \
				ret = (guysbuf[ri->npcdataref].member *10000); \
			} \
		} \

		#define GET_NPCDATA_FLAG(member, str, indexbound) \
		{ \
			int32_t flag =  (value/10000);  \
			if( (unsigned) ri->npcdataref > (MAXNPCS-1) ) \
			{ \
				Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", str, (ri->npcdataref*10000)); \
			} \
			else \
			{ \
				ret = (guysbuf[ID].member&flag) ? 10000 : 0); \
			} \
		} \

		// These are for compat only, though seemingly no quests even use these.
		case NPCDATAFLAGS1:
		{
			if( (unsigned) ri->npcdataref > (MAXNPCS-1) )
			{
				Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", "Flags (deprecated)", (ri->npcdataref*10000));
				ret = -10000;
			}
			else
			{
				uint32_t value = guysbuf[ri->npcdataref].flags & 0xFFFFFFFFLL;
				ret = value * 10000;
			}
		}
		break;
		case NPCDATAFLAGS2:
		{
			if( (unsigned) ri->npcdataref > (MAXNPCS-1) )
			{
				Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", "Flags2 (deprecated)", (ri->npcdataref*10000));
				ret = -10000;
			}
			else
			{
				uint32_t value = (guysbuf[ri->npcdataref].flags >> 32) & 0xFFFFFFFFLL;
				ret = value * 10000;
			}
		}
		break;
		
		case NPCDATATILE: GET_NPCDATA_VAR_BYTE(tile, "Tile"); break;
		case NPCDATAWIDTH: GET_NPCDATA_VAR_BYTE(width, "Width"); break;
		case NPCDATAHEIGHT: GET_NPCDATA_VAR_BYTE(height, "Height"); break;
		case NPCDATASTILE: GET_NPCDATA_VAR_BYTE(s_tile, "STile"); break;
		case NPCDATASWIDTH: GET_NPCDATA_VAR_BYTE(s_width, "SWidth"); break;
		case NPCDATASHEIGHT: GET_NPCDATA_VAR_BYTE(s_height, "SHeight"); break;
		case NPCDATAETILE: GET_NPCDATA_VAR_INT32(e_tile, "ExTile"); break;
		case NPCDATAEWIDTH: GET_NPCDATA_VAR_BYTE(e_width, "ExWidth"); break;
		case NPCDATAEHEIGHT: GET_NPCDATA_VAR_BYTE(e_height, "ExHeight"); break;
		case NPCDATAHP: GET_NPCDATA_VAR_INT16(hp, "HP"); break;
		case NPCDATAFAMILY: GET_NPCDATA_VAR_INT16(family, "Family"); break;
		case NPCDATACSET: GET_NPCDATA_VAR_INT16(cset, "CSet"); break;
		case NPCDATAANIM: GET_NPCDATA_VAR_INT16(anim, "Anim"); break;
		case NPCDATAEANIM: GET_NPCDATA_VAR_INT16(e_anim, "ExAnim"); break;
		case NPCDATAFRAMERATE: GET_NPCDATA_VAR_INT16(frate, "Framerate"); break;
		case NPCDATAEFRAMERATE: GET_NPCDATA_VAR_INT16(e_frate, "ExFramerate"); break;
		case NPCDATATOUCHDAMAGE: GET_NPCDATA_VAR_INT16(dp, "TouchDamage"); break;
		case NPCDATAWEAPONDAMAGE: GET_NPCDATA_VAR_INT16(wdp, "WeaponDamage"); break;
		case NPCDATAWEAPON: GET_NPCDATA_VAR_INT16(weapon, "Weapon"); break;
		case NPCDATARANDOM: GET_NPCDATA_VAR_INT16(rate, "Random"); break;
		case NPCDATAHALT: GET_NPCDATA_VAR_INT16(hrate, "Haltrate"); break;
		case NPCDATASTEP: GET_NPCDATA_VAR_INT16(step, "Step"); break;
		case NPCDATAHOMING: GET_NPCDATA_VAR_INT16(homing, "Homing"); break;
		case NPCDATAHUNGER: GET_NPCDATA_VAR_INT16(grumble, "Hunger"); break;
		case NPCDATADROPSET: GET_NPCDATA_VAR_INT16(item_set, "Dropset"); break;
		case NPCDATABGSFX: GET_NPCDATA_VAR_INT16(bgsfx, "BGSFX"); break;
		case NPCDATADEATHSFX: GET_NPCDATA_VAR_BYTE(deadsfx, "DeathSFX"); break;
		case NPCDATAHITSFX: GET_NPCDATA_VAR_BYTE(hitsfx, "HitSFX"); break;
		case NPCDATAXOFS: GET_NPCDATA_VAR_INT32(xofs, "DrawXOffset"); break;
		case NPCDATAYOFS: GET_NPCDATA_VAR_INT32(yofs, "DrawYOffset"); break;
		case NPCDATAZOFS: GET_NPCDATA_VAR_INT32(zofs, "DrawZOffset"); break;
		case NPCDATAHXOFS: GET_NPCDATA_VAR_INT32(hxofs, "HitXOffset"); break;
		case NPCDATAHYOFS: GET_NPCDATA_VAR_INT32(hyofs, "HitYOffset"); break;
		case NPCDATAHITWIDTH: GET_NPCDATA_VAR_INT32(hxsz, "HitWidth"); break;
		case NPCDATAHITHEIGHT: GET_NPCDATA_VAR_INT32(hysz, "HitHeight"); break;
		case NPCDATAHITZ: GET_NPCDATA_VAR_INT32(hzsz, "HitZHeight"); break;
		case NPCDATASCRIPT: GET_NPCDATA_VAR_INT32(script, "Script"); break;
		case NPCDATATILEWIDTH: GET_NPCDATA_VAR_INT32(txsz, "TileWidth"); break;
		case NPCDATATILEHEIGHT: GET_NPCDATA_VAR_INT32(tysz, "TileHeight"); break;
		case NPCDATAWPNSPRITE: GET_NPCDATA_VAR_INT32(wpnsprite, "WeaponSprite"); break;
		case NPCDATAWEAPONSCRIPT: 
		{
			if( (unsigned) ri->npcdataref > (MAXNPCS-1) )
			{
				Z_scripterrlog("Invalid NPC ID passed to npcdata->WeaponScript: %d\n", (ri->npcdataref*10000));
				ret = -10000;
			}
			else ret = (guysbuf[ri->npcdataref].weap_data.script *10000);
			break;
		}
		case NPCDATASIZEFLAG: GET_NPCDATA_VAR_INT32(SIZEflags, "SizeFlags"); break;

		case NPCDATAFROZENTILE: GET_NPCDATA_VAR_INT32(frozentile, "FrozenTile"); break;
		case NPCDATAFROZENCSET: GET_NPCDATA_VAR_INT32(frozencset, "FrozenCSet"); break;
		case NPCDATAFIRESFX: GET_NPCDATA_VAR_BYTE(firesfx, "WeaponSFX"); break;

		case NPCDSHADOWSPR:
		{
			if(ri->npcdataref > (MAXNPCS-1) ) 
			{ 
				Z_scripterrlog("Invalid NPC ID passed to npcdata->ShadowSprite: %d\n", (ri->npcdataref*10000));
				ret = -10000; 
			} 
			else 
			{
				ret = guysbuf[ri->npcdataref].spr_shadow * 10000;
			} 
			break;
		}
		case NPCDSPAWNSPR:
		{
			if(ri->npcdataref > (MAXNPCS-1) ) 
			{ 
				Z_scripterrlog("Invalid NPC ID passed to npcdata->SpawnSprite: %d\n", (ri->npcdataref*10000));
				ret = -10000; 
			} 
			else 
			{
				ret = guysbuf[ri->npcdataref].spr_spawn * 10000;
			} 
			break;
		}
		case NPCDDEATHSPR:
		{
			if(ri->npcdataref > (MAXNPCS-1) ) 
			{ 
				Z_scripterrlog("Invalid NPC ID passed to npcdata->DeathSprite: %d\n", (ri->npcdataref*10000));
				ret = -10000; 
			} 
			else 
			{
				ret = guysbuf[ri->npcdataref].spr_death * 10000;
			} 
			break;
		}
		
		case NPCMATCHINITDLABEL: 	 //Same form as SetScreenD()
			//bool npcdata->MatchInitDLabel("label", d)
		{
			
			if( (unsigned) ri->npcdataref > (MAXNPCS-1) ) \
			{ 
				Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", "MatchInitDLabel()", (ri->npcdataref*10000));
				ret = 0; 
				break;
			} 
			
			int32_t arrayptr = get_register(sarg1);
			int32_t init_d_index = get_register(sarg2) / 10000;
			
			string name;
			ArrayH::getString(arrayptr, name, 256); // What's the limit on name length?
			
			bool match = (!( strcmp(name.c_str(), guysbuf[ri->npcdataref].initD_label[init_d_index] )));
			
			ret = ( match ? 10000 : 0 );
			break;
		}
		
		///----------------------------------------------------------------------------------------------------//
		//Dropset Variables

		case DROPSETNULLCHANCE:
		{
			if(ri->dropsetref > MAXITEMDROPSETS)
			{
				Z_scripterrlog("Invalid dropset pointer %d\n", ri->dropsetref);
				ret = -10000;
				break;
			}
			ret = item_drop_sets[ri->dropsetref].chance[0] * 10000;
			break;
		}
		case DROPSETCHOOSE:
		{
			if(ri->dropsetref > MAXITEMDROPSETS)
			{
				Z_scripterrlog("Invalid dropset pointer %d\n", ri->dropsetref);
				ret = -10000;
				break;
			}
			ret = select_dropitem(ri->dropsetref) * 10000;
			break;
		}
			
		///----------------------------------------------------------------------------------------------------//
		//Audio Variables

		case AUDIOPAN:
		{
			ret = FFScript::do_getSFX_pan() * 10000;
			break;
		}

		///----------------------------------------------------------------------------------------------------//
		//Graphics->

		case NUMDRAWS:
			ret = script_drawing_commands.Count() * 10000;
			//ret = FFCore.numscriptdraws * 10000; // This isn't updated until end of frame, making it useless!
			break;

		case MAXDRAWS:
			ret = MAX_SCRIPT_DRAWING_COMMANDS * 10000;
			break;

		case BITMAPWIDTH:
		{
			if (auto bmp = user_bitmaps.check(ri->bitmapref); bmp && bmp->u_bmp)
			{
				ret = bmp->width * 10000;
			}
			else
			{
				ret = -10000;
			}
			break;
		}

		case BITMAPHEIGHT:
		{
			if (auto bmp = user_bitmaps.check(ri->bitmapref); bmp && bmp->u_bmp)
			{
				ret = bmp->height * 10000;
			}
			else
			{
				ret = -10000;
			}
			break;
		}
		///----------------------------------------------------------------------------------------------------//
		//File->
		case FILEPOS:
		{
			if(user_file* f = checkFile(ri->fileref, true))
			{
				ret = ftell(f->file); //NOT *10000 -V
			}
			else ret = -10000L;
			break;
		}
		case FILEEOF:
		{
			if(user_file* f = checkFile(ri->fileref, true))
			{
				ret = feof(f->file) ? 10000L : 0L; //Boolean
			}
			else ret = -10000L;
			break;
		}
		case FILEERR:
		{
			if(user_file* f = checkFile(ri->fileref, true))
			{
				ret = ferror(f->file) * 10000L;
			}
			else ret = -10000L;
			break;
		}
		
		///----------------------------------------------------------------------------------------------------//
		//Directory->
		case DIRECTORYSIZE:
		{
			if(user_dir* dr = checkDir(ri->directoryref, true))
			{
				ret = dr->size() * 10000L;
			}
			else ret = -10000L;
			break;
		}
		
		///----------------------------------------------------------------------------------------------------//
		//Stack->
		case STACKSIZE:
		{
			if(user_stack* st = checkStack(ri->stackref, true))
			{
				ret = st->size(); //NOT *10000
			}
			else ret = -10000L;
			break;
		}
		case STACKFULL:
		{
			if(user_stack* st = checkStack(ri->stackref, true))
			{
				ret = st->full() ? 10000L : 0L;
			}
			else ret = -10000L;
			break;
		}
		
		///----------------------------------------------------------------------------------------------------//
		//Misc./Internal
		case REFFFC:
			ret = ZScriptVersion::ffcRefIsSpriteId() ? ri->ffcref : ri->ffcref * 10000;
			break;
			
		case REFITEM:
			ret = ri->itemref;
			break;
			
		case REFITEMCLASS:
			ret = ri->idata;
			break;
			
		case REFLWPN:
			ret = ri->lwpn;
			break;
			
		case REFEWPN:
			ret = ri->ewpn;
			break;
			
		case REFNPC:
			ret = ri->guyref;
			break;
		
		case REFSPRITE:
			ret = ri->spriteref;
			break;
		
		case REFMAPDATA: ret = ri->mapsref; break;
		case REFSCREENDATA: ret = ri->screenref; break;
		case REFCOMBODATA: ret = ri->combosref; break;
		case REFCOMBOTRIGGER: ret = ri->combotrigref; break;
		case REFSPRITEDATA: ret = ri->spritedataref; break;
		case REFBITMAP: ret = ri->bitmapref; break;
		case REFNPCCLASS: ret = ri->npcdataref; break;
		
		
		case REFDMAPDATA: ret = ri->dmapsref; break;
		case REFSHOPDATA: ret = ri->shopsref; break;
		case REFMSGDATA: ret = ri->zmsgref; break;
		
		case REFDROPS: ret = ri->dropsetref; break;
		case REFBOTTLETYPE: ret = ri->bottletyperef; break;
		case REFBOTTLESHOP: ret = ri->bottleshopref; break;
		case REFGENERICDATA: ret = ri->genericdataref; break;
		case REFFILE: ret = ri->fileref; break;
		case REFDIRECTORY: ret = ri->directoryref; break;
		case REFSTACK: ret = ri->stackref; break;
		case REFSUBSCREEN: ret = ri->subdataref; break;
		case REFSUBSCREENPAGE: ret = ri->subpageref; break;
		case REFSUBSCREENWIDG: ret = ri->subwidgref; break;
		case REFRNG: ret = ri->rngref; break;
		case REFWEBSOCKET: ret = ri->websocketref; break;
		case CLASS_THISKEY: ret = ri->thiskey; break;
		case CLASS_THISKEY2: ret = ri->thiskey2; break;
		case REFPALDATA: ret = ri->paldataref; break;
		
			
		case SP:
			ret = ri->sp * 10000;
			break;
		case SP2:
			ret = ri->sp;
			break;
			
		case PC:
			ret = ri->pc;
			break;
		
		case SWITCHKEY:
			ret = ri->switchkey;
			break;
			
		case SCRIPTRAM:
		case GLOBALRAM:
			ret = ArrayH::getElement(ri->d[rINDEX], ri->d[rINDEX2] / 10000);
			break;
			
		case SCRIPTRAMD:
		case GLOBALRAMD:
			ret = ArrayH::getElement(ri->d[rINDEX], 0);
			break;
			
		case GDD: // Unused, remove?
			ret = read_array(game->global_d, ri->d[rINDEX] / 10000);
			break;

		///----------------------------------------------------------------------------------------------------//
		
		case GENDATARUNNING:
		{
			ret = 0;
			if(user_genscript* scr = checkGenericScr(ri->genericdataref))
			{
				ret = scr->doscript() ? 10000L : 0L;
			}
			break;
		}
		case GENDATASIZE:
		{
			ret = 0;
			if(user_genscript* scr = checkGenericScr(ri->genericdataref))
			{
				ret = scr->dataSize()*10000;
			}
			break;
		}
		
		///----------------------------------------------------------------------------------------------------//
		
		case PORTALX:
		{
			ret = -10000;
			if(portal* p = checkPortal(ri->portalref))
				ret = p->x.getZLong();
			break;
		}
		case PORTALY:
		{
			ret = -10000;
			if(portal* p = checkPortal(ri->portalref))
				ret = p->y.getZLong();
			break;
		}
		case PORTALDMAP:
		{
			ret = -10000;
			if(portal* p = checkPortal(ri->portalref))
				ret = p->destdmap*10000;
			break;
		}
		case PORTALSCREEN:
		{
			ret = -10000;
			if(portal* p = checkPortal(ri->portalref))
				ret = p->destscr*10000;
			break;
		}
		case PORTALACLK:
		{
			ret = -10000;
			if(portal* p = checkPortal(ri->portalref))
				ret = p->aclk*10000;
			break;
		}
		case PORTALAFRM:
		{
			ret = -10000;
			if(portal* p = checkPortal(ri->portalref))
				ret = p->aframe*10000;
			break;
		}
		case PORTALOTILE:
		{
			ret = -10000;
			if(portal* p = checkPortal(ri->portalref))
				ret = p->o_tile*10000;
			break;
		}
		case PORTALASPD:
		{
			ret = -10000;
			if(portal* p = checkPortal(ri->portalref))
				ret = p->aspd*10000;
			break;
		}
		case PORTALFRAMES:
		{
			ret = -10000;
			if(portal* p = checkPortal(ri->portalref))
				ret = p->frames*10000;
			break;
		}
		case PORTALSAVED:
		{
			ret = 0;
			if(portal* p = checkPortal(ri->portalref))
				ret = p->saved_data;
			break;
		}
		case PORTALCLOSEDIS:
		{
			ret = 0;
			if(portal* p = checkPortal(ri->portalref))
				ret = p->prox_active ? 0 : 10000; //Inverted
			break;
		}
		case REFPORTAL:
		{
			ret = ri->portalref;
			break;
		}
		case REFSAVPORTAL:
		{
			ret = ri->saveportalref;
			break;
		}
		case PORTALWARPSFX:
		{
			ret = 0;
			if(portal* p = checkPortal(ri->portalref))
				ret = p->wsfx ? 0 : 10000;
			break;
		}
		case PORTALWARPVFX:
		{
			ret = 0;
			if(portal* p = checkPortal(ri->portalref))
				ret = p->weffect ? 0 : 10000;
			break;
		}
		case SAVEDPORTALX:
		{
			ret = -10000;
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				ret = p->x;
			break;
		}
		case SAVEDPORTALY:
		{
			ret = -10000;
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				ret = p->y;
			break;
		}
		case SAVEDPORTALSRCDMAP:
		{
			ret = -10000;
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				ret = p->srcdmap * 10000;
			break;
		}
		case SAVEDPORTALDESTDMAP:
		{
			ret = -10000;
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				ret = p->destdmap * 10000;
			break;
		}
		case SAVEDPORTALSRCSCREEN:
		{
			ret = -10000;
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				ret = p->srcscr * 10000;
			break;
		}
		case SAVEDPORTALDSTSCREEN:
		{
			ret = -10000;
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				ret = p->destscr * 10000;
			break;
		}
		case SAVEDPORTALWARPSFX:
		{
			ret = -10000;
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				ret = p->sfx * 10000;
			break;
		}
		case SAVEDPORTALWARPVFX:
		{
			ret = -10000;
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				ret = p->warpfx * 10000;
			break;
		}
		case SAVEDPORTALSPRITE:
		{
			ret = -10000;
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				ret = p->spr * 10000;
			break;
		}
		case SAVEDPORTALPORTAL:
		{
			ret = 0;
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				ret = getPortalFromSaved(p);
			break;
		}
		case PORTALCOUNT:
		{
			ret = portals.Count()*10000;
			break;
		}
		case SAVEDPORTALCOUNT:
		{
			ret = game->user_portals.size()*10000;
			break;
		}
		
		case GAMEASUBOPEN:
		{
			ret = subscreen_open ? 10000 : 0;
			break;
		}
		case GAMEASUBYOFF:
		{
			ret = active_sub_yoff*10000;
			break;
		}
		case GAMENUMASUB:
		{
			ret = subscreens_active.size()*10000;
			break;
		}
		case GAMENUMPSUB:
		{
			ret = subscreens_passive.size()*10000;
			break;
		}
		case GAMENUMOSUB:
		{
			ret = subscreens_overlay.size()*10000;
			break;
		}
		
		///----------------------------------------------------------------------------------------------------//
		
		case SUBDATACURPG:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref))
				if(sub->sub_type == sstACTIVE)
					ret = 10000*sub->curpage;
			break;
		}
		case SUBDATANUMPG:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref))
			{
				if(sub->sub_type == sstACTIVE)
					ret = 10000*sub->pages.size();
				else ret = 10000;
			}
			break;
		}
		case SUBDATATYPE:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref))
				ret = sub->sub_type*10000;
			break;
		}

		///---- ACTIVE SUBSCREENS ONLY
		case SUBDATACURSORPOS:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
			{
				SubscrPage& pg = sub->cur_page();
				ret = pg.cursor_pos * 10000;
			}
			break;
		}
		case SUBDATASCRIPT:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
				ret = sub->script * 10000;
			break;
		}
		case SUBDATATRANSLEFTTY:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
			{
				auto& trans = sub->trans_left;
				ret = trans.type * 10000;
			}
			break;
		}
		case SUBDATATRANSLEFTSFX:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
			{
				auto& trans = sub->trans_left;
				ret = trans.tr_sfx * 10000;
			}
			break;
		}
		case SUBDATATRANSRIGHTTY:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
			{
				auto& trans = sub->trans_right;
				ret = trans.type * 10000;
			}
			break;
		}
		case SUBDATATRANSRIGHTSFX:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
			{
				auto& trans = sub->trans_right;
				ret = trans.tr_sfx * 10000;
			}
			break;
		}
		case SUBDATASELECTORDSTX:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
				ret = sub->selector_setting.x * 10000;
			break;
		}
		case SUBDATASELECTORDSTY:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
				ret = sub->selector_setting.y * 10000;
			break;
		}
		case SUBDATASELECTORDSTW:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
				ret = sub->selector_setting.w * 10000;
			break;
		}
		case SUBDATASELECTORDSTH:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
				ret = sub->selector_setting.h * 10000;
			break;
		}
		///---- CURRENTLY OPEN ACTIVE SUBSCREEN ONLY
		case SUBDATATRANSCLK:
		{
			ret = -10000;
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
			{
				if(sub != new_subscreen_active)
					Z_scripterrlog("'subscreendata->TransClock' is only"
						" valid for the current active subscreen!\n");
				else if(subscreen_open && subscr_pg_animating)
					ret = subscr_pg_clk*10000;
			}
			break;
		}
		case SUBDATATRANSTY:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
			{
				auto& trans = subscr_pg_transition;
				if(sub != new_subscreen_active)
					Z_scripterrlog("'subscreendata->TransType' is only"
						" valid for the current active subscreen!\n");
				else if(subscreen_open)
					ret = trans.type*10000;
			}
			break;
		}
		case SUBDATATRANSFROMPG:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
			{
				if(sub != new_subscreen_active)
					Z_scripterrlog("'subscreendata->TransFromPage' is only"
						" valid for the current active subscreen!\n");
				else if(subscreen_open)
					ret = subscr_pg_from*10000;
			}
			break;
		}
		case SUBDATATRANSTOPG:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
			{
				if(sub != new_subscreen_active)
					Z_scripterrlog("'subscreendata->TransToPage' is only"
						" valid for the current active subscreen!\n");
				else if(subscreen_open)
					ret = subscr_pg_to*10000;
			}
			break;
		}
		
		///----------------------------------------------------------------------------------------------------//
		case SUBPGINDEX: 
		{
			if(SubscrPage* pg = checkSubPage(ri->subpageref))
				ret = pg->getIndex() * 10000;
			break;
		}
		case SUBPGNUMWIDG: 
		{
			if(SubscrPage* pg = checkSubPage(ri->subpageref))
				ret = pg->size() * 10000;
			break;
		}
		case SUBPGSUBDATA: 
		{
			if(SubscrPage* pg = checkSubPage(ri->subpageref))
			{
				auto [sub,ty,_pgid,_ind] = from_subref(ri->subpageref);
				ret = get_subref(sub,ty,0,0);
			}
			break;
		}
		case SUBPGCURSORPOS: 
		{
			if(SubscrPage* pg = checkSubPage(ri->subpageref))
				ret = pg->cursor_pos * 10000;
			break;
		}
		///----------------------------------------------------------------------------------------------------//
		///---- ANY WIDGET TYPE
		case SUBWIDGTYPE:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				ret = 10000*widg->getType();
			break;
		}
		case SUBWIDGINDEX:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto [_sub,_ty,_pgid,ind] = from_subref(ri->subwidgref);
				ret = 10000*ind;
			}
			break;
		}
		case SUBWIDGDISPITM:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				ret = 10000*widg->getDisplayItem();
			}
			break;
		}
		case SUBWIDGEQPITM:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				ret = 10000*widg->getItemVal();
			}
			break;
		}
		case SUBWIDGPAGE:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto [sub,ty,pgid,_ind] = from_subref(ri->subwidgref);
				ret = get_subref(sub,ty,pgid,0);
			}
			break;
		}
		case SUBWIDGPOS:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				ret = 10000*widg->pos;
			break;
		}
		case SUBWIDGX:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				ret = 10000*widg->x;
			break;
		}
		case SUBWIDGY:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				ret = 10000*widg->y;
			break;
		}
		case SUBWIDGW:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				ret = 10000*widg->w;
			break;
		}
		case SUBWIDGH:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				ret = 10000*widg->h;
			break;
		}
		case SUBWIDG_DISPX:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				ret = 10000*widg->getX();
			break;
		}
		case SUBWIDG_DISPY:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				ret = 10000*widg->getY();
			break;
		}
		case SUBWIDG_DISPW:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				ret = 10000*widg->getW();
			break;
		}
		case SUBWIDG_DISPH:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				ret = 10000*widg->getH();
			break;
		}
		case SUBWIDGREQCOUNTER:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				ret = 10000 * widg->req_counter;
			break;
		}
		case SUBWIDGREQCOUNTERCOND:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				ret = 10000 * widg->req_counter_cond_type;
			break;
		}
		case SUBWIDGREQCOUNTERVAL:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				ret = 10000 * widg->req_counter_val;
			break;
		}
		case SUBWIDGREQLITEMS:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				ret = 10000 * widg->req_litems;
			break;
		}
		case SUBWIDGREQLITEMLEVEL:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				ret = 10000 * widg->req_litem_level;
			break;
		}
		case SUBWIDGREQSCRIPTDISABLED:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				ret = widg->is_disabled ? 10000 : 0;
			break;
		}
		///---- ACTIVE SUBSCREENS ONLY
		case SUBWIDGSELECTORDSTX:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref, sstACTIVE))
				ret = 10000*widg->selector_override.x;
			break;
		}
		case SUBWIDGSELECTORDSTY:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref, sstACTIVE))
				ret = 10000*widg->selector_override.y;
			break;
		}
		case SUBWIDGSELECTORDSTW:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref, sstACTIVE))
				ret = 10000*widg->selector_override.w;
			break;
		}
		case SUBWIDGSELECTORDSTH:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref, sstACTIVE))
				ret = 10000*widg->selector_override.h;
			break;
		}
				
		case SUBWIDGPRESSSCRIPT:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref, sstACTIVE))
				ret = 10000*widg->generic_script;
			break;
		}
		case SUBWIDGPGMODE:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref, sstACTIVE))
				ret = 10000*widg->pg_mode;
			break;
		}
		case SUBWIDGPGTARG:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref, sstACTIVE))
				ret = 10000*widg->pg_targ;
			break;
		}
		
		case SUBWIDGTRANSPGTY:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref, sstACTIVE))
			{
				auto& trans = widg->pg_trans;
				ret = 10000*trans.type;
			}
			break;
		}
		case SUBWIDGTRANSPGSFX:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref, sstACTIVE))
			{
				auto& trans = widg->pg_trans;
				ret = 10000*trans.tr_sfx;
			}
			break;
		}
		///---- VARYING WIDGET TYPES
		case SUBWIDGTY_FONT:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						ret = 10000*((SW_Text*)widg)->fontid;
						break;
					case widgTEXTBOX:
						ret = 10000*((SW_TextBox*)widg)->fontid;
						break;
					case widgSELECTEDTEXT:
						ret = 10000*((SW_SelectedText*)widg)->fontid;
						break;
					case widgTIME:
						ret = 10000*((SW_Time*)widg)->fontid;
						break;
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->fontid;
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->fontid;
						break;
					case widgOLDCTR:
						ret = 10000*((SW_Counters*)widg)->fontid;
						break;
					case widgMMAPTITLE:
						ret = 10000*((SW_MMapTitle*)widg)->fontid;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_ALIGN:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						ret = 10000*((SW_Text*)widg)->align;
						break;
					case widgTEXTBOX:
						ret = 10000*((SW_TextBox*)widg)->align;
						break;
					case widgSELECTEDTEXT:
						ret = 10000*((SW_SelectedText*)widg)->align;
						break;
					case widgTIME:
						ret = 10000*((SW_Time*)widg)->align;
						break;
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->align;
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->align;
						break;
					case widgMMAPTITLE:
						ret = 10000*((SW_MMapTitle*)widg)->align;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_SHADOWTY:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						ret = 10000*((SW_Text*)widg)->shadtype;
						break;
					case widgTEXTBOX:
						ret = 10000*((SW_TextBox*)widg)->shadtype;
						break;
					case widgSELECTEDTEXT:
						ret = 10000*((SW_SelectedText*)widg)->shadtype;
						break;
					case widgTIME:
						ret = 10000*((SW_Time*)widg)->shadtype;
						break;
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->shadtype;
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->shadtype;
						break;
					case widgOLDCTR:
						ret = 10000*((SW_Counters*)widg)->shadtype;
						break;
					case widgMMAPTITLE:
						ret = 10000*((SW_MMapTitle*)widg)->shadtype;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_TXT:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						ret = 10000*((SW_Text*)widg)->c_text.get_int_color();
						break;
					case widgTEXTBOX:
						ret = 10000*((SW_TextBox*)widg)->c_text.get_int_color();
						break;
					case widgSELECTEDTEXT:
						ret = 10000*((SW_SelectedText*)widg)->c_text.get_int_color();
						break;
					case widgTIME:
						ret = 10000*((SW_Time*)widg)->c_text.get_int_color();
						break;
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->c_text.get_int_color();
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->c_text.get_int_color();
						break;
					case widgOLDCTR:
						ret = 10000*((SW_Counters*)widg)->c_text.get_int_color();
						break;
					case widgMMAPTITLE:
						ret = 10000*((SW_MMapTitle*)widg)->c_text.get_int_color();
						break;
					case widgMCGUFF_FRAME:
						ret = 10000*((SW_TriFrame*)widg)->c_number.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_SHD:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						ret = 10000*((SW_Text*)widg)->c_shadow.get_int_color();
						break;
					case widgTEXTBOX:
						ret = 10000*((SW_TextBox*)widg)->c_shadow.get_int_color();
						break;
					case widgSELECTEDTEXT:
						ret = 10000*((SW_SelectedText*)widg)->c_shadow.get_int_color();
						break;
					case widgTIME:
						ret = 10000*((SW_Time*)widg)->c_shadow.get_int_color();
						break;
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->c_shadow.get_int_color();
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->c_shadow.get_int_color();
						break;
					case widgOLDCTR:
						ret = 10000*((SW_Counters*)widg)->c_shadow.get_int_color();
						break;
					case widgMMAPTITLE:
						ret = 10000*((SW_MMapTitle*)widg)->c_shadow.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_BG:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						ret = 10000*((SW_Text*)widg)->c_bg.get_int_color();
						break;
					case widgTEXTBOX:
						ret = 10000*((SW_TextBox*)widg)->c_bg.get_int_color();
						break;
					case widgSELECTEDTEXT:
						ret = 10000*((SW_SelectedText*)widg)->c_bg.get_int_color();
						break;
					case widgTIME:
						ret = 10000*((SW_Time*)widg)->c_bg.get_int_color();
						break;
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->c_bg.get_int_color();
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->c_bg.get_int_color();
						break;
					case widgOLDCTR:
						ret = 10000*((SW_Counters*)widg)->c_bg.get_int_color();
						break;
					case widgMMAPTITLE:
						ret = 10000*((SW_MMapTitle*)widg)->c_bg.get_int_color();
						break;
					case widgBGCOLOR:
						ret = 10000*((SW_Clear*)widg)->c_bg.get_int_color();
						break;
					case widgCOUNTERPERCBAR:
						ret = 10000*((SW_CounterPercentBar*)widg)->c_bg.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		
		case SUBWIDGTY_COLOR_TXT2:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->c_text2.get_int_color();
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->c_text2.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_SHD2:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->c_shadow2.get_int_color();
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->c_shadow2.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_BG2:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->c_bg2.get_int_color();
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->c_bg2.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_OLINE:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLINE:
						ret = 10000*((SW_Line*)widg)->c_line.get_int_color();
						break;
					case widgRECT:
						ret = 10000*((SW_Rect*)widg)->c_outline.get_int_color();
						break;
					case widgMCGUFF_FRAME:
						ret = 10000*((SW_TriFrame*)widg)->c_outline.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_FILL:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgRECT:
						ret = 10000*((SW_Rect*)widg)->c_fill.get_int_color();
						break;
					case widgCOUNTERPERCBAR:
						ret = 10000*((SW_CounterPercentBar*)widg)->c_fill.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_BUTTON:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgBTNITM:
						ret = 10000*((SW_ButtonItem*)widg)->btn;
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->btn;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_MINDIG:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->mindigits;
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->mindigits;
						break;
					case widgOLDCTR:
						ret = 10000*((SW_Counters*)widg)->digits;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_MAXDIG:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->maxdigits;
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->maxdigits;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_INFITM:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->infitm;
						break;
					case widgOLDCTR:
						ret = 10000*((SW_Counters*)widg)->infitm;
						break;
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->inf_item;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_INFCHAR:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						ret = 10000*byte(((SW_Counter*)widg)->infchar);
						break;
					case widgOLDCTR:
						ret = 10000*byte(((SW_Counters*)widg)->infchar);
						break;
					case widgBTNCOUNTER:
						ret = 10000*byte(((SW_BtnCounter*)widg)->infchar);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COSTIND:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->costind;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -1;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_PLAYER:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						ret = 10000*((SW_MMap*)widg)->c_plr.get_int_color();
						break;
					case widgLMAP:
						ret = 10000*((SW_LMap*)widg)->c_plr.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_CMPBLNK:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						ret = 10000*((SW_MMap*)widg)->c_cmp_blink.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_CMPOFF:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						ret = 10000*((SW_MMap*)widg)->c_cmp_off.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_ROOM:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLMAP:
						ret = 10000*((SW_LMap*)widg)->c_room.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_ITEMCLASS:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgITEMSLOT:
						ret = 10000*((SW_ItemSlot*)widg)->iclass;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_ITEMID:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgITEMSLOT:
						ret = 10000*((SW_ItemSlot*)widg)->iid;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_FRAMETILE:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF_FRAME:
						ret = 10000*((SW_TriFrame*)widg)->frame_tile;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_FRAMECSET:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF_FRAME:
						ret = 10000*((SW_TriFrame*)widg)->frame_cset;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_PIECETILE:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF_FRAME:
						ret = 10000*((SW_TriFrame*)widg)->piece_tile;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_PIECECSET:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF_FRAME:
						ret = 10000*((SW_TriFrame*)widg)->piece_cset;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_FLIP:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF:
						ret = 10000*((SW_McGuffin*)widg)->flip;
						break;
					case widgTILEBLOCK:
						ret = 10000*((SW_TileBlock*)widg)->flip;
						break;
					case widgMINITILE:
						ret = 10000*((SW_MiniTile*)widg)->flip;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_NUMBER:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF:
						ret = 10000*((SW_McGuffin*)widg)->number;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_FRAMES:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->frames;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_SPEED:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->speed;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_DELAY:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->delay;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_CONTAINER:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->container;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_GAUGE_WID:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						ret = 10000*(((SW_GaugePiece*)widg)->gauge_wid+1);
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_GAUGE_HEI:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						ret = 10000*(((SW_GaugePiece*)widg)->gauge_hei+1);
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_UNITS:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						ret = 10000*(((SW_GaugePiece*)widg)->unit_per_frame+1);
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_HSPACE:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->hspace;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_VSPACE:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->vspace;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_GRIDX:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->grid_xoff;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_GRIDY:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->grid_yoff;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_ANIMVAL:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->anim_val;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_SHOWDRAIN:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMGAUGE:
						ret = 10000*((SW_MagicGaugePiece*)widg)->showdrain;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_PERCONTAINER:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMISCGAUGE:
						ret = 10000*((SW_MiscGaugePiece*)widg)->per_container;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_TABSIZE:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXTBOX:
						ret = 10000*((SW_TextBox*)widg)->tabsize;
						break;
					case widgSELECTEDTEXT:
						ret = 10000*((SW_SelectedText*)widg)->tabsize;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_LITEMS:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						ret = 10000*((SW_MMap*)widg)->compass_litems;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}

		default:
		{
			if (zasm_array_supports(arg))
			{
				int ref_arg = get_register_ref_dependency(arg).value_or(0);
				int ref = ref_arg ? get_ref(ref_arg) : 0;
				ret = zasm_array_get(arg, ref, ri->d[rINDEX] / 10000);
			}
			else if (auto r = scripting_engine_get_register(arg))
				ret = *r;
			break;
		}
	}

	current_zasm_register = 0;

	return ret;
}

//Setter Instructions


void set_register(int32_t arg, int32_t value)
{
	if (arg >= D(0) && arg <= D(7))
	{
		ri->d[arg - D(0)] = value;
		return;
	}
	else if (arg >= GD(0) && arg <= GD(MAX_SCRIPT_REGISTERS))
	{
		game->global_d[arg-GD(0)] = value;
		return;
	}

	//Macros
	
	#define	SET_SPRITEDATA_VAR_INT(member, str) \
	{ \
		if(unsigned(ri->spritedataref) > (MAXWPNS-1) ) \
		{ \
			Z_scripterrlog("Invalid Sprite ID passed to spritedata->%s: %d\n", str, (ri->spritedataref*10000)); \
		} \
		else \
		{ \
			wpnsbuf[ri->spritedataref].member = vbound((value / 10000),0,214747); \
		} \
	} \

	#define	SET_SPRITEDATA_VAR_BYTE(member, str) \
	{ \
		if(unsigned(ri->spritedataref) > (MAXWPNS-1) ) \
		{ \
			Z_scripterrlog("Invalid Sprite ID passed to spritedata->%s: %d\n", str, (ri->spritedataref*10000)); \
		} \
		else \
		{ \
			wpnsbuf[ri->spritedataref].member = vbound((value / 10000),0,255); \
		} \
	} \

	current_zasm_register = arg;

	// Do not ever use `return` in these cases!
	switch(arg)
	{
	///----------------------------------------------------------------------------------------------------//
	//FFC Variables
		case DATA:
			if (auto ffc = ResolveFFC(ri->ffcref))
			{
				zc_ffc_set(*ffc, vbound(value/10000,0,MAXCOMBOS-1));
			}
			break;
		
		case FFSCRIPT:
			if (auto ffc = ResolveFFC(ri->ffcref))
			{
				ffc->script = vbound(value/10000, 0, NUMSCRIPTFFC-1);
				for(int32_t i=0; i<16; i++)
					ffc->miscellaneous[i] = 0;
				if (get_qr(qr_CLEARINITDONSCRIPTCHANGE))
				{
					for(int32_t i=0; i<8; i++)
						ffc->initd[i] = 0;
				}
				on_reassign_script_engine_data(ScriptType::FFC, ffc->index);
			}
			break;
			
			
		case FCSET:
			if (auto ffc = ResolveFFC(ri->ffcref))
				ffc->cset = (value/10000)&15;
			break;
			
		case DELAY:
			if (auto ffc = ResolveFFC(ri->ffcref))
				ffc->delay = value/10000;
			break;
			
		case FX:
			if (auto ffc = ResolveFFC(ri->ffcref))
				ffc->x = zslongToFix(value);
			break;
			
		case FY:
			if (auto ffc = ResolveFFC(ri->ffcref))
				ffc->y=zslongToFix(value);
			break;
			
		case XD:
			if (auto ffc = ResolveFFC(ri->ffcref))
				ffc->vx=zslongToFix(value);
			break;
			
		case YD:
			if (auto ffc = ResolveFFC(ri->ffcref))
				ffc->vy=zslongToFix(value);
			break;
		
		case FFCID:
			break;
			
		case XD2:
			if (auto ffc = ResolveFFC(ri->ffcref))
				ffc->ax=zslongToFix(value);
			break;
			
		case YD2:
			if (auto ffc = ResolveFFC(ri->ffcref))
				ffc->ay=zslongToFix(value);
			break;
			
		case FFCWIDTH:
			if (auto ffc = ResolveFFC(ri->ffcref))
				ffc->hit_width = (value/10000);
			break;
			
		case FFCHEIGHT:
			if (auto ffc = ResolveFFC(ri->ffcref))
				ffc->hit_height = (value/10000);
			break;
			
		case FFTWIDTH:
			if (auto ffc = ResolveFFC(ri->ffcref))
				ffc->txsz = vbound(value/10000, 1, 4);
			break;
			
		case FFTHEIGHT:
			if (auto ffc = ResolveFFC(ri->ffcref))
				ffc->tysz = vbound(value/10000, 1, 4);
			break;
			
		case FFCLAYER:
			if (auto ffc = ResolveFFC(ri->ffcref))
				ffc->layer = vbound(value/10000, 0, 7);
			break;
			
		case FFLINK:
			if (auto ffc = ResolveFFC(ri->ffcref))
				(ffc->link)=vbound(value/10000, 0, MAXFFCS-1); // Allow "ffc->Link = 0" to unlink ffc.
			//0 is none, setting this before made it impssible to clear it. -Z
			break;
			
		case FFCLASTCHANGERX:
			if (auto ffc = ResolveFFC(ri->ffcref) )
				ffc->changer_x=vbound(zslongToFix(value).getInt(),-32768, 32767);
			break;
			
		case FFCLASTCHANGERY:
			if (auto ffc = ResolveFFC(ri->ffcref) )
				ffc->changer_y=vbound(zslongToFix(value).getInt(),-32768, 32767);
			break;
		
			
			
	///----------------------------------------------------------------------------------------------------//
	//Hero's Variables
		case LINKX:
		{
			if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
			{
				Hero.setXfix(zslongToFix(value));
			}
			else
			{
				Hero.setX(value/10000);
			}
		}
		break;
		
		case LINKCSET:
		{
			Hero.cs = value/10000;
			break;
		}
		case LINKY:
		{
			if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
			{
				Hero.setYfix(zslongToFix(value));
			}
			else
			{
				Hero.setY(value/10000);
			}
		}
		break;
			
		case LINKZ:
		{
			if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
			{
				Hero.setZfix(zslongToFix(value));
			}
			else
			{
				Hero.setZ(value/10000);
			}
		}
		break;
			
		case LINKJUMP:
			Hero.setJump(zslongToFix(value));
			break;
		
		case HEROFAKEJUMP:
			Hero.setFakeFall(zslongToFix(value) * -100);
			break;
			
		case LINKDIR:
		{
			//Hero->setDir() calls reset_hookshot(), which removes the sword sprite.. O_o
			if(Hero.getAction() == attacking || Hero.getAction() == sideswimattacking) Hero.dir = (value/10000);
			else Hero.setDir(value/10000);
			
			break;
		}
		
		case LINKHITDIR:
			Hero.setHitDir(value / 10000);
			break;
		
		case LINKGRAVITY:
			if(value)
				Hero.moveflags |= move_obeys_grav;
			else
				Hero.moveflags &= ~move_obeys_grav;
			break;
		
		case HERONOSTEPFORWARD:
			FFCore.nostepforward = ( (value) ? 1 : 0 ); 
			break;
			
		case LINKHP:
			game->set_life(zc_max(0, zc_min(value/10000,game->get_maxlife())));
			break;
			
		case LINKMP:
			game->set_magic(zc_max(0, zc_min(value/10000,game->get_maxmagic())));
			break;
			
		case LINKMAXHP:
			game->set_maxlife(value/10000);
			break;
			
		case LINKMAXMP:
			game->set_maxmagic(value/10000);
			break;
			
		case LINKACTION:
		{
			int32_t act = value / 10000;
			switch(act)
			{
				case hookshotout:
				case stunned:
				case ispushing:
					FFCore.setHeroAction(act);
					break;
				default:
					Hero.setAction((actiontype)(act));
			}
			//Protect from writing illegal actions to Hero's raw variable. 
			//in the future, we can move all scripted actions that are not possible
			//to set in ZC into this mechanic. -Z
			break;
		}
			
		case HEROHEALTHBEEP:
		{
			int32_t beep = vbound((value/10000),-4, 255); 
			//-2 suspends system control of stopping the sound
			//-3 suspends system control of stopping the sound AND suspends
			// system control over starting or playing it.
			heart_beep_timer = beep;
			if ( heart_beep_timer > -1 )
			{
				cont_sfx(QMisc.miscsfx[sfxLOWHEART]);
			}
			else
			{
				stop_sfx(QMisc.miscsfx[sfxLOWHEART]);
			}
			break;
		}
		
		case LINKHELD:
			Hero.setHeldItem(vbound(value/10000,0,MAXITEMS-1));
			break;
		
		case HEROSTEPRATE:
			if(!get_qr(qr_NEW_HERO_MOVEMENT))
			{
				Z_scripterrlog("To use '%s', you must %s the quest rule '%s'.", "Hero->Step", "enable", "New Hero Movement");
			}
			Hero.setStepRate(zc_max(value/10000,0));
			if(!get_qr(qr_SCRIPT_WRITING_HEROSTEP_DOESNT_CARRY_OVER))
				zinit.heroStep = Hero.getStepRate();
			break;
		case HEROSHOVEOFFSET:
			if(!get_qr(qr_NEW_HERO_MOVEMENT2))
				Z_scripterrlog("To use 'Hero->ShoveOffset', you must enable the quest rule 'Newer Hero Movement'.");
			Hero.shove_offset = vbound(zslongToFix(value),16_zf,0_zf);
			if(!get_qr(qr_SCRIPT_WRITING_HEROSTEP_DOESNT_CARRY_OVER))
				zinit.shove_offset = Hero.shove_offset;
			break;
			
		case LINKEQUIP:
		{
			if ( FFCore.getQuestHeaderInfo(vZelda) == 0x250 && FFCore.getQuestHeaderInfo(vBuild) < 33 )
			{
				break;
			}
			//int32_t seta = (value/10000) >> 8; int32_t setb = value/10000) & 0xFF;
			int32_t setb = ((value/10000)&0xFF00)>>8, seta = (value/10000)&0xFF;
			seta = vbound(seta,-1,255);
			setb = vbound(setb,-1,255);
				
			Awpn = seta;
			game->awpn = 255;
			game->forced_awpn = seta;
			game->items_off[seta] = 0;
			directItemA = seta;
			
			Bwpn = setb;
			game->bwpn = 255;
			game->forced_bwpn = setb;
			game->items_off[setb] = 0;
			directItemB = setb;
			break;
		}
		
		  
		case SETITEMSLOT:
		{
			//ri->d[rINDEX2] = 1st arg
			//ri->d[rINDEX] = 2nd arg
			//value = third arg
			//int32_t item, int32_t slot, int32_t force
			int32_t itm = ri->d[rINDEX]/10000;
			itm = vbound(itm, -1, 255);
			
			int32_t slot = ri->d[rINDEX2]/10000;
			int32_t force = ri->d[rEXP1]/10000;
			
			//If we add more item buttons, slot should be an int32_t
			//and force shuld be an int32_t
			
			/*
				For zScript, 
					const int32_t ITM_REQUIRE_NONE = 0
					const int32_t ITM_REQUIRE_INVENTORY = 1
					const int32_t ITM_REQUIRE_A_SLOT_RULE = 2
					//Combine as flags
			*/
			if ( force == 0 )
			{
				switch(slot)
				{
					case 0: //b
						Bwpn = itm;
						game->items_off[itm] = 0;
						game->bwpn = 255;
						game->forced_bwpn = itm;
						directItemB = itm;
						break;
					
					case 1: //a
						Awpn = itm;
						game->items_off[itm] = 0;
						game->awpn = 255;
						game->forced_awpn = itm;
						directItemA = itm;
						break;
					
					case 2: //x
						Xwpn = itm;
						game->items_off[itm] = 0;
						game->xwpn = 255;
						game->forced_xwpn = itm;
						directItemX = itm;
						break;
					
					case 3: //y
						Ywpn = itm;
						game->items_off[itm] = 0;
						game->ywpn = 255;
						game->forced_ywpn = itm;
						directItemX = itm;
						break;
				}
			}
			else if ( force == 1 )
			{
				if (game->item[itm])
				{
					switch(slot)
					{
						case 0: //b
							Bwpn = itm;
							game->items_off[itm] = 0;
							game->bwpn = 255;
							game->forced_bwpn = itm;
							directItemB = itm;
							break;
						
						case 1: //a
							Awpn = itm;
							game->items_off[itm] = 0;
							game->awpn = 255;
							game->forced_awpn = itm;
							directItemA = itm;
							break;
						
						case 2: //x
							Xwpn = itm;
							game->items_off[itm] = 0;
							game->xwpn = 255;
							game->forced_xwpn = itm;
							directItemX = itm;
							break;
						
						case 3: //y
							Ywpn = itm;
							game->items_off[itm] = 0;
							game->ywpn = 255;
							game->forced_ywpn = itm;
							directItemY = itm;
							break;
					}
				}
			}
			else if ( force == 2 )
			{
				switch(slot)
				{
					case 0: //b
						Bwpn = itm;
						game->items_off[itm] = 0;
						game->bwpn = 255;
						game->forced_bwpn = itm;
						directItemB = itm;
						break;
					
					case 1: //a
						if (get_qr(qr_SELECTAWPN))
						{
							Awpn = itm;
							game->items_off[itm] = 0;
							game->awpn = 255;
							game->forced_awpn = itm;
							directItemA = itm;
						}
						break;
					
					case 2:  //x
						Xwpn = itm;
						game->items_off[itm] = 0;
						game->xwpn = 255;
						game->forced_xwpn = itm;
						directItemX = itm;
						break;
					
					case 3: //y
						Ywpn = itm;
						game->items_off[itm] = 0;
						game->ywpn = 255;
						game->forced_ywpn = itm;
						directItemY = itm;
						break;
				}
			}
			else if ( force == 3 ) //Flag ITM_REQUIRE_INVENTORY + ITM_REQUIRE_SLOT_A_RULE
			{
				if ( game->item[itm] )
				{
					switch(slot)
					{
						case 0: //b
							Bwpn = itm;
							game->items_off[itm] = 0;
							game->bwpn = 255;
							game->forced_bwpn = itm;
							directItemB = itm;
							break;
						
						case 1: //a
							if (get_qr(qr_SELECTAWPN))
							{
								Awpn = itm;
								game->items_off[itm] = 0;
								game->awpn = 255;
								game->forced_awpn = itm;
								directItemA = itm;
							}
							break;
						
						case 2: //x
							Xwpn = itm;
							game->items_off[itm] = 0;
							game->xwpn = 255;
							game->forced_xwpn = itm;
							directItemX = itm;
							break;
						
						case 3: //y
							Ywpn = itm;
							game->items_off[itm] = 0;
							game->ywpn = 255;
							game->forced_ywpn = itm;
							directItemY = itm;
							break;
					}
				}
			}
		}
		break;
		  
		case LINKINVIS:
			Hero.setDontDraw((value ? 2 : 0));
			break;
			
		case LINKINVINC:
			Hero.scriptcoldet=(value/10000);
			break;
		
		case LINKENGINEANIMATE:
			Hero.do_animation=value;
			break;
			
		case LINKSWORDJINX:
			Hero.setSwordClk(value/10000);
			break;
			
		case LINKITEMJINX:
			Hero.setItemClk(value/10000);
			break;
			
		case LINKDRUNK:
			Hero.setDrunkClock(value/10000);
			break;

		case LINKHXOFS:
			(Hero.hxofs)=(zfix)(value/10000);
			break;

		case LINKROTATION:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				break;
			}
			(Hero.rotation)=(value/10000);
			break;
		
		case LINKSCALE:
		{
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				break;
			}
			(Hero.scale)=(value/100.0);
			break;
		}

		case LINKHYOFS:
			(Hero.hyofs)=(zfix)(value/10000);
			break;
			
		case LINKXOFS:
			(Hero.xofs)=(zfix)(value/10000);
			break;
			
		case LINKYOFS:
			(Hero.yofs)=(zfix)(value/10000)+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
			break;
		case HEROTOTALDYOFFS:
			break; //READ-ONLY
			
		case HEROSHADOWXOFS:
			(Hero.shadowxofs)=(zfix)(value/10000);
			break;
			
		case HEROSHADOWYOFS:
			(Hero.shadowyofs)=(zfix)(value/10000);
			break;
			
		case LINKZOFS:
			(Hero.zofs)=(zfix)(value/10000);
			break;
			
		case LINKHXSZ:
			(Hero.hit_width)=(zfix)(value/10000);
			break;
			
		case LINKHYSZ:
			(Hero.hit_height)=(zfix)(value/10000);
			break;
			
		case LINKHZSZ:
			(Hero.hzsz)=(zfix)(value/10000);
			break;
			
		case LINKTXSZ:
			(Hero.txsz)=(zfix)(value/10000);
			break;
			
		case LINKTYSZ:
			(Hero.tysz)=(zfix)(value/10000);
			break;
			
		case LINKTILE:
			(Hero.tile)=(zfix)(value/10000);
			break;
			
		case LINKFLIP:
			(Hero.flip)=(zfix)(value/10000);
			break;
		
		
		
		case LINKINVFRAME:
			Hero.setHClk( (int32_t)vbound((value/10000), 0, 214747) );
			break;
		
		case LINKCANFLICKER:
			Hero.setCanFlicker((value/10000)?1:0);
			break;
		
		case LINKHURTSFX:
			Hero.setHurtSFX( (int32_t)vbound((value/10000), 0, 255) );
			break;
			
		
		case LINKITEMB:
		{
			if ( value/10000 < -1 ) 
			{
				al_trace("Tried to write an invalid item ID to Hero->ItemB: %d\n",value/10000);
				break;
			}		
			if ( value/10000 > MAXITEMS-1 ) 
			{
				al_trace("Tried to write an invalid item ID to Hero->ItemB: %d\n",value/10000);
				break;
			}
			//Hero->setBButtonItem(vbound((value/10000),0,(MAXITEMS-1)));
			
			if (Bwpn != (value/10000))
			{
				Bwpn = value/10000;
				if(new_subscreen_active)
					new_subscreen_active->get_page_pos(Bwpn, game->bwpn);
				game->forced_bwpn = value/10000;
				game->items_off[value/10000] = 0;
			}
			directItemB = value/10000;
			break;
		}
		
		
		case LINKITEMA:
		{
			if ( value/10000 < -1 ) 
			{
				Z_scripterrlog("Tried to write an invalid item ID to Hero->ItemA: %d\n",value/10000);
				break;
			}		
			if ( value/10000 > MAXITEMS-1 ) 
			{
				Z_scripterrlog("Tried to write an invalid item ID to Hero->ItemA: %d\n",value/10000);
				break;
			}		
			//Hero->setBButtonItem(vbound((value/10000),0,(MAXITEMS-1)));
			if (Awpn != (value/10000))
			{
				Awpn = value/10000;
				if(new_subscreen_active)
					new_subscreen_active->get_page_pos(Awpn, game->awpn);
				game->items_off[value/10000] = 0;
				game->forced_awpn = value/10000;
			}
			directItemA = value/10000;
			break;
		}
		
		case LINKITEMX:
		{
			if ( value/10000 < -1 ) 
			{
				Z_scripterrlog("Tried to write an invalid item ID to Hero->ItemX: %d\n",value/10000);
				break;
			}		
			if ( value/10000 > MAXITEMS-1 ) 
			{
				Z_scripterrlog("Tried to write an invalid item ID to Hero->ItemX: %d\n",value/10000);
				break;
			}		
			//Hero->setBButtonItem(vbound((value/10000),0,(MAXITEMS-1)));
			if (Xwpn != (value/10000))
			{
				Xwpn = value/10000;
				if(new_subscreen_active)
					new_subscreen_active->get_page_pos(Xwpn, game->xwpn);
				game->items_off[value/10000] = 0;
				game->forced_xwpn = value/10000;
			}
			directItemX = value/10000;
			break;
		}
		case LINKITEMY:
		{
			if ( value/10000 < -1 ) 
			{
				Z_scripterrlog("Tried to write an invalid item ID to Hero->ItemY: %d\n",value/10000);
				break;
			}		
			if ( value/10000 > MAXITEMS-1 ) 
			{
				Z_scripterrlog("Tried to write an invalid item ID to Hero->ItemY: %d\n",value/10000);
				break;
			}		
			//Hero->setBButtonItem(vbound((value/10000),0,(MAXITEMS-1)));
			if (Ywpn != (value/10000))
			{
				Ywpn = value/10000;
				if(new_subscreen_active)
					new_subscreen_active->get_page_pos(Ywpn, game->ywpn);
				game->items_off[value/10000] = 0;
				game->forced_ywpn = value/10000;
			}
			directItemY = value/10000;
			break;
		}


		case LINKEATEN:
			Hero.setEaten(value/10000);
			break;
		case LINKGRABBED:
			Hero.inwallm = value != 0;
			break;
		case HEROBUNNY:
			Hero.setBunnyClock(value/10000);
			break;
		case LINKPUSH:
			Hero.pushing = zc_max((value/10000),0);
			break;
		case LINKSTUN:
			Hero.setStunClock(value/10000);
			break;
		case LINKSCRIPTTILE:
			script_hero_sprite=vbound((value/10000), -1, NEWMAXTILES-1);
			break;
		
		case HEROSCRIPTCSET:
			script_hero_cset=vbound((value/10000), -1, 0xF);
			break;
		case LINKSCRIPFLIP:
			script_hero_flip=vbound((value/10000),-1,256);
			break;
		
		case GAMESETA:
		{
			//int32_t state   = (ri->d[rINDEX2]/10000);
			//int32_t extend = (ri->d[rINDEX2]/10000);
			//int32_t dir = (ri->d[rINDEX]/10000);
			// Z_message("Trying to force-set the A-button item().\n");
			// Hero.setAButtonItem(vbound((value/10000),0,(MAXITEMS-1)));
		}
		break;
		
		case GAMESETB:
		{
			//int32_t state   = (ri->d[rINDEX2]/10000);
			//int32_t extend = (ri->d[rINDEX2]/10000);
			//int32_t dir = (ri->d[rINDEX]/10000);
			// Z_message("Trying to force-set the A-button item().\n");
			// Hero.setBButtonItem(vbound((value/10000),0,(MAXITEMS-1)));
		}
		break;
		
		//Set Hero Diagonal
		case LINKDIAG:
			Hero.setDiagMove(value?1:0);
			set_qr(qr_LTTPWALK, value?1:0);
			break;
		
		//Set Hero Big Hitbox
		case LINKBIGHITBOX:
			Hero.setBigHitbox((value/10000)?1:0);
			set_qr(qr_LTTPCOLLISION, (value/10000)?1:0);
			break;
		
		case LINKCLIMBING:
			Hero.setOnSideviewLadder(value!=0?true:false);
			break;
			
		case HEROJUMPCOUNT:
			Hero.extra_jump_count = value/10000;
			break;
		
		case HEROPULLCLK:
			Hero.pit_pullclk = value/10000;
			break;
		case HEROFALLCLK:
		{
			int32_t val = vbound(value/10000,0,70);
			if(val)
				Hero.setAction(falling);
			else if(Hero.action == falling)
			{
				Hero.setAction(none);
			}
			Hero.fallclk = val;
			break;
		}
		case HEROFALLCMB:
			Hero.fallCombo = vbound(value/10000,0,MAXCOMBOS-1);
			break;
		case HERODROWNCLK:
		{
			int32_t val = vbound(value/10000,0,70);
			if(val)
			{
				if (Hero.action != lavadrowning) Hero.setAction(drowning);
			}
			else if(Hero.action == drowning || Hero.action == lavadrowning)
			{
				Hero.setAction(none);
			}
			Hero.drownclk = val;
			break;
		}
		case HERODROWNCMB:
			Hero.drownCombo = vbound(value/10000,0,MAXCOMBOS-1);
			break;
		case HEROFAKEZ:
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					Hero.setFakeZfix(zslongToFix(value));
				}
				else
				{
					Hero.setFakeZ(value/10000);
				}
			}
			break;
		
		case HEROSHIELDJINX:
		{
			Hero.shieldjinxclk = value / 10000;
			break;
		}
		
		case CLOCKACTIVE:
		{
			Hero.setClock(watch=(value?true:false));
			break;
		}
		
		case CLOCKCLK:
			clockclk = vbound((value/10000), 0, 214748);
			break;
		
		case HERORESPAWNX:
		{
			zfix zx = zslongToFix(value);
			Hero.respawn_x = vbound(zx, 0_zf, 240_zf);
			break;
		}
		
		case HERORESPAWNY:
		{
			zfix zy = zslongToFix(value);
			Hero.respawn_y = vbound(zy, 0_zf, 160_zf);
			break;
		}
		
		case HERORESPAWNDMAP:
		{
			Hero.respawn_dmap = vbound(value/10000, 0, MAXDMAPS-1);
			break;
		}
		
		case HERORESPAWNSCR:
		{
			Hero.respawn_scr = vbound(value/10000, 0, 0x7F);
			break;
		}
		
		
		case HEROSWITCHMAXTIMER:
		case HEROSWITCHTIMER:
			break; //read-only
		
		case HEROIMMORTAL:
		{
			Hero.setImmortal(value/10000);
			break;
		}
		
		case HEROCOYOTETIME:
		{
			auto v = value/10000;
			if(v < 0 || v > 65535) v = 65535;
			Hero.coyotetime = word(v);
			break;
		}
		case HEROLIFTEDWPN:
		{
			if(Hero.lift_wpn)
			{
				delete Hero.lift_wpn;
				Hero.lift_wpn = nullptr;
			}
			if(value)
			{
				if(weapon* wpn = checkLWpn(value))
				{
					if(wpn == Hero.lift_wpn) break;
					Hero.lift_wpn = wpn;
					if(Lwpns.find(wpn) > -1)
						Lwpns.remove(wpn);
					if(curScriptType == ScriptType::Lwpn && value == curScriptIndex)
						earlyretval = RUNSCRIPT_SELFREMOVE;
				}
			} 
			break;
		}
		case HEROLIFTTIMER:
		{
			Hero.liftclk = value/10000;
			break;
		}
		case HEROLIFTMAXTIMER:
		{
			Hero.tliftclk = value/10000;
			break;
		}
		case HEROLIFTHEIGHT:
		{
			Hero.liftheight = zslongToFix(value);
			break;
		}
		case HEROHAMMERSTATE:
		{
			//readonly
			break;
		}
		case HEROFLICKERCOLOR:
		{
			Hero.flickercolor = value/10000;
			break;
		}
		case HEROFLICKERTRANSP:
		{
			Hero.flickertransp = value / 10000;
			break;
		}
		case HEROSCRICECMB:
			Hero.script_ice_combo = vbound(value/10000,-1,MAXCOMBOS); break;
		case HEROICEVX:
			Hero.ice_vx = zslongToFix(value); break;
		case HEROICEVY:
			Hero.ice_vy = zslongToFix(value); break;
		case HEROICEENTRYFRAMES:
			Hero.ice_entry_count = vbound(value/10000,0,255); break;
		case HEROICEENTRYMAXFRAMES:
			Hero.ice_entry_mcount = vbound(value/10000,0,255); break;
		
		
	///----------------------------------------------------------------------------------------------------//
	//Input States
		case INPUTSTART:
		{
			control_state[6]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) drunk_toggle_state[6]=false;
			break;
		}
			
		case INPUTMAP:
		{
			control_state[9]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) 
				drunk_toggle_state[9]=false;
			break;
		}
			
		case INPUTUP:
		{
			control_state[0]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) drunk_toggle_state[0]=false;
			break;
		}
			
		case INPUTDOWN:
		{
			control_state[1]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) 
				drunk_toggle_state[1]=false;
			break;
		}
			
		case INPUTLEFT:
		{
			control_state[2]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) drunk_toggle_state[2]=false;
			break;
		}
			
		case INPUTRIGHT:
		{
			control_state[3]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) drunk_toggle_state[3]=false;
			break;
		}
			
		case INPUTA:
		{
			control_state[4]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) drunk_toggle_state[4]=false;
			break;
		}
			
		case INPUTB:
		{
			control_state[5]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) drunk_toggle_state[5]=false;
			break;
		}
			
		case INPUTL:
		{
			control_state[7]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) drunk_toggle_state[7]=false;
			break;
		}
			
		case INPUTR:
		{
			control_state[8]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) drunk_toggle_state[8]=false;
			break;
		}
			
		case INPUTEX1:
		{
			control_state[10]=(value?true:false);
			break;
		}
			
		case INPUTEX2:
			control_state[11]=(value?true:false);
			break;
			
		case INPUTEX3:
			control_state[12]=(value?true:false);
			break;
			
		case INPUTEX4:
			control_state[13]=(value?true:false);
			break;
			
		case INPUTAXISUP:
			control_state[14]=(value?true:false);
			break;
			
		case INPUTAXISDOWN:
			control_state[15]=(value?true:false);
			break;
			
		case INPUTAXISLEFT:
			control_state[16]=(value?true:false);
			break;
			
		case INPUTAXISRIGHT:
			control_state[17]=(value?true:false);
			break;
			
		case INPUTPRESSSTART:
			button_press[6]=(value?true:false);
			break;
			
		case INPUTPRESSMAP:
			button_press[9]=(value?true:false);
			break;
			
		case INPUTPRESSUP:
			button_press[0]=(value?true:false);
			break;
			
		case INPUTPRESSDOWN:
			button_press[1]=(value?true:false);
			break;
			
		case INPUTPRESSLEFT:
			button_press[2]=(value?true:false);
			break;
			
		case INPUTPRESSRIGHT:
			button_press[3]=(value?true:false);
			break;
			
		case INPUTPRESSA:
			button_press[4]=(value?true:false);
			break;
			
		case INPUTPRESSB:
			button_press[5]=(value?true:false);
			break;
			
		case INPUTPRESSL:
			button_press[7]=(value?true:false);
			break;
			
		case INPUTPRESSR:
			button_press[8]=(value?true:false);
			break;
			
		case INPUTPRESSEX1:
			button_press[10]=(value?true:false);
			break;
			
		case INPUTPRESSEX2:
			button_press[11]=(value?true:false);
			break;
			
		case INPUTPRESSEX3:
			button_press[12]=(value?true:false);
			break;
			
		case INPUTPRESSEX4:
			button_press[13]=(value?true:false);
			break;
			
		case PRESSAXISUP:
			button_press[14]=(value?true:false);
			break;
			
		case PRESSAXISDOWN:
			button_press[15]=(value?true:false);
			break;
			
		case PRESSAXISLEFT:
			button_press[16]=(value?true:false);
			break;
			
		case PRESSAXISRIGHT:
			button_press[17]=(value?true:false);
			break;
			
		case INPUTMOUSEX:
		{
			auto [x, y] = rti_game.local_to_world(value/10000, mouse_y);
			position_mouse(x, y);
			break;
		}
		
		case INPUTMOUSEY:
		{
			int32_t mousequakeoffset = 56+((int32_t)(zc::math::Sin((double)(quakeclk*int64_t(2)-frame))*4));
			int32_t tempoffset = (quakeclk > 0) ? mousequakeoffset : (get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
			auto [x, y] = rti_game.local_to_world(mouse_x, value/10000 + tempoffset);
			position_mouse(x, y);
			break;
		}
		
		case INPUTMOUSEZ:
			position_mouse_z(value/10000);
			break;
		
		case SIMULATEKEYPRESS:
		{
			//if ( !keypressed() ) break; //Don;t return values set by setting Hero->Input/Press
			//hmm...no, this won;t return properly for modifier keys. 
			int32_t keyid = ri->d[rINDEX]/10000;
			//key = vbound(key,0,n);
			if (value/10000) simulate_keypress(keyid << 8);
		}
		break;
		
		case KEYMODIFIERS:
		{
			key_shifts = ( value/10000 );
			break;
		}
		break;

	///----------------------------------------------------------------------------------------------------//
	//Itemdata Variables
		//not mine, but let;s guard some of them all the same -Z
		//item class
		case IDATAFAMILY:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].family)=vbound(value/10000,0, 254);
			flushItemCache();
			break;
		
		case IDATAUSEWPN:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].weap_data.imitate_weapon)=vbound(value/10000, 0, 255);
			break;
		case IDATAUSEDEF:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].weap_data.default_defense)=vbound(value/10000, 0, 255);
			break;
		case IDATAWRANGE:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].weaprange)=vbound(value/10000, 0, 255);
			break;
		case IDATAMAGICTIMER:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].magiccosttimer[0])=vbound(value/10000, 0, 214747);
			break;
		case IDATAMAGICTIMER2:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].magiccosttimer[1])=vbound(value/10000, 0, 214747);
			break;
		case IDATADURATION:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].weapduration)=vbound(value/10000, 0, 255);
			break;
		 
		case IDATADUPLICATES:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].duplicates)=vbound(value/10000, 0, 255);
			break;
		case IDATADRAWLAYER:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].drawlayer)=vbound(value/10000, 0, 7);
			break;
		case IDATACOLLECTFLAGS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			//int32_t a = ri->d[rINDEX] / 10000;
			(itemsbuf[ri->idata].collectflags)=vbound(value/10000, 0, 214747);
			break;
		case IDATAWEAPONSCRIPT:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].weap_data.script)=vbound(value/10000, 0, 255);
			break;
		case IDATAWEAPHXOFS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].weap_data.hxofs)=(value/10000);
			break;
		case IDATAWEAPHYOFS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].weap_data.hyofs)=(value/10000);
			break;
		case IDATAWEAPHXSZ:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].weap_data.hxsz)=(value/10000);
			break;
		case IDATAWEAPHYSZ:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].weap_data.hysz)=(value/10000);
			break;
		case IDATAWEAPHZSZ:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].weap_data.hzsz)=(value/10000);
			break;
		case IDATAWEAPXOFS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].weap_data.xofs)=(value/10000);
			break;
		case IDATAWEAPYOFS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].weap_data.yofs)=(value/10000);
			break;

		
		case IDATAHXOFS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].hxofs)=(value/10000);
			break;
		case IDATAHYOFS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].hyofs)=(value/10000);
			break;
		case IDATAHXSZ:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].hxsz)=(value/10000);
			break;
		case IDATAHYSZ:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].hysz)=(value/10000);
			break;
		case IDATAHZSZ:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].hzsz)=(value/10000);
			break;
		case IDATADXOFS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].xofs)=(value/10000);
			break;
		case IDATADYOFS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].yofs)=(value/10000);
			break;
		case IDATATILEW:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].tilew)=(value/10000);
			break;
		case IDATATILEH:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].tileh)=(value/10000);
			break;
		case IDATAPICKUP:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].pickup)=(value/10000);
			break;
		case IDATAOVERRIDEFL:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].overrideFLAGS)=(value/10000);
			break;

		case IDATATILEWWEAP:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].weap_data.tilew)=(value/10000);
			break;
		case IDATATILEHWEAP:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].weap_data.tileh)=(value/10000);
			break;
		case IDATAOVERRIDEFLWEAP:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].weap_data.override_flags)=(value/10000);
			break;
		
		case IDATAUSEMVT:
		{
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			int32_t a = vbound((ri->d[rINDEX] / 10000),0,(ITEM_MOVEMENT_PATTERNS-1));
			(itemsbuf[ri->idata].weap_pattern[a])=vbound(value/10000, 0, 255);
			break;
		}
		
		case IDATALEVEL:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].fam_type)=vbound(value/10000, 0, 512);
			flushItemCache();
			break;
		case IDATAKEEP:
			item_flag(item_gamedata, value);
			break;
		case IDATAAMOUNT:
		{
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			int32_t v = vbound(value/10000, -9999, 16383);
			itemsbuf[ri->idata].amount &= 0x8000;
			itemsbuf[ri->idata].amount |= (abs(v)&0x3FFF)|(v<0?0x4000:0);
			break;
		}
		case IDATAGRADUAL:
		{
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			SETFLAG(itemsbuf[ri->idata].amount, 0x8000, value!=0);
			break;
		}
		case IDATACONSTSCRIPT:
			item_flag(item_passive_script, value);
			break;
		case IDATASSWIMDISABLED:
			item_flag(item_sideswim_disabled, value);
			break;
		case IDATABUNNYABLE:
			item_flag(item_bunny_enabled, value);
			break;
		case IDATAJINXIMMUNE:
			item_flag(item_jinx_immune, value);
			break;
		case IDATAJINXSWAP:
			item_flag(item_flip_jinx, value);
			break;
		case IDATAUSEBURNSPR:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
			}
			else SETFLAG(itemsbuf[ri->idata].weap_data.wflags, WFLAG_UPDATE_IGNITE_SPRITE, value);
			break;
		case IDATASETMAX:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].setmax)=value/10000;
			break;
			
		case IDATAMAX:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].max)=value/10000;
			break;
			
		case IDATAPOWER:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].power)=value/10000;
			break;
			
		case IDATACOUNTER:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].count)=vbound(value/10000,0,31);
			break;
			
		case IDATAPSOUND:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].playsound)=vbound(value/10000, 0, 255);
			break;
			
		case IDATAUSESOUND:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].usesound)=vbound(value/10000, 0, 255);
			break;
			
		case IDATAUSESOUND2:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].usesound2)=vbound(value/10000, 0, 255);
			break;
		
		//2.54
		//My additions begin here. -Z
		//Stack item to gain next level
		case IDATACOMBINE:
			item_flag(item_combine, value);
			break;
		//using a level of an item downgrades to a lower one
		case IDATADOWNGRADE:
			item_flag(item_downgrade, value);
			break;
		  //Only validate the cost, don't charge it
		case IDATAVALIDATE:
			item_flag(item_validate_only, value);
			break;
		case IDATAVALIDATE2:
			item_flag(item_validate_only_2, value);
			break;
		

		//Keep Old in editor
		case IDATAKEEPOLD:
			item_flag(item_keep_old, value);
			break;
		//Ruppes for magic
		case IDATARUPEECOST:
			item_flag(item_rupee_magic, value);
			break;
		//can be eaten
		case IDATAEDIBLE:
			item_flag(item_edible, value);
			break;
		//Unused at this time
		case IDATAFLAGUNUSED:
			item_flag(item_unused, value);
			break;
		//gain lower level items
		case IDATAGAINLOWER:
			item_flag(item_gain_old, value);
			break;
		//Set the action script
		case IDATASCRIPT:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			FFScript::deallocateAllScriptOwned(ScriptType::Item, ri->idata);
			itemsbuf[ri->idata].script=vbound(value/10000,0,255);
			break;
		case IDATASPRSCRIPT:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			itemsbuf[ri->idata].sprite_script=vbound(value/10000,0,255);
			break;

		//Hero tile modifier. 
		case IDATALTM:
		{
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			auto new_value = value/10000;
			if (new_value != itemsbuf[ri->idata].ltm)
				cache_tile_mod_clear();
			itemsbuf[ri->idata].ltm = new_value;
			break;
		}
		//Pickup script
		case IDATAPSCRIPT:
		{
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			//Need to get collect script ref, not standard idata ref!
			const int32_t new_ref = ri->idata!=0 ? -(ri->idata) : COLLECT_SCRIPT_ITEM_ZERO;
			FFScript::deallocateAllScriptOwned(ScriptType::Item,new_ref);
			itemsbuf[ri->idata].collect_script=vbound(value/10000, 0, 255);
			break;
		}
		//pickup string
		case IDATAPSTRING:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			itemsbuf[ri->idata].pstring=vbound(value/10000, 1, 255);
			break;
		case IDATAPFLAGS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			itemsbuf[ri->idata].pickup_string_flags=vbound(value/10000, 0, 214748);
			break;
		case IDATAPICKUPLITEMS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			itemsbuf[ri->idata].pickup_litems = vbound(value/10000, 0, 214748) & liALL;
			break;
		case IDATAPICKUPLITEMLEVEL:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			itemsbuf[ri->idata].pickup_litem_level = vbound(value/10000, -1, MAXLEVELS-1);
			break;
		//magic cost
		case IDATAMAGCOST:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			itemsbuf[ri->idata].cost_amount[0]=vbound(value/10000,32767,-32768);
			break;
		case IDATACOST2:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			itemsbuf[ri->idata].cost_amount[1]=vbound(value/10000,32767,-32768);
			break;
		//cost counter ref
		case IDATACOSTCOUNTER:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			itemsbuf[ri->idata].cost_counter[0]=(vbound(value/10000,-1,32));
			break;
		case IDATACOSTCOUNTER2:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			itemsbuf[ri->idata].cost_counter[1]=(vbound(value/10000,-1,32));
			break;
		//min hearts to pick up
		case IDATAMINHEARTS:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			itemsbuf[ri->idata].pickup_hearts=vbound(value/10000, 0, 214748);
			break;
		//item tile
		case IDATATILE:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			itemsbuf[ri->idata].tile=vbound(value/10000, 0, NEWMAXTILES-1);
			break;
		//flash
		case IDATAMISC:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			itemsbuf[ri->idata].misc_flags=value/10000;
			break;
		//cset
		case IDATACSET:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}

			itemsbuf[ri->idata].csets = (itemsbuf[ri->idata].csets & 0xF0) | vbound(value/10000,0,15);

			// If we find quests that broke, use this code.
			// if (QHeader.compareVer(2, 55, 9) >= 0)
			// 	itemsbuf[ri->idata].csets = (itemsbuf[ri->idata].csets & 0xF0) | vbound(value/10000,0,15);
			// else
			// 	itemsbuf[ri->idata].csets = vbound(value/10000,0,13);
			break;
		
		case IDATAFLASHCSET:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}

			itemsbuf[ri->idata].csets = (itemsbuf[ri->idata].csets & 0xF) | (vbound(value/10000,0,15)<<4);
			break;
		/*
		case IDATAFRAME:
			itemsbuf[ri->idata].frame=value/10000;
			break;
		*/
		//A.Frames
		case IDATAFRAMES:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			(itemsbuf[ri->idata].frames)=vbound(value/10000, 0, 214748);
			break;
		//A.speed
		case IDATAASPEED:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			itemsbuf[ri->idata].speed=vbound(value/10000, 0, 214748);
			break;
		//Anim delay
		case IDATADELAY:
			if(unsigned(ri->idata) >= MAXITEMS)
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
				break;
			}
			itemsbuf[ri->idata].delay=vbound(value/10000, 0, 214748);
			break;
		
	///----------------------------------------------------------------------------------------------------//
	//LWeapon Variables
		
		case LWPNSCALE:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				break;
			}
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->scale=(zfix)(value/100.0);
				
			break;
		
		case LWPNX:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->x=get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
			break;
		
		case SPRITEMAXLWPN:
		{
			//No bounds check, as this is a universal function and works from NULL pointers!
			Lwpns.setMax(vbound((value/10000),1,MAX_LWPN_SPRITES));
			break;
		}
			
		case LWPNY:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->y=get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
				
			break;
			
		case LWPNZ:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)s)->z=get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
				if(((weapon*)s)->z < 0) ((weapon*)s)->z = 0_zf;
			}
				
			break;
			
		case LWPNJUMP:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->fall=zslongToFix(value)*-100;
				
			break;
			
		case LWPNFAKEJUMP:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->fakefall=zslongToFix(value)*-100;
				
			break;
			
		case LWPNDIR:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)s)->dir=(value/10000);
				((weapon*)s)->doAutoRotate(true);
			}
				
			break;
			
		case LWPNSPECIAL:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->specialinfo=(value/10000);
				
			break;
		 
		case LWPNGRAVITY:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				if(value)
					((weapon*)s)->moveflags |= move_obeys_grav;
				else
					((weapon*)s)->moveflags &= ~move_obeys_grav;
			}
			break;
			
		case LWPNSTEP:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				// fp math is bad for replay, so always ignore this QR when replay is active.
				// TODO: can we just delete this QR? Would it actually break anything? For now,
				// just disable for replay and wait for more tests to be played with this QR
				// ignored.
				if ( get_qr(qr_STEP_IS_FLOAT) || replay_is_active() )
				{
					((weapon*)s)->step= zslongToFix(value / 100);
				}
				else
				{
					//old, buggy code replication, round two: Go! -Z
					//zfix val = zslongToFix(value);
					//val.doFloor();
					//((weapon*)s)->step = ((val / 100.0).getFloat());
					
					//old, buggy code replication, round THREE: Go! -Z
					((weapon*)s)->step = ((value/10000)/100.0);
				}
				
			}
				
			break;
			
		case LWPNANGLE:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)s)->angle=(double)(value/10000.0);
				((weapon*)(s))->doAutoRotate();
			}
				
			break;
			
		case LWPNDEGANGLE:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				double rangle = (value / 10000.0) * (PI / 180.0);
				((weapon*)s)->angle=(double)(rangle);
				((weapon*)(s))->doAutoRotate();
			}
				
			break;
			
		case LWPNVX:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				double vy;
				double vx = (value / 10000.0);
				if (((weapon*)(s))->angular)
					vy = zc::math::Sin(((weapon*)s)->angle)*((weapon*)s)->step;
				else
				{
					switch(NORMAL_DIR(((weapon*)(s))->dir))
					{
						case l_up:
						case r_up:
						case up:
							vy = -1.0*((weapon*)s)->step;
							break;
						case l_down:
						case r_down:
						case down:
							vy = ((weapon*)s)->step;
							break;
							
						default:
							vy = 0;
							break;
					}
				}
				((weapon*)s)->angular = true;
				((weapon*)s)->angle=atan2(vy, vx);
				((weapon*)s)->step=FFCore.Distance(0, 0, vx, vy)/10000.0;
				((weapon*)(s))->doAutoRotate();
			}
				
			break;
		
		case LWPNVY:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				double vx;
				double vy = (value / 10000.0);
				if (((weapon*)(s))->angular)
					vx = zc::math::Cos(((weapon*)s)->angle)*((weapon*)s)->step;
				else
				{
					switch(NORMAL_DIR(((weapon*)(s))->dir))
					{
						case l_up:
						case l_down:
						case left:
							vx = -1.0*((weapon*)s)->step;
							break;
						case r_down:
						case r_up:
						case right:
							vx = ((weapon*)s)->step;
							break;
							
						default:
							vx = 0;
							break;
					}
				}
				((weapon*)s)->angular = true;
				((weapon*)s)->angle=atan2(vy, vx);
				((weapon*)s)->step=FFCore.Distance(0, 0, vx, vy)/10000.0;
				((weapon*)(s))->doAutoRotate();
			}
				
			break;
			
		case LWPNANGULAR:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)s)->angular=(value!=0);
				((weapon*)(s))->doAutoRotate(false, true);
			}
				
			break;
			
		case LWPNAUTOROTATE:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)s)->autorotate=(value!=0);
				((weapon*)(s))->doAutoRotate(false, true);
			}
				
			break;
			
		case LWPNBEHIND:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->behind=(value!=0);
				
			break;
			
		case LWPNDRAWTYPE:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->drawstyle=(value/10000);
				
			break;
			
		case LWPNPOWER:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->power=(value/10000);
				
			break;
		/*
		case LWPNRANGE:
			if(0!=(s=checkLWpn(ri->lwpn)))
			((weapon*)s)->scriptrange=vbound((value/10000),0,512); //Allow it to move off-screen. -Z           
			break;
		*/        
		case LWPNDEAD:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				auto dead = value/10000;
				((weapon*)s)->dead=dead;
				if(dead != 0) ((weapon*)s)->weapon_dying_frame = false;
			}
			break;
			
		case LWPNID:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->id=(value/10000);
				
			break;
			
		case LWPNTILE:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->tile=(value/10000);
				
			break;
		
		case LWPNSCRIPTTILE:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->scripttile=vbound((value/10000),-1,NEWMAXTILES-1);
				
			break;
		
		case LWPNSCRIPTFLIP:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->scriptflip=vbound((value/10000),-1,127);
				
			break;
			
		case LWPNCSET:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->cs=(value/10000)&15;
				
			break;
			
		case LWPNFLASHCSET:
			if(0!=(s=checkLWpn(ri->lwpn)))
				(((weapon*)s)->o_cset)|=(value/10000)<<4;
				
			break;
			
		case LWPNFRAMES:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->frames=(value/10000);
				
			break;
			
		case LWPNFRAME:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->aframe=(value/10000);
				
			break;
			
		case LWPNASPEED:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->o_speed=(value/10000);
				
			break;
			
		case LWPNFLASH:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->flash=(value/10000);
				
			break;
			
		case LWPNFLIP:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->flip=(value/10000);
				
			break;

		case LWPNROTATION:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				break;
			}
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->rotation=(value/10000);
				
			break;
			
		case LWPNEXTEND:
			if(0!=(s=checkLWpn(ri->lwpn)))
				((weapon*)s)->extend=(value/10000);
				
			break;
			
		case LWPNOTILE:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
					((weapon*)s)->o_tile=(value/10000);
					((weapon*)s)->ref_o_tile=(value/10000);
					//((weapon*)s)->script_wrote_otile=1; //Removing this as of 26th October, 2019 -Z
				//if at some future point we WANT writing ->Tile to also overwrite ->OriginalTile,
				//then either the user will need to manually write tile, or we can add a QR and 
				// write ->tile here. 'script_wrote_otile' is out.
			}
			break;
			
		case LWPNOCSET:
			if(0!=(s=checkLWpn(ri->lwpn)))
				(((weapon*)s)->o_cset)|=(value/10000)&15;
				
			break;
			
		case LWPNHXOFS:
			if(0!=(s=checkLWpn(ri->lwpn)))
				(((weapon*)s)->hxofs)=(value/10000);
				
			break;
			
		case LWPNHYOFS:
			if(0!=(s=checkLWpn(ri->lwpn)))
				(((weapon*)s)->hyofs)=(value/10000);
				
			break;
			
		case LWPNXOFS:
			if(0!=(s=checkLWpn(ri->lwpn)))
				(((weapon*)s)->xofs)=(zfix)(value/10000);
				
			break;
			
		case LWPNYOFS:
			if(0!=(s=checkLWpn(ri->lwpn)))
				(((weapon*)s)->yofs)=(zfix)(value/10000)+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
				
			break;
		
		case LWPNSHADOWXOFS:
			if(0!=(s=checkLWpn(ri->lwpn)))
				(((weapon*)s)->shadowxofs)=(zfix)(value/10000);
				
			break;
		
		case LWPNSHADOWYOFS:
			if(0!=(s=checkLWpn(ri->lwpn)))
				(((weapon*)s)->shadowyofs)=(zfix)(value/10000);
				
			break;
			
		case LWPNTOTALDYOFFS:
			break; //READ-ONLY
			
		case LWPNZOFS:
			if(0!=(s=checkLWpn(ri->lwpn)))
				(((weapon*)s)->zofs)=(zfix)(value/10000);
				
			break;
			
		case LWPNHXSZ:
			if(0!=(s=checkLWpn(ri->lwpn)))
				(((weapon*)s)->hit_width)=(value/10000);
				
			break;
			
		case LWPNHYSZ:
			if(0!=(s=checkLWpn(ri->lwpn)))
				(((weapon*)s)->hit_height)=(value/10000);
				
			break;
			
		case LWPNHZSZ:
			if(0!=(s=checkLWpn(ri->lwpn)))
				(((weapon*)s)->hzsz)=(value/10000);
				
			break;
			
		case LWPNTXSZ:
			if(0!=(s=checkLWpn(ri->lwpn)))
				(((weapon*)s)->txsz)=vbound((value/10000),1,20);
				
			break;
			
		case LWPNTYSZ:
			if(0!=(s=checkLWpn(ri->lwpn)))
				(((weapon*)s)->tysz)=vbound((value/10000),1,20);
				
			break;

		case LWPNCOLLDET:
			if(0!=(s=checkLWpn(ri->lwpn)))
				(((weapon*)(s))->scriptcoldet) = value;

			break;
		
		case LWPNENGINEANIMATE:
			if(0!=(s=checkLWpn(ri->lwpn)))
				(((weapon*)(s))->do_animation)=value;
				
			break;
		
		case LWPNPARENT:
		{
			//int32_t pitm = (vbound(value/10000,1,(MAXITEMS-1)));
					
			if(0!=(s=checkLWpn(ri->lwpn)))
				(((weapon*)(s))->parentitem)=(vbound(value/10000,-1,(MAXITEMS-1)));
			break;
		}

		case LWPNLEVEL:
			if(0!=(s=checkLWpn(ri->lwpn)))
				(((weapon*)(s))->type)=value/10000;
				
			break;
		
		case LWPNSCRIPT:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				(((weapon*)(s))->script)=vbound(value/10000,0,NUMSCRIPTWEAPONS-1);
				if ( get_qr(qr_CLEARINITDONSCRIPTCHANGE))
				{
					for(int32_t q=0; q<8; q++)
						(((weapon*)(s))->initD[q]) = 0;
				}
				on_reassign_script_engine_data(ScriptType::Lwpn, ri->lwpn);
			}  
			break;
		
		case LWPNUSEWEAPON:
			if(0!=(s=checkLWpn(ri->lwpn)))
			(((weapon*)(s))->useweapon)=vbound(value/10000,0,255);
				
			break;
		
		case LWPNUSEDEFENCE:
			if(0!=(s=checkLWpn(ri->lwpn)))
			(((weapon*)(s))->usedefense)=vbound(value/10000,0,255);
				
			break;

		case LWPNFALLCLK:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				if(((weapon*)(s))->fallclk != 0 && value == 0)
				{
					((weapon*)(s))->cs = ((weapon*)(s))->o_cset;
					((weapon*)(s))->tile = ((weapon*)(s))->o_tile;
				}
				else if(((weapon*)(s))->fallclk == 0 && value != 0) ((weapon*)(s))->o_cset = ((weapon*)(s))->cs;
				((weapon*)(s))->fallclk = vbound(value/10000,0,70);
			}
			break;
		case LWPNFALLCMB:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)(s))->fallCombo = vbound(value/10000,0,MAXCOMBOS-1);
			}
			break;
		case LWPNDROWNCLK:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				if(((weapon*)(s))->drownclk != 0 && value == 0)
				{
					((weapon*)(s))->cs = ((weapon*)(s))->o_cset;
					((weapon*)(s))->tile = ((weapon*)(s))->o_tile;
				}
				else if(((weapon*)(s))->drownclk == 0 && value != 0) ((weapon*)(s))->o_cset = ((weapon*)(s))->cs;
				((weapon*)(s))->drownclk = vbound(value/10000,0,70);
			}
			break;
		case LWPNDROWNCMB:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)(s))->drownCombo = vbound(value/10000,0,MAXCOMBOS-1);
			}
			break;
		case LWPNFAKEZ:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)s)->fakez=get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
				if(((weapon*)s)->fakez < 0) ((weapon*)s)->fakez = 0_zf;
			}
				
			break;

		case LWPNGLOWRAD:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)(s))->glowRad = vbound(value/10000,0,255);
			}
			break;
			
		case LWPNGLOWSHP:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)(s))->glowShape = vbound(value/10000,0,255);
			}
			break;
			
		case LWPNUNBL:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)(s))->unblockable = (value/10000)&WPNUNB_ALL;
			}
			break;
			
		case LWPNSHADOWSPR:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)(s))->spr_shadow = vbound(value/10000, 0, 255);
			}
			break;
		case LWSWHOOKED:
			break; //read-only
		case LWPNTIMEOUT:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)(s))->weap_timeout = vbound(value/10000,0,214748);
			}
			break;
		case LWPNDEATHITEM:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)(s))->death_spawnitem = vbound(value/10000,-1,MAXITEMS-1);
			}
			break;
		case LWPNDEATHDROPSET:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)(s))->death_spawndropset = vbound(value/10000,-1,MAXITEMDROPSETS-1);
			}
			break;
		case LWPNDEATHIPICKUP:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)(s))->death_item_pflags = value/10000;
			}
			break;
		case LWPNDEATHSPRITE:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)(s))->death_sprite = vbound(value/10000,-255,MAXWPNS-1);
			}
			break;
		case LWPNDEATHSFX:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)(s))->death_sfx = vbound(value/10000,0,WAV_COUNT);
			}
			break;
		case LWPNLIFTLEVEL:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)(s))->lift_level = vbound(value/10000,0,255);
			}
			break;
		case LWPNLIFTTIME:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)(s))->lift_time = vbound(value/10000,0,255);
			}
			break;
		case LWPNLIFTHEIGHT:
			if(0!=(s=checkLWpn(ri->lwpn)))
			{
				((weapon*)(s))->lift_height = zslongToFix(value);
			}
			break;
			
	///----------------------------------------------------------------------------------------------------//
	//EWeapon Variables
		case EWPNSCALE:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				break;
			}
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->scale=(zfix)(value/100.0);
				
			break;
		
		case EWPNX:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->x = (get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000));
				
			break;
		
		case SPRITEMAXEWPN:
		{
			//No bounds check, as this is a universal function and works from NULL pointers!
			Ewpns.setMax(vbound((value/10000),1,MAX_EWPN_SPRITES));
			break;
		}
		
		case EWPNY:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->y = (get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000));
				
			break;
			
		case EWPNZ:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)s)->z=get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
				if(((weapon*)s)->z < 0) ((weapon*)s)->z = 0_zf;
			}
				
			break;
			
		case EWPNJUMP:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->fall=zslongToFix(value)*-100;
				
			break;
			
		case EWPNFAKEJUMP:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->fakefall=zslongToFix(value)*-100;
				
			break;
			
		case EWPNDIR:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)s)->dir=(value/10000);
				((weapon*)s)->doAutoRotate(true);
			}
				
			break;
			
		case EWPNLEVEL:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->type=(value/10000);
				
			break;
		  
		case EWPNGRAVITY:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				if(value)
					((weapon*)s)->moveflags |= move_obeys_grav;
				else
					((weapon*)s)->moveflags &= ~move_obeys_grav;
			}
			break;
			
		case EWPNSTEP:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				if ( get_qr(qr_STEP_IS_FLOAT) || replay_is_active() )
				{
					((weapon*)s)->step= zslongToFix(value / 100);
				}
				else
				{
					//old, buggy code replication, round two: Go! -Z
					//zfix val = zslongToFix(value);
					//val.doFloor();
					//((weapon*)s)->step = ((val / 100.0).getFloat());
					
					//old, buggy code replication, round THREE: Go! -Z
					((weapon*)s)->step = ((value/10000)/100.0);
				}
			}
				
			break;
			
		case EWPNANGLE:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)s)->angle=(double)(value/10000.0);
				((weapon*)(s))->doAutoRotate();
			}
				
			break;
			
		case EWPNDEGANGLE:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				double rangle = (value / 10000.0) * (PI / 180.0);
				((weapon*)s)->angle=(double)(rangle);
				((weapon*)(s))->doAutoRotate();
			}
				
			break;
			
		case EWPNVX:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				double vy;
				double vx = (value / 10000.0);
				if (((weapon*)(s))->angular)
					vy = zc::math::Sin(((weapon*)s)->angle)*((weapon*)s)->step;
				else
				{
					switch(NORMAL_DIR(((weapon*)(s))->dir))
					{
						case l_up:
						case r_up:
						case up:
							vy = -1.0*((weapon*)s)->step;
							break;
						case l_down:
						case r_down:
						case down:
							vy = ((weapon*)s)->step;
							break;
							
						default:
							vy = 0;
							break;
					}
				}
				((weapon*)s)->angular = true;
				((weapon*)s)->angle=atan2(vy, vx);
				((weapon*)s)->step=FFCore.Distance(0, 0, vx, vy)/10000;
				((weapon*)(s))->doAutoRotate();
			}
				
			break;
		
		case EWPNVY:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				double vx;
				double vy = (value / 10000.0);
				if (((weapon*)(s))->angular)
					vx = zc::math::Cos(((weapon*)s)->angle)*((weapon*)s)->step;
				else
				{
					switch(NORMAL_DIR(((weapon*)(s))->dir))
					{
						case l_up:
						case l_down:
						case left:
							vx = -1.0*((weapon*)s)->step;
							break;
						case r_down:
						case r_up:
						case right:
							vx = ((weapon*)s)->step;
							break;
							
						default:
							vx = 0;
							break;
					}
				}
				((weapon*)s)->angular = true;
				((weapon*)s)->angle=atan2(vy, vx);
				((weapon*)s)->step=FFCore.Distance(0, 0, vx, vy)/10000;
				((weapon*)(s))->doAutoRotate();
			}
				
			break;
			
		case EWPNANGULAR:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)s)->angular=(value!=0);
				((weapon*)(s))->doAutoRotate(false, true);
			}
				
			break;
			
		case EWPNAUTOROTATE:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)s)->autorotate=(value!=0);
				((weapon*)(s))->doAutoRotate(false, true);
			}
				
			break;
			
		case EWPNBEHIND:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->behind=(value!=0);
				
			break;
			
		case EWPNDRAWTYPE:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->drawstyle=(value/10000);
				
			break;
			
		case EWPNPOWER:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->power=(value/10000);
				
			break;
			
		case EWPNDEAD:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				auto dead = value/10000;
				((weapon*)s)->dead=dead;
				if(dead != 0) ((weapon*)s)->weapon_dying_frame = false;
			}
				
			break;
			
		case EWPNID:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->id=(value/10000);
				
			break;
			
		case EWPNTILE:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->tile=(value/10000);
				
			break;
			
		case EWPNSCRIPTTILE:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->scripttile=vbound((value/10000),-1, NEWMAXTILES-1);
				
			break;
		
		case EWPNSCRIPTFLIP:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->scriptflip=vbound((value/10000),-1, 127);
				
			break;
			
		case EWPNCSET:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->cs=(value/10000)&15;
				
			break;
			
		case EWPNFLASHCSET:
			if(0!=(s=checkEWpn(ri->ewpn)))
				(((weapon*)s)->o_cset)|=(value/10000)<<4;
				
			break;
			
		case EWPNFRAMES:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->frames=(value/10000);
				
			break;
			
		case EWPNFRAME:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->aframe=(value/10000);
				
			break;
			
		case EWPNASPEED:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->o_speed=(value/10000);
				
			break;
			
		case EWPNFLASH:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->flash=(value/10000);
				
			break;
			
		case EWPNFLIP:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->flip=(value/10000);
				
			break;
			
		case EWPNROTATION:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'");
				break;
			}
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->rotation=(value/10000);
				
			break;
			
		case EWPNEXTEND:
			if(0!=(s=checkEWpn(ri->ewpn)))
				((weapon*)s)->extend=(value/10000);
				
			break;
			
		case EWPNOTILE:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)s)->o_tile=(value/10000);
				((weapon*)s)->ref_o_tile=(value/10000);
			}
				
			break;
			
		case EWPNOCSET:
			if(0!=(s=checkEWpn(ri->ewpn)))
				(((weapon*)s)->o_cset)|=(value/10000)&15;
				
			break;
			
		case EWPNHXOFS:
			if(0!=(s=checkEWpn(ri->ewpn)))
				(((weapon*)s)->hxofs)=(value/10000);
				
			break;
			
		case EWPNHYOFS:
			if(0!=(s=checkEWpn(ri->ewpn)))
				(((weapon*)s)->hyofs)=(value/10000);
				
			break;
			
		case EWPNXOFS:
			if(0!=(s=checkEWpn(ri->ewpn)))
				(((weapon*)s)->xofs)=(zfix)(value/10000);
				
			break;
			
		case EWPNYOFS:
			if(0!=(s=checkEWpn(ri->ewpn)))
				(((weapon*)s)->yofs)=(zfix)(value/10000)+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
				
			break;
			
		case EWPNSHADOWXOFS:
			if(0!=(s=checkEWpn(ri->ewpn)))
				(((weapon*)s)->shadowxofs)=(zfix)(value/10000);
				
			break;
			
		case EWPNSHADOWYOFS:
			if(0!=(s=checkEWpn(ri->ewpn)))
				(((weapon*)s)->shadowyofs)=(zfix)(value/10000);
				
			break;
			
		case EWPNZOFS:
			if(0!=(s=checkEWpn(ri->ewpn)))
				(((weapon*)s)->zofs)=(zfix)(value/10000);
				
			break;
			
		case EWPNHXSZ:
			if(0!=(s=checkEWpn(ri->ewpn)))
				(((weapon*)s)->hit_width)=(value/10000);
				
			break;
			
		case EWPNHYSZ:
			if(0!=(s=checkEWpn(ri->ewpn)))
				(((weapon*)s)->hit_height)=(value/10000);
				
			break;
			
		case EWPNHZSZ:
			if(0!=(s=checkEWpn(ri->ewpn)))
				(((weapon*)s)->hzsz)=(value/10000);
				
			break;
			
		case EWPNTXSZ:
			if(0!=(s=checkEWpn(ri->ewpn)))
				(((weapon*)s)->txsz)=vbound((value/10000),1,20);
				
			break;
			
		case EWPNTYSZ:
			if(0!=(s=checkEWpn(ri->ewpn)))
				(((weapon*)s)->tysz)=vbound((value/10000),1,20);
				
			break;
			
		case EWPNCOLLDET:
			if(0!=(s=checkEWpn(ri->ewpn)))
				(((weapon*)(s))->scriptcoldet)=value;
				
			break;
		
		case EWPNENGINEANIMATE:
			if(0!=(s=checkEWpn(ri->ewpn)))
				(((weapon*)(s))->do_animation)=value;
				
			break;
		
		
		case EWPNPARENTUID:
			if(0!=(s=checkEWpn(ri->ewpn)))
				s->setParent(sprite::getByUID(value));
			break;
		
		case EWPNPARENT:
			if(0!=(s=checkEWpn(ri->ewpn)))
				(((weapon*)(s))->parentid)= ( (get_qr(qr_OLDEWPNPARENT)) ? value / 10000 : value );
				
			break;
		
		case EWPNSCRIPT:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				(((weapon*)(s))->script)=vbound(value/10000,0,NUMSCRIPTWEAPONS-1);
				if ( get_qr(qr_CLEARINITDONSCRIPTCHANGE))
				{
					for(int32_t q=0; q<8; q++)
						(((weapon*)(s))->initD[q]) = 0;
				}
				on_reassign_script_engine_data(ScriptType::Ewpn, ri->ewpn);
			}
			break;
		
		case EWPNFALLCLK:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				if(((weapon*)(s))->fallclk != 0 && value == 0)
				{
					((weapon*)(s))->cs = ((weapon*)(s))->o_cset;
					((weapon*)(s))->tile = ((weapon*)(s))->o_tile;
				}
				else if(((weapon*)(s))->fallclk == 0 && value != 0) ((weapon*)(s))->o_cset = ((weapon*)(s))->cs;
				((weapon*)(s))->fallclk = vbound(value/10000,0,70);
			}
			break;
		case EWPNFALLCMB:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)(s))->fallCombo = vbound(value/10000,0,MAXCOMBOS-1);
			}
			break;
		case EWPNDROWNCLK:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				if(((weapon*)(s))->drownclk != 0 && value == 0)
				{
					((weapon*)(s))->cs = ((weapon*)(s))->o_cset;
					((weapon*)(s))->tile = ((weapon*)(s))->o_tile;
				}
				else if(((weapon*)(s))->drownclk == 0 && value != 0) ((weapon*)(s))->o_cset = ((weapon*)(s))->cs;
				((weapon*)(s))->drownclk = vbound(value/10000,0,70);
			}
			break;
		case EWPNDROWNCMB:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)(s))->drownCombo = vbound(value/10000,0,MAXCOMBOS-1);
			}
			break;
		case EWPNFAKEZ:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)s)->fakez=get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
				if(((weapon*)s)->fakez < 0) ((weapon*)s)->fakez = 0_zf;
			}
				
			break;
		
		case EWPNGLOWRAD:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)(s))->glowRad = vbound(value/10000,0,255);
			}
			break;
		case EWPNGLOWSHP:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)(s))->glowShape = vbound(value/10000,0,255);
			}
			break;
			
		case EWPNUNBL:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)(s))->unblockable = (value/10000)&WPNUNB_ALL;
			}
			break;
			
		case EWPNSHADOWSPR:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)(s))->spr_shadow = vbound(value/10000, 0, 255);
			}
			break;
		case EWSWHOOKED:
			break; //read-only
		case EWPNTIMEOUT:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)(s))->weap_timeout = vbound(value/10000,0,214748);
			}
			break;case EWPNDEATHITEM:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)(s))->death_spawnitem = vbound(value/10000,-1,MAXITEMS-1);
			}
			break;
		case EWPNDEATHDROPSET:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)(s))->death_spawndropset = vbound(value/10000,-1,MAXITEMDROPSETS-1);
			}
			break;
		case EWPNDEATHIPICKUP:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)(s))->death_item_pflags = value/10000;
			}
			break;
		case EWPNDEATHSPRITE:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)(s))->death_sprite = vbound(value/10000,-255,MAXWPNS-1);
			}
			break;
		case EWPNDEATHSFX:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)(s))->death_sfx = vbound(value/10000,0,WAV_COUNT);
			}
			break;
		case EWPNLIFTLEVEL:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)(s))->lift_level = vbound(value/10000,0,255);
			}
			break;
		case EWPNLIFTTIME:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)(s))->lift_time = vbound(value/10000,0,255);
			}
			break;
		case EWPNLIFTHEIGHT:
			if(0!=(s=checkEWpn(ri->ewpn)))
			{
				((weapon*)(s))->lift_height = zslongToFix(value);
			}
			break;
		
	///----------------------------------------------------------------------------------------------------//
	//Screen Information

		case SCREENSCRDATASIZE:
		{
			int index = map_screen_index(cur_map, ri->screenref);
			if (index < 0) break;

			game->scriptDataResize(index, value/10000);
			break;
		}

		case GAMEGUYCOUNTD:
		{
			int mi = mapind(cur_map, ri->d[rINDEX] / 10000);
			game->guys[mi] = value / 10000;
			break;
		}
			
	///----------------------------------------------------------------------------------------------------//
	//BottleTypes
		
		case BOTTLENEXT:
		{
			if(bottletype* ptr = checkBottleData(ri->bottletyperef))
			{
				ptr->next_type = vbound(value/10000, 0, 64);
			}
		}
		break;

	///----------------------------------------------------------------------------------------------------//
	//Viewport

	case VIEWPORT_TARGET:
	{
		if (auto s = ResolveBaseSprite(value))
			set_viewport_sprite(s);
	}
	break;

	case VIEWPORT_MODE:
	{
		int val = value;
		if (BC::checkBounds(val, (int)ViewportMode::First, (int)ViewportMode::Last) != SH::_NoError)
		{
			break;
		}

		viewport_mode = (ViewportMode)val;
	}
	break;

	case VIEWPORT_X:
	{
		viewport.x = value / 10000;
	}
	break;

	case VIEWPORT_Y:
	{
		viewport.y = value / 10000;
	}
	break;

	case VIEWPORT_WIDTH:
	{
		int val = value / 10000;
		if (BC::checkBounds(val, 0, 256) != SH::_NoError)
			break;

		viewport.w = val;
	}
	break;

	case VIEWPORT_HEIGHT:
	{
		int val = value / 10000;
		if (BC::checkBounds(val, 0, 232) != SH::_NoError)
			break;

		viewport.h = val;
	}
	break;
		
	///----------------------------------------------------------------------------------------------------//
	//Screen Variables
		
		#define	SET_SCREENDATA_VAR_INT32(member, str) \
		{ \
			get_scr(ri->screenref)->member = vbound((value / 10000),-214747,214747); \
		} \
		
		#define	SET_SCREENDATA_VAR_INT16(member, str) \
		{ \
			get_scr(ri->screenref)->member = vbound((value / 10000),0,32767); \
		} \

		#define	SET_SCREENDATA_VAR_BYTE(member, str) \
		{ \
			get_scr(ri->screenref)->member = vbound((value / 10000),0,255); \
		} \
		
		#define SET_SCREENDATA_BYTE_INDEX(member, str, indexbound) \
		{ \
			int32_t indx = ri->d[rINDEX] / 10000; \
			get_scr(ri->screenref)->member[indx] = vbound((value / 10000),0,255); \
		}

		///max screen id is higher! vbound properly... -Z
		#define SET_SCREENDATA_LAYERSCREEN_INDEX(member, str, indexbound) \
		{ \
			int32_t indx = ri->d[rINDEX] / 10000; \
			int32_t scrn_id = value/10000; \
			if ( FFCore.quest_format[vFFScript] < 11 ) ++indx; \
			if (BC::checkIndex(indx, 1, indexbound) != SH::_NoError) \
			{ \
			} \
			else if ( scrn_id > MAPSCRS ) \
			{ \
				Z_scripterrlog("Script attempted to use a mapdata->LayerScreen[%d].\n",scrn_id); \
				Z_scripterrlog("Valid Screen values are (0) through (%d).\n",MAPSCRS); \
			} \
			else get_scr(ri->screenref)->member[indx-1] = vbound((scrn_id),0,MAPSCRS); \
		}
		
		#define SET_SCREENDATA_FLAG(member, str) \
		{ \
			int32_t flag =  (value/10000);  \
			if ( flag != 0 ) \
			{ \
				get_scr(ri->screenref)->member|=flag; \
			} \
			else get_scr(ri->screenref)->.member|= ~flag; \
		} \
		
		#define SET_SCREENDATA_BOOL_INDEX(member, str, indexbound) \
		{ \
			int32_t indx = ri->d[rINDEX] / 10000; \
			if(indx < 0 || indx > indexbound ) \
			{ \
				Z_scripterrlog("Invalid Index passed to Screen->%s[]: %d\n", (indx), str); \
				break; \
			} \
			get_scr(ri->screenref)->member[indx] =( (value/10000) ? 1 : 0 ); \
		}
		
		case SCREENDATAVALID:
		{
			SET_SCREENDATA_VAR_BYTE(valid, "Valid"); //b
			mark_current_region_handles_dirty();
			break;
		}
		case SCREENDATAGUY: 		SET_SCREENDATA_VAR_BYTE(guy, "Guy"); break;		//b
		case SCREENDATASTRING:		SET_SCREENDATA_VAR_INT32(str, "String"); break;		//w
		case SCREENDATAROOM: 		SET_SCREENDATA_VAR_BYTE(room, "RoomType");	break;		//b
		case SCREENDATAITEM:
		{
			auto v = vbound((value / 10000),-1,255);
			auto scr = get_scr(ri->screenref);
			if(v > -1)
				scr->item = v;
			scr->hasitem = v > -1;
			break;
		}
		case SCREENDATAHASITEM: 		SET_SCREENDATA_VAR_BYTE(hasitem, "HasItem"); break;	//b
		case SCREENDATADOORCOMBOSET: 	SET_SCREENDATA_VAR_INT32(door_combo_set, "DoorComboSet"); break;	//w
		case SCREENDATAWARPRETURNC: 	SET_SCREENDATA_VAR_INT32(warpreturnc, "WarpReturnC"); break;	//w
		case SCREENDATASTAIRX: 		SET_SCREENDATA_VAR_BYTE(stairx, "StairsX"); break;	//b
		case SCREENDATASTAIRY: 		SET_SCREENDATA_VAR_BYTE(stairy, "StairsY"); break;	//b
		case SCREENDATAITEMX:		SET_SCREENDATA_VAR_BYTE(itemx, "ItemX"); break; //itemx
		case SCREENDATAITEMY:		SET_SCREENDATA_VAR_BYTE(itemy, "ItemY"); break;	//itemy
		case SCREENDATACOLOUR: 		SET_SCREENDATA_VAR_INT32(color, "CSet"); break;	//w
		case SCREENDATAENEMYFLAGS: 	SET_SCREENDATA_VAR_BYTE(flags11, "EnemyFlags");	break;	//b
		case SCREENDATADOOR: 		SET_SCREENDATA_BYTE_INDEX(door, "Door", 3); break;	//b, 4 of these
		case SCREENDATAEXITDIR: 		SET_SCREENDATA_VAR_BYTE(exitdir, "ExitDir"); break;	//b
		case SCREENDATAPATTERN: 		SET_SCREENDATA_VAR_BYTE(pattern, "Pattern"); break;	//b
		case SCREENDATAWARPARRIVALX: 	SET_SCREENDATA_VAR_BYTE(warparrivalx, "WarpArrivalX"); break;	//b
		case SCREENDATAWARPARRIVALY: 	SET_SCREENDATA_VAR_BYTE(warparrivaly, "WarpArrivalY"); break;	//b
		case SCREENDATASIDEWARPINDEX: 	SET_SCREENDATA_VAR_BYTE(sidewarpindex, "SideWarpIndex"); break;	//b
		case SCREENDATAUNDERCOMBO: 	SET_SCREENDATA_VAR_INT32(undercombo, "Undercombo"); break;	//w
		case SCREENDATAUNDERCSET:	 	SET_SCREENDATA_VAR_BYTE(undercset,	"UnderCSet"); break; //b
		case SCREENDATACATCHALL:	 	SET_SCREENDATA_VAR_INT32(catchall,	"Catchall"); break; //W

		case SCREENDATACSENSITIVE: 	SET_SCREENDATA_VAR_BYTE(csensitive, "CSensitive"); break;	//B
		case SCREENDATANORESET: 		SET_SCREENDATA_VAR_INT32(noreset, "NoReset"); break;	//W
		case SCREENDATANOCARRY: 		SET_SCREENDATA_VAR_INT32(nocarry, "NoCarry"); break;	//W
		
		case SCREENDATATIMEDWARPTICS: 	SET_SCREENDATA_VAR_INT32(timedwarptics, "TimedWarpTimer"); break;	//W
		case SCREENDATANEXTMAP: 		SET_SCREENDATA_VAR_BYTE(nextmap, "NextMap"); break;	//B
		case SCREENDATANEXTSCREEN: 	SET_SCREENDATA_VAR_BYTE(nextscr, "NextScreen"); break;	//B
		case SCREENDATAVIEWX: 		break;//SET_SCREENDATA_VAR_INT32(viewX, "ViewX"); break;	//W
		case SCREENDATAVIEWY: 		break;//SET_SCREENDATA_VAR_INT32(viewY, "ViewY"); break; //W
		case SCREENDATASCREENWIDTH: 	break;//SET_SCREENDATA_VAR_BYTE(scrWidth, "Width"); break;	//B
		case SCREENDATASCREENHEIGHT: 	break;//SET_SCREENDATA_VAR_BYTE(scrHeight,	"Height"); break;	//B
		case SCREENDATAENTRYX: 		
		{
			int32_t newx = vbound((value/10000),0,255);
			get_scr(ri->screenref)->entry_x = newx;
			if ( get_qr(qr_WRITE_ENTRYPOINTS_AFFECTS_HEROCLASS) )
			{
				Hero.respawn_x = (zfix)(newx);
			}
			break;
		}
		case SCREENDATAENTRYY: 		
		{
			
			int32_t newy = vbound((value/10000),0,175);
			get_scr(ri->screenref)->entry_y = newy;
			if ( get_qr(qr_WRITE_ENTRYPOINTS_AFFECTS_HEROCLASS) )
			{
				Hero.respawn_y = (zfix)(newy);
			}
			break;	//B
		}

		case SCREENDATANUMFF: 	
		{
			break;
		}

		case SCREENDATAFFINITIALISED:
		{
			int32_t indx = ri->d[rINDEX] / 10000;
			if (indx < 0 || indx > MAX_FFCID)
			{
				Z_scripterrlog("Invalid Index passed to Screen->%s[]: %d\n", "FFCRunning", (indx));
				break;
			}
			get_script_engine_data(ScriptType::FFC, indx).initialized = (value/10000) ? true : false;
		}
		break;

		case SCREENDATASCRIPTENTRY:
		{
			Z_scripterrlog("Unimplemented: %s\n", "ScriptEntry");
		}
		break;
		case SCREENDATASCRIPTOCCUPANCY:
		{
			Z_scripterrlog("Unimplemented: %s\n", "ScriptOccupancy");
		}
		break;
		case SCREENDATASCRIPTEXIT:
		{
			Z_scripterrlog("Unimplemented: %s\n", "ExitScript");
		}
		break;

		case SCREENDATAOCEANSFX:
		{
			int32_t v = vbound(value/10000, 0, 255);
			auto scr = get_scr(ri->screenref);
			if (scr == hero_scr && scr->oceansfx != v)
			{
				stop_sfx(scr->oceansfx);
				scr->oceansfx = v;
				cont_sfx(scr->oceansfx);
			}
			break;
		}
		case SCREENDATABOSSSFX: 		SET_SCREENDATA_VAR_BYTE(bosssfx, "BossSFX"); break;	//B
		case SCREENDATASECRETSFX:	 	SET_SCREENDATA_VAR_BYTE(secretsfx, "SecretSFX"); break;	//B
		case SCREENDATAHOLDUPSFX:	 	SET_SCREENDATA_VAR_BYTE(holdupsfx,	"ItemSFX"); break; //B
		case SCREENDATASCREENMIDI:
		{
			get_scr(ri->screenref)->screen_midi = vbound((value / 10000)-(MIDIOFFSET_MAPSCR-MIDIOFFSET_ZSCRIPT),-1,32767);
			break;
		}
		case SCREENDATALENSLAYER:	 	SET_SCREENDATA_VAR_BYTE(lens_layer, "LensLayer"); break;	//B, OLD QUESTS ONLY?
		
		case SCREENDATAGUYCOUNT:
		{
			int mi = mapind(cur_map, ri->screenref);
			if(mi > -1)
				game->guys[mi] = vbound(value/10000,10,0);
			break;
		}
		case SCREENDATAEXDOOR:
		{
			int mi = mapind(cur_map, ri->screenref);
			if(mi < 0) break;
			int dir = SH::read_stack(ri->sp+1) / 10000;
			int ind = SH::read_stack(ri->sp+0) / 10000;
			if(unsigned(dir) > 3)
				Z_scripterrlog("Invalid dir '%d' passed to 'Screen->SetExDoor()'; must be 0-3\n", dir);
			else if(unsigned(ind) > 7)
				Z_scripterrlog("Invalid index '%d' passed to 'Screen->SetExDoor()'; must be 0-7\n", ind);
			else
				set_xdoorstate_mi(mi, dir, ind);
			break;
		}

		//These use the same method as SetScreenD
		case SCREENWIDTH:
			// FFScript::set_screenWidth(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)], value/10000);
			break;

		case SCREENHEIGHT:
			// FFScript::set_screenHeight(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)], value/10000);
			break;

		case SCREENVIEWX:
			// FFScript::set_screenViewX(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)], value/10000);
			break;

		case SCREENVIEWY:
			// FFScript::set_screenViewY(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)], value/10000);
			break;

		//These use the method of SetScreenEnemy
		
		case GDD:
			write_array(game->global_d, ri->d[rINDEX] / 10000, value);
			break;
			
		case SDDD:
			FFScript::set_screen_d((ri->d[rINDEX])/10000 + ((get_currdmap())<<7), ri->d[rINDEX2]/10000, value);
			break;
			
		case SDDDD:
			FFScript::set_screen_d(ri->d[rINDEX2]/10000 + ((ri->d[rINDEX]/10000)<<7), ri->d[rEXP1]/10000, value);
			break;
			
		case SCREENSCRIPT:
		{
			mapscr* scr = get_scr(ri->screenref);

			if ( get_qr(qr_CLEARINITDONSCRIPTCHANGE))
			{
				for(int32_t q=0; q<8; q++)
					scr->screeninitd[q] = 0;
			}

			scr->script=vbound(value/10000, 0, NUMSCRIPTSCREEN-1);
			on_reassign_script_engine_data(ScriptType::Screen, ri->screenref);
			break;
		}

		case LIT:
			set_lights(value);
			break;
			
		case WAVY:
			wavy=value/10000;
			break;
			
		case QUAKE:
			quakeclk=value/10000;
			break;
			
		case ROOMTYPE:
			get_scr(ri->screenref)->room=value/10000; break; //this probably doesn't work too well...
		
		case ROOMDATA:
			get_scr(ri->screenref)->catchall=value/10000;
			break;
			
		case PUSHBLOCKLAYER:
			mblock2.blockLayer=vbound(value/10000, 0, 6);
			break;
			
		case PUSHBLOCKCOMBO:
			mblock2.bcombo=value/10000;
			break;
			
		case PUSHBLOCKCSET:
			mblock2.cs=value/10000;
			mblock2.oldcset=value/10000;
			break;
			
		case UNDERCOMBO:
			get_scr(ri->screenref)->undercombo=value/10000;
			break;
			
		case UNDERCSET:
			get_scr(ri->screenref)->undercset=value/10000;
			break;
		
		case SCREEN_DRAW_ORIGIN:
			if (BC::checkBounds(value, (int)DrawOrigin::First, (int)DrawOrigin::Last) != SH::_NoError)
				break;

			ri->screen_draw_origin = (DrawOrigin)value;
			break;

		case SCREEN_DRAW_ORIGIN_TARGET:
		{
			if (ResolveBaseSprite(value))
				ri->screen_draw_origin_target = value;

			break;
		}

	///----------------------------------------------------------------------------------------------------//
	//New Datatype Variables
		
	///----------------------------------------------------------------------------------------------------//
	//spritedata sp-> Variables
		case SPRITEDATATILE: SET_SPRITEDATA_VAR_INT(tile, "Tile"); break;
		case SPRITEDATAMISC: SET_SPRITEDATA_VAR_BYTE(misc, "Misc"); break;
		case SPRITEDATACSETS:
		{
			if(unsigned(ri->spritedataref) > (MAXWPNS-1) )
			{
				Z_scripterrlog("Invalid Sprite ID passed to spritedata->CSet: %d\n", (ri->spritedataref*10000));
			}
			else
			{
				wpnsbuf[ri->spritedataref].csets &= 0xF0;
				wpnsbuf[ri->spritedataref].csets |= vbound((value / 10000),0,15);
			}
			break;
		}
		case SPRITEDATAFLCSET:
		{
			if(unsigned(ri->spritedataref) > (MAXWPNS-1) )
			{
				Z_scripterrlog("Invalid Sprite ID passed to spritedata->FlashCSet: %d\n", (ri->spritedataref*10000));
			}
			else
			{
				wpnsbuf[ri->spritedataref].csets &= 0x0F;
				wpnsbuf[ri->spritedataref].csets |= vbound((value / 10000),0,15)<<4;
			}
			break;
		}
		case SPRITEDATAFRAMES: SET_SPRITEDATA_VAR_BYTE(frames, "Frames"); break;
		case SPRITEDATASPEED: SET_SPRITEDATA_VAR_BYTE(speed, "Speed"); break;
		case SPRITEDATATYPE: SET_SPRITEDATA_VAR_BYTE(type, "Type"); break;
		
	///----------------------------------------------------------------------------------------------------//
	//mapdata m-> Variables
		//mapdata m-> Variables
		
		#define	SET_MAPDATA_VAR_INT32(member) \
		{ \
			if (mapscr *m = ResolveMapdataScr(ri->mapsref)) \
			{ \
				m->member = vbound((value / 10000),-214747,214747); \
			} \
			break; \
		} \
		
		#define	SET_MAPDATA_VAR_INT16(member) \
		{ \
			if (mapscr *m = ResolveMapdataScr(ri->mapsref)) \
			{ \
				m->member = vbound((value / 10000),0,32767); \
			} \
			break; \
		} \

		#define	SET_MAPDATA_VAR_BYTE(member) \
		{ \
			if (mapscr *m = ResolveMapdataScr(ri->mapsref)) \
			{ \
				m->member = vbound((value / 10000),0,255); \
			} \
			break; \
		} \
		
		#define SET_MAPDATA_VAR_INDEX32(member, indexbound) \
		{ \
			int32_t indx = ri->d[rINDEX] / 10000; \
			if (BC::checkIndex(indx, 0, indexbound) != SH::_NoError) \
			{ \
			} \
			else if (mapscr *m = ResolveMapdataScr(ri->mapsref)) \
			{ \
				m->member[indx] = vbound((value / 10000),-214747,214747); \
			} \
			break; \
		} \
		
		#define SET_MAPDATA_VAR_INDEX16(member, indexbound) \
		{ \
			int32_t indx = ri->d[rINDEX] / 10000; \
			if (BC::checkIndex(indx, 0, indexbound) != SH::_NoError) \
			{ \
			} \
			else if (mapscr *m = ResolveMapdataScr(ri->mapsref)) \
			{ \
				m->member[indx] = vbound((value / 10000),-32767,32767); \
			} \
			break; \
		} \

		#define SET_MAPDATA_BYTE_INDEX(member, indexbound) \
		{ \
			int32_t indx = ri->d[rINDEX] / 10000; \
			if (BC::checkIndex(indx, 0, indexbound) != SH::_NoError) \
			{ \
			} \
			else if (mapscr *m = ResolveMapdataScr(ri->mapsref)) \
			{ \
				m->member[indx] = vbound((value / 10000),0,255); \
			} \
			break; \
		}\
		
		#define SET_MAPDATA_LAYER_INDEX(member, indexbound) \
		{ \
			int32_t indx = ri->d[rINDEX] / 10000; \
			if ( FFCore.quest_format[vFFScript] < 11 ) ++indx; \
			if (BC::checkIndex(indx, 1, indexbound) != SH::_NoError) \
			{ \
			} \
			else if (mapscr *m = ResolveMapdataScr(ri->mapsref)) \
			{ \
				m->member[indx-1] = vbound((value / 10000),0,255); \
			} \
			break; \
		} \
		
		#define SET_MAPDATA_LAYERSCREEN_INDEX(member, indexbound) \
		{ \
			int32_t indx = ri->d[rINDEX] / 10000; \
			if ( FFCore.quest_format[vFFScript] < 11 ) ++indx; \
			int32_t scrn_id = value/10000; \
			if (BC::checkIndex(indx, 1, indexbound) != SH::_NoError) \
			{ \
			} \
			else if ( scrn_id > MAPSCRS ) \
			{ \
				Z_scripterrlog("Script attempted to use a mapdata->LayerScreen[%d].\n",scrn_id); \
				Z_scripterrlog("Valid Screen values are (0) through (%d).\n",MAPSCRS); \
			} \
			else if (mapscr *m = ResolveMapdataScr(ri->mapsref)) \
			{ \
				m->member[indx-1] = vbound((scrn_id),0,MAPSCRS); \
			} \
			break; \
		}\
		
		#define SET_MAPDATA_BOOL_INDEX(member, indexbound) \
		{ \
			int32_t indx = ri->d[rINDEX] / 10000; \
			if (BC::checkIndex(indx, 0, indexbound) != SH::_NoError) \
			{ \
			} \
			else if (mapscr *m = ResolveMapdataScr(ri->mapsref)) \
			{ \
				m->member[indx] =( (value/10000) ? 1 : 0 ); \
			} \
			break; \
		} \


		#define SET_FFC_MAPDATA_BOOL_INDEX(member, indexbound) \
		{ \
			int32_t index = ri->d[rINDEX] / 10000; \
			if (auto handle = ResolveMapdataFFC(ri->mapsref, index)) \
			{ \
				handle.ffc->member =( (value/10000) ? 1 : 0 ); \
			} \
			break; \
		} \
		
		#define SET_MAPDATA_FLAG(member) \
		{ \
			int32_t flag =  (value/10000);  \
			if (mapscr *m = ResolveMapdataScr(ri->mapsref)) \
			{ \
				if ( flag != 0 ) \
				{ \
					m->member|=flag; \
				} \
				else m->.member|= ~flag; \
			} \
			break; \
		} \
		
		case MAPDATAVALID:		SET_MAPDATA_VAR_BYTE(valid); break;		//b
		case MAPDATAGUY: 		SET_MAPDATA_VAR_BYTE(guy); break;		//b
		case MAPDATASTRING:		SET_MAPDATA_VAR_INT32(str); break;		//w
		case MAPDATAROOM: 		SET_MAPDATA_VAR_BYTE(room);	break;		//b
		case MAPDATAITEM:
		{
			if (mapscr *m = ResolveMapdataScr(ri->mapsref))
			{
				auto v = vbound((value / 10000),-1,255);
				if(v > -1)
					m->item = v;
				m->hasitem = v > -1;
			}
			break;
		}
		case MAPDATAREGIONID:
		{
			int region_id = value / 10000;
			if (BC::checkBounds(region_id, 0, 9) != SH::_NoError)
				break;

			auto result = decode_mapdata_ref(ri->mapsref);
			if (result.scr)
			{
				if (result.type == mapdata_type::CanonicalScreen)
				{
					Regions[result.scr->map].set_region_id(result.screen, region_id);
				}
				else
				{
					scripting_log_error_with_context("This may only be set for canonical screens");
				}
			}
			else
			{
				scripting_log_error_with_context("mapdata pointer is either invalid or uninitialised");
			}
			break;
		}
		case MAPDATAHASITEM: 		SET_MAPDATA_VAR_BYTE(hasitem); break;	//b
		case MAPDATADOORCOMBOSET: 	SET_MAPDATA_VAR_INT32(door_combo_set); break;	//w
		case MAPDATAWARPRETURNC: 	SET_MAPDATA_VAR_INT32(warpreturnc); break;	//w
		case MAPDATASTAIRX: 		SET_MAPDATA_VAR_BYTE(stairx); break;	//b
		case MAPDATASTAIRY: 		SET_MAPDATA_VAR_BYTE(stairy); break;	//b
		case MAPDATAITEMX:		SET_MAPDATA_VAR_BYTE(itemx); break; //itemx
		case MAPDATAITEMY:		SET_MAPDATA_VAR_BYTE(itemy); break;	//itemy
		case MAPDATACOLOUR: 		SET_MAPDATA_VAR_INT32(color); break;	//w
		case MAPDATAENEMYFLAGS: 	SET_MAPDATA_VAR_BYTE(flags11);	break;	//b
		case MAPDATAEXITDIR: 		SET_MAPDATA_VAR_BYTE(exitdir); break;	//b
		case MAPDATAPATTERN: 		SET_MAPDATA_VAR_BYTE(pattern); break;	//b
		case MAPDATAWARPARRIVALX: 	SET_MAPDATA_VAR_BYTE(warparrivalx); break;	//b
		case MAPDATAWARPARRIVALY: 	SET_MAPDATA_VAR_BYTE(warparrivaly); break;	//b

		case MAPDATASIDEWARPINDEX: 	SET_MAPDATA_VAR_BYTE(sidewarpindex); break;	//b
		case MAPDATAUNDERCOMBO: 	SET_MAPDATA_VAR_INT32(undercombo); break;	//w
		case MAPDATAUNDERCSET:	 	SET_MAPDATA_VAR_BYTE(undercset); break; //b
		case MAPDATACATCHALL:	 	SET_MAPDATA_VAR_INT32(catchall); break; //W

		case MAPDATACSENSITIVE: 	SET_MAPDATA_VAR_BYTE(csensitive); break;	//B
		case MAPDATANORESET: 		SET_MAPDATA_VAR_INT32(noreset); break;	//W
		case MAPDATANOCARRY: 		SET_MAPDATA_VAR_INT32(nocarry); break;	//W
		case MAPDATATIMEDWARPTICS: 	SET_MAPDATA_VAR_INT32(timedwarptics); break;	//W
		case MAPDATANEXTMAP: 		SET_MAPDATA_VAR_BYTE(nextmap); break;	//B
		case MAPDATANEXTSCREEN: 	SET_MAPDATA_VAR_BYTE(nextscr); break;	//B
		case MAPDATAVIEWX: 		break;//SET_MAPDATA_VAR_INT32(viewX, "ViewX"); break;	//W
		case MAPDATASCRIPT:
		{
			auto result = decode_mapdata_ref(ri->mapsref);
			if (result.scr)
			{
				if (result.current())
				{
					if (get_qr(qr_CLEARINITDONSCRIPTCHANGE))
					{
						for (int q=0; q<8; q++)
							result.scr->screeninitd[q] = 0;
					}

					on_reassign_script_engine_data(ScriptType::Screen, ri->screenref);
				}

				result.scr->script = vbound(value/10000, 0, NUMSCRIPTSCREEN-1);
			} 
			else 
			{ 
				Z_scripterrlog("Script attempted to use a mapdata->%s on an invalid pointer\n","Script");
			} 
			break;
		}
		case MAPDATAVIEWY: 		break;//SET_MAPDATA_VAR_INT32(viewY, "ViewY"); break; //W
		case MAPDATASCREENWIDTH: 	break;//SET_MAPDATA_VAR_BYTE(scrWidth, "Width"); break;	//B
		case MAPDATASCREENHEIGHT: 	break;//SET_MAPDATA_VAR_BYTE(scrHeight,	"Height"); break;	//B
		case MAPDATAENTRYX: 		SET_MAPDATA_VAR_BYTE(entry_x); break;	//B
		case MAPDATAENTRYY: 		SET_MAPDATA_VAR_BYTE(entry_y); break;	//B
		
		//Number of ffcs that are in use (have valid data
		case MAPDATANUMFF: 	
		{
			break;
		}

		case MAPDATAINTID:
		{
			int32_t index = (ri->d[rINDEX]/10000);
			int32_t dindex = ri->d[rINDEX2]/10000;

			if (BC::checkBounds(dindex, 0, 7) != SH::_NoError)
				break;

			if (auto handle = ResolveMapdataFFC(ri->mapsref, index))
				handle.ffc->initd[dindex] = value;
			break;
		}	

		case MAPDATASCRIPTENTRY:
		{
			Z_scripterrlog("Unimplemented: %s\n", "ScriptEntry");
		}
		break;
		case MAPDATASCRIPTOCCUPANCY:
		{
			Z_scripterrlog("Unimplemented: %s\n", "ScriptOccupancy");
		}
		break;
		case MAPDATASCRIPTEXIT:
		{
			Z_scripterrlog("Unimplemented: %s\n", "ExitScript");
		}
		break;

		case MAPDATAOCEANSFX:
		{
			if (mapscr *m = ResolveMapdataScr(ri->mapsref))
			{
				int32_t v = vbound(value/10000, 0, 255);
				if(m == hero_scr && m->oceansfx != v)
				{
					stop_sfx(m->oceansfx);
					m->oceansfx = v;
					cont_sfx(m->oceansfx);
				}
				else m->oceansfx = v;
			}
			break;
		}
		case MAPDATABOSSSFX: 		SET_MAPDATA_VAR_BYTE(bosssfx); break;	//B
		case MAPDATASECRETSFX:	 	SET_MAPDATA_VAR_BYTE(secretsfx); break;	//B
		case MAPDATAHOLDUPSFX:	 	SET_MAPDATA_VAR_BYTE(holdupsfx); break; //B
		case MAPDATASCREENMIDI:
		{
			if (mapscr *m = ResolveMapdataScr(ri->mapsref))
			{
				m->screen_midi = vbound((value / 10000)-(MIDIOFFSET_MAPSCR-MIDIOFFSET_ZSCRIPT),-1,32767);
			}
			break;
		}
		case MAPDATALENSLAYER:	 	SET_MAPDATA_VAR_BYTE(lens_layer); break;	//B, OLD QUESTS ONLY?

		case MAPDATASCRDATASIZE:
		{
			if (mapscr *m = ResolveMapdataScr(ri->mapsref))
			{
				int index = get_ref_map_index(ri->mapsref);
				if (index < 0) break;

				game->scriptDataResize(index, value/10000);
			}
			break;
		}
		case MAPDATAGUYCOUNT:
		{
			if (mapscr *m = ResolveMapdataScr(ri->mapsref))
			{
				int mi = get_mi(ri->mapsref);
				if(mi > -1)
				{
					game->guys[mi] = vbound(value/10000,10,0);
					break;
				}
			}
			break;
		}
		case MAPDATAEXDOOR:
		{
			if (mapscr *m = ResolveMapdataScr(ri->mapsref))
			{
				int mi = get_mi(ri->mapsref);
				if(mi < 0) break;
				int dir = SH::read_stack(ri->sp+1) / 10000;
				int ind = SH::read_stack(ri->sp+0) / 10000;
				if(unsigned(dir) > 3)
					Z_scripterrlog("Invalid dir '%d' passed to 'mapdata->SetExDoor()'; must be 0-3\n", dir);
				else if(unsigned(ind) > 7)
					Z_scripterrlog("Invalid index '%d' passed to 'mapdata->SetExDoor()'; must be 0-7\n", ind);
				else
					set_xdoorstate_mi(mi, dir, ind);
			}
			break;
		}
		
		///----------------------------------------------------------------------------------------------------//
		//dmapdata dmd-> Variables
		case DMAPDATAMAP: 	//byte
		{
			DMaps[ri->dmapsref].map = ((byte)(value / 10000)) - 1; break;
		}
		case DMAPDATALEVEL:	//word
		{
			DMaps[ri->dmapsref].level = ((word)(value / 10000)); break;
		}
		case DMAPDATAOFFSET:	//char
		{
			DMaps[ri->dmapsref].xoff = ((char)(value / 10000)); break;
		}
		case DMAPDATACOMPASS:	//byte
		{
			DMaps[ri->dmapsref].compass = ((byte)(value / 10000)); break;
		}
		case DMAPDATAPALETTE:	//word
		{
			DMaps[ri->dmapsref].color= ((word)(value / 10000));
			if(ri->dmapsref == cur_dmap)
			{
				loadlvlpal(DMaps[ri->dmapsref].color);
				currcset = DMaps[ri->dmapsref].color;
			}
			break;
		}
		case DMAPDATAMIDI:	//byte
		{
			DMaps[ri->dmapsref].midi = ((byte)((value / 10000)+MIDIOFFSET_DMAP)); break;
		}
		case DMAPDATACONTINUE:	//byte
		{
			DMaps[ri->dmapsref].cont = ((byte)(value / 10000)); break;
		}
		case DMAPDATATYPE:	//byte
		{
			DMaps[ri->dmapsref].type = (((byte)(value / 10000))&dmfTYPE) | (DMaps[ri->dmapsref].type&~dmfTYPE); break;
		}
		case DMAPSCRIPT:	//byte
		{
			DMaps[ri->dmapsref].script = vbound((value / 10000),0,NUMSCRIPTSDMAP-1);
			on_reassign_script_engine_data(ScriptType::DMap, ri->dmapsref);
			break;
		}
		case DMAPDATASIDEVIEW:	//byte, treat as bool
		{
			DMaps[ri->dmapsref].sideview = ((value) ? 1 : 0); break;
		}
		case DMAPDATAMUISCTRACK:	//byte
		{
			DMaps[ri->dmapsref].tmusictrack= ((byte)(value / 10000)); break;
		}
		case DMAPDATASUBSCRA:
		{
			bool changed = DMaps[ri->dmapsref].active_subscreen != ((byte)(value / 10000));
			DMaps[ri->dmapsref].active_subscreen= ((byte)(value / 10000));
			if(changed&&ri->dmapsref==cur_dmap)
				update_subscreens();
			break;
		}
		case DMAPDATASUBSCRP:
		{
			bool changed = DMaps[ri->dmapsref].passive_subscreen != ((byte)(value / 10000));
			DMaps[ri->dmapsref].passive_subscreen= ((byte)(value / 10000));
			if(changed&&ri->dmapsref==cur_dmap)
				update_subscreens();
			break;
		}
		case DMAPDATASUBSCRO:
		{
			bool changed = DMaps[ri->dmapsref].overlay_subscreen != ((byte)(value / 10000));
			DMaps[ri->dmapsref].overlay_subscreen = ((byte)(value / 10000));
			if(changed&&ri->dmapsref==cur_dmap)
				update_subscreens();
			break;
		}
		case DMAPDATAFLAGS:	 //int32_t
		{
			DMaps[ri->dmapsref].flags = (value / 10000); break;
		}
		case DMAPDATAMIRRDMAP:
		{
			DMaps[ri->dmapsref].mirrorDMap = vbound(value / 10000, -1, MAXDMAPS); break;
		}
		case DMAPDATALOOPSTART:
		{
			DMaps[ri->dmapsref].tmusic_loop_start = value; 
			if (ri->dmapsref == cur_dmap)
			{
				if (FFCore.doing_dmap_enh_music(cur_dmap))
				{
					zcmusic_set_loop(zcmusic, double(DMaps[cur_dmap].tmusic_loop_start / 10000.0), double(DMaps[cur_dmap].tmusic_loop_end / 10000.0));
				}
			}
			break;
		}
		case DMAPDATALOOPEND:
		{
			DMaps[ri->dmapsref].tmusic_loop_end = value;
			if (ri->dmapsref == cur_dmap)
			{
				if (FFCore.doing_dmap_enh_music(cur_dmap))
				{
					zcmusic_set_loop(zcmusic, double(DMaps[cur_dmap].tmusic_loop_start / 10000.0), double(DMaps[cur_dmap].tmusic_loop_end / 10000.0));
				}
			}
			break;
		}
		case DMAPDATAXFADEIN:
		{
			DMaps[ri->dmapsref].tmusic_xfade_in = (value / 10000);
			break;
		}
		case DMAPDATAXFADEOUT:
		{
			DMaps[ri->dmapsref].tmusic_xfade_out = (value / 10000);
			if (DMaps[cur_dmap].tmusic[0]!=0 && strcmp(DMaps[ri->dmapsref].tmusic, zcmusic->filename) == 0)
			{
				zcmusic->fadeoutframes = (value / 10000);
			}
			break;
		}
		case DMAPDATAINTROSTRINGID:
		{
			DMaps[ri->dmapsref].intro_string_id = (value / 10000);
			break;
		}
		case MUSICUPDATECOND:
		{
			FFCore.music_update_cond = vbound(value / 10000, 0, 255);
			break;
		}
		case DMAPDATAASUBSCRIPT:	//byte
		{
			DMaps[ri->dmapsref].active_sub_script = vbound((value / 10000),0,NUMSCRIPTSDMAP-1);
			on_reassign_script_engine_data(ScriptType::ScriptedActiveSubscreen, ri->dmapsref);
			break;
		}
		case DMAPDATAMAPSCRIPT:	//byte
		{
			DMaps[ri->dmapsref].onmap_script = vbound((value / 10000),0,NUMSCRIPTSDMAP-1);
			on_reassign_script_engine_data(ScriptType::OnMap, ri->dmapsref);
			break;
		}
		case DMAPDATAPSUBSCRIPT:	//byte
		{
			FFScript::deallocateAllScriptOwned(ScriptType::ScriptedPassiveSubscreen, ri->dmapsref);
			word val = vbound((value / 10000),0,NUMSCRIPTSDMAP-1);
			if (FFCore.doscript(ScriptType::ScriptedPassiveSubscreen) && ri->dmapsref == cur_dmap && val == DMaps[ri->dmapsref].passive_sub_script)
				break;
			DMaps[ri->dmapsref].passive_sub_script = val;
			if(ri->dmapsref == cur_dmap)
			{
				FFCore.doscript(ScriptType::ScriptedPassiveSubscreen) = val != 0;
			};
			break;
		}

		///----------------------------------------------------------------------------------------------------//
		//messagedata msgd-> Variables
		 

		case MESSAGEDATANEXT: //W
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else 
				MsgStrings[ID].nextstring = vbound((value/10000), 0, (msg_count-1));
			break;
		}	

		case MESSAGEDATATILE: //W
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else 
				MsgStrings[ID].tile = vbound((value/10000), 0, (NEWMAXTILES));
			break;
		}	

		case MESSAGEDATACSET: //b
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else 
				MsgStrings[ID].cset = ((byte)vbound((value/10000), 0, 15));
			break;
		}	
		case MESSAGEDATATRANS: //BOOL
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else 
				(MsgStrings[ID].trans) = ((value)?true:false);
			break;
		}	
		case MESSAGEDATAFONT: //B
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else 
				MsgStrings[ID].font = ((byte)vbound((value/10000), 0, 255));
			break;
		}	
		case MESSAGEDATAX: //SHORT
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else 
				MsgStrings[ID].x = ((int16_t)vbound((value/10000), SHRT_MIN, SHRT_MAX));
			break;
		}	
		case MESSAGEDATAY: //SHORT
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else 
				MsgStrings[ID].y = ((int16_t)vbound((value/10000), SHRT_MIN, SHRT_MAX));
			break;
		}	
		case MESSAGEDATAW: //UNSIGNED SHORT
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else 
				MsgStrings[ID].w = ((uint16_t)vbound((value/10000), 0, USHRT_MAX));
			break;
		}	
		case MESSAGEDATAH: //UNSIGNED SHORT
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else 
				MsgStrings[ID].h = ((uint16_t)vbound((value/10000), 0, USHRT_MAX));
			break;
		}	
		case MESSAGEDATASFX: //BYTE
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else 
				MsgStrings[ID].sfx = ((byte)vbound((value/10000), 0, 255));
			break;
		}	
		case MESSAGEDATALISTPOS: //WORD
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else 
				MsgStrings[ID].listpos = vbound((value/10000), 1, (msg_count-1));
			break;
		}	
		case MESSAGEDATAVSPACE: //BYTE
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else 
				MsgStrings[ID].vspace = ((byte)vbound((value/10000), 0, 255));
			break;
		}	
		case MESSAGEDATAHSPACE: //BYTE
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else 
				MsgStrings[ID].hspace = ((byte)vbound((value/10000), 0, 255));
			break;
		}	
		case MESSAGEDATAFLAGS: //BYTE
		{
			int32_t ID = ri->zmsgref;	

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else 
				MsgStrings[ID].stringflags = ((byte)vbound((value/10000), 0, 255));
			break;
		}
		case MESSAGEDATAPORTTILE: //INT
		{
			int32_t ID = ri->zmsgref;
			
			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].portrait_tile = vbound((value/10000), 0, (NEWMAXTILES));
			break;
		}
		case MESSAGEDATAPORTCSET: //BYTE
		{
			int32_t ID = ri->zmsgref;
			
			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].portrait_cset = ((byte)vbound((value/10000), 0, 15));
			break;
		}
		case MESSAGEDATAPORTX: //BYTE
		{
			int32_t ID = ri->zmsgref;
			
			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].portrait_x = ((byte)vbound((value/10000), 0, 255));
			break;
		}
		case MESSAGEDATAPORTY: //BYTE
		{
			int32_t ID = ri->zmsgref;
			
			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].portrait_y = ((byte)vbound((value/10000), 0, 255));
			break;
		}
		case MESSAGEDATAPORTWID: //BYTE
		{
			int32_t ID = ri->zmsgref;
			
			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].portrait_tw = ((byte)vbound((value/10000), 0, 16));
			break;
		}
		case MESSAGEDATAPORTHEI: //BYTE
		{
			int32_t ID = ri->zmsgref;
			
			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].portrait_th = ((byte)vbound((value/10000), 0, 14));
			break;
		}

	///----------------------------------------------------------------------------------------------------//
	//combodata cd-> Setter Variables
	//newcombo	
		#define	SET_COMBO_VAR_INT(member) \
		{ \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
			} \
			else \
			{ \
				screen_combo_modify_pre(ri->combosref); \
				combobuf[ri->combosref].member = vbound((value / 10000),0,214747); \
				screen_combo_modify_post(ri->combosref); \
				\
			} \
		} \
		
		#define	SET_COMBO_VAR_DWORD(member) \
		{ \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
			} \
			else \
			{ \
				screen_combo_modify_pre(ri->combosref); \
				combobuf[ri->combosref].member = vbound((value / 10000),0,32767); \
				screen_combo_modify_post(ri->combosref); \
			} \
		} \

		#define	SET_COMBO_VAR_BYTE(member) \
		{ \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
			} \
			else \
			{ \
			    screen_combo_modify_pre(ri->combosref); \
				combobuf[ri->combosref].member = vbound((value / 10000),0,255); \
				screen_combo_modify_post(ri->combosref); \
			} \
		} \
		
		#define SET_COMBO_FLAG(member) \
		{ \
			int32_t flag =  (value/10000);  \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
			} \
			else \
			{ \
				if ( flag != 0 ) \
				{ \
					combobuf[ri->combosref].member|=flag; \
				} \
				else combobuf[ri->combosref].member|= ~flag; \
			} \
		} \
		
		//comboclass
		#define	SET_COMBOCLASS_VAR_INT(member) \
		{ \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
			} \
			else \
			{ \
				combo_class_buf[combobuf[ri->combosref].type].member = vbound((value / 10000),0,214747); \
			} \
		} \
		
		#define	SET_COMBOCLASS_VAR_DWORD(member) \
		{ \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
			} \
			else \
			{ \
				combo_class_buf[combobuf[ri->combosref].type].member = vbound((value / 10000),0,32767); \
			} \
		} \

		#define	SET_COMBOCLASS_VAR_BYTE(member) \
		{ \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
			} \
			else \
			{ \
				combo_class_buf[combobuf[ri->combosref].type].member = vbound((value / 10000),0,255); \
			} \
		} \

		#define SET_COMBOCLASS_BYTE_INDEX(member, indexbound) \
		{ \
				int32_t indx = ri->d[rINDEX] / 10000; \
				if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
				{ \
					scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
				} \
				else if ( indx < 0 || indx > indexbound ) \
				{ \
					scripting_log_error_with_context("Invalid Array Index: {}", indx); \
				} \
				else \
				{ \
					combo_class_buf[combobuf[ri->combosref].type].member[indx] = vbound((value / 10000),0,255); \
				} \
		}
		
		#define SET_COMBOCLASS_FLAG(member, str) \
		{ \
			int32_t flag =  (value/10000);  \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				scripting_log_error_with_context("Invalid combodata ID: {}", (ri->combosref*10000)); \
			} \
			else \
			{ \
				if ( flag != 0 ) \
				{ \
					combo_class_buf[combobuf[ri->combosref].type].member|=flag; \
				} \
				else combo_class_buf[combobuf[ri->combosref].type].member|= ~flag; \
			} \
		} \
		
		//NEWCOMBO STRUCT
		case COMBODTILE:	SET_COMBO_VAR_INT(tile); break;						//word
		case COMBODOTILE:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else
			{
				newcombo& cdata = combobuf[ri->combosref];
				cdata.o_tile = vbound((value / 10000),0,NEWMAXTILES);
				if(get_qr(qr_NEW_COMBO_ANIMATION))
				{
					cdata.tile = cdata.o_tile + ((1+cdata.skipanim)*cdata.cur_frame);
					if(int32_t rowoffset = TILEROW(cdata.tile)-TILEROW(cdata.o_tile))
					{
						cdata.tile += cdata.skipanimy * rowoffset * TILES_PER_ROW;
					}
					combo_caches::drawing.refresh(ri->combosref);
				}
			}
			break;
		}
		case COMBODFRAME:		SET_COMBO_VAR_BYTE(cur_frame); break;				//char
		case COMBODACLK:		SET_COMBO_VAR_BYTE(aclk); break;				//char
		case COMBODATASCRIPT:	SET_COMBO_VAR_DWORD(script); break;						//word
		case COMBODASPEED:	SET_COMBO_VAR_BYTE(speed); break;						//char
		case COMBODFLIP:	SET_COMBO_VAR_BYTE(flip); break;						//char
		case COMBODWALK:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else
			{
				combobuf[ri->combosref].walk &= ~0x0F;
				combobuf[ri->combosref].walk |= (value / 10000)&0x0F;
			}
			break;
		}
		case COMBODEFFECT:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else
			{
				combobuf[ri->combosref].walk &= ~0xF0;
				combobuf[ri->combosref].walk |= ((value / 10000)&0x0F)<<4;
			}
			break;
		}
		case COMBODTYPE:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else
			{
				screen_combo_modify_pre(ri->combosref);
				combobuf[ri->combosref].type = vbound((value / 10000),0,255);
				screen_combo_modify_post(ri->combosref);
			}
			break;
		}
		case COMBODCSET:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else
			{
				screen_combo_modify_pre(ri->combosref);
				int8_t v = vbound(value, -8, 7);
				combobuf[ri->combosref].csets &= ~0xF;
				combobuf[ri->combosref].csets |= v;
				screen_combo_modify_post(ri->combosref);
			}
			break;
		}
		case COMBODCSET2FLAGS:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else
			{
				screen_combo_modify_pre(ri->combosref);
				combobuf[ri->combosref].csets &= 0xF;
				combobuf[ri->combosref].csets |= (value&0xF)<<4;
				screen_combo_modify_post(ri->combosref);
			}
			break;
		}
		case COMBODFOO:		break;							//W
		case COMBODFRAMES:	SET_COMBO_VAR_BYTE(frames); break;						//C
		case COMBODNEXTD:	SET_COMBO_VAR_INT(nextcombo); break;						//W
		case COMBODNEXTC:	SET_COMBO_VAR_BYTE(nextcset); break;					//C
		case COMBODFLAG:	SET_COMBO_VAR_BYTE(flag); break;						//C
		case COMBODSKIPANIM:	SET_COMBO_VAR_BYTE(skipanim); break;					//C
		case COMBODNEXTTIMER:	SET_COMBO_VAR_DWORD(nexttimer); break;					//W
		case COMBODAKIMANIMY:	SET_COMBO_VAR_BYTE(skipanimy); break;					//C
		case COMBODANIMFLAGS:	SET_COMBO_VAR_BYTE(animflags); break;					//C
		case COMBODUSRFLAGS:	SET_COMBO_VAR_INT(usrflags); break;					//LONG
		case COMBODTRIGGERITEM:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->triggeritem = vbound(value/10000,0,255);
			break;
		}
		case COMBODTRIGGERTIMER:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else
			{
				screen_combo_modify_pre(ri->combosref);
				if(auto* trig = get_first_combo_trigger())
					trig->trigtimer = vbound(value/10000,0,65535);
				screen_combo_modify_post(ri->combosref);
			}
			break;
		}
		case COMBODTRIGGERSFX:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trigsfx = vbound(value/10000,0,255);
			break;
		}
		case COMBODTRIGGERCHANGECMB:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trigchange = vbound(value/10000,-65535,65535);
			break;
		}
		case COMBODTRIGGERPROX:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trigprox = vbound(value/10000,0,65535);
			break;
		}
		case COMBODTRIGGERLIGHTBEAM:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->triglbeam = vbound(value/10000,0,32);
			break;
		}
		case COMBODTRIGGERCTR:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trigctr = vbound(value/10000, sscMIN, MAX_COUNTERS-1);
			break;
		}
		case COMBODTRIGGERCTRAMNT:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trigctramnt = vbound(value/10000, -65535, 65535);
			break;
		}
		
		case COMBODTRIGGERCOOLDOWN:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trigcooldown = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODTRIGGERCOPYCAT:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trigcopycat = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODTRIGITEMPICKUP:
		{
			const int32_t allowed_pflags = ipHOLDUP | ipTIMER | ipSECRETS | ipCANGRAB;
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->spawnip = (value/10000)&allowed_pflags;
			break;
		}
		case COMBODTRIGEXSTATE:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->exstate = vbound(value/10000, -1, 31);
			break;
		}
		case COMBODTRIGEXDOORDIR:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->exdoor_dir = vbound(value/10000, -1, 3);
			break;
		}
		case COMBODTRIGEXDOORIND:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->exdoor_ind = vbound(value/10000, 0, 7);
			break;
		}
		case COMBODTRIGSPAWNENEMY:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->spawnenemy = vbound(value/10000, 0, 511);
			break;
		}
		case COMBODTRIGSPAWNITEM:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->spawnitem = vbound(value/10000, -255, 255);
			break;
		}
		case COMBODTRIGCSETCHANGE:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trigcschange = vbound(value/10000, -15, 15);
			break;
		}
		case COMBODTRIGLITEMS:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trig_levelitems = (value/10000)&liALL;
			break;
		}
		case COMBODTRIGDMAPLVL:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trigdmlevel = vbound(value/10000, -1, MAXLEVELS-1);
			break;
		}
		case COMBODTRIGTINTR:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trigtint[0] = scripting_write_pal_color(vbound(value/10000, -scripting_max_color_val, scripting_max_color_val));
			break;
		}
		case COMBODTRIGTINTG:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trigtint[1] = scripting_write_pal_color(vbound(value/10000, -scripting_max_color_val, scripting_max_color_val));
			break;
		}
		case COMBODTRIGTINTB:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trigtint[2] = scripting_write_pal_color(vbound(value/10000, -scripting_max_color_val, scripting_max_color_val));
			break;
		}
		case COMBODTRIGLVLPAL:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->triglvlpalette = vbound(value/10000, -1, 512);
			break;
		}
		case COMBODTRIGBOSSPAL:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trigbosspalette = vbound(value/10000, -1, 29);
			break;
		}
		case COMBODTRIGQUAKETIME:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trigquaketime = zc_max(value/10000, -1);
			break;
		}
		case COMBODTRIGWAVYTIME:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trigwavytime = zc_max(value/10000, -1);
			break;
		}
		case COMBODTRIGSWORDJINX:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trig_swjinxtime = zc_max(value/10000, -2);
			break;
		}
		case COMBODTRIGITEMJINX:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trig_itmjinxtime = zc_max(value/10000, -2);
			break;
		}
		case COMBODTRIGSHIELDJINX:
		{
			if (ri->combosref < 0 || ri->combosref >(MAXCOMBOS - 1))
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trig_shieldjinxtime = zc_max(value / 10000, -2);
			break;
		}
		case COMBODTRIGSTUN:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trig_stuntime = zc_max(value/10000, -2);
			break;
		}
		case COMBODTRIGBUNNY:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trig_bunnytime = zc_max(value/10000, -2);
			break;
		}
		case COMBODTRIGPUSHTIME:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trig_pushtime = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODLIFTGFXCOMBO:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else combobuf[ri->combosref].liftcmb = vbound(value/10000, 0, MAXCOMBOS);
			break;
		}
		case COMBODLIFTGFXCCSET:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else combobuf[ri->combosref].liftcs = vbound(value/10000, 0, 13);
			break;
		}
		case COMBODLIFTUNDERCMB:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else combobuf[ri->combosref].liftundercmb = vbound(value/10000, 0, MAXCOMBOS);
			break;
		}
		case COMBODLIFTUNDERCS:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else combobuf[ri->combosref].liftundercs = vbound(value/10000, 0, 13);
			break;
		}
		case COMBODLIFTDAMAGE:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else combobuf[ri->combosref].liftdmg = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODLIFTLEVEL:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else combobuf[ri->combosref].liftlvl = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODLIFTITEM:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else combobuf[ri->combosref].liftitm = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODLIFTGFXTYPE:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else combobuf[ri->combosref].liftgfx = vbound(value/10000, 0, 2);
			break;
		}
		case COMBODLIFTGFXSPRITE:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else combobuf[ri->combosref].liftsprite = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODLIFTSFX:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else combobuf[ri->combosref].liftsfx = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODLIFTBREAKSPRITE:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else combobuf[ri->combosref].liftbreaksprite = vbound(value/10000, -4, 255);
			break;
		}
		case COMBODLIFTBREAKSFX:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else combobuf[ri->combosref].liftbreaksfx = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODLIFTHEIGHT:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else combobuf[ri->combosref].lifthei = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODLIFTTIME:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else combobuf[ri->combosref].lifttime = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODLIFTLIGHTRAD:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else combobuf[ri->combosref].lift_weap_data.light_rads[WPNSPR_BASE] = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODLIFTLIGHTSHAPE:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else combobuf[ri->combosref].lift_weap_data.glow_shape = vbound(value/10000, 0, 2);
			break;
		}
		case COMBODLIFTWEAPONITEM:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else combobuf[ri->combosref].lift_parent_item = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODTRIGGERLSTATE:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trig_lstate = vbound(value/10000, 0, 31);
			break;
		}
		case COMBODTRIGGERGSTATE:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trig_gstate = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODTRIGGERGROUP:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trig_group = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODTRIGGERGROUPVAL:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trig_group_val = vbound(value/10000, 0, 65535);
			break;
		}
		case COMBODTRIGGERGTIMER:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trig_statetime = vbound(value/10000, 0, 214748);
			break;
		}
		case COMBODTRIGGERGENSCRIPT:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->trig_genscr = vbound(value/10000, 0, 65535);
			break;
		}
		case COMBODTRIGGERLEVEL:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else if(auto* trig = get_first_combo_trigger())
				trig->triggerlevel = vbound(value/10000, 0, 214747);
			break;
		}
		case COMBODNUMTRIGGERS:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else
				combobuf[ri->combosref].triggers.resize(vbound(value / 10000, 0, MAX_COMBO_TRIGGERS));
			break;
		}
		case COMBODONLYGEN:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else
				combobuf[ri->combosref].only_gentrig = value != 0 ? 1 : 0;
			break;
		}
		case COMBOD_Z_HEIGHT:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else
				combobuf[ri->combosref].z_height = zslongToFix(value);
			break;
		}
		case COMBOD_Z_STEP_HEIGHT:
		{
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
			{
				scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
			}
			else
				combobuf[ri->combosref].z_step_height = zslongToFix(zc_max(0,value));
			break;
		}
	
		


		//COMBOCLASS STRUCT
		//case COMBODNAME:		//CHAR[64], STRING
		case COMBODBLOCKNPC:		SET_COMBOCLASS_VAR_BYTE(block_enemies); break;			//C
		case COMBODBLOCKHOLE:		SET_COMBOCLASS_VAR_BYTE(block_hole); break;			//C
		case COMBODBLOCKTRIG:		SET_COMBOCLASS_VAR_BYTE(block_trigger); break; 		//C
		// Note: not used?
		case COMBODBLOCKWEAPON:		SET_COMBOCLASS_BYTE_INDEX(block_weapon, 32); break;			//C, 32 INDICES
		case COMBODCONVXSPEED:		SET_COMBOCLASS_VAR_DWORD(conveyor_x_speed); break;		//SHORT
		case COMBODCONVYSPEED:		SET_COMBOCLASS_VAR_DWORD(conveyor_y_speed); break;		//SHORT
		case COMBODSPAWNNPC:		SET_COMBOCLASS_VAR_DWORD(create_enemy); break;			//W
		case COMBODSPAWNNPCWHEN:	SET_COMBOCLASS_VAR_BYTE(create_enemy_when); break;		//C
		case COMBODSPAWNNPCCHANGE:	SET_COMBOCLASS_VAR_INT(create_enemy_change); break;		//LONG
		case COMBODDIRCHANGETYPE:	SET_COMBOCLASS_VAR_BYTE(directional_change_type); break;		//C
		case COMBODDISTANCECHANGETILES:	SET_COMBOCLASS_VAR_INT(distance_change_tiles); break; 	//LONG
		case COMBODDIVEITEM:		SET_COMBOCLASS_VAR_DWORD(dive_item); break;				//SHORT
		case COMBODDOCK:		SET_COMBOCLASS_VAR_BYTE(dock); break;					//C
		case COMBODFAIRY:		SET_COMBOCLASS_VAR_BYTE(fairy); break;					//C
		case COMBODFFATTRCHANGE:	SET_COMBOCLASS_VAR_BYTE(ff_combo_attr_change); break;	//C
		case COMBODFOORDECOTILE:	SET_COMBOCLASS_VAR_INT(foot_decorations_tile); break;		//LONG
		case COMBODFOORDECOTYPE:	SET_COMBOCLASS_VAR_BYTE(foot_decorations_type); break;	//C
		case COMBODHOOKSHOTPOINT:	SET_COMBOCLASS_VAR_BYTE(hookshot_grab_point); break;		//C
		case COMBODLADDERPASS:		SET_COMBOCLASS_VAR_BYTE(ladder_pass); break;				//C
		case COMBODLOCKBLOCK:		SET_COMBOCLASS_VAR_BYTE(lock_block_type); break;			//C
		case COMBODLOCKBLOCKCHANGE:	SET_COMBOCLASS_VAR_INT(lock_block_change); break;		//LONG
		case COMBODMAGICMIRROR:		SET_COMBOCLASS_VAR_BYTE(magic_mirror_type); break;			//C
		case COMBODMODHPAMOUNT:		SET_COMBOCLASS_VAR_DWORD(modify_hp_amount); break;		//SHORT
		case COMBODMODHPDELAY:		SET_COMBOCLASS_VAR_BYTE(modify_hp_delay); break;			//C
		case COMBODMODHPTYPE:		SET_COMBOCLASS_VAR_BYTE(modify_hp_type); break; 			//C
		case COMBODNMODMPAMOUNT:	SET_COMBOCLASS_VAR_DWORD(modify_mp_amount); break;		//SHORT
		case COMBODMODMPDELAY:		SET_COMBOCLASS_VAR_BYTE(modify_mp_delay); break;			//C
		case COMBODMODMPTYPE:		SET_COMBOCLASS_VAR_BYTE(modify_mp_type); break;				//C
		case COMBODNOPUSHBLOCK:		SET_COMBOCLASS_VAR_BYTE(no_push_blocks); break;			//C
		case COMBODOVERHEAD:		SET_COMBOCLASS_VAR_BYTE(overhead); break;				//C
		case COMBODPLACENPC:		SET_COMBOCLASS_VAR_BYTE(place_enemy); break;			//C
		case COMBODPUSHDIR:		SET_COMBOCLASS_VAR_BYTE(push_direction); break; 			//C
		case COMBODPUSHWAIT:		SET_COMBOCLASS_VAR_BYTE(push_wait); break;				//C
		case COMBODPUSHHEAVY:		SET_COMBOCLASS_VAR_BYTE(push_weight); break;				//C
		case COMBODPUSHED:		SET_COMBOCLASS_VAR_BYTE(pushed); break;				//C
		case COMBODRAFT:		SET_COMBOCLASS_VAR_BYTE(raft); break;					//C
		case COMBODRESETROOM:		SET_COMBOCLASS_VAR_BYTE(reset_room); break;			//C
		case COMBODSAVEPOINTTYPE:	SET_COMBOCLASS_VAR_BYTE(save_point_type); break;			//C
		case COMBODSCREENFREEZETYPE:	SET_COMBOCLASS_VAR_BYTE(screen_freeze_type); break;		//C
		case COMBODSECRETCOMBO:		SET_COMBOCLASS_VAR_BYTE(secret_combo); break;			//C
		case COMBODSINGULAR:		SET_COMBOCLASS_VAR_BYTE(singular); break;				//C
		case COMBODSLOWWALK:		SET_COMBOCLASS_VAR_BYTE(slow_movement); break;			//C
		case COMBODSTATUETYPE:		SET_COMBOCLASS_VAR_BYTE(statue_type); break;				//C
		case COMBODSTEPTYPE:		SET_COMBOCLASS_VAR_BYTE(step_type); break;				//C
		case COMBODSTEPCHANGEINTO:	SET_COMBOCLASS_VAR_INT(step_change_to); break;			//LONG
		case COMBODSTRIKEWEAPONS:	SET_COMBOCLASS_BYTE_INDEX(strike_weapons, 32); break;			//BYTE, 32 INDICES. 
		case COMBODSTRIKEREMNANTS:	SET_COMBOCLASS_VAR_INT(strike_remnants); break;		//LONG
		case COMBODSTRIKEREMNANTSTYPE:	SET_COMBOCLASS_VAR_BYTE(strike_remnants_type); break;	//C
		case COMBODSTRIKECHANGE:	SET_COMBOCLASS_VAR_INT(strike_change); break;			//LONG
		case COMBODSTRIKEITEM:		SET_COMBOCLASS_VAR_DWORD(strike_item); break;			//SHORT
		case COMBODTOUCHITEM:		SET_COMBOCLASS_VAR_DWORD(touch_item); break;			//SHORT
		case COMBODTOUCHSTAIRS:		SET_COMBOCLASS_VAR_BYTE(touch_stairs); break;			//C
		case COMBODTRIGGERTYPE:		SET_COMBOCLASS_VAR_BYTE(trigger_type); break;			//C
		case COMBODTRIGGERSENS:		SET_COMBOCLASS_VAR_BYTE(trigger_sensitive); break;	//C
		case COMBODWARPTYPE:		SET_COMBOCLASS_VAR_BYTE(warp_type); break;				//C
		case COMBODWARPSENS:		SET_COMBOCLASS_VAR_BYTE(warp_sensitive); break; 		//C
		case COMBODWARPDIRECT:		SET_COMBOCLASS_VAR_BYTE(warp_direct); break;			//C
		case COMBODWARPLOCATION:	SET_COMBOCLASS_VAR_BYTE(warp_location); break;			//C
		case COMBODWATER:		SET_COMBOCLASS_VAR_BYTE(water); break;					//C
		case COMBODWHISTLE:		SET_COMBOCLASS_VAR_BYTE(whistle); break;				//C
		case COMBODWINGAME:		SET_COMBOCLASS_VAR_BYTE(win_game); break; 				//C
		case COMBODBLOCKWPNLEVEL:	SET_COMBOCLASS_VAR_BYTE(block_weapon_lvl); break;		//C



		///----------------------------------------------------------------------------------------------------//
		case CMBTRIGWPNLEVEL:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->triggerlevel = vbound(value/10000, 0, 214748);
			}
			break;
		}
		case CMBTRIGREQITEM:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->triggeritem = vbound(value/10000, 0, MAXITEMS-1);
			}
			break;
		}
		case CMBTRIGTIMER:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trigtimer = vbound(value/10000, 0, 65535);
			}
			break;
		}
		case CMBTRIGSFX:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trigsfx = vbound(value/10000, 0, 255);
			}
			break;
		}
		case CMBTRIGCHANGECMB:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trigchange = value/10000;
			}
			break;
		}
		case CMBTRIGCSETCHANGE:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trigcschange = vbound(value/10000, -128, 127);
			}
			break;
		}
		case CMBTRIGPROX:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trigprox = vbound(value/10000, 0, 65535);
			}
			break;
		}
		case CMBTRIGLIGHTBEAM:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->triglbeam = vbound(value/10000,0,32);
			}
			break;
		}
		case CMBTRIGCTR:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trigctr = vbound(value/10000, sscMIN, MAX_COUNTERS-1);
			}
			break;
		}
		case CMBTRIGCTRAMNT:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trigctramnt = vbound(value/10000, -65535, 65535);
			}
			break;
		}
		case CMBTRIGCOOLDOWN:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trigcooldown = vbound(value/10000, 0, 255);
			}
			break;
		}
		case CMBTRIGCOPYCAT:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trigcopycat = vbound(value/10000, 0, 255);
			}
			break;
		}
		case CMBTRIGITEMPICKUP:
		{
			const int32_t allowed_pflags = ipHOLDUP | ipTIMER | ipSECRETS | ipCANGRAB;
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->spawnip = (value/10000)&allowed_pflags;
			}
			break;
		}
		case CMBTRIGEXSTATE:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->exstate = vbound(value/10000, -1, 31);
			}
			break;
		}
		case CMBTRIGEXDOORDIR:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->exdoor_dir = vbound(value/10000, -1, 3);
			}
			break;
		}
		case CMBTRIGEXDOORIND:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->exdoor_ind = vbound(value/10000, 0, 7);
			}
			break;
		}
		case CMBTRIGSPAWNENEMY:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->spawnenemy = vbound(value/10000, 0, 511);
			}
			break;
		}
		case CMBTRIGSPAWNITEM:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->spawnitem = vbound(value/10000, -255, 255);
			}
			break;
		}
		case CMBTRIGLSTATE:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trig_lstate = vbound(value/10000, 0, 31);
			}
			break;
		}
		case CMBTRIGGSTATE:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trig_gstate = vbound(value/10000, 0, 255);
			}
			break;
		}
		case CMBTRIGGTIMER:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trig_statetime = vbound(value/10000, 0, 214748);
			}
			break;
		}
		case CMBTRIGGENSCRIPT:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trig_genscr = vbound(value/10000, 0, 65535);
			}
			break;
		}
		case CMBTRIGGROUP:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trig_group = vbound(value/10000, 0, 255);
			}
			break;
		}
		case CMBTRIGGROUPVAL:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trig_group_val = vbound(value/10000, 0, 65535);
			}
			break;
		}
		case CMBTRIGLITEMS:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trig_levelitems = (value/10000) & liALL;
			}
			break;
		}
		case CMBTRIGDMAPLVL:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trigdmlevel = vbound(value/10000, -1, MAXLEVELS-1);
			}
			break;
		}
		case CMBTRIGTINTR:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trigtint[0] = scripting_write_pal_color(vbound(value/10000, -scripting_max_color_val, scripting_max_color_val));
			}
			break;
		}
		case CMBTRIGTINTG:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trigtint[1] = scripting_write_pal_color(vbound(value/10000, -scripting_max_color_val, scripting_max_color_val));
			}
			break;
		}
		case CMBTRIGTINTB:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trigtint[2] = scripting_write_pal_color(vbound(value/10000, -scripting_max_color_val, scripting_max_color_val));
			}
			break;
		}
		case CMBTRIGLVLPAL:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->triglvlpalette = vbound(value/10000, -1, 512);
			}
			break;
		}
		case CMBTRIGBOSSPAL:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trigbosspalette = vbound(value/10000, -1, 29);
			}
			break;
		}
		case CMBTRIGQUAKETIME:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trigquaketime = zc_max(value/10000, -1);
			}
			break;
		}
		case CMBTRIGWAVYTIME:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trigwavytime = zc_max(value/10000, -1);
			}
			break;
		}
		case CMBTRIGSWORDJINX:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trig_swjinxtime = zc_max(value/10000, -2);
			}
			break;
		}
		case CMBTRIGITEMJINX:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trig_itmjinxtime = zc_max(value/10000, -2);
			}
			break;
		}
		case CMBTRIGSHIELDJINX:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trig_shieldjinxtime = zc_max(value/10000, -2);
			}
			break;
		}
		case CMBTRIGSTUN:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trig_stuntime = zc_max(value/10000, -2);
			}
			break;
		}
		case CMBTRIGBUNNY:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trig_bunnytime = zc_max(value/10000, -2);
			}
			break;
		}
		case CMBTRIGPUSHTIME:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
			{
				trig->trig_pushtime = vbound(value/10000, 0, 255);
			}
			break;
		}
		case CMBTRIGGERPROMPTCID:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->prompt_cid = vbound(value/10000, 0, MAXCOMBOS-1);
			break;
		}
		case CMBTRIGGERPROMPTCS:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->prompt_cs = (value/10000)&15;
			break;
		}
		case CMBTRIGGERFAILPROMPTCID:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->fail_prompt_cid = vbound(value/10000, 0, MAXCOMBOS-1);
			break;
		}
		case CMBTRIGGERFAILPROMPTCS:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->fail_prompt_cs = (value/10000)&15;
			break;
		}
		case CMBTRIGGERPROMPTX:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->prompt_x = vbound(value/10000, -32768, 32767);
			break;
		}
		case CMBTRIGGERPROMPTY:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->prompt_y = vbound(value/10000, -32768, 32767);
			break;
		}
		case CMBTRIGGERTRIGSTR:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->trig_msgstr = vbound(value/10000, 0, msg_count-1);
			break;
		}
		case CMBTRIGGERFAILSTR:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->fail_msgstr = vbound(value/10000, 0, msg_count-1);
			break;
		}
		case CMBTRIGGERPLAYERBOUNCE:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->player_bounce = zslongToFix(value);
			break;
		}
		case CMBTRIGGERREQPLAYERZ:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->req_player_z = zslongToFix(value);
			break;
		}
		case CMBTRIGGERDESTHEROX:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->dest_player_x = zslongToFix(value);
			break;
		}
		case CMBTRIGGERDESTHEROY:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->dest_player_y = zslongToFix(value);
			break;
		}
		case CMBTRIGGERDESTHEROZ:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->dest_player_z = zslongToFix(value);
			break;
		}
		case CMBTRIGGERREQPLAYERJUMP:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->req_player_jump = zslongToFix(value);
			break;
		}
		case CMBTRIGGERREQPLAYERX:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->req_player_x = zslongToFix(value);
			break;
		}
		case CMBTRIGGERREQPLAYERY:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->req_player_y = zslongToFix(value);
			break;
		}
		case CMBTRIGGERFORCEPLAYERDIR:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->dest_player_dir = vbound(value/10000, 3, -1);
			break;
		}
		case CMBTRIGGERICECOMBO:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->force_ice_combo = vbound(value/10000, MAXCOMBOS-1, -1);
			break;
		}
		case CMBTRIGGERICEVX:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->force_ice_vx = zslongToFix(value);
			break;
		}
		case CMBTRIGGERICEVY:
		{
			if(auto* trig = get_combo_trigger(ri->combotrigref))
				trig->force_ice_vy = zslongToFix(value);
			break;
		}
		///----------------------------------------------------------------------------------------------------//
		//npcdata nd-> Variables
		
		#define	SET_NPCDATA_VAR_INT(member, str) \
		{ \
			if( auto nd = checkNPCData(ri->npcdataref) ) \
				nd->member = vbound((value / 10000),0,214747); \
		} \
		
		#define	SET_NPCDATA_VAR_DWORD(member, str) \
		{ \
			if( auto nd = checkNPCData(ri->npcdataref) ) \
				nd->member = vbound((value / 10000),0,32767); \
		} \

		#define	SET_NPCDATA_VAR_ENUM(member, str) \
		{ \
			if( auto nd = checkNPCData(ri->npcdataref) ) \
				nd->member = (decltype(guysbuf[ri->npcdataref].member))vbound((value / 10000),0,32767); \
		} \

		#define	SET_NPCDATA_VAR_BYTE(member, str) \
		{ \
			if( auto nd = checkNPCData(ri->npcdataref) ) \
				nd->member = vbound((value / 10000),0,255); \
		} \
		
		#define SET_NPCDATA_FLAG(member, str) \
		{ \
			int32_t flag =  (value/10000);  \
			if( auto nd = checkNPCData(ri->npcdataref) ) \
			{ \
				if ( flag ) \
				{ \
					nd->member|=flag; \
				} \
				else nd->member|= ~flag; \
			} \
		} \
		
		case NPCDATATILE: SET_NPCDATA_VAR_BYTE(tile, "Tile"); break;
		case NPCDATAWIDTH: SET_NPCDATA_VAR_BYTE(width, "Width"); break;
		case NPCDATAHEIGHT: SET_NPCDATA_VAR_BYTE(height, "Height"); break;
		case NPCDATAFLAGS1: SET_NPCDATA_VAR_ENUM(flags, "Flags (deprecated)"); break;
		case NPCDATAFLAGS2: SET_NPCDATA_VAR_ENUM(flags, "Flags2 (deprecated)"); break;
		case NPCDATASTILE: SET_NPCDATA_VAR_BYTE(s_tile, "STile"); break;
		case NPCDATASWIDTH: SET_NPCDATA_VAR_BYTE(s_width, "SWidth"); break;
		case NPCDATASHEIGHT: SET_NPCDATA_VAR_BYTE(s_height, "SHeight"); break;
		case NPCDATAETILE: SET_NPCDATA_VAR_INT(e_tile, "ExTile"); break;
		case NPCDATAEWIDTH: SET_NPCDATA_VAR_BYTE(e_width, "ExWidth"); break;
		case NPCDATASCRIPT: SET_NPCDATA_VAR_BYTE(script, "Script"); break;
		case NPCDATAEHEIGHT: SET_NPCDATA_VAR_BYTE(e_height, "ExHeight"); break;
		case NPCDATAHP: SET_NPCDATA_VAR_DWORD(hp, "HP"); break;
		case NPCDATAFAMILY: SET_NPCDATA_VAR_DWORD(family, "Family"); break;
		case NPCDATACSET: SET_NPCDATA_VAR_DWORD(cset, "CSet"); break;
		case NPCDATAANIM: SET_NPCDATA_VAR_DWORD(anim, "Anim"); break;
		case NPCDATAEANIM: SET_NPCDATA_VAR_DWORD(e_anim, "ExAnim"); break;
		case NPCDATAFRAMERATE: SET_NPCDATA_VAR_DWORD(frate, "Framerate"); break;
		case NPCDATAEFRAMERATE: SET_NPCDATA_VAR_DWORD(e_frate, "ExFramerate"); break;
		case NPCDATATOUCHDAMAGE: SET_NPCDATA_VAR_DWORD(dp, "TouchDamage"); break;
		case NPCDATAWEAPONDAMAGE: SET_NPCDATA_VAR_DWORD(wdp, "WeaponDamage"); break;
		case NPCDATAWEAPON: SET_NPCDATA_VAR_DWORD(weapon, "Weapon"); break;
		case NPCDATARANDOM: SET_NPCDATA_VAR_DWORD(rate, "Random"); break;
		case NPCDATAHALT: SET_NPCDATA_VAR_DWORD(hrate, "Haltrate"); break;
		case NPCDATASTEP: SET_NPCDATA_VAR_DWORD(step, "Step"); break;
		case NPCDATAHOMING: SET_NPCDATA_VAR_DWORD(homing, "Homing"); break;
		case NPCDATAHUNGER: SET_NPCDATA_VAR_DWORD(grumble, "Hunger"); break;
		case NPCDATADROPSET: SET_NPCDATA_VAR_DWORD(item_set, "Dropset"); break;
		case NPCDATABGSFX: SET_NPCDATA_VAR_DWORD(bgsfx, "BGSFX"); break;
		case NPCDATADEATHSFX: SET_NPCDATA_VAR_BYTE(deadsfx, "DeathSFX"); break;
		case NPCDATAHITSFX: SET_NPCDATA_VAR_BYTE(hitsfx, "HitSFX"); break;
		case NPCDATAXOFS: SET_NPCDATA_VAR_INT(xofs, "DrawXOffset"); break;
		case NPCDATAYOFS: SET_NPCDATA_VAR_INT(yofs, "DrawYOffset"); break;
		case NPCDATAZOFS: SET_NPCDATA_VAR_INT(zofs, "DrawZOffset"); break;
		case NPCDATAHXOFS: SET_NPCDATA_VAR_INT(hxofs, "HitXOffset"); break;
		case NPCDATAHYOFS: SET_NPCDATA_VAR_INT(hyofs, "HitYOffset"); break;
		case NPCDATAHITWIDTH: SET_NPCDATA_VAR_INT(hxsz, "HitWidth"); break;
		case NPCDATAHITHEIGHT: SET_NPCDATA_VAR_INT(hysz, "HitHeight"); break;
		case NPCDATAHITZ: SET_NPCDATA_VAR_INT(hzsz, "HitZHeight"); break;
		case NPCDATATILEWIDTH: SET_NPCDATA_VAR_INT(txsz, "TileWidth"); break;
		case NPCDATATILEHEIGHT: SET_NPCDATA_VAR_INT(tysz, "TileHeight"); break;
		case NPCDATAWPNSPRITE: SET_NPCDATA_VAR_INT(wpnsprite, "WeaponSprite"); break;
		case NPCDATAWEAPONSCRIPT: 
		{
			if( (unsigned) ri->npcdataref > (MAXNPCS-1) )
				Z_scripterrlog("Invalid NPC ID passed to npcdata->WeaponScript: %d\n", (ri->npcdataref*10000));
			else
				guysbuf[ri->npcdataref].weap_data.script = vbound((value / 10000),0,214747);
			break;
		}
		case NPCDATASIZEFLAG: SET_NPCDATA_VAR_INT(SIZEflags, "SizeFlags"); break;

		case NPCDATAFROZENTILE: SET_NPCDATA_VAR_INT(frozentile, "FrozenTile"); break;
		case NPCDATAFROZENCSET: SET_NPCDATA_VAR_INT(frozencset, "FrozenCSet"); break;
		case NPCDATAFIRESFX: SET_NPCDATA_VAR_BYTE(firesfx, "WeaponSFX"); break;

		case NPCDSHADOWSPR:
		{
			if(ri->npcdataref > (MAXNPCS-1) ) 
			{ 
				Z_scripterrlog("Invalid NPC ID passed to npcdata->ShadowSprite: %d\n", (ri->npcdataref*10000));
			} 
			else 
			{
				guysbuf[ri->npcdataref].spr_shadow = vbound(value/10000, 0, 255);
			} 
			break;
		}
		case NPCDSPAWNSPR:
		{
			if(ri->npcdataref > (MAXNPCS-1) ) 
			{ 
				Z_scripterrlog("Invalid NPC ID passed to npcdata->SpawnSprite: %d\n", (ri->npcdataref*10000));
			} 
			else 
			{
				guysbuf[ri->npcdataref].spr_spawn = vbound(value/10000, 0, 255);
			} 
			break;
		}
		case NPCDDEATHSPR:
		{
			if(ri->npcdataref > (MAXNPCS-1) ) 
			{ 
				Z_scripterrlog("Invalid NPC ID passed to npcdata->DeathSprite: %d\n", (ri->npcdataref*10000));
			} 
			else 
			{
				guysbuf[ri->npcdataref].spr_death = vbound(value/10000, 0, 255);
			} 
			break;
		}


	///----------------------------------------------------------------------------------------------------//
	//Dropset Variables

		case DROPSETNULLCHANCE:
		{
			if(ri->dropsetref > MAXITEMDROPSETS)
			{
				Z_scripterrlog("Invalid dropset pointer %d\n", ri->dropsetref);
				break;
			}
			item_drop_sets[ri->dropsetref].chance[0] = vbound((value / 10000),0,32767);
			break;
		}

	///----------------------------------------------------------------------------------------------------//
	//Audio Variables

		case AUDIOPAN:
		{
			if ( !(FFCore.coreflags&FFCORE_SCRIPTED_PANSTYLE) ) 
			{
				FFCore.usr_panstyle = FFScript::do_getSFX_pan();
				FFCore.SetFFEngineFlag(FFCORE_SCRIPTED_PANSTYLE,true);
			}
			FFScript::do_setSFX_pan(value/10000);
			break;
		}

	///----------------------------------------------------------------------------------------------------//
	//Graphics->

		case NUMDRAWS:
			break;
		
		case MAXDRAWS: break;

	///----------------------------------------------------------------------------------------------------//
	//Misc./Internal
		case SP:
			ri->sp = value / 10000;
			break;
		
		case SP2:
			ri->sp = value;
			break;
			
		case PC:
			ri->pc = value;
			break;
			
		case SWITCHKEY:
			ri->switchkey = value;
			break;
			
		case SCRIPTRAM:
		case GLOBALRAM:
			ArrayH::setElement(ri->d[rINDEX], ri->d[rINDEX2] / 10000, value);
			break;
			
		case SCRIPTRAMD:
		case GLOBALRAMD:
			ArrayH::setElement(ri->d[rINDEX], 0, value);
			break;
			
		case REFFFC:
			ri->ffcref = ZScriptVersion::ffcRefIsSpriteId() ? value : value / 10000;
			break;
			
		case REFITEM:
			ri->itemref = value;
			break;
			
		case REFITEMCLASS:
			ri->idata = value;
			break;
			
		case REFLWPN:
			ri->lwpn = value;
			break;
			
		case REFEWPN:
			ri->ewpn = value;
			break;
			
		case REFNPC:
			ri->guyref = value;
			break;

		case REFSPRITE:
			ri->spriteref = value;
			break;

		case REFMAPDATA: ri->mapsref = value; break;
		case REFSCREENDATA: ri->screenref = value; break;
		case REFCOMBODATA: ri->combosref = value; break;
		case REFCOMBOTRIGGER: ri->combotrigref = value; break;
		case REFSPRITEDATA: ri->spritedataref = value; break;
		case REFBITMAP: ri->bitmapref = value; break;
		case REFNPCCLASS: ri->npcdataref = value; break;
		
		case REFDMAPDATA: ri->dmapsref = value; break;
		case REFSHOPDATA: ri->shopsref = value; break;
		case REFMSGDATA: ri->zmsgref = value; break;
		
		
		case REFDROPS:  ri->dropsetref = value; break;
		case REFBOTTLETYPE:  ri->bottletyperef = value; break;
		case REFBOTTLESHOP:  ri->bottleshopref = value; break;
		case REFGENERICDATA:  ri->genericdataref = value; break;
		case REFFILE: ri->fileref = value; break;
		case REFDIRECTORY: ri->directoryref = value; break;
		case REFSTACK: ri->stackref = value; break;
		case REFSUBSCREEN: ri->subdataref = value; break;
		case REFSUBSCREENPAGE: ri->subpageref = value; break;
		case REFSUBSCREENWIDG: ri->subwidgref = value; break;
		case REFRNG: ri->rngref = value; break;
		case REFWEBSOCKET: ri->websocketref = value; break;
		case CLASS_THISKEY: ri->thiskey = value; break;
		case CLASS_THISKEY2: ri->thiskey2 = value; break;
		case REFPALDATA: ri->paldataref = value; break;
	
		//-------------------------------------------------------------------------------------------------

		case GENDATARUNNING:
		{
			if(user_genscript* scr = checkGenericScr(ri->genericdataref))
			{
				if(value)
					scr->launch();
				else scr->quit();
			}
			break;
		}
		case GENDATASIZE:
		{
			if(user_genscript* scr = checkGenericScr(ri->genericdataref))
			{
				scr->dataResize(value/10000);
			}
			break;
		}
		
		//----------------------------------------------------------------------------------------------------//
		
		case PORTALX:
		{
			if(portal* p = checkPortal(ri->portalref))
				p->x = zslongToFix(value);
			break;
		}
		case PORTALY:
		{
			if(portal* p = checkPortal(ri->portalref))
				p->y = zslongToFix(value);
			break;
		}
		case PORTALDMAP:
		{
			if(portal* p = checkPortal(ri->portalref))
				p->destdmap = vbound(value/10000,-1,MAXDMAPS-1);
			break;
		}
		case PORTALSCREEN:
		{
			if(portal* p = checkPortal(ri->portalref))
				p->destscr = vbound(value/10000,0,255);
			break;
		}
		case PORTALACLK:
		{
			if(portal* p = checkPortal(ri->portalref))
				p->aclk = vbound(value/10000, 0, 9999);
			break;
		}
		case PORTALAFRM:
		{
			if(portal* p = checkPortal(ri->portalref))
				p->aframe = vbound(value/10000, 0, 9999);
			break;
		}
		case PORTALOTILE:
		{
			if(portal* p = checkPortal(ri->portalref))
				p->o_tile = vbound(value/10000, 0, NEWMAXTILES-1);
			break;
		}
		case PORTALASPD:
		{
			if(portal* p = checkPortal(ri->portalref))
				p->aspd = vbound(value/10000, 0, 9999);
			break;
		}
		case PORTALFRAMES:
		{
			if(portal* p = checkPortal(ri->portalref))
				p->frames = vbound(value/10000, 0, 9999);
			break;
		}
		case PORTALSAVED:
		{
			if(ri->portalref < 0 || value < 0) break;
			if(portal* p = checkPortal(ri->portalref))
			{
				if(!value)
					p->saved_data = 0;
				else if(savedportal* sp = checkSavedPortal(value))
					p->saved_data = sp->getUID();
			}
			break;
		}
		case PORTALCLOSEDIS:
		{
			if(portal* p = checkPortal(ri->portalref))
				p->prox_active = value==0; //Inverted
			break;
		}
		case REFPORTAL:
		{
			ri->portalref = value;
			break;
		}
		case REFSAVPORTAL:
		{
			ri->saveportalref = value;
			break;
		}
		case PORTALWARPSFX:
		{
			if(portal* p = checkPortal(ri->portalref))
				p->wsfx = vbound(value/10000,0,255);
			break;
		}
		case PORTALWARPVFX:
		{
			if(portal* p = checkPortal(ri->portalref))
				p->weffect = vbound(value/10000,0,255);
			break;
		}
		case SAVEDPORTALX:
		{
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				p->x = value;
			break;
		}
		case SAVEDPORTALY:
		{
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				p->y = value;
			break;
		}
		case SAVEDPORTALSRCDMAP:
		{
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				p->srcdmap = vbound(value/10000, -1, MAXDMAPS-1);
			break;
		}
		case SAVEDPORTALDESTDMAP:
		{
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				p->destdmap = vbound(value/10000, -1, MAXDMAPS-1);
			break;
		}
		case SAVEDPORTALSRCSCREEN:
		{
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				p->srcscr = vbound(value/10000,0,255);
			break;
		}
		case SAVEDPORTALDSTSCREEN:
		{
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				p->destscr = vbound(value/10000,0,255);
			break;
		}
		case SAVEDPORTALWARPSFX:
		{
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				p->sfx = vbound(value/10000,0,255);
			break;
		}
		case SAVEDPORTALWARPVFX:
		{
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				p->warpfx = vbound(value/10000,0,255);
			break;
		}
		case SAVEDPORTALSPRITE:
		{
			if(savedportal* p = checkSavedPortal(ri->saveportalref))
				p->spr = vbound(value/10000,0,255);
			break;
		}
		case SAVEDPORTALPORTAL:
		{
			if(ri->saveportalref < 0 || value < 0) break;
			if(savedportal* sp = checkSavedPortal(ri->saveportalref))
			{
				int32_t id = getPortalFromSaved(sp);
				if(id == value) break; //no change
				portal* p = checkPortal(value);
				if(p)
				{
					p->saved_data = sp->getUID();
					if(id > 0)
					{
						portal* p = checkPortal(id);
						p->saved_data = 0;
					}
				}
			}
			break;
		}
		
		case GAMENUMASUB:
		{
			if(value >= 0)
			{
				size_t sz = vbound(value/10000, 0, 256);
				while(subscreens_active.size() < sz)
				{
					auto& sub = subscreens_active.emplace_back();
					sub.sub_type = sstACTIVE;
				}
				while(subscreens_active.size() > sz)
					subscreens_active.pop_back();
			}
			break;
		}
		case GAMENUMPSUB:
		{
			if(value >= 0)
			{
				size_t sz = vbound(value/10000, 0, 256);
				while(subscreens_passive.size() < sz)
				{
					auto& sub = subscreens_passive.emplace_back();
					sub.sub_type = sstPASSIVE;
				}
				while(subscreens_passive.size() > sz)
					subscreens_passive.pop_back();
			}
			break;
		}
		case GAMENUMOSUB:
		{
			if(value >= 0)
			{
				size_t sz = vbound(value/10000, 0, 256);
				while(subscreens_overlay.size() < sz)
				{
					auto& sub = subscreens_overlay.emplace_back();
					sub.sub_type = sstOVERLAY;
				}
				while(subscreens_overlay.size() > sz)
					subscreens_overlay.pop_back();
			}
			break;
		}
		///----------------------------------------------------------------------------------------------------//
		
		case SUBDATACURPG:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref))
				if(sub->sub_type == sstACTIVE)
					sub->curpage = vbound(value/10000,0,sub->pages.size()-1);
			break;
		}
		case SUBDATANUMPG:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref))
				if(sub->sub_type == sstACTIVE && value >= 10000)
				{
					size_t sz = value/10000;
					while(sub->pages.size() < sz)
						if(!sub->add_page(MAX_SUBSCR_PAGES))
							break;
					while(sub->pages.size() > sz)
						sub->delete_page(sub->pages.size()-1);
				}
			break;
		}
		case SUBDATATYPE: break; //READONLY
		///---- ACTIVE SUBSCREENS ONLY
		case SUBDATACURSORPOS:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
			{
				SubscrPage& pg = sub->cur_page();
				//Should this be sanity checked? Or should nulling out
				// the cursor by setting it invalid be allowed? -Em
				pg.cursor_pos = vbound(value/10000,0,255);
			}
			break;
		}
		case SUBDATASCRIPT:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
				sub->script = vbound(value/10000,0,NUMSCRIPTSSUBSCREEN-1);
			break;
		}

		case SUBDATATRANSLEFTTY:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
			{
				auto& trans = sub->trans_left;
				trans.type = vbound(value/10000,0,sstrMAX-1);
			}
			break;
		}
		case SUBDATATRANSLEFTSFX:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
			{
				auto& trans = sub->trans_left;
				trans.tr_sfx = vbound(value/10000,0,255);
			}
			break;
		}
		case SUBDATATRANSRIGHTTY:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
			{
				auto& trans = sub->trans_right;
				trans.type = vbound(value/10000,0,sstrMAX-1);
			}
			break;
		}
		case SUBDATATRANSRIGHTSFX:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
			{
				auto& trans = sub->trans_right;
				trans.tr_sfx = vbound(value/10000,0,255);
			}
			break;
		}
		case SUBDATASELECTORDSTX:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
				sub->selector_setting.x = vbound(value/10000,-32768,32767);
			break;
		}
		case SUBDATASELECTORDSTY:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
				sub->selector_setting.y = vbound(value/10000,-32768,32767);
			break;
		}
		case SUBDATASELECTORDSTW:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
				sub->selector_setting.w = vbound(value/10000,-32768,32767);
			break;
		}
		case SUBDATASELECTORDSTH:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
				sub->selector_setting.h = vbound(value/10000,-32768,32767);
			break;
		}
		///---- CURRENTLY OPEN ACTIVE SUBSCREEN ONLY
		case SUBDATATRANSCLK:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
			{
				if(sub != new_subscreen_active)
					Z_scripterrlog("'subscreendata->TransClock' is only"
						" valid for the current active subscreen!\n");
				else if(subscreen_open)
				{
					int val = value/10000;
					if(val < 0)
						subscrpg_clear_animation();
					else if(!subscr_pg_animating)
					{
						SubscrTransition tr = subscr_pg_transition;
						tr.tr_sfx = 0;
						subscrpg_animate(subscr_pg_from,subscr_pg_to,tr,*new_subscreen_active);
						subscr_pg_clk = val;
					}
					else subscr_pg_clk = val;
				}
			}
			break;
		}
		case SUBDATATRANSTY:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
			{
				auto& trans = subscr_pg_transition;
				if(sub != new_subscreen_active)
					Z_scripterrlog("'subscreendata->TransType' is only"
						" valid for the current active subscreen!\n");
				else if(subscreen_open)
					trans.type = vbound(value/10000,0,sstrMAX-1);
			}
			break;
		}
		case SUBDATATRANSFROMPG:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
			{
				if(sub != new_subscreen_active)
					Z_scripterrlog("'subscreendata->TransFromPage' is only"
						" valid for the current active subscreen!\n");
				else if(subscreen_open)
					subscr_pg_from = vbound(value/10000,0,sub->pages.size()-1);
			}
			break;
		}
		case SUBDATATRANSTOPG:
		{
			if(ZCSubscreen* sub = checkSubData(ri->subdataref, sstACTIVE))
			{
				if(sub != new_subscreen_active)
					Z_scripterrlog("'subscreendata->TransToPage' is only"
						" valid for the current active subscreen!\n");
				else if(subscreen_open)
					subscr_pg_to = vbound(value/10000,0,sub->pages.size()-1);
			}
			break;
		}
		
		///----------------------------------------------------------------------------------------------------//
		case SUBPGINDEX: break; //READ-ONLY
		case SUBPGNUMWIDG: break; //READ-ONLY
		case SUBPGSUBDATA: break; //READ-ONLY
		case SUBPGCURSORPOS:
		{
			if(SubscrPage* pg = checkSubPage(ri->subpageref))
				pg->cursor_pos = vbound(value/10000,0,255);
			break;
		}
		///----------------------------------------------------------------------------------------------------//
		///---- ANY WIDGET TYPE
		case SUBWIDGTYPE: break; //READ-ONLY
		case SUBWIDGINDEX: break; //READ-ONLY
		case SUBWIDGPAGE: break; //READ-ONLY
		case SUBWIDGDISPITM: break; //READ-ONLY
		case SUBWIDGEQPITM: break; //READ-ONLY
		case SUBWIDGPOS:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				widg->pos = vbound(value/10000,0,255);
			break;
		}
		case SUBWIDGX:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				widg->x = vbound(value/10000,-32768,32767);
			break;
		}
		case SUBWIDGY:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				widg->y = vbound(value/10000,-32768,32767);
			break;
		}
		case SUBWIDGW:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				widg->w = vbound(value/10000,0,65535);
			break;
		}
		case SUBWIDGH:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				widg->h = vbound(value/10000,0,65535);
			break;
		}
		case SUBWIDGREQCOUNTER:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				widg->req_counter = vbound(value/10000,sscMIN,MAX_COUNTERS);
			break;
		}
		case SUBWIDGREQCOUNTERCOND:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				widg->req_counter_cond_type = vbound(value/10000,CONDTY_NONE,CONDTY_MAX-1);
			break;
		}
		case SUBWIDGREQCOUNTERVAL:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				widg->req_counter_val = vbound(value/10000,0,65535);
			break;
		}
		case SUBWIDGREQLITEMS:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				widg->req_litems = vbound(value/10000,0,liALL);
			break;
		}
		case SUBWIDGREQLITEMLEVEL:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				widg->req_litem_level = vbound(value/10000,-1,MAXLEVELS);
			break;
		}
		case SUBWIDGREQSCRIPTDISABLED:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				widg->is_disabled = value != 0;
			break;
		}
		///---- ACTIVE SUBSCREENS ONLY
		case SUBWIDGSELECTORDSTX:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref, sstACTIVE))
				widg->selector_override.x = vbound(value/10000,-32768,32767);
			break;
		}
		case SUBWIDGSELECTORDSTY:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref, sstACTIVE))
				widg->selector_override.y = vbound(value/10000,-32768,32767);
			break;
		}
		case SUBWIDGSELECTORDSTW:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref, sstACTIVE))
				widg->selector_override.w = vbound(value/10000,-32768,32767);
			break;
		}
		case SUBWIDGSELECTORDSTH:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref, sstACTIVE))
				widg->selector_override.h = vbound(value/10000,-32768,32767);
			break;
		}
				
		case SUBWIDGPRESSSCRIPT:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref, sstACTIVE))
				widg->generic_script = vbound(value/10000,0,NUMSCRIPTSGENERIC-1);
			break;
		}
		case SUBWIDGPGMODE:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref, sstACTIVE))
				widg->pg_mode = vbound(value/10000,0,PGGOTO_MAX-1);
			break;
		}
		case SUBWIDGPGTARG:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref, sstACTIVE))
				widg->pg_targ = vbound(value/10000,0,MAX_SUBSCR_PAGES-1);
			break;
		}
		
		case SUBWIDGTRANSPGTY:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref, sstACTIVE))
			{
				auto& trans = widg->pg_trans;
				trans.type = vbound(value/10000,0,sstrMAX-1);
			}
			break;
		}
		case SUBWIDGTRANSPGSFX:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref, sstACTIVE))
			{
				auto& trans = widg->pg_trans;
				trans.tr_sfx = vbound(value/10000,0,255);
			}
			break;
		}
		case SUBWIDGTY_FONT:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,font_max-1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						((SW_Text*)widg)->fontid = val;
						break;
					case widgTEXTBOX:
						((SW_TextBox*)widg)->fontid = val;
						break;
					case widgSELECTEDTEXT:
						((SW_SelectedText*)widg)->fontid = val;
						break;
					case widgTIME:
						((SW_Time*)widg)->fontid = val;
						break;
					case widgCOUNTER:
						((SW_Counter*)widg)->fontid = val;
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->fontid = val;
						break;
					case widgOLDCTR:
						((SW_Counters*)widg)->fontid = val;
						break;
					case widgMMAPTITLE:
						((SW_MMapTitle*)widg)->fontid = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_ALIGN:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,sstaMAX-1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						((SW_Text*)widg)->align = val;
						break;
					case widgTEXTBOX:
						((SW_TextBox*)widg)->align = val;
						break;
					case widgSELECTEDTEXT:
						((SW_SelectedText*)widg)->align = val;
						break;
					case widgTIME:
						((SW_Time*)widg)->align = val;
						break;
					case widgCOUNTER:
						((SW_Counter*)widg)->align = val;
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->align = val;
						break;
					case widgMMAPTITLE:
						((SW_MMapTitle*)widg)->align = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_SHADOWTY:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,sstsMAX-1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						((SW_Text*)widg)->shadtype = val;
						break;
					case widgTEXTBOX:
						((SW_TextBox*)widg)->shadtype = val;
						break;
					case widgSELECTEDTEXT:
						((SW_SelectedText*)widg)->shadtype = val;
						break;
					case widgTIME:
						((SW_Time*)widg)->shadtype = val;
						break;
					case widgCOUNTER:
						((SW_Counter*)widg)->shadtype = val;
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->shadtype = val;
						break;
					case widgOLDCTR:
						((SW_Counters*)widg)->shadtype = val;
						break;
					case widgMMAPTITLE:
						((SW_MMapTitle*)widg)->shadtype = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_TXT:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						((SW_Text*)widg)->c_text.set_int_color(val);
						break;
					case widgTEXTBOX:
						((SW_TextBox*)widg)->c_text.set_int_color(val);
						break;
					case widgSELECTEDTEXT:
						((SW_SelectedText*)widg)->c_text.set_int_color(val);
						break;
					case widgTIME:
						((SW_Time*)widg)->c_text.set_int_color(val);
						break;
					case widgCOUNTER:
						((SW_Counter*)widg)->c_text.set_int_color(val);
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->c_text.set_int_color(val);
						break;
					case widgOLDCTR:
						((SW_Counters*)widg)->c_text.set_int_color(val);
						break;
					case widgMMAPTITLE:
						((SW_MMapTitle*)widg)->c_text.set_int_color(val);
						break;
					case widgMCGUFF_FRAME:
						((SW_TriFrame*)widg)->c_number.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_SHD:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						((SW_Text*)widg)->c_shadow.set_int_color(val);
						break;
					case widgTEXTBOX:
						((SW_TextBox*)widg)->c_shadow.set_int_color(val);
						break;
					case widgSELECTEDTEXT:
						((SW_SelectedText*)widg)->c_shadow.set_int_color(val);
						break;
					case widgTIME:
						((SW_Time*)widg)->c_shadow.set_int_color(val);
						break;
					case widgCOUNTER:
						((SW_Counter*)widg)->c_shadow.set_int_color(val);
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->c_shadow.set_int_color(val);
						break;
					case widgOLDCTR:
						((SW_Counters*)widg)->c_shadow.set_int_color(val);
						break;
					case widgMMAPTITLE:
						((SW_MMapTitle*)widg)->c_shadow.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_BG:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						((SW_Text*)widg)->c_bg.set_int_color(val);
						break;
					case widgTEXTBOX:
						((SW_TextBox*)widg)->c_bg.set_int_color(val);;
						break;
					case widgSELECTEDTEXT:
						((SW_SelectedText*)widg)->c_bg.set_int_color(val);;
						break;
					case widgTIME:
						((SW_Time*)widg)->c_bg.set_int_color(val);
						break;
					case widgCOUNTER:
						((SW_Counter*)widg)->c_bg.set_int_color(val);
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->c_bg.set_int_color(val);
						break;
					case widgOLDCTR:
						((SW_Counters*)widg)->c_bg.set_int_color(val);
						break;
					case widgMMAPTITLE:
						((SW_MMapTitle*)widg)->c_bg.set_int_color(val);
						break;
					case widgBGCOLOR:
						((SW_Clear*)widg)->c_bg.set_int_color(val);
						break;
					case widgCOUNTERPERCBAR:
						((SW_CounterPercentBar*)widg)->c_bg.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		
		case SUBWIDGTY_COLOR_TXT2:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						((SW_Counter*)widg)->c_text2.set_int_color(val);
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->c_text2.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_SHD2:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						((SW_Counter*)widg)->c_shadow2.set_int_color(val);
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->c_shadow2.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_BG2:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						((SW_Counter*)widg)->c_bg2.set_int_color(val);
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->c_bg2.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		
		case SUBWIDGTY_COLOR_OLINE:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLINE:
						((SW_Line*)widg)->c_line.set_int_color(val);
						break;
					case widgRECT:
						((SW_Rect*)widg)->c_outline.set_int_color(val);
						break;
					case widgMCGUFF_FRAME:
						((SW_TriFrame*)widg)->c_outline.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		
		case SUBWIDGTY_COLOR_FILL:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgRECT:
						((SW_Rect*)widg)->c_fill.set_int_color(val);
						break;
					case widgCOUNTERPERCBAR:
						((SW_CounterPercentBar*)widg)->c_fill.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_BUTTON:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,3);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgBTNITM:
						((SW_ButtonItem*)widg)->btn = val;
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->btn = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_MINDIG:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,5);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						((SW_Counter*)widg)->mindigits = val;
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->mindigits = val;
						break;
					case widgOLDCTR:
						((SW_Counters*)widg)->digits = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_MAXDIG:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,5);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						((SW_Counter*)widg)->maxdigits = val;
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->maxdigits = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_INFITM:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,-1,MAXITEMS-1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						((SW_Counter*)widg)->infitm = val;
						break;
					case widgOLDCTR:
						((SW_Counters*)widg)->infitm = val;
						break;
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						((SW_GaugePiece*)widg)->inf_item = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_INFCHAR:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				char val = vbound(value/10000,0,255);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						((SW_Counter*)widg)->infchar = val;
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->infchar = val;
						break;
					case widgOLDCTR:
						((SW_Counters*)widg)->infchar = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COSTIND:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->costind = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_PLAYER:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						((SW_MMap*)widg)->c_plr.set_int_color(val);
						break;
					case widgLMAP:
						((SW_LMap*)widg)->c_plr.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_CMPBLNK:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						((SW_MMap*)widg)->c_cmp_blink.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_CMPOFF:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						((SW_MMap*)widg)->c_cmp_off.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_ROOM:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLMAP:
						((SW_LMap*)widg)->c_room.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_ITEMCLASS:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,itype_maxusable-1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgITEMSLOT:
						((SW_ItemSlot*)widg)->iclass = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_ITEMID:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,-1,MAXITEMS-1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgITEMSLOT:
						((SW_ItemSlot*)widg)->iid = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_FRAMETILE:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,NEWMAXTILES-1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF_FRAME:
						((SW_TriFrame*)widg)->frame_tile = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_FRAMECSET:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,15);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF_FRAME:
						((SW_TriFrame*)widg)->frame_cset = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_PIECETILE:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,NEWMAXTILES-1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF_FRAME:
						((SW_TriFrame*)widg)->piece_tile = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_PIECECSET:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,15);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF_FRAME:
						((SW_TriFrame*)widg)->piece_cset = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_FLIP:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,15);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF:
						((SW_McGuffin*)widg)->flip = val;
						break;
					case widgTILEBLOCK:
						((SW_TileBlock*)widg)->flip = val;
						break;
					case widgMINITILE:
						((SW_MiniTile*)widg)->flip = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_NUMBER:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,255);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF:
						((SW_McGuffin*)widg)->number = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_FRAMES:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,1,65535);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						((SW_GaugePiece*)widg)->frames = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_SPEED:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,1,65535);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						((SW_GaugePiece*)widg)->speed = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_DELAY:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,65535);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						((SW_GaugePiece*)widg)->delay = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_CONTAINER:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,65535);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						((SW_GaugePiece*)widg)->container = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_GAUGE_WID:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,1,32)-1;
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						((SW_GaugePiece*)widg)->gauge_wid = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_GAUGE_HEI:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,1,32)-1;
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						((SW_GaugePiece*)widg)->gauge_hei = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_UNITS:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,1,256);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						((SW_GaugePiece*)widg)->unit_per_frame = val-1;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_HSPACE:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,-128,127);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						((SW_GaugePiece*)widg)->hspace = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_VSPACE:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,-128,127);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						((SW_GaugePiece*)widg)->vspace = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_GRIDX:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,-32768,32767);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						((SW_GaugePiece*)widg)->grid_xoff = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_GRIDY:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,-32768,32767);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						((SW_GaugePiece*)widg)->grid_yoff = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_ANIMVAL:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,65535);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE:
					case widgMGAUGE:
					case widgMISCGAUGE:
						((SW_GaugePiece*)widg)->anim_val = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_SHOWDRAIN:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,-1,32767);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMGAUGE:
						((SW_MagicGaugePiece*)widg)->showdrain = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_PERCONTAINER:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,1,65535);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMISCGAUGE:
						((SW_MiscGaugePiece*)widg)->per_container = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_TABSIZE:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,255);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXTBOX:
						((SW_TextBox*)widg)->tabsize = val;
						break;
					case widgSELECTEDTEXT:
						((SW_SelectedText*)widg)->tabsize = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_LITEMS:
		{
			if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
			{
				auto val = vbound(value/10000,0,255);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						((SW_MMap*)widg)->compass_litems = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		
		default:
		{
			if (zasm_array_supports(arg))
			{
				int ref_arg = get_register_ref_dependency(arg).value_or(0);
				int ref = ref_arg ? get_ref(ref_arg) : 0;
				zasm_array_set(arg, ref, ri->d[rINDEX] / 10000, value);
			}
			else
			{
				scripting_engine_set_register(arg, value);
			}
		}
	}

	current_zasm_register = 0;
} //end set_register

static std::map<std::string, int> name_to_slot_index_ffcmap;
static std::map<std::string, int> name_to_slot_index_globalmap;
static std::map<std::string, int> name_to_slot_index_genericmap;
static std::map<std::string, int> name_to_slot_index_itemmap;
static std::map<std::string, int> name_to_slot_index_npcmap;
static std::map<std::string, int> name_to_slot_index_ewpnmap;
static std::map<std::string, int> name_to_slot_index_lwpnmap;
static std::map<std::string, int> name_to_slot_index_playermap;
static std::map<std::string, int> name_to_slot_index_dmapmap;
static std::map<std::string, int> name_to_slot_index_screenmap;
static std::map<std::string, int> name_to_slot_index_itemspritemap;
static std::map<std::string, int> name_to_slot_index_comboscriptmap;
static std::map<std::string, int> name_to_slot_index_subscreenmap;

void script_init_name_to_slot_index_maps()
{
	int i;
	#define DECL_INIT_MAP(name) \
	{\
		name_to_slot_index_##name.clear();\
		i = 0;\
		for (auto& it : name)\
		{\
			if (!name_to_slot_index_##name.contains(it.second.scriptname))\
				name_to_slot_index_##name[it.second.scriptname] = i;\
			i++;\
		}\
	}

	DECL_INIT_MAP(ffcmap);
	DECL_INIT_MAP(globalmap);
	DECL_INIT_MAP(genericmap);
	DECL_INIT_MAP(itemmap);
	DECL_INIT_MAP(npcmap);
	DECL_INIT_MAP(ewpnmap);
	DECL_INIT_MAP(lwpnmap);
	DECL_INIT_MAP(playermap);
	DECL_INIT_MAP(dmapmap);
	DECL_INIT_MAP(screenmap);
	DECL_INIT_MAP(itemspritemap);
	DECL_INIT_MAP(comboscriptmap);
	DECL_INIT_MAP(subscreenmap);
}

static void do_get_script_index_by_name(const std::map<std::string, int>& name_to_slot_index)
{
	int32_t arrayptr = get_register(sarg1);
	string name;
	int32_t num=-1;
	ArrayH::getString(arrayptr, name, 256); // What's the limit on name length?
	
	auto it = name_to_slot_index.find(name);
	if (it != name_to_slot_index.end())
		num = it->second + 1;
	
	set_register(sarg1, num * 10000);
}

int32_t legacy_get_int_arr(const int32_t ptr, int32_t indx)
{
	switch(ptr)
	{
		case INTARR_SCREEN_NPC:
		{
			current_zasm_context = "Screen->NPCs[]";
			if(BC::checkGuyIndex(indx) != SH::_NoError)
				return 0;
			current_zasm_context = "";

			return guys.spr(indx)->getUID();
		}
		case INTARR_SCREEN_ITEMSPR:
		{
			current_zasm_context = "Screen->Items[]";
			if(BC::checkItemIndex(indx) != SH::_NoError)
				return 0;
			current_zasm_context = "";

			return items.spr(indx)->getUID();
		}
		case INTARR_SCREEN_LWPN:
		{
			current_zasm_context = "Screen->LWeapons[]";
			if(BC::checkLWeaponIndex(indx) != SH::_NoError)
				return 0;
			current_zasm_context = "";

			return Lwpns.spr(indx)->getUID();
		}
		case INTARR_SCREEN_EWPN:
		{
			current_zasm_context = "Screen->EWeapons[]";
			if(BC::checkEWeaponIndex(indx) != SH::_NoError)
				return 0;
			current_zasm_context = "";

			return Ewpns.spr(indx)->getUID();
		}
		case INTARR_SCREEN_FFC:
		{
			current_zasm_context = "Screen->FFCs[]";
			if (auto ffc = ResolveFFCWithID(indx))
			{
				current_zasm_context = "";

				if (ZScriptVersion::ffcRefIsSpriteId())
					return ffc->getUID();

				return indx * 10000;
			}

			return 0;
		}
		case INTARR_SCREEN_PORTALS:
		{
			current_zasm_context = "Screen->Portals[]";
			if(BC::checkBoundsOneIndexed(indx, 0, portals.Count()-1) != SH::_NoError)
				return 0;
			current_zasm_context = "";

			return portals.spr(indx)->getUID();
		}
		case INTARR_SAVPRTL:
		{
			current_zasm_context = "Game->SavedPortals[]";
			if(BC::checkBoundsOneIndexed(indx, 0, game->user_portals.size()-1) != SH::_NoError)
				return 0;
			current_zasm_context = "";

			return (indx+1)*10000;
		}
		default:
		{
			scripting_log_error_with_context("Unknown internal array '{}' read from!", ptr);
			return 0;
		}
	}
}
void legacy_set_int_arr(const int32_t ptr, int32_t indx, int32_t val)
{
	switch(ptr)
	{
		case INTARR_SCREEN_NPC:
			scripting_log_error_with_context("Read-only array 'Screen->NPCs' cannot be written to!");
			return;
		case INTARR_SCREEN_ITEMSPR:
			scripting_log_error_with_context("Read-only array 'Screen->Items' cannot be written to!");
			return;
		case INTARR_SCREEN_LWPN:
			scripting_log_error_with_context("Read-only array 'Screen->LWeapons' cannot be written to!");
			return;
		case INTARR_SCREEN_EWPN:
			scripting_log_error_with_context("Read-only array 'Screen->EWeapons' cannot be written to!");
			return;
		case INTARR_SCREEN_FFC:
			scripting_log_error_with_context("Read-only array 'Screen->FFCs' cannot be written to!");
			return;
		case INTARR_SCREEN_PORTALS:
			scripting_log_error_with_context("Read-only array 'Screen->Portals' cannot be written to!");
			return;
		case INTARR_SAVPRTL:
			scripting_log_error_with_context("Read-only array 'Game->SavedPortals' cannot be written to!");
			return;
		
		default:
		{
			scripting_log_error_with_context("Unknown internal array '{}' written to!", ptr);
			return;
		}
	}
}
int32_t legacy_sz_int_arr(const int32_t ptr)
{
	switch(ptr)
	{
		case INTARR_SCREEN_NPC:
		{
			return guys.Count();
		}
		case INTARR_SCREEN_ITEMSPR:
		{
			return items.Count();
		}
		case INTARR_SCREEN_LWPN:
		{
			return Lwpns.Count();
		}
		case INTARR_SCREEN_EWPN:
		{
			return Ewpns.Count();
		}
		case INTARR_SCREEN_FFC:
		{
			return MAXFFCS;
		}
		case INTARR_SCREEN_PORTALS:
		{
			return portals.Count();
		}
		case INTARR_SAVPRTL:
		{
			return game->user_portals.size();
		}
		default:
		{
			scripting_log_error_with_context("Unknown internal array '{}' size read!", ptr);
			return -1;
		}
	}
}

///----------------------------------------------------------------------------------------------------//
//                                       ASM Functions                                                 //
///----------------------------------------------------------------------------------------------------//

bool check_stack(uint32_t sp)
{
	if (sp >= MAX_STACK_SIZE)
	{
		log_stack_overflow_error();
		ri->overflow = true;
		return false;
	}

	return true;
}

void retstack_push(int32_t val)
{
	if(ri->retsp >= ret_stack->size())
	{
		log_call_limit_error();
		ri->overflow = true;
		return;
	}
	ret_stack->at(ri->retsp++) = val;
}
optional<int32_t> retstack_pop()
{
	if(!ri->retsp)
		return nullopt; //return from root, so, QUIT
	return ret_stack->at(--ri->retsp);
}

void stack_push(int32_t val)
{
	SH::write_stack(--ri->sp, val);
}
void stack_push(int32_t val, size_t count)
{
	if (!check_stack(ri->sp - count))
		return;

	for(int q = 0; q < count; ++q)
	{
		--ri->sp;
		(*stack)[ri->sp] = val;
	}
}

int32_t stack_pop()
{
	const int32_t val = SH::read_stack(ri->sp);
	++ri->sp;
	return val;
}
int32_t stack_pop(size_t count)
{
	ri->sp += count;
	const int32_t val = SH::read_stack(ri->sp-1);
	return val;
}

///----------------------------------------------------------------------------------------------------//
//Internal (to ZScript)

// Changing the script of the currently executing scriptable object is not supported.
bool is_guarded_script_register(int reg)
{
	switch (reg)
	{
		case DMAPSCRIPT:
		case EWPNSCRIPT:
		case FFSCRIPT:
		case IDATAPSCRIPT:
		case IDATASCRIPT:
		case ITEMSPRITESCRIPT:
		case LWPNSCRIPT:
		case NPCSCRIPT:
		case SCREENSCRIPT:
			return true;
	}

	return false;
}

void do_set(int reg, int value)
{
	if (!is_guarded_script_register(reg))
	{
		set_register(reg, value);
		return;
	}

	ScriptType whichType = curScriptType;
	int32_t whichUID = curScriptIndex;

	bool allowed = true;
	switch(whichType) //Check for objects attempting to change own script
	{
		//case ScriptType::Global:
		
		case ScriptType::FFC:
			if (reg == FFSCRIPT)
			{
				if (auto ffc = ResolveFFC(ri->ffcref); ffc && ffc->index == whichUID)
					allowed = false;
			}
			break;
		
		case ScriptType::Screen:
			if(reg==SCREENSCRIPT) //Only 1 screen script running at a time, no UID check needed
				allowed = false;
			break;
		
		case ScriptType::Item:
		{
			bool collect = ( ( whichUID < 1 ) || (whichUID == COLLECT_SCRIPT_ITEM_ZERO) );
			int32_t new_UID = ( collect ) ? (( whichUID != COLLECT_SCRIPT_ITEM_ZERO ) ? (whichUID * -1) : 0) : whichUID;
			
			if(collect)
			{
				if(reg==IDATAPSCRIPT && ri->idata==new_UID)
					allowed = false;
			}
			else if(reg==IDATASCRIPT && ri->idata==new_UID)
				allowed = false;
			break;
		}
		
		case ScriptType::Lwpn:
			if(reg==LWPNSCRIPT && ri->lwpn==whichUID)
				allowed = false;
			break;
			
		case ScriptType::NPC:
			if(reg==NPCSCRIPT && ri->guyref==whichUID)
				allowed = false;
			break;
		
		case ScriptType::Ewpn:
			if(reg==EWPNSCRIPT && ri->ewpn==whichUID)
				allowed = false;
			break;
		
		case ScriptType::DMap:
			if(reg==DMAPSCRIPT && ri->dmapsref==whichUID)
				allowed = false;
			break;
		
		case ScriptType::ItemSprite:
			if(reg==ITEMSPRITESCRIPT && ri->itemref==whichUID)
				allowed = false;
			break;
	}

	if (!allowed)
	{
		Z_scripterrlog("Script attempted to change own object's script! This has been ignored.\n");
		return;
	}

	set_register(reg, value);
}

void do_set_command(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v);
	do_set(sarg1, temp);
}

void do_push(const bool v)
{
	const int32_t value = SH::get_arg(sarg1, v);
	stack_push(value);
}
void do_push_varg(const bool v)
{
	const int32_t value = SH::get_arg(sarg1, v);
	zs_vargs.push_back(value);
}

void do_push_vargs(const bool v)
{
	if(sarg2 < 1) return;
	const int value = SH::get_arg(sarg1, v);
	zs_vargs.insert(zs_vargs.end(), sarg2, value);
	zs_vargs.push_back(value);
}

void do_pop()
{
	set_register(sarg1, stack_pop());
}

void do_peek()
{
	set_register(sarg1, SH::read_stack(ri->sp));
}

void do_peekat(const bool v)
{
	auto offs = SH::get_arg(sarg2,v);
	set_register(sarg1, SH::read_stack(ri->sp+offs));
}

void do_writeat(const bool v1, const bool v2)
{
	auto val = SH::get_arg(sarg1,v1);
	auto offs = SH::get_arg(sarg2,v2);
	SH::write_stack(ri->sp+offs, val);
}

void do_pops() // Pop past a bunch of stuff at once. Useful for clearing the stack.
{
	set_register(sarg1, stack_pop(sarg2));
}

void do_pushs(const bool v) // Push a bunch of the same thing. Useful for filling the stack.
{
	const int value = SH::get_arg(sarg1, v);
	stack_push(value, sarg2);
}

void do_loadi()
{
	const int32_t stackoffset = get_register(sarg2) / 10000;
	const int32_t value = SH::read_stack(stackoffset);
	set_register(sarg1, value);
}

void do_storei()
{
	const int32_t stackoffset = get_register(sarg2) / 10000;
	const int32_t value = get_register(sarg1);
	SH::write_stack(stackoffset, value);
}

void do_loadd()
{
	const int32_t stackoffset = (sarg2+ri->d[rSFRAME]) / 10000;
	const int32_t value = SH::read_stack(stackoffset);
	set_register(sarg1, value);
}

void do_load()
{
	const int32_t stackoffset = ri->d[rSFRAME] + sarg2;
	const int32_t value = SH::read_stack(stackoffset);
	set_register(sarg1, value);
}

static void do_load_internal_array()
{
	CHECK(ZScriptVersion::gc_arrays());

	auto array = find_or_create_internal_script_array({sarg2, 0});
	set_register(sarg1, array ? array->id : 0);
}

static void do_load_internal_array_ref()
{
	CHECK(ZScriptVersion::gc_arrays());

	int ref = get_register(sarg3);
	auto array = find_or_create_internal_script_array({sarg2, ref});
	set_register(sarg1, array ? array->id : 0);
}

void do_stored(const bool v)
{
	const int32_t stackoffset = (sarg2+ri->d[rSFRAME]) / 10000;
	const int32_t value = SH::get_arg(sarg1, v);
	SH::write_stack(stackoffset, value);
}

void do_store(const bool v)
{
	const int32_t stackoffset = ri->d[rSFRAME] + sarg2;
	const int32_t value = SH::get_arg(sarg1, v);
	SH::write_stack(stackoffset, value);
}

void script_store_object(uint32_t offset, uint32_t new_id)
{
	DCHECK(offset < MAX_STACK_SIZE);

	// Increase, then decrease, to handle the case where a variable (holding the only reference to an object) is assigned to itself.
	// This is unlikely so lets not bother with a conditional that skips both ref modifications when the ids are equal.
	uint32_t id = SH::read_stack(offset);
	script_object_ref_inc(new_id);
	if (ri->stack_pos_is_object.contains(offset))
		script_object_ref_dec(id);
	else
		ri->stack_pos_is_object.insert(offset);

	SH::write_stack(offset, new_id);

	if (util::remove_if_exists(script_object_autorelease_pool, new_id))
		script_object_ref_dec(new_id);
}

void do_store_object(const bool v)
{
	const int32_t stackoffset = ri->d[rSFRAME] + sarg2;
	const int32_t new_id = SH::get_arg(sarg1, v);
	script_store_object(stackoffset, new_id);
}

void script_remove_object_ref(int32_t offset)
{
	if (offset < 0 || offset >= MAX_STACK_SIZE)
	{
		assert(false);
		return;
	}

	if (!ri->stack_pos_is_object.contains(offset))
		return;

	uint32_t id = SH::read_stack(offset);
	script_object_ref_dec(id);
	ri->stack_pos_is_object.erase(offset);
}

void do_enqueue(const bool)
{
}
void do_dequeue(const bool)
{
}

void do_comp(bool v, const bool inv = false)
{
	bool v2 = false;
	if(inv) zc_swap(v,v2);
	ri->cmp_op2 = SH::get_arg(sarg2, v);
	ri->cmp_op1 = SH::get_arg(sarg1, v2);
	ri->cmp_strcache = nullopt;
}

void do_internal_strcmp()
{
	int32_t arrayptr_a = get_register(sarg1);
	int32_t arrayptr_b = get_register(sarg2);
	string strA;
	string strB;
	ArrayH::getString(arrayptr_a, strA);
	ArrayH::getString(arrayptr_b, strB);
	ri->cmp_strcache = strcmp(strA.c_str(), strB.c_str());
}

void do_internal_stricmp()
{
	int32_t arrayptr_a = get_register(sarg1);
	int32_t arrayptr_b = get_register(sarg2);
	string strA;
	string strB;
	ArrayH::getString(arrayptr_a, strA);
	ArrayH::getString(arrayptr_b, strB);
	ri->cmp_strcache = stricmp(strA.c_str(), strB.c_str());
}

void do_resize_array()
{
	int32_t size = vbound(get_register(sarg2) / 10000, 0, 214748);
	dword ptrval = get_register(sarg1);
	ArrayManager am(ptrval);
	am.resize(size);
}

void do_own_array(int arrindx, ScriptType scriptType, const int32_t UID)
{
	ArrayManager am(arrindx);
	
	if(am.internal())
	{
		Z_scripterrlog_force_trace("Cannot 'OwnArray()' an internal array '%d'\n", arrindx);
		return;
	}
	if(arrindx >= NUM_ZSCRIPT_ARRAYS && arrindx < NUM_ZSCRIPT_ARRAYS*2)
	{
		//ignore global arrays
	}
	else if(!am.invalid())
	{
		if(arrindx > 0 && arrindx < NUM_ZSCRIPT_ARRAYS)
		{
			arrayOwner[arrindx].reown(scriptType, UID);
			arrayOwner[arrindx].specOwned = true;
		}
		else if(arrindx < 0) //object array
			Z_scripterrlog_force_trace("Cannot 'OwnArray()' an object-based array '%d'\n", arrindx);
	}
	else Z_scripterrlog_force_trace("Tried to 'OwnArray()' an invalid array '%d'\n", arrindx);
}

void do_destroy_array()
{
	if (ZScriptVersion::gc_arrays())
	{
		scripting_log_error_with_context("Cannot force destroy arrays in 3.0+");
		return;
	}

	int arrindx = get_register(sarg1);
	
	ArrayManager am(arrindx);
	
	if(am.internal())
	{
		Z_scripterrlog_force_trace("Cannot 'DestroyArray()' an internal array '%d'\n", arrindx);
		return;
	}
	
	if(arrindx >= NUM_ZSCRIPT_ARRAYS && arrindx < NUM_ZSCRIPT_ARRAYS*2)
	{
		//ignore global arrays
	}
	else if(!am.invalid())
	{
		if(arrindx > 0 && arrindx < NUM_ZSCRIPT_ARRAYS)
		{
			arrayOwner[arrindx].clear();
			
			if(localRAM[arrindx].Valid())
				localRAM[arrindx].Clear();
			
			arrayOwner[arrindx].specCleared = true;
		}
		else if(arrindx < 0) //object array
			Z_scripterrlog_force_trace("Cannot 'DestroyArray()' an object-based array '%d'\n", arrindx);
	}
	else Z_scripterrlog_force_trace("Tried to 'DestroyArray()' an invalid array '%d'\n", arrindx);
}

static dword allocatemem_old(int32_t size, bool local, ScriptType type, const uint32_t UID, script_object_type object_type)
{
	dword ptrval;
	
	if(size < 0)
	{
		Z_scripterrlog_force_trace("Array initialized to invalid size of %d\n", size);
		return 0;
	}
	
	if(local)
	{
		//localRAM[0] is used as an invalid container, so 0 can be the NULL pointer in ZScript
		for(ptrval = 1; ptrval < NUM_ZSCRIPT_ARRAYS && localRAM[ptrval].Valid(); ptrval++) ;

		if(ptrval >= NUM_ZSCRIPT_ARRAYS)
		{
			Z_scripterrlog_force_trace("%d local arrays already in use, no more can be allocated\n", NUM_ZSCRIPT_ARRAYS-1);
			ptrval = 0;
			DCHECK(false);
		}
		else
		{
			ZScriptArray &a = localRAM[ptrval];
			
			a.Resize(size);
			a.setValid(true);
			a.setObjectType(object_type);
			
			for(dword j = 0; j < (dword)size; j++)
				a[j] = 0; //initialize array
				
			// Keep track of which object created the array so we know which to deallocate
			arrayOwner[ptrval].clear();
			arrayOwner[ptrval].reown(type, UID);
		}
	}
	else
	{
		//Globals are only allocated here at first play, otherwise in init_game
		for(ptrval = 0; ptrval < game->globalRAM.size() && game->globalRAM[ptrval].Valid(); ptrval++) ;
		
		if(ptrval >= game->globalRAM.size())
		{
			Z_scripterrlog_force_trace("Invalid pointer value of %u passed to global allocate\n", ptrval);
			ptrval = 0;
			//this shouldn't happen, unless people are putting ALLOCATEGMEM in their ZASM scripts where they shouldn't be
			DCHECK(false);
			return ptrval * 10000;
		}

		ZScriptArray &a = game->globalRAM[ptrval];
		
		a.Resize(size);
		a.setValid(true);
		a.setObjectType(object_type);
		
		for(dword j = 0; j < (dword)size; j++)
			a[j] = 0;
			
		ptrval += NUM_ZSCRIPT_ARRAYS; //so each pointer has a unique value
	}

	return ptrval * 10000;
}

uint32_t allocatemem(int32_t size, bool local, ScriptType type, const uint32_t UID, script_object_type object_type)
{
	if (!ZScriptVersion::gc_arrays())
		return allocatemem_old(size, local, type, UID, object_type);

	if(size < 0)
	{
		Z_scripterrlog_force_trace("Array initialized to invalid size of %d\n", size);
		return 0;
	}

	auto* array = script_arrays.create();
	if (!array)
		return 0;

	ZScriptArray &a = array->arr;
	a.Resize(size);
	a.setValid(true);
	a.setObjectType(object_type);

	for(dword j = 0; j < (dword)size; j++)
		a[j] = 0; //initialize array

	return array->id;
}

void do_allocatemem(bool v, const bool local, ScriptType type, const uint32_t UID)
{
	int32_t size = SH::get_arg(sarg2, v) / 10000;
	assert(sarg3 >= 0 && sarg3 <= (int)script_object_type::last);
	uint32_t id = allocatemem(size, local, type, UID, (script_object_type)sarg3);
	set_register(sarg1, id);
}

void do_deallocatemem()
{
	const int32_t ptrval = get_register(sarg1) / 10000;
	
	FFScript::deallocateArray(ptrval);
}

///----------------------------------------------------------------------------------------------------//
//Mathematical

void do_add(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v);
	int32_t temp2 = get_register(sarg1);
	
	set_register(sarg1, temp2 + temp);
}

void do_sub(bool v, const bool inv = false)
{
	bool v2 = false;
	if(inv) zc_swap(v,v2);
	auto destreg = (inv ? sarg2 : sarg1);
	int32_t temp = SH::get_arg(sarg2, v);
	int32_t temp2 = SH::get_arg(sarg1, v2);
	set_register(destreg, temp2 - temp);
}

void do_mult(const bool v)
{
	int64_t temp = SH::get_arg(sarg2, v);
	int32_t temp2 = get_register(sarg1);
	
	set_register(sarg1, int32_t((temp * temp2) / 10000));
}

void do_div(bool v, const bool inv = false)
{
	bool v2 = false;
	if(inv) zc_swap(v,v2);
	auto destreg = (inv ? sarg2 : sarg1);
	int64_t temp = SH::get_arg(sarg2, v);
	int64_t temp2 = SH::get_arg(sarg1, v2);
	
	if(temp == 0)
	{
		scripting_log_error_with_context("Attempted to divide by zero!");
		set_register(destreg, int32_t(sign(temp2) * MAX_SIGNED_32));
	}
	else
	{
		set_register(destreg, int32_t((temp2 * 10000) / temp));
	}
}

void do_mod(bool v, const bool inv = false)
{
	bool v2 = false;
	if(inv) zc_swap(v,v2);
	auto destreg = (inv ? sarg2 : sarg1);
	int32_t temp = SH::get_arg(sarg2, v);
	int32_t temp2 = SH::get_arg(sarg1, v2);
	
	if(temp == 0)
	{
		scripting_log_error_with_context("Attempted to modulo by zero!");
		temp = 1;
	}
	
	set_register(destreg, temp2 % temp);
}

void do_trig(const bool v, const byte type)
{
	double rangle = (SH::get_arg(sarg2, v) / 10000.0) * PI / 180.0;
	
	switch(type)
	{
		case 0:
			set_register(sarg1, int32_t(zc::math::Sin(rangle) * 10000.0));
			break;
			
		case 1:
			set_register(sarg1, int32_t(zc::math::Cos(rangle) * 10000.0));
			break;
			
		case 2:
			set_register(sarg1, int32_t(zc::math::Tan(rangle) * 10000.0));
			break;
	}
}

void do_degtorad()
{
	double rangle = (SH::get_arg(sarg2, false) / 10000.0) * (PI / 180.0);
	rangle += rangle < 0?-0.00005:0.00005;
	
	set_register(sarg1, int32_t(rangle * 10000.0));
}

void do_radtodeg()
{
	double rangle = (SH::get_arg(sarg2, false) / 10000.0) * (180.0 / PI);
	
	set_register(sarg1, int32_t(rangle * 10000.0));
}

void do_asin(const bool v)
{
	double temp = double(SH::get_arg(sarg2, v)) / 10000.0;
	
	if(temp >= -1 && temp <= 1)
		set_register(sarg1, int32_t(asin(temp) * 10000.0));
	else
	{
		Z_scripterrlog("Script attempted to pass %f into ArcSin!\n",temp);
		set_register(sarg1, -10000);
	}
}

void do_acos(const bool v)
{
	double temp = double(SH::get_arg(sarg2, v)) / 10000.0;
	
	if(temp >= -1 && temp <= 1)
		set_register(sarg1, int32_t(acos(temp) * 10000.0));
	else
	{
		Z_scripterrlog("Script attempted to pass %f into ArcCos!\n",temp);
		set_register(sarg1, -10000);
	}
}

void do_arctan()
{
	double xpos = ri->d[rINDEX] / 10000.0;
	double ypos = ri->d[rINDEX2] / 10000.0;
	
	set_register(sarg1, int32_t(atan2(ypos, xpos) * 10000.0));
}

void do_abs(const bool v)
{
	int32_t temp = SH::get_arg(sarg1, v);
	set_register(sarg1, abs(temp));
}

void do_log10(const bool v)
{
	double temp = double(SH::get_arg(sarg1, v)) / 10000.0;
	
	if(temp > 0)
		set_register(sarg1, int32_t(log10(temp) * 10000.0));
	else
	{
		Z_scripterrlog("Script tried to calculate log of %f\n", temp / 10000.0);
		set_register(sarg1, 0);
	}
}

void do_naturallog(const bool v)
{
	double temp = double(SH::get_arg(sarg1, v)) / 10000.0;
	
	if(temp > 0)
		set_register(sarg1, int32_t(log(temp) * 10000.0));
	else
	{
		Z_scripterrlog("Script tried to calculate ln of %f\n", temp / 10000.0);
		set_register(sarg1, 0);
	}
}

void do_min(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v);
	int32_t temp2 = get_register(sarg1);
	set_register(sarg1, zc_min(temp2, temp));
}

void do_max(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v);
	int32_t temp2 = get_register(sarg1);
	
	set_register(sarg1, zc_max(temp2, temp));
}
void do_wrap_rad(const bool v)
{
	ri->d[rEXP1] = wrap_zslong_rad(SH::get_arg(sarg1, v));
}
void do_wrap_deg(const bool v)
{
	ri->d[rEXP1] = wrap_zslong_deg(SH::get_arg(sarg1, v));
}


void do_rnd(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v) / 10000;

	if(temp > 0)
		set_register(sarg1, (zc_oldrand() % temp) * 10000);
	else if(temp < 0)
		set_register(sarg1, (zc_oldrand() % (-temp)) * -10000);
	else
		set_register(sarg1, 0); // Just return 0. (Do not log an error)
}

void do_srnd(const bool v)
{
	uint32_t seed = SH::get_arg(sarg1, v); //Do not `/10000`- allow the decimal portion to be used! -V
	zc_game_srand(seed);
}

void do_srndrnd()
{
	//Randomize the seed to the current system time, + or - the product of 2 random numbers.
	int32_t seed = time(0) + ((zc_rand() * int64_t(zc_rand())) * (zc_rand(1) ? 1 : -1));
	set_register(sarg1, seed);
	zc_game_srand(seed);
}

//Returns the system Real-Time-Clock value for a specific type. 
void FFScript::getRTC(const bool v)
{
	//int32_t type = get_register(sarg1) / 10000;
	//int32_t time = getTime(type);
	set_register(sarg1, getTime((get_register(sarg1) / 10000)) * 10000);
}


void do_factorial(const bool v)
{
	int32_t temp;
	
	if(v)
		return;  //must factorial a register, not a value (why is this exactly? ~Joe123)
	else
	{
		temp = get_register(sarg1) / 10000;
		
		if(temp < 2)
		{
			set_register(sarg1, temp >= 0 ? 10000 : 0);
			return;
		}
	}
	
	int32_t temp2 = 1;
	
	for(int32_t temp3 = temp; temp > 1; temp--)
		temp2 *= temp3;
		
	set_register(sarg1, temp2 * 10000);
}

void do_power(bool v, const bool inv = false)
{
	bool v2 = false;
	if(inv) zc_swap(v,v2);
	auto destreg = (inv ? sarg2 : sarg1);
	double temp = double(SH::get_arg(sarg2, v)) / 10000.0;
	double temp2 = double(SH::get_arg(sarg1, v2)) / 10000.0;
	
	if(temp == 0 && temp2 == 0)
	{
		set_register(destreg, 10000);
		return;
	}
	
	set_register(destreg, int32_t(pow(temp2, temp) * 10000.0));
}

void do_lpower(bool v, const bool inv = false)
{
	bool v2 = false;
	if(inv) zc_swap(v,v2);
	auto destreg = (inv ? sarg2 : sarg1);
	int32_t temp = SH::get_arg(sarg2, v);
	int32_t temp2 = SH::get_arg(sarg1, v2);
	
	if(temp == 0 && temp2 == 0)
	{
		set_register(destreg, 1);
		return;
	}
	
	set_register(destreg, int32_t(pow(temp2, temp)));
}

//could use recursion or something to avoid truncation.
void do_ipower(const bool v)
{
	double sarg2val = double(SH::get_arg(sarg2, v));
	if ( sarg2val == 0 )
	{
		Z_scripterrlog("Division by 0 Err: InvPower() exponent divisor cannot be 0!!\n");
		set_register(sarg1, 1);
		return;
	}
	double temp = 10000.0 / sarg2val;
	double temp2 = double(get_register(sarg1)) / 10000.0;
	
	if(temp == 0 && temp2 == 0)
	{
		set_register(sarg1, 1);
		return;
	}
	
	set_register(sarg1, int32_t(pow(temp2, temp) * 10000.0));
}

void do_sqroot(const bool v)
{
	double temp = double(SH::get_arg(sarg2, v)) / 10000.0;
	
	if(temp < 0)
	{
		Z_scripterrlog("Script attempted to calculate square root of %f!\n", temp);
		set_register(sarg1, -10000);
		return;
	}
	
	set_register(sarg1, int32_t(sqrt(temp) * 10000.0));
}

///----------------------------------------------------------------------------------------------------//
//Bitwise

void do_and(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v) / 10000;
	int32_t temp2 = get_register(sarg1) / 10000;
	set_register(sarg1, (temp2 & temp) * 10000);
}

void do_and32(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v);
	int32_t temp2 = get_register(sarg1);
	set_register(sarg1, (temp2 & temp));
}

void do_or(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v) / 10000;
	int32_t temp2 = get_register(sarg1) / 10000;
	set_register(sarg1, (temp2 | temp) * 10000);
}

void do_or32(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v);
	int32_t temp2 = get_register(sarg1);
	set_register(sarg1, (temp2 | temp));
}

void do_xor(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v) / 10000;
	int32_t temp2 = get_register(sarg1) / 10000;
	set_register(sarg1, (temp2 ^ temp) * 10000);
}

void do_xor32(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v);
	int32_t temp2 = get_register(sarg1);
	set_register(sarg1, (temp2 ^ temp));
}

void do_nand(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v) / 10000;
	int32_t temp2 = get_register(sarg1) / 10000;
	set_register(sarg1, (~(temp2 & temp)) * 10000);
}

void do_nor(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v) / 10000;
	int32_t temp2 = get_register(sarg1) / 10000;
	set_register(sarg1, (~(temp2 | temp)) * 10000);
}

void do_xnor(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v) / 10000;
	int32_t temp2 = get_register(sarg1) / 10000;
	set_register(sarg1, (~(temp2 ^ temp)) * 10000);
}

void do_not(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v);
	set_register(sarg1, !temp);
}

void do_bitwisenot(const bool v)
{
	int32_t temp = SH::get_arg(sarg1, v) / 10000;
	set_register(sarg1, (~temp) * 10000);
}

void do_bitwisenot32(const bool v)
{
	int32_t temp = SH::get_arg(sarg1, v);
	set_register(sarg1, (~temp));
}

void do_lshift(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v) / 10000;
	int32_t temp2 = get_register(sarg1) / 10000;
	set_register(sarg1, (temp2 << temp) * 10000);
}

void do_lshift32(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v) / 10000;
	int32_t temp2 = get_register(sarg1);
	set_register(sarg1, (temp2 << temp));
}

void do_rshift(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v) / 10000;
	int32_t temp2 = get_register(sarg1) / 10000;
	set_register(sarg1, (temp2 >> temp) * 10000);
}

void do_rshift32(const bool v)
{
	int32_t temp = SH::get_arg(sarg2, v) / 10000;
	int32_t temp2 = get_register(sarg1);
	set_register(sarg1, (temp2 >> temp));
}

///----------------------------------------------------------------------------------------------------//
//Casting

void do_boolcast(const bool isFloat)
{
	set_register(sarg1, (get_register(sarg1) ? (isFloat ? 1 : 10000) : 0));
}

///----------------------------------------------------------------------------------------------------//
//Text ptr functions
void do_fontheight()
{
	int32_t font = get_register(sarg1)/10000;
	ri->d[rEXP1] = text_height(get_zc_font(font))*10000;
}

void do_strwidth()
{
	int32_t strptr = get_register(sarg1);
	int32_t font = get_register(sarg2)/10000;
	string the_string;
	ArrayH::getString(strptr, the_string, 512);
	ri->d[rEXP1] = text_length(get_zc_font(font), the_string.c_str())*10000;
}

void do_charwidth()
{
	char chr = get_register(sarg1)/10000;
	int32_t font = get_register(sarg2)/10000;
	char *cstr = new char[2];
	cstr[0] = chr;
	cstr[1] = '\0';
	ri->d[rEXP1] = text_length(get_zc_font(font), cstr)*10000;
	delete[] cstr;
}

int32_t do_msgwidth(int32_t ID)
{
	if(BC::checkMessage(ID) != SH::_NoError)
	{
		return -1;
	}
	
	int32_t v = text_length(get_zc_font(MsgStrings[ID].font),
		MsgStrings[ID].s.substr(0,MsgStrings[ID].s.find_last_not_of(' ')+1).c_str());
	return v;
}

int32_t do_msgheight(int32_t ID)
{
	if(BC::checkMessage(ID) != SH::_NoError)
	{
		return -1;
	}
	return text_height(get_zc_font(MsgStrings[ID].font));
}

///----------------------------------------------------------------------------------------------------//
//Gameplay functions

void do_warp(bool v)
{
	int32_t dmapid = SH::get_arg(sarg1, v) / 10000;
	int32_t screen = SH::get_arg(sarg2, v) / 10000;
	if ( ((unsigned)dmapid) >= MAXDMAPS ) 
	{
		Z_scripterrlog("Invalid DMap ID (%d) passed to Warp(). Aborting.\n", dmapid);
		return;
	}
	if ( ((unsigned)screen) >= MAPSCRS ) 
	{
		Z_scripterrlog("Invalid Screen Index (%d) passed to Warp(). Aborting.\n", screen);
		return;
	}
	if ( map_screen_index(DMaps[dmapid].map, screen + DMaps[dmapid].xoff) >= (int32_t)TheMaps.size() )
	{
		Z_scripterrlog("Invalid destination passed to Warp(). Aborting.\n");
		return;
	}
	hero_scr->sidewarpdmap[0] = dmapid;
	hero_scr->sidewarpscr[0]  = screen;
	hero_scr->sidewarptype[0] = wtIWARP;
	if(!get_qr(qr_OLD_HERO_WARP_RETSQUARE))
	{
		hero_scr->warpreturnc &= ~(3 << 8);
		set_bit(&hero_scr->sidewarpoverlayflags,0,0);
	}
	Hero.ffwarp = true;
}

void do_pitwarp(bool v)
{
	int32_t dmapid = SH::get_arg(sarg1, v) / 10000;
	int32_t screen = SH::get_arg(sarg2, v) / 10000;
	if ( ((unsigned)dmapid) >= MAXDMAPS ) 
	{
		Z_scripterrlog("Invalid DMap ID (%d) passed to PitWarp(). Aborting.\n", dmapid);
		return;
	}
	if ( ((unsigned)screen) >= MAPSCRS ) 
	{
		Z_scripterrlog("Invalid Screen Index (%d) passed to PitWarp(). Aborting.\n", screen);
		return;
	}
	//Extra sanity guard. 
	if ( map_screen_index(DMaps[dmapid].map, screen + DMaps[dmapid].xoff) >= (int32_t)TheMaps.size() )
	{
		Z_scripterrlog("Invalid destination passed to Warp(). Aborting.\n");
		return;
	}
	hero_scr->sidewarpdmap[0] = dmapid;
	hero_scr->sidewarpscr[0]  = screen;
	hero_scr->sidewarptype[0] = wtIWARP;
	if(!get_qr(qr_OLD_HERO_WARP_RETSQUARE))
	{
		hero_scr->warpreturnc &= ~(3 << 8);
		set_bit(&hero_scr->sidewarpoverlayflags,0,0);
	}
	Hero.ffwarp = true;
	Hero.ffpit = true;
}



void do_showsavescreen()
{
	bool didsaved = save_game(false, 0);
	set_register(sarg1, didsaved ? 10000 : 0);
}

void do_selectweapon(bool v, int32_t btn)
{
	switch(btn)
	{
		case 1:
			if(!get_qr(qr_SELECTAWPN))
				return;
			break;
		case 2:
			if(!get_qr(qr_SET_XBUTTON_ITEMS))
				return;
			break;
		case 3:
			if(!get_qr(qr_SET_YBUTTON_ITEMS))
				return;
			break;
	}
		
	byte dir=(byte)(SH::get_arg(sarg1, v)/10000);
	
	// Selection directions don't match the normal ones...
	switch(dir)
	{
	case 0:
		dir=SEL_UP;
		break;
		
	case 1:
		dir=SEL_DOWN;
		break;
		
	case 2:
		dir=SEL_LEFT;
		break;
		
	case 3:
		dir=SEL_RIGHT;
		break;
		
	default:
		return;
	}
	
	switch(btn)
	{
		case 0:
			selectNextBWpn(dir);
			break;
		case 1:
			selectNextAWpn(dir);
			break;
		case 2:
			selectNextXWpn(dir);
			break;
		case 3:
			selectNextYWpn(dir);
			break;
	}
}

///----------------------------------------------------------------------------------------------------//
//Screen Information

void do_issolid()
{
	int32_t x = int32_t(ri->d[rINDEX] / 10000);
	int32_t y = int32_t(ri->d[rINDEX2] / 10000);
	
	set_register(sarg1, (_walkflag(x, y, 1) ? 10000 : 0));
}

void do_mapdataissolid()
{
	auto result = decode_mapdata_ref(ri->mapsref);
	if (!result.scr)
	{
		scripting_log_error_with_context("mapdata pointer is either invalid or uninitialised");
		set_register(sarg1,10000);
	}
	else
	{
		int32_t x = int32_t(ri->d[rINDEX] / 10000);
		int32_t y = int32_t(ri->d[rINDEX2] / 10000);

		if (result.type == mapdata_type::CanonicalScreen)
		{
			set_register(sarg1, (_walkflag(x, y, 1, result.scr) ? 10000 : 0));
			return;
		}

		if (result.type == mapdata_type::TemporaryCurrentRegion && result.layer == 0)
		{
			set_register(sarg1, (_walkflag(x, y, 1)) ? 10000 : 0);
		}
		else if (result.type == mapdata_type::TemporaryScrollingRegion && result.layer == 0)
		{
			mapscr* s0 = GetScrollingMapscr(0, x, y);
			mapscr* s1 = GetScrollingMapscr(1, x, y);
			mapscr* s2 = GetScrollingMapscr(2, x, y);
			if (!s1->valid) s1 = s0;
			if (!s2->valid) s2 = s0;
			bool result = _walkflag_new(s0, s1, s2, x, y, 0_zf, true);
			set_register(sarg1, result ? 10000 : 0);
		}
		else
		{
			set_register(sarg1, (_walkflag(x, y, 1, result.scr) ? 10000 : 0));
		}
	}
}

void do_mapdataissolid_layer()
{
	auto result = decode_mapdata_ref(ri->mapsref);
	if (!result.scr)
	{
		scripting_log_error_with_context("mapdata pointer is either invalid or uninitialised");
		set_register(sarg1,10000);
	}
	else
	{
		int32_t x = int32_t(ri->d[rINDEX] / 10000);
		int32_t y = int32_t(ri->d[rINDEX2] / 10000);
		int32_t layer = int32_t(ri->d[rEXP1] / 10000);
		if(BC::checkBounds(layer, 0, 6) != SH::_NoError)
		{
			set_register(sarg1,10000);
		}
		else
		{
			if (result.type == mapdata_type::TemporaryCurrentRegion && result.layer == 0)
			{
				set_register(sarg1, (_walkflag_layer(x, y, 1, result.scr)) ? 10000 : 0);
			}
			else if (result.type == mapdata_type::TemporaryScrollingRegion && result.layer == 0)
			{
				set_register(sarg1, (_walkflag_layer_scrolling(x, y, 1, result.scr)) ? 10000 : 0);
			}
			else
			{
				mapscr* m = result.scr;

				if(layer > 0)
				{
					if(m->layermap[layer] == 0)
					{
						set_register(sarg1,10000);
						return;
					}

					m = &TheMaps[(m->layermap[layer]*MAPSCRS + m->layerscreen[layer])];
				}

				set_register(sarg1, (_walkflag_layer(x, y, 1, m) ? 10000 : 0));
			}
		}
	}
}

void do_issolid_layer()
{
	int32_t x = int32_t(ri->d[rINDEX] / 10000);
	int32_t y = int32_t(ri->d[rINDEX2] / 10000);
	int32_t layer = int32_t(ri->d[rEXP1] / 10000);
	if(BC::checkBounds(layer, 0, 6) != SH::_NoError)
	{
		set_register(sarg1,10000);
	}
	else
	{
		set_register(sarg1, (_walkflag_layer(x, y, layer - 1, 1)) ? 10000 : 0);
	}
}

void do_setsidewarp()
{
	int32_t warp   = SH::read_stack(ri->sp + 3) / 10000;
	int32_t scrn = SH::read_stack(ri->sp + 2) / 10000;
	int32_t dmap   = SH::read_stack(ri->sp + 1) / 10000;
	int32_t type   = SH::read_stack(ri->sp + 0) / 10000;
	
	current_zasm_extra_context = "warp";
	if (BC::checkBounds(warp, -1, 3) != SH::_NoError)
		return;

	current_zasm_extra_context = "screen";
	if (BC::checkBounds(scrn, -1, 0x87) != SH::_NoError)
		return;

	current_zasm_extra_context = "dmap";
	if (BC::checkBounds(dmap, -1, MAXDMAPS - 1) != SH::_NoError)
		return;

	current_zasm_extra_context = "type";
	if (BC::checkBounds(type, -1, wtMAX - 1) != SH::_NoError)
		return;

	current_zasm_extra_context = "";
	
	mapscr* scr = get_scr(ri->screenref);
		
	if(scrn > -1)
		scr->sidewarpscr[warp] = scrn;
		
	if(dmap > -1)
		scr->sidewarpdmap[warp] = dmap;
		
	if(type > -1)
		scr->sidewarptype[warp] = type;
}

void do_settilewarp()
{
	int32_t warp   = SH::read_stack(ri->sp + 3) / 10000;
	int32_t scrn = SH::read_stack(ri->sp + 2) / 10000;
	int32_t dmap   = SH::read_stack(ri->sp + 1) / 10000;
	int32_t type   = SH::read_stack(ri->sp + 0) / 10000;

	current_zasm_extra_context = "warp";
	if (BC::checkBounds(warp, -1, 3) != SH::_NoError)
		return;

	current_zasm_extra_context = "screen";
	if (BC::checkBounds(scrn, -1, 0x87) != SH::_NoError)
		return;

	current_zasm_extra_context = "dmap";
	if (BC::checkBounds(dmap, -1, MAXDMAPS - 1) != SH::_NoError)
		return;

	current_zasm_extra_context = "type";
	if (BC::checkBounds(type, -1, wtMAX - 1) != SH::_NoError)
		return;

	current_zasm_extra_context = "";

	mapscr* scr = get_scr(ri->screenref);
		
	if(scrn > -1)
		scr->tilewarpscr[warp] = scrn;
		
	if(dmap > -1)
		scr->tilewarpdmap[warp] = dmap;
		
	if(type > -1)
		scr->tilewarptype[warp] = type;
}

void do_getsidewarpdmap(const bool v)
{
	int32_t warp = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkBounds(warp, -1, 3) != SH::_NoError)
	{
		set_register(sarg1, -10000);
		return;
	}
	
	set_register(sarg1, get_scr(ri->screenref)->sidewarpdmap[warp]*10000);
}

void do_getsidewarpscr(const bool v)
{
	int32_t warp = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkBounds(warp, -1, 3) != SH::_NoError)
	{
		set_register(sarg1, -10000);
		return;
	}
	
	set_register(sarg1, get_scr(ri->screenref)->sidewarpscr[warp]*10000);
}

void do_getsidewarptype(const bool v)
{
	int32_t warp = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkBounds(warp, -1, 3) != SH::_NoError)
	{
		set_register(sarg1, -10000);
		return;
	}
	
	set_register(sarg1, get_scr(ri->screenref)->sidewarptype[warp]*10000);
}

void do_gettilewarpdmap(const bool v)
{
	int32_t warp = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkBounds(warp, -1, 3) != SH::_NoError)
	{
		set_register(sarg1, -10000);
		return;
	}
	
	set_register(sarg1, get_scr(ri->screenref)->tilewarpdmap[warp]*10000);
}

void do_gettilewarpscr(const bool v)
{
	int32_t warp = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkBounds(warp, -1, 3) != SH::_NoError)
	{
		set_register(sarg1, -10000);
		return;
	}
	
	set_register(sarg1, get_scr(ri->screenref)->tilewarpscr[warp]*10000);
}

void do_gettilewarptype(const bool v)
{
	int32_t warp = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkBounds(warp, -1, 3) != SH::_NoError)
	{
		set_register(sarg1, -10000);
		return;
	}
	
	set_register(sarg1, get_scr(ri->screenref)->tilewarptype[warp]*10000);
}

void do_layerscreen()
{
	int32_t layer = (get_register(sarg2) / 10000) - 1;
	
	if(BC::checkBounds(layer, 0, 5) != SH::_NoError || get_scr(ri->screenref)->layermap[layer] == 0)
		set_register(sarg1, -10000);
	else
		set_register(sarg1, get_scr(ri->screenref)->layerscreen[layer] * 10000);
}

void do_layermap()
{
	int32_t layer = (get_register(sarg2) / 10000) - 1;
	
	if(BC::checkBounds(layer, 0, 5) != SH::_NoError || get_scr(ri->screenref)->layermap[layer] == 0)
		set_register(sarg1, -10000);
	else
		set_register(sarg1, get_scr(ri->screenref)->layermap[layer] * 10000);
}



void do_triggersecrets(int screen)
{
	if (!is_in_current_region(screen))
	{
		scripting_log_error_with_context("Must be given a screen in the current region. got: {}", screen);
		return;
	}

	trigger_secrets_for_screen(TriggerSource::Script, screen, false);
}

void FFScript::do_graphics_getpixel()
{
	int32_t yoffset = 0;
	const bool brokenOffset= ( (get_er(er_BITMAPOFFSET)!=0) || (get_qr(qr_BITMAPOFFSETFIX)!=0) );
	int32_t ref = (ri->d[rEXP1]);
	
	BITMAP *bitty = FFCore.GetScriptBitmap(ref, screen);
	int32_t xpos  = ri->d[rINDEX2] / 10000;
	
	if(!brokenOffset && (ref-10) == -1 )
	{
		yoffset = 56; //should this be -56?
	}
	else
	{
		yoffset = 0;
	}
	
	int32_t ypos = (ri->d[rINDEX] / 10000)+yoffset;
	if(!bitty)
	{
		bitty = scrollbuf;
	}
	
	int32_t ret =  getpixel(bitty, xpos, ypos); //This is a palette index value. 
	
	if(!get_qr(qr_BROKEN_GETPIXEL_VALUE))
		ret *= 10000;
	set_register(sarg1, ret);
}




///----------------------------------------------------------------------------------------------------//
//Pointer handling

bool is_valid_array(int32_t ptr)
{
	if(!ptr) return false;

	if (ZScriptVersion::gc_arrays())
	{
		if (auto array = checkArray(ptr, true))
			return !array->internal_expired;

		return false;
	}

	ptr /= 10000;

	if(ptr < 0) //An object array?
	{
		int32_t objptr = -ptr;
		auto it = objectRAM.find(objptr);
		if(it == objectRAM.end())
			return false;
		return true;
	}
	else if(ptr >= NUM_ZSCRIPT_ARRAYS) //check global
	{
		dword gptr = ptr - NUM_ZSCRIPT_ARRAYS;
		
		if(gptr > game->globalRAM.size())
			return false;
		else return game->globalRAM[gptr].Valid();
	}
	else
	{
		return localRAM[ptr].Valid();
	}
}

void do_isvalidarray()
{
	int32_t ptr = get_register(sarg1);
	
	set_register(sarg1,is_valid_array(ptr) ? 10000 : 0);
}

void do_isvaliditem()
{
	int32_t IID = get_register(sarg1);
	//int32_t ct = items.Count();
  
	//for ( int32_t j = items.Count()-1; j >= 0; --j )
	for(int32_t j = 0; j < items.Count(); j++)
	//for(int32_t j = 0; j < ct; j++)
		if(items.spr(j)->getUID() == IID)
		{
			set_register(sarg1, 10000);
			return;
		}
		
	set_register(sarg1, 0);
}

void do_isvalidnpc()
{
	int32_t UID = get_register(sarg1);
	//for ( int32_t j = guys.Count()-1; j >= 0; --j )
	//int32_t ct = guys.Count(); 
	
	for(int32_t j = 0; j < guys.Count(); j++)
	//for(int32_t j = 0; j < ct; j++)
		if(guys.spr(j)->getUID() == UID)
		{
			set_register(sarg1, 10000);
			return;
		}
		
	set_register(sarg1, 0);
}

void do_isvalidlwpn()
{
	int32_t WID = get_register(sarg1);
	//int32_t ct = Lwpns.Count();
	
	//for ( int32_t j = Lwpns.Count()-1; j >= 0; --j )
	for(int32_t j = 0; j < Lwpns.Count(); j++)
	//for(int32_t j = 0; j < ct; j++)
		if(Lwpns.spr(j)->getUID() == WID)
		{
			set_register(sarg1, 10000);
			return;
		}
	if(Hero.lift_wpn && Hero.lift_wpn->getUID() == WID)
	{
		set_register(sarg1, 10000);
		return;
	}
	set_register(sarg1, 0);
}

void do_isvalidewpn()
{
	int32_t WID = get_register(sarg1);

	for(int32_t j = 0; j < Ewpns.Count(); j++)
		if(Ewpns.spr(j)->getUID() == WID)
		{
			set_register(sarg1, 10000);
			return;
		}
	// unsure how an ewpn would be lifted, but, checking just to be safe
	if(Hero.lift_wpn && Hero.lift_wpn->getUID() == WID)
	{
		set_register(sarg1, 10000);
		return;
	}
	set_register(sarg1, 0);
}

void do_lwpnmakeangular()
{
	if(LwpnH::loadWeapon(ri->lwpn) == SH::_NoError)
	{
		if (!LwpnH::getWeapon()->angular)
		{
			double vx;
			double vy;
			switch(NORMAL_DIR(LwpnH::getWeapon()->dir))
			{
				case l_up:
				case l_down:
				case left:
					vx = -1.0*((weapon*)s)->step;
					break;
				case r_down:
				case r_up:
				case right:
					vx = ((weapon*)s)->step;
					break;
					
				default:
					vx = 0;
					break;
			}
			switch(NORMAL_DIR(LwpnH::getWeapon()->dir))
			{
				case l_up:
				case r_up:
				case up:
					vy = -1.0*((weapon*)s)->step;
					break;
				case l_down:
				case r_down:
				case down:
					vy = ((weapon*)s)->step;
					break;
					
				default:
					vy = 0;
					break;
			}
			LwpnH::getWeapon()->angular = true;
			LwpnH::getWeapon()->angle=atan2(vy, vx);
			LwpnH::getWeapon()->step=FFCore.Distance(0, 0, vx, vy)/10000.0;
			LwpnH::getWeapon()->doAutoRotate();
		}
	}
}

void do_lwpnmakedirectional()
{
	if(LwpnH::loadWeapon(ri->lwpn) == SH::_NoError)
	{
		if (LwpnH::getWeapon()->angular)
		{
			LwpnH::getWeapon()->dir = NORMAL_DIR(AngleToDir(WrapAngle(LwpnH::getWeapon()->angle)));
			LwpnH::getWeapon()->angular = false;
			LwpnH::getWeapon()->doAutoRotate(true);
		}
	}
}

void do_ewpnmakeangular()
{
	if(EwpnH::loadWeapon(ri->ewpn) == SH::_NoError)
	{
		if (!EwpnH::getWeapon()->angular)
		{
			double vx;
			double vy;
			switch(NORMAL_DIR(EwpnH::getWeapon()->dir))
			{
				case l_up:
				case l_down:
				case left:
					vx = -1.0*((weapon*)s)->step;
					break;
				case r_down:
				case r_up:
				case right:
					vx = ((weapon*)s)->step;
					break;
					
				default:
					vx = 0;
					break;
			}
			switch(NORMAL_DIR(EwpnH::getWeapon()->dir))
			{
				case l_up:
				case r_up:
				case up:
					vy = -1.0*((weapon*)s)->step;
					break;
				case l_down:
				case r_down:
				case down:
					vy = ((weapon*)s)->step;
					break;
					
				default:
					vy = 0;
					break;
			}
			EwpnH::getWeapon()->angular = true;
			EwpnH::getWeapon()->angle=atan2(vy, vx);
			EwpnH::getWeapon()->step=FFCore.Distance(0, 0, vx, vy)/10000.0;
			EwpnH::getWeapon()->doAutoRotate();
		}
	}
}

void do_ewpnmakedirectional()
{
	if(EwpnH::loadWeapon(ri->lwpn) == SH::_NoError)
	{
		if (EwpnH::getWeapon()->angular)
		{
			EwpnH::getWeapon()->dir = NORMAL_DIR(AngleToDir(WrapAngle(EwpnH::getWeapon()->angle)));
			EwpnH::getWeapon()->angular = false;
			EwpnH::getWeapon()->doAutoRotate(true);
		}
	}
}

void do_lwpnusesprite(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkWeaponMiscSprite(ID) != SH::_NoError)
		return;
		
	if(LwpnH::loadWeapon(ri->lwpn) == SH::_NoError)
		LwpnH::getWeapon()->LOADGFX(ID);
}

void do_ewpnusesprite(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkWeaponMiscSprite(ID) != SH::_NoError)
		return;
		
	if(EwpnH::loadWeapon(ri->ewpn) == SH::_NoError)
		EwpnH::getWeapon()->LOADGFX(ID);
}

void do_portalusesprite()
{
	int32_t ID = get_register(sarg1) / 10000;
	
	if(BC::checkWeaponMiscSprite(ID) != SH::_NoError)
		return;
	
	if(portal* p = checkPortal(ri->portalref))
		p->LOADGFX(ID);
}

void do_clearsprites(const bool v)
{
	int32_t spritelist = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkBounds(spritelist, 0, 5) != SH::_NoError)
		return;
		
	switch(spritelist)
	{
		case 0:
			guys.clear();
			break;
			
		case 1:
			items.clear();
			break;
			
		case 2:
			Ewpns.clear();
			break;
			
		case 3:
			Lwpns.clear();
			Hero.reset_hookshot();
			break;
			
		case 4:
			decorations.clear();
			break;
			
		case 5:
			particles.clear();
			break;
	}
}

void do_loadlweapon(const bool v)
{
	int32_t index = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkLWeaponIndex(index) != SH::_NoError)
		ri->lwpn = 0; //MAX_DWORD; //Now NULL
	else
	{
		ri->lwpn = Lwpns.spr(index)->getUID();
		// This is too trivial to log. -L
	}
}

void do_loadeweapon(const bool v)
{
	int32_t index = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkEWeaponIndex(index) != SH::_NoError)
		ri->ewpn = 0; //MAX_DWORD; //Now NULL
	else
	{
		ri->ewpn = Ewpns.spr(index)->getUID();
	}
}

void do_loaditem(const bool v)
{
	int32_t index = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkItemIndex(index) != SH::_NoError)
		ri->itemref = 0; //MAX_DWORD; //Now NULL
	else
	{
		ri->itemref = items.spr(index)->getUID();
	}
}


void do_loaditemdata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	//I *think* this is the right check ~Joe
	if(BC::checkItemID(ID) != SH::_NoError)
	{
		ri->idata = -1; //new null value
		return;
	}
	ri->idata = ID;
}

void do_loadnpc(const bool v)
{
	int32_t index = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkGuyIndex(index) != SH::_NoError)
		ri->guyref = 0; // MAX_DWORD;
	else
	{
		ri->guyref = guys.spr(index)->getUID();
	}
}

void FFScript::do_loaddmapdata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if ( ID < 0 || ID > 511 )
	{
		Z_scripterrlog("Invalid DMap ID passed to Game->LoadDMapData(): %d\n", ID);
		ri->dmapsref = MAX_DWORD;
	}
	else ri->dmapsref = ID;
}

void FFScript::do_load_active_subscreendata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if(ID == -1 || (unsigned(ID) < subscreens_active.size() && unsigned(ID) < 256))
	{
		ri->subdataref = get_subref(ID, sstACTIVE);
	}
	else
	{
		Z_scripterrlog("Invalid Subscreen ID passed to Game->LoadASubData(): %d\n", ID);
		ri->subdataref = 0;
	}
	ri->d[rEXP1] = ri->subdataref;
}
void FFScript::do_load_passive_subscreendata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if(ID == -1 || (unsigned(ID) < subscreens_passive.size() && unsigned(ID) < 256))
	{
		ri->subdataref = get_subref(ID, sstPASSIVE);
	}
	else
	{
		Z_scripterrlog("Invalid Subscreen ID passed to Game->LoadPSubData(): %d\n", ID);
		ri->subdataref = 0;
	}
	ri->d[rEXP1] = ri->subdataref;
}
void FFScript::do_load_overlay_subscreendata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if(ID == -1 || (unsigned(ID) < subscreens_overlay.size() && unsigned(ID) < 256))
	{
		ri->subdataref = get_subref(ID, sstOVERLAY);
	}
	else
	{
		Z_scripterrlog("Invalid Subscreen ID passed to Game->LoadOSubData(): %d\n", ID);
		ri->subdataref = 0;
	}
	ri->d[rEXP1] = ri->subdataref;
}
void FFScript::do_load_subscreendata(const bool v, const bool v2)
{
	int32_t ty = SH::get_arg(sarg2, v2) / 10000;
	switch(ty)
	{
		case sstACTIVE:
			do_load_active_subscreendata(v);
			break;
		case sstPASSIVE:
			do_load_passive_subscreendata(v);
			break;
		case sstOVERLAY:
			do_load_overlay_subscreendata(v);
			break;
		default:
		{
			Z_scripterrlog("Invalid Subscreen Type passed to ???: %d\n", ty);
			ri->subdataref = 0;
			break;
		}
	}
	ri->d[rEXP1] = ri->subdataref;
}

void FFScript::do_loadrng()
{
	auto rng = user_rngs.create();
	if (!rng)
	{
		ri->d[rEXP1] = 0;
		return;
	}

	int q = script_object_ids_by_type[script_object_type::rng].size() - 1;
	rng->gen = &script_rnggens[q];
	ri->rngref = rng->id;
	ri->d[rEXP1] = rng->id;
}

void FFScript::do_loaddirectory()
{
	int32_t arrayptr = get_register(sarg1);
	string user_path;
	ArrayH::getString(arrayptr, user_path, 2048);

	std::string resolved_path;
	if (auto r = parse_user_path(user_path, false); !r)
	{
		scripting_log_error_with_context("Error: {}", r.error());
		return;
	} else resolved_path = r.value();

	if (checkPath(resolved_path.c_str(), true))
	{
		ri->directoryref = user_dirs.get_free();
		if(!ri->directoryref) return;
		user_dir* d = checkDir(ri->directoryref, true);
		set_register(sarg1, ri->directoryref);
		d->setPath(resolved_path.c_str());
		return;
	}

	scripting_log_error_with_context("Path '{}' points to a file; must point to a directory!", resolved_path);
	ri->directoryref = 0;
	set_register(sarg1, 0);
}

void FFScript::do_loadstack()
{
	ri->stackref = user_stacks.get_free();
	ri->d[rEXP1] = ri->stackref;
}

void FFScript::do_loaddropset(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if ( ID < 0 || ID > MAXITEMDROPSETS )
	{
		scripting_log_error_with_context("Invalid Dropset ID: {}", ID);
		ri->dropsetref = MAX_DWORD;
	}
		
	else ri->dropsetref = ID;
}

void FFScript::do_loadbottle(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if ( ID < 1 || ID > 64 )
	{
		scripting_log_error_with_context("Invalid BottleType ID: {}", ID);
		ri->bottletyperef = 0;
	}
	else ri->bottletyperef = ID;
}

void FFScript::do_loadbottleshop(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;

	if ( ID < 0 || ID > 255 )
	{
		scripting_log_error_with_context("Invalid BottleShopType ID: {}", ID);
		ri->bottleshopref = 0;
	}
	else ri->bottleshopref = ID+1;
}
void FFScript::do_loadgenericdata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if ( ID < 1 || ID > NUMSCRIPTSGENERIC )
	{
		scripting_log_error_with_context("Invalid GenericData ID: {}", ID);
		ri->genericdataref = 0;
	}
	else ri->genericdataref = ID;
}

void FFScript::do_create_paldata()
{
	ri->paldataref = user_paldatas.get_free();
	ri->d[rEXP1] = ri->paldataref;
}

void FFScript::do_create_paldata_clr()
{
	ri->paldataref = user_paldatas.get_free();
	if (ri->paldataref > 0)
	{
		user_paldata& pd = user_paldatas[ri->paldataref];
		int32_t clri = get_register(sarg1);

		RGB c = _RGB((clri >> 16) & 0xFF, (clri >> 8) & 0xFF, clri & 0xFF);

		c.r = vbound(c.r, 0, scripting_max_color_val);
		c.g = vbound(c.g, 0, scripting_max_color_val);
		c.b = vbound(c.b, 0, scripting_max_color_val);

		for (int32_t q = 0; q < 240; ++q)
			pd.set_color(q, c);
	}
	ri->d[rEXP1] = ri->paldataref;
}

void FFScript::do_mix_clr()
{
	int32_t clr_start = SH::read_stack(ri->sp + 3);
	int32_t clr_end = SH::read_stack(ri->sp + 2);
	float percent = SH::read_stack(ri->sp + 1) / 10000.0;
	int32_t color_space = SH::read_stack(ri->sp + 0) / 10000;

	RGB ref1c = _RGB((clr_start >> 16) & 0xFF, (clr_start >> 8) & 0xFF, clr_start & 0xFF);
	RGB ref2c = _RGB((clr_end >> 16) & 0xFF, (clr_end >> 8) & 0xFF, clr_end & 0xFF);
	RGB outputc = user_paldata::mix_color(ref1c, ref2c, percent, color_space);

	int32_t r = vbound(outputc.r, 0, scripting_max_color_val);
	int32_t g = vbound(outputc.g, 0, scripting_max_color_val);
	int32_t b = vbound(outputc.b, 0, scripting_max_color_val);

	ri->d[rEXP1] = (r << 16) | (g << 8) | b;
}

void FFScript::do_create_rgb_hex()
{
	int32_t hexrgb = get_register(sarg1);

	int32_t r = (hexrgb >> 16) & 0xFF;
	int32_t g = (hexrgb >> 8) & 0xFF;
	int32_t b = hexrgb & 0xFF;

	if (scripting_use_8bit_colors)
	{
		r = vbound(r, 0, 255);
		g = vbound(g, 0, 255);
		b = vbound(b, 0, 255);
	}
	else
	{
		r = vbound(r / 4, 0, 63);
		g = vbound(g / 4, 0, 63);
		b = vbound(b / 4, 0, 63);
	}

	ri->d[rEXP1] = (r << 16) | (g << 8) | b;
}

void FFScript::do_create_rgb()
{
	int32_t r = SH::read_stack(ri->sp + 2) / 10000;
	int32_t g = SH::read_stack(ri->sp + 1) / 10000;
	int32_t b = SH::read_stack(ri->sp + 0) / 10000;

	int max_value = scripting_max_color_val;
	if (unsigned(r) > max_value || unsigned(g) > max_value || unsigned(b) > max_value)
	{
		scripting_log_error_with_context("R/G/B values should range from 0-{}, got: {} {} {}", max_value, r, g, b);
	}

	r = vbound(r, 0, max_value);
	g = vbound(g, 0, max_value);
	b = vbound(b, 0, max_value);

	ri->d[rEXP1] = (r << 16) | (g << 8) | b;
}

void FFScript::do_convert_from_rgb()
{
	int32_t buf = SH::read_stack(ri->sp + 2) / 10000;
	int32_t clri = SH::read_stack(ri->sp + 1);
	int32_t color_space = SH::read_stack(ri->sp + 0) / 10000;

	ArrayManager am(buf);
	if (am.invalid()) return;
	int32_t zscript_array_size = am.size();
	int32_t target_size;
	
	switch (color_space)
	{
	case user_paldata::CSPACE_CMYK:
			target_size = 4;
			break;
		default:
			target_size = 3;
	}

	if (zscript_array_size < target_size)
	{
		scripting_log_error_with_context("Array not large enough. Should be at least size {}", target_size);
		return;
	}
	
	RGB c = _RGB((clri >> 16) & 0xFF, (clri >> 8) & 0xFF, clri & 0xFF);
	double convert[4];
	user_paldata::RGBTo(c, convert, color_space);

	for (int32_t q = 0; q < target_size; ++q)
	{
		am.set(q, int32_t(convert[q]*10000));
	}

	return;
}

void FFScript::do_convert_to_rgb()
{
	int32_t buf = SH::read_stack(ri->sp + 1) / 10000;
	int32_t color_space = SH::read_stack(ri->sp + 0) / 10000;
	
	ArrayManager am(buf);
	if (am.invalid()) return;
	int32_t zscript_array_size = am.size();
	int32_t target_size;

	switch (color_space)
	{
	case user_paldata::CSPACE_CMYK:
		target_size = 4;
		break;
	default:
		target_size = 3;
	}

	if (zscript_array_size < target_size)
	{
		scripting_log_error_with_context("Array not large enough. Should be at least size {}", target_size);
		return;
	}

	double convert[4];
	for (int32_t q = 0; q < target_size; ++q)
	{
		convert[q] = am.get(q) / 10000.0;
	}
	RGB c = user_paldata::RGBFrom(convert, color_space);

	ri->d[rEXP1] = (c.r << 16) | (c.g << 8) | c.b;
}

void FFScript::do_paldata_load_level()
{
	if (user_paldata* pd = checkPalData(ri->paldataref))
	{
		int32_t lvl = get_register(sarg1) / 10000;
		//Load CSets 2-4
		pd->load_cset(2, lvl * pdLEVEL + poLEVEL + 0);
		pd->load_cset(3, lvl * pdLEVEL + poLEVEL + 1);
		pd->load_cset(4, lvl * pdLEVEL + poLEVEL + 2);
		//Load CSet 9
		pd->load_cset(9, lvl * pdLEVEL + poLEVEL + 3);
		//Load 1, 5, 7, 8
		pd->load_cset(1, lvl * pdLEVEL + poNEWCSETS);
		pd->load_cset(5, lvl * pdLEVEL + poNEWCSETS + 1);
		pd->load_cset(7, lvl * pdLEVEL + poNEWCSETS + 2);
		pd->load_cset(8, lvl * pdLEVEL + poNEWCSETS + 3);
	}
	return;
}

void FFScript::do_paldata_load_sprite()
{
	if (user_paldata* pd = checkPalData(ri->paldataref))
	{
		int32_t page = get_register(sarg1) / 10000;

		int32_t pageoffset = 0;
		switch (page)
		{
		case 0: pageoffset += 0;  break;
		case 1: pageoffset += 15; break;
		default:
			scripting_log_error_with_context("Invalid page: {}. Valid pages are 0 or 1. Aborting.", page);
			return;
		}
		for (int32_t q = 0; q < 15; ++q)
		{
			pd->load_cset(q, poSPRITE255 + pageoffset + q);
		}
	}
	return;
}

void FFScript::do_paldata_load_main()
{
	if (user_paldata* pd = checkPalData(ri->paldataref))
	{
		for (int32_t q = 0; q <= 15; ++q)
		{
			pd->load_cset_main(q);
		}
	}
	return;
}

void FFScript::do_paldata_load_cycle()
{
	if (user_paldata* pd = checkPalData(ri->paldataref))
	{
		int32_t lvl = get_register(sarg1) / 10000;
		for (int32_t q = 4; q <= 12; ++q)
		{
			pd->load_cset(q, lvl * pdLEVEL + poLEVEL + q);
		}
	}
	return;
}

void FFScript::do_paldata_load_bitmap()
{
	if (user_paldata* pd = checkPalData(ri->paldataref))
	{
		int32_t pathptr = get_register(sarg1);
		string user_path, str;
		ArrayH::getString(pathptr, user_path, 256);

		if (get_qr(qr_BITMAP_AND_FILESYSTEM_PATHS_ALWAYS_RELATIVE))
		{
			if (auto r = parse_user_path(user_path, true); !r)
			{
				scripting_log_error_with_context("Error: {}", r.error());
				return;
			} else str = r.value();
		}
		else
		{
			str = user_path;
			regulate_path(str);
		}

		if (str.empty())
		{
			al_trace("String pointer is null! Internal error. \n");
			return;
		}

		PALETTE tempPal;
		get_palette(tempPal);
		if (checkPath(str.c_str(), false))
		{
			BITMAP* bmp = load_bitmap(str.c_str(), tempPal);
			if (!bmp)
			{
				Z_scripterrlog("LoadBitmapPalette() failed to load image file %s.\n", str.c_str());
			}
			else
			{
				for (int32_t q = 0; q < PALDATA_NUM_COLORS; ++q)
				{
					pd->colors[q] = tempPal[q];
					set_bit(pd->colors_used, q, true);
				}
			}
			destroy_bitmap(bmp);
		}
		else
		{
			Z_scripterrlog("Failed to load image file: %s. File not found.\n", str.c_str());
		}
	}
	return;
}

void FFScript::do_paldata_write_level()
{
	if (user_paldata* pd = checkPalData(ri->paldataref))
	{
		int32_t lvl = get_register(sarg1) / 10000;
		bool changed = false;
		//Write CSets 2-4
		if (pd->check_cset(2, lvl * pdLEVEL + poLEVEL + 0))
		{
			pd->write_cset(2, lvl * pdLEVEL + poLEVEL + 0);
			changed = true;
		}
		if (pd->check_cset(3, lvl * pdLEVEL + poLEVEL + 1))
		{
			pd->write_cset(3, lvl * pdLEVEL + poLEVEL + 1);
			changed = true;
		}
		if (pd->check_cset(4, lvl * pdLEVEL + poLEVEL + 2))
		{
			pd->write_cset(4, lvl * pdLEVEL + poLEVEL + 2);
			changed = true;
		}
		//Write CSet 9
		if (pd->check_cset(9, lvl * pdLEVEL + poLEVEL + 3))
		{
			pd->write_cset(9, lvl * pdLEVEL + poLEVEL + 3);
			changed = true;
		}
		//Write 1, 5, 7, 8
		if (pd->check_cset(1, lvl * pdLEVEL + poNEWCSETS + 0))
		{
			pd->write_cset(1, lvl * pdLEVEL + poNEWCSETS + 0);
			changed = true;
		}
		if (pd->check_cset(5, lvl * pdLEVEL + poNEWCSETS + 1))
		{
			pd->write_cset(5, lvl * pdLEVEL + poNEWCSETS + 1);
			changed = true;
		}
		if (pd->check_cset(7, lvl * pdLEVEL + poNEWCSETS + 2))
		{
			pd->write_cset(7, lvl * pdLEVEL + poNEWCSETS + 2);
			changed = true;
		}
		if (pd->check_cset(8, lvl * pdLEVEL + poNEWCSETS + 3))
		{
			pd->write_cset(8, lvl * pdLEVEL + poNEWCSETS + 3);
			changed = true;
		}

		if (changed && DMaps[cur_dmap].color == lvl)
		{
			loadlvlpal(lvl);
			currcset = lvl;
			if (darkroom && !get_qr(qr_NEW_DARKROOM))
			{
				if (get_qr(qr_FADE))
				{
					interpolatedfade();
				}
				else
				{
					loadfadepal((DMaps[cur_dmap].color) * pdLEVEL + poFADE3);
				}
			}
		}
	}
	return;
}

void FFScript::do_paldata_write_levelcset()
{
	if (user_paldata* pd = checkPalData(ri->paldataref))
	{
		int32_t lvl = get_register(sarg1) / 10000;
		int32_t cs = get_register(sarg2) / 10000;

		bool changed = false;

		switch (cs)
		{
		case 1:
			if (pd->check_cset(1, lvl * pdLEVEL + poNEWCSETS + 0))
			{
				pd->write_cset(1, lvl * pdLEVEL + poNEWCSETS + 0);
				changed = true;
			}
			break;
		case 2:
			if (pd->check_cset(2, lvl * pdLEVEL + poLEVEL + 0))
			{
				pd->write_cset(2, lvl * pdLEVEL + poLEVEL + 0);
				changed = true;
			}
			break;
		case 3:
			if (pd->check_cset(3, lvl * pdLEVEL + poLEVEL + 1))
			{
				pd->write_cset(3, lvl * pdLEVEL + poLEVEL + 1);
				changed = true;
			}
			break;
		case 4:
			if (pd->check_cset(4, lvl * pdLEVEL + poLEVEL + 2))
			{
				pd->write_cset(4, lvl * pdLEVEL + poLEVEL + 2);
				changed = true;
			}
			break;
		case 5:
			if (pd->check_cset(5, lvl * pdLEVEL + poNEWCSETS + 1))
			{
				pd->write_cset(5, lvl * pdLEVEL + poNEWCSETS + 1);
				changed = true;
			}
			break;
		case 7:
			if (pd->check_cset(7, lvl * pdLEVEL + poNEWCSETS + 2))
			{
				pd->write_cset(7, lvl * pdLEVEL + poNEWCSETS + 2);
				changed = true;
			}
			break;
		case 8:
			if (pd->check_cset(8, lvl * pdLEVEL + poNEWCSETS + 3))
			{
				pd->write_cset(8, lvl * pdLEVEL + poNEWCSETS + 3);
				changed = true;
			}
			break;
		case 9:
			if (pd->check_cset(9, lvl * pdLEVEL + poLEVEL + 3))
			{
				pd->write_cset(9, lvl * pdLEVEL + poLEVEL + 3);
				changed = true;
			}
			break;
		default:
			Z_scripterrlog("Invalid CSet (%d) passed to 'paldata->WriteLevelCSet()'. Level palettes can use CSets 1, 2, 3, 4, 5, 7, 8, 9.\n", cs);
			return;
		}
	
		if (changed && DMaps[cur_dmap].color == lvl)
		{
			loadlvlpal(lvl);
			if (darkroom && !get_qr(qr_NEW_DARKROOM))
			{
				if (get_qr(qr_FADE))
				{
					interpolatedfade();
				}
				else
				{
					loadfadepal((DMaps[cur_dmap].color) * pdLEVEL + poFADE3);
				}
			}
			currcset = lvl;
		}
	}
}

void FFScript::do_paldata_write_sprite()
{
	if (user_paldata* pd = checkPalData(ri->paldataref))
	{
		int32_t page = get_register(sarg1) / 10000;

		int32_t pageoffset = 0;
		switch (page)
		{
		case 0: pageoffset += 0;  break;
		case 1: pageoffset += 15; break;
		default:
			Z_scripterrlog("Invalid page (%d) passed to paldata->WriteSpritePalette(). Valid pages are 0 or 1. Aborting.\n", page);
			return;
		}
		bool changed6 = false;
		bool changed14 = false;
		for (int32_t q = 0; q < 15; ++q)
		{
			if (pd->check_cset(q, poSPRITE255 + pageoffset + q))
			{
				pd->write_cset(q, poSPRITE255 + pageoffset + q);
				if (pageoffset + q == currspal6)
				{
					changed6 = true;
				}
				if (pageoffset + q == currspal14)
				{
					changed14 = true;
				}
			}
		}

		//If either sprite palette has been changed, update the main palette
		if (changed6 || changed14)
		{
			if (changed6) 
			{
				loadpalset(6, poSPRITE255 + currspal6, false);
			}
			if (changed14)
			{
				loadpalset(14, poSPRITE255 + currspal14, false);
			}

			if (isUserTinted()) {
				restoreTint();
			}
		}
	}
	return;
}

void FFScript::do_paldata_write_spritecset()
{
	if (user_paldata* pd = checkPalData(ri->paldataref))
	{
		int32_t page = get_register(sarg1) / 10000;
		int32_t cs = get_register(sarg2) / 10000;

		int32_t pageoffset = 0;
		switch (page)
		{
		case 0: pageoffset += 0;  break;
		case 1: pageoffset += 15; break;
		default:
			Z_scripterrlog("Invalid page (%d) passed to paldata->WriteSpriteCSet(). Valid pages are 0 or 1. Aborting.\n", page);
			return;
		}
		bool changed6 = false;
		bool changed14 = false;
		if (unsigned(cs) > 15)
		{
			Z_scripterrlog("Invalid CSet (%d) passed to paldata->WriteSpriteCSet(). Valid CSets are 0-15. Aborting.\n", cs);
			return;
		}
		if (pd->check_cset(cs, poSPRITE255 + pageoffset + cs))
		{
			pd->write_cset(cs, poSPRITE255 + pageoffset + cs);
			if (pageoffset + cs == currspal6)
			{
				changed6 = true;
			}
			if (pageoffset + cs == currspal14)
			{
				changed14 = true;
			}
		}

		//If either sprite palette has been changed, update the main palette
		if (changed6 || changed14)
		{
			if (changed6)
			{
				loadpalset(6, poSPRITE255 + currspal6, false);
			}
			if (changed14)
			{
				loadpalset(14, poSPRITE255 + currspal14, false);
			}

			if (isUserTinted()) {
				restoreTint();
			}
		}
	}
	return;
}

void FFScript::do_paldata_write_main()
{
	if (user_paldata* pd = checkPalData(ri->paldataref))
	{
		bool changed = false;
		for (int32_t q = 0; q <= 15; ++q)
		{
			if (pd->check_cset_main(q))
			{
				pd->write_cset_main(q);
				changed = true;
			}
		}

		if (changed)
		{
			refreshpal = true;
		}
	}
	return;
}

void FFScript::do_paldata_write_maincset()
{
	if (user_paldata* pd = checkPalData(ri->paldataref))
	{
		int32_t cs = get_register(sarg1) / 10000;

		bool changed = false;

		if (unsigned(cs) < 16)
		{
			if (pd->check_cset_main(cs))
			{
				pd->write_cset_main(cs);
				changed = true;
			}
		}
		else
		{
			Z_scripterrlog("Invalid CSet (%d) passed to 'paldata->WriteMainCSet()'. Valid csets are 0-15. Aborting.\n", cs);
			return;
		}

		if (changed) 
		{
			refreshpal = true;
		}
	}
}

void FFScript::do_paldata_write_cycle()
{
	if (user_paldata* pd = checkPalData(ri->paldataref))
	{
		int32_t lvl = get_register(sarg1) / 10000;
		for (int32_t q = 4; q <= 12; ++q)
		{
			if (pd->check_cset(q, lvl * pdLEVEL + poLEVEL + q))
			{
				pd->write_cset(q, lvl * pdLEVEL + poLEVEL + q);
			}
		}
	}
	return;
}

void FFScript::do_paldata_write_cyclecset()
{
	if (user_paldata* pd = checkPalData(ri->paldataref))
	{
		int32_t lvl = get_register(sarg1) / 10000;
		int32_t cs = get_register(sarg2) / 10000;

		bool changed = false;

		switch (cs)
		{
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
			if (pd->check_cset(cs, lvl * pdLEVEL + poLEVEL + cs))
			{
				pd->write_cset(cs, lvl * pdLEVEL + poLEVEL + cs);
				changed = true;
			}
			break;
		default:
			Z_scripterrlog("Invalid CSet (%d) passed to 'paldata->WriteCycleCSet()'. Cycle palettes use CSets 4-12.\n", cs);
			return;
		}

		if (changed && DMaps[cur_dmap].color == lvl)
		{
			loadlvlpal(lvl);
			currcset = lvl;
		}
	}
}

void FFScript::do_paldata_colorvalid()
{
	if (user_paldata* pd = checkPalData(ri->paldataref))
	{
		int32_t ind = get_register(sarg1) / 10000;
		if (unsigned(ind) >= PALDATA_NUM_COLORS)
		{
			Z_scripterrlog("Invalid color index (%d) passed to paldata->ColorValid(). Valid indices are 0-255.\n", ind);
			set_register(sarg1, 0);
			return;
		}

		if (get_bit(pd->colors_used, ind))
		{
			set_register(sarg1, 10000);
		}
		else
		{
			set_register(sarg1, 0);
		}
	}
}

void FFScript::do_paldata_clearcolor()
{
	if (user_paldata* pd = checkPalData(ri->paldataref))
	{
		int32_t ind = get_register(sarg1) / 10000;
		if (unsigned(ind) >= PALDATA_NUM_COLORS)
		{
			Z_scripterrlog("Invalid color index (%d) passed to paldata->ClearColor(). Valid indices are 0-255. Aborting.\n", ind);
			return;
		}
		set_bit(pd->colors_used, ind, false);
	}
}

void FFScript::do_paldata_clearcset()
{
	if (user_paldata* pd = checkPalData(ri->paldataref))
	{
		int32_t cs = get_register(sarg1) / 10000;
		if (unsigned(cs) > 15)
		{
			Z_scripterrlog("Invalid cset (%d) passed to paldata->ClearCSet(). Valid csets are 0-15. Aborting.\n", cs);
			return;
		}
		for (int32_t q = 0; q < 16; ++q)
		{
			set_bit(pd->colors_used, CSET(cs) + q, false);
		}
	}
}

int32_t FFScript::do_paldata_getrgb(user_paldata* pd, int32_t index, int32_t c)
{
	if (pd)
	{
		int32_t ind = index;
		if (unsigned(ind) >= PALDATA_NUM_COLORS)
		{
			scripting_log_error_with_context("Invalid color index ({}). Valid indices are 0-255.", ind);
			return -10000;
		}
		if (!get_bit(pd->colors_used, ind))
		{
			scripting_log_error_with_context("Tried to access unused color {}.", ind);
			return -10000;
		}
		switch (c)
		{
			case 0:
				return pd->colors[ind].r * 10000;
			case 1:
				return pd->colors[ind].g * 10000;
			case 2:
				return pd->colors[ind].b * 10000;
		}
	}

	return -10000;
}

void FFScript::do_paldata_setrgb(user_paldata* pd, int32_t index, int32_t val, int32_t c)
{
	if (pd)
	{
		int32_t ind = index;
		if (unsigned(ind) >= PALDATA_NUM_COLORS)
		{
			scripting_log_error_with_context("Invalid color index ({}). Valid indices are 0-255. Aborting.", ind);
			return;
		}
		if (unsigned(val) > scripting_max_color_val)
		{
			scripting_log_error_with_context("RGB value({}) is out of range. RGB values range from 0 - {}.", val, scripting_max_color_val);
			val = vbound(val, 0, scripting_max_color_val);
		}
		if (!get_bit(pd->colors_used, ind))
		{
			scripting_log_error_with_context("Tried to access unused color {}.", ind);
			return;
		}
		switch (c)
		{
			case 0:
				pd->colors[ind].r = val;
				break;
			case 1:
				pd->colors[ind].g = val;
				break;
			case 2:
				pd->colors[ind].b = val;
				break;
		}
	}
}

void FFScript::do_paldata_mix()
{
	int32_t ref = SH::read_stack(ri->sp + 4);
	if (user_paldata* pd = checkPalData(ref))
	{
		int32_t ref1 = SH::read_stack(ri->sp + 3);
		int32_t ref2 = SH::read_stack(ri->sp + 2);
		double percent = SH::read_stack(ri->sp + 1)/10000.0;
		int32_t color_space = SH::read_stack(ri->sp + 0)/10000;
		if (user_paldata* pd_start = checkPalData(ref1))
		{
			if (user_paldata* pd_end = checkPalData(ref2))
			{
				pd->mix(pd_start, pd_end, percent, color_space);
			}
		}
	}
}

void FFScript::do_paldata_mixcset()
{
	int32_t ref = SH::read_stack(ri->sp + 5);
	if (user_paldata* pd = checkPalData(ref))
	{
		int32_t ref1 = SH::read_stack(ri->sp + 4);
		int32_t ref2 = SH::read_stack(ri->sp + 3);
		int32_t cset = SH::read_stack(ri->sp + 2) / 10000;
		double percent = SH::read_stack(ri->sp + 1) / 10000.0;
		int32_t color_space = SH::read_stack(ri->sp + 0) / 10000;
		if (user_paldata* pd_start = checkPalData(ref1))
		{
			if (user_paldata* pd_end = checkPalData(ref2))
			{
				if (unsigned(cset) > 15)
				{
					Z_scripterrlog("CSet passed to 'paldata->MixCSet()' out of range. Valid CSets are 0-15\n");
					return;
				}
				pd->mix(pd_start, pd_end, percent, color_space, CSET(cset), CSET(cset) + 16);
			}
		}
	}
}

void FFScript::do_paldata_copy()
{
	if (user_paldata* pd = checkPalData(ri->paldataref))
	{
		int32_t ref_dest = get_register(sarg1);
		if (user_paldata* pd_dest = checkPalData(ref_dest))
		{
			for (int32_t q = 0; q < PALDATA_NUM_COLORS; ++q)
			{
				pd_dest->colors[q] = pd->colors[q];
			}
			for (int32_t q = 0; q < PALDATA_BITSTREAM_SIZE; ++q)
			{
				pd_dest->colors_used[q] = pd->colors_used[q];
			}
		}
	}
}

void FFScript::do_paldata_copycset()
{
	ri->paldataref = SH::read_stack(ri->sp + 3);
	if (user_paldata* pd = checkPalData(ri->paldataref))
	{
		int32_t ref_dest = SH::read_stack(ri->sp + 2);
		int32_t cs = SH::read_stack(ri->sp + 1) / 10000;
		int32_t cs_dest = SH::read_stack(ri->sp + 0) / 10000;
		if (user_paldata* pd_dest = checkPalData(ref_dest))
		{
			if (unsigned(cs) > 15)
			{
				Z_scripterrlog("Invalid CSet (%d) passed to paldata->CopyCSet(). Valid CSets are 0-15. Aborting.\n", cs);
				return;
			}
			for (int32_t q = 0; q < 16; ++q)
			{
				pd_dest->colors[CSET(cs_dest) + q] = pd->colors[CSET(cs) + q];
				set_bit(pd_dest->colors_used, CSET(cs_dest) + q, bool(get_bit(pd->colors_used, CSET(cs) + q)));
			}
		}
	}
}

//Loads a cset to paldata from memory
void user_paldata::load_cset(int32_t cset, int32_t dataset)
{
	byte* si = colordata + CSET(dataset) * 3;
	for (int32_t q = 0; q < 16; ++q)
	{
		int32_t ind = CSET(cset) + q;
		colors[ind].r = scripting_read_pal_color(si[0]);
		colors[ind].g = scripting_read_pal_color(si[1]);
		colors[ind].b = scripting_read_pal_color(si[2]);
		set_bit(colors_used, ind, true);
		si += 3;
	}
}

//Loads a cset to paldata from the main palette
void user_paldata::load_cset_main(int32_t cset)
{
	for (int32_t q = 0; q < 16; ++q)
	{
		int32_t ind = CSET(cset) + q;
		colors[ind].r = scripting_read_pal_color(RAMpal[ind].r);
		colors[ind].g = scripting_read_pal_color(RAMpal[ind].g);
		colors[ind].b = scripting_read_pal_color(RAMpal[ind].b);
		set_bit(colors_used, ind, true);
	}
}

//Writes to a memory cset from paldata
void user_paldata::write_cset(int32_t cset, int32_t dataset)
{
	byte* si = colordata + CSET(dataset) * 3;
	for (int32_t q = 0; q < 16; ++q)
	{
		int32_t ind = CSET(cset) + q;
		if (get_bit(colors_used, ind))
		{
			si[0] = scripting_write_pal_color(colors[ind].r);
			si[1] = scripting_write_pal_color(colors[ind].g);
			si[2] = scripting_write_pal_color(colors[ind].b);
		}
		si += 3;
	}
}

//Writes to a main palette cset from paldata
void user_paldata::write_cset_main(int32_t cset)
{
	for (int32_t q = 0; q < 16; ++q)
	{
		int32_t ind = CSET(cset) + q;
		if (get_bit(colors_used, ind))
		{
			RAMpal[ind] = colors[ind];
			if (!scripting_use_8bit_colors)
				convertRGB(RAMpal[ind]);
		}
	}
}


//Checks a memory cset from 




bool user_paldata::check_cset(int32_t cset, int32_t dataset)
{
	byte* si = colordata + CSET(dataset) * 3;
	for (int32_t q = 0; q < 16; ++q)
	{
		int32_t ind = CSET(cset) + q;
		if (get_bit(colors_used, ind))
		{
			if (scripting_read_pal_color(si[0]) != colors[ind].r)
				return true;
			if (scripting_read_pal_color(si[1]) != colors[ind].g)
				return true;
			if (scripting_read_pal_color(si[2]) != colors[ind].b)
				return true;
		}
		si += 3;
	}
	return false;
}

//Checks a memory cset from the main palette
bool user_paldata::check_cset_main(int32_t cset)
{
	for (int32_t q = 0; q < 16; ++q)
	{
		int32_t ind = CSET(cset) + q;
		if (get_bit(colors_used, ind))
		{
			if (scripting_read_pal_color(RAMpal[ind].r) != colors[ind].r)
				return true;
			if (scripting_read_pal_color(RAMpal[ind].g) != colors[ind].g)
				return true;
			if (scripting_read_pal_color(RAMpal[ind].b) != colors[ind].b)
				return true;
		}
	}
	return false;
}

//Mixes a color between two paldatas
RGB user_paldata::mix_color(RGB start, RGB end, double percent, int32_t color_space)
{
	double upper = scripting_max_color_val;
	int32_t direction = 0;
	switch (color_space)
	{
	case CSPACE_RGB:
		return _RGB(byte(vbound(double(zc::math::Lerp(start.r, end.r, percent)), 0.0, upper)),
			byte(vbound(double(zc::math::Lerp(start.g, end.g, percent)), 0.0, upper)),
			byte(vbound(double(zc::math::Lerp(start.b, end.b, percent)), 0.0, upper)));
	case CSPACE_CMYK:
	{
		double convert_start[4];
		double convert_end[4];
		double convert_result[4];
		RGBTo(start, convert_start, color_space);
		RGBTo(end, convert_end, color_space);
		convert_result[0] = zc::math::Lerp(convert_start[0], convert_end[0], percent);
		convert_result[1] = zc::math::Lerp(convert_start[1], convert_end[1], percent);
		convert_result[2] = zc::math::Lerp(convert_start[2], convert_end[2], percent);
		convert_result[3] = zc::math::Lerp(convert_start[3], convert_end[3], percent);
		return RGBFrom(convert_result, color_space);
	}
	case CSPACE_HSV_CW:
		if (color_space == CSPACE_HSV_CW)
			direction = 1;
		[[fallthrough]];
	case CSPACE_HSV_CCW:
		if (color_space == CSPACE_HSV_CCW)
			direction = -1;
		[[fallthrough]];
	case CSPACE_HSV:
	{
		double convert_start[3];
		double convert_end[3];
		double convert_result[3];
		RGBTo(start, convert_start, color_space);
		RGBTo(end, convert_end, color_space);
		convert_result[0] = WrapLerp(convert_start[0], convert_end[0], percent, 0.0, 1.0, direction);
		convert_result[1] = zc::math::Lerp(convert_start[1], convert_end[1], percent);
		convert_result[2] = zc::math::Lerp(convert_start[2], convert_end[2], percent);
		return RGBFrom(convert_result, color_space);
	}
	case CSPACE_HSL_CW:
		if (color_space == CSPACE_HSL_CW)
			direction = 1;
		[[fallthrough]];
	case CSPACE_HSL_CCW:
		if (color_space == CSPACE_HSL_CCW)
			direction = -1;
		[[fallthrough]];
	case CSPACE_HSL:
	{
		double convert_start[3];
		double convert_end[3];
		double convert_result[3];
		RGBTo(start, convert_start, color_space);
		RGBTo(end, convert_end, color_space);
		convert_result[0] = WrapLerp(convert_start[0], convert_end[0], percent, 0.0, 1.0, direction);
		convert_result[1] = zc::math::Lerp(convert_start[1], convert_end[1], percent);
		convert_result[2] = zc::math::Lerp(convert_start[2], convert_end[2], percent);
		return RGBFrom(convert_result, color_space);
	}
	case CSPACE_LAB:
	{
		double convert_start[3];
		double convert_end[3];
		double convert_result[3];
		RGBTo(start, convert_start, color_space);
		RGBTo(end, convert_end, color_space);
		convert_result[0] = zc::math::Lerp(convert_start[0], convert_end[0], percent);
		convert_result[1] = zc::math::Lerp(convert_start[1], convert_end[1], percent);
		convert_result[2] = zc::math::Lerp(convert_start[2], convert_end[2], percent);
		return RGBFrom(convert_result, color_space);
	}
	case CSPACE_LCH_CW:
		if (color_space == CSPACE_LCH_CW)
			direction = 1;
		[[fallthrough]];
	case CSPACE_LCH_CCW:
		if (color_space == CSPACE_LCH_CCW)
			direction = -1;
		[[fallthrough]];
	case CSPACE_LCH:
	{
		double convert_start[3];
		double convert_end[3];
		double convert_result[3];
		RGBTo(start, convert_start, color_space);
		RGBTo(end, convert_end, color_space);
		convert_result[0] = zc::math::Lerp(convert_start[0], convert_end[0], percent);
		convert_result[1] = zc::math::Lerp(convert_start[1], convert_end[1], percent);
		convert_result[2] = WrapLerp(convert_start[2], convert_end[2], percent, 0.0, 360.0, direction);
		return RGBFrom(convert_result, color_space);
	}
	}
	return start;
}

void user_paldata::RGBTo(RGB c, double arr[], int32_t color_space)
{
	//From easyrgb.com/en/math.php
	double upper = scripting_max_color_val;
	double r = vbound(c.r / upper, 0.0, 1.0);
	double g = vbound(c.g / upper, 0.0, 1.0);
	double b = vbound(c.b / upper, 0.0, 1.0);
	switch (color_space)
	{
	case CSPACE_CMYK:
	{
		double c = 1.0 - r;
		double m = 1.0 - g;
		double y = 1.0 - b;

		double k = 1.0;

		if (c < k) k = c;
		if (m < k) k = m;
		if (y < k) k = y;
		if (k == 1)
		{
			c = 0.0;
			m = 0.0;
			y = 0.0;
		}
		else
		{
			c = (c - k) / (1.0 - k);
			m = (m - k) / (1.0 - k);
			y = (y - k) / (1.0 - k);
		}
		arr[0] = c;
		arr[1] = m;
		arr[2] = y;
		arr[3] = k;
		break;
	}
	case CSPACE_HSV_CW:
	case CSPACE_HSV_CCW:
	case CSPACE_HSV:
	{
		double min_val = std::min(std::min(r, g), b);
		double max_val = std::max(std::max(r, g), b);
		double del_max = max_val - min_val;

		double h = 0;
		double s = 0;
		double v = max_val;

		if (del_max != 0) //Set chroma if not gray
		{
			s = del_max / max_val;

			double del_r = (((max_val - r) / 6.0) + (del_max / 2.0)) / del_max;
			double del_g = (((max_val - g) / 6.0) + (del_max / 2.0)) / del_max;
			double del_b = (((max_val - b) / 6.0) + (del_max / 2.0)) / del_max;

			if (r == max_val) h = del_b - del_g;
			else if (g == max_val) h = (1.0 / 3.0) + del_r - del_b;
			else if (b == max_val) h = (2.0 / 3.0) + del_g - del_r;

			if (h < 0) ++h;
			if (h > 1) --h;
		}

		arr[0] = h;
		arr[1] = s;
		arr[2] = v;
		break;
	}
	case CSPACE_HSL_CW:
	case CSPACE_HSL_CCW:
	case CSPACE_HSL:
	{
		double min_val = std::min(std::min(r, g), b);
		double max_val = std::max(std::max(r, g), b);
		double del_max = max_val - min_val;

		double h = 0;
		double s = 0;
		double l = (max_val + min_val) / 2.0;

		if (del_max != 0) //Set chroma if not gray
		{
			if (l < 0.5) s = del_max / (max_val + min_val);
			else s = del_max / (2 - max_val - min_val);

			double del_r = (((max_val - r) / 6.0) + (del_max / 2.0)) / del_max;
			double del_g = (((max_val - g) / 6.0) + (del_max / 2.0)) / del_max;
			double del_b = (((max_val - b) / 6.0) + (del_max / 2.0)) / del_max;

			if (r == max_val) h = del_b - del_g;
			else if (g == max_val) h = (1.0 / 3.0) + del_r - del_b;
			else if (b == max_val) h = (2.0 / 3.0) + del_g - del_r;

			if (h < 0) ++h;
			if (h > 1) --h;
		}

		arr[0] = h;
		arr[1] = s;
		arr[2] = l;
		break;
	}
	case CSPACE_LAB:
	{
		if (r > 0.04045) r = pow(((r + 0.055) / 1.055), 2.4);
		else r /= 12.92;
		if (g > 0.04045) g = pow(((g + 0.055) / 1.055), 2.4);
		else g /= 12.92;
		if (b > 0.04045) b = pow(((b + 0.055) / 1.055), 2.4);
		else b /= 12.92;

		double x = r * 0.4124 + g * 0.3576 + b * 0.1805;
		double y = r * 0.2126 + g * 0.7152 + b * 0.0722;
		double z = r * 0.0193 + g * 0.1192 + b * 0.9505;

		if (x > 0.008856) x = pow(x, 1.0 / 3.0);
		else x = (7.787 * x) + (16.0 / 116.0);
		if (y > 0.008856) y = pow(y, 1.0 / 3.0);
		else y = (7.787 * y) + (16.0 / 116.0);
		if (z > 0.008856) z = pow(z, 1.0 / 3.0);
		else z = (7.787 * z) + (16.0 / 116.0);

		double CIEL = (116 * y) - 16;
		double CIEa = 500 * (x - y);
		double CIEb = 200 * (y - z);

		arr[0] = CIEL;
		arr[1] = CIEa;
		arr[2] = CIEb;
		break;
	}
	case CSPACE_LCH_CW:
	case CSPACE_LCH_CCW:
	case CSPACE_LCH:
	{
		if (r > 0.04045) r = pow(((r + 0.055) / 1.055), 2.4);
		else r /= 12.92;
		if (g > 0.04045) g = pow(((g + 0.055) / 1.055), 2.4);
		else g /= 12.92;
		if (b > 0.04045) b = pow(((b + 0.055) / 1.055), 2.4);
		else b /= 12.92;

		double x = r * 0.4124 + g * 0.3576 + b * 0.1805;
		double y = r * 0.2126 + g * 0.7152 + b * 0.0722;
		double z = r * 0.0193 + g * 0.1192 + b * 0.9505;

		if (x > 0.008856) x = pow(x, 1.0 / 3.0);
		else x = (7.787 * x) + (16.0 / 116.0);
		if (y > 0.008856) y = pow(y, 1.0 / 3.0);
		else y = (7.787 * y) + (16.0 / 116.0);
		if (z > 0.008856) z = pow(z, 1.0 / 3.0);
		else z = (7.787 * z) + (16.0 / 116.0);

		double CIEL = (116 * y) - 16;
		double CIEa = 500 * (x - y);
		double CIEb = 200 * (y - z);

		double h = atan2(CIEb, CIEa);
		if (h > 0) h = (h / PI) * 180;
		else h = 360 - (abs(h) / PI) * 180;

		double CIEC = sqrt(pow(CIEa, 2) + pow(CIEb, 2));

		arr[0] = CIEL;
		arr[1] = CIEC;
		arr[2] = h;
		break;
	}
	}

}

RGB user_paldata::RGBFrom(double arr[], int32_t color_space)
{
	double upper = scripting_max_color_val;
	double r = 0.0;
	double g = 0.0;
	double b = 0.0;
	switch (color_space)
	{
	case CSPACE_CMYK:
	{
		double c = (arr[0] * (1 - arr[3]) + arr[3]);
		double m = (arr[1] * (1 - arr[3]) + arr[3]);
		double y = (arr[2] * (1 - arr[3]) + arr[3]);

		r = vbound((1 - c) * upper, 0.0, upper);
		g = vbound((1 - m) * upper, 0.0, upper);
		b = vbound((1 - y) * upper, 0.0, upper);
		return _RGB(r, g, b);
		break;
	}
	case CSPACE_HSV_CW:
	case CSPACE_HSV_CCW:
	case CSPACE_HSV:
	{
		double h = arr[0];
		double s = arr[1];
		double v = arr[2];

		if (s == 0)
		{
			r = v;
			g = v;
			b = v;
		}
		else
		{
			double var_h = h * 6;
			if (var_h >= 6) var_h = 0;
			int32_t var_i = floor(var_h);
			double var_1 = v * (1 - s);
			double var_2 = v * (1 - s * (var_h - var_i));
			double var_3 = v * (1 - s * (1 - (var_h - var_i)));

			switch (var_i)
			{
			case 0:
				r = v;
				g = var_3;
				b = var_1;
				break;
			case 1:
				r = var_2;
				g = v;
				b = var_1;
				break;
			case 2:
				r = var_1;
				g = v;
				b = var_3;
				break;
			case 3:
				r = var_1;
				g = var_2;
				b = v;
				break;
			case 4:
				r = var_3;
				g = var_1;
				b = v;
				break;
			default:
				r = v;
				g = var_1;
				b = var_2;
			}
		}

		r = vbound(r * upper, 0.0, upper);
		g = vbound(g * upper, 0.0, upper);
		b = vbound(b * upper, 0.0, upper);

		return _RGB(r, g, b);
	}
	case CSPACE_HSL_CW:
	case CSPACE_HSL_CCW:
	case CSPACE_HSL:
	{
		double h = arr[0];
		double s = arr[1];
		double l = arr[2];

		if (s == 0)
		{
			r = l;
			g = l;
			b = l;
		}
		else
		{
			double var_1;
			double var_2;
			if (l < 0.5)var_2 = l * (1 + s);
			else var_2 = (l + s) - (s * l);

			var_1 = 2 * l - var_2;

			r = HueToRGB(var_1, var_2, h + (1.0 / 3.0));
			g = HueToRGB(var_1, var_2, h);
			b = HueToRGB(var_1, var_2, h - (1.0 / 3.0));
		}

		r = vbound(r * upper, 0.0, upper);
		g = vbound(g * upper, 0.0, upper);
		b = vbound(b * upper, 0.0, upper);

		return _RGB(r, g, b);
	}
	case CSPACE_LAB:
	{
		double CIEL = arr[0];
		double CIEa = arr[1];
		double CIEb = arr[2];

		double var_y = (CIEL + 16) / 116.0;
		double var_x = CIEa / 500.0 + var_y;
		double var_z = var_y - CIEb / 200.0;

		if (pow(var_x, 3) > 0.008856) var_x = pow(var_x, 3);
		else var_x = (var_x - 16.0 / 116.0) / 7.787;
		if (pow(var_y, 3) > 0.008856) var_y = pow(var_y, 3);
		else var_y = (var_y - 16.0 / 116.0) / 7.787;
		if (pow(var_z, 3) > 0.008856) var_z = pow(var_z, 3);
		else var_z = (var_z - 16.0 / 116.0) / 7.787;

		r = var_x * 3.2406 + var_y * -1.5372 + var_z * -0.4986;
		g = var_x * -0.9689 + var_y * 1.8758 + var_z * 0.0415;
		b = var_x * 0.0557 + var_y * -0.2040 + var_z * 1.0570;

		if (r > 0.0031308) r = 1.055 * pow(r, (1 / 2.4)) - 0.055;
		else r = 12.92 * r;
		if (g > 0.0031308) g = 1.055 * pow(g, (1 / 2.4)) - 0.055;
		else g = 12.92 * g;
		if (b > 0.0031308) b = 1.055 * pow(b, (1 / 2.4)) - 0.055;
		else b = 12.92 * b;

		r = vbound(r * upper, 0.0, upper);
		g = vbound(g * upper, 0.0, upper);
		b = vbound(b * upper, 0.0, upper);

		return _RGB(r, g, b);
	}
	case CSPACE_LCH_CW:
	case CSPACE_LCH_CCW:
	case CSPACE_LCH:
	{
		double CIEL = arr[0];
		double CIEa = cos((arr[2] * PI) / 180.0) * arr[1];
		double CIEb = sin((arr[2] * PI) / 180.0) * arr[1];

		double var_y = (CIEL + 16) / 116.0;
		double var_x = CIEa / 500.0 + var_y;
		double var_z = var_y - CIEb / 200.0;

		if (pow(var_y, 3) > 0.008856) var_y = pow(var_y, 3);
		else var_y = (var_y - 16.0 / 116.0) / 7.787;
		if (pow(var_x, 3) > 0.008856) var_x = pow(var_x, 3);
		else var_x = (var_x - 16.0 / 116.0) / 7.787;
		if (pow(var_z, 3) > 0.008856) var_z = pow(var_z, 3);
		else var_z = (var_z - 16.0 / 116.0) / 7.787;

		r = var_x * 3.2406 + var_y * -1.5372 + var_z * -0.4986;
		g = var_x * -0.9689 + var_y * 1.8758 + var_z * 0.0415;
		b = var_x * 0.0557 + var_y * -0.2040 + var_z * 1.0570;

		if (r > 0.0031308) r = 1.055 * pow(r, (1 / 2.4)) - 0.055;
		else r = 12.92 * r;
		if (g > 0.0031308) g = 1.055 * pow(g, (1 / 2.4)) - 0.055;
		else g = 12.92 * g;
		if (b > 0.0031308) b = 1.055 * pow(b, (1 / 2.4)) - 0.055;
		else b = 12.92 * b;

		r = vbound(r * upper, 0.0, upper);
		g = vbound(g * upper, 0.0, upper);
		b = vbound(b * upper, 0.0, upper);

		return _RGB(r, g, b);
	}
	}
	return _RGB(0, 0, 0);
}
double user_paldata::HueToRGB(double v1, double v2, double vH)
{
	if (vH < 0) vH += 1;
	if (vH > 1) vH -= 1;
	if ((6 * vH) < 1) return (v1 + (v2 - v1) * 6 * vH);
	if ((2 * vH) < 1) return (v2);
	if ((3 * vH) < 2) return (v1 + (v2 - v1) * ((2.0 / 3.0) - vH) * 6);
	return (v1);
}

double user_paldata::WrapLerp(double a, double b, double t, double min, double max, int32_t direction)
{
	double dif = abs(a - b);
	double range = abs(max - min);

	switch (direction)
	{
	case 0:
		if (dif > range * 0.5)
			dif = range - dif;
		if (a + dif == b)
			direction = 1;
		else
			direction = -1;
		break;
	case 1:
		if (b < a)
			dif = range - dif;
		break;
	case -1:
		if (b > a)
			dif = range - dif;
		break;
	}

	double ret = zc::math::Lerp(a, a + dif * direction, t);

	if (ret <= min)
		ret += range;
	else if (ret >= max)
		ret -= range;
	return ret;
}

//Mixes an entire palette given two paldatas
void user_paldata::mix(user_paldata *pal_start, user_paldata *pal_end, double percent, int32_t color_space, int32_t start_color, int32_t end_color)
{
	for (int32_t q = start_color; q < end_color; ++q)
	{
		if (get_bit(pal_start->colors_used, q) && get_bit(pal_end->colors_used, q)) {
			RGB start = pal_start->colors[q];
			RGB end = pal_end->colors[q];
			colors[q] = mix_color(start, end, percent, color_space);
			set_bit(colors_used, q, true);
		}
	}
}

void item_display_name(const bool setter)
{
	int32_t ID = ri->idata;
	if(unsigned(ID) >= MAXITEMS)
		return;
	int32_t arrayptr = get_register(sarg1);
	if(setter)
	{
		std::string str;
		ArrayH::getString(arrayptr, str, 255);
		strcpy(itemsbuf[ID].display_name, str.c_str());
	}
	else
	{
		if(ArrayH::setArray(arrayptr, string(itemsbuf[ID].display_name)) == SH::_Overflow)
			Z_scripterrlog("Array supplied to 'itemdata->GetDisplayName()' not large enough\n");
	}
}
void item_shown_name()
{
	int32_t ID = ri->idata;
	if(unsigned(ID) >= MAXITEMS)
		return;
	int32_t arrayptr = get_register(sarg1);
	if(ArrayH::setArray(arrayptr, itemsbuf[ID].get_name()) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'itemdata->GetShownName()' not large enough\n");
}

void FFScript::do_getDMapData_dmapname(const bool v)
{
	int32_t ID = ri->dmapsref;
	int32_t arrayptr = get_register(sarg1);
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	if(ArrayH::setArray(arrayptr, string(DMaps[ID].name)) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'dmapdata->GetName()' not large enough\n");
}

void FFScript::do_setDMapData_dmapname(const bool v)
{
	int32_t ID = ri->dmapsref;
	int32_t arrayptr = get_register(sarg1);

	string filename_str;
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
		
	ArrayH::getString(arrayptr, filename_str, 22);
	strncpy(DMaps[ID].name, filename_str.c_str(), 21);
	DMaps[ID].name[20]='\0';
}

void FFScript::do_getDMapData_dmaptitle(const bool v)
{
	int32_t ID = ri->dmapsref;
	int32_t arrayptr = get_register(sarg1);
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	if (!get_qr(qr_OLD_DMAP_INTRO_STRINGS))
	{
		ArrayManager am(arrayptr);
		am.resize(DMaps[ID].title.size() + 1);
	}
	if(ArrayH::setArray(arrayptr, string(DMaps[ID].title)) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'dmapdata->GetTitle()' not large enough\n");
}

void FFScript::do_setDMapData_dmaptitle(const bool v)
{
	int32_t ID = ri->dmapsref;
	int32_t arrayptr = get_register(sarg1);
	string filename_str;

	if(BC::checkDMapID(ID) != SH::_NoError)
		return;

	if (get_qr(qr_OLD_DMAP_INTRO_STRINGS))
	{
		char namestr[21];
		ArrayH::getString(arrayptr, filename_str, 21);
		strncpy(namestr, filename_str.c_str(), 20);
		namestr[20] = '\0';
		DMaps[ID].title.assign(namestr);
	}
	else
	{
		ArrayH::getString(arrayptr, filename_str, ArrayH::getSize(arrayptr));
		DMaps[ID].title = filename_str;
	}
}

void FFScript::do_getDMapData_dmapintro(const bool v)
{
	int32_t ID = ri->dmapsref;
	int32_t arrayptr = get_register(sarg1);
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	if(ArrayH::setArray(arrayptr, string(DMaps[ID].intro)) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'dmapdata->GetIntro()' not large enough\n");
}

void FFScript::do_setDMapData_dmapintro(const bool v)
{
	int32_t ID = ri->dmapsref;
	int32_t arrayptr = get_register(sarg1);
	string filename_str;
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
		
	ArrayH::getString(arrayptr, filename_str, 73);
	strncpy(DMaps[ID].intro, filename_str.c_str(), 72);
	DMaps[ID].intro[72]='\0';
}

void FFScript::do_getDMapData_music(const bool v)
{
	int32_t ID = ri->dmapsref;
	int32_t arrayptr = get_register(sarg1);
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	if(ArrayH::setArray(arrayptr, string(DMaps[ID].tmusic)) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'dmapdata->GetMusic()' not large enough\n");
}

void FFScript::do_setDMapData_music(const bool v)
{
	int32_t ID = ri->dmapsref;
	int32_t arrayptr = get_register(sarg1);
	string filename_str;
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
		
	ArrayH::getString(arrayptr, filename_str, 56);
	strncpy(DMaps[ID].tmusic, filename_str.c_str(), 55);
	DMaps[ID].tmusic[55]='\0';
}

void FFScript::do_loadnpcdata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if ( ID < 1 || ID > (MAXGUYS-1) )
	{
		Z_scripterrlog("Invalid NPC ID passed to Game->LoadNPCData: %d\n", ID);
		ri->npcdataref = MAX_DWORD;
	}
		
	else ri->npcdataref = ID;
}
void FFScript::do_loadmessagedata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if ( ID < 1 || ID > (msg_count-1) )
	{
		Z_scripterrlog("Invalid Message ID passed to Game->LoadMessageData: %d\n", ID);
		ri->zmsgref = MAX_DWORD;
	}
		
	else ri->zmsgref = ID;
}
//same syntax as loadmessage data
//the input is an array
void FFScript::do_messagedata_setstring(const bool v)
{
	int32_t arrayptr = get_register(sarg1);
	int32_t ID = ri->zmsgref;
	if(BC::checkMessage(ID) != SH::_NoError)
		return;
	
	std::string s;
	ArrayH::getString(arrayptr, s, MSG_NEW_SIZE);
	MsgStrings[ID].setFromLegacyEncoding(s);
}
void FFScript::do_messagedata_getstring(const bool v)
{
	int32_t ID = ri->zmsgref;
	int32_t arrayptr = get_register(sarg1);
	
	if(BC::checkMessage(ID) != SH::_NoError)
		return;
		
	if(ArrayH::setArray(arrayptr, MsgStrings[ID].s) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'messagedata->Get()' not large enough\n");
}

void FFScript::do_loadcombodata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if ( (unsigned)ID > (MAXCOMBOS-1) )
	{
		scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
		ri->combosref = 0;
	}

	else ri->combosref = ID;
}

void FFScript::do_loadmapdata_tempscr(const bool v)
{
	int32_t layer = SH::get_arg(sarg1, v) / 10000;

	if (BC::checkBounds(layer, 0, 6) != SH::_NoError)
	{
		ri->mapsref = 0;
		set_register(sarg1, ri->mapsref);
		return;
	}

	ri->mapsref = create_mapdata_temp_ref(mapdata_type::TemporaryCurrentRegion, cur_screen, layer);
	set_register(sarg1, ri->mapsref);
}

void FFScript::do_loadmapdata_tempscr2(const bool v)
{
	int32_t layer = SH::get_arg(sarg1, v) / 10000;
	int32_t screen = SH::get_arg(sarg2, v) / 10000;

	if (BC::checkBounds(layer, 0, 6) != SH::_NoError)
	{
		ri->mapsref = 0;
		set_register(sarg1, ri->mapsref);
		return;
	}

	if (!is_in_current_region(screen))
	{
		scripting_log_error_with_context("Must use a screen in the current region. got: {}", screen);
		ri->mapsref = 0;
		set_register(sarg1, ri->mapsref);
		return;
	}

	ri->mapsref = create_mapdata_temp_ref(mapdata_type::TemporaryCurrentScreen, screen, layer);
	set_register(sarg1, ri->mapsref);
}

static void do_loadtmpscrforcombopos(const bool v)
{
	int32_t layer = SH::get_arg(sarg1, v) / 10000;
	rpos_t rpos = (rpos_t)(SH::get_arg(sarg2, v) / 10000);

	if (BC::checkBoundsRpos(rpos, (rpos_t)0, region_max_rpos) != SH::_NoError)
	{
		ri->mapsref = 0;
		set_register(sarg1, ri->mapsref);
		return;
	}
	if (BC::checkBounds(layer, 0, 6) != SH::_NoError)
	{
		ri->mapsref = 0;
		set_register(sarg1, ri->mapsref);
		return;
	}

	set_register(sarg1, create_mapdata_temp_ref(mapdata_type::TemporaryCurrentScreen, get_screen_for_rpos(rpos), layer));
}

void FFScript::do_loadmapdata_scrollscr(const bool v)
{
	int32_t layer = SH::get_arg(sarg1, v) / 10000;

	if (BC::checkBounds(layer, 0, 6) != SH::_NoError)
	{
		ri->mapsref = 0;
		set_register(sarg1, ri->mapsref);
		return;
	}

	ri->mapsref = create_mapdata_temp_ref(mapdata_type::TemporaryScrollingRegion, scrolling_hero_screen, layer);
	set_register(sarg1, ri->mapsref);
}

void FFScript::do_loadmapdata_scrollscr2(const bool v)
{
	int32_t layer = SH::get_arg(sarg1, v) / 10000;
	int32_t screen = SH::get_arg(sarg2, v) / 10000;

	if (BC::checkBounds(layer, 0, 6) != SH::_NoError)
	{
		ri->mapsref = 0;
		set_register(sarg1, ri->mapsref);
		return;
	}

	if (!is_in_scrolling_region(screen))
	{
		scripting_log_error_with_context("Must use a screen in the current scrolling region. got: {}", screen);
		ri->mapsref = 0;
		set_register(sarg1, ri->mapsref);
		return;
	}

	ri->mapsref = create_mapdata_temp_ref(mapdata_type::TemporaryScrollingScreen, screen, layer);
	set_register(sarg1, ri->mapsref);
}
	
void FFScript::do_loadshopdata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if ( (unsigned)ID > 255 )
	{
		Z_scripterrlog("Invalid Shop ID passed to Game->LoadShopData: %d\n", ID);
		ri->shopsref = 0;
	}	
	else ri->shopsref = ID;
}


void FFScript::do_loadinfoshopdata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if ( (unsigned)ID > 255 )
	{
		Z_scripterrlog("Invalid Shop ID passed to Game->LoadShopData: %d\n", ID);
		ri->shopsref = 0;
	}	
	else ri->shopsref = ID+NUMSHOPS;
}

void FFScript::do_loadspritedata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if ( (unsigned)ID > (MAXWPNS-1) )
	{
		Z_scripterrlog("Invalid Sprite ID passed to Game->LoadSpriteData: %d\n", ID);
		ri->spritedataref = 0; 
	}

	else ri->spritedataref = ID;
}

void FFScript::do_loadbitmapid(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	switch(ID)
	{
		case rtSCREEN:
		case rtBMP0:
		case rtBMP1:
		case rtBMP2:
		case rtBMP3:
		case rtBMP4:
		case rtBMP5:
		case rtBMP6:
			ri->bitmapref = ID+10; break;
		default:
		{
			Z_scripterrlog("Invalid Bitmap ID passed to Game->Load BitmapID: %d\n", ID);
			ri->bitmapref = 0; break;
		}
	}
}

void do_createlweapon(const bool v)
{
	const int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkWeaponID(ID) != SH::_NoError)
		return;
	
	if ( Lwpns.has_space() )
	{
		(void)Lwpns.add
		(
			new weapon
			(
				(zfix)0, /*X*/
				(zfix)0, /*Y*/
				(zfix)0, /*Z*/
				ID,	 /*id*/
				0,	 /*type*/
				0,	 /*power*/
				0,	 /*dir*/
				-1,	 /*Parentid*/
				Hero.getUID(), /*prntid*/
				false,	 /*isdummy*/
				1,	 /*script_gen*/
				1,  /*islwpn*/
				(ID==wWind?1:0)  /*special*/
			)
		);
		ri->lwpn = Lwpns.spr(Lwpns.Count() - 1)->getUID();
		weapon *w = (weapon*)Lwpns.spr(Lwpns.Count()-1); //last created
		w->screen_spawned = ri->screenref;
		w->ScriptGenerated = 1;
		w->isLWeapon = 1;
		if(ID == wWind) w->specialinfo = 1;
		Z_eventlog("Script created lweapon %d with UID = %u\n", ID, ri->lwpn);
	}
	else
	{
		ri->lwpn = 0; // Now NULL
		Z_scripterrlog("Couldn't create lweapon %d, screen lweapon limit reached\n", ID);
	}
}

void do_createeweapon(const bool v)
{
	const int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkWeaponID(ID) != SH::_NoError)
		return;
		
	if ( Ewpns.has_space() )
	{
		addEwpn(0, 0, 0, ID, 0, 0, 0, -1,1); //Param 9 marks it as script-generated.
		if( ID > wEnemyWeapons || ( ID >= wScript1 && ID <= wScript10) )
		{
			weapon *w = (weapon*)Ewpns.spr(Ewpns.Count()-1); //last created
			w->screen_spawned = ri->screenref;
			w->ScriptGenerated = 1;
			w->isLWeapon = 0;
			ri->ewpn = Ewpns.spr(Ewpns.Count() - 1)->getUID();
			Z_eventlog("Script created eweapon %d with UID = %u\n", ID, ri->ewpn);
		}
		else
		{
			Z_scripterrlog("Couldn't create eweapon: Invalid ID/Type (%d) specified.\n", ID);
			return;
		}
	}
	else
	{
		ri->ewpn = 0;
		Z_scripterrlog("Couldn't create eweapon %d, screen eweapon limit reached\n", ID);
	}
}

void do_createitem(const bool v)
{
	const int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkItemID(ID) != SH::_NoError)
		return;

	if ( items.has_space() )
	{
		additem(0, (get_qr(qr_NOITEMOFFSET) ? 1: 0), ID, ipBIGRANGE);
		sprite* item = items.spr(items.Count() - 1);
		item->screen_spawned = ri->screenref;
		ri->itemref = item->getUID();
		Z_eventlog("Script created item \"%s\" with UID = %u\n", item_string[ID], ri->itemref);
	}
	else
	{
		ri->itemref = 0;
		Z_scripterrlog("Couldn't create item \"%s\", screen item limit reached\n", item_string[ID]);
	}
}

void do_createnpc(const bool v)
{
	const int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkGuyID(ID) != SH::_NoError)
		return;
		
	//If we make a segmented enemy there'll be more than one sprite created
	word numcreated = addenemy(ri->screenref, 0, 0, ID, -10);
	
	if(numcreated == 0)
	{
		//ri->guyref = MAX_DWORD;
		ri->guyref = 0;
		Z_scripterrlog("Couldn't create NPC \"%s\", screen NPC limit reached\n", guy_string[ID]);
	}
	else
	{
		word index = guys.Count() - numcreated; //Get the main enemy, not a segment
		ri->guyref = guys.spr(index)->getUID();
		
		for(; index<guys.Count(); index++)
			((enemy*)guys.spr(index))->script_spawned=true;
			
		Z_eventlog("Script created NPC \"%s\" with UID = %u\n", guy_string[ID], ri->guyref);
	}
}

///----------------------------------------------------------------------------------------------------//
//Drawing & Sound

void do_message(const bool v)
{
	const int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkMessage(ID) != SH::_NoError)
		return;
		
	if(ID == 0)
	{
		dismissmsg();
		msgfont = get_zc_font(font_zfont);
		blockpath = false;
		Hero.finishedmsg();
	}
	else
		donewmsg(get_scr(ri->screenref), ID);
}

INLINE void set_drawing_command_args(const int32_t j, const word numargs)
{
	assert(numargs <= DRAWCMD_MAX_ARG_COUNT);
	for(int32_t k = 1; k <= numargs; k++)
		script_drawing_commands[j][k] = SH::read_stack(ri->sp + (numargs - k));
}

INLINE void set_user_bitmap_command_args(const int32_t j, const word numargs)
{
	assert(numargs <= DRAWCMD_MAX_ARG_COUNT);
	//ri->bitmapref = SH::read_stack(ri->sp+numargs);
	for(int32_t k = 1; k <= numargs; k++)
		script_drawing_commands[j][k] = SH::read_stack(ri->sp + (numargs - k));
}

static DrawOrigin get_draw_origin_for_screen_draw_command()
{
	DrawOrigin draw_origin = ri->screen_draw_origin;

	if (draw_origin == DrawOrigin::Default)
	{
		bool in_scrolling_region = is_in_scrolling_region() || (screenscrolling && scrolling_region.screen_count > 1);
		draw_origin = in_scrolling_region ? DrawOrigin::Region : DrawOrigin::PlayingField;
	}
	if (draw_origin == DrawOrigin::Region)
	{
		if (scrolling_using_new_region_coords)
			draw_origin = DrawOrigin::RegionScrollingNew;
	}
	else if (draw_origin == DrawOrigin::RegionScrollingOld)
	{
		draw_origin = DrawOrigin::Region;
	}
	else if (draw_origin == DrawOrigin::RegionScrollingNew)
	{
		if (!screenscrolling)
			draw_origin = DrawOrigin::Region;
	}

	return draw_origin;
}

static DrawOrigin get_draw_origin_for_bitmap_draw_command()
{
	return DrawOrigin::Screen;
}

static std::pair<DrawOrigin, int> get_draw_origin_for_draw_command(bool is_screen_draw, int scripting_bitmap_id)
{
	if (get_qr(qr_BROKEN_SCRIPTS_BITMAP_DRAW_ORIGIN))
		return {get_draw_origin_for_screen_draw_command(), ri->screen_draw_origin_target};

	auto [bitmap_id, _] = resolveScriptingBitmapId(scripting_bitmap_id);

	if (FFCore.doesResolveToDeprecatedSystemBitmap(bitmap_id))
		return {DrawOrigin::Screen, 0};

	if (FFCore.doesResolveToScreenBitmap(bitmap_id))
		return {get_draw_origin_for_screen_draw_command(), ri->screen_draw_origin_target};

	if (!is_screen_draw)
		return {get_draw_origin_for_bitmap_draw_command(), 0};

	return {DrawOrigin::Screen, 0};
}

static void do_drawing_command(int32_t script_command, bool is_screen_draw)
{
	if (FFCore.skipscriptdraws)
		return;

	int32_t j = script_drawing_commands.GetNext();
	if(j == -1)  //out of drawing command space
	{
		Z_scripterrlog("Max draw primitive limit reached\n");
		return;
	}

	script_drawing_commands[j] = {};
	script_drawing_commands[j][0] = script_command;
	script_drawing_commands[j][DRAWCMD_CURRENT_TARGET] = zscriptDrawingRenderTarget->GetCurrentRenderTarget();

	switch(script_command)
	{
		case RECTR:
			set_drawing_command_args(j, 12);
			break;
			
		case FRAMER:
			set_drawing_command_args(j, 9);
			break;
			
		case CIRCLER:
			set_drawing_command_args(j, 11);
			break;
			
		case ARCR:
			set_drawing_command_args(j, 14);
			break;
			
		case ELLIPSER:
			set_drawing_command_args(j, 12);
			break;
			
		case LINER:
			set_drawing_command_args(j, 11);
			break;
			
		case PUTPIXELR:
			set_drawing_command_args(j, 8);
			break;
		
		case PIXELARRAYR:
		{
			set_drawing_command_args(j, 5);
			std::vector<int32_t> *v = script_drawing_commands.GetVector();
			
			int32_t arrayptr = script_drawing_commands[j][2];
			if ( !arrayptr ) //Don't crash because of vector size.
			{
				Z_scripterrlog("Invalid array pointer %d passed to Screen->PutPixels(). Aborting.", arrayptr);
				break;
			}
			int32_t sz = ArrayH::getSize(arrayptr);
			if(!sz)
			{
				script_drawing_commands.PopLast();
				return;
			}
			v->resize(sz, 0);
			int32_t* pos = &v->at(0);
			
			ArrayH::getValues(script_drawing_commands[j][2], pos, sz);
			script_drawing_commands[j].SetVector(v);
			break;
		}
		
		case TILEARRAYR:
		{
			set_drawing_command_args(j, 2);
			std::vector<int32_t> *v = script_drawing_commands.GetVector();
			
			int32_t arrayptr = script_drawing_commands[j][2];
			if ( !arrayptr ) //Don't crash because of vector size.
			{
				Z_scripterrlog("Invalid array pointer %d passed to Screen->DrawTiles(). Aborting.", arrayptr);
				break;
			}
			int32_t sz = ArrayH::getSize(arrayptr);
			if(!sz)
			{
				script_drawing_commands.PopLast();
				return;
			}
			v->resize(sz, 0);
			int32_t* pos = &v->at(0);
			
			ArrayH::getValues(script_drawing_commands[j][2], pos, sz);
			script_drawing_commands[j].SetVector(v);
			break;
			}
			
		case LINESARRAY:
		{
			set_drawing_command_args(j, 2);
			std::vector<int32_t> *v = script_drawing_commands.GetVector();
			
			int32_t arrayptr = script_drawing_commands[j][2];
			if ( !arrayptr ) //Don't crash because of vector size.
			{
				Z_scripterrlog("Invalid array pointer %d passed to Screen->Lines(). Aborting.", arrayptr);
				break;
			}
			int32_t sz = ArrayH::getSize(arrayptr);
			if(!sz)
			{
				script_drawing_commands.PopLast();
				return;
			}
			v->resize(sz, 0);
			int32_t* pos = &v->at(0);
			
			ArrayH::getValues(script_drawing_commands[j][2], pos, sz);
			script_drawing_commands[j].SetVector(v);
			break;
			}
			
		case COMBOARRAYR:
		{
			set_drawing_command_args(j, 2);
			std::vector<int32_t> *v = script_drawing_commands.GetVector();
			int32_t arrayptr = script_drawing_commands[j][2];
			if ( !arrayptr ) //Don't crash because of vector size.
			{
				Z_scripterrlog("Invalid array pointer %d passed to Screen->DrawCombos(). Aborting.", arrayptr);
				break;
			}
			int32_t sz = ArrayH::getSize(arrayptr);
			if(!sz)
			{
				script_drawing_commands.PopLast();
				return;
			}
			v->resize(sz, 0);
			int32_t* pos = &v->at(0);
			
			ArrayH::getValues(script_drawing_commands[j][2], pos, sz);
			script_drawing_commands[j].SetVector(v);
			break;
		}
		case POLYGONR:
		{
			set_drawing_command_args(j, 5);
				
			int32_t arrayptr = script_drawing_commands[j][3];
			if ( !arrayptr ) //Don't crash because of vector size.
			{
				Z_scripterrlog("Invalid array pointer %d passed to Screen->Polygon(). Aborting.", arrayptr);
				break;
			}
			int32_t sz = ArrayH::getSize(arrayptr);
			if(!sz)
			{
				script_drawing_commands.PopLast();
				return;
			}
				
			std::vector<int32_t> *v = script_drawing_commands.GetVector();
			v->resize(sz, 0);
			
			int32_t* pos = &v->at(0);
			
			
			ArrayH::getValues(script_drawing_commands[j][3], pos, sz);
			script_drawing_commands[j].SetVector(v);
		}
		break;
			
		case DRAWTILER:
			set_drawing_command_args(j, 15);
			break;
			
		case DRAWTILECLOAKEDR:
			set_drawing_command_args(j, 7);
			break;
			
		case DRAWCOMBOR:
			set_drawing_command_args(j, 16);
			break;
			
		case DRAWCOMBOCLOAKEDR:
			set_drawing_command_args(j, 7);
			break;
			
		case FASTTILER:
			set_drawing_command_args(j, 6);
			break;
			
		case FASTCOMBOR:
			set_drawing_command_args(j, 6);
			break;
			
		case DRAWCHARR:
			set_drawing_command_args(j, 10);
			break;
			
		case DRAWINTR:
			set_drawing_command_args(j, 11);
			break;
			
		case SPLINER:
			set_drawing_command_args(j, 11);
			break;
			
		case QUADR:
			set_drawing_command_args(j, 15);
			break;
			
		case TRIANGLER:
			set_drawing_command_args(j, 13);
			break;
			
		case BITMAPR:
			set_drawing_command_args(j, 12);
			break;
		
		case BITMAPEXR:
			set_drawing_command_args(j, 16);
			break;
			
		case DRAWLAYERR:
			set_drawing_command_args(j, 8);
			break;
			
		case DRAWSCREENR:
			set_drawing_command_args(j, 6);
			break;
			
		case QUAD3DR:
		{
			set_drawing_command_args(j, 8);
			int32_t arrayptr = script_drawing_commands[j][2];
			int32_t sz = ArrayH::getSize(arrayptr);
			arrayptr = script_drawing_commands[j][3];
			sz += ArrayH::getSize(arrayptr);
			arrayptr = script_drawing_commands[j][4];
			sz += ArrayH::getSize(arrayptr);
			arrayptr = script_drawing_commands[j][5];
			sz += ArrayH::getSize(arrayptr);
			if(sz < 25)
			{
				script_drawing_commands.PopLast();
				return;
			}
			std::vector<int32_t> *v = script_drawing_commands.GetVector();
			v->resize(sz, 0);
			
			int32_t* pos = &v->at(0);
			int32_t* uv = &v->at(12);
			int32_t* col = &v->at(20);
			int32_t* size = &v->at(24);
			
			ArrayH::getValues((script_drawing_commands[j][2]), pos, 12);
			ArrayH::getValues((script_drawing_commands[j][3]), uv, 8);
			ArrayH::getValues((script_drawing_commands[j][4]), col, 4);
			//FFCore.getValues2(script_drawing_commands[j][5], size, 2);
			ArrayH::getValues((script_drawing_commands[j][5]), size, 2);
			
			script_drawing_commands[j].SetVector(v);
		}
		break;
		
		case TRIANGLE3DR:
		{
			set_drawing_command_args(j, 8);
				
			int32_t arrayptr = script_drawing_commands[j][2];
			int32_t sz = ArrayH::getSize(arrayptr);
			arrayptr = script_drawing_commands[j][3];
			sz += ArrayH::getSize(arrayptr);
			arrayptr = script_drawing_commands[j][4];
			sz += ArrayH::getSize(arrayptr);
			arrayptr = script_drawing_commands[j][5];
			sz += ArrayH::getSize(arrayptr);
			if(sz < 19)
			{
				script_drawing_commands.PopLast();
				return;
			}
			
			std::vector<int32_t> *v = script_drawing_commands.GetVector();
			v->resize(sz, 0);
			
			int32_t* pos = &v->at(0);
			int32_t* uv = &v->at(9);
			int32_t* col = &v->at(15);
			int32_t* size = &v->at(18);
			
			ArrayH::getValues(script_drawing_commands[j][2], pos, 8);
			ArrayH::getValues(script_drawing_commands[j][3], uv, 6);
			ArrayH::getValues(script_drawing_commands[j][4], col, 3);
			ArrayH::getValues(script_drawing_commands[j][5], size, 2);
			
			script_drawing_commands[j].SetVector(v);
		}
		break;
		
		case DRAWSTRINGR:
		{
			set_drawing_command_args(j, 9);
			// Unused
			//const int32_t index = script_drawing_commands[j][19] = j;
			
			string *str = script_drawing_commands.GetString();
			ArrayH::getString(script_drawing_commands[j][8], *str, 256);
			script_drawing_commands[j].SetString(str);
		}
		break;
		
		case DRAWSTRINGR2:
		{
			set_drawing_command_args(j, 11);
			// Unused
			//const int32_t index = script_drawing_commands[j][19] = j;
			
			string *str = script_drawing_commands.GetString();
			ArrayH::getString(script_drawing_commands[j][8], *str, 256);
			script_drawing_commands[j].SetString(str);
		}
		break;
		
		case BMPRECTR:	
			set_user_bitmap_command_args(j, 12); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+12);
			//Pop the args off the stack first. Then pop the pointer and push it to sdci[17]. 
			//The pointer for the bitmap variable (its literal value) is always ri->sp+numargs, so, with 12 args, it is sp+12.
			break;
		
		case BMPFRAMER:	
			set_user_bitmap_command_args(j, 9);
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+9);
			break;
			
		case CLEARBITMAP:	
		{
			set_user_bitmap_command_args(j, 1);
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+1); 
			break;
		}
		case BITMAPCLEARTOCOLOR:	
		{
			set_user_bitmap_command_args(j, 2);
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+2); 
			break;
		}
		case REGENERATEBITMAP:	
		{
			set_user_bitmap_command_args(j, 3);
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+3);
			break;
		}
		case BMPPOLYGONR:
		{
			set_user_bitmap_command_args(j, 5);
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+5); 
			int32_t arrayptr = script_drawing_commands[j][3];
			if ( !arrayptr ) //Don't crash because of vector size.
			{
				Z_scripterrlog("Invalid array pointer %d passed to Screen->Polygon(). Aborting.", arrayptr);
				break;
			}
			int32_t sz = ArrayH::getSize(arrayptr);
			if(!sz)
			{
				script_drawing_commands.PopLast();
				return;
			}
			std::vector<int32_t> *v = script_drawing_commands.GetVector();
			v->resize(sz, 0);
			
			int32_t* pos = &v->at(0);
			
			
			ArrayH::getValues(script_drawing_commands[j][3], pos, sz);
			script_drawing_commands[j].SetVector(v);
		}
		break;
		case READBITMAP:	
		{
			set_user_bitmap_command_args(j, 2);
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+2);
			string& user_path = *script_drawing_commands.GetString();
			ArrayH::getString(script_drawing_commands[j][2], user_path, 256);

			if (get_qr(qr_BITMAP_AND_FILESYSTEM_PATHS_ALWAYS_RELATIVE))
			{
				if (auto r = parse_user_path(user_path, true); !r)
				{
					scripting_log_error_with_context("Error: {}", r.error());
					return;
				} else user_path = r.value();
			}
			else
			{
				regulate_path(user_path);
			}
			
			script_drawing_commands[j].SetString(&user_path);
			break;
		}
		case WRITEBITMAP:	
		{
			set_user_bitmap_command_args(j, 3);
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+3); 
			std::string& user_path = *script_drawing_commands.GetString();
			ArrayH::getString(script_drawing_commands[j][2], user_path, 256);

			if (get_qr(qr_BITMAP_AND_FILESYSTEM_PATHS_ALWAYS_RELATIVE))
			{
				if (auto r = parse_user_path(user_path, true); !r)
				{
					scripting_log_error_with_context("Error: {}", r.error());
					return;
				} else user_path = r.value();
			}
			else
			{
				regulate_path(user_path);
			}

			script_drawing_commands[j].SetString(&user_path);
			break;
		}
		
		case BMPCIRCLER:	set_user_bitmap_command_args(j, 11); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+11);  break;
		case BMPARCR:	set_user_bitmap_command_args(j, 14); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+14);  break;
		case BMPELLIPSER:	set_user_bitmap_command_args(j, 12); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+12);  break;
		case BMPLINER:	set_user_bitmap_command_args(j, 11); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+11); break;
		case BMPSPLINER:	set_user_bitmap_command_args(j, 11); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+11); break;
		case BMPPUTPIXELR:	set_user_bitmap_command_args(j, 8); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+8); break;
		case BMPDRAWTILER:	set_user_bitmap_command_args(j, 15); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+15); break;
		case BMPDRAWTILECLOAKEDR:	set_user_bitmap_command_args(j, 7); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+7); break;
		case BMPDRAWCOMBOR:	set_user_bitmap_command_args(j, 16); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+16); break;
		case BMPDRAWCOMBOCLOAKEDR:	set_user_bitmap_command_args(j, 7); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+7); break;
		case BMPFASTTILER:	set_user_bitmap_command_args(j, 6); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+6); break;
		case BMPFASTCOMBOR:  set_user_bitmap_command_args(j, 6); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+6); break;
		case BMPDRAWCHARR:	set_user_bitmap_command_args(j, 10); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+10); break;
		case BMPDRAWINTR:	set_user_bitmap_command_args(j, 11); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+11); break;
		case BMPDRAWSTRINGR:	
		{
			set_user_bitmap_command_args(j, 9);
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+9);
			// Unused
			//const int32_t index = script_drawing_commands[j][19] = j;
			
			string *str = script_drawing_commands.GetString();
			ArrayH::getString(script_drawing_commands[j][8], *str, 256);
			script_drawing_commands[j].SetString(str);
			
		}
		break;
		case BMPDRAWSTRINGR2:	
		{
			set_user_bitmap_command_args(j, 11);
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+11);
			// Unused
			//const int32_t index = script_drawing_commands[j][19] = j;
			
			string *str = script_drawing_commands.GetString();
			ArrayH::getString(script_drawing_commands[j][8], *str, 256);
			script_drawing_commands[j].SetString(str);
			
		}
		break;
		case BMPQUADR:	set_user_bitmap_command_args(j, 16);  script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+16); break;
		case BMPQUAD3DR:
		{
			set_drawing_command_args(j, 9);
			std::vector<int32_t> *v = script_drawing_commands.GetVector();
			v->resize(26, 0);
			
			int32_t* pos = &v->at(0);
			int32_t* uv = &v->at(12);
			int32_t* col = &v->at(20);
			int32_t* size = &v->at(24);
			
			
			ArrayH::getValues(script_drawing_commands[j][2], pos, 12);
			ArrayH::getValues(script_drawing_commands[j][3], uv, 8);
			ArrayH::getValues(script_drawing_commands[j][4], col, 4);
			ArrayH::getValues(script_drawing_commands[j][5], size, 2);
			
			script_drawing_commands[j].SetVector(v);
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+9);
			
		}
		break;
		case BMPTRIANGLER:	set_user_bitmap_command_args(j, 14); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+14); break;
		case BMPTRIANGLE3DR:
		{
			set_drawing_command_args(j, 9);
			
			std::vector<int32_t> *v = script_drawing_commands.GetVector();
			v->resize(20, 0);
			
			int32_t* pos = &v->at(0);
			int32_t* uv = &v->at(9);
			int32_t* col = &v->at(15);
			int32_t* size = &v->at(18);
			
			
			ArrayH::getValues(script_drawing_commands[j][2], pos, 8);
			ArrayH::getValues(script_drawing_commands[j][3], uv, 6);
			ArrayH::getValues(script_drawing_commands[j][4], col, 3);
			ArrayH::getValues(script_drawing_commands[j][5], size, 2);
			
			script_drawing_commands[j].SetVector(v);
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+9);
			break;
		}
		
		case BMPDRAWLAYERR:
			set_user_bitmap_command_args(j, 8);
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+8);
			break;
		case BMPDRAWLAYERSOLIDR: 
		case BMPDRAWLAYERCFLAGR: 
		case BMPDRAWLAYERCTYPER: 
		case BMPDRAWLAYERCIFLAGR: 
		case BMPDRAWLAYERSOLIDITYR: set_user_bitmap_command_args(j, 9); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+9); break;
		case BMPDRAWSCREENR:
		case BMPDRAWSCREENSOLIDR:
		case BMPDRAWSCREENSOLID2R:
		case BMPDRAWSCREENCOMBOFR:
		case BMPDRAWSCREENCOMBOIR:
		case BMPDRAWSCREENCOMBOTR:
			set_user_bitmap_command_args(j, 6); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+6); break;
		case BITMAPGETPIXEL:
		{
			set_user_bitmap_command_args(j, 3); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+3);
			break;
		}
		case BMPBLIT:	
		{
			set_user_bitmap_command_args(j, 16); 

			int bmp_target = SH::read_stack(ri->sp+16);
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = bmp_target;

			if (!get_qr(qr_BROKEN_SCRIPTS_BITMAP_DRAW_ORIGIN))
			{
				int bmp_dest = script_drawing_commands[j][2];
				auto [draw_origin, draw_origin_target] = get_draw_origin_for_draw_command(is_screen_draw, bmp_dest);
				script_drawing_commands[j].secondary_draw_origin = draw_origin;
				script_drawing_commands[j].secondary_draw_origin_target = draw_origin_target;
			}
			break;
		}
		case BMPBLITTO:	
		{
			set_user_bitmap_command_args(j, 16); 
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+16);

			if (!get_qr(qr_BROKEN_SCRIPTS_BITMAP_DRAW_ORIGIN))
			{
				int bmp_source = script_drawing_commands[j][2];
				auto [draw_origin, draw_origin_target] = get_draw_origin_for_draw_command(is_screen_draw, bmp_source);
				script_drawing_commands[j].secondary_draw_origin = draw_origin;
				script_drawing_commands[j].secondary_draw_origin_target = draw_origin_target;
			}
			break;
		}
		case TILEBLIT:
		{
			set_drawing_command_args(j, 17); 
			break;
		}
		case COMBOBLIT:
		{
			set_drawing_command_args(j, 17); 
			break;
		}
		case BMPTILEBLIT:
		{
			set_user_bitmap_command_args(j, 17); 
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+17);
			break;
		}
		case BMPCOMBOBLIT:
		{
			set_user_bitmap_command_args(j, 17); 
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+17);
			break;
		}
		case BMPMODE7:	
		{
			set_user_bitmap_command_args(j, 13); 
			//for(int32_t q = 0; q < 8; ++q )
			//Z_scripterrlog("FFscript blit() ri->d[%d] is: %d\n", q, ri->d[q]);
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+13);
			break;
		}
		
		case BMPWRITETILE:
		{
			set_user_bitmap_command_args(j, 6);
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+6);
			break;
		}
		case BMPDITHER:
		{
			set_user_bitmap_command_args(j, 5);
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+5);
			break;
		}
		case BMPMASKDRAW:
		{
			set_user_bitmap_command_args(j, 3);
			script_drawing_commands[j][4] = 0x01 * 10000L;
			script_drawing_commands[j][5] = 0xFF * 10000L;
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+3);
			break;
		}
		case BMPMASKDRAW2:
		{
			set_user_bitmap_command_args(j, 4);
			script_drawing_commands[j][5] = script_drawing_commands[j][4];
			script_drawing_commands[j][0] = BMPMASKDRAW;
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+4);
			break;
		}
		case BMPMASKDRAW3:
		{
			set_user_bitmap_command_args(j, 5);
			script_drawing_commands[j][0] = BMPMASKDRAW;
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+5);
			break;
		}
		case BMPMASKBLIT:
		{
			set_user_bitmap_command_args(j, 4);
			script_drawing_commands[j][5] = 0x01 * 10000L;
			script_drawing_commands[j][6] = 0xFF * 10000L;
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+4);
			break;
		}
		case BMPMASKBLIT2:
		{
			set_user_bitmap_command_args(j, 5);
			script_drawing_commands[j][6] = script_drawing_commands[j][5];
			script_drawing_commands[j][0] = BMPMASKBLIT;
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+5);
			break;
		}
		case BMPMASKBLIT3:
		{
			set_user_bitmap_command_args(j, 6);
			script_drawing_commands[j][0] = BMPMASKBLIT;
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+6);
			break;
		}
		case BMPREPLCOLOR:
		case BMPSHIFTCOLOR:
		{
			set_user_bitmap_command_args(j, 4);
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+4);
			break;			
		}

		case DRAWLIGHT_CONE:
		case DRAWLIGHT_CIRCLE:
		case DRAWLIGHT_SQUARE:
		{
			// These draw commands implicitly draw at the SPLAYER_DARKROOM_UNDER timing.
			// Shift the given args up by one.
			int num_args = script_command == DRAWLIGHT_CONE ? 8 : 7;
			set_drawing_command_args(j, num_args);
			for (int i = num_args; i >= 1; i--)
				script_drawing_commands[j][i + 1] = script_drawing_commands[j][i];
			script_drawing_commands[j][1] = SPLAYER_DARKROOM_UNDER * 10000;
			break;
		}
	}

	int bmp_target;
	if (is_screen_draw)
		bmp_target = zscriptDrawingRenderTarget->GetCurrentRenderTarget() + 10;
	else
		bmp_target = script_drawing_commands[j][DRAWCMD_BMP_TARGET];

	auto [draw_origin, draw_origin_target] = get_draw_origin_for_draw_command(is_screen_draw, bmp_target);
	script_drawing_commands[j].draw_origin = draw_origin;
	script_drawing_commands[j].draw_origin_target = draw_origin_target;

	script_drawing_commands.mark_dirty(script_drawing_commands[j][1]/10000);
}

void do_set_rendertarget(bool)
{
	int32_t target = int32_t(SH::read_stack(ri->sp) / 10000);
	zscriptDrawingRenderTarget->SetCurrentRenderTarget(target);
}

void do_sfx(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkSFXID(ID) != SH::_NoError)
		return;
		
	sfx(ID);
}

void do_sfx_ex(const bool restart)
{
	int32_t ID = SH::read_stack(ri->sp + 4) / 10000;
	int32_t vol = vbound(SH::read_stack(ri->sp + 3), 0, 10000 * 100);
	int32_t pan = vbound(SH::read_stack(ri->sp + 2)/10000 + 128, 0, 255);
	int32_t freq = SH::read_stack(ri->sp + 1);
	bool loop = SH::read_stack(ri->sp) / 10000;

	if (BC::checkSFXID(ID) != SH::_NoError)
		return;

	if (!restart && !sfx_allocated(ID))
		return;

	sfx(ID, pan, loop, restart, vol, freq);
}

static int get_sfx_completion()
{
	int32_t ID = get_register(sarg1) / 10000;

	if (!sfx_allocated(ID))
	{
		return -10000;
	}

	int sample_pos = voice_get_position(sfx_voice[ID]);
	if (sample_pos < 0)
	{
		return -10000;
	}

	uint32_t sample_length = sfx_get_length(ID);
	uint64_t res = ((uint64_t)sample_pos * 10000 * 100) / sample_length;
	return int32_t(res);
}

void do_get_sfx_completion()
{
	int32_t ID = get_register(sarg1) / 10000;
	if (replay_is_active())
		replay_step_comment(fmt::format("ID {}", ID));
	int32_t value = replay_get_state(ReplayStateType::SfxPosition, get_sfx_completion);
	set_register(sarg1, value);
}

void FFScript::AlloffLimited(int32_t flagset)
{
	clear_bitmap(msg_txt_display_buf);
	clear_bitmap(msg_bg_display_buf);
	clear_bitmap(msg_portrait_display_buf);
	set_clip_state(msg_txt_display_buf, 1);
	set_clip_state(msg_bg_display_buf, 1);
	set_clip_state(msg_portrait_display_buf, 1);
	
	
	clear_bitmap(pricesdisplaybuf);
	set_clip_state(pricesdisplaybuf, 1);
	
	if(items.idCount(iPile))
	{
		loadlvlpal(DMaps[cur_dmap].color);
	}
	
	/*
	
	#define warpFlagCLEARITEMS 0x200
	#define warpFlagCLEARGUYS 0x400
	#define warpFlagCLEARLWEAPONS 0x800
	#define warpFlagCLEAREWEAPONS 0x1000
	#define warpFlagCLEARHOOKSHOT 0x2000
	#define warpFlagCLEARDECORATIONS 0x4000
	#define warpFlagCLEARPARTICLES 0x8000
	*/
	
	if ( (flagset&warpFlagCLEARITEMS) ) items.clear();
	if ( (flagset&warpFlagCLEARGUYS) ) guys.clear();
	if ( (flagset&warpFlagCLEARLWEAPONS) ) Lwpns.clear();
	if ( (flagset&warpFlagCLEAREWEAPONS) ) Ewpns.clear();
	if ( (flagset&warpFlagCLEARHOOKSHOT) ) 
	{
		chainlinks.clear();
		Hero.reset_hookshot();
	}
	if ( (flagset&warpFlagCLEARDECORATIONS) ) decorations.clear();
	if ( (flagset&warpFlagCLEARPARTICLES) ) particles.clear();
	clearScriptHelperData();
	
	
	
	clearScriptHelperData();
	
	lensclk = 0;
	lensid=-1;
	drawguys=true;
	down_control_states[btnUp] =
		down_control_states[btnDown] =
			down_control_states[btnLeft] =
				down_control_states[btnRight] =
					down_control_states[btnA] =
						down_control_states[btnB] =
							down_control_states[btnS] = true;

	if(watch && !cheat_superman)
	{
		Hero.setClock(false);
	}
	
	watch=freeze_guys=loaded_guys=blockpath=false;

	activation_counters.clear();
	activation_counters_ffc.clear();
	for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
		get_screen_state(scr->screen).loaded_enemies = false;
	});

	sle_clk=0;
	
	if(usebombpal)
	{
		memcpy(RAMpal, tempbombpal, PAL_SIZE*sizeof(RGB));
		refreshpal=true;
		usebombpal=false;
	}
	
	
}

void doWarpEffect(int32_t warpEffect, bool out)
{
	switch(warpEffect)
	{
		case warpEffectZap:
			if(out) zapout();
			else zapin();
			break;
		case warpEffectWave:
			if(out) wavyout(false);
			else wavyin();
			break;
		case warpEffectInstant:
			if(out) blackscr(30,true);
			break;
		case warpEffectMozaic:
			//!TODO Unimplemented
			break;
		case warpEffectOpen:
			if(out) closescreen();
			else openscreen();
			break;
	}
}

void FFScript::queueWarp(int32_t wtype, int32_t tdm, int32_t tscr, int32_t wx, int32_t wy,
	int32_t weff, int32_t wsfx, int32_t wflag, int32_t wdir)
{
	warpex[wexActive] = 1;
	warpex[wexType] = wtype;
	warpex[wexDMap] = tdm;
	warpex[wexScreen] = tscr;
	warpex[wexX] = wx;
	warpex[wexY] = wy;
	warpex[wexEffect] = weff;
	warpex[wexSound] = wsfx;
	warpex[wexFlags] = wflag;
	warpex[wexDir] = wdir;
}

bool FFScript::warp_player(int32_t warpType, int32_t dmap, int32_t screen, int32_t warpDestX, int32_t warpDestY, int32_t warpEffect, int32_t warpSound, int32_t warpFlags, int32_t heroFacesDir)
{
	if(DEVLOGGING)
	{
		zprint("FFScript::warp_player() arg %s is: %d \n", "warpType", warpType);
		zprint("FFScript::warp_player() arg %s is: %d \n", "dmap", dmap);
		zprint("FFScript::warp_player() arg %s is: %d \n", "screen", screen);
		zprint("FFScript::warp_player() arg %s is: %d \n", "warpDestX", warpDestX);
		zprint("FFScript::warp_player() arg %s is: %d \n", "warpDestY", warpDestY);
		zprint("FFScript::warp_player() arg %s is: %d \n", "warpEffect", warpEffect);
		zprint("FFScript::warp_player() arg %s is: %d \n", "warpSound", warpSound);
		zprint("FFScript::warp_player() arg %s is: %d \n", "warpFlags", warpFlags);
		zprint("FFScript::warp_player() arg %s is: %d \n", "heroFacesDir", heroFacesDir);
	}
	if ( ((unsigned)dmap) >= MAXDMAPS ) 
	{
		Z_scripterrlog("Invalid DMap ID (%d) passed to WarpEx(). Aborting.\n", dmap);
		return false;
	}
	if ( ((unsigned)screen) >= MAPSCRS ) 
	{
		Z_scripterrlog("Invalid Screen Index (%d) passed to WarpEx(). Aborting.\n", screen);
		return false;
	}
	//Extra sanity guard. 
	if ( map_screen_index(DMaps[dmap].map, screen + DMaps[dmap].xoff) >= (int32_t)TheMaps.size() )
	{
		Z_scripterrlog("Invalid destination passed to WarpEx(). Aborting.\n");
		return false;
	}
	byte t = 0;
	t=(cur_screen<128)?0:1;
	bool overlay=false;
	bool intradmap = (dmap == cur_dmap);
	int32_t olddmap = cur_dmap;
	//if ( intradmap ) 
	//{
	//	initZScriptDMapScripts();    //Not needed.
	//}

	if ( warpType == wtNOWARP ) { Z_eventlog("Used a Cancel Warped to DMap %d: %s, screen %d", cur_dmap, DMaps[cur_dmap].name,cur_screen); return false; }
	int32_t dest_map = DMaps[dmap].map;
	int32_t mapID = dest_map + 1;
	int32_t dest_dmap_xoff = DMaps[dmap].xoff;
	int32_t dest_screen = dest_dmap_xoff + screen;
	//mapscr *m = &TheMaps[mapID * MAPSCRS + scrID]; 
	mapscr *m = &TheMaps[(zc_max((mapID)-1,0) * MAPSCRS + dest_screen)];
	if ( warpFlags&warpFlagNOSTEPFORWARD ) FFCore.temp_no_stepforward = 1;
	int32_t wx = 0, wy = 0;
	if ( warpDestX < 0 )
	{
		if(DEVLOGGING) zprint("WarpEx() was set to warp return point:%d\n", warpDestY); 
		if ( (unsigned)warpDestY < 4 )
		{
			wx = m->warpreturnx[warpDestY];
			wy = m->warpreturny[warpDestY];
			if(DEVLOGGING)
			{
				zprint("WarpEx Return Point X is: %d\n",wx);
				zprint("WarpEx Return Point Y is: %d\n",wy);
			}
		}
		else
		{
			if ( warpDestY == 5 || warpDestY < 0)
			{
				//Pit warp
				wx = Hero.getX();
				wy = Hero.getY();
			}
			else
			{
				Z_scripterrlog("Invalid Warp Return Square Type (%d) provided as an arg to Hero->WarpEx().\n",warpDestY);
				return false;
			}
		}
	}
	else 
	{
		region_t region;
		int rx, ry;
		calculate_region(dest_map, dest_screen, region, rx, ry);
		if ( (unsigned)warpDestX < region.width && (unsigned)warpDestY < region.height )
		{
			wx = warpDestX;
			wy = warpDestY;
		}
		else
		{
			Z_scripterrlog("Invalid pixel coordinates of x = %d, y = %d, supplied to Hero->WarpEx()\n",warpDestX,warpDestY);
			return false;
		}
	} 
	//warp coordinates are wx, wy, not x, y! -Z
	if ( !(warpFlags&warpFlagDONTKILLSCRIPTDRAWS) ) script_drawing_commands.Clear();
	//we also need to check if dmaps are sideview here! -Z
	//Likewise, we need to add that check to the normal Hero:;dowarp(0
	bool wasSideview = isSideViewGravity(t);
	
	//int32_t last_entr_scr = -1;
	//int32_t last_entr_dmap = -1;
	
	if ( warpType < wtEXIT ) warpType = wtIWARP; //Sanity check. We can't use wtCave, or wtPassage, with scritped warps at present.
	Hero.is_warping = true;
	switch(warpType)
	{
		case wtIWARP:
		case wtIWARPBLK:
		case wtIWARPOPEN:
		case wtIWARPZAP:
		case wtIWARPWAVE: 
		{
			bool wasswimming = (Hero.getAction()==swimming);
			bool wassideswim = (Hero.getAction()==sideswimming);
			int32_t olddiveclk = Hero.diveclk;
			if ( !(warpFlags&warpFlagDONTCLEARSPRITES) )
			{
				ALLOFF();
			}
			else FFCore.AlloffLimited(warpFlags);
			if (warpFlags&warpFlagFORCERESETMUSIC) music_stop();
			if ( !(warpFlags&warpFlagDONTKILLSOUNDS) ) kill_sfx();
			sfx(warpSound);
			if(wasswimming)
			{
				Hero.setAction(swimming); FFCore.setHeroAction(swimming);
				Hero.diveclk = olddiveclk;
			}
			if(wassideswim)
			{
				Hero.setAction(sideswimming); FFCore.setHeroAction(sideswimming);
				Hero.diveclk = 0;
			}
			doWarpEffect(warpEffect, true);
			int32_t c = DMaps[cur_dmap].color;
			bool changedlevel = false;
			bool changeddmap = false;
			if(cur_dmap != dmap)
			{
				timeExitAllGenscript(GENSCR_ST_CHANGE_DMAP);
				changeddmap = true;
			}
			if(dlevel != DMaps[dmap].level)
			{
				timeExitAllGenscript(GENSCR_ST_CHANGE_LEVEL);
				changedlevel = true;
			}
			dlevel = DMaps[dmap].level;
			cur_dmap = dmap;
			if(changeddmap)
			{
				throwGenScriptEvent(GENSCR_EVENT_CHANGE_DMAP);
			}
			if(changedlevel)
			{
				throwGenScriptEvent(GENSCR_EVENT_CHANGE_LEVEL);
			}
			cur_map = DMaps[cur_dmap].map;
			init_dmap();
			update_subscreens(dmap);
			
			ringcolor(false);
			
			if(DMaps[cur_dmap].color != c)
				loadlvlpal(DMaps[cur_dmap].color);
			
			lightingInstant(); // Also sets naturaldark
			int prev_screen = hero_screen;
			loadscr(cur_dmap, screen + DMaps[cur_dmap].xoff, -1, overlay);

			// In the case where we did not call ALLOFF, preserve the "enemies have spawned"
			// state for the new screen.
			if (warpFlags&warpFlagDONTCLEARSPRITES)
			{
				if (get_screen_state(prev_screen).loaded_enemies)
					get_screen_state(hero_screen).loaded_enemies = true;
			}
			
			Hero.x = (zfix)wx;
			Hero.y = (zfix)wy;
			update_viewport();
			
			switch(heroFacesDir)
			{
				case up:
				case down:
				case left:
				case right:
					Hero.dir = heroFacesDir;
					break;
				default:
					if((int32_t)Hero.x==(zfix)0)  
					{
						Hero.dir=right;
					}
					if((int32_t)Hero.x==(zfix)240) 
					{
						Hero.dir=left;
					}
					
					if((int32_t)Hero.y==(zfix)0)   
					{
						Hero.dir=down;
					}
					
					if((int32_t)Hero.y==(zfix)160) 
					{
						Hero.dir=up;
					}
			}
			
			markBmap(Hero.dir^1, hero_screen);
			
			if(iswaterex_z3(MAPCOMBO((int32_t)Hero.x,(int32_t)Hero.y+8), -1, Hero.x, Hero.y+8, true) && _walkflag((int32_t)Hero.x,(int32_t)Hero.y+8,0) && current_item(itype_flippers))
			{
				Hero.hopclk=0xFF;
				Hero.attackclk = Hero.charging = Hero.spins = 0;
				if (isSideViewHero() && get_qr(qr_SIDESWIM)) {Hero.setAction(sideswimming); FFCore.setHeroAction(sideswimming);}
				else {Hero.setAction(swimming); FFCore.setHeroAction(swimming);}
			}
			else
			{
				Hero.setAction(none); FFCore.setHeroAction(none);
			}
				
			//preloaded freeform combos
			ffscript_engine(true);
			
			putscr(hero_scr, scrollbuf, 0, 0);
			putscrdoors(hero_scr, scrollbuf, 0, 0);
			
			doWarpEffect(warpEffect, false);
			show_subscreen_life=true;
			show_subscreen_numbers=true;
			if (!(warpFlags&warpFlagFORCECONTINUEMUSIC)) Play_Level_Music();
			currcset=DMaps[cur_dmap].color;
			dointro();
			Hero.set_respawn_point();
			Hero.trySideviewLadder();
			
			break;
		}
		
		
		case wtEXIT:
		{
			lighting(false,false,pal_litRESETONLY);//Reset permLit, and do nothing else; lighting was not otherwise called on a wtEXIT warp.
			ALLOFF();
			if (warpFlags&warpFlagFORCERESETMUSIC) music_stop();
			if ( !(warpFlags&warpFlagDONTKILLSOUNDS) ) kill_sfx();
			sfx(warpSound);
			blackscr(30,false);
			bool changedlevel = false;
			bool changeddmap = false;
			if(cur_dmap != dmap)
			{
				timeExitAllGenscript(GENSCR_ST_CHANGE_DMAP);
				changeddmap = true;
			}
			if(dlevel != DMaps[dmap].level)
			{
				timeExitAllGenscript(GENSCR_ST_CHANGE_LEVEL);
				changedlevel = true;
			}
			dlevel = DMaps[dmap].level;
			cur_dmap = dmap;
			if(changeddmap)
			{
				throwGenScriptEvent(GENSCR_EVENT_CHANGE_DMAP);
			}
			if(changedlevel)
			{
				throwGenScriptEvent(GENSCR_EVENT_CHANGE_LEVEL);
			}
			cur_map=DMaps[cur_dmap].map;
			init_dmap();
			update_subscreens(dmap);
			loadfullpal();
			ringcolor(false);
			loadlvlpal(DMaps[cur_dmap].color);
			loadscr(cur_dmap, screen + DMaps[cur_dmap].xoff, -1, overlay);
			
			if((hero_scr->flags&fDARK) && !get_qr(qr_NEW_DARKROOM))
			{
				if(get_qr(qr_FADE))
				{
				interpolatedfade();
				}
				else
				{
				loadfadepal((DMaps[cur_dmap].color)*pdLEVEL+poFADE3);
				}
				
				darkroom=naturaldark=true;
			}
			else
			{
				darkroom=naturaldark=false;
			}
				
			
			//Move Hero's coordinates
			Hero.x = (zfix)wx;
			Hero.y = (zfix)wy;
			update_viewport();

			//set his dir
			switch(heroFacesDir)
			{
				case up:
				case down:
				case left:
				case right:
					Hero.dir = heroFacesDir;
					break;
				default:
					Hero.dir=down;
					if((int32_t)Hero.x==(zfix)0)  
					{
						Hero.dir=right;
					}
					if((int32_t)Hero.x==(zfix)240) 
					{
						Hero.dir=left;
					}
					
					if((int32_t)Hero.y==(zfix)0)   
					{
						Hero.dir=down;
					}
					
					if((int32_t)Hero.y==(zfix)160) 
					{
						Hero.dir=up;
					}
			}
			
			if(dlevel)
			{
				// reset enemy kill counts
				for(int32_t i=0; i<128; i++)
				{
					int mi = mapind(cur_map, i);
					game->guys[mi] = 0;
					game->maps[mi] &= ~mTMPNORET;
				}
			}
			
			markBmap(Hero.dir^1, hero_screen);
			//preloaded freeform combos
			ffscript_engine(true);
			Hero.reset_hookshot();
			
			if(isdungeon())
			{
				openscreen();
				if(get_er(er_SHORTDGNWALK)==0 && get_qr(qr_SHORTDGNWALK)==0)
				Hero.stepforward(Hero.diagonalMovement?11:12, false);
				else
				// Didn't walk as far pre-1.93, and some quests depend on that
				Hero.stepforward(8, false);
			}
			else
			{
				openscreen();
			}
			
			show_subscreen_life=true;
			show_subscreen_numbers=true;
			if (!(warpFlags&warpFlagFORCECONTINUEMUSIC))Play_Level_Music();
			currcset=DMaps[cur_dmap].color;
			dointro();
			Hero.set_respawn_point();
			Hero.trySideviewLadder();
			
			for(int32_t i=0; i<6; i++)
				visited[i]=-1;
				
			//last_entr_scr = scrID;
			//last_entr_dmap = dmapID;
			
			break;
			
		}
		case wtSCROLL:                                          // scrolling warp
		{
			int32_t c = DMaps[cur_dmap].color;
			scrolling_dmap = cur_dmap;
			scrolling_map = cur_map;
			cur_map = DMaps[dmap].map;
			update_subscreens(dmap);
			
			dlevel = DMaps[dmap].level;
				//check if Hero has the map for the new location before updating the subscreen. ? -Z
				//This works only in one direction, if Hero had a map, to not having one.
				//If Hero does not have a map, and warps somewhere where he does, then the map still briefly shows. 
			update_subscreens(dmap);
				
			// if ( has_item(itype_map, dlevel) ) 
			// {
				// //Blank the map during an intra-dmap scrolling warp. 
				// dlevel = -1; //a hack for the minimap. This works!! -Z
			// }
				
			// fix the scrolling direction, if it was a tile or instant warp
			Hero.sdir = vbound(Hero.dir,0,3);
			
			
			Hero.scrollscr(Hero.sdir, screen+DMaps[dmap].xoff, dmap);
			bool changedlevel = false;
			bool changeddmap = false;
			if(cur_dmap != dmap)
			{
				timeExitAllGenscript(GENSCR_ST_CHANGE_DMAP);
				changeddmap = true;
			}
			if(dlevel != DMaps[dmap].level)
			{
				timeExitAllGenscript(GENSCR_ST_CHANGE_LEVEL);
				changedlevel = true;
			}
			dlevel = DMaps[dmap].level;
			cur_dmap = dmap;
			if(changeddmap)
			{
				throwGenScriptEvent(GENSCR_EVENT_CHANGE_DMAP);
			}
			if(changedlevel)
			{
				throwGenScriptEvent(GENSCR_EVENT_CHANGE_LEVEL);
			}
			
			Hero.reset_hookshot();
			
			if(!intradmap)
			{
				if(((wx>0||wy>0)||(get_qr(qr_WARPSIGNOREARRIVALPOINT)))&&(!get_qr(qr_NOSCROLLCONTINUE))&&(!(hero_scr->flags6&fNOCONTINUEHERE)))
				{
					if(dlevel)
					{
						lastentrance = cur_screen;
					}
					else
					{
						lastentrance = DMaps[cur_dmap].cont + DMaps[cur_dmap].xoff;
					}
					
					lastentrance_dmap = dmap;
				}
			}
			
			if(DMaps[cur_dmap].color != c)
			{
				lighting(false, true);
			}
			
			if (!(warpFlags&warpFlagFORCECONTINUEMUSIC)) Play_Level_Music();
			currcset=DMaps[cur_dmap].color;
			dointro();
			break;
		}
		//Cannot use these types with scripts, or with strings. 
		case wtCAVE:
		case wtPASS:
		case wtWHISTLE:
		default: 
		{
			Z_scripterrlog("Invalid warp type (%d) supplied to Hero->WarpEx()!. Cannot warp!!\n", warpType);
			Hero.is_warping = false;
			return false;
		}
	}
	// Stop Hero from drowning!
	if(Hero.getAction()==drowning)
	{
		Hero.drownclk=0;
		Hero.setAction(none); FFCore.setHeroAction(none);
	}
		
	// But keep him swimming if he ought to be!
	if(Hero.getAction()!=rafting && iswaterex_z3(MAPCOMBO((int32_t)Hero.x,(int32_t)Hero.y+8), -1, Hero.x, Hero.y+8, true) && (_walkflag((int32_t)Hero.x,(int32_t)Hero.y+8,0) || get_qr(qr_DROWN))
			&& (current_item(itype_flippers)) && (Hero.getAction()!=inwind))
	{
		Hero.hopclk=0xFF;
		if (isSideViewHero() && get_qr(qr_SIDESWIM)) {Hero.setAction(sideswimming); FFCore.setHeroAction(sideswimming);}
		else {Hero.setAction(swimming); FFCore.setHeroAction(swimming);}
	}
		
	newscr_clk=frame;
	activated_timed_warp=false;
	eat_buttons();
		
	if(warpType!=wtIWARP) { Hero.attackclk=0; }
		
	Hero.didstuff=0;
	Hero.usecounts.clear();
	map_bkgsfx(true);
	loadside=Hero.dir^1;
	whistleclk=-1;
		
	if(((int32_t)Hero.z>0 || (int32_t)Hero.fakez>0) && isSideViewHero())
	{
		Hero.y-=Hero.z;
		Hero.y-=Hero.fakez;
		Hero.z=0;
		Hero.fakez=0;
	}
	else if(!isSideViewHero())
	{
		Hero.fall=0;
		Hero.fakefall=0;
	}
		
	// If warping between top-down and sideview screens,
	// fix enemies that are carried over by Full Screen Warp
	const bool tmpscr_is_sideview = isSideViewGravity();
		
	if(!wasSideview && tmpscr_is_sideview)
	{
		for(int32_t i=0; i<guys.Count(); i++)
		{
			if(guys.spr(i)->z > 0)
			{
				guys.spr(i)->y -= guys.spr(i)->z;
				guys.spr(i)->z = 0;
			}
			
			if(((enemy*)guys.spr(i))->family!=eeTRAP && ((enemy*)guys.spr(i))->family!=eeSPINTILE)
			guys.spr(i)->yofs += 2;
		}
	}
	else if(wasSideview && !tmpscr_is_sideview)
	{
		for(int32_t i=0; i<guys.Count(); i++)
		{
			if(((enemy*)guys.spr(i))->family!=eeTRAP && ((enemy*)guys.spr(i))->family!=eeSPINTILE)
			guys.spr(i)->yofs -= 2;
		}
	}
	if ( warpType == wtEXIT )
	{
		game->set_continue_scrn(cur_screen);
		game->set_continue_dmap(dmap);
		lastentrance = cur_screen;
		lastentrance_dmap = dmap;
	}
	else
	{
		if ( (warpFlags&warpFlagSETENTRANCESCREEN) ) lastentrance = cur_screen;
		if ( (warpFlags&warpFlagSETENTRANCEDMAP) ) lastentrance_dmap = dmap;
		if ( (warpFlags&warpFlagSETCONTINUESCREEN) ) game->set_continue_scrn(cur_screen);
		if ( (warpFlags&warpFlagSETCONTINUEDMAP) ) game->set_continue_dmap(dmap);
	}
	if(hero_scr->flags4&fAUTOSAVE)
	{
		save_game(true,0);
	}
		
	if(hero_scr->flags6&fCONTINUEHERE)
	{
		lastentrance_dmap = cur_dmap;
		lastentrance = home_screen;
	}
		
	update_subscreens();
	verifyBothWeapons();
	Z_eventlog("Warped to DMap %d: %s, screen %d, via %s.\n", cur_dmap, DMaps[cur_dmap].name,cur_screen,
						warpType==wtEXIT ? "Entrance/Exit" :
						warpType==wtSCROLL ? "Scrolling Warp" :
						warpType==wtNOWARP ? "Cancel Warp" :
						"Insta-Warp");
						
	eventlog_mapflags();
	if (((warpFlags&warpFlagDONTRESTARTDMAPSCRIPT) != 0) == (get_qr(qr_SCRIPT_WARPS_DMAP_SCRIPT_TOGGLE) != 0)|| olddmap != cur_dmap) //Changed DMaps, or needs to reset the script
	{
		FFScript::deallocateAllScriptOwned(ScriptType::DMap, olddmap);
		initZScriptDMapScripts();
	}
	Hero.is_warping = false;
	if(!get_qr(qr_SCROLLWARP_NO_RESET_FRAME))
		GameFlags |= GAMEFLAG_RESET_GAME_LOOP;
	return true;
}

void FFScript::do_adjustvolume(const bool v)
{
	if (get_qr(qr_OLD_SCRIPT_VOLUME))
	{
		int32_t perc = (SH::get_arg(sarg1, v) / 10000);
		float pct = perc / 100.0;
		float temp_midi = 0;
		float temp_digi = 0;
		float temp_mus = 0;
		if (!(coreflags & FFCORE_SCRIPTED_MIDI_VOLUME))
		{
			temp_midi = do_getMIDI_volume();
			usr_midi_volume = do_getMIDI_volume();
			SetFFEngineFlag(FFCORE_SCRIPTED_MIDI_VOLUME, true);
		}
		else
		{
			temp_midi = (float)usr_midi_volume;
		}
		if (!(coreflags & FFCORE_SCRIPTED_DIGI_VOLUME))
		{
			temp_digi = do_getDIGI_volume();
			usr_digi_volume = do_getDIGI_volume();
			SetFFEngineFlag(FFCORE_SCRIPTED_DIGI_VOLUME, true);
		}
		else
		{
			temp_digi = (float)usr_digi_volume;
		}
		if (!(coreflags & FFCORE_SCRIPTED_MUSIC_VOLUME))
		{
			temp_mus = do_getMusic_volume();
			usr_music_volume = do_getMusic_volume();
			SetFFEngineFlag(FFCORE_SCRIPTED_MUSIC_VOLUME, true);
		}
		else
		{
			temp_mus = (float)usr_music_volume;
		}

		temp_midi *= pct;
		temp_digi *= pct;
		temp_mus *= pct;
		do_setMIDI_volume((int32_t)temp_midi);
		do_setDIGI_volume((int32_t)temp_digi);
		do_setMusic_volume((int32_t)temp_mus);
	}
	else
	{
		int32_t perc = SH::get_arg(sarg1, v);
		FFCore.usr_music_volume = vbound(perc, 0, 10000 * 100);

		if (zcmusic != NULL)
		{
			if (zcmusic->playing != ZCM_STOPPED)
			{
				int32_t temp_volume = emusic_volume;
				if (!get_qr(qr_OLD_SCRIPT_VOLUME))
					temp_volume = (emusic_volume * FFCore.usr_music_volume) / 10000 / 100;
				temp_volume = (temp_volume * zcmusic->fadevolume) / 10000;
				zcmusic_play(zcmusic, temp_volume);
				return;
			}
		}
		else if (currmidi > -1)
		{
			jukebox(currmidi);
			master_volume(digi_volume, midi_volume);
		}
	}
}

void FFScript::do_adjustsfxvolume(const bool v)
{
	if (get_qr(qr_OLD_SCRIPT_VOLUME))
	{
		int32_t perc = (SH::get_arg(sarg1, v) / 10000);
		float pct = perc / 100.0;
		float temp_sfx = 0;
		if (!(coreflags & FFCORE_SCRIPTED_SFX_VOLUME))
		{
			temp_sfx = do_getSFX_volume();
			usr_sfx_volume = (int32_t)temp_sfx;
			SetFFEngineFlag(FFCORE_SCRIPTED_SFX_VOLUME, true);
		}
		else
		{
			temp_sfx = (float)usr_sfx_volume;
		}
		temp_sfx *= pct;
		do_setSFX_volume((int32_t)temp_sfx);
	}
	else
	{
		int32_t perc = SH::get_arg(sarg1, v);
		FFCore.usr_sfx_volume = vbound(perc, 0, 10000 * 100);
	}
}
	

void do_midi(bool v)
{
	int32_t MIDI = SH::get_arg(sarg1, v) / 10000;
	
	if(MIDI == 0)
		music_stop();
	else
		jukebox(MIDI + (ZC_MIDI_COUNT - 1));
}


void stop_sfx(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t sfx = (int32_t)ID;
	if(BC::checkSFXID(ID) != SH::_NoError)
		return;
	stop_sfx(sfx);
}

void pause_sfx(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t sfx = (int32_t)ID;
	if(BC::checkSFXID(ID) != SH::_NoError)
		return;
	pause_sfx(sfx);
}

void resume_sfx(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t sfx = (int32_t)ID;
	if(BC::checkSFXID(ID) != SH::_NoError)
		return;
	resume_sfx(sfx);
}



void do_enh_music(bool v)
{
	int32_t arrayptr = SH::get_arg(sarg1, v);
	int32_t track = (SH::get_arg(sarg2, v) / 10000)-1;
	
	if(arrayptr == 0)
		music_stop();
	else // Pointer to a string..
	{
		string filename_str;
		char filename_char[256];
		bool ret;
		ArrayH::getString(arrayptr, filename_str, 256);
		strncpy(filename_char, filename_str.c_str(), 255);
		filename_char[255]='\0';
		ret=try_zcmusic(filename_char, qstpath, track, -1000, get_emusic_volume());
		set_register(sarg2, ret ? 10000 : 0);
	}
}

void do_enh_music_crossfade()
{
	int32_t arrayptr = SH::read_stack(ri->sp + 5);
	int32_t track = SH::read_stack(ri->sp + 4) / 10000;
	int32_t fadeoutframes = zc_max(SH::read_stack(ri->sp + 3) / 10000, 0);
	int32_t fadeinframes = zc_max(SH::read_stack(ri->sp + 2) / 10000, 0);
	int32_t fademiddleframes = SH::read_stack(ri->sp + 1) / 10000;
	int32_t startpos = SH::read_stack(ri->sp);

	if (arrayptr == 0)
	{
		bool ret = play_enh_music_crossfade(NULL, qstpath, track, get_emusic_volume(), fadeoutframes, fadeinframes, fademiddleframes, startpos);
		ri->d[rEXP1] = ret ? 10000 : 0;
	}
	else
	{
		string filename_str;
		char filename_char[256];
		ArrayH::getString(arrayptr, filename_str, 256);
		strncpy(filename_char, filename_str.c_str(), 255);
		filename_char[255] = '\0';
		bool ret = play_enh_music_crossfade(filename_char, qstpath, track, get_emusic_volume(), fadeoutframes, fadeinframes, fademiddleframes, startpos, true);
		ri->d[rEXP1] = ret ? 10000 : 0;
	}
}

bool FFScript::doing_dmap_enh_music(int32_t dm)
{
	if (DMaps[dm].tmusic[0] != 0)
	{
		if (zcmusic != NULL)
		{
			if (strcmp(zcmusic->filename, DMaps[dm].tmusic) == 0)
			{
				switch (zcmusic_get_type(zcmusic))
				{
				case ZCMF_OGG:
				case ZCMF_MP3:
					return true;
				case ZCMF_DUH:
				case ZCMF_GME:
					if (zcmusic->track == DMaps[dm].tmusictrack)
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool FFScript::can_dmap_change_music(int32_t dm)
{
	switch (music_update_cond)
	{
	case MUSIC_UPDATE_SCREEN:
		return true;
	case MUSIC_UPDATE_DMAP:
		return dm != -1 && dm != cur_dmap;
	case MUSIC_UPDATE_LEVEL:
		return dm != -1 && DMaps[dm].level != DMaps[cur_dmap].level;
	}
	return false;
}

void FFScript::do_set_music_position(const bool v)
{
	int32_t newposition = SH::get_arg(sarg1, v);
	
	set_zcmusicpos(newposition);
}

void FFScript::do_get_music_position()
{
	int32_t pos = replay_get_state(ReplayStateType::MusicPosition, [](){
		return zcmusic_get_curpos(zcmusic);
	});
	set_register(sarg1, pos);
}

void FFScript::do_set_music_speed(const bool v)
{
	int32_t newspeed = SH::get_arg(sarg1, v);
	set_zcmusicspeed(newspeed);
}

void FFScript::do_get_music_length()
{
	int32_t len = get_zcmusiclen();
	set_register(sarg1, len);
}

void FFScript::do_set_music_loop()
{
	double start = (get_register(sarg1) / 10000.0);
	double end = (get_register(sarg2) / 10000.0);

	set_zcmusicloop(start, end);
}

void do_get_enh_music_filename(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2);
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	if(ArrayH::setArray(arrayptr, string(DMaps[ID].tmusic)) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'Game->GetDMapMusicFilename' not large enough\n");
}

void do_get_enh_music_track(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	set_register(sarg1, (DMaps[ID].tmusictrack+1)*10000);
}

void do_set_dmap_enh_music(const bool v)
{
	int32_t ID   = SH::read_stack(ri->sp + 2) / 10000;
	int32_t arrayptr = SH::read_stack(ri->sp + 1);
	int32_t track = (SH::read_stack(ri->sp + 0) / 10000)-1;
	string filename_str;
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	ArrayH::getString(arrayptr, filename_str, 56);
	strncpy(DMaps[ID].tmusic, filename_str.c_str(), 55);
	DMaps[ID].tmusic[55]='\0';
	DMaps[ID].tmusictrack=track;
}






///----------------------------------------------------------------------------------------------------//
//Array & string related

void do_arraysize()
{
	int32_t arrayptr = get_register(sarg1);
	ri->d[rEXP1] = ArrayH::getSize(arrayptr) * 10000;
}

void do_tobyte()
{
	int32_t b1 = get_register(sarg1) / 10000;
	byte b2 = b1;
	set_register(sarg1, b2 * 10000);
}

void do_tosignedbyte()
{
	int32_t b1 = get_register(sarg1) / 10000;
	signed char b2 = b1;
	set_register(sarg1, b2 * 10000);
}

void do_tointeger()
{
	int32_t b1 = get_register(sarg1) / 10000;
	set_register(sarg1, b1 * 10000);
}

void do_floor()
{
	set_register(sarg1, zslongToFix(get_register(sarg1)).doFloor().getZLong());
}

void do_trunc()
{
	set_register(sarg1, zslongToFix(get_register(sarg1)).doTrunc().getZLong());
}

void do_ceiling()
{
	set_register(sarg1, zslongToFix(get_register(sarg1)).doCeil().getZLong());
}

void do_round()
{
	set_register(sarg1, zslongToFix(get_register(sarg1)).doRound().getZLong());
}

void do_roundaway()
{
	set_register(sarg1, zslongToFix(get_register(sarg1)).doRoundAway().getZLong());
}

void do_toword()
{
	int32_t b1 = get_register(sarg1) / 10000;
	word b2 = b1;
	set_register(sarg1, b2 * 10000);
}

void do_toshort()
{
	int32_t b1 = get_register(sarg1) / 10000;
	int16_t b2 = b1;
	set_register(sarg1, b2 * 10000);
}

//Set npc and item names t.b.a. -Z

void do_getitemname()
{
	int32_t arrayptr = get_register(sarg1);
	if(unsigned(ri->idata) >= MAXITEMS)
	{
		scripting_log_error_with_context("Invalid itemdata access: {}", ri->idata);
		return;
	}
	
	if(ArrayH::setArray(arrayptr, item_string[ri->idata]) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'itemdata->GetName' not large enough\n");
}

void do_getffcscript()
{
	do_get_script_index_by_name(name_to_slot_index_ffcmap);
}

void do_getitemscript()
{
	do_get_script_index_by_name(name_to_slot_index_itemmap);
}

///----------------------------------------------------------------------------------------------------//
//Tile Manipulation

void do_copytile(const bool v, const bool v2)
{
	int32_t tile = SH::get_arg(sarg1, v) / 10000;
	int32_t tile2 = SH::get_arg(sarg2, v2) / 10000;
	
	copy_tile(newtilebuf, tile, tile2, false);
}

int32_t FFScript::IsBlankTile(int32_t i)
{
	if( ((unsigned)i) > NEWMAXTILES )
	{
		scripting_log_error_with_context("Invalid tile ID {}", i);
		return -1;
	}
	    
	byte *tilestart=newtilebuf[i].data;
	qword *di=(qword*)tilestart;
	int32_t parts=tilesize(newtilebuf[i].format)>>3;
    
	for(int32_t j=0; j<parts; ++j, ++di)
	{
		if(*di!=0)
		{
			return 0;
		}
	}
    
	return 1;
}

int32_t FFScript::Is8BitTile(int32_t i)
{
	if (((unsigned)i) > NEWMAXTILES)
	{
		scripting_log_error_with_context("Invalid tile ID {}", i);
		return -1;
	}

	return newtilebuf[i].format == tf8Bit ? 1 : 0;
}

void do_swaptile(const bool v, const bool v2)
{
	int32_t tile = SH::get_arg(sarg1, v) / 10000;
	int32_t tile2 = SH::get_arg(sarg2, v2) / 10000;
	
	copy_tile(newtilebuf, tile, tile2, true);
}

void do_overlaytile(const bool v, const bool v2)
{
	int32_t tile = SH::get_arg(sarg1, v) / 10000;
	int32_t tile2 = SH::get_arg(sarg2, v2) / 10000;
	
	if(BC::checkTile(tile) != SH::_NoError ||
			BC::checkTile(tile2) != SH::_NoError)
		return;
		
	//Could add an arg for the CSet or something instead of just passing 0, currently only 8-bit is supported
	overlay_tile(newtilebuf, tile, tile2, 0, false);
}

void do_fliprotatetile(const bool v, const bool v2)
{
	int32_t tile = SH::get_arg(sarg1, v) / 10000;
	int32_t tile2 = SH::get_arg(sarg2, v2) / 10000;
	
	if(BC::checkTile(tile) != SH::_NoError ||
			BC::checkTile(tile2) != SH::_NoError)
		return;
		
	//fliprotatetile
}

void do_settilepixel()
{
	int32_t tile = SH::read_stack(ri->sp + 3) / 10000;
	int32_t x = SH::read_stack(ri->sp + 2) / 10000;
	int32_t y = SH::read_stack(ri->sp + 1) / 10000;
	int32_t val = SH::read_stack(ri->sp + 0) / 10000;
	
	if(BC::checkTile(tile) != SH::_NoError)
		return;
		
	x = vbound(x, 0, 15);
	y = vbound(y, 0, 15);
	unpack_tile(newtilebuf, tile, 0, false);
	if (newtilebuf[tile].format == tf4Bit)
		val &= 0xF;
	unpackbuf[y * 16 + x] = val;
	pack_tile(newtilebuf, unpackbuf, tile);
}

void do_gettilepixel()
{
	int32_t tile = SH::read_stack(ri->sp + 3) / 10000;
	int32_t x = SH::read_stack(ri->sp + 2) / 10000;
	int32_t y = SH::read_stack(ri->sp + 1) / 10000;
	int32_t cs = SH::read_stack(ri->sp + 0) / 10000;

	if(BC::checkTile(tile) != SH::_NoError)
		return;
		
	x = vbound(x, 0, 15);
	y = vbound(y, 0, 15);
	unpack_tile(newtilebuf, tile, 0, false);
	int32_t csoffs = newtilebuf[tile].format == tf8Bit ? 0 : cs * 16;
	ri->d[rEXP1] = 10000 * (unpackbuf[y * 16 + x] + csoffs);
}

void do_shifttile(const bool v, const bool v2)
{
	int32_t tile = SH::get_arg(sarg1, v) / 10000;
	int32_t tile2 = SH::get_arg(sarg2, v2) / 10000;
	
	if(BC::checkTile(tile) != SH::_NoError ||
			BC::checkTile(tile2) != SH::_NoError)
		return;
		
	//shifttile
}

void do_cleartile(const bool v)
{
	int32_t tile = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkTile(tile) != SH::_NoError)
		return;
		
	reset_tile(newtilebuf, tile, newtilebuf[tile].format);
}

void do_combotile(const bool v)
{
	int32_t combo = SH::get_arg(sarg2, v) / 10000;
	
	if(BC::checkCombo(combo) != SH::_NoError)
		return;
		
	set_register(sarg1, combobuf[combo].tile * 10000);
}

void do_readpod(const bool v)
{
	int32_t indx = SH::get_arg(sarg2, v) / 10000;
	int32_t val = ArrayH::getElement(ri->d[rINDEX], indx, can_neg_array);
	set_register(sarg1, val);
}
void do_writepod(const bool v1, const bool v2)
{
	int32_t indx = SH::get_arg(sarg1, v1) / 10000;
	int32_t val = SH::get_arg(sarg2, v2);
	ArrayH::setElement(ri->d[rINDEX], indx, val, can_neg_array);
}
void do_writepodstr()
{
	if(!sargstr) return;
	uint32_t id = get_register(sarg1);
	ArrayH::setArray(id, *sargstr);
}
void do_writepodarr()
{
	if(!sargvec) return;

	uint32_t id = get_register(sarg1);
	ArrayH::setArray(id, sargvec->size(), sargvec->data(), false);
}

int32_t get_own_i(ScriptType type)
{
	switch(type)
	{
		case ScriptType::Lwpn:
			return ri->lwpn;
		case ScriptType::Ewpn:
			return ri->ewpn;
		case ScriptType::ItemSprite:
			return ri->itemref;
		case ScriptType::NPC:
			return ri->guyref;
		case ScriptType::FFC:
			if (auto ffc = ResolveFFC(ri->ffcref))
				return ffc->index;
	}
	return 0;
}

portal* loadportal(savedportal& p);

///----------------------------------------------------------------------------------------------------//
//                                       Run the script                                                //
///----------------------------------------------------------------------------------------------------//

static bool check_cmp(uint cmp)
{
	if(cmp & CMP_BOOL)
	{
		if(ri->cmp_strcache) return false; //Cast string to bool? nonsense...
		switch(cmp & CMP_FLAGS)
		{
			case CMP_EQ:
				return !ri->cmp_op1 == !ri->cmp_op2;
			case CMP_NE:
				return !ri->cmp_op1 != !ri->cmp_op2;
		}
		return false;
	}
	else if(ri->cmp_strcache)
	{
		if(*ri->cmp_strcache < 0)
			return (cmp & CMP_LT);
		if(*ri->cmp_strcache > 0)
			return (cmp & CMP_GT);
		return (cmp & CMP_EQ);
	}
	else
	{
		if(cmp & CMP_GT)
			if(ri->cmp_op1 > ri->cmp_op2)
				return true;
		if(cmp & CMP_LT)
			if(ri->cmp_op1 < ri->cmp_op2)
				return true;
		if(cmp & CMP_EQ)
			if(ri->cmp_op1 == ri->cmp_op2)
				return true;
		return false;
	}
}

static void markRegisterType(int reg, int type)
{
	// Currently only marking globals as objects is supported.
	if (!(reg >= GD(0) && reg <= GD(MAX_SCRIPT_REGISTERS)))
	{
		assert(false);
	}
	if (!(type >= 0 && type <= (int)script_object_type::last))
	{
		assert(false);
	}

	int index = reg - GD(0);
	game->global_d_types[index] = (script_object_type)type;
}

static void markGlobalRegisters()
{
	word scommand;
	auto& init_script = *globalscripts[GLOBAL_SCRIPT_INIT];
	if (!init_script.valid())
		return;

	auto& zasm = init_script.zasm_script->zasm;
	uint32_t start_pc = init_script.pc, end_pc = init_script.end_pc;

	for (auto pc = start_pc; pc < end_pc; pc++)
	{
		scommand = zasm[pc].command;
		if(scommand == MARK_TYPE_REG)
			markRegisterType(zasm[pc].arg1, zasm[pc].arg2);
	}
}

void goto_err(char const* opname)
{
	auto i = curScriptIndex;
	const char* type_str = ScriptTypeToString(curScriptType);
	switch(curScriptType)
	{
		case ScriptType::FFC:
			Z_scripterrlog("%s Script %s attempted to %s an invalid jump to (%d).\n", type_str, ffcmap[i].scriptname.c_str(), opname, sarg1); break;
		case ScriptType::NPC:
			Z_scripterrlog("%s Script %s attempted to %s an invalid jump to (%d).\n", type_str, npcmap[i].scriptname.c_str(), opname, sarg1); break;
		case ScriptType::Lwpn:
			Z_scripterrlog("%s Script %s attempted to %s an invalid jump to (%d).\n", type_str, lwpnmap[i].scriptname.c_str(), opname, sarg1); break;
		case ScriptType::Ewpn:
			Z_scripterrlog("%s Script %s attempted to %s an invalid jump to (%d).\n", type_str, ewpnmap[i].scriptname.c_str(), opname, sarg1); break;
		case ScriptType::ItemSprite:
			Z_scripterrlog("%s Script %s attempted to %s an invalid jump to (%d).\n", type_str, itemspritemap[i].scriptname.c_str(), opname, sarg1); break;
		case ScriptType::Item:
			Z_scripterrlog("%s Script %s attempted to %s an invalid jump to (%d).\n", type_str, itemmap[i].scriptname.c_str(), opname, sarg1); break;
		case ScriptType::Global:
			Z_scripterrlog("%s Script %s attempted to %s an invalid jump to (%d).\n", type_str, globalmap[i].scriptname.c_str(), opname, sarg1); break;
		case ScriptType::Hero:
			Z_scripterrlog("%s Script %s attempted to %s an invalid jump to (%d).\n", type_str, playermap[i].scriptname.c_str(), opname, sarg1); break;
		case ScriptType::Screen:
			Z_scripterrlog("%s Script %s attempted to %s an invalid jump to (%d).\n", type_str, screenmap[i].scriptname.c_str(), opname, sarg1); break;
		case ScriptType::OnMap:
		case ScriptType::DMap:
		case ScriptType::ScriptedActiveSubscreen:
		case ScriptType::ScriptedPassiveSubscreen:
			Z_scripterrlog("%s Script %s attempted to %s an invalid jump to (%d).\n", type_str, dmapmap[i].scriptname.c_str(), opname, sarg1); break;
		case ScriptType::Combo:
			Z_scripterrlog("%s Script %s attempted to %s an invalid jump to (%d).\n", type_str, comboscriptmap[i].scriptname.c_str(), opname, sarg1); break;
		
		default: break;
	}
}

static void script_exit_cleanup(bool no_dealloc)
{
	ScriptType type = curScriptType;
	word script = curScriptNum;
	int32_t i = curScriptIndex;

	switch(type)
	{
		case ScriptType::FFC:
		{
			if (auto ffc = ResolveFFCWithID(i))
				ffc->script = 0;
			auto& data = get_script_engine_data(type, i);
			data.doscript = false;
			data.clear_ref();
		}
		break;

		case ScriptType::Screen:
			get_scr(i)->script = 0;
		case ScriptType::Global:
		case ScriptType::Hero:
		case ScriptType::DMap:
		case ScriptType::OnMap:
		case ScriptType::ScriptedActiveSubscreen:
		case ScriptType::ScriptedPassiveSubscreen:
		case ScriptType::EngineSubscreen:
		case ScriptType::Combo:
		{
			auto& data = get_script_engine_data(type, i);
			data.doscript = false;
			data.clear_ref();
		}
		break;
		case ScriptType::Ewpn:
		case ScriptType::Lwpn:
		case ScriptType::NPC:
		case ScriptType::ItemSprite:
		{
			auto& data = get_script_engine_data(type, i);
			data.doscript = false;
			data.clear_ref();
			if (auto spr = sprite::getByUID(i))
				spr->script = 0;
		}
		break;
		
		case ScriptType::Generic:
			user_genscript::get(script).quit();
			break;
		
		case ScriptType::GenericFrozen:
		{
			// TODO use `i`?
			auto& data = get_script_engine_data(type, gen_frozen_index-1);
			data.doscript = false;
			data.clear_ref();
			break;
		}

		case ScriptType::Item:
		{
			bool collect = ( ( i < 1 ) || (i == COLLECT_SCRIPT_ITEM_ZERO) );
			auto& data = get_script_engine_data(type, i);
			if ( !collect )
			{
				if ( (itemsbuf[i].flags&item_passive_script) && game->item[i] ) itemsbuf[i].script = 0; //Quit perpetual scripts, too.
			}
			data.doscript = 0;
			data.clear_ref();
			break;
		}
	}
	if(!no_dealloc)
		switch(type)
		{
			case ScriptType::Item:
			{
				bool collect = ( ( i < 1 ) || (i == COLLECT_SCRIPT_ITEM_ZERO) );
				int new_i = ( collect ) ? (( i != COLLECT_SCRIPT_ITEM_ZERO ) ? (i * -1) : 0) : i;
				FFScript::deallocateAllScriptOwned(type, new_i);
				break;
			}
			
			default:
				FFScript::deallocateAllScriptOwned(type, i);
				break;
		}
}

int32_t run_script(ScriptType type, word script, int32_t i)
{
	if(Quit==qRESET || Quit==qEXIT) // In case an earlier script hung
		return RUNSCRIPT_ERROR;
		
	if(type != ScriptType::Global && !script) return RUNSCRIPT_OK; //Safeguard against running null scripts
	
	combopos_modified = -1;
	curScriptType=type;
	curScriptNum=script;
	curScriptIndex=i;
	current_zasm_register=0;
	//numInstructions=0; //DON'T CLEAR THIS OR IT CAN HARDLOCK! -Em

	if (!(type >= ScriptType::First && type <= ScriptType::Last))
	{
		al_trace("Invalid script type: %d\n", (int)type);
		return RUNSCRIPT_ERROR;
	}

	auto& data = get_script_engine_data(type, i);
	bool got_initialized = set_current_script_engine_data(data, type, script, i);

	// Because qst.cpp likes to write script_data without setting this.
	curscript->meta.script_type = type;

	// If script isn't valid, we don't have a `pc` to start from... just exit.
	if(!curscript->valid())
	{
		script_exit_cleanup(false);
		return RUNSCRIPT_OK;
	}

	script_funcrun = false;

	JittedScriptHandle* jitted_script = nullptr;
	if (jit_is_enabled())
	{
		auto& data = get_script_engine_data(type, i);
		if (!data.jitted_script)
			data.jitted_script = std::shared_ptr<JittedScriptHandle>(jit_create_script_handle(curscript, ri, got_initialized));
		jitted_script = data.jitted_script.get();
	}

	runtime_script_debug_handle = nullptr;
	if (script_debug_is_runtime_debugging())
	{
		if (!script_debug_handles.contains(curscript->id))
		{
			script_debug_handles.emplace(curscript->id, ScriptDebugHandle(
				curscript->zasm_script.get(), ScriptDebugHandle::OutputSplit::ByFrame, curscript->name()));
		}
		runtime_script_debug_handle = &script_debug_handles.at(curscript->id);
		runtime_script_debug_handle->update_file();
		std::string line = fmt::format("=== running script type: {} index: {} name: {} i: {} script: {}", ScriptTypeToString(curscript->id.type), curscript->id.index, curscript->meta.script_name, i, script);
		runtime_script_debug_handle->print("\n");
		runtime_script_debug_handle->print(line.c_str());
		runtime_script_debug_handle->print("\n");

		replay_step_comment(line);
	}
	if (script_debug_is_runtime_debugging() == 1)
	{
		std::string line = script_debug_registers_and_stack_to_string();
		runtime_script_debug_handle->print(line.c_str());
		runtime_script_debug_handle->print("\n");

		util::replchar(line, '\n', ' ');
		replay_step_comment(line);
	}

	int32_t result;
	if (jitted_script)
	{
		if (ri->waitframes)
		{
			--ri->waitframes;
			result = RUNSCRIPT_OK;
		}
		else
		{
			// Retain the script handler because if deleted while running, terrible things can happen (crash),
			// as the jit runtimes often write to it. Typically a script won't delete its own script handle,
			// but scripts can run nested, so lets capture a temporary retaining reference as part of the
			// call stack.
			auto retainer = data.jitted_script;
			result = jit_run_script(jitted_script);

			if (result == RUNSCRIPT_JIT_STACK_OVERFLOW || result == RUNSCRIPT_JIT_CALL_LIMIT)
			{
				ri->overflow = true;
				if (result == RUNSCRIPT_JIT_STACK_OVERFLOW)
					log_stack_overflow_error();
				else
					log_call_limit_error();

				if (!(script_funcrun && curscript->meta.ffscript_v < 23))
				{
					script_exit_cleanup(false);
					result = RUNSCRIPT_STOPPED;
				}
				else
				{
					result = RUNSCRIPT_OK;
				}
			}
		}
	}
	else
	{
		result = run_script_int(false);
	}

	if (ZScriptVersion::gc())
	{
		// Drain the autorelease pool.
		// Move the vector, since destructors can possibly
		// create objects and modify `script_object_autorelease_pool`.
		auto ids = std::move(script_object_autorelease_pool);
		for (auto id : ids)
			script_object_ref_dec(id);

		// This throttles the actual full GC run.
		maybe_run_gc();
	}

	if (replay_is_active() && replay_get_meta_bool("debug_script_state"))
	{
		std::string str = script_debug_registers_and_stack_to_string();
		util::replstr(str, "\n", " ");
		replay_step_comment(str);
	}

	if (runtime_script_debug_handle)
	{
		runtime_script_debug_handle->print(fmt::format("result: {}\n", result).c_str());
		replay_step_comment(fmt::format("result: {}", result));
	}
	return result;
}

int32_t run_script_int(bool is_jitted)
{
	ScriptType type = curScriptType;
	word script = curScriptNum;
	int32_t i = curScriptIndex;

	current_zasm_command=(ASM_DEFINE)0; // this is actually SETV, but we never will print that as a context string, so it's fine.

	int commands_run = 0;
	int jit_waiting_nop = false;
	bool old_script_funcrun = script_funcrun && curscript->meta.ffscript_v < 23;
	if(!is_jitted)
	{
		if(ri->waitframes)
		{
			--ri->waitframes;
			return RUNSCRIPT_OK;
		}
		zs_vargs.clear();
		
	#ifdef _FFDISSASSEMBLY
		
		if(curscript->zasm[ri->pc].command != 0xFFFF)
		{
	#ifdef _FFONESCRIPTDISSASSEMBLY
			zc_trace_clear();
	#endif
			
			switch(type)
			{
			case ScriptType::FFC:
				al_trace("\nStart of FFC script %i processing on FFC %i:\n", script, i);
				break;
				
			case ScriptType::Item:
				al_trace("\nStart of item script %i processing:\n", script);
				break;
				
			case ScriptType::Global:
				al_trace("\nStart of global script %I processing:\n", script);
				break;
			}
		}
		
	#endif
	}
	//j_command
	bool is_debugging = script_debug_is_runtime_debugging() == 2;
	bool increment = true;
	static std::vector<ffscript> empty_zasm = {{0xFFFF}};
	const auto& zasm = curscript->valid() ? curscript->zasm_script->zasm : empty_zasm;
	word scommand = zasm[ri->pc].command;
	bool hit_invalid_zasm = false;
	bool no_dealloc = false;
	while(scommand != 0xFFFF)
	{
		const auto& op = zasm[ri->pc];
		scommand = op.command;
		sarg1 = op.arg1;
		sarg2 = op.arg2;
		sarg3 = op.arg3;
		sargstr = op.strptr;
		sargvec = op.vecptr;

		current_zasm_command = (ASM_DEFINE)op.command;

		if (is_debugging && (!is_jitted || commands_run > 0))
		{
			runtime_script_debug_handle->pre_command();
		}

		bool waiting = true;
		switch(scommand) //Handle waitframe-type commands first
		{
			case WAITDRAW:
			{
				if(script_funcrun)
					scommand = NOP;
				else switch(type)
				{
					case ScriptType::EngineSubscreen: //ignore waitdraws
						Z_scripterrlog("'Waitdraw()' is invalid in subscreen scripts, will be ignored\n");
						scommand = NOP;
						break;
					case ScriptType::Generic:
					case ScriptType::GenericFrozen: //ignore waitdraws
						Z_scripterrlog("'Waitdraw()' is invalid in generic scripts, will be ignored\n");
						scommand = NOP;
						break;
				}
				break;
			}
			case WAITTO:
			{
				if(script_funcrun)
					scommand = NOP;
				else switch(type)
				{
					case ScriptType::GenericFrozen:
						//ignore, no warn/error
						scommand = NOP;
						break;
					case ScriptType::Generic:
					{
						user_genscript& scr = user_genscript::get(script);
						int32_t target = get_register(sarg1)/10000L;
						bool atleast = get_register(sarg2)!=0;
						if(unsigned(target) > SCR_TIMING_END_FRAME)
						{
							Z_scripterrlog("Invalid value '%d' provided to 'WaitTo()'\n", target);
							scommand = NOP;
							break;
						}
						if(genscript_timing == target ||
							(atleast && genscript_timing < target))
						{
							//Already that time, skip the command
							scommand = NOP;
							break;
						}
						scr.waituntil = scr_timing(target);
						scr.wait_atleast = atleast;
						break;
					}
					default:
						Z_scripterrlog("'WaitTo()' is only valid in 'generic' scripts!\n");
						scommand = NOP;
						break;
				}
				break;
			}
			case WAITEVENT:
			{
				if(script_funcrun)
					scommand = NOP;
				else switch(type)
				{
					case ScriptType::GenericFrozen:
						scommand = WAITFRAME;
						ri->d[0] = GENSCR_EVENT_NIL*10000; //no event
						break;
					case ScriptType::Generic:
					{
						user_genscript& scr = user_genscript::get(script);
						scr.waitevent = true;
						break;
					}
					default:
						Z_scripterrlog("'WaitEvent()' is only valid in 'generic' scripts!\n");
						scommand = NOP;
						break;
				}
				break;
			}
			case WAITFRAME:
			{
				if(script_funcrun)
					scommand = NOP;
				else switch(type)
				{
					case ScriptType::Generic:
					{
						user_genscript& scr = user_genscript::get(script);
						scr.waituntil = SCR_TIMING_START_FRAME;
						scr.wait_atleast = false;
						break;
					}
				}
				break;
			}
			case WAITFRAMESR:
			{
				auto count = get_register(sarg1);
				if(script_funcrun || count <= 0)
				{
					scommand = NOP;
					break;
				}
				auto frames = count/10000;
				if(count%10000) ++frames; //round up decimals
				ri->waitframes = frames-1; //this frame doesn't count
				switch(type)
				{
					case ScriptType::Generic:
					{
						user_genscript& scr = user_genscript::get(script);
						scr.waituntil = SCR_TIMING_START_FRAME;
						scr.wait_atleast = false;
						break;
					}
				}
				break;
			}
			default: waiting = false;
		}
		if(waiting && scommand != NOP)
		{
			if (is_jitted)
				jit_waiting_nop = true;
			break;
		}
		
		numInstructions++;
		if(numInstructions==hangcount) // No need to check frequently
		{
			numInstructions=0;
			poll_keyboard();
			checkQuitKeys();
			if(Quit)
				scommand=0xFFFF;
		}
		
		switch(scommand)
		{
			//always first
			case 0xFFFF:  //invalid command
			{
				const char* type_str = ScriptTypeToString(type);
				switch(type)
				{
					case ScriptType::FFC:
						zprint("%s Script %s has exited.\n", type_str, ffcmap[i].scriptname.c_str()); break;
					case ScriptType::NPC:
						zprint("%s Script %s has exited.\n", type_str, npcmap[i].scriptname.c_str()); break;
					case ScriptType::Lwpn:
						zprint("%s Script %s has exited.\n", type_str, lwpnmap[i].scriptname.c_str()); break;
					case ScriptType::Ewpn:
						zprint("%s Script %s has exited.\n", type_str, ewpnmap[i].scriptname.c_str()); break;
					case ScriptType::ItemSprite:
						zprint("%s Script %s has exited.\n", type_str, itemspritemap[i].scriptname.c_str()); break;
					case ScriptType::Item:
						zprint("%s Script %s has exited.\n", type_str, itemmap[i].scriptname.c_str()); break;
					case ScriptType::Global:
						zprint("%s Script %s has exited.\n", type_str, globalmap[i].scriptname.c_str()); break;
					case ScriptType::Hero:
						zprint("%s Script %s has exited.\n", type_str, playermap[i].scriptname.c_str()); break;
					case ScriptType::Screen:
						zprint("%s Script %s has exited.\n", type_str, screenmap[i].scriptname.c_str()); break;
					case ScriptType::OnMap:
					case ScriptType::DMap:
					case ScriptType::ScriptedActiveSubscreen:
					case ScriptType::ScriptedPassiveSubscreen:
						zprint("%s Script %s has exited.\n", type_str, dmapmap[i].scriptname.c_str()); break;
					case ScriptType::Combo: zprint("%s Script %s has exited.\n", type_str, comboscriptmap[i].scriptname.c_str()); break;
					
					default: break;					
				}
				break;
			}
			case QUIT:
				scommand = 0xFFFF;
				break;
			case QUIT_NO_DEALLOC:
				scommand = 0xFFFF;
				no_dealloc = true;
				break;
				
			case NOP: //No Operation. Do nothing. -Em
			{
				// While we are here, skip many NOPs in a row to avoid the overhead
				// of the interpreter loop. This is especially good for how `zasm_optimize`
				// works, since it replaces many commands with a sequence of NOPs.
				// No need to do a bounds check - the last command should always be 0xFFFF.
				if (is_debugging)
					break;
				while (zasm[ri->pc + 1].command == NOP)
					ri->pc++;
				break;
			}
			case GOTO:
			{
				if(sarg1 < 0 )
				{
					goto_err("GOTO");
					scommand = 0xFFFF;
					break;
				}
				ri->pc = sarg1;
				increment = false;
				break;
			}
			case GOTOR:
			{
				if(sarg1 < 0 )
				{
					goto_err("GOTOR");
					scommand = 0xFFFF;
					break;
				}
				ri->pc = (get_register(sarg1) / 10000) - 1;
				increment = false;
			}
			break;
			
			case GOTOTRUE:
				if(check_cmp(CMP_EQ))
				{
					if(sarg1 < 0 )
					{
						goto_err("GOTOTRUE");
						scommand = 0xFFFF;
						break;
					}
					ri->pc = sarg1;
					increment = false;
				}
				break;
				
			case GOTOFALSE:
				if(check_cmp(CMP_NE))
				{
					if(sarg1 < 0 )
					{
						goto_err("GOTOFALSE");
						scommand = 0xFFFF;
						break;
					}
					ri->pc = sarg1;
					increment = false;
				}
				break;
				
			case GOTOMORE:
				if(check_cmp(CMP_GE))
				{
					if(sarg1 < 0 )
					{
						goto_err("GOTOMORE");
						scommand = 0xFFFF;
						break;
					}
					ri->pc = sarg1;
					increment = false;
				}
				break;
				
			case GOTOLESS:
				if(check_cmp(get_qr(qr_GOTOLESSNOTEQUAL) ? CMP_LT : CMP_LE))
				{
					if(sarg1 < 0 )
					{
						goto_err("GOTOLESS");
						scommand = 0xFFFF;
						break;
					}
					ri->pc = sarg1;
					increment = false;
				}
				break;
			
			case GOTOCMP:
			{
				bool run = check_cmp(sarg2);
				if(run)
				{
					if(sarg1 < 0 )
					{
						goto_err("GOTOCMP");
						scommand = 0xFFFF;
						break;
					}
					ri->pc = sarg1;
					increment = false;
				}
				break;
			}
			
			case SETCMP:
			{
				bool run = check_cmp(sarg2);
				set_register(sarg1, run ? ((sarg2 & CMP_SETI) ? 10000 : 1) : 0);
				break;
			}
			
			case CALLFUNC:
			{
				retstack_push(ri->pc+1);
				if(sarg1 < 0 )
				{
					goto_err("CALLFUNC");
					scommand = 0xFFFF;
					break;
				}
				ri->pc = sarg1;
				increment = false;
				break;
			}
			case RETURNFUNC:
			{
				if(auto retpc = retstack_pop())
				{
					if(*retpc < 0)
					{
						goto_err("RETURNFUNC");
						scommand = 0xFFFF;
						break;
					}
					ri->pc = *retpc;
					increment = false;
				}
				else //Returned from 'void run()', QUIT
				{
					scommand = 0xFFFF;
				}
				break;
			}
				
			case LOOP:
			{
				if(get_register(sarg2) > 0)
				{
					ri->pc = sarg1;
					increment = false;
				}
				else
				{
					set_register(sarg1, sarg1 - 1);
				}
			}
			break;

			case RETURN:
			{
				if (script_funcrun)
					break; //handled below, poorly. 'RETURNFUNC' does this better now.
				ri->pc = SH::read_stack(ri->sp) - 1;
				++ri->sp;
				increment = false;
				break;
			}
			
			case SETTRUE:
				set_register(sarg1, check_cmp(CMP_EQ) ? 1 : 0);
				break;
				
			case SETFALSE:
				set_register(sarg1, check_cmp(CMP_NE) ? 1 : 0);
				break;
				
			case SETMORE:
				set_register(sarg1, check_cmp(CMP_GE) ? 1 : 0);
				break;
				
			case SETLESS:
				set_register(sarg1, check_cmp(CMP_LE) ? 1 : 0);
				break;
			
			case SETTRUEI:
				set_register(sarg1, check_cmp(CMP_EQ) ? 10000 : 0);
				break;
				
			case SETFALSEI:
				set_register(sarg1, check_cmp(CMP_NE) ? 10000 : 0);
				break;
				
			case SETMOREI:
				set_register(sarg1, check_cmp(CMP_GE) ? 10000 : 0);
				break;
				
			case SETLESSI:
				set_register(sarg1, check_cmp(CMP_LE) ? 10000 : 0);
				break;
	
			case READPODARRAYR:
			{
				do_readpod(false);
				break;
			}
			case READPODARRAYV:
			{
				do_readpod(true);
				break;
			}
			case WRITEPODARRAYRR:
			{
				do_writepod(false,false);
				break;
			}
			case WRITEPODARRAYRV:
			{
				do_writepod(false,true);
				break;
			}
			case WRITEPODARRAYVR:
			{
				do_writepod(true,false);
				break;
			}
			case WRITEPODARRAYVV:
			{
				do_writepod(true,true);
				break;
			}
			case WRITEPODSTRING:
			{
				do_writepodstr();
				break;
			}
			case WRITEPODARRAY:
			{
				do_writepodarr();
				break;
			}
			
			case NOT:
				do_not(false);
				break;
				
			case COMPAREV:
				do_comp(true);
				break;
			case COMPAREV2:
				do_comp(true,true);
				break;
				
			case COMPARER:
				do_comp(false);
				break;
			
			case STRCMPR:
				do_internal_strcmp();
				break;
			
			case STRICMPR:
				do_internal_stricmp();
				break;
				
			case SETV:
				do_set_command(true);
				break;
				
			case SETR:
				do_set_command(false);
				break;
				
			case PUSHR:
				do_push(false);
				break;
				
			case PUSHV:
				do_push(true);
				break;
				
			case PEEK:
				do_peek();
				break;
			case PEEKATV:
				do_peekat(true);
				break;
			case STACKWRITEATRV:
				do_writeat(false, true);
				break;
			case STACKWRITEATVV_IF:
				if(!check_cmp(sarg3))
					break;
			[[fallthrough]];
			case STACKWRITEATVV:
				do_writeat(true, true);
				break;
			case POP:
				do_pop();
				break;
			
			case POPARGS:
				do_pops();
				break;
			
			case PUSHARGSR:
				do_pushs(false);
				break;
			
			case PUSHARGSV:
				do_pushs(true);
				break;
				
			case LOADI:
				do_loadi();
				break;
				
			case STOREI:
				do_storei();
				break;
				
			case LOADD:
				do_loadd();
				break;

			case LOAD:
				do_load();
				break;
				
			case STORED:
				do_stored(false);
				break;
			case STOREDV:
				do_stored(true);
				break;
			case STORE:
				do_store(false);
				break;
			case STOREV:
				do_store(true);
				break;
			case STORE_OBJECT:
				do_store_object(false);
				break;
				
			// Note: this was never used.
			case ALLOCATEGMEMR:
				if(type == ScriptType::Global) do_allocatemem(false, false, type, i);
				
				break;
				
			case ALLOCATEGMEMV:
				if(type == ScriptType::Global) do_allocatemem(true, false, type, i);
				
				break;
				
			case ALLOCATEMEMR:
				do_allocatemem(false, true, type, i);
				break;
				
			case ALLOCATEMEMV:
				do_allocatemem(true, true, type, i);
				break;
			
			case RESIZEARRAYR:
				do_resize_array();
				break;
			case OWNARRAYR:
				do_own_array(get_register(sarg1), type, i);
				break;
			case DESTROYARRAYR:
				do_destroy_array();
				break;

			// Pre-3.0, the compiler inserted this command for every local array at the end of it scope.
			case DEALLOCATEMEMR:
				do_deallocatemem();
				break;

			case SAVEGAMESTRUCTS:
				using_SRAM = 1;
				FFCore.do_savegamestructs(false,false);
				using_SRAM = 0;
				break;
			case READGAMESTRUCTS:
				using_SRAM = 1;
				FFCore.do_loadgamestructs(false,false);
				using_SRAM = 0;
				break;
			case ARRAYSIZE:
				do_arraysize();
				break;
			
			case GETFFCSCRIPT:
				do_getffcscript();
				break;
			case GETITEMSCRIPT:
				do_getitemscript();
				break;

			case LOAD_FFC:
			{
				if (!ZScriptVersion::ffcRefIsSpriteId())
				{
					set_register(sarg1, get_register(sarg1) - 10000);
					break;
				}

				int ffc_id = get_register(sarg1) / 10000 - 1;
				if (auto ffc = ResolveFFCWithID(ffc_id))
					set_register(sarg1, ffc->getUID());
				else
					set_register(sarg1, 0);
				break;
			}

			case LOAD_FFC_2:
			{
				if (!ZScriptVersion::ffcRefIsSpriteId())
				{
					set_register(sarg1, get_register(sarg2) - 10000);
					break;
				}

				int screen = get_register(sarg1) / 10000;
				int index = get_register(sarg2) / 10000;

				if (!is_in_current_region(screen))
				{
					scripting_log_error_with_context("Must use a screen in the current region. got: {}", screen);
					break;
				}
				if (BC::checkMapdataFFC(index) != SH::_NoError)
					break;

				ffc_id_t ffc_id = get_region_screen_offset(screen)*MAXFFCS + index;
				if (auto ffc = ResolveFFCWithID(ffc_id))
					set_register(sarg1, ffc->getUID());
				else
					set_register(sarg1, 0);

				break;
			}

			case CASTBOOLI:
				do_boolcast(false);
				break;
				
			case CASTBOOLF:
				do_boolcast(true);
				break;
				
			case ADDV:
				do_add(true);
				break;
				
			case ADDR:
				do_add(false);
				break;
				
			case SUBV:
				do_sub(true);
				break;
			case SUBV2:
				do_sub(true,true);
				break;
				
			case SUBR:
				do_sub(false);
				break;
				
			case MULTV:
				do_mult(true);
				break;
				
			case MULTR:
				do_mult(false);
				break;
				
			case DIVV:
				do_div(true);
				break;
			case DIVV2:
				do_div(true,true);
				break;
				
			case DIVR:
				do_div(false);
				break;
				
			case MODV:
				do_mod(true);
				break;
			case MODV2:
				do_mod(true,true);
				break;
				
			case MODR:
				do_mod(false);
				break;
				
			case SINV:
				do_trig(true, 0);
				break;
				
			case SINR:
				do_trig(false, 0);
				break;
				
			case COSV:
				do_trig(true, 1);
				break;
				
			case COSR:
				do_trig(false, 1);
				break;
				
			case TANV:
				do_trig(true, 2);
				break;
				
			case TANR:
				do_trig(false, 2);
				break;
				
			case DEGTORAD:
				do_degtorad();
				break;
				
			case RADTODEG:
				do_radtodeg();
				break;
			
			case STRINGLENGTH:
				FFCore.do_strlen(false);
				break;
				
			case ARCSINR:
				do_asin(false);
				break;
				
			case ARCCOSR:
				do_acos(false);
				break;
				
			case ARCTANR:
				do_arctan();
				break;
			
			//Text ptr functions
			case FONTHEIGHTR:
				do_fontheight();
				break;
			case STRINGWIDTHR:
				do_strwidth();
				break;
			case CHARWIDTHR:
				do_charwidth();
				break;
			case MESSAGEWIDTHR:
				ri->d[rEXP1] = 10000* do_msgwidth(get_register(sarg1)/10000);
				break;
			case MESSAGEHEIGHTR:
				ri->d[rEXP1] = 10000* do_msgheight(get_register(sarg1)/10000);
				break;
			//

			case COMBO_AT:
			{
				int32_t x = get_register(sarg1) / 10000;
				int32_t y = get_register(sarg2) / 10000;
				x = std::clamp(x, 0, world_w - 1);
				y = std::clamp(y, 0, world_h - 1);
				set_register(sarg1, (int)COMBOPOS_REGION(x, y) * 10000);
				break;
			}

			case COMBO_ADJUST:
			{
				rpos_t rpos = (rpos_t)(get_register(sarg1) / 10000);
				if (!is_valid_rpos(rpos))
				{
					set_register(sarg1, -1);
					break;
				}

				auto [x, y] = COMBOXY_REGION(rpos);
				x += get_register(sarg2) / 10000;
				y += get_register(sarg3) / 10000;
				x = std::clamp(x, 0, world_w - 1);
				y = std::clamp(y, 0, world_h - 1);
				set_register(sarg1, (int)COMBOPOS_REGION(x, y) * 10000);
				break;
			}

			//String.h functions 2.55 Alpha 23
			case STRINGCOMPARE: FFCore.do_strcmp(); break;
			case STRINGICOMPARE: FFCore.do_stricmp(); break;
			case STRINGCOPY: FFCore.do_strcpy(false,false); break;
			case ARRAYCOPY: FFCore.do_arraycpy(false,false); break;
			case STRINGNCOMPARE: FFCore.do_strncmp(); break;
			case STRINGNICOMPARE: FFCore.do_strnicmp(); break;
			
			//More string.h functions, 19th May, 2019 
			case XLEN: FFCore.do_xlen(false); break;
			case XTOI: FFCore.do_xtoi(false); break;
			case ILEN: FFCore.do_ilen(false); break;
			case ATOI: FFCore.do_atoi(false); break;
			case ATOL: FFCore.do_atol(false); break;
			case STRCSPN: FFCore.do_strcspn(); break;
			case STRSTR: FFCore.do_strstr(); break;
			case XTOA: FFCore.do_xtoa(); break;
			case ITOA: FFCore.do_itoa(); break;
			case ITOACAT: FFCore.do_itoacat(); break;
			case STRCAT: FFCore.do_strcat(); break;
			case STRSPN: FFCore.do_strspn(); break;
			case STRCHR: FFCore.do_strchr(); break;
			case STRRCHR: FFCore.do_strrchr(); break;
			case XLEN2: FFCore.do_xlen2(); break;
			case XTOI2: FFCore.do_xtoi2(); break;
			case ILEN2: FFCore.do_ilen2(); break;
			case ATOI2: FFCore.do_atoi2(); break;
			case REMCHR2: FFCore.do_remchr2(); break;
			case UPPERTOLOWER: FFCore.do_UpperToLower(false); break;
			case LOWERTOUPPER: FFCore.do_LowerToUpper(false); break;
			case CONVERTCASE: FFCore.do_ConvertCase(false); break;
				
			case GETNPCSCRIPT:	FFCore.do_getnpcscript(); break;
			case GETCOMBOSCRIPT:	FFCore.do_getcomboscript(); break;
			case GETLWEAPONSCRIPT:	FFCore.do_getlweaponscript(); break;
			case GETEWEAPONSCRIPT:	FFCore.do_geteweaponscript(); break;
			case GETHEROSCRIPT:	FFCore.do_getheroscript(); break;
			case GETGENERICSCRIPT:	FFCore.do_getgenericscript(); break;
			case GETGLOBALSCRIPT:	FFCore.do_getglobalscript(); break;
			case GETDMAPSCRIPT:	FFCore.do_getdmapscript(); break;
			case GETSCREENSCRIPT:	FFCore.do_getscreenscript(); break;
			case GETSPRITESCRIPT:	FFCore.do_getitemspritescript(); break;
			case GETUNTYPEDSCRIPT:	FFCore.do_getuntypedscript(); break;
			case GETSUBSCREENSCRIPT:FFCore.do_getsubscreenscript(); break;
			case GETNPCBYNAME:	FFCore.do_getnpcbyname(); break;
			case GETITEMBYNAME:	FFCore.do_getitembyname(); break;
			case GETCOMBOBYNAME:	FFCore.do_getcombobyname(); break;
			case GETDMAPBYNAME:	FFCore.do_getdmapbyname(); break;
				
			case ABS:
				do_abs(false);
				break;
				
			case MINR:
				do_min(false);
				break;
				
			case MINV:
				do_min(true);
				break;
				
			case MAXR:
				do_max(false);
				break;
			case MAXV:
				do_max(true);
				break;
			case WRAPRADIANS:
				do_wrap_rad(false);
				break;
			case WRAPDEGREES:
				do_wrap_deg(false);
				break;
			
			case MAXVARG:
				FFCore.do_varg_max();
				break;
			case MINVARG:
				FFCore.do_varg_min();
				break;
			case CHOOSEVARG:
				FFCore.do_varg_choose();
				break;
			case MAKEVARGARRAY:
				assert(sarg1 >= 0 && sarg1 <= (int)script_object_type::last);
				FFCore.do_varg_makearray(type, i, (script_object_type)sarg1);
				break;
			
			case PUSHVARGV:
				do_push_varg(true);
				break;
			case PUSHVARGR:
				do_push_varg(false);
				break;
			case PUSHVARGSV:
				do_push_vargs(true);
				break;
			case PUSHVARGSR:
				do_push_vargs(false);
				break;
				
			case RNDR:
				do_rnd(false);
				break;
				
			case RNDV:
				do_rnd(true);
				break;
				
			case SRNDR:
				do_srnd(false);
				break;
				
			case SRNDV:
				do_srnd(true);
				break;
				
			case SRNDRND:
				do_srndrnd();
				break;
			
			case GETRTCTIMER:
				FFCore.getRTC(false);
				break;
			case GETRTCTIMEV:
				FFCore.getRTC(true);
				break;
				
			case FACTORIAL:
				do_factorial(false);
				break;
				
			case SQROOTV:
				do_sqroot(true);
				break;
				
			case SQROOTR:
				do_sqroot(false);
				break;
				
			case POWERR:
				do_power(false);
				break;
			case POWERV:
				do_power(true);
				break;
			case POWERV2:
				do_power(true,true);
				break;
				
			case LPOWERR:
				do_lpower(false);
				break;
			case LPOWERV:
				do_lpower(true);
				break;
			case LPOWERV2:
				do_lpower(true,true);
				break;
				
			case IPOWERR:
				do_ipower(false);
				break;
				
			case IPOWERV:
				do_ipower(true);
				break;
				
			case LOG10:
				do_log10(false);
				break;
				
			case LOGE:
				do_naturallog(false);
				break;
				
			case ANDR:
				do_and(false);
				break;
				
			case ANDV:
				do_and(true);
				break;
				
			case ORR:
				do_or(false);
				break;
				
			case ORV:
				do_or(true);
				break;
				
			case XORR:
				do_xor(false);
				break;
				
			case XORV:
				do_xor(true);
				break;
				
			case NANDR:
				do_nand(false);
				break;
				
			case NANDV:
				do_nand(true);
				break;
				
			case NORR:
				do_nor(false);
				break;
				
			case NORV:
				do_nor(true);
				break;
				
			case XNORR:
				do_xnor(false);
				break;
				
			case XNORV:
				do_xnor(true);
				break;
				
			case BITNOT:
				do_bitwisenot(false);
				break;
				
			case LSHIFTR:
				do_lshift(false);
				break;
				
			case LSHIFTV:
				do_lshift(true);
				break;
				
			case RSHIFTR:
				do_rshift(false);
				break;
				
			case RSHIFTV:
				do_rshift(true);
				break;
				
			case ANDR32:
				do_and32(false);
				break;
				
			case ANDV32:
				do_and32(true);
				break;
				
			case ORR32:
				do_or32(false);
				break;
				
			case ORV32:
				do_or32(true);
				break;
				
			case XORR32:
				do_xor32(false);
				break;
				
			case XORV32:
				do_xor32(true);
				break;
				
			case BITNOT32:
				do_bitwisenot32(false);
				break;
				
			case LSHIFTR32:
				do_lshift32(false);
				break;
				
			case LSHIFTV32:
				do_lshift32(true);
				break;
				
			case RSHIFTR32:
				do_rshift32(false);
				break;
				
			case RSHIFTV32:
				do_rshift32(true);
				break;
				
			case TRACER:
				FFCore.do_trace(false);
				break;
				
			case TRACELR:
				FFCore.do_tracel(false);
				break;
				
			case TRACEV:
				FFCore.do_trace(true);
				break;
				
			case TRACE2R:
				FFCore.do_tracebool(false);
				break;
			
			//Zap and Wavy Effects
			case FXWAVYR:
				FFCore.do_fx_wavy(false);
				break;
			case FXZAPR:
				FFCore.do_fx_zap(false);
				break;
			//Zap and Wavy Effects
			case FXWAVYV:
				FFCore.do_fx_wavy(true);
				break;
			case FXZAPV:
				FFCore.do_fx_zap(true);
				break;
			case GREYSCALER:
				FFCore.do_greyscale(false);
				break;
			case GREYSCALEV:
				FFCore.do_greyscale(true);
				break;
			case MONOCHROMER:
				FFCore.do_monochromatic(false);
				break;
			case MONOCHROMEV:
				FFCore.do_monochromatic(true);
				break;
				
			case TRACE2V:
				FFCore.do_tracebool(true);
				break;
				
			case TRACE3:
				FFCore.do_tracenl();
				break;
				
			case TRACE4:
				FFCore.do_cleartrace();
				break;
				
			case TRACE5:
				FFCore.do_tracetobase();
				break;
				
			case TRACE6:
				FFCore.do_tracestring();
				break;
			
			case PRINTFV:
				FFCore.do_printf(true, false);
				break;
			case SPRINTFV:
				FFCore.do_sprintf(true, false);
				break;
				
			case PRINTFVARG:
				FFCore.do_printf(true, true);
				break;
			case SPRINTFVARG:
				FFCore.do_sprintf(true, true);
				break;
			case PRINTFA:
				FFCore.do_printfarr();
				break;
			case SPRINTFA:
				FFCore.do_sprintfarr();
				break;
			case ARRAYPUSH:
			{
				auto ptr = SH::read_stack(ri->sp + 2);
				auto val = SH::read_stack(ri->sp + 1);
				auto indx = SH::read_stack(ri->sp + 0) / 10000;
				ArrayManager am(ptr);
				ri->d[rEXP1] = am.push(val,indx) ? 10000 : 0;
				break;
			}
			case ARRAYPOP:
			{
				auto ptr = SH::read_stack(ri->sp + 1);
				auto indx = SH::read_stack(ri->sp + 0) / 10000;
				ArrayManager am(ptr);
				ri->d[rEXP1] = am.pop(indx);
				break;
			}
			
			case BREAKPOINT:
				FFCore.do_breakpoint();
				break;
				
			case WARP:
				do_warp(true);
				break;
				
			case WARPR:
				do_warp(false);
				break;
				
			case PITWARP:
				do_pitwarp(true);
				break;
				
			case PITWARPR:
				do_pitwarp(false);
				break;
				
			case SELECTAWPNV:
				do_selectweapon(true, 1);
				break;
				
			case SELECTAWPNR:
				do_selectweapon(false, 1);
				break;
				
			case SELECTBWPNV:
				do_selectweapon(true, 0);
				break;
				
			case SELECTBWPNR:
				do_selectweapon(false, 0);
				break;
				
			case SELECTXWPNR:
				do_selectweapon(false, 2);
				break;
				
			case SELECTYWPNR:
				do_selectweapon(false, 3);
				break;
				
			case PLAYSOUNDR:
				do_sfx(false);
				break;
				
			case PLAYSOUNDV:
				do_sfx(true);
				break;
			
			case ADJUSTSFXVOLUMER: FFCore.do_adjustsfxvolume(false); break;
			case ADJUSTSFXVOLUMEV: FFCore.do_adjustsfxvolume(true); break;	
			case ADJUSTVOLUMER: FFCore.do_adjustvolume(false); break;
			case ADJUSTVOLUMEV: FFCore.do_adjustvolume(true); break;
				
			case PLAYMIDIR:
				do_midi(false);
				break;
				
			case PLAYMIDIV:
				do_midi(true);
				break;
				
			case PLAYENHMUSIC:
				do_enh_music(false);
				break;
				
			case GETMUSICFILE:
				do_get_enh_music_filename(false);
				break;
				
			case GETMUSICTRACK:
				do_get_enh_music_track(false);
				break;
				
			case SETDMAPENHMUSIC:
				do_set_dmap_enh_music(false);
				break;
			
			// Audio->
			
			case ENDSOUNDR:
				stop_sfx(false);
				break;
				
			case ENDSOUNDV:
				stop_sfx(true);
				break;
			
			case PAUSESOUNDR:
				pause_sfx(false);
				break;
				
			case PAUSESOUNDV:
				pause_sfx(true);
				break;
			
			case RESUMESOUNDR:
				resume_sfx(false);
				break;
				
			case RESUMESOUNDV:
				resume_sfx(true);
				break;
			
			
			
			case PAUSESFX:
			{
				int32_t sound = ri->d[rINDEX]/10000;
				pause_sfx(sound);
				
			}
			break;

			case RESUMESFX:
			{
				int32_t sound = ri->d[rINDEX]/10000;
				resume_sfx(sound);
			}
			break;

			case ADJUSTSFX:
			{
				do_sfx_ex(false);
			}
			break;

			case PLAYSOUNDEX:
			{
				do_sfx_ex(true);
			}
			break;

			case GETSFXCOMPLETION:
			{
				do_get_sfx_completion();
			}
			break;

			case CONTINUESFX:
			{
				int32_t sound = ri->d[rINDEX]/10000;
				//Backend::sfx->cont_sfx(sound);
				
				//! cont_sfx was not ported to the new back end!!!
				// I believe this restarted the loop. 
				resume_sfx(sound);
				//What was the old instruction, again? Did it exist? -Z
				//continue_sfx(sound);
			}
			break;	

			
			/*
			case STOPITEMSOUND:
				void stop_item_sfx(int32_t family)
			*/
			
			// Note: these have never worked.
			case PAUSEMUSIC:
				//What was the instruction prior to adding backends?
				//! The pauseAll() function pauses sfx, not music, so this instruction is not doing what I intended. -Z
				//Check AllOff() -Z
				//zcmusic_pause(ZCMUSIC* zcm, int32_t pause); is in zcmusic.h
				// midi_paused = true; 
				//pause_all_sfx();
			
				//Backend::sfx->pauseAll();
				break;
			case RESUMEMUSIC:
				//What was the instruction prior to adding backends?
				//Check AllOff() -Z
				//resume_all_sfx();
				// midi_paused = false; 
				//Backend::sfx->resumeAll();
				break;
				
			case MSGSTRR:
				do_message(false);
				break;
				
			case MSGSTRV:
				do_message(true);
				break;
				
			case ITEMNAME:
				do_getitemname();
				break;
			
			case LOADLWEAPONR:
				do_loadlweapon(false);
				break;
				
			case LOADLWEAPONV:
				do_loadlweapon(true);
				break;
				
			case LOADEWEAPONR:
				do_loadeweapon(false);
				break;
				
			case LOADEWEAPONV:
				do_loadeweapon(true);
				break;
				
			case LOADITEMR:
				do_loaditem(false);
				break;
				
			case LOADITEMV:
				do_loaditem(true);
				break;
				
			case LOADITEMDATAR:
				do_loaditemdata(false);
				break;
			
			//New Datatypes
			case LOADSHOPR:
				FFScript::do_loadshopdata(false);
				break;
			case LOADSHOPV:
				FFScript::do_loadshopdata(true);
				break;
			
			case LOADINFOSHOPR:
				FFScript::do_loadinfoshopdata(false);
				break;
			case LOADINFOSHOPV:
				FFScript::do_loadinfoshopdata(true);
				break;
			case LOADNPCDATAR:
				FFScript::do_loadnpcdata(false);
				break;
			case LOADNPCDATAV:
				FFScript::do_loadnpcdata(true);
				break;
			
			case LOADCOMBODATAR:
				FFScript::do_loadcombodata(false);
				break;
			case LOADCOMBODATAV:
				FFScript::do_loadcombodata(true);
				break;
			
			case LOADTMPSCR:
				FFScript::do_loadmapdata_tempscr(false);
				break;
			case LOADTMPSCR2:
				FFScript::do_loadmapdata_tempscr2(false);
				break;
			case REGION_LOAD_TMPSCR_FOR_LAYER_COMBO_POS:
				do_loadtmpscrforcombopos(false);
				break;
			case LOADSCROLLSCR:
				FFScript::do_loadmapdata_scrollscr(false);
				break;
			case LOADSCROLLSCR2:
				FFScript::do_loadmapdata_scrollscr2(false);
				break;
			
			case LOADSPRITEDATAR:
				FFScript::do_loadspritedata(false);
				break;
			case LOADSPRITEDATAV:
				FFScript::do_loadspritedata(true);
				break;
			
			case LOADBITMAPDATAR:
				FFScript::do_loadbitmapid(false);
				break;
			
			
			case LOADBITMAPDATAV:
				FFScript::do_loadbitmapid(true);
				break;
			
			//functions
			case LOADDIRECTORYR:
				FFCore.do_loaddirectory(); break;
			case CREATEPALDATA:
				FFCore.do_create_paldata(); break;
			case CREATEPALDATACLR:
				FFCore.do_create_paldata_clr(); break;
			case MIXCLR:
				FFCore.do_mix_clr(); break;
			case CREATERGBHEX:
				FFCore.do_create_rgb_hex(); break;
			case CREATERGB:
				FFCore.do_create_rgb(); break;
			case CONVERTFROMRGB:
				FFCore.do_convert_from_rgb(); break;
			case CONVERTTORGB:
				FFCore.do_convert_to_rgb(); break;
			case PALDATALOADLEVEL:
				FFCore.do_paldata_load_level(); break;
			case PALDATALOADSPRITE:
				FFCore.do_paldata_load_sprite(); break;
			case PALDATALOADMAIN:
				FFCore.do_paldata_load_main(); break;
			case PALDATALOADCYCLE:
				FFCore.do_paldata_load_cycle(); break;
			case PALDATALOADBITMAP:
				FFCore.do_paldata_load_bitmap(); break;
			case PALDATAWRITELEVEL:
				FFCore.do_paldata_write_level(); break;
			case PALDATAWRITELEVELCS:
				FFCore.do_paldata_write_levelcset(); break;
			case PALDATAWRITESPRITE:
				FFCore.do_paldata_write_sprite(); break;
			case PALDATAWRITESPRITECS:
				FFCore.do_paldata_write_spritecset(); break;
			case PALDATAWRITEMAIN:
				FFCore.do_paldata_write_main(); break;
			case PALDATAWRITEMAINCS:
				FFCore.do_paldata_write_maincset(); break;
			case PALDATAWRITECYCLE:
				FFCore.do_paldata_write_cycle(); break;
			case PALDATAWRITECYCLECS:
				FFCore.do_paldata_write_cyclecset(); break;
			case PALDATAVALIDCLR:
				FFCore.do_paldata_colorvalid(); break;
			case PALDATACLEARCLR:
				FFCore.do_paldata_clearcolor(); break;
			case PALDATACLEARCSET:
				FFCore.do_paldata_clearcset(); break;
			case PALDATAMIX:
				FFCore.do_paldata_mix(); break;
			case PALDATAMIXCS:
				FFCore.do_paldata_mixcset(); break;
			case PALDATACOPY:
				FFCore.do_paldata_copy(); break;
			case PALDATACOPYCSET:
				FFCore.do_paldata_copycset(); break;
			case PALDATAFREE:
				if (user_paldata* pd = checkPalData(ri->paldataref, true))
				{
					free_script_object(pd->id);
				}
				break;
			case PALDATAOWN:
				if (user_paldata* pd = checkPalData(ri->paldataref, false))
				{
					own_script_object(pd, type, i);
				}
				break;
			case LOADRNG: //command
				FFCore.do_loadrng(); break;

			case ITEMGETDISPLAYNAME: //command
				item_display_name(false); break;
			case ITEMSETDISPLAYNAME: //command
				item_display_name(true); break;
			case ITEMGETSHOWNNAME: //command
				item_shown_name(); break;

			case DMAPDATAGETNAMER: //command
				FFScript::do_getDMapData_dmapname(false); break;
			case DMAPDATAGETNAMEV: //command
				FFScript::do_getDMapData_dmapname(true); break;

			case DMAPDATASETNAMER: //command
				FFScript::do_setDMapData_dmapname(false); break;
			case DMAPDATASETNAMEV: //command
				FFScript::do_setDMapData_dmapname(true); break;



			case DMAPDATAGETTITLER: //command
				FFScript::do_getDMapData_dmaptitle(false); break;
			case DMAPDATAGETTITLEV: //command
				FFScript::do_getDMapData_dmaptitle(true); break;
			case DMAPDATASETTITLER: //command
				FFScript::do_setDMapData_dmaptitle(false); break;
			case DMAPDATASETTITLEV: //command
				FFScript::do_setDMapData_dmaptitle(true); break;


			case DMAPDATAGETINTROR: //command
				FFScript::do_getDMapData_dmapintro(false); break;
			case DMAPDATAGETINTROV: //command
				FFScript::do_getDMapData_dmapintro(true); break;
			case DMAPDATANSETITROR: //command
				FFScript::do_setDMapData_dmapintro(false); break;
			case DMAPDATASETINTROV: //command
				FFScript::do_setDMapData_dmapintro(true); break;


			case DMAPDATAGETMUSICR: //command, string to load a music file
				FFScript::do_getDMapData_music(false); break;
			case DMAPDATAGETMUSICV: //command, string to load a music file
				FFScript::do_getDMapData_music(true); break;
			case DMAPDATASETMUSICR: //command, string to load a music file
				FFScript::do_setDMapData_music(false); break;
			case DMAPDATASETMUSICV: //command, string to load a music file
				FFScript::do_setDMapData_music(true); break;

			case LOADMESSAGEDATAR: //COMMAND
				FFScript::do_loadmessagedata(false);
				break;
			case LOADMESSAGEDATAV: //COMMAND
				FFScript::do_loadmessagedata(false);
				break;
			

			case MESSAGEDATASETSTRINGR: //command
				FFScript::do_messagedata_setstring(false);
				break;
			case MESSAGEDATASETSTRINGV: //command
				FFScript::do_messagedata_setstring(false);
				break;
			
			case MESSAGEDATAGETSTRINGR: //command
				FFScript::do_messagedata_getstring(false);
				break;
			case MESSAGEDATAGETSTRINGV: //command
				FFScript::do_messagedata_getstring(false);
				break;	
			case LOADITEMDATAV:
				do_loaditemdata(true);
				break;
				
			case LOADNPCBYSUID:
				FFCore.do_loadnpc_by_script_uid(false);
				break;
			
			case LOADLWEAPONBYSUID:
				FFCore.do_loadlweapon_by_script_uid(false);
				break;
			
			case LOADWEAPONCBYSUID:
				FFCore.do_loadeweapon_by_script_uid(false);
				break;
			
			case LOADNPCR:
				do_loadnpc(false);
				break;
				
			case LOADNPCV:
				do_loadnpc(true);
				break;
				
			case CREATELWEAPONR:
				do_createlweapon(false);
				break;
				
			case CREATELWEAPONV:
				do_createlweapon(true);
				break;
				
			case CREATEEWEAPONR:
				do_createeweapon(false);
				break;
				
			case CREATEEWEAPONV:
				do_createeweapon(true);
				break;
				
			case CREATEITEMR:
				do_createitem(false);
				break;
				
			case CREATEITEMV:
				do_createitem(true);
				break;
				
			case CREATENPCR:
				do_createnpc(false);
				break;
				
			case CREATENPCV:
				do_createnpc(true);
				break;
				
			case ISVALIDARRAY:
				do_isvalidarray();
				break;
				
			case ISVALIDITEM:
				do_isvaliditem();
				break;
			
			case ISVALIDBITMAP:
				FFCore.do_isvalidbitmap();
				break;
			
			case ISALLOCATEDBITMAP:
				FFCore.do_isallocatedbitmap();
				break;
				
			case ISVALIDNPC:
				do_isvalidnpc();
				break;
				
			case ISVALIDLWPN:
				do_isvalidlwpn();
				break;
				
			case ISVALIDEWPN:
				do_isvalidewpn();
				break;
				
			case LWPNMAKEANGULAR:
				do_lwpnmakeangular();
				break;
				
			case EWPNMAKEANGULAR:
				do_ewpnmakeangular();
				break;
			
			case LWPNMAKEDIRECTIONAL:
				do_lwpnmakedirectional();
				break;
				
			case EWPNMAKEDIRECTIONAL:
				do_ewpnmakedirectional();
				break;
				
			case LWPNUSESPRITER:
				do_lwpnusesprite(false);
				break;
				
			case LWPNUSESPRITEV:
				do_lwpnusesprite(true);
				break;
				
			case EWPNUSESPRITER:
				do_ewpnusesprite(false);
				break;
				
			case EWPNUSESPRITEV:
				do_ewpnusesprite(true);
				break;
				
			case CLEARSPRITESR:
				do_clearsprites(false);
				break;
				
			case CLEARSPRITESV:
				do_clearsprites(true);
				break;
				
			case ISSOLID:
				do_issolid();
				break;
			
			case MAPDATAISSOLID:
				do_mapdataissolid();
				break;
				
			case MAPDATAISSOLIDLYR:
				do_mapdataissolid_layer();
				break;
				
			case ISSOLIDLAYER:
				do_issolid_layer();
				break;
				
			case SETSIDEWARP:
				do_setsidewarp();
				break;
				
			case SETTILEWARP:
				do_settilewarp();
				break;
				
			case GETSIDEWARPDMAP:
				do_getsidewarpdmap(false);
				break;
				
			case GETSIDEWARPSCR:
				do_getsidewarpscr(false);
				break;
				
			case GETSIDEWARPTYPE:
				do_getsidewarptype(false);
				break;
				
			case GETTILEWARPDMAP:
				do_gettilewarpdmap(false);
				break;
				
			case GETTILEWARPSCR:
				do_gettilewarpscr(false);
				break;
				
			case GETTILEWARPTYPE:
				do_gettilewarptype(false);
				break;
				
			case LAYERSCREEN:
				do_layerscreen();
				break;
				
			case LAYERMAP:
				do_layermap();
				break;
				
			case SECRETS:
				do_triggersecrets(ri->screenref);
				break;

			case REGION_TRIGGER_SECRETS:
			{
				int screen = get_register(sarg1) / 10000;
				if (!is_in_current_region(screen))
				{
					scripting_log_error_with_context("Must use a screen in the current region. got: {}", screen);
					break;
				}

				do_triggersecrets(screen);
				break;
			}

			case GRAPHICSGETPIXEL:
				FFCore.do_graphics_getpixel();
				break;
			case GRAPHICSCOUNTCOLOR:
				FFCore.do_bmpcollision();
				break;

			case COMBOTILE:
				do_combotile(false);
				break;
			
			case DRAWLIGHT_CIRCLE:
			{
				if (get_qr(qr_SCRIPTS_SCREEN_DRAW_LIGHT_NO_OFFSET))
				{
					static const int ARGS = 7;
					zfix cx = zslongToFix(SH::read_stack(ri->sp + (ARGS-1)));
					zfix cy = zslongToFix(SH::read_stack(ri->sp + (ARGS-2)));
					int radius = SH::read_stack(ri->sp + (ARGS-3));
					int transp_rad = SH::read_stack(ri->sp + (ARGS-4));
					int dith_rad = SH::read_stack(ri->sp + (ARGS-5));
					int dith_type = SH::read_stack(ri->sp + (ARGS-6));
					int dith_arg = SH::read_stack(ri->sp + (ARGS-7));
					if(radius >= 0) radius /= 10000;
					else radius = game->get_light_rad();
					if(!radius) break;
					if(transp_rad >= 0) transp_rad /= 10000;
					if(dith_rad >= 0) dith_rad /= 10000;
					if(dith_type >= 0) dith_type /= 10000;
					if(dith_arg >= 0) dith_arg /= 10000;
					
					doDarkroomCircle(cx,cy,radius,darkscr_bmp,nullptr,dith_rad,transp_rad,dith_type,dith_arg);
				}
				else do_drawing_command(scommand, true);

				break;
			}
			case DRAWLIGHT_SQUARE:
			{
				if (get_qr(qr_SCRIPTS_SCREEN_DRAW_LIGHT_NO_OFFSET))
				{
					static const int ARGS = 7;
					zfix cx = zslongToFix(SH::read_stack(ri->sp + (ARGS-1)));
					zfix cy = zslongToFix(SH::read_stack(ri->sp + (ARGS-2)));
					int radius = SH::read_stack(ri->sp + (ARGS-3));
					int transp_rad = SH::read_stack(ri->sp + (ARGS-4));
					int dith_rad = SH::read_stack(ri->sp + (ARGS-5));
					int dith_type = SH::read_stack(ri->sp + (ARGS-6));
					int dith_arg = SH::read_stack(ri->sp + (ARGS-7));
					if(radius >= 0) radius /= 10000;
					else radius = game->get_light_rad();
					if(!radius) break;
					if(transp_rad >= 0) transp_rad /= 10000;
					if(dith_rad >= 0) dith_rad /= 10000;
					if(dith_type >= 0) dith_type /= 10000;
					if(dith_arg >= 0) dith_arg /= 10000;

					doDarkroomSquare(cx,cy,radius,darkscr_bmp,nullptr,dith_rad,transp_rad,dith_type,dith_arg);
				}
				else do_drawing_command(scommand, true);

				break;
			}
			case DRAWLIGHT_CONE:
			{
				if (get_qr(qr_SCRIPTS_SCREEN_DRAW_LIGHT_NO_OFFSET))
				{
					static const int ARGS = 8;
					zfix cx = zslongToFix(SH::read_stack(ri->sp + (ARGS-1)));
					zfix cy = zslongToFix(SH::read_stack(ri->sp + (ARGS-2)));
					int dir = SH::read_stack(ri->sp + (ARGS-3)) / 10000;
					int length = SH::read_stack(ri->sp + (ARGS-4));
					int transp_rad = SH::read_stack(ri->sp + (ARGS-5));
					int dith_rad = SH::read_stack(ri->sp + (ARGS-6));
					int dith_type = SH::read_stack(ri->sp + (ARGS-7));
					int dith_arg = SH::read_stack(ri->sp + (ARGS-8));
					if(length >= 0) length /= 10000;
					else length = game->get_light_rad()*2;
					if(!length) break;
					if(dir < 0) break;
					else dir = NORMAL_DIR(dir);
					if(transp_rad >= 0) transp_rad /= 10000;
					if(dith_rad >= 0) dith_rad /= 10000;
					if(dith_type >= 0) dith_type /= 10000;
					if(dith_arg >= 0) dith_arg /= 10000;
	
					doDarkroomCone(cx,cy,length,dir,darkscr_bmp,nullptr,dith_rad,transp_rad,dith_type,dith_arg);
				}
				else do_drawing_command(scommand, true);

				break;
			}
			
			case RECTR:
			case CIRCLER:
			case ARCR:
			case ELLIPSER:
			case LINER:
			case PUTPIXELR:
			case PIXELARRAYR:
			case TILEARRAYR:
			case LINESARRAY:
			case COMBOARRAYR:
			case DRAWTILER:
			case DRAWTILECLOAKEDR:
			case DRAWCOMBOR:
			case DRAWCOMBOCLOAKEDR:
			case DRAWCHARR:
			case DRAWINTR:
			case QUADR:
			case TRIANGLER:
			case QUAD3DR:
			case TRIANGLE3DR:
			case FASTTILER:
			case FASTCOMBOR:
			case DRAWSTRINGR:
			case DRAWSTRINGR2:
			case BMPDRAWSTRINGR2:
			case SPLINER:
			case BITMAPR:
			case BITMAPEXR:
			case DRAWLAYERR:
			case DRAWSCREENR:
			case POLYGONR:
			case FRAMER:
			case TILEBLIT:
			case COMBOBLIT:
				do_drawing_command(scommand, true);
				break;
				
			case BMPRECTR:	
			case BMPCIRCLER:
			case BMPARCR:
			case BMPELLIPSER:
			case BMPLINER:
			case BMPSPLINER:
			case BMPPUTPIXELR:
			case BMPDRAWTILER:
			case BMPDRAWTILECLOAKEDR:
			case BMPDRAWCOMBOR:
			case BMPDRAWCOMBOCLOAKEDR:
			case BMPFASTTILER:
			case BMPFASTCOMBOR:
			case BMPDRAWCHARR:
			case BMPDRAWINTR:
			case BMPDRAWSTRINGR:
			case BMPQUADR:
			case BMPQUAD3DR:
			case BMPTRIANGLER:
			case BMPTRIANGLE3DR:
			case BMPPOLYGONR:
			case BMPDRAWLAYERR: 
			case BMPDRAWLAYERSOLIDR: 
			case BMPDRAWLAYERCFLAGR: 
			case BMPDRAWLAYERCTYPER: 
			case BMPDRAWLAYERCIFLAGR: 
			case BMPDRAWLAYERSOLIDITYR: 
			case BMPDRAWSCREENR:
			case BMPDRAWSCREENSOLIDR:
			case BMPDRAWSCREENSOLID2R:
			case BMPDRAWSCREENCOMBOFR:
			case BMPDRAWSCREENCOMBOIR:
			case BMPDRAWSCREENCOMBOTR:
			case BITMAPGETPIXEL:
			case BMPBLIT:
			case BMPBLITTO:
			case BMPTILEBLIT:
			case BMPCOMBOBLIT:
			case BMPMODE7:
			case WRITEBITMAP:
			case CLEARBITMAP:
			case BITMAPCLEARTOCOLOR:
			case BMPFRAMER:
			case BMPWRITETILE:
			case BMPDITHER:
			case BMPREPLCOLOR:
			case BMPSHIFTCOLOR:
			case BMPMASKDRAW:
			case BMPMASKDRAW2:
			case BMPMASKDRAW3:
			case BMPMASKBLIT:
			case BMPMASKBLIT2:
			case BMPMASKBLIT3:
				do_drawing_command(scommand, false);
				break;
			case READBITMAP:
			{
				uint32_t bitref = SH::read_stack(ri->sp+2);
				if(user_bitmap* b = checkBitmap(bitref,false,true))
					do_drawing_command(scommand, false);
				else //If the pointer isn't allocated, attempt to allocate it first
				{
					bitref = FFCore.get_free_bitmap();
					ri->d[rEXP2] = bitref; //Return to ptr
					if(bitref) SH::write_stack(ri->sp+2,bitref); //Write the ref, for the drawing command to read
					else break; //No ref allocated; don't enqueue the drawing command.
					do_drawing_command(scommand, false);
				}
				break;
			}
			case REGENERATEBITMAP:
			{
				ri->d[rEXP2] = SH::read_stack(ri->sp+3);
				if(user_bitmap* b = checkBitmap(ri->d[rEXP2],false,true))
					do_drawing_command(scommand, false);
				else //If the pointer isn't allocated
				{
					int32_t w = SH::read_stack(ri->sp) / 10000;
					int32_t h = SH::read_stack(ri->sp+1) / 10000;
					if ( get_qr(qr_OLDCREATEBITMAP_ARGS) )
					{
						//flip height and width
						h = h ^ w;
						w = h ^ w; 
						h = h ^ w;
					}
					
					ri->d[rEXP2] = FFCore.create_user_bitmap_ex(h,w); //Return to ptr
				}
				break;
			}
			
			case BITMAPFREE:
			{
				FFCore.do_deallocate_bitmap();
				break;
			}
			
			case BITMAPOWN:
			{
				if(FFCore.isSystemBitref(ri->bitmapref))
					break; //Don't attempt to own system bitmaps!

				if (auto bitmap = checkBitmap(ri->bitmapref, false))
					own_script_object(bitmap, type, i);
				break;
			}
			
			case OBJ_OWN_BITMAP:
			{
				int bmpid = get_register(sarg1);
				if(FFCore.isSystemBitref(bmpid))
					break; //Don't attempt to own system bitmaps!
				user_bitmap* b = checkBitmap(bmpid, false);
				if(!b) break;
				ScriptType own_type = (ScriptType)sarg2;
				int32_t own_i = get_own_i(own_type);
				own_script_object(b, own_type, own_i);
				break;
			}
			case OBJ_OWN_PALDATA:
			{
				int palid = get_register(sarg1);
				user_paldata* pd = checkPalData(palid, false);
				if(!pd) break;
				ScriptType own_type = (ScriptType)sarg2;
				int32_t own_i = get_own_i(own_type);
				own_script_object(pd, own_type, own_i);
				break;
			}
			case OBJ_OWN_FILE:
			{
				int fileid = get_register(sarg1);
				user_file* f = checkFile(fileid, false);
				if(!f) break;
				ScriptType own_type = (ScriptType)sarg2;
				int32_t own_i = get_own_i(own_type);
				own_script_object(f, own_type, own_i);
				break;
			}
			case OBJ_OWN_DIR:
			{
				int dirid = get_register(sarg1);
				user_dir* dr = checkDir(dirid, false);
				if(!dr) break;
				ScriptType own_type = (ScriptType)sarg2;
				int32_t own_i = get_own_i(own_type);
				own_script_object(dr, own_type, own_i);
				break;
			}
			case OBJ_OWN_STACK:
			{
				int stackid = get_register(sarg1);
				user_stack* st = checkStack(stackid, false);
				if(!st) break;
				ScriptType own_type = (ScriptType)sarg2;
				int32_t own_i = get_own_i(own_type);
				own_script_object(st, own_type, own_i);
				break;
			}
			case OBJ_OWN_RNG:
			{
				int rngid = get_register(sarg1);
				user_rng* r = checkRNG(rngid, false);
				if(!r) break;
				ScriptType own_type = (ScriptType)sarg2;
				int32_t own_i = get_own_i(own_type);
				own_script_object(r, own_type, own_i);
				break;
			}
			case OBJ_OWN_ARRAY:
			{
				int arrid = get_register(sarg1);
				ScriptType own_type = (ScriptType)sarg2;
				int32_t own_i = get_own_i(own_type);
				do_own_array(arrid, own_type, own_i);
				break;
			}
				
			case COPYTILEVV:
				do_copytile(true, true);
				break;
				
			case COPYTILEVR:
				do_copytile(true, false);
				break;
				
			case COPYTILERV:
				do_copytile(false, true);
				break;
				
			case COPYTILERR:
				do_copytile(false, false);
				break;
				
			case SWAPTILEVV:
				do_swaptile(true, true);
				break;
				
			case SWAPTILEVR:
				do_swaptile(true, false);
				break;
				
			case SWAPTILERV:
				do_swaptile(false, true);
				break;
				
			case SWAPTILERR:
				do_swaptile(false, false);
				break;
				
			case CLEARTILEV:
				do_cleartile(true);
				break;
				
			case CLEARTILER:
				do_cleartile(false);
				break;
				
			case OVERLAYTILEVV:
				do_overlaytile(true, true);
				break;
				
			case OVERLAYTILEVR:
				do_overlaytile(true, false);
				break;
				
			case OVERLAYTILERV:
				do_overlaytile(false, true);
				break;
				
			case OVERLAYTILERR:
				do_overlaytile(false, false);
				break;
				
			case FLIPROTTILEVV:
				do_fliprotatetile(true, true);
				break;
				
			case FLIPROTTILEVR:
				do_fliprotatetile(true, false);
				break;
				
			case FLIPROTTILERV:
				do_fliprotatetile(false, true);
				break;
				
			case FLIPROTTILERR:
				do_fliprotatetile(false, false);
				break;
				
			case GETTILEPIXEL:
				do_gettilepixel();
				break;
				
			case SETTILEPIXEL:
				do_settilepixel();
				break;
				
			case SHIFTTILEVV:
				do_shifttile(true, true);
				break;
				
			case SHIFTTILEVR:
				do_shifttile(true, false);
				break;
				
			case SHIFTTILERV:
				do_shifttile(false, true);
				break;
				
			case SHIFTTILERR:
				do_shifttile(false, false);
				break;
				
			case SETRENDERTARGET:
				do_set_rendertarget(true);
				break;
				
			case GAMEEND:
				if ( using_SRAM )
				{
					Z_scripterrlog("Cannot End Game while reading or writing to SRAM. Aborting End. /n");
					break;
				}
				Quit = qQUIT;
				skipcont = 1;
				scommand = 0xFFFF;
				break;
			case GAMEEXIT:
				Quit = qEXIT;
				skipcont = 1;
				scommand = 0xFFFF;
				break;
			case GAMERELOAD:
				if ( using_SRAM )
				{
					Z_scripterrlog("Cannot Reload Game while reading or writing to SRAM. Aborting Reload. /n");
					break;
				}
				Quit = qRELOAD;
				skipcont = 1;
				scommand = 0xFFFF;
				break;
			case GAMESETCUSTOMCURSOR:
			{
				int32_t bmpptr = SH::read_stack(ri->sp + 4);
				int fx = SH::read_stack(ri->sp + 3) / 10000;
				int fy = SH::read_stack(ri->sp + 2) / 10000;
				bool recolor = SH::read_stack(ri->sp + 1)!=0;
				bool scale = SH::read_stack(ri->sp + 0)!=0;
				if(user_bitmap* b = checkBitmap(bmpptr,true))
				{
					custom_mouse(b->u_bmp,fx,fy,recolor,scale);
				}
				break;
			}
			case CURRENTITEMID:
			{
				int ity = SH::read_stack(ri->sp + 1) / 10000;
				int flags = SH::read_stack(ri->sp + 0) / 10000;
				bool checkcost = flags&0x01;
				bool checkjinx = flags&0x02;
				bool check_bunny = flags&0x04;
				ri->d[rEXP1] = current_item_id(ity,checkcost,checkjinx,check_bunny) * 10000;
				break;
			}
			
			case GAMECONTINUE:
				if ( using_SRAM )
				{
					Z_scripterrlog("Cannot Continue Game while reading or writing to SRAM. Aborting Continue. /n");
					break;
				}
				reset_all_combo_animations();
			
				Quit = qCONT;
				skipcont = 1;
				//cont_game();
				scommand = 0xFFFF;
				break;
				
			case GAMESAVEQUIT:
				if ( using_SRAM )
				{
					Z_scripterrlog("Cannot Save Game while reading or writing to SRAM. Aborting Save. /n");
					break;
				}
				Quit = qSAVE;
				skipcont = 1;
				scommand =0xFFFF;
				break;
				
			case GAMESAVECONTINUE:
				Quit = qSAVECONT;
				skipcont = 1;
				scommand =0xFFFF;
				break;
				
			case SAVE:
				if ( using_SRAM )
				{
					Z_scripterrlog("Cannot Save Game while reading or writing to SRAM. Aborting Save. /n");
					break;
				}
				if(scriptCanSave)
				{
					save_game(false);
					scriptCanSave=false;
				}
				break;
				
			case SAVESCREEN:
				do_showsavescreen();
				break;
			
			case SHOWF6SCREEN:
				onTryQuit();
				break;
				
			case SAVEQUITSCREEN:
				save_game(false, 1);
				break;
				
				//Not Implemented
			case ELLIPSE2:
			case FLOODFILL:
				break;
				
			case ENQUEUER:
				do_enqueue(false);
				break;
				
			case ENQUEUEV:
				do_enqueue(true);
				break;
				
			case DEQUEUE:
				do_dequeue(false);
				break;
			
			//Visual Effects
			case WAVYIN:
				FFScript::do_wavyin();
				break;
			case WAVYOUT:
				FFScript::do_wavyout();
				break;
			case ZAPIN:
				FFScript::do_zapin();
				break;
			case ZAPOUT:
				FFScript::do_zapout();
				break;
			case OPENWIPE:
			{
				FFScript::do_openscreen();
				break;
			}
			case CLOSEWIPE:
			{
				FFScript::do_closescreen();
				break;
			}
			case OPENWIPESHAPE:
			{
				FFScript::do_openscreenshape();
				break;
			}
			case CLOSEWIPESHAPE:
			{
				FFScript::do_closescreenshape();
				break;
			}
			
			case TINT:
			{
				FFCore.Tint();
				break;
			}
			
			case CLEARTINT:
			{
				FFCore.clearTint();
				break;
			}
			
			case MONOHUE:
			{
				FFCore.gfxmonohue();
				break;
			}
			
			case SCREENDOSPAWN:
			{
				ri->d[rEXP1] = scriptloadenemies(ri->screenref) ? 10000 : 0;
				break;
			}
			
			case SCRTRIGGERCOMBO:
			{
				int32_t lyr = get_register(sarg1) / 10000;
				int32_t pos = get_register(sarg2) / 10000;
				rpos_t rpos = (rpos_t)pos;
				if (BC::checkComboRpos(rpos) != SH::_NoError)
				{
					break;
				}

				set_register(sarg1, do_trigger_combo(get_rpos_handle(rpos, lyr), 0) ? 10000 : 0);
				break;
			}
			case SCRTRIGGERCOMBO2:
			{
				int32_t lyr = get_register(sarg1) / 10000;
				int32_t pos = get_register(sarg2) / 10000;
				int32_t idx = get_register(sarg3) / 10000;
				rpos_t rpos = (rpos_t)pos;
				if (BC::checkComboRpos(rpos) != SH::_NoError)
					break;

				set_register(sarg1, do_trigger_combo(get_rpos_handle(rpos, lyr), idx) ? 10000 : 0);
				break;
			}
			
			case SWITCHNPC:
			{
				byte effect = vbound(get_register(sarg1)/10000, 0, 255);
				set_register(sarg1,0);
				if(Hero.switchhookclk) break; //Already switching!
				if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
				{
					switching_object = guys.getByUID(ri->guyref);
					hooked_comborpos = rpos_t::None;
					hooked_layerbits = 0;
					switching_object->switch_hooked = true;
					Hero.doSwitchHook(effect);
					set_register(sarg1,10000);
				}
				break;
			}
			
			case SWITCHITM:
			{
				byte effect = vbound(get_register(sarg1)/10000, 0, 255);
				set_register(sarg1,0);
				if(Hero.switchhookclk) break; //Already switching!
				if(ItemH::loadItem(ri->itemref) == SH::_NoError)
				{
					switching_object = ItemH::getItem();
					hooked_comborpos = rpos_t::None;
					hooked_layerbits = 0;
					switching_object->switch_hooked = true;
					Hero.doSwitchHook(effect);
					set_register(sarg1,10000);
				}
				break;
			}
			
			case SWITCHLW:
			{
				byte effect = vbound(get_register(sarg1)/10000, 0, 255);
				set_register(sarg1,0);
				if(Hero.switchhookclk) break; //Already switching!
				if(LwpnH::loadWeapon(ri->lwpn) == SH::_NoError)
				{
					switching_object = LwpnH::getWeapon();
					hooked_comborpos = rpos_t::None;
					hooked_layerbits = 0;
					switching_object->switch_hooked = true;
					Hero.doSwitchHook(effect);
					set_register(sarg1,10000);
				}
				break;
			}
			
			case SWITCHEW:
			{
				byte effect = vbound(get_register(sarg1)/10000, 0, 255);
				set_register(sarg1,0);
				if(Hero.switchhookclk) break; //Already switching!
				if(EwpnH::loadWeapon(ri->ewpn) == SH::_NoError)
				{
					switching_object = EwpnH::getWeapon();
					hooked_comborpos = rpos_t::None;
					hooked_layerbits = 0;
					switching_object->switch_hooked = true;
					Hero.doSwitchHook(effect);
					set_register(sarg1,10000);
				}
				break;
			}
			
			case SWITCHCMB:
			{
				rpos_t rpos = (rpos_t)(get_register(sarg1)/10000);
				set_register(sarg1,0);
				if(Hero.switchhookclk) break; //Already switching!
				if (!is_valid_rpos(rpos))
					break;
				switching_object = NULL;
				hooked_comborpos = rpos;
				hooked_layerbits = 0;
				Hero.doSwitchHook(get_register(sarg2)/10000);
				if(!hooked_layerbits) //failed
					Hero.reset_hookshot();
				else set_register(sarg1,10000); //success return
				break;
			}
			
			case LINKWARPEXR:
			{
				FFCore.do_warp_ex(false);
				break;
			}
			
			case KILLPLAYER:
			{
				Hero.kill(get_register(sarg1));
				break;
			}
			
			case HEROMOVEXY:
			{
				zfix dx = zslongToFix(SH::read_stack(ri->sp + 4));
				zfix dy = zslongToFix(SH::read_stack(ri->sp + 3));
				bool kb = SH::read_stack(ri->sp + 2)!=0;
				bool ign_sv = SH::read_stack(ri->sp + 1)!=0;
				bool shove = SH::read_stack(ri->sp + 0)!=0;
				ri->d[rEXP1] = Hero.movexy(dx, dy, kb, ign_sv, shove) ? 10000 : 0;
				break;
			}
			case HEROCANMOVEXY:
			{
				zfix dx = zslongToFix(SH::read_stack(ri->sp + 4));
				zfix dy = zslongToFix(SH::read_stack(ri->sp + 3));
				bool kb = SH::read_stack(ri->sp + 2)!=0;
				bool ign_sv = SH::read_stack(ri->sp + 1)!=0;
				bool shove = SH::read_stack(ri->sp + 0)!=0;
				ri->d[rEXP1] = Hero.can_movexy(dx, dy, kb, ign_sv, shove) ? 10000 : 0;
				break;
			}
			case HEROMOVEATANGLE:
			{
				zfix degrees = zslongToFix(SH::read_stack(ri->sp + 4));
				zfix pxamnt = zslongToFix(SH::read_stack(ri->sp + 3));
				bool kb = SH::read_stack(ri->sp + 2)!=0;
				bool ign_sv = SH::read_stack(ri->sp + 1)!=0;
				bool shove = SH::read_stack(ri->sp + 0)!=0;
				ri->d[rEXP1] = Hero.moveAtAngle(degrees, pxamnt, kb, ign_sv, shove) ? 10000 : 0;
				break;
			}
			case HEROCANMOVEATANGLE:
			{
				zfix degrees = zslongToFix(SH::read_stack(ri->sp + 4));
				zfix pxamnt = zslongToFix(SH::read_stack(ri->sp + 3));
				bool kb = SH::read_stack(ri->sp + 2)!=0;
				bool ign_sv = SH::read_stack(ri->sp + 1)!=0;
				bool shove = SH::read_stack(ri->sp + 0)!=0;
				ri->d[rEXP1] = Hero.can_moveAtAngle(degrees, pxamnt, kb, ign_sv, shove) ? 10000 : 0;
				break;
			}
			case HEROMOVE:
			{
				int dir = SH::read_stack(ri->sp + 4)/10000;
				zfix pxamnt = zslongToFix(SH::read_stack(ri->sp + 3));
				bool kb = SH::read_stack(ri->sp + 2)!=0;
				bool ign_sv = SH::read_stack(ri->sp + 1)!=0;
				bool shove = SH::read_stack(ri->sp + 0)!=0;
				ri->d[rEXP1] = Hero.moveDir(dir, pxamnt, kb, ign_sv, shove) ? 10000 : 0;
				break;
			}
			case HEROCANMOVE:
			{
				int dir = SH::read_stack(ri->sp + 4)/10000;
				zfix pxamnt = zslongToFix(SH::read_stack(ri->sp + 3));
				bool kb = SH::read_stack(ri->sp + 2)!=0;
				bool ign_sv = SH::read_stack(ri->sp + 1)!=0;
				bool shove = SH::read_stack(ri->sp + 0)!=0;
				ri->d[rEXP1] = Hero.can_moveDir(dir, pxamnt, kb, ign_sv, shove) ? 10000 : 0;
				break;
			}
			case HEROLIFTRELEASE:
			{
				if(Hero.lift_wpn)
				{
					ri->d[rEXP1] = Hero.lift_wpn->getUID();
					Lwpns.add(Hero.lift_wpn);
					Hero.lift_wpn = nullptr;
				}
				else ri->d[rEXP1] = 0;
				break;
			}
			case HEROLIFTGRAB:
			{
				auto lwuid = SH::read_stack(ri->sp + 2);
				auto lifttime = SH::read_stack(ri->sp + 1)/10000;
				auto liftheight = zslongToFix(SH::read_stack(ri->sp + 0));
				if(weapon* wpn = checkLWpn(lwuid))
				{
					Hero.lift(wpn, lifttime, liftheight);
					if(Lwpns.find(wpn) > -1)
						Lwpns.remove(wpn);
					if(type == ScriptType::Lwpn && lwuid == i)
						earlyretval = RUNSCRIPT_SELFREMOVE;
				}
				break;
			}
			case HEROISFLICKERFRAME:
				ri->d[rEXP1] = Hero.is_hitflickerframe() ? 10000 : 0;
				break;
			case LOADPORTAL:
			{
				auto val = get_register(sarg1)/10000;
				if(val != -1)
				{
					portal* prt = (portal*)portals.spr(val);
					if(prt)
						val = prt->getUID();
					else
					{
						Z_scripterrlog("Tried to load invalid portal index '%d'\n", val);
						val = 0;
					}
				}
				ri->portalref = ri->d[rEXP1] = val;
				break;
			}
			case CREATEPORTAL:
			{
				portal* p = new portal();
				if(portals.add(p))
					ri->portalref = ri->d[rEXP1] = p->getUID();
				else
				{
					ri->portalref = ri->d[rEXP1] = 0;
					Z_scripterrlog("Unable to create new portal! Limit reached!\n");
				}
				break;
			}
			case LOADSAVPORTAL:
			{
				auto val = get_register(sarg1)/10000;
				savedportal* prt = checkSavedPortal(val);
				ri->saveportalref = ri->d[rEXP1] = prt ? val : 0;
				break;
			}
			case CREATESAVPORTAL:
			{
				if(game->user_portals.size() >= MAX_SAVED_PORTALS)
				{
					ri->saveportalref = ri->d[rEXP1] = 0;
					Z_scripterrlog("Cannot create any more Saved Portals! Remove some first!\n");
					break;
				}
				savedportal& ref = game->user_portals.emplace_back();
				ri->saveportalref = ri->d[rEXP1] = ref.getUID();
				break;
			}
			case PORTALREMOVE:
			{
				if(portal* p = checkPortal(ri->portalref, true))
				{
					if(p == &mirror_portal)
						p->clear();
					else
					{
						auto id = portals.find(p);
						if(id > -1)
							portals.del(id,true);
					}
				}
				break;
			}
			case PORTALUSESPRITE:
				do_portalusesprite();
				break;
			case SAVEDPORTALREMOVE:
			{
				if(savedportal* sp = checkSavedPortal(ri->saveportalref, true))
				{
					if(sp == &(game->saved_mirror_portal))
						sp->clear();
					else
					{
						//ensure all pointers to the object are cleared before deleting
						portals.forEach([&](sprite& spr)
						{
							portal* tmp = (portal*)&spr;
							if(sp->getUID() == tmp->saved_data)
							{
								tmp->saved_data = 0;
							}
							return false;
						});
						//delete the savedportal object from the vector
						for(auto it = game->user_portals.begin();
							it != game->user_portals.end();)
						{
							savedportal& tmp = *it;
							if(sp == &tmp)
							{
								game->user_portals.erase(it);
								break;
							}
							else ++it;
						}
					}
				}
				break;
			}
			case SAVEDPORTALGENERATE:
			{
				auto retval = 0;
				if(savedportal* sp = checkSavedPortal(ri->saveportalref))
				{
					retval = getPortalFromSaved(sp);
					if(!retval)
					{
						if(portal* p = loadportal(*sp))
							if(portals.add(p))
								retval = p->getUID();
					}
				}
				ri->d[rEXP1] = retval;
				break;
			}
			
			case LINKEXPLODER:
			{
				int32_t mode = get_register(sarg1) / 10000;
				if ( (unsigned) mode > 2 ) 
				{
					Z_scripterrlog("Invalid mode (%d) passed to Hero->Explode(int32_t mode)\n",mode);
				}
				else Hero.explode(mode);
				break;
			}
			case NPCEXPLODER:
			{
				int32_t mode = get_register(sarg1) / 10000;
				if ( (unsigned) mode > 2 ) 
				{
					Z_scripterrlog("Invalid mode (%d) passed to npc->Explode(int32_t mode)\n",mode);
				}
				else
				{
					if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
					{
						GuyH::getNPC()->explode(mode);
					}
				}
				break;
			}
			
			case ITEMEXPLODER:
			{
				
				int32_t mode = get_register(sarg1) / 10000;
				if ( (unsigned) mode > 2 ) 
				{
					Z_scripterrlog("Invalid mode (%d) passed to item->Explode(int32_t mode)\n",mode);
				}
				else
				{
					if(ItemH::loadItem(ri->itemref) == SH::_NoError)
					{
						ItemH::getItem()->explode(mode);
					}
				}
				break;
			}
			case LWEAPONEXPLODER:
			{
				int32_t mode = get_register(sarg1) / 10000;
				if ( (unsigned) mode > 2 ) 
				{
					Z_scripterrlog("Invalid mode (%d) passed to lweapon->Explode(int32_t mode)\n",mode);
				}
				else
				{
					if(LwpnH::loadWeapon(ri->lwpn) == SH::_NoError)
					{
						LwpnH::getWeapon()->explode(mode);
					}
				}
				break;
			}
			case EWEAPONEXPLODER:
			{
				int32_t mode = get_register(sarg1) / 10000;
				if ( (unsigned) mode > 2 ) 
				{
					Z_scripterrlog("Invalid mode (%d) passed to eweapon->Explode(int32_t mode)\n",mode);
				}
				else
				{
					if(EwpnH::loadWeapon(ri->ewpn) == SH::_NoError)
					{
						EwpnH::getWeapon()->explode(mode);
					}
				}
				break;
			}
				
			case BOTTLENAMEGET:
			{
				int32_t arrayptr = get_register(sarg1);
				int32_t id = ri->bottletyperef-1;
				if(unsigned(id) > 63)
				{
					Z_scripterrlog("Invalid bottledata ID (%d) passed to bottledata->GetName().\n", id);
					break;
				}
				
				if(ArrayH::setArray(arrayptr, QMisc.bottle_types[id].name) == SH::_Overflow)
					Z_scripterrlog("Array supplied to 'bottledata->GetName()' not large enough\n");
				break;
			}
			case BOTTLENAMESET:
			{
				int32_t arrayptr = get_register(sarg1);
				int32_t id = ri->bottletyperef-1;
				if(unsigned(id) > 63)
				{
					Z_scripterrlog("Invalid bottledata ID (%d) passed to bottledata->SetName().\n", id+1);
					break;
				}
				string name;
				ArrayH::getString(arrayptr, name, 31);
				strcpy(QMisc.bottle_types[id].name, name.c_str());
				break;
			}
			case BSHOPNAMEGET:
			{
				int32_t arrayptr = get_register(sarg1);
				int32_t id = ri->bottleshopref-1;
				if(unsigned(id) > 255)
				{
					Z_scripterrlog("Invalid bottleshopdata ID (%d) passed to bottleshopdata->GetName().\n", id+1);
					break;
				}
				
				if(ArrayH::setArray(arrayptr, QMisc.bottle_shop_types[id].name) == SH::_Overflow)
					Z_scripterrlog("Array supplied to 'bottleshopdata->GetName()' not large enough\n");
				break;
			}
			case BSHOPNAMESET:
			{
				int32_t arrayptr = get_register(sarg1);
				int32_t id = ri->bottleshopref;
				if(unsigned(id) > 255)
				{
					Z_scripterrlog("Invalid bottleshopdata ID (%d) passed to bottleshopdata->SetName().\n", id);
					break;
				}
				string name;
				ArrayH::getString(arrayptr, name, 31);
				strcpy(QMisc.bottle_shop_types[id].name, name.c_str());
				break;
			}
			
			case RUNITEMSCRIPT:
			{
				int32_t itemid = ri->idata;
				if(unsigned(itemid) > MAXITEMS) break;
				int32_t mode = get_register(sarg1) / 10000;
				auto& data = get_script_engine_data(ScriptType::Item, itemid);
				switch(mode)
				{
					case 0:
					{
						data.doscript = 4;
						break;
					}
					case 1:
					{
						if ( itemsbuf[itemid].script != 0 ) //&& !data.doscript )
						{
							if ( !data.doscript ) 
							{
								data.clear_ref();
								data.doscript = 1;
								//ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[itemid].script, itemid);
							}
							else
							{
								//Emily, clear the stack here, clear refinfo, and set up to run again on the next frame from the beginning.
							}
						}
						break;
					}
					case 2:
					default:
					{
						if ( itemsbuf[itemid].script != 0 ) //&& !data.doscript )
						{
							if (data.doscript != 2 )data.doscript = 2;
						}
						break;
					}
					/*
					case 0:
					{
						data.doscript = 0;
						break;
					}
					default:
					{
					
						if ( itemsbuf[itemid].script != 0 ) //&& !data.doscript )
						{
							//itemScriptData[itemid].Clear();
							//for ( int32_t q = 0; q < 1024; q++ ) item_stack[itemid][q] = 0;
							//ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[itemid].script, itemid & 0xFFF);
							data.doscript = 2;
						}
						break;
					}
					*/
				}
				break;
			}
			
			//case NPCData
			
			case GETNPCDATATILE: FFScript::getNPCData_tile(); break;
			case GETNPCDATAEHEIGHT: FFScript::getNPCData_e_height(); break;
			case GETNPCDATAFLAGS: FFScript::getNPCData_flags(); break;
			case GETNPCDATAFLAGS2: FFScript::getNPCData_flags2(); break;
			case GETNPCDATAWIDTH: FFScript::getNPCData_width(); break;
			case GETNPCDATAHEIGHT: FFScript::getNPCData_height(); break;
			case GETNPCDATASTILE: FFScript::getNPCData_s_tile(); break;
			case GETNPCDATASWIDTH: FFScript::getNPCData_s_width(); break;
			case GETNPCDATASHEIGHT: FFScript::getNPCData_s_height(); break;
			case GETNPCDATAETILE: FFScript::getNPCData_e_tile(); break;
			case GETNPCDATAEWIDTH: FFScript::getNPCData_e_width(); break;
			case GETNPCDATAHP: FFScript::getNPCData_hp(); break;
			case GETNPCDATAFAMILY: FFScript::getNPCData_family(); break;
			case GETNPCDATACSET: FFScript::getNPCData_cset(); break;
			case GETNPCDATAANIM: FFScript::getNPCData_anim(); break;
			case GETNPCDATAEANIM: FFScript::getNPCData_e_anim(); break;
			case GETNPCDATAFRAMERATE: FFScript::getNPCData_frate(); break;
			case GETNPCDATAEFRAMERATE: FFScript::getNPCData_e_frate(); break;
			case GETNPCDATATOUCHDMG: FFScript::getNPCData_dp(); break;
			case GETNPCDATAWPNDAMAGE: FFScript::getNPCData_wdp(); break;
			case GETNPCDATAWEAPON: FFScript::getNPCData_wdp(); break;
			case GETNPCDATARANDOM: FFScript::getNPCData_rate(); break;
			case GETNPCDATAHALT: FFScript::getNPCData_hrate(); break;
			case GETNPCDATASTEP: FFScript::getNPCData_step(); break;
			case GETNPCDATAHOMING: FFScript::getNPCData_homing(); break;
			case GETNPCDATAHUNGER: FFScript::getNPCData_grumble(); break;
			case GETNPCDATADROPSET: FFScript::getNPCData_item_set(); break;
			case GETNPCDATABGSFX: FFScript::getNPCData_bgsfx(); break;
			case GETNPCDATADEATHSFX: FFScript::getNPCData_deadsfx(); break; 
			case GETNPCDATAXOFS: FFScript::getNPCData_xofs(); break;
			case GETNPCDATAYOFS: FFScript::getNPCData_yofs(); break;
			case GETNPCDATAZOFS: FFScript::getNPCData_zofs(); break;
			case GETNPCDATAHXOFS: FFScript::getNPCData_hxofs(); break;
			case GETNPCDATAHYOFS: FFScript::getNPCData_hyofs(); break;
			case GETNPCDATAHITWIDTH: FFScript::getNPCData_hxsz(); break;
			case GETNPCDATAHITHEIGHT: FFScript::getNPCData_hysz(); break;
			case GETNPCDATAHITZ: FFScript::getNPCData_hzsz(); break;
			case GETNPCDATATILEWIDTH: FFScript::getNPCData_txsz(); break;
			case GETNPCDATATILEHEIGHT: FFScript::getNPCData_tysz(); break;
			case GETNPCDATAWPNSPRITE: FFScript::getNPCData_wpnsprite(); break;
			//case GETNPCDATASCRIPTDEF: FFScript::getNPCData_scriptdefence(); break; //2.future cross-compat. 
			case GETNPCDATADEFENSE: FFScript::getNPCData_defense(); break; 
			case GETNPCDATASIZEFLAG: FFScript::getNPCData_SIZEflags(); break;
			case GETNPCDATAATTRIBUTE: FFScript::getNPCData_misc(); break;
			case GETNPCDATAHITSFX: FFScript::getNPCData_hitsfx(); break;
				
			case SETNPCDATAFLAGS: FFScript::setNPCData_flags(); break;
			case SETNPCDATAFLAGS2: FFScript::setNPCData_flags2(); break;
			case SETNPCDATAWIDTH: FFScript::setNPCData_width(); break;
			case SETNPCDATAHEIGHT: FFScript::setNPCData_height(); break;
			case SETNPCDATASTILE: FFScript::setNPCData_s_tile(); break;
			case SETNPCDATASWIDTH: FFScript::setNPCData_s_width(); break;
			case SETNPCDATASHEIGHT: FFScript::setNPCData_s_height(); break;
			case SETNPCDATAETILE: FFScript::setNPCData_e_tile(); break;
			case SETNPCDATAEWIDTH: FFScript::setNPCData_e_width(); break;
			case SETNPCDATAHP: FFScript::setNPCData_hp(); break;
			case SETNPCDATAFAMILY: FFScript::setNPCData_family(); break;
			case SETNPCDATACSET: FFScript::setNPCData_cset(); break;
			case SETNPCDATAANIM: FFScript::setNPCData_anim(); break;
			case SETNPCDATAEANIM: FFScript::setNPCData_e_anim(); break;
			case SETNPCDATAFRAMERATE: FFScript::setNPCData_frate(); break;
			case SETNPCDATAEFRAMERATE: FFScript::setNPCData_e_frate(); break;
			case SETNPCDATATOUCHDMG: FFScript::setNPCData_dp(); break;
			case SETNPCDATAWPNDAMAGE: FFScript::setNPCData_wdp(); break;
			case SETNPCDATAWEAPON: FFScript::setNPCData_weapon(); break;
			case SETNPCDATARANDOM: FFScript::setNPCData_rate(); break;
			case SETNPCDATAHALT: FFScript::setNPCData_hrate(); break;
			case SETNPCDATASTEP: FFScript::setNPCData_step(); break;
			case SETNPCDATAHOMING: FFScript::setNPCData_homing(); break;
			case SETNPCDATAHUNGER: FFScript::setNPCData_grumble(); break;
			case SETNPCDATADROPSET: FFScript::setNPCData_item_set(); break;
			case SETNPCDATABGSFX: FFScript::setNPCData_bgsfx(); break;
			case SETNPCDATADEATHSFX: FFScript::setNPCData_hitsfx(); break;
			case SETNPCDATAXOFS: FFScript::setNPCData_xofs(); break;
			case SETNPCDATAYOFS: FFScript::setNPCData_yofs(); break;
			case SETNPCDATAZOFS: FFScript::setNPCData_zofs(); break;
			case SETNPCDATAHXOFS: FFScript::setNPCData_hxofs(); break;
			case SETNPCDATAHYOFS: FFScript::setNPCData_hyofs(); break;
			case SETNPCDATAHITWIDTH: FFScript::setNPCData_hxsz(); break;
			case SETNPCDATAHITHEIGHT: FFScript::setNPCData_hysz(); break;
			case SETNPCDATAHITZ: FFScript::setNPCData_hzsz(); break;
			case SETNPCDATATILEWIDTH: FFScript::setNPCData_txsz(); break;
			case SETNPCDATATILEHEIGHT: FFScript::setNPCData_tysz(); break;
			case SETNPCDATAWPNSPRITE: FFScript::setNPCData_wpnsprite(); break;
			case SETNPCDATAHITSFX: FFScript::setNPCData_hitsfx(); break;
			case SETNPCDATATILE: FFScript::setNPCData_tile(); break;
			case SETNPCDATAEHEIGHT: FFScript::setNPCData_e_height(); break;
			
			
			
			

				
			//case SETNPCDATASCRIPTDEF  : FFScript::setNPCData_scriptdefence(); break;
			case SETNPCDATADEFENSE : FFScript::setNPCData_defense(ri->d[rEXP1]); break;
			case SETNPCDATASIZEFLAG : FFScript::setNPCData_SIZEflags(ri->d[rEXP1]); break;
			case SETNPCDATAATTRIBUTE : FFScript::setNPCData_misc(ri->d[rEXP1]); break;
			
			
			//ComboData
			
			case GCDBLOCKENEM:  FFScript::getComboData_block_enemies(); break;
			case GCDBLOCKHOLE:  FFScript::getComboData_block_hole(); break;
			case GCDBLOCKTRIG:  FFScript::getComboData_block_trigger(); break;
			case GCDCONVEYSPDX:  FFScript::getComboData_conveyor_x_speed(); break;
			case GCDCONVEYSPDY:  FFScript::getComboData_conveyor_y_speed(); break;
			case GCDCREATEENEM:  FFScript::getComboData_create_enemy(); break;
			case GCDCREATEENEMWH:  FFScript::getComboData_create_enemy_when(); break;
			case GCDCREATEENEMCH:  FFScript::getComboData_create_enemy_change(); break;
			case GCDDIRCHTYPE:  FFScript::getComboData_directional_change_type(); break;
			case GCDDISTCHTILES:  FFScript::getComboData_distance_change_tiles(); break;
			case GCDDIVEITEM:  FFScript::getComboData_dive_item(); break;
			case GCDDOCK:  FFScript::getComboData_dock(); break;
			case GCDFAIRY:  FFScript::getComboData_fairy(); break;
			case GCDFFCOMBOATTRIB:  FFScript::getComboData_ff_combo_attr_change(); break;
			case GCDFOOTDECOTILE:  FFScript::getComboData_foot_decorations_tile(); break;
			case GCDFOOTDECOTYPE:  FFScript::getComboData_foot_decorations_type(); break;
			case GCDHOOKSHOTGRAB:  FFScript::getComboData_hookshot_grab_point(); break;
			case GCDLADDERPASS:  FFScript::getComboData_ladder_pass(); break;
			case GCDLOCKBLOCKTYPE:  FFScript::getComboData_lock_block_type(); break;
			case GCDLOCKBLOCKCHANGE:  FFScript::getComboData_lock_block_change(); break;
			case GCDMAGICMIRRORTYPE:  FFScript::getComboData_magic_mirror_type(); break;
			case GCDMODIFYHPAMOUNT:  FFScript::getComboData_modify_hp_amount(); break;
			case GCDMODIFYHPDELAY:  FFScript::getComboData_modify_hp_delay(); break;
			case GCDMODIFYHPTYPE:  FFScript::getComboData_modify_hp_type(); break;
			case GCDMODIFYMPAMOUNT:  FFScript::getComboData_modify_mp_amount(); break;
			case GCDMODIFYMPDELAY:  FFScript::getComboData_modify_mp_delay(); break;
			case GCDMODIFYMPTYPE:  FFScript::getComboData_modify_mp_type(); break;
			case GCDNOPUSHBLOCKS:  FFScript::getComboData_no_push_blocks(); break;
			case GCDOVERHEAD:  FFScript::getComboData_overhead(); break;
			case GCDPLACEENEMY:  FFScript::getComboData_place_enemy(); break;
			case GCDPUSHDIR:  FFScript::getComboData_push_direction(); break;
			case GCDPUSHWEIGHT:  FFScript::getComboData_push_weight(); break;
			case GCDPUSHWAIT:  FFScript::getComboData_push_wait(); break;
			case GCDPUSHED:  FFScript::getComboData_pushed(); break;
			case GCDRAFT:  FFScript::getComboData_raft(); break;
			case GCDRESETROOM:  FFScript::getComboData_reset_room(); break;
			case GCDSAVEPOINT:  FFScript::getComboData_save_point_type(); break;
			case GCDSCREENFREEZE:  FFScript::getComboData_screen_freeze_type(); break;
			case GCDSECRETCOMBO:  FFScript::getComboData_secret_combo(); break;
			case GCDSINGULAR:  FFScript::getComboData_singular(); break;
			case GCDSLOWMOVE:  FFScript::getComboData_slow_movement(); break;
			case GCDSTATUE:  FFScript::getComboData_statue_type(); break;
			case GCDSTEPTYPE:  FFScript::getComboData_step_type(); break;
			case GCDSTEPCHANGETO:  FFScript::getComboData_step_change_to(); break;
			case GCDSTRIKEREMNANTS:  FFScript::getComboData_strike_remnants(); break;
			case GCDSTRIKEREMNANTSTYPE:  FFScript::getComboData_strike_remnants_type(); break;
			case GCDSTRIKECHANGE:  FFScript::getComboData_strike_change(); break;
			case GCDSTRIKECHANGEITEM:  FFScript::getComboData_strike_item(); break;
			case GCDTOUCHITEM:  FFScript::getComboData_touch_item(); break;
			case GCDTOUCHSTAIRS:  FFScript::getComboData_touch_stairs(); break;
			case GCDTRIGGERTYPE:  FFScript::getComboData_trigger_type(); break;
			case GCDTRIGGERSENS:  FFScript::getComboData_trigger_sensitive(); break;
			case GCDWARPTYPE:  FFScript::getComboData_warp_type(); break;
			case GCDWARPSENS:  FFScript::getComboData_warp_sensitive(); break;
			case GCDWARPDIRECT:  FFScript::getComboData_warp_direct(); break;
			case GCDWARPLOCATION:  FFScript::getComboData_warp_location(); break;
			case GCDWATER:  FFScript::getComboData_water(); break;
			case GCDWHISTLE:  FFScript::getComboData_whistle(); break;
			case GCDWINGAME:  FFScript::getComboData_win_game(); break;
			case GCDBLOCKWEAPLVL:  FFScript::getComboData_block_weapon_lvl(); break;
			case GCDTILE:  FFScript::getComboData_tile(); break;
			case GCDFLIP:  FFScript::getComboData_flip(); break;
			case GCDWALK:  FFScript::getComboData_walk(); break;
			case GCDTYPE:  FFScript::getComboData_type(); break;
			case GCDCSETS:  FFScript::getComboData_csets(); break;
			case GCDFOO:  break;
			case GCDFRAMES:  FFScript::getComboData_frames(); break;
			case GCDSPEED:  FFScript::getComboData_speed(); break;
			case GCDNEXTCOMBO:  FFScript::getComboData_nextcombo(); break;
			case GCDNEXTCSET:  FFScript::getComboData_nextcset(); break;
			case GCDFLAG:  FFScript::getComboData_flag(); break;
			case GCDSKIPANIM:  FFScript::getComboData_skipanim(); break;
			case GCDNEXTTIMER:  FFScript::getComboData_nexttimer(); break;
			case GCDSKIPANIMY:  FFScript::getComboData_skipanimy(); break;
			case GCDANIMFLAGS:  FFScript::getComboData_animflags(); break;
			case GCDBLOCKWEAPON:  FFScript::getComboData_block_weapon(); break;
			case GCDSTRIKEWEAPONS:  FFScript::getComboData_strike_weapons(); break;
			case SCDBLOCKENEM:  FFScript::setComboData_block_enemies(); break;
			case SCDBLOCKHOLE:  FFScript::setComboData_block_hole(); break;
			case SCDBLOCKTRIG:  FFScript::setComboData_block_trigger(); break;
			case SCDCONVEYSPDX:  FFScript::setComboData_conveyor_x_speed(); break;
			case SCDCONVEYSPDY:  FFScript::setComboData_conveyor_y_speed(); break;
			case SCDCREATEENEM:  FFScript::setComboData_create_enemy(); break;
			case SCDCREATEENEMWH:  FFScript::setComboData_create_enemy_when(); break;
			case SCDCREATEENEMCH:  FFScript::setComboData_create_enemy_change(); break;
			case SCDDIRCHTYPE:  FFScript::setComboData_directional_change_type(); break;
			case SCDDISTCHTILES:  FFScript::setComboData_distance_change_tiles(); break;
			case SCDDIVEITEM:  FFScript::setComboData_dive_item(); break;
			case SCDDOCK:  FFScript::setComboData_dock(); break;
			case SCDFAIRY:  FFScript::setComboData_fairy(); break;
			case SCDFFCOMBOATTRIB:  FFScript::setComboData_ff_combo_attr_change(); break;
			case SCDFOOTDECOTILE:  FFScript::setComboData_foot_decorations_tile(); break;
			case SCDFOOTDECOTYPE:  FFScript::setComboData_foot_decorations_type(); break;
			case SCDHOOKSHOTGRAB:  FFScript::setComboData_hookshot_grab_point(); break;
			case SCDLADDERPASS:  FFScript::setComboData_ladder_pass(); break;
			case SCDLOCKBLOCKTYPE:  FFScript::setComboData_lock_block_type(); break;
			case SCDLOCKBLOCKCHANGE:  FFScript::setComboData_lock_block_change(); break;
			case SCDMAGICMIRRORTYPE:  FFScript::setComboData_magic_mirror_type(); break;
			case SCDMODIFYHPAMOUNT:  FFScript::setComboData_modify_hp_amount(); break;
			case SCDMODIFYHPDELAY:  FFScript::setComboData_modify_hp_delay(); break;
			case SCDMODIFYHPTYPE:  FFScript::setComboData_modify_hp_type(); break;
			case SCDMODIFYMPAMOUNT:  FFScript::setComboData_modify_mp_amount(); break;
			case SCDMODIFYMPDELAY:  FFScript::setComboData_modify_mp_delay(); break;
			case SCDMODIFYMPTYPE:  FFScript::setComboData_modify_mp_type(); break;
			case SCDNOPUSHBLOCKS:  FFScript::setComboData_no_push_blocks(); break;
			case SCDOVERHEAD:  FFScript::setComboData_overhead(); break;
			case SCDPLACEENEMY:  FFScript::setComboData_place_enemy(); break;
			case SCDPUSHDIR:  FFScript::setComboData_push_direction(); break;
			case SCDPUSHWEIGHT:  FFScript::setComboData_push_weight(); break;
			case SCDPUSHWAIT:  FFScript::setComboData_push_wait(); break;
			case SCDPUSHED:  FFScript::setComboData_pushed(); break;
			case SCDRAFT:  FFScript::setComboData_raft(); break;
			case SCDRESETROOM:  FFScript::setComboData_reset_room(); break;
			case SCDSAVEPOINT:  FFScript::setComboData_save_point_type(); break;
			case SCDSCREENFREEZE:  FFScript::setComboData_screen_freeze_type(); break;
			case SCDSECRETCOMBO:  FFScript::setComboData_secret_combo(); break;
			case SCDSINGULAR:  FFScript::setComboData_singular(); break;
			case SCDSLOWMOVE:  FFScript::setComboData_slow_movement(); break;
			case SCDSTATUE:  FFScript::setComboData_statue_type(); break;
			case SCDSTEPTYPE:  FFScript::setComboData_step_type(); break;
			case SCDSTEPCHANGETO:  FFScript::setComboData_step_change_to(); break;
			case SCDSTRIKEREMNANTS:  FFScript::setComboData_strike_remnants(); break;
			case SCDSTRIKEREMNANTSTYPE:  FFScript::setComboData_strike_remnants_type(); break;
			case SCDSTRIKECHANGE:  FFScript::setComboData_strike_change(); break;
			case SCDSTRIKECHANGEITEM:  FFScript::setComboData_strike_item(); break;
			case SCDTOUCHITEM:  FFScript::setComboData_touch_item(); break;
			case SCDTOUCHSTAIRS:  FFScript::setComboData_touch_stairs(); break;
			case SCDTRIGGERTYPE:  FFScript::setComboData_trigger_type(); break;
			case SCDTRIGGERSENS:  FFScript::setComboData_trigger_sensitive(); break;
			case SCDWARPTYPE:  FFScript::setComboData_warp_type(); break;
			case SCDWARPSENS:  FFScript::setComboData_warp_sensitive(); break;
			case SCDWARPDIRECT:  FFScript::setComboData_warp_direct(); break;
			case SCDWARPLOCATION:  FFScript::setComboData_warp_location(); break;
			case SCDWATER:  FFScript::setComboData_water(); break;
			case SCDWHISTLE:  FFScript::setComboData_whistle(); break;
			case SCDWINGAME:  FFScript::setComboData_win_game(); break;
			case SCDBLOCKWEAPLVL:  FFScript::setComboData_block_weapon_lvl(); break;
			case SCDTILE:  FFScript::setComboData_tile(); break;
			case SCDFLIP:  FFScript::setComboData_flip(); break;
			case SCDWALK:  FFScript::setComboData_walk(); break;
			case SCDTYPE:  FFScript::setComboData_type(); break;
			case SCDCSETS:  FFScript::setComboData_csets(); break;
			case SCDFOO:  break;
			case SCDFRAMES:  FFScript::setComboData_frames(); break;
			case SCDSPEED:  FFScript::setComboData_speed(); break;
			case SCDNEXTCOMBO:  FFScript::setComboData_nextcombo(); break;
			case SCDNEXTCSET:  FFScript::setComboData_nextcset(); break;
			case SCDFLAG:  FFScript::setComboData_flag(); break;
			case SCDSKIPANIM:  FFScript::setComboData_skipanim(); break;
			case SCDNEXTTIMER:  FFScript::setComboData_nexttimer(); break;
			case SCDSKIPANIMY:  FFScript::setComboData_skipanimy(); break;
			case SCDANIMFLAGS:  FFScript::setComboData_animflags(); break;
			case SCDBLOCKWEAPON:  FFScript::setComboData_block_weapon(ri->d[rEXP1]); break;
			case SCDSTRIKEWEAPONS:  FFScript::setComboData_strike_weapons(ri->d[rEXP1]); break;

			//SpriteData
			
			//case GETSPRITEDATASTRING: 
			case GETSPRITEDATATILE: FFScript::getSpriteDataTile(); break;
			case GETSPRITEDATAMISC: FFScript::getSpriteDataCSets(); break;
			case GETSPRITEDATACGETS: FFScript::getSpriteDataCSets(); break;
			case GETSPRITEDATAFRAMES: FFScript::getSpriteDataFrames(); break;
			case GETSPRITEDATASPEED: FFScript::getSpriteDataSpeed(); break;
			case GETSPRITEDATATYPE: FFScript::getSpriteDataType(); break;

			//case SETSPRITEDATASTRING:
			case SETSPRITEDATATILE: FFScript::setSpriteDataTile(); break;
			case SETSPRITEDATAMISC: FFScript::setSpriteDataMisc(); break;
			case SETSPRITEDATACSETS: FFScript::setSpriteDataCSets(); break;
			case SETSPRITEDATAFRAMES: FFScript::setSpriteDataFrames(); break;
			case SETSPRITEDATASPEED: FFScript::setSpriteDataSpeed(); break;
			case SETSPRITEDATATYPE: FFScript::setSpriteDataType(); break;
			
			//Game over Screen
			case SETCONTINUESCREEN: FFScript::FFChangeSubscreenText(); break;
			case SETCONTINUESTRING: FFScript::FFSetSaveScreenSetting(); break;
			
			case LWPNDEL:
			{
				if(type == ScriptType::Lwpn && ri->lwpn == i)
				{
					FFCore.do_lweapon_delete();
					return RUNSCRIPT_SELFDELETE;
				}

				FFCore.do_lweapon_delete();
				break;
			}
			case EWPNDEL:
			{
				if(type == ScriptType::Ewpn && ri->ewpn == i)
				{
					FFCore.do_eweapon_delete();
					return RUNSCRIPT_SELFDELETE;
				}

				FFCore.do_eweapon_delete();
				break;
			}
			
			case PLAYENHMUSICEX:
				// DEPRECATED
				do_enh_music(false);
				break;
				
			case GETENHMUSICPOS:
				FFCore.do_get_music_position();
				break;
				
			case SETENHMUSICPOS:
				FFCore.do_set_music_position(false);
				break;
				
			case SETENHMUSICSPEED:
				FFCore.do_set_music_speed(false);
				break;

			case GETENHMUSICLEN:
				FFCore.do_get_music_length();
				break;

			case SETENHMUSICLOOP:
				FFCore.do_set_music_loop();
				break;

			case ENHCROSSFADE:
				do_enh_music_crossfade();
				break;
			
			case DIREXISTS:
				FFCore.do_checkdir(true);
				break;
			
			case FILEEXISTS:
				FFCore.do_checkdir(false);
				break;
			
			case FILESYSREMOVE:
				FFCore.do_fs_remove();
				break;
			
			case TOBYTE:
				do_tobyte();
				break;
			case TOWORD:
				do_toword();
				break;
			case TOSHORT: do_toshort(); break;
			case TOSIGNEDBYTE: do_tosignedbyte(); break;
			case TOINTEGER: do_tointeger(); break;
			case CEILING: do_ceiling(); break;
			case FLOOR: do_floor(); break;
			case TRUNCATE: do_trunc(); break;
			case ROUND: do_round(); break;
			case ROUNDAWAY: do_roundaway(); break;
			
			case FILECLOSE:
			{
				FFCore.do_fclose();
				break;
			}
			case FILEFREE:
			{
				FFCore.do_deallocate_file();
				break;
			}
			case FILEOWN:
			{
				user_file* f = checkFile(ri->fileref, false);
				if(f) own_script_object(f, type, i);
				break;
			}
			case FILEISALLOCATED:
			{
				FFCore.do_file_isallocated();
				break;
			}
			case FILEISVALID:
			{
				FFCore.do_file_isvalid();
				break;
			}
			case FILEALLOCATE:
			{
				FFCore.do_allocate_file();
				break;
			}
			case FILEFLUSH:
			{
				FFCore.do_fflush();
				break;
			}
			case FILEREMOVE:
			{
				FFCore.do_fremove();
				break;
			}
			case FILEGETCHAR:
			{
				FFCore.do_file_getchar();
				break;
			}
			case FILEREWIND:
			{
				FFCore.do_file_rewind();
				break;
			}
			case FILECLEARERR:
			{
				FFCore.do_file_clearerr();
				break;
			}
			
			case FILEOPEN:
			{
				FFCore.do_fopen(false, "rb+");
				break;
			}
			case FILECREATE:
			{
				FFCore.do_fopen(false, "wb+");
				break;
			}
			case FILEOPENMODE:
			{
				int32_t arrayptr = get_register(sarg2);
				string mode;
				ArrayH::getString(arrayptr, mode, 16);
				FFCore.do_fopen(false, mode.c_str());
				break;
			}
			case FILEREADSTR:
			{
				FFCore.do_file_readstring();
				break;
			}
			case FILEWRITESTR:
			{
				FFCore.do_file_writestring();
				break;
			}
			case FILEPUTCHAR:
			{
				FFCore.do_file_putchar();
				break;
			}
			case FILEUNGETCHAR:
			{
				FFCore.do_file_ungetchar();
				break;
			}
			
			case FILEREADCHARS:
			{
				FFCore.do_file_readchars();
				break;
			}
			case FILEREADBYTES:
			{
				FFCore.do_file_readbytes();
				break;
			}
			case FILEREADINTS:
			{
				FFCore.do_file_readints();
				break;
			}
			case FILEWRITECHARS:
			{
				FFCore.do_file_writechars();
				break;
			}
			case FILEWRITEBYTES:
			{
				FFCore.do_file_writebytes();
				break;
			}
			case FILEWRITEINTS:
			{
				FFCore.do_file_writeints();
				break;
			}
			case FILESEEK:
			{
				FFCore.do_file_seek();
				break;
			}
			case FILEGETERROR:
			{
				FFCore.do_file_geterr();
				break;
			}
			//Directory
			case DIRECTORYGET:
			{
				FFCore.do_directory_get();
				break;
			}
			case DIRECTORYRELOAD:
			{
				FFCore.do_directory_reload();
				break;
			}
			case DIRECTORYFREE:
			{
				FFCore.do_directory_free();
				break;
			}
			case DIRECTORYOWN:
			{
				if(user_dir* dr = checkDir(ri->directoryref))
				{
					own_script_object(dr, type, i);
				}
				break;
			}
			//Stack
			case STACKFREE:
			{
				if(user_stack* st = checkStack(ri->stackref, true))
				{
					free_script_object(st->id);
				}
				break;
			}
			case STACKOWN:
			{
				if(user_stack* st = checkStack(ri->stackref))
				{
					own_script_object(st, type, i);
				}
				break;
			}
			case STACKCLEAR:
			{
				if(user_stack* st = checkStack(ri->stackref))
				{
					st->clearStack();
				}
				break;
			}
			case STACKGET:
			{
				if(user_stack* st = checkStack(ri->stackref, true))
				{
					int32_t indx = get_register(sarg1); //NOT /10000
					set_register(sarg1, st->get(indx)); //NOT *10000
				}
				else set_register(sarg1, 0L);
				break;
			}
			case STACKSET:
			{
				if(user_stack* st = checkStack(ri->stackref, true))
				{
					int32_t indx = get_register(sarg1); //NOT /10000
					int32_t val = get_register(sarg2); //NOT /10000
					st->set(indx, val); //NOT *10000
				}
				break;
			}
			case STACKPOPBACK:
			{
				if(user_stack* st = checkStack(ri->stackref, true))
				{
					set_register(sarg1, st->pop_back()); //NOT *10000
				}
				else set_register(sarg1, 0L);
				break;
			}
			case STACKPOPFRONT:
			{
				if(user_stack* st = checkStack(ri->stackref, true))
				{
					set_register(sarg1, st->pop_front()); //NOT *10000
				}
				else set_register(sarg1, 0L);
				break;
			}
			case STACKPEEKBACK:
			{
				if(user_stack* st = checkStack(ri->stackref, true))
				{
					set_register(sarg1, st->peek_back()); //NOT *10000
				}
				else set_register(sarg1, 0L);
				break;
			}
			case STACKPEEKFRONT:
			{
				if(user_stack* st = checkStack(ri->stackref, true))
				{
					set_register(sarg1, st->peek_front()); //NOT *10000
				}
				else set_register(sarg1, 0L);
				break;
			}
			case STACKPUSHBACK:
			{
				if(user_stack* st = checkStack(ri->stackref, true))
				{
					int32_t val = get_register(sarg1); //NOT /10000
					st->push_back(val);
				}
				break;
			}
			case STACKPUSHFRONT:
			{
				if(user_stack* st = checkStack(ri->stackref, true))
				{
					int32_t val = get_register(sarg1); //NOT /10000
					st->push_front(val);
				}
				break;
			}
			
			//Module
			case MODULEGETIC:
			{
				
				int32_t buf_pointer = SH::get_arg(sarg1, false) / 10000;
				int32_t element = SH::get_arg(sarg2, false) / 10000;
				
				if ( ((unsigned)element) > 511 )
				{
					Z_scripterrlog("Illegal itemclass supplied to ZInfo->GetItemClass().\nLegal values are 1 to 511.\n");
				}
				else
				{
					char buffer[256] = {0};
					strcpy(buffer,ZI.getItemClassName(element));
					buffer[255] = '\0';
					if(ArrayH::setArray(buf_pointer, buffer) == SH::_Overflow)
					{
						Z_scripterrlog("Dest string supplied to 'Module->GetItemClass()' is not large enough\n");
					}
				}
			
				break;
			}
			
			//{ Randgen Stuff
			case RNGRAND1:
				if(user_rng* r = checkRNG(ri->rngref))
				{
					ri->d[rEXP1] = r->rand(214748, -214748)*10000L;
				}
				else ri->d[rEXP1] = -10000L;
				break;
			case RNGRAND2:
				if(user_rng* r = checkRNG(ri->rngref))
				{
					set_register(sarg1,r->rand(get_register(sarg1)/10000L)*10000L);
				}
				else set_register(sarg1,-10000L);
				break;
			case RNGRAND3:
				if(user_rng* r = checkRNG(ri->rngref))
				{
					set_register(sarg1,r->rand(get_register(sarg1)/10000L, get_register(sarg2)/10000L)* 10000L);
				}
				else set_register(sarg1,-10000L);
				break;
			case RNGLRAND1:
				if(user_rng* r = checkRNG(ri->rngref))
				{
					ri->d[rEXP1] = r->rand();
				}
				else ri->d[rEXP1] = -10000L;
				break;
			case RNGLRAND2:
				if(user_rng* r = checkRNG(ri->rngref))
				{
					ri->d[rEXP1] = r->rand(get_register(sarg1));
				}
				else ri->d[rEXP1] = -10000L;
				break;
			case RNGLRAND3:
				if(user_rng* r = checkRNG(ri->rngref))
				{
					ri->d[rEXP1] = r->rand(get_register(sarg1), get_register(sarg2));
				}
				else ri->d[rEXP1] = -10000L;
				break;
			case RNGSEED:
				if(user_rng* r = checkRNG(ri->rngref))
				{
					r->srand(get_register(sarg1));
				}
				break;
			case RNGRSEED:
				if(user_rng* r = checkRNG(ri->rngref))
				{
					ri->d[rEXP1] = r->srand();
				}
				else ri->d[rEXP1] = -10000;
				break;
			case RNGFREE:
				if(user_rng* r = checkRNG(ri->rngref, true))
				{
					free_script_object(r->id);
				}
				break;
			case RNGOWN:
				if(user_rng* r = checkRNG(ri->rngref, false))
				{
					own_script_object(r, type, i);
				}
				break;
			//}
			case LOADGENERICDATA:
				FFCore.do_loadgenericdata(false); break;
			case RUNGENFRZSCR:
			{
				bool r = FFCore.runGenericFrozenEngine(word(ri->genericdataref));
				set_register(sarg1, r ? 10000L : 0L);
				break;
			}
			
			///----------------------------------------------------------------------------------------------------//
			
			case SUBDATA_GET_NAME:
			{
				if(ZCSubscreen* sub = checkSubData(ri->subdataref))
				{
					auto aptr = get_register(sarg1);
					if(ArrayH::setArray(aptr, sub->name, true) == SH::_Overflow)
						Z_scripterrlog("Array supplied to 'subscreendata->GetName()' not large enough,"
							" and couldn't be resized!\n");
				}
				break;
			}
			case SUBDATA_SET_NAME:
			{
				if(ZCSubscreen* sub = checkSubData(ri->subdataref))
				{
					auto aptr = get_register(sarg1);
					ArrayH::getString(aptr, sub->name);
				}
				break;
			}
			case SUBDATA_SWAP_PAGES:
			{
				ri->subdataref = SH::read_stack(ri->sp+2);
				if(ZCSubscreen* sub = checkSubData(ri->subdataref))
				{
					int p1 = SH::read_stack(ri->sp+1) / 10000;
					int p2 = SH::read_stack(ri->sp+0) / 10000;
					if(unsigned(p1) >= sub->pages.size())
						Z_scripterrlog("Invalid page index '%d' passed to subscreendata->SwapPages()\n", p1);
					else if(unsigned(p2) >= sub->pages.size())
						Z_scripterrlog("Invalid page index '%d' passed to subscreendata->SwapPages()\n", p2);
					else sub->swap_pages(p1,p2);
				}
				break;
			}
			case SUBPAGE_SWAP_WIDG:
			{
				ri->subpageref = SH::read_stack(ri->sp+2);
				if(SubscrPage* pg = checkSubPage(ri->subpageref))
				{
					int p1 = SH::read_stack(ri->sp+1) / 10000;
					int p2 = SH::read_stack(ri->sp+0) / 10000;
					if(unsigned(p1) >= pg->size())
						Z_scripterrlog("Invalid page index '%d' passed to subscreenpage->SwapWidgets()\n", p1);
					else if(unsigned(p2) >= pg->size())
						Z_scripterrlog("Invalid page index '%d' passed to subscreenpage->SwapWidgets()\n", p2);
					else pg->swap_widg(p1,p2);
				}
				break;
			}
			case SUBPAGE_FIND_WIDGET:
			{
				ri->d[rEXP1] = 0;
				ri->subpageref = SH::read_stack(ri->sp+1);
				if(SubscrPage* pg = checkSubPage(ri->subpageref, sstACTIVE))
				{
					int cursorpos = SH::read_stack(ri->sp+0) / 10000;
					if(auto* widg = pg->get_widg_pos(cursorpos,false))
					{
						auto q = pg->widget_index(widg);
						if(q > -1)
						{
							auto [sub,ty,pgid,_ind] = from_subref(ri->subpageref);
							ri->d[rEXP1] = get_subref(sub,ty,pgid,q);
						}
					}
				}
				break;
			}
			case SUBPAGE_FIND_WIDGET_BY_LABEL:
			{
				ri->d[rEXP1] = 0;
				ri->subpageref = SH::read_stack(ri->sp+1);
				if(SubscrPage* pg = checkSubPage(ri->subpageref))
				{
					int aptr = SH::read_stack(ri->sp+0);
					std::string lbl;
					ArrayH::getString(aptr, lbl);
					if(lbl.size())
					{
						auto q = pg->find_label_index(lbl);
						if(q > -1)
						{
							auto [sub,ty,pgid,_ind] = from_subref(ri->subpageref);
							ri->d[rEXP1] = get_subref(sub,ty,pgid,q);
						}
					}
				}
				break;
			}
			case SUBPAGE_MOVE_SEL:
			{
				#define SUBSEL_FLAG_NO_NONEQUIP 0x01
				#define SUBSEL_FLAG_NEED_ITEM 0x02
				ri->subpageref = SH::read_stack(ri->sp+3);
				if(SubscrPage* pg = checkSubPage(ri->subpageref))
				{
					int flags = SH::read_stack(ri->sp+0) / 10000;
					int dir = SH::read_stack(ri->sp+1) / 10000;
					int pos = SH::read_stack(ri->sp+2) / 10000;
					switch(dir)
					{
						case up:
							dir = SEL_UP;
							break;
						case down:
							dir = SEL_DOWN;
							break;
						case left:
							dir = SEL_LEFT;
							break;
						case right: default:
							dir = SEL_RIGHT;
							break;
					}
					
					auto newpos = pg->movepos_legacy(dir, (pos<<8)|pg->getIndex(),
						255, 255, 255, flags&SUBSEL_FLAG_NO_NONEQUIP,
						flags&SUBSEL_FLAG_NEED_ITEM, true) >> 8;
					ri->d[rEXP1] = 10000*newpos;
				}
				break;
			}
			case SUBPAGE_NEW_WIDG:
			{
				ri->subpageref = SH::read_stack(ri->sp+1);
				if(SubscrPage* pg = checkSubPage(ri->subpageref))
				{
					if(pg->size() == 0x2000)
						break; //Page is full!
					int ty = SH::read_stack(ri->sp+0) / 10000;
					if(auto* widg = SubscrWidget::newType(ty))
					{
						widg->posflags = sspUP | sspDOWN | sspSCROLLING;
						widg->w = 1;
						widg->h = 1;
						pg->push_back(widg);
						auto [sub,ty,pgid,_ind] = from_subref(ri->subpageref);
						ri->d[rEXP1] = get_subref(sub,ty,pgid,pg->size()-1);
					}
					else Z_scripterrlog("Invalid type %d passed to subscreenpage->CreateWidget()\n",ty);
				}
				break;
			}
			case SUBPAGE_DELETE:
			{
				if(SubscrPage* pg = checkSubPage(ri->subpageref))
				{
					auto [sub,_ty] = load_subdata(ri->subpageref);
					sub->delete_page(pg->getIndex());
				}
				break;
			}
			case SUBWIDG_GET_SELTEXT_OVERRIDE:
			{
				if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				{
					auto aptr = get_register(sarg1);
					if(ArrayH::setArray(aptr, widg->override_text, true) == SH::_Overflow)
						Z_scripterrlog("Array supplied to 'subscreenwidget->GetSelTextOverride()' not large enough,"
							" and couldn't be resized!\n");
				}
				break;
			}
			case SUBWIDG_SET_SELTEXT_OVERRIDE:
			{
				if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				{
					auto aptr = get_register(sarg1);
					ArrayH::getString(aptr, widg->override_text);
				}
				break;
			}
			case SUBWIDG_GET_LABEL:
			{
				if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				{
					auto aptr = get_register(sarg1);
					if(ArrayH::setArray(aptr, widg->label, true) == SH::_Overflow)
						Z_scripterrlog("Array supplied to 'subscreenwidget->GetLabel()' not large enough,"
							" and couldn't be resized!\n");
				}
				break;
			}
			case SUBWIDG_SET_LABEL:
			{
				if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				{
					auto aptr = get_register(sarg1);
					ArrayH::getString(aptr, widg->label);
				}
				break;
			}
			case SUBWIDG_CHECK_CONDITIONS:
			{
				if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				{
					set_register(sarg1, widg->check_conditions() ? 10000 : 0);
				}
				break;
			}
			case SUBWIDG_CHECK_VISIBLE:
			{
				if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				{
					extern int current_subscr_pos;
					set_register(sarg1, widg->visible(current_subscr_pos, game->should_show_time()) ? 10000 : 0);
				}
				break;
			}
			case SUBWIDG_TY_GETTEXT:
			{
				if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				{
					std::string const* str = nullptr;
					byte ty = widg->getType();
					switch(ty)
					{
						case widgTEXT:
							str = &((SW_Text*)widg)->text;
							break;
						case widgTEXTBOX:
							str = &((SW_TextBox*)widg)->text;
							break;
						default:
							bad_subwidg_type(true, ty);
							break;
					}
					if(str)
					{
						auto aptr = get_register(sarg1);
						if(ArrayH::setArray(aptr, *str, true) == SH::_Overflow)
							Z_scripterrlog("Array supplied to 'subscreenwidget->GetText()' not large enough,"
								" and couldn't be resized!\n");
					}
				}
				break;
			}
			case SUBWIDG_TY_SETTEXT:
			{
				if(SubscrWidget* widg = checkSubWidg(ri->subwidgref))
				{
					std::string* str = nullptr;
					byte ty = widg->getType();
					switch(ty)
					{
						case widgTEXT:
							str = &((SW_Text*)widg)->text;
							break;
						case widgTEXTBOX:
							str = &((SW_TextBox*)widg)->text;
							break;
						default:
							bad_subwidg_type(true, ty);
							break;
					}
					if(str)
					{
						auto aptr = get_register(sarg1);
						ArrayH::getString(aptr, *str);
					}
				}
				break;
			}
			
			case COMBOD_GET_TRIGGER:
			{
				if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) )
				{
					scripting_log_error_with_context("Invalid combodata ID: {}", ri->combosref);
				}
				else
				{
					auto aptr = get_register(sarg1) / 10000;
					string name;
					ArrayH::getString(aptr, name, 256);
					newcombo const& cmb = combobuf[ri->combosref];
					int32_t ret = 0;
					for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
					{
						if(cmb.triggers[idx].label == name)
						{
							ret = dword(ri->combosref) | (dword(idx)<<24);
							break;
						}
					}
					
					set_register(sarg1, ret);
				}
				break;
			}
			case CMBTRIG_GET_LABEL:
			{
				if(auto* trig = get_combo_trigger(ri->combotrigref))
				{
					auto aptr = get_register(sarg1) / 10000;
					if(ArrayH::setArray(aptr, trig->label, true) == SH::_Overflow)
						Z_scripterrlog("Array supplied to 'combotrigger->GetLabel()' not large enough,"
							" and couldn't be resized!\n");
				}
				break;
			}
			case CMBTRIG_SET_LABEL:
			{
				if (auto* trig = get_combo_trigger(ri->combotrigref))
				{
					auto aptr = get_register(sarg1) / 10000;
					ArrayH::getString(aptr, trig->label);
				}
				break;
			}
			
			case REF_INC:
			{
				int offset = ri->d[rSFRAME] + sarg1;
				if (!ri->stack_pos_is_object.contains(offset))
				{
					assert(false);
					break;
				}

				uint32_t id = SH::read_stack(offset);
				script_object_ref_inc(id);
				break;
			}
			case REF_DEC:
			{
				int offset = ri->d[rSFRAME] + sarg1;
				if (!ri->stack_pos_is_object.contains(offset))
				{
					assert(false);
					break;
				}

				uint32_t id = SH::read_stack(offset);
				script_object_ref_dec(id);
				break;
			}
			case REF_AUTORELEASE:
			{
				uint32_t id = get_register(sarg1);
				if (id && !util::contains(script_object_autorelease_pool, id))
				{
					script_object_autorelease_pool.push_back(id);
					if (auto object = get_script_object_checked(id))
						object->ref_count++;
				}
				break;
			}
			case REF_COUNT:
			{
				if (!use_testingst_start)
				{
					scripting_log_error_with_context("This function can only be used in test mode");
					break;
				}

				uint32_t id = get_register(sarg1);
				auto object = get_script_object(id);
				int count = object ? object->ref_count : -1;
				set_register(sarg1, count);
				break;
			}
			case MARK_TYPE_STACK:
			{
				int offset = ri->d[rSFRAME] + sarg2;
				if (offset < 0 || offset >= MAX_STACK_SIZE)
				{
					assert(false);
					break;
				}
				if (sarg1 < 0 || sarg1 > 1)
				{
					assert(false);
					break;
				}

				if (sarg1)
					ri->stack_pos_is_object.insert(offset);
				else
					ri->stack_pos_is_object.erase(offset);
				break;
			}
			case MARK_TYPE_REG:
			{
				markRegisterType(sarg1, sarg2);
				break;
			}
			case REF_REMOVE:
			{
				int offset = ri->d[rSFRAME] + sarg1;
				script_remove_object_ref(offset);
				break;
			}
			case GC:
			{
				if (!use_testingst_start)
				{
					Z_error_fatal("GC can only be used in test mode\n");
					break;
				}

				run_gc();
				break;
			}
			case SET_OBJECT:
			{
				if (!(sarg1 >= GD(0) && sarg1 <= GD(MAX_SCRIPT_REGISTERS)))
				{
					assert(false);
					break;
				}

				int value = get_register(sarg2);
				int index = sarg1-GD(0);
				assert(game->global_d_types[index] != script_object_type::none);
				script_object_ref_inc(value);
				script_object_ref_dec(game->global_d[index]);
				game->global_d[index] = value;
				break;
			}
			case LOAD_INTERNAL_ARRAY:
			{
				do_load_internal_array();
				break;
			}
			case LOAD_INTERNAL_ARRAY_REF:
			{
				do_load_internal_array_ref();
				break;
			}

			default:
			{
				if (auto r = scripting_engine_run_command(scommand))
				{
					if (*r != RUNSCRIPT_OK)
						return *r;
					break;
				}

				scripting_log_error_with_context("Invalid ZASM command {} reached; terminating", scommand);
				hit_invalid_zasm = true;
				scommand = 0xFFFF;
				break;
			}
		}
		if(earlyretval == RUNSCRIPT_SELFDELETE)
		{
			earlyretval = -1;
			return RUNSCRIPT_SELFDELETE;
		}
		if (ri->overflow)
		{
			if (old_script_funcrun)
				return RUNSCRIPT_OK;
			scommand = 0xFFFF;
		}
		if(hit_invalid_zasm) break;
		if(old_script_funcrun && (ri->pc == MAX_PC || scommand == RETURN))
			return RUNSCRIPT_OK;
		
		if (type == ScriptType::Combo)
		{
			if(combopos_modified == i)
			{
				//Combo changed! Abort script!
				return RUNSCRIPT_OK;
			}
		}
		if(scommand != 0xFFFF)
		{
			if(increment)	ri->pc++;
			else			increment = true;
			if ( ri->pc == MAX_PC ) //rolled over from overflow?
			{
				Z_scripterrlog("Script PC overflow! Too many ZASM lines?\n");
				ri->pc = curscript->pc;
				scommand = 0xFFFF;
			}
		}
		
		if(earlyretval > -1) //Should this be below the 'commands_run += 1'? Unsure. -Em
		{
			auto v = earlyretval;
			earlyretval = -1;
			return earlyretval;
		}
		
		// If running a JIT compiled script, we're only here to do a few commands.
		commands_run += 1;
		if (is_jitted && commands_run == jitted_uncompiled_command_count)
		{
			current_zasm_command=(ASM_DEFINE)0;
			break;
		}
	}
	if(script_funcrun) return RUNSCRIPT_OK;
	
	if(!scriptCanSave)
		scriptCanSave=true;
	
	if(scommand == WAITDRAW)
	{
		switch(type)
		{
			case ScriptType::Global:
			case ScriptType::Hero:
			case ScriptType::DMap:
			case ScriptType::OnMap:
			case ScriptType::ScriptedPassiveSubscreen:
			case ScriptType::ScriptedActiveSubscreen:
			case ScriptType::Screen:
			case ScriptType::Combo:
			case ScriptType::NPC:
			case ScriptType::Lwpn:
			case ScriptType::Ewpn:
			case ScriptType::ItemSprite:
				FFCore.waitdraw(type, i) = true;
				break;
			
			case ScriptType::Item:
			{
				if (!get_qr(qr_NOITEMWAITDRAW))
				{
					FFCore.waitdraw(ScriptType::Item, i) = true;
				}
				break;
			}
						
			case ScriptType::FFC:
			{
				if ( !(get_qr(qr_NOFFCWAITDRAW)) )
				{
					FFCore.waitdraw(ScriptType::FFC, i) = true;
				}
				else
				{
					Z_scripterrlog("Waitdraw cannot be used in script type: %s\n", "ffc, with Script Rule 'No FFC Waitdraw() enabled!");
				}
				break;
			}
			
			case ScriptType::Generic:
			case ScriptType::GenericFrozen:
			case ScriptType::EngineSubscreen:
				//No Waitdraw
				break;
			
			default:
				Z_scripterrlog("Waitdraw cannot be used in script type: %s\n", ScriptTypeToString(type));
				break;
		}
	}
	
	if(scommand == 0xFFFF) //Quit/command list end reached/bad command
	{
		script_exit_cleanup(no_dealloc);
		return RUNSCRIPT_STOPPED;
	}
	else
		ri->pc++;

	if(jit_waiting_nop)
		return RUNSCRIPT_STOPPED;

	return RUNSCRIPT_OK;
}

script_data* load_scrdata(ScriptType type, word script, int32_t i)
{
	switch(type)
	{
		case ScriptType::FFC:
			return ffscripts[script];
		case ScriptType::NPC:
			return guyscripts[guys.getByUID(i)->script];
		case ScriptType::Lwpn:
			return lwpnscripts[Lwpns.getByUID(i)->script];
		case ScriptType::Ewpn:
			return ewpnscripts[Ewpns.getByUID(i)->script];
		case ScriptType::ItemSprite:
			return itemspritescripts[items.getByUID(i)->script];
		case ScriptType::Item:
			return itemscripts[script];
		case ScriptType::Global:
			return globalscripts[script];
		case ScriptType::Generic:
		case ScriptType::GenericFrozen:
			return genericscripts[script];
		case ScriptType::Hero:
			return playerscripts[script];
		case ScriptType::DMap:
			return dmapscripts[script];
		case ScriptType::OnMap:
		case ScriptType::ScriptedActiveSubscreen:
		case ScriptType::ScriptedPassiveSubscreen:
			return dmapscripts[script];
		case ScriptType::Screen:
			return screenscripts[script];
		case ScriptType::Combo:
			return comboscripts[script];
		case ScriptType::EngineSubscreen:
			return subscreenscripts[script];
	}
	return nullptr;
}

//This keeps ffc scripts running beyond the first frame. 
int32_t ffscript_engine(const bool preload)
{
	if(preload)
	{
		throwGenScriptEvent(GENSCR_EVENT_FFC_PRELOAD);
		handle_region_load_trigger();
	}

	if (!FFCore.system_suspend[susptSCREENSCRIPTS] && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 && !get_qr(qr_ZS_OLD_SUSPEND_FFC))
	{
		for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
			if ((preload && scr->preloadscript) || !preload)
			{
				if (scr->script > 0 && FFCore.doscript(ScriptType::Screen, scr->screen))
				{
					ZScriptVersion::RunScript(ScriptType::Screen, scr->script, scr->screen);
				}
			}
			});
	}

	if (!FFCore.system_suspend[susptFFCSCRIPTS])
	{
		//intentional it's for compatability
		if (FFCore.getQuestHeaderInfo(vZelda) >= 0x255 && get_qr(qr_ZS_OLD_SUSPEND_FFC))
		{
			for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
				if ((preload && scr->preloadscript) || !preload)
				{
					if (scr->script > 0 && FFCore.doscript(ScriptType::Screen, scr->screen))
					{
						ZScriptVersion::RunScript(ScriptType::Screen, scr->script, scr->screen);
					}
				}
				});
		}

		for_every_ffc([&](const ffc_handle_t& ffc_handle) {
			if(ffc_handle.ffc->script == 0)
				return;
				
			if(preload && !(ffc_handle.ffc->flags&ffc_preload))
				return;
				
			if((ffc_handle.ffc->flags&ffc_ignoreholdup)==0 && Hero.getHoldClk()>0)
				return;

			ZScriptVersion::RunScript(ScriptType::FFC, ffc_handle.ffc->script, ffc_handle.id);
		});
	}
	
	
	return 0;
}



///----------------------------------------------------------------------------------------------------

void FFScript::user_files_init()
{
	user_files.clear();
}

void FFScript::user_dirs_init()
{
	user_dirs.clear();
}
void FFScript::user_objects_init()
{
	::user_object_init();
}

void FFScript::user_stacks_init()
{
	user_stacks.clear();
}

void FFScript::user_rng_init()
{
	user_rngs.clear();
	for(int32_t q = 0; q < MAX_USER_RNGS; ++q)
	{
		replay_register_rng(&script_rnggens[q]);

		// Just to seed it.
		user_rng rng;
		rng.set_gen(&script_rnggens[q]);
	}
}

void FFScript::user_paldata_init()
{
	user_paldatas.clear();
}

void FFScript::user_websockets_init()
{
	websocket_init();
}

void FFScript::script_arrays_init()
{
	script_arrays.clear();
}

// Gotten from 'https://fileinfo.com/filetypes/executable'
static std::set<std::string> banned_extensions = {".xlm",".caction",".8ck", ".actc",".a6p", ".m3g",".run",".workflow",".otm",".apk",".fxp",".73k",".0xe",".exe",".cmd",".jsx",".scar",".wcm",".jar",".ebs2",".ipa",".xap",".ba_",".ac",".bin",".vlx",".icd",".elf",".xbap",".89k",".widget",".a7r",".ex_",".zl9",".cgi",".scr",".coffee",".ahk",".plsc",".air",".ear",".app",".scptd",".xys",".hms",".cyw",".ebm",".pwc",".xqt",".msl",".seed",".vexe",".ebs",".mcr",".gpu",".celx",".wsh",".frs",".vxp",".action",".com",".out",".gadget",".command",".script",".rfu",".tcp",".widget",".ex4",".bat",".cof",".phar",".rxe",".scb",".ms",".isu",".fas",".mlx",".gpe",".mcr",".mrp",".u3p",".js",".acr",".epk",".exe1",".jsf",".rbf",".rgs",".vpm",".ecf",".hta",".dld",".applescript",".prg",".pyc",".spr",".nexe",".server",".appimage",".pyo",".dek",".mrc",".fpi",".rpj",".iim",".vbs",".pif",".mel",".scpt",".csh",".paf",".ws",".mm",".acc",".ex5",".mac",".plx",".snap",".ps1",".vdo",".mxe",".gs",".osx",".sct",".wiz",".x86",".e_e",".fky",".prg",".fas",".azw2",".actm",".cel",".tiapp",".thm",".kix",".wsf",".vbe",".lo",".ls",".tms",".ezs",".ds",".n",".esh",".vbscript",".arscript",".qit",".pex",".dxl",".wpm",".s2a",".sca",".prc",".shb",".rbx",".jse",".beam",".udf",".mem",".kx",".ksh",".rox",".upx",".ms",".mam",".btm",".es",".asb",".ipf",".mio",".sbs",".hpf",".ita",".eham",".ezt",".dmc",".qpx",".ore",".ncl",".exopc",".smm",".pvd",".ham",".wpk"};

// If the path is valid, returns an absolute path under the quest "Files" directory.
static expected<std::string, std::string> parse_user_path(const std::string& user_path, bool is_file)
{
	// First check for non-portable path characters.
	static const char* invalid_chars = "<>|?*&^$#\":";
	if (auto index = user_path.find_first_of(invalid_chars) != string::npos)
	{
		return make_unexpected(fmt::format("Bad path: {} - invalid character {}", user_path, user_path[index]));
	}
	for (char c : user_path)
	{
		if (c < 32)
			return make_unexpected(fmt::format("Bad path: {} - invalid control character {:#x}", user_path, c));
	}

	// Any leading slashes are ignored.
	// This makes path always relative.
	const char* path = user_path.c_str();
	while (path[0] == '/' || path[0] == '\\')
		path++;

	// Normalize `user_path` and check if it accesses a parent path.
	auto files_path = fs::absolute(fs::path(qst_files_path));
	auto normalized_path = fs::path(path).lexically_normal();
	if (!normalized_path.empty() && normalized_path.begin()->string() == "..")
	{
		return make_unexpected(fmt::format("Bad path: {} (resolved to {}) - cannot access filesystem outside {} (too many ..?)",
			path, normalized_path.string(), files_path.string()));
	}

	auto resolved_path = files_path / normalized_path;

	// The above should be enough to guarantee that `resolved_path` is within
	// the quest "Files" folder, but check to be safe.
	auto mismatch_pair = std::mismatch(
		resolved_path.begin(), resolved_path.end(),
		files_path.begin(), files_path.end());
	bool is_subpath = mismatch_pair.second == files_path.end();
	if (!is_subpath)
	{
		return make_unexpected(fmt::format("Bad path: {} (resolved to {}) - cannot access filesystem outside {}",
			user_path, resolved_path.string(), files_path.string()));
	}

	// Any extension other than banned ones, including no extension, is allowed.
	if (is_file && resolved_path.has_extension())
	{
		auto ext = resolved_path.extension().string();
		if (banned_extensions.find(ext) != banned_extensions.end())
			return make_unexpected(fmt::format("Bad path: {} - banned extension", user_path));
	}

	if (is_file && !resolved_path.has_filename())
		return make_unexpected(fmt::format("Bad path: {} - missing filename", user_path));

	// https://stackoverflow.com/a/31976060/2788187
	if (is_file)
	{
		static auto banned_fnames = {
			"..", ".", "AUX", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6",
			"COM7", "COM8", "COM9", "CON", "LPT1", "LPT2", "LPT3", "LPT4",
			"LPT5", "LPT6", "LPT7", "LPT8", "LPT9", "NUL", "PRN",
		};

		auto stem = resolved_path.stem().string();
		auto fname = resolved_path.filename().string();
		bool banned = std::find(std::begin(banned_fnames), std::end(banned_fnames), stem) != std::end(banned_fnames);
		banned |= fname.ends_with(".") || fname.ends_with(" ");

		if (banned)
			return make_unexpected(fmt::format("Bad path: {} - banned filename", user_path));
	}

	return resolved_path.string();
}

bool FFScript::get_scriptfile_path(char* buf, const char* path)
{
	while((path[0] == '/' || path[0] == '\\') && path[0]) ++path;
	if(path[0])
		sprintf(buf, "%s%c%s", qst_files_path, PATH_SLASH, path);
	else sprintf(buf, "%s", qst_files_path);
	return true;
}

void check_file_error(int32_t ref)
{
	if(user_file* f = checkFile(ref, true, true))
	{
		int32_t err = ferror(f->file);
		if(err != 0)
		{
			Z_scripterrlog("File with UID '%d' encountered an error.\n", ref);
			Z_scripterrlog("File error: %s\n", strerror(err));
		}
	}
}

void FFScript::do_fopen(const bool v, const char* f_mode)
{
	int32_t arrayptr = SH::get_arg(sarg1, v);
	string user_path;
	ArrayH::getString(arrayptr, user_path, 512);

	ri->d[rEXP1] = 0L; //Presume failure; update to 10000L on success
	ri->d[rEXP2] = 0;

	std::string resolved_path;
	if (auto r = parse_user_path(user_path, true); !r)
	{
		scripting_log_error_with_context("Error: {}", r.error());
		return;
	} else resolved_path = r.value();

	user_file* f = checkFile(ri->fileref, false, true);
	if(!f) //auto-allocate
	{
		ri->fileref = user_files.get_free();
		f = checkFile(ri->fileref, false, true);
	}
	ri->d[rEXP2] = ri->fileref; //Returns to the variable!
	if(f)
	{
		f->close(); //Close the old FILE* before overwriting it!
		bool create = false;
		for(int32_t q = 0; f_mode[q]; ++q)
		{
			if(f_mode[q] == 'w' || f_mode[q] == 'a')
			{
				create = true;
				break;
			}
		}
		if(!create || make_dirs_for_file(resolved_path))
		{
			f->file = fopen(resolved_path.c_str(), f_mode);
			fflush(f->file);
			zc_chmod(resolved_path.c_str(), SCRIPT_FILE_MODE);
			f->setPath(resolved_path.c_str());
			//r+; read-write, will not create if does not exist, will not delete content if does exist.
			//w+; read-write, will create if does not exist, will delete all content if does exist.
			if(f->file)
			{
				ri->d[rEXP1] = 10000L; //Success
				return;
			}
		}
		else
		{
			Z_scripterrlog("Script failed to create directories for file path '%s'.\n", resolved_path.c_str());
			ri->d[rEXP2] = 0;
			return;
		}
	}
}

void FFScript::do_fremove()
{
	if(user_file* f = checkFile(ri->fileref, true))
	{
		zprint2("Removing file %d\n", ri->fileref);
		ri->d[rEXP1] = f->do_remove() ? 0L : 10000L;
	}
	else ri->d[rEXP1] = 0L;
}

void FFScript::do_fclose()
{
	if(user_file* f = checkFile(ri->fileref, false, true))
	{
		f->close();
	}
	//No else. If invalid, no error is thrown.
}

void FFScript::do_allocate_file()
{
	//Get a file and return it
	ri->fileref = user_files.get_free();
	ri->d[rEXP2] = ri->fileref; //Return to ptr
	ri->d[rEXP1] = (ri->d[rEXP2] == 0 ? 0L : 10000L);
}

void FFScript::do_deallocate_file()
{
	user_file* f = checkFile(ri->fileref, false, true);
	if(f) free_script_object(f->id);
}

void FFScript::do_file_isallocated() //Returns true if file is allocated
{
	user_file* f = checkFile(ri->fileref, false, true);
	ri->d[rEXP1] = (f) ? 10000L : 0L;
}

void FFScript::do_file_isvalid() //Returns true if file is allocated and has an open FILE*
{
	user_file* f = checkFile(ri->fileref, true, true);
	ri->d[rEXP1] = (f) ? 10000L : 0L;
}

void FFScript::do_fflush()
{
	ri->d[rEXP1] = 0L;
	if(user_file* f = checkFile(ri->fileref, true))
	{
		if(!fflush(f->file))
			ri->d[rEXP1] = 10000L;
		check_file_error(ri->fileref);
	}
}

void FFScript::do_file_readchars()
{
	if(user_file* f = checkFile(ri->fileref, true))
	{
		uint32_t pos = zc_max(ri->d[rINDEX] / 10000,0);
		int32_t count = get_register(sarg2) / 10000;
		if(count == 0) return;
		int32_t arrayptr = get_register(sarg1);
		ArrayManager am(arrayptr);
		int32_t sz = am.size();
		if(sz <= 0)
			return;
		if(pos >= sz)
		{
		    Z_scripterrlog("Pos (%d) passed to %s is outside the bounds of array %d. Aborting.\n", pos, "ReadChars()", arrayptr);
		    return;
		}
		if(count < 0 || unsigned(count) > sz-pos) count = sz-pos;
		int32_t limit = pos+count;
		char c;
		word q;
		ri->d[rEXP1] = 0;
		for(q = pos; q < limit; ++q)
		{
			c = fgetc(f->file);
			if(feof(f->file) || ferror(f->file))
				break;
			if(c <= 0)
				break;
			am.set(q,c * 10000L);
			++ri->d[rEXP1]; //Don't count nullchar towards length
		}
		if(q >= limit)
		{
			--q;
			--ri->d[rEXP1];
			ungetc(am.get(q), f->file); //Put the character back before overwriting it
		}
		am.set(q,0); //Force null-termination
		ri->d[rEXP1] *= 10000L;
		check_file_error(ri->fileref);
		return;
	}
	ri->d[rEXP1] = 0L;
}
void FFScript::do_file_readbytes()
{
	if(user_file* f = checkFile(ri->fileref, true))
	{
		uint32_t pos = zc_max(ri->d[rINDEX] / 10000,0);
		int32_t count = get_register(sarg2) / 10000;
		if(count == 0) return;
		int32_t arrayptr = get_register(sarg1);
		ArrayManager am(arrayptr);
		int32_t sz = am.size();
		if(sz <= 0)
			return;
		if(pos >= sz)
		{
		    Z_scripterrlog("Pos (%d) passed to %s is outside the bounds of array %d. Aborting.\n", pos, "ReadBytes()", arrayptr);
		    return;
		}
		if(count < 0 || unsigned(count) > sz-pos) count = sz-pos;
		std::vector<uint8_t> data(count);
		ri->d[rEXP1] = 10000L * fread((void*)&(data[0]), 1, count, f->file);
		for(int32_t q = 0; q < count; ++q)
		{
			am.set(q+pos, 10000L * data[q]);
		}
		check_file_error(ri->fileref);
		return;
	}
	ri->d[rEXP1] = 0L;
}
void FFScript::do_file_readstring()
{
	if(user_file* f = checkFile(ri->fileref, true))
	{
		int32_t arrayptr = get_register(sarg1);
		ArrayManager am(arrayptr);
		int32_t sz = am.size();
		if(sz <= 0)
			return;
		int32_t limit = sz;
		int32_t c;
		word q;
		ri->d[rEXP1] = 0;
		for(q = 0; q < limit; ++q)
		{
			c = fgetc(f->file);
			if(feof(f->file) || ferror(f->file))
				break;
			if(c <= 0)
				break;
			am.set(q,c * 10000L);
			++ri->d[rEXP1]; //Don't count nullchar towards length
			if(c == '\n')
			{
				++q;
				break;
			}
		}
		if(q >= limit)
		{
			--q;
			--ri->d[rEXP1];
			ungetc(am.get(q), f->file); //Put the character back before overwriting it
		}
		am.set(q,0); //Force null-termination
		ri->d[rEXP1] *= 10000L;
		check_file_error(ri->fileref);
		return;
	}
	ri->d[rEXP1] = 0L;
}
void FFScript::do_file_readints()
{
	if(user_file* f = checkFile(ri->fileref, true))
	{
		uint32_t pos = zc_max(ri->d[rINDEX] / 10000,0);
		int32_t count = get_register(sarg2) / 10000;
		if(count == 0) return;
		int32_t arrayptr = get_register(sarg1);
		ArrayManager am(arrayptr);
		int32_t sz = am.size();
		if(sz <= 0)
			return;
		if(pos >= sz) 
		{
		    Z_scripterrlog("Pos (%d) passed to %s is outside the bounds of array %d. Aborting.\n", pos, "ReadInts()", arrayptr);
		    return;
		}
		if(count < 0 || unsigned(count) > sz-pos) count = sz-pos;
		
		std::vector<int32_t> data(count);
		ri->d[rEXP1] = 10000L * fread((void*)&(data[0]), 4, count, f->file);
		for(int32_t q = 0; q < count; ++q)
		{
			am.set(q+pos,data[q]);
		}
		check_file_error(ri->fileref);
		return;
	}
	ri->d[rEXP1] = 0L;
}
void FFScript::do_file_writechars()
{
	if(user_file* f = checkFile(ri->fileref, true))
	{
		int32_t pos = zc_max(ri->d[rINDEX] / 10000,0);
		int32_t count = get_register(sarg2) / 10000;
		if(count == 0) return;
		if(count == -1 || count > (MAX_ZC_ARRAY_SIZE-pos)) count = MAX_ZC_ARRAY_SIZE-pos;
		int32_t arrayptr = get_register(sarg1);
		string output;
		ArrayH::getString(arrayptr, output, count, pos);
		uint32_t q = 0;
		for(; q < output.length(); ++q)
		{
			if(fputc(output[q], f->file)<0)
				break;
		}
		ri->d[rEXP1] = q * 10000L;
		check_file_error(ri->fileref);
		return;
	}
	ri->d[rEXP1] = 0L;
}

void FFScript::do_file_writebytes()
{
	if(user_file* f = checkFile(ri->fileref, true))
	{
		uint32_t pos = zc_max(ri->d[rINDEX] / 10000,0);
		int32_t arg = get_register(sarg2) / 10000;
		if(arg == 0) return;
		uint32_t count = ((arg<0 || unsigned(arg) >(MAX_ZC_ARRAY_SIZE - pos)) ? MAX_ZC_ARRAY_SIZE - pos : unsigned(arg));
		int32_t arrayptr = get_register(sarg1);
		string output;
		ArrayManager am(arrayptr);
		if(am.invalid()) return;
		int32_t sz = am.size();
		if(sz <= 0)
			return;
		if(pos >= sz)
		{
		    Z_scripterrlog("Pos (%d) passed to %s is outside the bounds of array %d. Aborting.\n", pos, "WriteBytes()", arrayptr);
		    return;
		}
		if (count > sz-pos) count = sz-pos;
		std::vector<uint8_t> data(count);
		for(uint32_t q = 0; q < count; ++q)
		{
			data[q] = am.get(q+pos) / 10000;
		}
		ri->d[rEXP1] = 10000L * fwrite((const void*)&(data[0]), 1, count, f->file);
		check_file_error(ri->fileref);
		return;
	}
	ri->d[rEXP1] = 0L;
}
void FFScript::do_file_writestring()
{
	if(user_file* f = checkFile(ri->fileref, true))
	{
		int32_t arrayptr = get_register(sarg1);
		string output;
		ArrayH::getString(arrayptr, output, ZSCRIPT_MAX_STRING_CHARS);
		uint32_t q = 0;
		for(; q < output.length(); ++q)
		{
			if(fputc(output[q], f->file)<0)
				break;
		}
		ri->d[rEXP1] = q * 10000L;
		check_file_error(ri->fileref);
		return;
	}
	ri->d[rEXP1] = 0L;
}
void FFScript::do_file_writeints()
{
	if(user_file* f = checkFile(ri->fileref, true))
	{
		uint32_t pos = zc_max(ri->d[rINDEX] / 10000,0);
		int32_t count = get_register(sarg2) / 10000;
		if(count == 0) return;
		int32_t arrayptr = get_register(sarg1);
		ArrayManager am(arrayptr);
		if(am.invalid()) return;
		int32_t sz = am.size();
		if(sz <= 0)
			return;
		if(pos >= sz) 
		{
		    Z_scripterrlog("Pos (%d) passed to %s is outside the bounds of array %d. Aborting.\n", pos, "WriteInts()", arrayptr);
		    return;
		}
		
		if(count < 0 || unsigned(count) > sz-pos) count = sz-pos;
		std::vector<int32_t> data(count);
		for(int32_t q = 0; q < count; ++q)
		{
			data[q] = am.get(q+pos);
		}
		ri->d[rEXP1] = 10000L * fwrite((const void*)&(data[0]), 4, count, f->file);
		check_file_error(ri->fileref);
		return;
	}
	ri->d[rEXP1] = 0L;
}

void FFScript::do_file_getchar()
{
	if(user_file* f = checkFile(ri->fileref, true))
	{
		ri->d[rEXP1] = fgetc(f->file) * 10000L;
		check_file_error(ri->fileref);
		return;
	}
	ri->d[rEXP1] = -10000L; //-1 == EOF; error value
}
void FFScript::do_file_putchar()
{
	if(user_file* f = checkFile(ri->fileref, true))
	{
		int32_t c = get_register(sarg1) / 10000;
		if(char(c) != c)
		{
			Z_scripterrlog("Invalid character val %d passed to PutChar(); value will overflow.", c);
			c = char(c);
		}
		ri->d[rEXP1] = fputc(c, f->file) * 10000L;
		check_file_error(ri->fileref);
		return;
	}
	ri->d[rEXP1] = -10000L; //-1 == EOF; error value
}
void FFScript::do_file_ungetchar()
{
	if(user_file* f = checkFile(ri->fileref, true))
	{
		int32_t c = get_register(sarg1) / 10000;
		if(char(c) != c)
		{
			Z_scripterrlog("Invalid character val %d passed to UngetChar(); value will overflow.", c);
			c = char(c);
		}
		ri->d[rEXP1] = ungetc(c,f->file) * 10000L;
		check_file_error(ri->fileref);
		return;
	}
	ri->d[rEXP1] = -10000L; //-1 == EOF; error value
}

void FFScript::do_file_seek()
{
	if(user_file* f = checkFile(ri->fileref, true))
	{
		int32_t pos = get_register(sarg1); //NOT /10000 -V
		int32_t origin = get_register(sarg2) ? SEEK_CUR : SEEK_SET;
		ri->d[rEXP1] = fseek(f->file, pos, origin) ? 0L : 10000L;
		check_file_error(ri->fileref);
		return;
	}
	ri->d[rEXP1] = 0;
}
void FFScript::do_file_rewind()
{
	if(user_file* f = checkFile(ri->fileref, true))
	{
		//fseek(f->file, 0L, SEEK_END);
		rewind(f->file);
		check_file_error(ri->fileref);
	}
}
void FFScript::do_file_clearerr()
{
	if(user_file* f = checkFile(ri->fileref, true))
	{
		clearerr(f->file);
	}
}

void FFScript::do_file_geterr()
{
	if(user_file* f = checkFile(ri->fileref, true))
	{
		int32_t err = ferror(f->file);
		int32_t arrayptr = get_register(sarg1);
		if(err)
		{
			string error = strerror(err);
			ArrayH::setArray(arrayptr, error);
		}
		else
		{
			ArrayH::setArray(arrayptr, "\0");
		}
	}
}
///----------------------------------------------------------------------------------------------------
//Directory

void FFScript::do_directory_get()
{
	if(user_dir* dr = checkDir(ri->directoryref, true))
	{
		int32_t indx = get_register(sarg1) / 10000L;
		int32_t arrayptr = get_register(sarg2);
		char buf[2048] = {0};
		set_register(sarg1, dr->get(indx, buf) ? 10000L : 0L);
		if(ArrayH::setArray(arrayptr, string(buf)) == SH::_Overflow)
			scripting_log_error_with_context("Array is not large enough");
	}
	else set_register(sarg1, 0L);
}

void FFScript::do_directory_reload()
{
	if(user_dir* dr = checkDir(ri->directoryref, true))
	{
		dr->refresh();
	}
}

void FFScript::do_directory_free()
{
	if(user_dir* dr = checkDir(ri->directoryref, true))
	{
		free_script_object(dr->id);
	}
}

///----------------------------------------------------------------------------------------------------

void FFScript::set_sarg1(int32_t v)
{
	set_register(sarg1, v);
}

void FFScript::do_isvalidbitmap()
{
	int32_t id = get_register(sarg1);

	if (id >= 0)
	{
		auto bmp = user_bitmaps.check(id, true);
		if (bmp && bmp->u_bmp)
		{
			set_register(sarg1, 10000);
			return;
		}
	}

	set_register(sarg1, 0);
}
void FFScript::do_isallocatedbitmap()
{
	int32_t id = get_register(sarg1);

	if (id >= 0)
	{
		auto bmp = user_bitmaps.check(id, true);
		if (bmp)
		{
			set_register(sarg1, 10000);
			return;
		}
	}

	set_register(sarg1, 0);
}

void FFScript::user_bitmaps_init()
{
	user_bitmaps.clear();
}

int32_t FFScript::do_create_bitmap()
{
	int32_t w = (ri->d[rINDEX2] / 10000);
	int32_t h = (ri->d[rINDEX]/10000);
	if ( get_qr(qr_OLDCREATEBITMAP_ARGS) )
	{
		std::swap(w, h);
	}
	
	return create_user_bitmap_ex(h,w);
}

uint32_t FFScript::create_user_bitmap_ex(int32_t w, int32_t h)
{
	auto bmp = user_bitmaps.create();
	if (!bmp)
		return 0;

	bmp->width = w;
	bmp->height = h;
	bmp->u_bmp = create_bitmap_ex(8,w,h);
	clear_bitmap(bmp->u_bmp);
	return bmp->id;
}

bool FFScript::doesResolveToScreenBitmap(int32_t bitmap_id)
{
	if (bitmap_id == rtSCREEN)
		return true;

	if (bitmap_id == -2)
	{
		int curr_rt = zscriptDrawingRenderTarget->GetCurrentRenderTarget();
		if (curr_rt >= 0 && curr_rt < 7) 
			return false;

		return true;
	}

	return false;
}

bool FFScript::doesResolveToDeprecatedSystemBitmap(int32_t bitmap_id)
{
	switch (bitmap_id)
	{
		case rtBMP0:
		case rtBMP1:
		case rtBMP2:
		case rtBMP3:
		case rtBMP4:
		case rtBMP5:
		case rtBMP6:
		{
			return true;
		}
	}

	if (bitmap_id == -2)
	{
		int curr_rt = zscriptDrawingRenderTarget->GetCurrentRenderTarget();
		if (curr_rt >= 0 && curr_rt < 7) 
			return true;
	}

	return false;
}

BITMAP* FFScript::GetScriptBitmap(int32_t id, BITMAP* screen_bmp, bool skipError)
{
	switch (id - 10)
	{
		case rtSCREEN:
			return screen_bmp;

		case rtBMP0:
		case rtBMP1:
		case rtBMP2:
		case rtBMP3:
		case rtBMP4:
		case rtBMP5:
		case rtBMP6: //old system bitmaps (render targets)
		{
			return zscriptDrawingRenderTarget->GetBitmapPtr(id - 10);
		}
	}

	if (auto bitmap = checkBitmap(id, true, skipError))
		return bitmap->u_bmp;

	return nullptr;
}

uint32_t FFScript::get_free_bitmap(bool skipError)
{
	auto bmp = user_bitmaps.create(skipError);
	if (!bmp)
		return 0;
	return bmp->id;
}

void FFScript::do_deallocate_bitmap()
{
	if (ZScriptVersion::gc())
		return;

	if(isSystemBitref(ri->bitmapref))
	{
		return; //Don't attempt to deallocate system bitmaps!
	}

	// Bitmaps are not deallocated right away, but deferred until the next call to scb.update()
	if (auto b = checkBitmap(ri->bitmapref, false, true))
		b->free_obj();
}

bool FFScript::isSystemBitref(int32_t ref)
{
	switch(ref-10)
	{
		case rtSCREEN:
		case rtBMP0:
		case rtBMP1:
		case rtBMP2:
		case rtBMP3:
		case rtBMP4:
		case rtBMP5:
		case rtBMP6:
			return true;
	}
	return false;
}

///----------------------------------------------------------------------------------------------------

int32_t FFScript::GetQuestVersion()
{
	return QHeader.zelda_version;
}
int32_t FFScript::GetQuestBuild()
{
	return QHeader.build;
}
int32_t FFScript::GetQuestSectionVersion(int32_t section)
{
	return QHeader.zelda_version;
}

int32_t FFScript::GetDefaultWeaponSprite(int32_t wpn_id)
{
	switch (wpn_id)
	{
		case wNone:
			return 0; 
		
		case wSword: return 0;
		case wBeam: return 1;
		case wBrang: return 4;
		case wBomb: return 9;
		case wSBomb: return 75;
		case wLitBomb: return 7;
		case wLitSBomb: return 8;
		case wArrow: return 10;
		case wRefArrow: return 10;
		case wFire: return 12;
		case wRefFire: return 12;
		case wRefFire2: return 12;
		case wWhistle: return 45; //blank, unused misc sprite
		case wBait: return 14;
		case wWand: return 15;
		case wMagic: return 16;
		case wCatching: return 45; //blank, unused misc sprite
		case wWind: return 13;
		case wRefMagic: return 16;
		case wRefFireball: return 17;
		case wRefRock: return 18;
		case wHammer: return 25;
		case wHookshot: return 26;
		case wHSHandle: return 28;
		case wHSChain: return 27;
		case wSSparkle: return 29;
		case wFSparkle: return 32;
		case wSmack: return 33;
		case wPhantom: return -1;
		case wCByrna: return 87;
		case wRefBeam: return 1;
		case wStomp: return 45; //blank, unused misc sprite
		case lwMax: return 45; //blank, unused misc sprite
		case wScript1: { if ( get_qr(qr_WRITING_NPC_WEAPON_UNIQUE_SPRITES ) ) return 246; else return 0; }
		case wScript2: { if ( get_qr(qr_WRITING_NPC_WEAPON_UNIQUE_SPRITES ) ) return 247; else return 0; }
		case wScript3: { if ( get_qr(qr_WRITING_NPC_WEAPON_UNIQUE_SPRITES ) ) return 248; else return 0; }
		case wScript4: { if ( get_qr(qr_WRITING_NPC_WEAPON_UNIQUE_SPRITES ) ) return 249; else return 0; }
		case wScript5: { if ( get_qr(qr_WRITING_NPC_WEAPON_UNIQUE_SPRITES ) ) return 250; else return 0; }
		case wScript6: { if ( get_qr(qr_WRITING_NPC_WEAPON_UNIQUE_SPRITES ) ) return 251; else return 0; }
		case wScript7: { if ( get_qr(qr_WRITING_NPC_WEAPON_UNIQUE_SPRITES ) ) return 252; else return 0; }
		case wScript8: { if ( get_qr(qr_WRITING_NPC_WEAPON_UNIQUE_SPRITES ) ) return 253; else return 0; }
		case wScript9: { if ( get_qr(qr_WRITING_NPC_WEAPON_UNIQUE_SPRITES ) ) return 254; else return 0; }
		case wScript10: { if ( get_qr(qr_WRITING_NPC_WEAPON_UNIQUE_SPRITES ) ) return 255; else return 0; }

		case wIce: return 83;
			//Cannot use any of these weapons yet. 
			//return -1;
		
		case wEnemyWeapons:
		case ewFireball: return 17;
		
		case ewArrow: return 19; 
		case ewBrang: return 4; 
		case ewSword: return 20; 
		case ewRock: return 18; 
		case ewMagic: return 21; 
		case ewBomb: return 78; 
		case ewSBomb: return 79; 
		case ewLitBomb: return 76; 
		case ewLitSBomb: return 77; 
		case ewFireTrail: return 80; 
		case ewFlame: return 35; 
		case ewWind: return 36; 
		case ewFlame2: return 81; 
		case ewFlame2Trail: return 82; 
		case ewIce: return 83; 
		case ewFireball2: return 17;  //fireball (rising)
		
			
		default:  return -1; //No assign.
		
	}
}

int32_t FFScript::GetDefaultWeaponSFX(int32_t wpn_id)
{
	switch (wpn_id)
	{
		case ewFireTrail:
		case ewFlame:
		case ewFlame2Trail:
		case ewFlame2:
			return WAV_FIRE; break;
		case ewWind:
		case ewMagic:
			return WAV_WAND; break;
		case ewIce:
			return WAV_ZN1ICE; break;
		case ewRock:
			return WAV_ZN1ROCK; break;
		case ewFireball2:
		case ewFireball:
			return WAV_ZN1FIREBALL; break;
	}
	return -1; //no assign
}

//bitmap->GetPixel()


int32_t FFScript::do_getpixel()
{
	int32_t xoffset = 0, yoffset = 0;
	int32_t xoff = 0; int32_t yoff = 0;
	const bool brokenOffset= ( (get_er(er_BITMAPOFFSET)!=0)
		|| (get_qr(qr_BITMAPOFFSETFIX)!=0) );
	
	BITMAP *bitty = FFCore.GetScriptBitmap(ri->bitmapref, screen);
	if(!bitty)
	{
		bitty = scrollbuf;
	}
	// draw to screen with subscreen offset
	if(!brokenOffset && ri->bitmapref == rtSCREEN + 10 )
	{
		xoffset = xoff;
		yoffset = 56; //should this be -56?
	}
	else
	{
		xoffset = 0;
		yoffset = 0;
	}
	
	int32_t yv = ri->d[rINDEX2]/10000 + yoffset;
	int32_t ret =  getpixel(bitty, ri->d[rINDEX]/10000, yv); //This is a palette index value. 
	if(!get_qr(qr_BROKEN_GETPIXEL_VALUE))
		ret *= 10000;
	return ret;
}

void FFScript::do_bmpcollision()
{
	int32_t bmpref = SH::read_stack(ri->sp + 5);
	int32_t maskbmpref = SH::read_stack(ri->sp + 4);
	int32_t x = SH::read_stack(ri->sp + 3) / 10000;
	int32_t y = SH::read_stack(ri->sp + 2) / 10000;
	int32_t checkCol = SH::read_stack(ri->sp + 1) / 10000;
	int32_t maskCol = SH::read_stack(ri->sp + 0) / 10000;
	BITMAP *checkbit = FFCore.GetScriptBitmap(bmpref, screen, true);
	BITMAP *maskbit = FFCore.GetScriptBitmap(maskbmpref, screen, true);
	if(!(checkbit && maskbit))
	{
		set_register(sarg1, -10000);
		char buf1[16];
		char buf2[16];
		zc_itoa(bmpref, buf1);
		zc_itoa(maskbmpref, buf2);
		Z_scripterrlog("Invalid bitmap%s passed to 'bitmap->CountColor()': %s%s%s\n",
			(checkbit || maskbit) ? "" : "s", checkbit ? "" : buf1,
			(checkbit || maskbit) ? "" : ", ", maskbit ? "" : buf2);
		return;
	}
	int32_t ret = countColor(checkbit, maskbit, x, y, checkCol, maskCol);
	set_register(sarg1, ret*10000);
}


int32_t FFScript::loadMapData()
{
	int32_t map = (ri->d[rINDEX] / 10000);
	int32_t screen = (ri->d[rINDEX2]/10000);
	int32_t indx = (zc_max((map)-1,0) * MAPSCRS + screen);
	 if ( map < 1 || map > map_count )
	{
		Z_scripterrlog("Invalid Map ID passed to Game->LoadMapData: %d\n", map);
		ri->mapsref = MAX_SIGNED_32;
	}
	else if ( screen < 0 || screen > 129 ) //0x00 to 0x81 -Z
	{
		Z_scripterrlog("Invalid Screen Index passed to Game->LoadMapData: %d\n", screen);
		ri->mapsref = MAX_SIGNED_32;
	}
	else ri->mapsref = indx;
	return ri->mapsref;
}

// Called when leaving a screen; deallocate arrays created by FFCs that aren't carried over
void FFScript::deallocateArray(int32_t ptrval)
{
	CHECK(!ZScriptVersion::gc_arrays());

	if(ptrval == 0)
		return;
	if(ptrval==0 || ptrval >= NUM_ZSCRIPT_ARRAYS)
		scripting_log_error_with_context("Script tried to deallocate memory at invalid address {}", ptrval);
	else if(ptrval<0)
		scripting_log_error_with_context("Script tried to deallocate memory at object-based address {}", ptrval);
	else
	{
		if(arrayOwner[ptrval].specOwned) return; //ignore this deallocation
		if(arrayOwner[ptrval].specCleared) return;
		arrayOwner[ptrval].clear();
		
		if(!localRAM[ptrval].Valid())
			scripting_log_error_with_context("Script tried to deallocate memory that was not allocated at address {}", ptrval);
		else
		{
			if (localRAM[ptrval].HoldsObjects())
			{
				auto&& aptr = localRAM[ptrval];
				for (int i = 0; i < aptr.Size(); i++)
				{
					int id = aptr[i];
					script_object_ref_dec(id);
				}
			}
			localRAM[ptrval].Clear();
		}
	}
}

int32_t FFScript::get_screen_d(int32_t index1, int32_t index2)
{
	if(index2 < 0 || index2 > 7)
	{
		scripting_log_error_with_context("You were trying to reference an out-of-bounds array index for a screen's D[] array ({}); valid indices are from 0 to 7.", index1);
		return 0;
	}
	if (index1 < 0 || index1 >= game->screen_d.size())
	{
		scripting_log_error_with_context("You were trying to reference an out-of-bounds screen for a D[] array ({}); valid indices are from 0 to %u.", index1, game->screen_d.size() - 1);
		return 0;
	}
	
	return game->screen_d[index1][index2];
}

void FFScript::set_screen_d(int32_t index1, int32_t index2, int32_t val)
{
	if(index2 < 0 || index2 > 7)
	{
		scripting_log_error_with_context("You were trying to reference an out-of-bounds array index for a screen's D[] array ({}); valid indices are from 0 to 7.", index1);
		return;
	}
	if (index1 < 0 || index1 >= game->screen_d.size())
	{
		scripting_log_error_with_context("You were trying to reference an out-of-bounds screen for a D[] array ({}); valid indices are from 0 to %u.", index1, game->screen_d.size() - 1);
		return;
	}
	
	game->screen_d[index1][index2] = val;
}

void FFScript::do_zapout()
{
	zapout();
}

void FFScript::do_zapin(){ zapin(); }

void FFScript::do_openscreen() { openscreen(); }
void FFScript::do_closescreen() { closescreen(); }
void FFScript::do_openscreenshape()
{
	int32_t shape = get_register(sarg1) / 10000;
	if(shape < 0 || shape >= bosMAX)
	{
		Z_scripterrlog("Invalid shape passed to %s! Valid range %d to %d. Using 'Circle' shape.\n", "Screen->OpeningWipe(int32_t)", 0, bosMAX-1);
		shape = bosCIRCLE;
	}
	openscreen(shape);
}
void FFScript::do_closescreenshape()
{
	int32_t shape = get_register(sarg1) / 10000;
	if(shape < 0 || shape >= bosMAX)
	{
		Z_scripterrlog("Invalid shape passed to %s! Valid range %d to %d. Using 'Circle' shape.\n", "Screen->ClosingWipe(int32_t)", 0, bosMAX-1);
		shape = bosCIRCLE;
	}
	closescreen(shape);
}
void FFScript::do_wavyin() { wavyin(); }
void FFScript::do_wavyout() { wavyout(false); }


void FFScript::do_triggersecret(const bool v)
{
	int32_t ID = vbound((SH::get_arg(sarg1, v) / 10000), 0, 255);
	mapscr *s = hero_scr;
	//Convert a flag type to a secret type.
	int32_t ft = combo_trigger_flag_to_secret_combo_index(ID);
	if (ft != -1)
	{		
		for(int32_t iter=0; iter<2; ++iter)
		{
			for ( int32_t q = 0; q < 176; q++ ) 
			{		
				//Placed flags
				if ( iter == 1 )
				{
					if ( s->sflag[q] == ID ) {
						auto rpos_handle = get_rpos_handle_for_screen(s->screen, 0, q);
						screen_combo_modify_preroutine(rpos_handle);
						s->data[q] = s->secretcombo[ft];
						s->cset[q] = s->secretcset[ft];
						s->sflag[q] = s->secretflag[ft];
						screen_combo_modify_postroutine(rpos_handle);
					}
				}
				//Inherent flags
				else
				{
					if ( combobuf[s->data[q]].flag == ID ) {
						auto rpos_handle = get_rpos_handle_for_screen(s->screen, 0, q);
						screen_combo_modify_preroutine(rpos_handle);
						s->data[q] = s->secretcombo[ft];
						s->cset[q] = s->secretcset[ft];
						screen_combo_modify_postroutine(rpos_handle);
					}
					
				}
			}
		}
	}
	
}
//NPCData

//NPCData Getter Macros


	

//NPCData-> Function
#define GET_NPCDATA_FUNCTION_VAR_INT(member) \
{ \
	int32_t ID = get_register(sarg2) / 10000; \
	if(ID < 1 || ID > (MAXGUYS-1)) \
		set_register(sarg1, -10000); \
	else \
		set_register(sarg1, guysbuf[ID].member * 10000); \
}

#define GET_NPCDATA_FUNCTION_VAR_INDEX(member, indexbound) \
{ \
	int32_t ID = int32_t(ri->d[rINDEX] / 10000);\
	int32_t indx = vbound((ri->d[rINDEX2] / 10000), 0, indexbound); \
	if(ID < 1 || ID > (MAXGUYS-1)) \
		set_register(sarg1, -10000); \
	else \
		set_register(sarg1, guysbuf[ID].member[indx] * 10000); \
}

#define GET_NPCDATA_FUNCTION_VAR_FLAG(member) \
{ \
	int32_t ID = int32_t(ri->d[rINDEX] / 10000);\
	int32_t flag = int32_t(ri->d[rINDEX2] / 10000);\
	if(ID < 1 || ID > (MAXGUYS-1)) \
		set_register(sarg1, -10000); \
	else \
		set_register(sarg1, (guysbuf[ID].member&flag) ? 10000 : 0); \
}

uint32_t get_upper_half_uint64(uint64_t value)
{
	return value >> 32;
}

uint32_t get_lower_half_uint64(uint64_t value)
{
	return value & 0xFFFFFFFF;
}

// Defunct.
void FFScript::getNPCData_flags(){
	int32_t ID = get_register(sarg2) / 10000;
	if(ID < 1 || ID > (MAXGUYS-1))
		set_register(sarg1, -10000);
	else
		set_register(sarg1, get_upper_half_uint64(guysbuf[ID].flags) * 10000);
}

// Defunct.
void FFScript::getNPCData_flags2(){ 
	int32_t ID = get_register(sarg2) / 10000;
	if(ID < 1 || ID > (MAXGUYS-1))
		set_register(sarg1, -10000);
	else
		set_register(sarg1, get_lower_half_uint64(guysbuf[ID].flags) * 10000);
}

void FFScript::getNPCData_tile() { GET_NPCDATA_FUNCTION_VAR_INT(tile); }
void FFScript::getNPCData_width(){ GET_NPCDATA_FUNCTION_VAR_INT(width); } 
void FFScript::getNPCData_height(){ GET_NPCDATA_FUNCTION_VAR_INT(height); } 
void FFScript::getNPCData_s_tile(){ GET_NPCDATA_FUNCTION_VAR_INT(s_tile); } 
void FFScript::getNPCData_s_width(){ GET_NPCDATA_FUNCTION_VAR_INT(s_width); } 
void FFScript::getNPCData_s_height(){ GET_NPCDATA_FUNCTION_VAR_INT(s_height); } 
void FFScript::getNPCData_e_tile(){ GET_NPCDATA_FUNCTION_VAR_INT(e_tile); } 
void FFScript::getNPCData_e_width(){ GET_NPCDATA_FUNCTION_VAR_INT(e_width); } 
void FFScript::getNPCData_e_height() { GET_NPCDATA_FUNCTION_VAR_INT(e_height); }
void FFScript::getNPCData_hp(){ GET_NPCDATA_FUNCTION_VAR_INT(hp); } 
void FFScript::getNPCData_family(){ GET_NPCDATA_FUNCTION_VAR_INT(family); } 
void FFScript::getNPCData_cset(){ GET_NPCDATA_FUNCTION_VAR_INT(cset); } 
void FFScript::getNPCData_anim(){ GET_NPCDATA_FUNCTION_VAR_INT(anim); } 
void FFScript::getNPCData_e_anim(){ GET_NPCDATA_FUNCTION_VAR_INT(e_anim); } 
void FFScript::getNPCData_frate(){ GET_NPCDATA_FUNCTION_VAR_INT(frate); } 
void FFScript::getNPCData_e_frate(){ GET_NPCDATA_FUNCTION_VAR_INT(e_frate); } 
void FFScript::getNPCData_dp(){ GET_NPCDATA_FUNCTION_VAR_INT(dp); } 
void FFScript::getNPCData_wdp(){ GET_NPCDATA_FUNCTION_VAR_INT(wdp); } 
void FFScript::getNPCData_weapon(){ GET_NPCDATA_FUNCTION_VAR_INT(weapon); } 
void FFScript::getNPCData_rate(){ GET_NPCDATA_FUNCTION_VAR_INT(rate); } 
void FFScript::getNPCData_hrate(){ GET_NPCDATA_FUNCTION_VAR_INT(hrate); } 
void FFScript::getNPCData_step(){ GET_NPCDATA_FUNCTION_VAR_INT(step); } 
void FFScript::getNPCData_homing(){ GET_NPCDATA_FUNCTION_VAR_INT(homing); } 
void FFScript::getNPCData_grumble(){ GET_NPCDATA_FUNCTION_VAR_INT(grumble); } 
void FFScript::getNPCData_item_set(){ GET_NPCDATA_FUNCTION_VAR_INT(item_set); } 
void FFScript::getNPCData_bgsfx(){ GET_NPCDATA_FUNCTION_VAR_INT(bgsfx); } 
void FFScript::getNPCData_hitsfx(){ GET_NPCDATA_FUNCTION_VAR_INT(hitsfx); } 
void FFScript::getNPCData_deadsfx(){ GET_NPCDATA_FUNCTION_VAR_INT(deadsfx); } 
void FFScript::getNPCData_xofs(){ GET_NPCDATA_FUNCTION_VAR_INT(xofs); } 
void FFScript::getNPCData_yofs(){ GET_NPCDATA_FUNCTION_VAR_INT(yofs); } 
void FFScript::getNPCData_zofs(){ GET_NPCDATA_FUNCTION_VAR_INT(zofs); } 
void FFScript::getNPCData_hxofs(){ GET_NPCDATA_FUNCTION_VAR_INT(hxofs); } 
void FFScript::getNPCData_hyofs(){ GET_NPCDATA_FUNCTION_VAR_INT(hyofs); } 
void FFScript::getNPCData_hxsz(){ GET_NPCDATA_FUNCTION_VAR_INT(hxsz); } 
void FFScript::getNPCData_hysz(){ GET_NPCDATA_FUNCTION_VAR_INT(hysz); } 
void FFScript::getNPCData_hzsz(){ GET_NPCDATA_FUNCTION_VAR_INT(hzsz); } 
void FFScript::getNPCData_txsz(){ GET_NPCDATA_FUNCTION_VAR_INT(txsz); } 
void FFScript::getNPCData_tysz(){ GET_NPCDATA_FUNCTION_VAR_INT(tysz); } 
void FFScript::getNPCData_wpnsprite(){ GET_NPCDATA_FUNCTION_VAR_INT(wpnsprite); } 
void FFScript::getNPCData_firesfx() { GET_NPCDATA_FUNCTION_VAR_INT(firesfx); }


void FFScript::getNPCData_defense(){GET_NPCDATA_FUNCTION_VAR_INDEX(defense,int32_t(edefLAST255))};


void FFScript::getNPCData_SIZEflags(){GET_NPCDATA_FUNCTION_VAR_FLAG(SIZEflags);}


void FFScript::getNPCData_misc()
{
	int32_t ID = int32_t(ri->d[rINDEX] / 10000); //the enemy ID value
	int32_t indx = int32_t(ri->d[rINDEX2] / 10000); //the misc index ID
	if ((ID < 1 || ID > 511) || ( indx < 0 || indx >= MAX_NPC_ATTRIBUTES ))
		set_register(sarg1, -10000); 
	else set_register(sarg1, guysbuf[ID].attributes[indx] * 10000);
}

//NPCData Setters, two inputs, no return; similar to void GetDMapIntro(int32_t DMap, int32_t buffer[]);

//NPCData Setter Macros

//Variables for spritedata sp->member
	
	

//Functions for NPCData->

#define SET_NPCDATA_FUNCTION_VAR_INT(member, bound) \
{ \
	int32_t ID = get_register(sarg1) / 10000; \
	int32_t val = get_register(sarg2) / 10000; \
	if(ID < 1 || ID > (MAXGUYS-1)) \
		set_register(sarg1, -10000); \
	else \
		guysbuf[ID].member = vbound(val,0,bound); \
}

#define SET_NPCDATA_FUNCTION_VAR_ENUM(member, bound) \
{ \
	int32_t ID = get_register(sarg1) / 10000; \
	int32_t val = get_register(sarg2) / 10000; \
	if(ID < 1 || ID > (MAXGUYS-1)) \
		set_register(sarg1, -10000); \
	else \
		guysbuf[ID].member = (decltype(guysbuf[ID].member))vbound(val,0,bound); \
}


#define SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(member) \
{ \
	int32_t ID = get_register(sarg1) / 10000; \
	int32_t val = get_register(sarg2) / 10000; \
	if(ID < 1 || ID > (MAXGUYS-1)) \
		set_register(sarg1, -10000); \
	else \
		guysbuf[ID].member = val; \
}


//SET_NPC_VAR_INDEX(member,value)
#define SET_NPCDATA_FUNCTION_VAR_INDEX(member, val, bound, indexbound) \
{ \
	int32_t ID = (ri->d[rINDEX]/10000);  \
	int32_t indx =  vbound((ri->d[rINDEX2]/10000),0,indexbound);  \
	if(ID < 1 || ID > (MAXGUYS-1)) \
		return; \
	else \
		guysbuf[ID].member[indx] = vbound(val,0,bound); \
}

//Special case for flags, three inputs one return
#define SET_NPCDATA_FUNCTION_VAR_FLAG(member, val) \
{ \
	int32_t ID = (ri->d[rINDEX]/10000);  \
	int32_t flag =  (ri->d[rINDEX2]/10000);  \
	if(ID < 1 || ID > (MAXGUYS-1)) \
		return; \
	else \
	{ \
		if ( val != 0 ) guysbuf[ID].member|=flag; \
		else guysbuf[ID].member|= ~flag; \
	}\
}

static uint64_t set_upper_half_uint64(uint64_t num, uint32_t half)
{
	uint64_t lower = num & 0x00000000FFFFFFFF;
	return lower | ((uint64_t)half << 32);
}

static uint64_t set_lower_half_uint64(uint64_t num, uint32_t half)
{
	uint64_t upper = num & 0xFFFFFFFF00000000;
	return upper | half;
}

// Defunct.
void FFScript::setNPCData_flags(){
	int32_t ID = get_register(sarg1) / 10000;
	int32_t val = get_register(sarg2) / 10000;
	if(ID < 1 || ID > (MAXGUYS-1))
		set_register(sarg1, -10000);
	else
	{
		guysbuf[ID].flags = (guy_flags)set_upper_half_uint64(guysbuf[ID].flags, vbound(val, 0, 0x7FFFFFFF));
	}
}

// Defunct.
void FFScript::setNPCData_flags2(){
	int32_t ID = get_register(sarg1) / 10000;
	int32_t val = get_register(sarg2) / 10000;
	if(ID < 1 || ID > (MAXGUYS-1))
		set_register(sarg1, -10000);
	else
	{
		guysbuf[ID].flags = (guy_flags)set_lower_half_uint64(guysbuf[ID].flags, vbound(val, 0, 0x7FFFFFFF));
	}
}
void FFScript::setNPCData_tile() { SET_NPCDATA_FUNCTION_VAR_INT(tile, ZS_WORD); }
void FFScript::setNPCData_width(){SET_NPCDATA_FUNCTION_VAR_INT(width,ZS_BYTE);}
void FFScript::setNPCData_height(){SET_NPCDATA_FUNCTION_VAR_INT(height,ZS_BYTE);}
void FFScript::setNPCData_s_tile(){SET_NPCDATA_FUNCTION_VAR_INT(s_tile,ZS_WORD);}
void FFScript::setNPCData_s_width(){SET_NPCDATA_FUNCTION_VAR_INT(s_width,ZS_BYTE);}
void FFScript::setNPCData_s_height(){SET_NPCDATA_FUNCTION_VAR_INT(s_height,ZS_BYTE);}
void FFScript::setNPCData_e_tile(){SET_NPCDATA_FUNCTION_VAR_INT(e_tile,ZS_WORD);}
void FFScript::setNPCData_e_width(){SET_NPCDATA_FUNCTION_VAR_INT(e_width,ZS_BYTE);}
void FFScript::setNPCData_e_height() { SET_NPCDATA_FUNCTION_VAR_INT(e_height, ZS_BYTE); }
void FFScript::setNPCData_hp(){SET_NPCDATA_FUNCTION_VAR_INT(hp,ZS_SHORT);}
void FFScript::setNPCData_family(){SET_NPCDATA_FUNCTION_VAR_INT(family,ZS_SHORT);}
void FFScript::setNPCData_cset(){SET_NPCDATA_FUNCTION_VAR_INT(cset,ZS_SHORT);}
void FFScript::setNPCData_anim(){SET_NPCDATA_FUNCTION_VAR_INT(anim,ZS_SHORT);}
void FFScript::setNPCData_e_anim(){SET_NPCDATA_FUNCTION_VAR_INT(e_anim,ZS_SHORT);}
void FFScript::setNPCData_frate(){SET_NPCDATA_FUNCTION_VAR_INT(frate,ZS_SHORT);}
void FFScript::setNPCData_e_frate(){SET_NPCDATA_FUNCTION_VAR_INT(e_frate,ZS_SHORT);}
void FFScript::setNPCData_dp(){SET_NPCDATA_FUNCTION_VAR_INT(dp,ZS_SHORT);}
void FFScript::setNPCData_wdp(){SET_NPCDATA_FUNCTION_VAR_INT(wdp,ZS_SHORT);}
void FFScript::setNPCData_weapon(){SET_NPCDATA_FUNCTION_VAR_INT(weapon,ZS_SHORT);}
void FFScript::setNPCData_rate(){SET_NPCDATA_FUNCTION_VAR_INT(rate,ZS_SHORT);}
void FFScript::setNPCData_hrate(){SET_NPCDATA_FUNCTION_VAR_INT(hrate,ZS_SHORT);}
void FFScript::setNPCData_step(){SET_NPCDATA_FUNCTION_VAR_INT(step,ZS_SHORT);}
void FFScript::setNPCData_homing(){SET_NPCDATA_FUNCTION_VAR_INT(homing,ZS_SHORT);}
void FFScript::setNPCData_grumble(){SET_NPCDATA_FUNCTION_VAR_INT(grumble,ZS_SHORT);}
void FFScript::setNPCData_item_set(){SET_NPCDATA_FUNCTION_VAR_INT(item_set,ZS_SHORT);}
void FFScript::setNPCData_bgsfx(){SET_NPCDATA_FUNCTION_VAR_INT(bgsfx,ZS_SHORT);}
void FFScript::setNPCData_hitsfx(){SET_NPCDATA_FUNCTION_VAR_INT(hitsfx,ZS_BYTE);}
void FFScript::setNPCData_deadsfx(){SET_NPCDATA_FUNCTION_VAR_INT(deadsfx,ZS_BYTE);}
void FFScript::setNPCData_xofs(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(xofs);}
void FFScript::setNPCData_yofs(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(yofs);}
void FFScript::setNPCData_zofs(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(zofs);}
void FFScript::setNPCData_hxofs(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(hxofs);}
void FFScript::setNPCData_hyofs(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(hyofs);}
void FFScript::setNPCData_hxsz(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(hxsz);}
void FFScript::setNPCData_hysz(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(hysz);}
void FFScript::setNPCData_hzsz(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(hzsz);}
void FFScript::setNPCData_txsz(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(txsz);}
void FFScript::setNPCData_tysz(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(tysz);}
void FFScript::setNPCData_wpnsprite(){SET_NPCDATA_FUNCTION_VAR_INT(wpnsprite,511);}
void FFScript::setNPCData_firesfx() { SET_NPCDATA_FUNCTION_VAR_INT(firesfx, 255); }

//NPCData Setters, three inputs, no return. works as SetDMapScreenD function







void FFScript::setNPCData_defense(int32_t v){SET_NPCDATA_FUNCTION_VAR_INDEX(defense,v, ZS_INT, int32_t(edefLAST255) );}
void FFScript::setNPCData_SIZEflags(int32_t v){SET_NPCDATA_FUNCTION_VAR_FLAG(SIZEflags,v);}
void FFScript::setNPCData_misc(int32_t val)
{
	int32_t ID = int32_t(ri->d[rINDEX] / 10000); //the enemy ID value
	int32_t indx = int32_t(ri->d[rINDEX2] / 10000); //the misc index ID
	if ((ID < 1 || ID > 511) || ( indx < 0 || indx >= MAX_NPC_ATTRIBUTES )) return;
	guysbuf[ID].attributes[indx] = val;
	
};

//ComboData

//Macros

//Are these right? newcombo is *combo_class_buf and the others are *combobuf

//Getters for ComboData 'Type' submembers. 
#define GET_COMBODATA_TYPE_INT(member) \
{ \
	int32_t ID = vbound((get_register(sarg2) / 10000),0,MAXCOMBOS);\
	set_register(sarg1, combo_class_buf[combobuf[ID].type].member * 10000); \
}

//this may need additional macros. 
//for combo_class_buf[ID].member ?
//I'm not sure which it needs to be at present. 

#define GET_COMBODATA_TYPE_INDEX(member, bound) \
{ \
	int32_t ID = int32_t(vbound((ri->d[rINDEX] / 10000),0,MAXCOMBOS));\
	int32_t indx = int32_t(vbound((ri->d[rINDEX2] / 10000), 0, bound));\
	set_register(sarg1, combo_class_buf[combobuf[ID].type].member[indx] * 10000); \
}

#define GET_COMBODATA_TYPE_FLAG(member) \
{ \
	int32_t ID = int32_t(vbound(ri->d[rINDEX] / 10000),0,MAXCOMBOS);\
	int32_t flag = int32_t(ri->d[rINDEX2] / 10000);\
	set_register(sarg1, (combo_class_buf[combobuf[ID].type].member&flag) ? 10000 : 0); \
}



//Getters for ComboData main members. 
#define GET_COMBODATA_VAR_INT(member) \
{ \
	int32_t ID = vbound( (get_register(sarg2) / 10000), 0, MAXCOMBOS);\
	set_register(sarg1, combobuf[ID].member * 10000); \
}

#define GET_COMBODATA_VAR_FLAG(member) \
{ \
	int32_t ID = int32_t( vbound( ( ri->d[rINDEX] / 10000),0,MAXCOMBOS) );\
	int32_t flag = int32_t(ri->d[rINDEX2] / 10000);\
	set_register(sarg1, (combobuf[ID].member&flag) ? 10000 : 0); \
}



//ComboData Setter Macros

//Setters for ComboData 'type' submembers.
#define SET_COMBODATA_TYPE_INT(member, bound) \
{ \
	int32_t ID = get_register(sarg1) / 10000; \
	int32_t val = vbound( (get_register(sarg2) / 10000), 0, bound); \
	if(ID < 1 || ID > 511) \
		set_register(sarg1, -10000); \
	else \
		combo_class_buf[combobuf[ID].type].member = val; \
}

#define SET_COMBODATA_TYPE_INDEX(member, val, bound, indexbound) \
{ \
	int32_t ID = vbound((ri->d[rINDEX]/10000),0,MAXCOMBOS);  \
	int32_t indx =  vbound((ri->d[rINDEX2]/10000),0,indexbound);  \
	combo_class_buf[combobuf[ID].type].member[indx] = vbound(val,0,bound); \
}

#define SET_COMBODATA_TYPE_FLAG(member, val, bound) \
{ \
	int32_t ID = vbound((ri->d[rINDEX]/10000),0,MAXCOMBOS);  \
	int32_t flag =  (ri->d[rINDEX2]/10000);  \
	combo_class_buf[combobuf[ID].type].member&flag = ((vbound(val,0,bound))!=0); \
 \


//Setters for ComboData main members
#define SET_COMBODATA_VAR_INT(member, bound) \
{ \
	int32_t ID = vbound( (get_register(sarg1) / 10000), 0, MAXCOMBOS); \
	int32_t val = vbound((get_register(sarg2) / 10000),0,bound); \
	screen_combo_modify_pre(ID); \
	combobuf[ID].member = val; \
	screen_combo_modify_post(ID); \
}

//Getters

//one input, one return
void FFScript::getComboData_block_enemies(){ GET_COMBODATA_TYPE_INT(block_enemies); } //byte a
void FFScript::getComboData_block_hole(){ GET_COMBODATA_TYPE_INT(block_hole); } //byte b
void FFScript::getComboData_block_trigger(){ GET_COMBODATA_TYPE_INT(block_trigger); } //byte c
void FFScript::getComboData_conveyor_x_speed(){ GET_COMBODATA_TYPE_INT(conveyor_x_speed); } //int16_t e
void FFScript::getComboData_conveyor_y_speed(){ GET_COMBODATA_TYPE_INT(conveyor_y_speed); } //int16_t f
void FFScript::getComboData_create_enemy(){ GET_COMBODATA_TYPE_INT(create_enemy); } //word g
void FFScript::getComboData_create_enemy_when(){ GET_COMBODATA_TYPE_INT(create_enemy_when); } //byte h
void FFScript::getComboData_create_enemy_change(){ GET_COMBODATA_TYPE_INT(create_enemy_change); } //int32_t i
void FFScript::getComboData_directional_change_type(){ GET_COMBODATA_TYPE_INT(directional_change_type); } //byte j
void FFScript::getComboData_distance_change_tiles(){ GET_COMBODATA_TYPE_INT(distance_change_tiles); } //int32_t k
void FFScript::getComboData_dive_item(){ GET_COMBODATA_TYPE_INT(dive_item); } //int16_t l
void FFScript::getComboData_dock(){ GET_COMBODATA_TYPE_INT(dock); } //byte m
void FFScript::getComboData_fairy(){ GET_COMBODATA_TYPE_INT(fairy); } //byte n
void FFScript::getComboData_ff_combo_attr_change(){ GET_COMBODATA_TYPE_INT(ff_combo_attr_change); } //byte o
void FFScript::getComboData_foot_decorations_tile(){ GET_COMBODATA_TYPE_INT(foot_decorations_tile); } //int32_t p
void FFScript::getComboData_foot_decorations_type(){ GET_COMBODATA_TYPE_INT(foot_decorations_type); } //byte q
void FFScript::getComboData_hookshot_grab_point(){ GET_COMBODATA_TYPE_INT(hookshot_grab_point); } //byte r
void FFScript::getComboData_ladder_pass(){ GET_COMBODATA_TYPE_INT(ladder_pass); } //byte s
void FFScript::getComboData_lock_block_type(){ GET_COMBODATA_TYPE_INT(lock_block_type); } //byte t
void FFScript::getComboData_lock_block_change(){ GET_COMBODATA_TYPE_INT(lock_block_change); } //int32_t u
void FFScript::getComboData_magic_mirror_type(){ GET_COMBODATA_TYPE_INT(magic_mirror_type); } //byte v
void FFScript::getComboData_modify_hp_amount(){ GET_COMBODATA_TYPE_INT(modify_hp_amount); } //int16_t w
void FFScript::getComboData_modify_hp_delay(){ GET_COMBODATA_TYPE_INT(modify_hp_delay); } //byte x
void FFScript::getComboData_modify_hp_type(){ GET_COMBODATA_TYPE_INT(modify_hp_type); } //byte y
void FFScript::getComboData_modify_mp_amount(){ GET_COMBODATA_TYPE_INT(modify_mp_amount); } //int16_t z
void FFScript::getComboData_modify_mp_delay(){ GET_COMBODATA_TYPE_INT(modify_mp_delay); } //byte aa
void FFScript::getComboData_modify_mp_type(){ GET_COMBODATA_TYPE_INT(modify_mp_type); } //byte ab
void FFScript::getComboData_no_push_blocks(){ GET_COMBODATA_TYPE_INT(no_push_blocks); } //byte ac
void FFScript::getComboData_overhead(){ GET_COMBODATA_TYPE_INT(overhead); } //byte ad
void FFScript::getComboData_place_enemy(){ GET_COMBODATA_TYPE_INT(place_enemy); } //byte ae
void FFScript::getComboData_push_direction(){ GET_COMBODATA_TYPE_INT(push_direction); } //byte af
void FFScript::getComboData_push_weight(){ GET_COMBODATA_TYPE_INT(push_weight); } //byte ag  heavy or not
void FFScript::getComboData_push_wait(){ GET_COMBODATA_TYPE_INT(push_wait); } //byte ah
void FFScript::getComboData_pushed(){ GET_COMBODATA_TYPE_INT(pushed); } //byte ai
void FFScript::getComboData_raft(){ GET_COMBODATA_TYPE_INT(raft); } //byte aj
void FFScript::getComboData_reset_room(){ GET_COMBODATA_TYPE_INT(reset_room); } //byte ak
void FFScript::getComboData_save_point_type(){ GET_COMBODATA_TYPE_INT(save_point_type); } //byte al
void FFScript::getComboData_screen_freeze_type(){ GET_COMBODATA_TYPE_INT(screen_freeze_type); } //byte am

void FFScript::getComboData_secret_combo(){ GET_COMBODATA_TYPE_INT(secret_combo); } //byte an
void FFScript::getComboData_singular(){ GET_COMBODATA_TYPE_INT(singular); } //byte ao
void FFScript::getComboData_slow_movement(){ GET_COMBODATA_TYPE_INT(slow_movement); } //byte ap
void FFScript::getComboData_statue_type(){ GET_COMBODATA_TYPE_INT(statue_type); } //byte aq
void FFScript::getComboData_step_type(){ GET_COMBODATA_TYPE_INT(step_type); } //byte ar
void FFScript::getComboData_step_change_to(){ GET_COMBODATA_TYPE_INT(step_change_to); } //int32_t as
void FFScript::getComboData_strike_remnants(){ GET_COMBODATA_TYPE_INT(strike_remnants); } //int32_t au
void FFScript::getComboData_strike_remnants_type(){ GET_COMBODATA_TYPE_INT(strike_remnants_type); } //byte av
void FFScript::getComboData_strike_change(){ GET_COMBODATA_TYPE_INT(strike_change); } //int32_t aw
void FFScript::getComboData_strike_item(){ GET_COMBODATA_TYPE_INT(strike_item); } //int16_t ax
void FFScript::getComboData_touch_item(){ GET_COMBODATA_TYPE_INT(touch_item); } //int16_t ay
void FFScript::getComboData_touch_stairs(){ GET_COMBODATA_TYPE_INT(touch_stairs); } //byte az
void FFScript::getComboData_trigger_type(){ GET_COMBODATA_TYPE_INT(trigger_type); } //byte ba
void FFScript::getComboData_trigger_sensitive(){ GET_COMBODATA_TYPE_INT(trigger_sensitive); } //byte bb
void FFScript::getComboData_warp_type(){ GET_COMBODATA_TYPE_INT(warp_type); } //byte bc
void FFScript::getComboData_warp_sensitive(){ GET_COMBODATA_TYPE_INT(warp_sensitive); } //byte bd
void FFScript::getComboData_warp_direct(){ GET_COMBODATA_TYPE_INT(warp_direct); } //byte be
void FFScript::getComboData_warp_location(){ GET_COMBODATA_TYPE_INT(warp_location); } //byte bf
void FFScript::getComboData_water(){ GET_COMBODATA_TYPE_INT(water); } //byte bg
void FFScript::getComboData_whistle(){ GET_COMBODATA_TYPE_INT(whistle); } //byte bh
void FFScript::getComboData_win_game(){ GET_COMBODATA_TYPE_INT(win_game); } //byte bi
void FFScript::getComboData_block_weapon_lvl(){ GET_COMBODATA_TYPE_INT(block_weapon_lvl); } //byte bj - max level of weapon to block

void FFScript::getComboData_tile(){ GET_COMBODATA_VAR_INT(tile); } //newcombo, word
void FFScript::getComboData_flip(){ GET_COMBODATA_VAR_INT(flip); } //newcombo byte

void FFScript::getComboData_walk(){ GET_COMBODATA_VAR_INT(walk); } //newcombo byte
void FFScript::getComboData_type(){ GET_COMBODATA_VAR_INT(type); } //newcombo byte
void FFScript::getComboData_csets(){ GET_COMBODATA_VAR_INT(csets); } //newcombo byte
void FFScript::getComboData_frames(){ GET_COMBODATA_VAR_INT(frames); } //newcombo byte
void FFScript::getComboData_speed(){ GET_COMBODATA_VAR_INT(speed); } //newcombo byte
void FFScript::getComboData_nextcombo(){ GET_COMBODATA_VAR_INT(nextcombo); } //newcombo word
void FFScript::getComboData_nextcset(){ GET_COMBODATA_VAR_INT(nextcset); } //newcombo byte
void FFScript::getComboData_flag(){ GET_COMBODATA_VAR_INT(flag); } //newcombo byte
void FFScript::getComboData_skipanim(){ GET_COMBODATA_VAR_INT(skipanim); } //newcombo byte
void FFScript::getComboData_nexttimer(){ GET_COMBODATA_VAR_INT(nexttimer); } //newcombo word
void FFScript::getComboData_skipanimy(){ GET_COMBODATA_VAR_INT(skipanimy); } //newcombo byte
void FFScript::getComboData_animflags(){ GET_COMBODATA_VAR_INT(animflags); } //newcombo byte


//two inputs, one return
void FFScript::getComboData_block_weapon(){ GET_COMBODATA_TYPE_INDEX(block_weapon,32); } //byte array[32] d (ID of LWeapon)
void FFScript::getComboData_strike_weapons(){ GET_COMBODATA_TYPE_INDEX(strike_weapons,32); } //byte at, arr[32]

//Setters, two inputs no returns

void FFScript::setComboData_block_enemies(){ SET_COMBODATA_TYPE_INT(block_enemies,ZS_BYTE); } //byte a
void FFScript::setComboData_block_hole(){ SET_COMBODATA_TYPE_INT(block_hole,ZS_BYTE); } //byte b
void FFScript::setComboData_block_trigger(){ SET_COMBODATA_TYPE_INT(block_trigger,ZS_BYTE); } //byte c
void FFScript::setComboData_conveyor_x_speed(){ SET_COMBODATA_TYPE_INT(conveyor_x_speed,ZS_SHORT); } //int16_t e
void FFScript::setComboData_conveyor_y_speed(){ SET_COMBODATA_TYPE_INT(conveyor_y_speed,ZS_SHORT); } //int16_t f
void FFScript::setComboData_create_enemy(){ SET_COMBODATA_TYPE_INT(create_enemy,ZS_WORD); } //word g
void FFScript::setComboData_create_enemy_when(){ SET_COMBODATA_TYPE_INT(create_enemy_when,ZS_BYTE); } //byte h
void FFScript::setComboData_create_enemy_change(){ SET_COMBODATA_TYPE_INT(create_enemy_change,ZS_LONG); } //int32_t i
void FFScript::setComboData_directional_change_type(){ SET_COMBODATA_TYPE_INT(directional_change_type,ZS_BYTE); } //byte j
void FFScript::setComboData_distance_change_tiles(){ SET_COMBODATA_TYPE_INT(distance_change_tiles,ZS_LONG); } //int32_t k
void FFScript::setComboData_dive_item(){ SET_COMBODATA_TYPE_INT(dive_item,ZS_SHORT); } //int16_t l
void FFScript::setComboData_dock(){ SET_COMBODATA_TYPE_INT(dock,ZS_BYTE); } //byte m
void FFScript::setComboData_fairy(){ SET_COMBODATA_TYPE_INT(fairy,ZS_BYTE); } //byte n
void FFScript::setComboData_ff_combo_attr_change(){ SET_COMBODATA_TYPE_INT(ff_combo_attr_change,ZS_BYTE); } //byte o
void FFScript::setComboData_foot_decorations_tile(){ SET_COMBODATA_TYPE_INT(foot_decorations_tile,ZS_LONG); } //int32_t p
void FFScript::setComboData_foot_decorations_type(){ SET_COMBODATA_TYPE_INT(foot_decorations_type,ZS_BYTE); } //byte q
void FFScript::setComboData_hookshot_grab_point(){ SET_COMBODATA_TYPE_INT(hookshot_grab_point,ZS_BYTE); } //byte r
void FFScript::setComboData_ladder_pass(){ SET_COMBODATA_TYPE_INT(ladder_pass,ZS_BYTE); } //byte s
void FFScript::setComboData_lock_block_type(){ SET_COMBODATA_TYPE_INT(lock_block_type,ZS_BYTE); } //byte t
void FFScript::setComboData_lock_block_change(){ SET_COMBODATA_TYPE_INT(lock_block_change,ZS_LONG); } //int32_t u
void FFScript::setComboData_magic_mirror_type(){ SET_COMBODATA_TYPE_INT(magic_mirror_type,ZS_BYTE); } //byte v
void FFScript::setComboData_modify_hp_amount(){ SET_COMBODATA_TYPE_INT(modify_hp_amount,ZS_SHORT); } //int16_t w
void FFScript::setComboData_modify_hp_delay(){ SET_COMBODATA_TYPE_INT(modify_hp_delay,ZS_BYTE); } //byte x
void FFScript::setComboData_modify_hp_type(){ SET_COMBODATA_TYPE_INT(modify_hp_type,ZS_BYTE); } //byte y
void FFScript::setComboData_modify_mp_amount(){ SET_COMBODATA_TYPE_INT(modify_mp_amount,ZS_SHORT); } //int16_t z
void FFScript::setComboData_modify_mp_delay(){ SET_COMBODATA_TYPE_INT(modify_mp_delay,ZS_BYTE); } //byte aa
void FFScript::setComboData_modify_mp_type(){ SET_COMBODATA_TYPE_INT(modify_mp_type,ZS_BYTE); } //byte ab
void FFScript::setComboData_no_push_blocks(){ SET_COMBODATA_TYPE_INT(no_push_blocks,ZS_BYTE); } //byte ac
void FFScript::setComboData_overhead(){ SET_COMBODATA_TYPE_INT(overhead,ZS_BYTE); } //byte ad
void FFScript::setComboData_place_enemy(){ SET_COMBODATA_TYPE_INT(place_enemy,ZS_BYTE); } //byte ae
void FFScript::setComboData_push_direction(){ SET_COMBODATA_TYPE_INT(push_direction,ZS_BYTE); } //byte af
void FFScript::setComboData_push_weight(){ SET_COMBODATA_TYPE_INT(push_weight,ZS_BYTE); } //byte ag  heavy or not
void FFScript::setComboData_push_wait(){ SET_COMBODATA_TYPE_INT(push_wait,ZS_BYTE); } //byte ah
void FFScript::setComboData_pushed(){ SET_COMBODATA_TYPE_INT(pushed,ZS_BYTE); } //byte ai
void FFScript::setComboData_raft(){ SET_COMBODATA_TYPE_INT(raft,ZS_BYTE); } //byte aj
void FFScript::setComboData_reset_room(){ SET_COMBODATA_TYPE_INT(reset_room,ZS_BYTE); } //byte ak
void FFScript::setComboData_save_point_type(){ SET_COMBODATA_TYPE_INT(save_point_type,ZS_BYTE); } //byte al
void FFScript::setComboData_screen_freeze_type(){ SET_COMBODATA_TYPE_INT(screen_freeze_type,ZS_BYTE); } //byte am

void FFScript::setComboData_secret_combo(){ SET_COMBODATA_TYPE_INT(secret_combo,ZS_BYTE); } //byte an
void FFScript::setComboData_singular(){ SET_COMBODATA_TYPE_INT(singular,ZS_BYTE); } //byte ao
void FFScript::setComboData_slow_movement(){ SET_COMBODATA_TYPE_INT(slow_movement,ZS_BYTE); } //byte ap
void FFScript::setComboData_statue_type(){ SET_COMBODATA_TYPE_INT(statue_type,ZS_BYTE); } //byte aq
void FFScript::setComboData_step_type(){ SET_COMBODATA_TYPE_INT(step_type,ZS_BYTE); } //byte ar
void FFScript::setComboData_step_change_to(){ SET_COMBODATA_TYPE_INT(step_change_to,ZS_LONG); } //int32_t as

void FFScript::setComboData_strike_remnants(){ SET_COMBODATA_TYPE_INT(strike_remnants,ZS_LONG); } //int32_t au
void FFScript::setComboData_strike_remnants_type(){ SET_COMBODATA_TYPE_INT(strike_remnants_type,ZS_BYTE); } //byte av
void FFScript::setComboData_strike_change(){ SET_COMBODATA_TYPE_INT(strike_change,ZS_LONG); } //int32_t aw
void FFScript::setComboData_strike_item(){ SET_COMBODATA_TYPE_INT(strike_item,ZS_SHORT); } //int16_t ax
void FFScript::setComboData_touch_item(){ SET_COMBODATA_TYPE_INT(touch_item,ZS_SHORT); } //int16_t ay
void FFScript::setComboData_touch_stairs(){ SET_COMBODATA_TYPE_INT(touch_stairs,ZS_BYTE); } //byte az
void FFScript::setComboData_trigger_type(){ SET_COMBODATA_TYPE_INT(trigger_type,ZS_BYTE); } //byte ba
void FFScript::setComboData_trigger_sensitive(){ SET_COMBODATA_TYPE_INT(trigger_sensitive,ZS_BYTE); } //byte bb
void FFScript::setComboData_warp_type(){ SET_COMBODATA_TYPE_INT(warp_type,ZS_BYTE); } //byte bc
void FFScript::setComboData_warp_sensitive(){ SET_COMBODATA_TYPE_INT(warp_sensitive,ZS_BYTE); } //byte bd
void FFScript::setComboData_warp_direct(){ SET_COMBODATA_TYPE_INT(warp_direct,ZS_BYTE); } //byte be
void FFScript::setComboData_warp_location(){ SET_COMBODATA_TYPE_INT(warp_location,ZS_BYTE); } //byte bf
void FFScript::setComboData_water(){ SET_COMBODATA_TYPE_INT(water,ZS_BYTE); } //byte bg
void FFScript::setComboData_whistle(){ SET_COMBODATA_TYPE_INT(whistle,ZS_BYTE); } //byte bh
void FFScript::setComboData_win_game(){ SET_COMBODATA_TYPE_INT(win_game,ZS_BYTE); } //byte bi
void FFScript::setComboData_block_weapon_lvl(){ SET_COMBODATA_TYPE_INT(block_weapon_lvl,ZS_BYTE); } //byte bj - max level of weapon to block

//combobuf
void FFScript::setComboData_tile(){ SET_COMBODATA_VAR_INT(tile,ZS_WORD); } //newcombo, word
void FFScript::setComboData_flip(){ SET_COMBODATA_VAR_INT(flip,ZS_BYTE); } //newcombo byte

void FFScript::setComboData_walk(){ SET_COMBODATA_VAR_INT(walk,ZS_BYTE); } //newcombo byte
void FFScript::setComboData_type(){ SET_COMBODATA_VAR_INT(type,ZS_BYTE); } //newcombo byte
void FFScript::setComboData_csets(){ SET_COMBODATA_VAR_INT(csets,ZS_BYTE); } //newcombo byte
void FFScript::setComboData_frames(){ SET_COMBODATA_VAR_INT(frames,ZS_BYTE); } //newcombo byte
void FFScript::setComboData_speed(){ SET_COMBODATA_VAR_INT(speed,ZS_BYTE); } //newcombo byte
void FFScript::setComboData_nextcombo(){ SET_COMBODATA_VAR_INT(nextcombo,ZS_WORD); } //newcombo word
void FFScript::setComboData_nextcset(){ SET_COMBODATA_VAR_INT(nextcset,ZS_BYTE); } //newcombo byte
void FFScript::setComboData_flag(){ SET_COMBODATA_VAR_INT(flag,ZS_BYTE); } //newcombo byte
void FFScript::setComboData_skipanim(){ SET_COMBODATA_VAR_INT(skipanim,ZS_BYTE); } //newcombo byte
void FFScript::setComboData_nexttimer(){ SET_COMBODATA_VAR_INT(nexttimer,ZS_WORD); } //newcombo word
void FFScript::setComboData_skipanimy(){ SET_COMBODATA_VAR_INT(skipanimy,ZS_BYTE); } //newcombo byte
void FFScript::setComboData_animflags(){ SET_COMBODATA_VAR_INT(animflags,ZS_BYTE); } //newcombo byte

//three inputs, no returns
void FFScript::setComboData_block_weapon(int32_t v){ SET_COMBODATA_TYPE_INDEX(block_weapon,v,ZS_BYTE,32); } //byte array[32] d (ID of LWeapon)
void FFScript::setComboData_strike_weapons(int32_t v){ SET_COMBODATA_TYPE_INDEX(strike_weapons,v,ZS_BYTE,32); } //byte at, arr[32]

//SpriteData Macros
#define GET_SPRITEDATA_TYPE_INT(member) \
{ \
	int32_t ID = vbound((get_register(sarg2) / 10000),0,255);\
	set_register(sarg1, wpnsbuf[ID].member * 10000); \
}

#define SET_SPRITEDATA_TYPE_INT(member, bound) \
{ \
	int32_t ID = get_register(sarg1) / 10000; \
	int32_t val = vbound( (get_register(sarg2) / 10000), 0, bound); \
	if(ID < 1 || ID > 255) \
		set_register(sarg1, -10000); \
	else \
		wpnsbuf[ID].member = val; \
}

#define SET_SPRITEDATA_TYPE_INT_NOBOUND(member) \
{ \
	int32_t ID = get_register(sarg1) / 10000; \
	int32_t val = get_register(sarg2) / 10000; \
	if(ID < 1 || ID > 255) \
		set_register(sarg1, -10000); \
	else \
		wpnsbuf[ID].member = val; \
}


void FFScript::getSpriteDataTile(){GET_SPRITEDATA_TYPE_INT(tile);}
void FFScript::getSpriteDataMisc(){GET_SPRITEDATA_TYPE_INT(misc);}
void FFScript::getSpriteDataCSets(){GET_SPRITEDATA_TYPE_INT(csets);}
void FFScript::getSpriteDataFrames(){GET_SPRITEDATA_TYPE_INT(frames);}
void FFScript::getSpriteDataSpeed(){GET_SPRITEDATA_TYPE_INT(speed);}
void FFScript::getSpriteDataType(){GET_SPRITEDATA_TYPE_INT(type);}



void FFScript::setSpriteDataTile(){SET_SPRITEDATA_TYPE_INT(tile,ZS_WORD);}
void FFScript::setSpriteDataMisc(){SET_SPRITEDATA_TYPE_INT(misc,ZS_CHAR);}
void FFScript::setSpriteDataCSets(){SET_SPRITEDATA_TYPE_INT(csets,ZS_CHAR);}
void FFScript::setSpriteDataFrames(){SET_SPRITEDATA_TYPE_INT(frames,ZS_CHAR);}
void FFScript::setSpriteDataSpeed(){SET_SPRITEDATA_TYPE_INT(speed,ZS_CHAR);}
void FFScript::setSpriteDataType(){SET_SPRITEDATA_TYPE_INT(type,ZS_CHAR);}


void FFScript::do_setMIDI_volume(int32_t m)
{
	master_volume(-1,(vbound(m,0,255)));
}
void FFScript::do_setMusic_volume(int32_t m)
{
	emusic_volume = vbound(m,0,255);
}
void FFScript::do_setDIGI_volume(int32_t m)
{
	master_volume((vbound(m,0,255)),-1);
}
void FFScript::do_setSFX_volume(int32_t m)
{
	sfx_volume = m;
}
void FFScript::do_setSFX_pan(int32_t m)
{
	pan_style = vbound(m,0,3);
}
int32_t FFScript::do_getMIDI_volume()
{
	return ((int32_t)midi_volume);
}
int32_t FFScript::do_getMusic_volume()
{
	return ((int32_t)emusic_volume);
}
int32_t FFScript::do_getDIGI_volume()
{
	return ((int32_t)digi_volume);
}
int32_t FFScript::do_getSFX_volume()
{
	return ((int32_t)sfx_volume);
}
int32_t FFScript::do_getSFX_pan()
{
	return ((int32_t)pan_style);
}


//Change Game Over Screen Values
void FFScript::FFSetSaveScreenSetting() 
{
	
	int32_t indx = get_register(sarg1) / 10000; 
	int32_t value = get_register(sarg2) / 10000; //bounded in zelda.cpp 
	if(indx < 0 || indx > 11) 
		set_register(sarg1, -10000); 
	else 
		SetSaveScreenSetting(indx, value); 
}
	
	
	
void FFScript::FFChangeSubscreenText() 
{ 
	
	int32_t index = get_register(sarg1) / 10000;
	int32_t arrayptr = get_register(sarg2);
	
	if ( index < 0 || index > 3 ) 
	{
		al_trace("The index supplied to Game->SetSubscreenText() is invalid. The index specified was: %d /n", index);
		return;
	}

	string filename_str;
	ArrayH::getString(arrayptr, filename_str, 73);
	ChangeSubscreenText(index,filename_str.c_str());
}

void FFScript::SetItemMessagePlayed(int32_t itm)
{
	game->item_messages_played[itm] = 1;
}
bool FFScript::GetItemMessagePlayed(int32_t itm)
{
	return ((game->item_messages_played[itm] ) ? true : false);
}

int32_t FFScript::getQRBit(int32_t rule)
{
	return ( get_qr(rule) ? 1 : 0 );
}

void FFScript::setHeroAction(int32_t a)
{
	FF_hero_action = vbound(a, 0, 255);
}

int32_t FFScript::getHeroAction()
{
	int32_t special_action = Hero.getAction2();
	if ( special_action != -1 ) return special_action; //spin, dive, charge
	else return FF_hero_action; //everything else
}

void FFScript::init()
{
	apply_qr_rules();
	eventData.clear();
	countGenScripts();
	// Some scripts can run even before ~Init (but only if qr_OLD_INIT_SCRIPT_TIMING is on), so figure out
	// the global register types ahead of time.
	markGlobalRegisters();
	for ( int32_t q = 0; q < wexLast; q++ ) warpex[q] = 0;
	temp_no_stepforward = 0;
	nostepforward = 0;
	numscriptdraws = 0;
	skipscriptdraws = false;
	max_ff_rules = qr_MAX;
	coreflags = 0;
	skip_ending_credits = 0;
	music_update_cond = 0;
	music_update_flags = 0;
	//quest_format : is this properly initialised?
	for ( int32_t q = 0; q < susptLAST; q++ ) { system_suspend[q] = 0; }

	usr_midi_volume = midi_volume;
	usr_digi_volume = digi_volume;
	usr_sfx_volume = sfx_volume;
	usr_music_volume = emusic_volume;

	usr_panstyle = pan_style;
	FF_hero_action = 0;
	enemy_removal_point[spriteremovalY1] = -32767;
	enemy_removal_point[spriteremovalY2] = 32767;
	enemy_removal_point[spriteremovalX1] = -32767;
	enemy_removal_point[spriteremovalX2] = 32767;
	enemy_removal_point[spriteremovalZ1] = -32767;
	enemy_removal_point[spriteremovalZ2] = 32767;
		
	for ( int32_t q = 0; q < 4; q++ ) 
	{
		FF_screenbounds[q] = 0;
		FF_screen_dimensions[q] = 0;
		FF_subscreen_dimensions[q] = 0;
		FF_eweapon_removal_bounds[q] = 0; 
		FF_lweapon_removal_bounds[q] = 0;
	}
	for ( int32_t q = 0; q < FFSCRIPTCLASS_CLOCKS; q++ )
	{
		FF_clocks[q] = 0;
	}
	for ( int32_t q = 0; q < SCRIPT_DRAWING_RULES; q++ )
	{
		ScriptDrawingRules[q] = 0;
	}
	for ( int32_t q = 0; q < NUM_USER_MIDI_OVERRIDES; q++ ) 
	{
		FF_UserMidis[q] = 0;
	}
	subscreen_scroll_speed = 0; //make a define for a default and read quest override! -Z
	kb_typing_mode = false;
	initIncludePaths();
	//clearRunningItemScripts();
	ScrollingScreensAll.clear();
	memset(ScrollingData, 0, sizeof(int32_t) * SZ_SCROLLDATA);
	ScrollingData[SCROLLDATA_DIR] = -1;
	user_rng_init();
	clear_script_engine_data();
	script_debug_handles.clear();
	runtime_script_debug_handle = nullptr;
}

void FFScript::shutdown()
{
	scriptEngineDatas.clear();
	objectRAM.clear();
	script_objects.clear();
}

void FFScript::SetFFEngineFlag(int32_t flag, bool state)
{
	if ( state ) { coreflags |= flag; }
	else coreflags &= ~flag;
}

void FFScript::setSubscreenScrollSpeed(byte n)
{
	subscreen_scroll_speed = n;
}

int32_t FFScript::getSubscreenScrollSpeed()
{
	return (int32_t)subscreen_scroll_speed;
}

void FFScript::do_greyscale(const bool v)
{
	// This has been removed.
}

void FFScript::do_monochromatic(const bool v)
{
	// This has been removed.
}

static int convert_6bit_to_8bit_color_shift_arg(int v)
{
	int va = abs(v);
	if (va < 64)
		return _rgb_scale_6[va] * sign(v);

	int vdiv = va / 63;
	int vmod = va % 63;
	return (vdiv * 255 + _rgb_scale_6[vmod]) * sign(v);
}

void FFScript::gfxmonohue()
{
	int32_t r   = SH::read_stack(ri->sp + 3) / 10000;
	int32_t g = SH::read_stack(ri->sp + 2) / 10000;
	int32_t b   = SH::read_stack(ri->sp + 1) / 10000;
	if (!scripting_use_8bit_colors)
	{
		r = convert_6bit_to_8bit_color_shift_arg(r);
		g = convert_6bit_to_8bit_color_shift_arg(g);
		b = convert_6bit_to_8bit_color_shift_arg(b);
	}
	bool m   = (SH::read_stack(ri->sp + 0) / 10000);
	doGFXMonohue(r,g,b,m);
}

void FFScript::clearTint()
{
	doClearTint();
}

void FFScript::Tint()
{
	int32_t r   = SH::read_stack(ri->sp + 2) / 10000;
	int32_t g = SH::read_stack(ri->sp + 1) / 10000;
	int32_t b   = SH::read_stack(ri->sp + 0) / 10000;
	if (!scripting_use_8bit_colors)
	{
		r = convert_6bit_to_8bit_color_shift_arg(r);
		g = convert_6bit_to_8bit_color_shift_arg(g);
		b = convert_6bit_to_8bit_color_shift_arg(b);
	}
	doTint(r,g,b);
}

void FFScript::do_fx_zap(const bool v)
{
	int32_t out = SH::get_arg(sarg1, v);

	if ( out ) { FFScript::do_zapout(); } 
	else FFScript::do_zapin();
}

void FFScript::do_fx_wavy(const bool v)
{
	int32_t out = SH::get_arg(sarg1, v);

	if ( out ) { FFScript::do_wavyout(); } 
	else FFScript::do_wavyin();
}

int32_t FFScript::getQuestHeaderInfo(int32_t type)
{
	return quest_format[type];
}

string get_filestr(const bool relative, bool is_file) //Used for 'FileSystem' functions.
{
	int32_t strptr = get_register(sarg1);
	string user_path;
	ArrayH::getString(strptr, user_path, 512);

	if (!relative)
	{
		user_path = user_path.substr(user_path.find_first_not_of('/'),string::npos); //Kill leading '/'
		size_t last = user_path.find_last_not_of('/');
		if(last!=string::npos)++last;
		user_path = user_path.substr(0,last); //Kill trailing '/'
		return user_path;
	}

	if (auto r = parse_user_path(user_path, is_file); !r)
	{
		scripting_log_error_with_context("Error: {}", r.error());
		return "";
	} else return r.value();
}

void FFScript::do_checkdir(const bool is_dir)
{
	string resolved_path = get_filestr(get_qr(qr_BITMAP_AND_FILESYSTEM_PATHS_ALWAYS_RELATIVE), false);
	set_register(sarg1, !resolved_path.empty() && checkPath(resolved_path.c_str(), is_dir) ? 10000 : 0);
}

void FFScript::do_fs_remove()
{
	string resolved_path = get_filestr(true, true);
	set_register(sarg1, !resolved_path.empty() && remove(resolved_path.c_str()) ? 0 : 10000);
}

void FFScript::Play_Level_Music()
{
	int32_t m = hero_scr->screen_midi;
	
	switch(m)
	{
	case -2:
		music_stop();
		break;
		
	case -1:
		play_DmapMusic();
		break;
		
	case 1:
		jukebox(ZC_MIDI_OVERWORLD);
		break;
		
	case 2:
		jukebox(ZC_MIDI_DUNGEON);
		break;
		
	case 3:
		jukebox(ZC_MIDI_LEVEL9);
		break;
		
	default:
		if(m>=4 && m<4+MAXCUSTOMMIDIS)
			jukebox(m+MIDIOFFSET_MAPSCR);
		else
			music_stop();
	}
}

void FFScript::do_warp_ex(bool v)
{
	int32_t zscript_array_ptr = SH::get_arg(sarg1, v);
	ArrayManager am(zscript_array_ptr);
	if(am.invalid()) return;
	int32_t zscript_array_size = am.size();
	switch(zscript_array_size)
	{
		case 8: // {int32_t type, int32_t dmap, int32_t screen, int32_t x, int32_t y, int32_t effect, int32_t sound, int32_t flags}
		case 9: // {int32_t type, int32_t dmap, int32_t screen, int32_t x, int32_t y, int32_t effect, int32_t sound, int32_t flags, int32_t dir}
		{
			int32_t tmpwarp[9]={0};
			for ( int32_t q = 0; q < 8; q++ )
			{
				tmpwarp[q] = (am.get(q)/10000);
			}
			tmpwarp[wexDir] = zscript_array_size < 9 ? -1 : (am.get(8)/10000);\
			if ( ((unsigned)tmpwarp[1]) >= MAXDMAPS ) 
			{
				Z_scripterrlog("Invalid DMap ID (%d) passed to WarpEx(). Aborting.\n", tmpwarp[1]);
				return;
			}
			if ( ((unsigned)tmpwarp[2]) >= MAPSCRS ) 
			{
				Z_scripterrlog("Invalid Screen Index (%d) passed to WarpEx(). Aborting.\n", tmpwarp[2]);
				return;
			}
			//Extra sanity guard.
			if ( map_screen_index(DMaps[tmpwarp[1]].map, tmpwarp[2] + DMaps[tmpwarp[1]].xoff) >= (int32_t)TheMaps.size() )
			{
				Z_scripterrlog("Invalid destination passed to WarpEx(). Aborting.\n");
				return;
			}
			if(get_qr(qr_OLD_BROKEN_WARPEX_MUSIC))
			{
				SETFLAG(tmpwarp[wexFlags],warpFlagFORCECONTINUEMUSIC,tmpwarp[wexFlags]&warpFlagFORCERESETMUSIC);
				TOGGLEFLAG(tmpwarp[wexFlags],warpFlagFORCERESETMUSIC);
			}
			//If we passed the sanity checks, populate the FFCore array and begin the action!
			for ( int32_t q = 0; q < wexActive; q++ )
			{
				FFCore.warpex[q] = tmpwarp[q];
			}
			FFCore.warpex[wexActive] = 1;
			break;
		}
	
		default: 
		{
			scripting_log_error_with_context("Array supplied is the wrong size! The array size was: %d, and valid sizes are 8 and 9.", zscript_array_size);
			break;
		}
	}
}

///////////////////////////////
//* SCRIPT ENGINE FUNCTIONS *//
////////////////////////////////////////////////////////////////////////////

void FFScript::clearRunningItemScripts()
{
	//for ( byte q = 0; q < 256; q++ ) runningItemScripts[q] = 0;
}


void FFScript::warpScriptCheck()
{
	if(get_qr(qr_SCRIPTDRAWSINWARPS))
	{
		FFCore.runWarpScripts(false);
		FFCore.runWarpScripts(true); //Waitdraw
	}
	else if(get_qr(qr_PASSIVE_SUBSCRIPT_RUNS_WHEN_GAME_IS_FROZEN) && doscript(ScriptType::ScriptedPassiveSubscreen))
	{
		if(DMaps[cur_dmap].passive_sub_script != 0)
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[cur_dmap].passive_sub_script, cur_dmap);
		if (waitdraw(ScriptType::ScriptedPassiveSubscreen) && DMaps[cur_dmap].passive_sub_script != 0 && doscript(ScriptType::ScriptedPassiveSubscreen))
		{
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[cur_dmap].passive_sub_script, cur_dmap);
			waitdraw(ScriptType::ScriptedPassiveSubscreen) = false;
		}	
	}
}

void FFScript::runWarpScripts(bool waitdraw)
{
	if(waitdraw)
	{
		if ((!( FFCore.system_suspend[susptGLOBALGAME] )) && FFCore.waitdraw(ScriptType::Global, GLOBAL_SCRIPT_GAME))
		{
			ZScriptVersion::RunScript(ScriptType::Global, GLOBAL_SCRIPT_GAME, GLOBAL_SCRIPT_GAME);
			FFCore.waitdraw(ScriptType::Global, GLOBAL_SCRIPT_GAME) = false;
		}
		if ( !FFCore.system_suspend[susptITEMSCRIPTENGINE] )
		{
			FFCore.itemScriptEngineOnWaitdraw();
		}
		if ( (!( FFCore.system_suspend[susptHEROACTIVE] )) && FFCore.waitdraw(ScriptType::Hero) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 )
		{
			ZScriptVersion::RunScript(ScriptType::Hero, SCRIPT_HERO_ACTIVE);
			FFCore.waitdraw(ScriptType::Hero) = false;
		}
		if ( (!( FFCore.system_suspend[susptDMAPSCRIPT] )) && FFCore.waitdraw(ScriptType::DMap) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 )
		{
			ZScriptVersion::RunScript(ScriptType::DMap, DMaps[cur_dmap].script,cur_dmap);
			FFCore.waitdraw(ScriptType::DMap) = false;
		}
		if ( (!( FFCore.system_suspend[susptDMAPSCRIPT] )) && FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 )
		{
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[cur_dmap].passive_sub_script,cur_dmap);
			FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) = false;
		}
		//no doscript check here, becauseb of preload? Do we want to write doscript here? -Z 13th July, 2019
		if (FFCore.getQuestHeaderInfo(vZelda) >= 0x255 && !FFCore.system_suspend[susptSCREENSCRIPTS])
		{
			for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
				if (scr->script != 0 && FFCore.waitdraw(ScriptType::Screen, scr->screen) && scr->preloadscript)
				{
					ZScriptVersion::RunScript(ScriptType::Screen, scr->script, scr->screen);  
					FFCore.waitdraw(ScriptType::Screen, scr->screen) = 0;
				}
			});
		}
	}
	else
	{
		if ((!( FFCore.system_suspend[susptGLOBALGAME] )) && FFCore.doscript(ScriptType::Global, GLOBAL_SCRIPT_GAME))
		{
			ZScriptVersion::RunScript(ScriptType::Global, GLOBAL_SCRIPT_GAME, GLOBAL_SCRIPT_GAME);
		}
		if ( !FFCore.system_suspend[susptITEMSCRIPTENGINE] )
		{
			FFCore.itemScriptEngine();
		}
		if ((!( FFCore.system_suspend[susptHEROACTIVE] )) && doscript(ScriptType::Hero) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255)
		{
			ZScriptVersion::RunScript(ScriptType::Hero, SCRIPT_HERO_ACTIVE);
		}
		if ( (!( FFCore.system_suspend[susptDMAPSCRIPT] )) && doscript(ScriptType::DMap) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) 
		{
			ZScriptVersion::RunScript(ScriptType::DMap, DMaps[cur_dmap].script,cur_dmap);
		}
		if ( (!( FFCore.system_suspend[susptDMAPSCRIPT] )) && FFCore.doscript(ScriptType::ScriptedPassiveSubscreen) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) 
		{
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[cur_dmap].passive_sub_script,cur_dmap);
		}
		if (FFCore.getQuestHeaderInfo(vZelda) >= 0x255 && !FFCore.system_suspend[susptSCREENSCRIPTS])
		{
			for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
				if (scr->script != 0 && scr->preloadscript)
				{
					ZScriptVersion::RunScript(ScriptType::Screen, scr->script, scr->screen);
				}
			});
		}
	}
}

void FFScript::runF6Engine()
{
	if(!Quit && (GameFlags&GAMEFLAG_TRYQUIT) && !(GameFlags&GAMEFLAG_F6SCRIPT_ACTIVE))
	{
		if(globalscripts[GLOBAL_SCRIPT_F6]->valid())
		{
			//Incase this was called mid-another script, store ref data
			push_ri();
			//
			clear_bitmap(f6_menu_buf);
			blit(framebuf, f6_menu_buf, 0, 0, 0, 0, framebuf->w, framebuf->h);
			initZScriptGlobalScript(GLOBAL_SCRIPT_F6);
			int32_t openingwipe = black_opening_count;
			int32_t openingshape = black_opening_shape;
			black_opening_count = 0; //No opening wipe during F6 menu
			if(black_opening_shape==bosFADEBLACK) black_fade(0);
			GameFlags |= GAMEFLAG_F6SCRIPT_ACTIVE;
			//auto tmpDrawCommands = script_drawing_commands.pop_commands();
			pause_all_sfx();

			auto& data = get_script_engine_data(ScriptType::Global, GLOBAL_SCRIPT_F6);
			while (data.doscript)
			{
				script_drawing_commands.Clear();
				load_control_state(); 
				ZScriptVersion::RunScript(ScriptType::Global, GLOBAL_SCRIPT_F6, GLOBAL_SCRIPT_F6);
				if (data.waitdraw)
				{
					ZScriptVersion::RunScript(ScriptType::Global, GLOBAL_SCRIPT_F6, GLOBAL_SCRIPT_F6);
					data.waitdraw = false;
				}
				//Draw
				clear_bitmap(framebuf);
				if( !FFCore.system_suspend[susptCOMBOANIM] ) animate_combos();
				doScriptMenuDraws();
				//
				advanceframe(true,true,false);
				if(Quit) break; //Something quit, end script running
			}
			resume_all_sfx();
			script_drawing_commands.Clear();
			//script_drawing_commands.push_commands(tmpDrawCommands);
			GameFlags &= ~GAMEFLAG_F6SCRIPT_ACTIVE;
			//Restore opening wipe
			black_opening_count = openingwipe;
			black_opening_shape = openingshape;
			if(openingshape == bosFADEBLACK)
			{
				refreshTints();
				memcpy(tempblackpal, RAMpal, PAL_SIZE*sizeof(RGB));
			}
			//Restore script refinfo
			pop_ri();
			//
			if(!Quit)
			{
				if(!get_qr(qr_NOCONTINUE))
					f_Quit(qQUIT);
			}
		}
		else f_Quit(qQUIT);
		zc_readkey(KEY_F6);
		GameFlags &= ~GAMEFLAG_TRYQUIT;
	}
}
void FFScript::runOnDeathEngine()
{
	if(!playerscripts[SCRIPT_HERO_DEATH]->valid()) return; //No script to run
	clear_bitmap(script_menu_buf);
	blit(framebuf, script_menu_buf, 0, 0, 0, 0, framebuf->w, framebuf->h);
	initZScriptHeroScripts();
	GameFlags |= GAMEFLAG_SCRIPTMENU_ACTIVE;
	kill_sfx(); //No need to pause/resume; the player is dead.
	//auto tmpDrawCommands = script_drawing_commands.pop_commands();
	
	auto& data = get_script_engine_data(ScriptType::Hero);
	while (data.doscript && !Quit)
	{
		script_drawing_commands.Clear();
		load_control_state();
		ZScriptVersion::RunScript(ScriptType::Hero, SCRIPT_HERO_DEATH);
		if (data.waitdraw)
		{
			ZScriptVersion::RunScript(ScriptType::Hero, SCRIPT_HERO_DEATH);
			data.waitdraw = false;
		}
		//Draw
		clear_bitmap(framebuf);
		if( !FFCore.system_suspend[susptCOMBOANIM] ) animate_combos();
		doScriptMenuDraws();
		//
		advanceframe(true);
	}
	script_drawing_commands.Clear();
	//script_drawing_commands.push_commands(tmpDrawCommands);
	GameFlags &= ~GAMEFLAG_SCRIPTMENU_ACTIVE;
}
void FFScript::runOnLaunchEngine()
{
	if(!globalscripts[GLOBAL_SCRIPT_ONLAUNCH]->valid()) return; //No script to run
	//Do NOT blit the prior screen to this bitmap; that would be the TITLE SCREEN.
	clear_to_color(script_menu_buf,BLACK);
	initZScriptGlobalScript(GLOBAL_SCRIPT_ONLAUNCH);
	GameFlags |= GAMEFLAG_SCRIPTMENU_ACTIVE;
	//auto tmpDrawCommands = script_drawing_commands.pop_commands();

	auto& data = get_script_engine_data(ScriptType::Global, GLOBAL_SCRIPT_ONLAUNCH);
	while (data.doscript && !Quit)
	{
		script_drawing_commands.Clear();
		load_control_state();
		ZScriptVersion::RunScript(ScriptType::Global, GLOBAL_SCRIPT_ONLAUNCH, GLOBAL_SCRIPT_ONLAUNCH);
		if (data.waitdraw)
		{
			ZScriptVersion::RunScript(ScriptType::Global, GLOBAL_SCRIPT_ONLAUNCH, GLOBAL_SCRIPT_ONLAUNCH);
			data.waitdraw = false;
		}
		//Draw
		clear_bitmap(framebuf);
		if( !FFCore.system_suspend[susptCOMBOANIM] ) animate_combos();
		
		doScriptMenuDraws();
		//
		advanceframe(true);
	}
	script_drawing_commands.Clear();
	//script_drawing_commands.push_commands(tmpDrawCommands);
	GameFlags &= ~GAMEFLAG_SCRIPTMENU_ACTIVE;
}
bool FFScript::runGenericFrozenEngine(const word script, const int32_t *init_data)
{
	user_genscript& scr = user_genscript::get(script);
	if(script < 1 || script >= NUMSCRIPTSGENERIC) return false;
	if(init_data)
	{
		for(int q = 0; q < 8; ++q)
			scr.initd[q] = init_data[q];
	}
	if(!genericscripts[script]->valid()) return false; //No script to run
	
	if(gen_frozen_index >= 400) // Experimentally tested to crash (stack overflow) at 500 for me -Em
	{
		Z_scripterrlog("Failed to run frozen generic script; too many (%zu) frozen scripts running already! Possible infinite recursion?\n", gen_frozen_index);
		return false;
	}
	//Store script refinfo
	push_ri();
	int local_i = int(gen_frozen_index++);
	reset_script_engine_data(ScriptType::GenericFrozen, local_i);
	//run script
	uint32_t fl = GameFlags & GAMEFLAG_SCRIPTMENU_ACTIVE;
	BITMAP* tmpbuf = script_menu_buf;
	if(fl)
	{
		script_menu_buf = create_bitmap_ex(8, framebuf->w, framebuf->h);
	}
	clear_bitmap(script_menu_buf);
	blit(framebuf, script_menu_buf, 0, 0, 0, 0, framebuf->w, framebuf->h);
	GameFlags |= GAMEFLAG_SCRIPTMENU_ACTIVE;
	//auto tmpDrawCommands = script_drawing_commands.pop_commands();
	while(doscript(ScriptType::GenericFrozen, local_i) && !Quit)
	{
		script_drawing_commands.Clear();
		load_control_state();
		ZScriptVersion::RunScript(ScriptType::GenericFrozen, script, local_i);
		//Draw
		clear_bitmap(framebuf);
		if( !FFCore.system_suspend[susptCOMBOANIM] ) animate_combos();
		doScriptMenuDraws();
		//
		advanceframe(true);
	}
	script_drawing_commands.Clear();
	//script_drawing_commands.push_commands(tmpDrawCommands);
	//clear
	GameFlags &= ~GAMEFLAG_SCRIPTMENU_ACTIVE;
	if(fl)
	{
		GameFlags |= fl;
		destroy_bitmap(script_menu_buf);
		script_menu_buf = tmpbuf;
	}
	clear_script_engine_data(ScriptType::GenericFrozen, local_i);
	--gen_frozen_index;
	//Restore script refinfo
	pop_ri();
	return true;
}

bool FFScript::runScriptedActiveSusbcreen()
{
	word activesubscript = DMaps[cur_dmap].active_sub_script;
	if(!activesubscript || !dmapscripts[activesubscript]->valid()) return false; //No script to run
	word passivesubscript = DMaps[cur_dmap].passive_sub_script;
	word dmapactivescript = DMaps[cur_dmap].script;
	clear_bitmap(script_menu_buf);
	blit(framebuf, script_menu_buf, 0, 0, 0, 0, framebuf->w, framebuf->h);
	initZScriptScriptedActiveSubscreen();
	GameFlags |= GAMEFLAG_SCRIPTMENU_ACTIVE;
	word script_dmap = cur_dmap;
	//auto tmpDrawCommands = script_drawing_commands.pop_commands();
	pause_all_sfx();
	auto& data = get_script_engine_data(ScriptType::ScriptedActiveSubscreen);
	while (data.doscript && !Quit)
	{
		script_drawing_commands.Clear();
		load_control_state();
		if(get_qr(qr_DMAP_ACTIVE_RUNS_DURING_ACTIVE_SUBSCRIPT) && DMaps[script_dmap].script != 0 && doscript(ScriptType::DMap))
		{
			ZScriptVersion::RunScript(ScriptType::DMap, dmapactivescript, script_dmap);
		}
		if(get_qr(qr_PASSIVE_SUBSCRIPT_RUNS_DURING_ACTIVE_SUBSCRIPT)!=0 && DMaps[script_dmap].passive_sub_script != 0 && FFCore.doscript(ScriptType::ScriptedPassiveSubscreen))
		{
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, passivesubscript, script_dmap);
		}
		ZScriptVersion::RunScript(ScriptType::ScriptedActiveSubscreen, activesubscript, script_dmap);
		if(waitdraw(ScriptType::DMap) && (get_qr(qr_DMAP_ACTIVE_RUNS_DURING_ACTIVE_SUBSCRIPT) && DMaps[script_dmap].script != 0 && doscript(ScriptType::DMap)))
		{
			ZScriptVersion::RunScript(ScriptType::DMap, dmapactivescript, script_dmap);
			waitdraw(ScriptType::DMap) = false;
		}
		if(waitdraw(ScriptType::ScriptedPassiveSubscreen) && (get_qr(qr_PASSIVE_SUBSCRIPT_RUNS_DURING_ACTIVE_SUBSCRIPT)!=0 && DMaps[script_dmap].passive_sub_script != 0 && FFCore.doscript(ScriptType::ScriptedPassiveSubscreen)))
		{
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, passivesubscript, script_dmap);
			waitdraw(ScriptType::ScriptedPassiveSubscreen) = false;
		}
		if (data.waitdraw && data.doscript)
		{
			ZScriptVersion::RunScript(ScriptType::ScriptedActiveSubscreen, activesubscript, script_dmap);
			data.waitdraw = false;
		}
		//Draw
		clear_bitmap(framebuf);
		if(cur_dmap == script_dmap && ( !FFCore.system_suspend[susptCOMBOANIM] ) ) animate_combos();
		doScriptMenuDraws();
		//
		advanceframe(true);
		//Handle warps; run game_loop once!
		if(cur_dmap != script_dmap)
		{
			activesubscript = DMaps[cur_dmap].active_sub_script;
			if(!activesubscript || !dmapscripts[activesubscript]->valid()) return true; //No script to run
			passivesubscript = DMaps[cur_dmap].passive_sub_script;
			dmapactivescript = DMaps[cur_dmap].script;
			script_dmap = cur_dmap;
			//Reset the background image
			game_loop();
			clear_bitmap(script_menu_buf);
			blit(framebuf, script_menu_buf, 0, 0, 0, 0, framebuf->w, framebuf->h);
			//Now loop without advancing frame, so that the subscreen script can draw immediately.
		}
	}
	resume_all_sfx();
	script_drawing_commands.Clear();
	//script_drawing_commands.push_commands(tmpDrawCommands);
	GameFlags &= ~GAMEFLAG_SCRIPTMENU_ACTIVE;
	GameFlags |= GAMEFLAG_RESET_GAME_LOOP;
	return true;
}
bool FFScript::runOnMapScriptEngine()
{
	word onmap_script = DMaps[cur_dmap].onmap_script;
	if(!onmap_script || !dmapscripts[onmap_script]->valid()) return false; //No script to run
	clear_bitmap(script_menu_buf);
	blit(framebuf, script_menu_buf, 0, 0, 0, 0, framebuf->w, framebuf->h);
	initZScriptOnMapScript();
	GameFlags |= GAMEFLAG_SCRIPTMENU_ACTIVE;
	word script_dmap = cur_dmap;
	//auto tmpDrawCommands = script_drawing_commands.pop_commands();
	pause_all_sfx();

	auto& data = get_script_engine_data(ScriptType::OnMap);
	while (data.doscript && !Quit)
	{
		script_drawing_commands.Clear();
		load_control_state();
		ZScriptVersion::RunScript(ScriptType::OnMap, onmap_script, script_dmap);
		if (data.waitdraw && data.doscript)
		{
			ZScriptVersion::RunScript(ScriptType::OnMap, onmap_script, script_dmap);
			data.waitdraw = false;
		}
		//Draw
		clear_bitmap(framebuf);
		if(cur_dmap == script_dmap && ( !FFCore.system_suspend[susptCOMBOANIM] ) ) animate_combos();
		doScriptMenuDraws();
		//
		advanceframe(true);
		//Handle warps; run game_loop once!
		if(cur_dmap != script_dmap)
		{
			onmap_script = DMaps[cur_dmap].onmap_script;
			if(!onmap_script || !dmapscripts[onmap_script]->valid()) return true; //No script to run
			script_dmap = cur_dmap;
			//Reset the background image
			game_loop();
			clear_bitmap(script_menu_buf);
			blit(framebuf, script_menu_buf, 0, 0, 0, 0, framebuf->w, framebuf->h);
			//Now loop without advancing frame, so that the subscreen script can draw immediately.
		}
	}
	resume_all_sfx();
	script_drawing_commands.Clear();
	//script_drawing_commands.push_commands(tmpDrawCommands);
	GameFlags &= ~GAMEFLAG_SCRIPTMENU_ACTIVE;
	GameFlags |= GAMEFLAG_RESET_GAME_LOOP;
	return true;
}

void FFScript::doScriptMenuDraws()
{
	BITMAP* menu_buf = ((GameFlags & GAMEFLAG_F6SCRIPT_ACTIVE) != 0) ? f6_menu_buf : script_menu_buf;
	blit(menu_buf, framebuf, 0, 0, 0, 0, framebuf->w, framebuf->h);
	//Script draws
	do_script_draws(framebuf, origin_scr, 0, playing_field_offset);
}

void FFScript::runOnSaveEngine()
{
	if(globalscripts[GLOBAL_SCRIPT_ONSAVE]->valid())
	{
		push_ri();
		//Prevent getting here via Quit from causing a forced-script-quit after 1000 commands!
		int32_t tQuit = Quit;
		Quit = 0;
		//
		initZScriptGlobalScript(GLOBAL_SCRIPT_ONSAVE);
		ZScriptVersion::RunScript(ScriptType::Global, GLOBAL_SCRIPT_ONSAVE, GLOBAL_SCRIPT_ONSAVE);
		//
		pop_ri();
		Quit = tQuit;
	}
}

bool FFScript::itemScriptEngine()
{
	if ( FFCore.system_suspend[susptITEMSCRIPTENGINE] ) return false;
	for ( int32_t q = 0; q < MAXITEMS; q++ )
	{
		
		if ( itemsbuf[q].script <= 0 || itemsbuf[q].script > NUMSCRIPTITEM ) continue; // > NUMSCRIPTITEM as someone could force an invaid script slot!
		
		auto& data = get_script_engine_data(ScriptType::Item, q);
		if ( data.doscript < 1 ) continue;
		
		//Passive items
		if (((itemsbuf[q].flags&item_passive_script)))
		{
			if(game->item[q] && (get_qr(qr_ITEMSCRIPTSKEEPRUNNING)))
			{
				if(get_qr(qr_PASSIVE_ITEM_SCRIPT_ONLY_HIGHEST)
					&& current_item(itemsbuf[q].family) > itemsbuf[q].fam_type)
					data.doscript = 0;
				else ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[q].script, q&0xFFF);
				if(!data.doscript)  //Item script ended. Clear the data, if any remains.
				{
					data.clear_ref();
					data.waitdraw = false;
					FFScript::deallocateAllScriptOwned(ScriptType::Item, q);
				}
			}
		}
		else
		{
		
			//Normal Items 
			/*! What happens here: When an item script is first run by the user using that utem, the script runs for one frame.
				After executing RunScript(), item_doscript is set to '1' in hero.cpp.
				If the quest allows the item to continue running, the itemScriptEngine() function ignores running the
				  same item script (again) that frame, and insteads increments item_doscript to '2'.
				If item_doscript == 2, then we know we are on the second frame, and we run it perpetually.
				If the QR to enable item scripts to run for more than one frame is not enabled, then item_doscript is set to '0'.
				If the item flag 'PERPETUAL SCRIPT' is enabled, then we ignore the lack of item_doscript==2.
				  This allows passive item scripts to function. 
			*/

			auto& data = get_script_engine_data(ScriptType::Item, q);
			
			if ( data.doscript == 1 ) // FIrst frame, normally set in hero.cpp
			{
				if ( get_qr(qr_ITEMSCRIPTSKEEPRUNNING) )
				{
					data.doscript = 2;
				}
			}
			else if (data.doscript == 2) //Second frame and later, if scripts continue to run.
			{
				ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[q].script, q&0xFFF);
			}
			else if (data.doscript == 3) //Run via itemdata->RunScript
			{
				if ( (get_qr(qr_ITEMSCRIPTSKEEPRUNNING)) ) 
				{
					data.doscript = 2; //Reduce to normal run status
				}
				else 
				{
					ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[q].script, q & 0xFFF);
					data.doscript = 0;
				}
			}
			else if(data.doscript==4)  //Item set itself false, kill script and clear data here
			{
				data.doscript = 0;
			}
			if(data.doscript==0)  //Item script ended. Clear the data, if any remains.
			{
				data.clear_ref();
				data.waitdraw = false;
				FFScript::deallocateAllScriptOwned(ScriptType::Item, q);
			}
		}
	}
	return false;
}

bool FFScript::itemScriptEngineOnWaitdraw()
{
	if ( FFCore.system_suspend[susptITEMSCRIPTENGINE] ) return false;
	for ( int32_t q = 0; q < MAXITEMS; q++ )
	{
		if ( itemsbuf[q].script <= 0 || itemsbuf[q].script > NUMSCRIPTITEM ) continue; // > NUMSCRIPTITEM as someone could force an invaid script slot!
		
		auto& data = get_script_engine_data(ScriptType::Item, q);

		if ( data.doscript < 1 ) continue;
		if (!data.waitdraw) continue;
		else data.waitdraw = false;
		
		/*! What happens here: When an item script is first run by the user using that utem, the script runs for one frame.
			After executing RunScript(), item_doscript is set to '1' in hero.cpp.
			If the quest allows the item to continue running, the itemScriptEngine() function ignores running the
			  same item script (again) that frame, and insteads increments item_doscript to '2'.
			If item_doscript == 2, then we know we are on the second frame, and we run it perpetually.
			If the QR to enable item scripts to run for more than one frame is not enabled, then item_doscript is set to '0'.
			If the item flag 'PERPETUAL SCRIPT' is enabled, then we ignore the lack of item_doscript==2.
			  This allows passive item scripts to function. 
		*/
		//Passive items
		if ((itemsbuf[q].flags&item_passive_script))
		{
			if(game->item[q] && (get_qr(qr_ITEMSCRIPTSKEEPRUNNING)))
			{
				if(get_qr(qr_PASSIVE_ITEM_SCRIPT_ONLY_HIGHEST)
					&& current_item(itemsbuf[q].family) > itemsbuf[q].fam_type)
					data.doscript = 0;
				else ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[q].script, q&0xFFF);
				if(!data.doscript)  //Item script ended. Clear the data, if any remains.
				{
					data.clear_ref();
					data.waitdraw = false;
					FFScript::deallocateAllScriptOwned(ScriptType::Item, q);
				}
			}
		}
		else
		{
			//Normal items
			if ( data.doscript == 1 ) // FIrst frame, normally set in hero.cpp
			{
				if ( get_qr(qr_ITEMSCRIPTSKEEPRUNNING) )
				{
					data.doscript = 2;
				}
				else data.doscript = 0;
			}
			else if (data.doscript == 2) //Second frame and later, if scripts continue to run.
			{
				ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[q].script, q&0xFFF);
			}
			else if (data.doscript == 3) //Run via itemdata->RunScript
			{
				if ( (get_qr(qr_ITEMSCRIPTSKEEPRUNNING)) ) 
				{
					data.doscript = 2; //Reduce to normal run status
				}
				else 
				{
					ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[q].script, q & 0xFFF);
					data.doscript = 0;
				}
			}
			else if(data.doscript==4)  //Item set itself false, kill script and clear data here.
			{
				data.doscript = 0;
			}
			if(!data.doscript)  //Item script ended. Clear the data, if any remains.
			{
				data.clear_ref();
				data.waitdraw = false;
				FFScript::deallocateAllScriptOwned(ScriptType::Item, q);
			}
		}
	}
	return false;
}
void FFScript::npcScriptEngineOnWaitdraw()
{
	if ( FFCore.system_suspend[susptNPCSCRIPTS] ) return;
	guys.run_script(MODE_WAITDRAW);
}

void FFScript::eweaponScriptEngine()
{
	if ( FFCore.system_suspend[susptEWEAPONSCRIPTS] ) return;
	Ewpns.run_script(MODE_NORMAL);
}

void FFScript::lweaponScriptEngineOnWaitdraw()
{
	if ( FFCore.system_suspend[susptLWEAPONSCRIPTS] ) return;
	Lwpns.run_script(MODE_WAITDRAW);
}

void FFScript::eweaponScriptEngineOnWaitdraw()
{
	if ( FFCore.system_suspend[susptEWEAPONSCRIPTS] ) return;
	Ewpns.run_script(MODE_WAITDRAW);
}

void FFScript::itemSpriteScriptEngine()
{
	if ( FFCore.system_suspend[susptITEMSPRITESCRIPTS] ) return;
	items.run_script(MODE_NORMAL);
}

void FFScript::itemSpriteScriptEngineOnWaitdraw()
{
	if ( FFCore.system_suspend[susptITEMSPRITESCRIPTS] ) return;
	items.run_script(MODE_WAITDRAW);
}


int32_t FFScript::getTime(int32_t type)
{
	//struct tm *tm_struct = localtime(time(NULL));
	struct tm * tm_struct;
	time_t sysRTC;
	time (&sysRTC);
	tm_struct = localtime (&sysRTC);
	int32_t rval = -1;
	
	switch(type)
	{
		case curyear:
		{
			//Year format starts at 1900, yeat
			//A raw read of '2018' would be '118', so we add 1900 to it to derive the actual year. 
			rval = tm_struct->tm_year + 1900; break;
			
		}
		case curmonth:
		{
			//Months start at 0, but we want 1->12
			rval = tm_struct->tm_mon +1; break;
		}
		case curday_month:
		{
			rval = tm_struct->tm_mday; break;
		}
		case curday_week: 
		{
			//It seems that weekdays are a value range of 1 to 7.
			rval = tm_struct->tm_wday; break;
		}
		case curhour:
		{
			rval = tm_struct->tm_hour; break;
		}
		case curminute: 
		{
			rval = tm_struct->tm_min; break;
		}
		case cursecond:
		{
			rval = tm_struct->tm_sec; break;
		}
		case curdayyear:
		{
			//The day (n/365) out of the entire year. 
			rval = tm_struct->tm_yday; break;
		}
		case curDST:
		{
			//Returns if the user is in a Time Zone with Daylight TIme of some sort. 
			//View the time.h docs for the actual values of this struct element.
			rval = tm_struct->tm_isdst;; break;
		}
		default: 
		{
			al_trace("Invalid category passed to GetSystemTime(%d)\n",type);
			rval = -1;  break;
		}
		
	}
	return rval;
}

void FFScript::do_lweapon_delete()
{
	if(0!=(s=checkLWpn(ri->lwpn)))
	{
		if(s==Hero.lift_wpn)
		{
			delete s;
			Hero.lift_wpn = nullptr;
		}
		else Lwpns.del(s);
	}
}

void FFScript::do_eweapon_delete()
{
	if(0!=(s=checkEWpn(ri->ewpn)))
	{
		Ewpns.del(s);
	}
}

void FFScript::updateIncludePaths()
{
	includePaths.clear();
	int32_t pos = 0;
	for ( int32_t q = 0; includePathString[pos]; ++q )
	{
		int32_t dest = 0;
		char buf[2048] = {0};
		while(includePathString[pos] != ';' && includePathString[pos])
		{
			buf[dest] = includePathString[pos];
			++pos;
			++dest;
		}
		++pos;
		std::string str(buf);
		includePaths.push_back(str);
	}
}

void FFScript::initIncludePaths()
{
	memset(includePathString,0,sizeof(includePathString));
	FILE* f = fopen("includepaths.txt", "r");
	if(f)
	{
		int32_t pos = 0;
		int32_t c;
		do
		{
			c = fgetc(f);
			if(c!=EOF) 
				includePathString[pos++] = c;
		}
		while(c!=EOF && pos<MAX_INCLUDE_PATH_CHARS);
		if(pos<MAX_INCLUDE_PATH_CHARS)
			includePathString[pos] = '\0';
		includePathString[MAX_INCLUDE_PATH_CHARS-1] = '\0';
		fclose(f);
	}
	else strcpy(includePathString, "include/;headers/;scripts/;");
	al_trace("Full path string is: ");
	safe_al_trace(includePathString);
	al_trace("\n");
	updateIncludePaths();

	for ( size_t q = 0; q < includePaths.size(); ++q )
	{
		al_trace("Include path %zu: ",q);
		safe_al_trace(includePaths.at(q));
		al_trace("\n");
	}
}

bool FFScript::checkExtension(std::string &filename, const std::string &extension)
//inline bool checkExtension(std::string filename, std::string extension)
{
	int32_t dot = filename.find_last_of(".");
	std::string exten = (dot == std::string::npos ? "" : filename.substr(dot, filename.length() - dot));
	return exten == extension;
}


void FFScript::do_loadgamestructs(const bool v, const bool v2)
{
	int32_t arrayptr = SH::get_arg(sarg1, v);
	int32_t section_id = SH::get_arg(sarg2, v2) / 10000;
	//Bitwise OR sections together
	string strA;
	ArrayH::getString(arrayptr, strA, 256);
	int32_t temp_sram_flags = section_id; int32_t sram_version = 0;

	if ( FFCore.checkExtension(strA, ".zcsram") )
	{
		PACKFILE *f = pack_fopen_password(strA.c_str(),F_READ, "");
		if (f)
		{
			p_igetl(&sram_version,f);
			p_igetl(&section_id,f);
			if ( sram_version > SRAM_VERSION ) //file version is greater than programme current version.
			{
				Z_scripterrlog("SRAM Version is from a version of ZC newer than the running version and cannot be loaded.\n");
				return;
			}
			if ( section_id != temp_sram_flags )
			{
				Z_scripterrlog("Reading an SRAM file with a section flag mismatch!\nThe file section flags are (%d) and the specified flagset is (%d).\nThis may cause errors!\n", section_id, temp_sram_flags);
			}
			
			if ( !section_id || section_id&svGUYS ) FFCore.read_enemies(f,sram_version);
			if ( !section_id || section_id&svITEMS )FFCore.read_items(f,sram_version);
			if ( !section_id || section_id&svWEAPONS ) FFCore.read_weaponsprtites(f,sram_version);
			if ( !section_id || section_id&svCOMBOS ) 
			{
				reset_all_combo_animations();
				FFCore.read_combos(f,sram_version);
			}
			if ( !section_id || section_id&svDMAPS ) FFCore.read_dmaps(f,sram_version);
			if ( !section_id || section_id&svMAPSCR ) FFCore.read_mapscreens(f,sram_version);
			pack_fclose(f);
			
			set_register(sarg1, 10000);
		}
		else 
		{
			Z_scripterrlog("FFCore.do_loadgamestructs could not read packfile!");
			set_register(sarg1, -10000);
		}
	}
	else
	{
		Z_scripterrlog("Tried to read a .zcsram file, but the file lacked the ..zcsram extension!\n");
		set_register(sarg1, -20000);
		
	}
}

void FFScript::do_savegamestructs(const bool v, const bool v2)
{
	int32_t arrayptr = SH::get_arg(sarg1, v);
	int32_t section_id = SH::get_arg(sarg2, v2) / 10000;
	//Bitwise OR sections together
	string strA;
	ArrayH::getString(arrayptr, strA, 256);
	int32_t cycles = 0;

	if ( FFCore.checkExtension(strA, ".zcsram") )
	{
		PACKFILE *f = pack_fopen_password(strA.c_str(),F_WRITE, "");
		if (f)
		{
			p_iputl(SRAM_VERSION,f);
			p_iputl(section_id,f);
			
			if ( !section_id || section_id&svGUYS ) FFCore.write_enemies(f,SRAM_VERSION);
			if ( !section_id || section_id&svITEMS ) FFCore.write_items(f,SRAM_VERSION);
			if ( !section_id || section_id&svWEAPONS ) FFCore.write_weaponsprtites(f,SRAM_VERSION);
			if ( !section_id || section_id&svCOMBOS ) 
			{
				reset_all_combo_animations();
				FFCore.write_combos(f,SRAM_VERSION);
			}
			if ( !section_id || section_id&svDMAPS ) FFCore.write_dmaps(f,SRAM_VERSION);
			if ( !section_id || section_id&svMAPSCR ) FFCore.write_mapscreens(f,SRAM_VERSION);
			pack_fclose(f);
			set_register(sarg1, 10000);
		}
		else 
		{
			Z_scripterrlog("FFCore.do_loadgamestructs could not read packfile!");
			set_register(sarg1, -10000);
		}
	}
	else
	{
		Z_scripterrlog("Tried to write a .zcsram file, but the file lacked the ..zcsram extension!\n");
		set_register(sarg1, -20000);
	}
}

void FFScript::do_strcmp()
{
	int32_t arrayptr_a = ri->d[rINDEX];
	int32_t arrayptr_b = ri->d[rINDEX2];
	string strA;
	string strB;
	ArrayH::getString(arrayptr_a, strA);
	ArrayH::getString(arrayptr_b, strB);
	set_register(sarg1, (strcmp(strA.c_str(), strB.c_str()) * 10000));
}

void FFScript::do_stricmp()
{
	int32_t arrayptr_a = ri->d[rINDEX];
	int32_t arrayptr_b = ri->d[rINDEX2];
	string strA;
	string strB;
	ArrayH::getString(arrayptr_a, strA);
	ArrayH::getString(arrayptr_b, strB);
	set_register(sarg1, (stricmp(strA.c_str(), strB.c_str()) * 10000));
}

void FFScript::do_LowerToUpper(const bool v)
{
	int32_t arrayptr_a = get_register(sarg1);
	string strA;
	ArrayH::getString(arrayptr_a, strA);
	for (char& c : strA)
		c = std::toupper(c); 
	ArrayH::setArray(arrayptr_a, strA);
	set_register(sarg1, 10000); // used to return 0 if string was empty.
}

void FFScript::do_UpperToLower(const bool v)
{
	int32_t arrayptr_a = get_register(sarg1);
	string strA;
	ArrayH::getString(arrayptr_a, strA);
	for (char& c : strA)
		c = std::tolower(c); 
	ArrayH::setArray(arrayptr_a, strA);
	set_register(sarg1, 10000); // used to return 0 if string was empty.
}

void FFScript::do_getnpcscript()
{
	do_get_script_index_by_name(name_to_slot_index_npcmap);
}

void FFScript::do_getcomboscript()
{
	do_get_script_index_by_name(name_to_slot_index_comboscriptmap);
}

void FFScript::do_getgenericscript()
{
	do_get_script_index_by_name(name_to_slot_index_genericmap);
}

void FFScript::do_getlweaponscript()
{
	do_get_script_index_by_name(name_to_slot_index_lwpnmap);
}
void FFScript::do_geteweaponscript()
{
	do_get_script_index_by_name(name_to_slot_index_ewpnmap);
}
void FFScript::do_getheroscript()
{
	do_get_script_index_by_name(name_to_slot_index_playermap);
}
void FFScript::do_getglobalscript()
{
	do_get_script_index_by_name(name_to_slot_index_globalmap);
}
void FFScript::do_getdmapscript()
{
	do_get_script_index_by_name(name_to_slot_index_dmapmap);
}
void FFScript::do_getscreenscript()
{
	do_get_script_index_by_name(name_to_slot_index_screenmap);
}
void FFScript::do_getitemspritescript()
{
	do_get_script_index_by_name(name_to_slot_index_itemspritemap);
}
//Not assigned to slots at present. If they ever are, then this would get the id of any script (any type) by name. -Z
void FFScript::do_getuntypedscript()
{
	set_register(sarg1, 0);
}
void FFScript::do_getsubscreenscript()
{
	do_get_script_index_by_name(name_to_slot_index_subscreenmap);
}
void FFScript::do_getnpcbyname()
{
	int32_t arrayptr = get_register(sarg1);
	string the_string;
	int32_t num = -1;
	ArrayH::getString(arrayptr, the_string, 256); //What is the max length of a script identifier?
	
	for(int32_t q = 0; q < MAXNPCS; q++)
	{
		if(!(strcmp(the_string.c_str(), guy_string[q])))
		{
			num = q;
			break;
		}
	}
	set_register(sarg1, (num * 10000));
}	
void FFScript::do_getitembyname()
{
	int32_t arrayptr = get_register(sarg1);
	string the_string;
	int32_t num = -1;
	ArrayH::getString(arrayptr, the_string, 256); //What is the max length of a script identifier?
	
	for(int32_t q = 0; q < MAXNPCS; q++)
	{
		if(!(strcmp(the_string.c_str(), item_string[q])))
		{
			num = q;
			break;
		}
	}
	set_register(sarg1, (num * 10000));
}	
void FFScript::do_getcombobyname()
{
	int32_t arrayptr = get_register(sarg1);
	string the_string;
	int32_t num = -1;
	ArrayH::getString(arrayptr, the_string, 256);
	
	if (!the_string.empty())
	{
		for(int32_t q = 0; q < MAXCOMBOS; q++)
		{
			if (the_string == combobuf[q].label)
			{
				num = q;
				break;
			}
		}
	}
	set_register(sarg1, (num * 10000));
}
void FFScript::do_getdmapbyname()
{
	int32_t arrayptr = get_register(sarg1);
	string the_string;
	int32_t num = -1;
	ArrayH::getString(arrayptr, the_string, 256); //What is the max length of a script identifier?
	
	for(int32_t q = 0; q < MAXDMAPS; q++)
	{
		if(!(strcmp(the_string.c_str(), DMaps[q].name)))
		{
			num = q;
			break;
		}
	}
	set_register(sarg1, (num * 10000));
}

////////////////////////
/// String Utilities ///
////////////////////////
void FFScript::do_ConvertCase(const bool v)
{
	int32_t arrayptr_a = get_register(sarg1);
	string strA;
	ArrayH::getString(arrayptr_a, strA);
	for (char& c : strA)
	{
		if (c < 'a')
			c += 32 * (c >= 'A' && c <= 'Z');
		else 
			c -= 32 * (c >= 'a' && c <= 'z');
	}
	ArrayH::setArray(arrayptr_a, strA);
	set_register(sarg1, (10000)); // used to return 0 if string was empty.
}

void FFScript::do_xlen(const bool v)
{
	//not implemented, xlen not found
	int32_t arrayptr = (SH::get_arg(sarg2, v));
	string str;
	ArrayH::getString(arrayptr, str);
}

void FFScript::do_xtoi(const bool v)
{
	int32_t arrayptr = (SH::get_arg(sarg2, v));
	string str;
	ArrayH::getString(arrayptr, str);
	double val = zc_xtoi(const_cast<char*>(str.c_str()));
	set_register(sarg1, (int32_t)(val) * 10000);
}
void FFScript::do_xtoi2() 
{
	int32_t arrayptr_a = ri->d[rINDEX];
	string strA;
	ArrayH::getString(arrayptr_a, strA);
	set_register(sarg1, (zc_xtoi(strA.c_str()) * 10000));
}

// Calculates log2 of number.  
double FFScript::Log2( double n )  
{  
    // log(n)/log(2) is log2.  
    return log( (double)n ) / log( (double)2 );  
}  

//xtoa, convert hex number to hex ascii
void FFScript::do_xtoa()
{
	
	int32_t arrayptr_a = get_register(sarg1);
	int32_t number = get_register(sarg2) / 10000;//ri->d[rEXP2]/10000; //why are you not in sarg2?!!
	
	
	
	bool isneg = false;
	if ( number < 0 ) 
	{
		isneg = true; 
		number *= -1;
	}
	double num = number;
	int32_t digits = num ? floor(FFCore.LogToBase(num, 16) + 1) : 1;
	
	
	int32_t pos = 0;
	string strA;
	if(number == 0) //Needs to precede str.resize(digits+3) as if the number is <= 0 then this breaks.
	{
		strA.resize(3);
		strA[pos+2] = '0';
		if(ArrayH::setArray(arrayptr_a, strA) == SH::_Overflow)
		{
			Z_scripterrlog("Dest string supplied to 'itoa()' not large enough\n");
			set_register(sarg1, 0);
		}
		else set_register(sarg1, 30000); //returns the pointer to the dest
		return;
	}
	int32_t ret = 0;
	strA.resize(digits+3+(isneg?1:0));
	//num = Floor(Abs(num));
	if ( isneg )
	{
		strA[pos] = '-';
		strA[pos+1] = '0';
		strA[pos+2] = 'x';
		ret = 3;
	}
	else
	{
		strA[pos] = '0';
		strA[pos+1] = 'x';
		ret = 2;
	}

	int32_t alphaoffset = 'A' - 0xA;
	for(int32_t i = 0; i < digits; ++i)
	{
		int32_t coeff = ((int32_t)floor((double)(((double)number) / pow((float)0x10, digits - i - 1))) % 0x10);
		strA[pos + ret + i] = coeff < 0xA ? coeff + '0' : coeff + alphaoffset;
	}
	if(ArrayH::setArray(arrayptr_a, strA) == SH::_Overflow)
	{
		scripting_log_error_with_context("Dest string parameter not large enough");
		set_register(sarg1, 0);
	}
	else set_register(sarg1, (ret + digits -(isneg?1:0))*10000); //don't count the - sign as a digit
}

void FFScript::do_ilen(const bool v)
{
	int32_t arrayptr = (SH::get_arg(sarg2, v));
	string str;
	ArrayH::getString(arrayptr, str);
	set_register(sarg1, (FFCore.ilen((char*)str.c_str()) * 10000));
}

//! Note atoi2 (atoi(str, len) can be accompished with str.resize after getString.
void FFScript::do_atoi(const bool v)
{
	int32_t arrayptr = (SH::get_arg(sarg2, v));
	string str;
	ArrayH::getString(arrayptr, str);
	set_register(sarg1, (atoi(str.c_str()) * 10000));
}
void FFScript::do_atol(const bool v)
{
	int32_t arrayptr = (SH::get_arg(sarg2, v));
	string str;
	ArrayH::getString(arrayptr, str);
	set_register(sarg1, (atoi(str.c_str())));
}

void FFScript::do_strstr()
{
	
	int32_t arrayptr_a = ri->d[rINDEX];
	int32_t arrayptr_b = ri->d[rINDEX2];
	string strA;
	string strB;
	ArrayH::getString(arrayptr_a, strA);
	ArrayH::getString(arrayptr_b, strB);
	if ( strA.size() < 1 ) 
	{
		scripting_log_error_with_context("String parameter is too small. Size is: {}", strA.size());
		set_register(sarg1,-10000);
		return;
	}
	set_register(sarg1, (strA.find(strB) * 10000));
}

void FFScript::do_strcat()
{
	
	int32_t arrayptr_a = ri->d[rINDEX];
	int32_t arrayptr_b = ri->d[rINDEX2];
	string strA;
	string strB;
	ArrayH::getString(arrayptr_a, strA);
	ArrayH::getString(arrayptr_b, strB);
	//char str_c[2048];
	//strcpy(str_c, strA.c_str());
	string strC = strA + strB;
	if(ArrayH::setArray(arrayptr_a, strC) == SH::_Overflow)
	{
		scripting_log_error_with_context("Dest string parameter is too small. Size is: {}", strA.size());
		set_register(sarg1, 0);
	}
	else set_register(sarg1, arrayptr_a); //returns the pointer to the dest
}
void FFScript::do_strspn()
{
	
	int32_t arrayptr_a = ri->d[rINDEX];
	int32_t arrayptr_b = ri->d[rINDEX2];
	string strA;
	string strB;
	ArrayH::getString(arrayptr_a, strA);
	ArrayH::getString(arrayptr_b, strB);
	set_register(sarg1, (strspn(strA.c_str(), strB.c_str()) * 10000));
}

void FFScript::do_strcspn()
{
	
	int32_t arrayptr_a = ri->d[rINDEX];
	int32_t arrayptr_b = ri->d[rINDEX2];
	string strA;
	string strB;
	ArrayH::getString(arrayptr_a, strA);
	ArrayH::getString(arrayptr_b, strB);
	set_register(sarg1, (strcspn(strA.c_str(), strB.c_str()) * 10000));
}

void FFScript::do_strchr()
{
	
	int32_t arrayptr_a = ri->d[rINDEX];
	char chr_to_find = (ri->d[rINDEX2]/10000);
	string strA; 
	ArrayH::getString(arrayptr_a, strA);
	if ( strA.size() < 1 ) 
	{
		scripting_log_error_with_context("String parameter is too small. Size is: {}", strA.size());
		set_register(sarg1,-10000);
		return;
	}
	
	set_register(sarg1,strA.find_first_of(chr_to_find)*10000);
}
void FFScript::do_strrchr()
{
	int32_t arrayptr_a = ri->d[rINDEX];
	char chr_to_find = (ri->d[rINDEX2]/10000);
	string strA; 
	ArrayH::getString(arrayptr_a, strA);
	if ( strA.size() < 1 ) 
	{
		scripting_log_error_with_context("String parameter is too small. Size is: {}", strA.size());
		set_register(sarg1,-10000);
		return;
	}
	set_register(sarg1,strA.find_last_of(chr_to_find)*10000);
}

void FFScript::do_remchr2()
{
	//Not implemented, remchr not found
	//not part of any standard library
	int32_t arrayptr_a = ri->d[rINDEX];
	string strA;
	ArrayH::getString(arrayptr_a, strA);
}
//Bookmark
void FFScript::do_atoi2()
{
	//not implemented; atoi does not take 2 params
	int32_t arrayptr_a = ri->d[rINDEX];
	string strA;
	ArrayH::getString(arrayptr_a, strA);
}
void FFScript::do_ilen2()
{
	//not implemented, ilen not found
	int32_t arrayptr_a = ri->d[rINDEX];
	string strA;
	ArrayH::getString(arrayptr_a, strA);
}
void FFScript::do_xlen2()
{
	//not implemented, xlen not found
	int32_t arrayptr_a = ri->d[rINDEX];
	string strA;
	ArrayH::getString(arrayptr_a, strA);
}

void FFScript::do_itoa()
{
	int32_t arrayptr_a = get_register(sarg1);
	int32_t number = get_register(sarg2) / 10000;
	
	char buf[16];
	zc_itoa(number, buf, 10);
	int32_t ret = ::strlen(buf) * 10000L;
	string strA(buf);
	
	if(ArrayH::setArray(arrayptr_a, strA) == SH::_Overflow)
	{
		scripting_log_error_with_context("Dest string parameter is too small. Size is: {}", strA.size());
		set_register(sarg1, -1);
	}
	else set_register(sarg1, ret); //returns the number of digits used
}

void FFScript::do_itoacat()
{
	int32_t arrayptr_a = get_register(sarg1);
	int32_t number = get_register(sarg2) / 10000;
	
	double num = number;
	int32_t digits = FFCore.numDigits(number); //int32_t(log10(temp) * 10000.0)
	int32_t pos = 0;
	int32_t ret = 0;
	string strA;
	string strB;
	strB.resize(digits);
	ArrayH::getString(arrayptr_a, strA);
	if(num < 0)
	{
		strB.resize(digits+1);
		strB[pos] = '-';
		++ret;
		num = -num;
	}
	else if(num == 0)
	{
		strB[pos] = '0';
		string strC = strA + strB;
		if(ArrayH::setArray(arrayptr_a, strC) == SH::_Overflow)
		{
			scripting_log_error_with_context("Dest string parameter is too small. Size is: {}", strA.size());
			set_register(sarg1, 0);
		}
		else set_register(sarg1, arrayptr_a); //returns the pointer to the dest
		return;
	}

	
	for(int32_t i = 0; i < digits; ++i)
		strB[pos + ret + i] = ((int32_t)floor((double)(num / pow((float)10, digits - i - 1))) % 10) + '0';
	
	string strC = strA + strB;
	if(ArrayH::setArray(arrayptr_a, strC) == SH::_Overflow)
	{
		scripting_log_error_with_context("Dest string parameter is too small. Size is: {}", strA.size());
		set_register(sarg1, 0);
	}
	else set_register(sarg1, arrayptr_a); //returns the pointer to the dest
}

void FFScript::do_strcpy(const bool a, const bool b)
{
	int32_t arrayptr_b = SH::get_arg(sarg1, a);
	int32_t arrayptr_a = SH::get_arg(sarg2, b);
	
	string strA;

	ArrayH::getString(arrayptr_a, strA);

	if(ArrayH::setArray(arrayptr_b, strA) == SH::_Overflow)
		scripting_log_error_with_context("Dest string parameter is too small. Size is: {}", strA.size());
}
void FFScript::do_arraycpy(const bool a, const bool b)
{
	int32_t arrayptr_dest = SH::get_arg(sarg1, a);
	int32_t arrayptr_src = SH::get_arg(sarg2, b);
	ArrayH::copyValues(arrayptr_dest, arrayptr_src);
}
void FFScript::do_strlen(const bool v)
{
	int32_t arrayptr = (SH::get_arg(sarg2, v));
	string str;
	ArrayH::getString(arrayptr, str);
	set_register(sarg1, (str.length() * 10000));
}

void FFScript::do_strncmp()
{
	int32_t arrayptr_a = ri->d[rINDEX];
	int32_t arrayptr_b = ri->d[rEXP2];
	int32_t len = ri->d[rEXP1]/10000;
	string strA;
	string strB;
	ArrayH::getString(arrayptr_a, strA);
	ArrayH::getString(arrayptr_b, strB);
	set_register(sarg1, (strncmp(strA.c_str(), strB.c_str(), len) * 10000));
}

void FFScript::do_strnicmp()
{
	int32_t arrayptr_a = ri->d[rINDEX];
	int32_t arrayptr_b = ri->d[rEXP2];
	int32_t len = ri->d[rEXP1]/10000;
	string strA;
	string strB;
	ArrayH::getString(arrayptr_a, strA);
	ArrayH::getString(arrayptr_b, strB);
	set_register(sarg1, (ustrnicmp(strA.c_str(), strB.c_str(), len) * 10000));
}

/////////////////////
/// MATHS HELPERS ///
/////////////////////

//Returns the log of val to the base 10. Any value <= 0 will return 0.
int32_t FFScript::Log10(double temp)
{
	int32_t ret = 0;
	if(temp > 0)
		ret = int32_t(log10(temp) * 10000.0);
	else ret = 0;
	return ret;
}

//Returns the number of digits in a given integer. 
int32_t FFScript::numDigits(int32_t number)
{
	int32_t digits = 0;
	while (number) 
	{
		number /= 10;
		digits++;
	}
	return digits;
}

// Returns the natural logarithm of val (to the base e). Any value <= 0 will return 0.
double FFScript::ln(double temp)
{
	
	if(temp > 0)
		return (log(temp));
	else
	{
		return 0;
	}
}

// Returns the logarithm of x to the given base.
double FFScript::LogToBase(double x, double base)
{
	if(x <= 0 || base <= 0) return 0;
	return FFCore.ln(x)/FFCore.ln(base);
}

///----------------------------------------------------------------------------------------------------//
//Debugger and Logging Consoles

template <typename ...Params>
void FFScript::ZScriptConsole(int32_t attributes,const char *format, Params&&... params)
{
	//if ( open )
	{
		zscript_coloured_console.Create("ZQuest Classic Logging Console", 600, 200, NULL, NULL);
		zscript_coloured_console.cls(CConsoleLoggerEx::COLOR_BACKGROUND_BLACK);
		zscript_coloured_console.gotoxy(0,0);
		zscript_coloured_console.cprintf( CConsoleLoggerEx::COLOR_BLUE | CConsoleLoggerEx::COLOR_INTENSITY |
		CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,"ZQuest Classic Logging Console\n");
	
		zscript_coloured_console.cprintf( attributes, format, std::forward<Params>(params)...);
	}
	//else
	//{
		//close
	//	zscript_coloured_console.Close();
	//}
}

void clearConsole()
{
	zscript_coloured_console.cls(CConsoleLoggerEx::COLOR_BACKGROUND_BLACK);
	zscript_coloured_console.gotoxy(0,0);

	zscript_coloured_console.cprintf( CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_BLUE | CConsoleLoggerEx::COLOR_INTENSITY |
		CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,"\n       _____   ____                  __ \n");
	zscript_coloured_console.cprintf( CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_BLUE | CConsoleLoggerEx::COLOR_INTENSITY |
		CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,"      /__  /  / __ \\__  _____  _____/ /_\n");
	zscript_coloured_console.cprintf( CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_BLUE | CConsoleLoggerEx::COLOR_INTENSITY |
		CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,"        / /  / / / / / / / _ \\/ ___/ __/\n");
	zscript_coloured_console.cprintf( CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_BLUE | CConsoleLoggerEx::COLOR_INTENSITY |
		CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,"       / /__/ /_/ / /_/ /  __(__  ) /_ \n");
	zscript_coloured_console.cprintf( CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_BLUE | CConsoleLoggerEx::COLOR_INTENSITY |
		CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,"      /____/\\___\\_\\__,_/\\___/____/\\__/\n\n");

	zscript_coloured_console.cprintf( CConsoleLoggerEx::COLOR_BLUE | CConsoleLoggerEx::COLOR_INTENSITY |
		CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,"ZC Console\n");
	
	zscript_coloured_console.cprintf( CConsoleLoggerEx::COLOR_BLUE |CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY |
		CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,"Running: %s\n", getVersionString());
	if ( FFCore.getQuestHeaderInfo(vZelda) > 0 )
	{
		char const* verstr = QHeader.getVerStr();
		if(verstr[0])
		{
			auto vercmp = QHeader.compareVer();
			auto astatecmp = compare(int32_t(QHeader.getAlphaState()), getAlphaState());
			auto avercmp = compare(QHeader.getAlphaVer(), 0);
			auto timecmp = QHeader.compareDate();
			if(!(vercmp || astatecmp || avercmp))
			{
				if(!timecmp || !QHeader.new_version_is_nightly)
					zscript_coloured_console.cprintf( CConsoleLoggerEx::COLOR_BLUE |CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY |
						CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,"Quest Made in this build\n", verstr);
				else if(timecmp < 0)
				{
					zscript_coloured_console.cprintf( CConsoleLoggerEx::COLOR_BLUE |CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY |
						CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,"Quest Made in an earlier nightly of the same build\n", verstr);
				}
				else
				{
					zscript_coloured_console.cprintf( CConsoleLoggerEx::COLOR_BLUE |CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY |
						CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,"Quest Made in an LATER nightly of the same build!\n"
							"This may be unsafe to play in this version!\n", verstr);
				}
			}
			else zscript_coloured_console.cprintf( CConsoleLoggerEx::COLOR_BLUE |CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY |
				CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,"Quest Made in: %s\n", verstr);
		}
	}
}
void FFScript::ZScriptConsole(bool open)
{
	if ( open )
	{
		zscript_coloured_console.Create("ZC Console", 600, 200, NULL, NULL);
		clearConsole();
		console_enabled = 1;
	}
	else
	{
		zscript_coloured_console.Close();
		console_enabled = 0;
	}
	zc_set_config("CONSOLE","enabled",console_enabled);
}

///----------------------------------------------------------------------------------------------------//
//Tracing

void FFScript::do_trace(bool v)
{
	bool should_replay_trace = replay_is_active() && replay_get_meta_bool("script_trace");
	// For now, only prevent tracing to allegro log for Web version. Some quests may expect players to
	// look in the logs for spoiler/secret stuff.
#ifdef __EMSCRIPTEN__
	bool should_trace = console_enabled || should_replay_trace;
	if (!should_trace) return;
#endif

	int32_t temp = SH::get_arg(sarg1, v);
	
	char tmp[100];
	sprintf(tmp, (temp < 0 ? "%06d" : "%05d"), temp);
	string s2(tmp);
	s2 = s2.substr(0, s2.size() - 4) + "." + s2.substr(s2.size() - 4, 4) + "\n";
	TraceScriptIDs();
	al_trace("%s", s2.c_str());
	if (should_replay_trace)
		replay_step_comment("trace: " + s2);
	
	if ( console_enabled ) 
	{
		zscript_coloured_console.safeprint((CConsoleLoggerEx::COLOR_WHITE | 
			CConsoleLoggerEx::COLOR_BACKGROUND_BLACK),s2.c_str());
	}
}
void FFScript::do_tracel(bool v)
{
	int32_t temp = SH::get_arg(sarg1, v);
	
	char tmp[32];
	sprintf(tmp, "%d\n", temp);
	TraceScriptIDs();
	al_trace("%s", tmp);
	if (replay_is_active() && replay_get_meta_bool("script_trace"))
		replay_step_comment(fmt::format("trace: {}", temp));
	
	if ( console_enabled ) 
	{
		zscript_coloured_console.safeprint((CConsoleLoggerEx::COLOR_WHITE | 
			CConsoleLoggerEx::COLOR_BACKGROUND_BLACK),tmp);
	}
}

void FFScript::do_tracebool(const bool v)
{
	int32_t temp = SH::get_arg(sarg1, v);
	TraceScriptIDs();
	char const* str = temp ? "true\n" : "false\n";
	al_trace("%s", str);
	if (replay_is_active() && replay_get_meta_bool("script_trace"))
		replay_step_comment(fmt::format("trace: {}", (bool)temp));
	
	if ( console_enabled ) 
	{
		zscript_coloured_console.safeprint((CConsoleLoggerEx::COLOR_WHITE | 
			CConsoleLoggerEx::COLOR_BACKGROUND_BLACK),str);
	}
}

void traceStr(string const& str)
{
	FFCore.TraceScriptIDs();
	safe_al_trace(str);
	if (replay_is_active() && replay_get_meta_bool("script_trace"))
		replay_step_comment("trace: " + str);
	
	if ( console_enabled ) 
	{
		zscript_coloured_console.safeprint((CConsoleLoggerEx::COLOR_WHITE | 
			CConsoleLoggerEx::COLOR_BACKGROUND_BLACK),str.c_str());
	}
}

void FFScript::do_tracestring()
{
	int32_t arrayptr = get_register(sarg1);
	string str;
	ArrayH::getString(arrayptr, str, 512);
	str += "\0"; //In the event that the user passed an array w/o NULL, don't crash.
	traceStr(str);
}

bool is_valid_format(char c)
{
	switch(c)
	{
		case 'f': case 'd': case 'i': case 'p':
		case 'l': case 's': case 'c': case 'X':
		case 'x': case 'b': case 'B': case 'a':
			return true;
	}
	return false;
}
#define FORMATTER_FLAG_0FILL    0x01
char const* zs_formatter(char const* format, int32_t arg, int32_t mindig, dword flags)
{
	static std::string ret;
	
	ret.clear();
	if(format)
	{
		std::string mdstr = fmt::format("%{}{}{}",(flags&FORMATTER_FLAG_0FILL)?"0":"",
			mindig, (format[0] == 'x' || format[0] == 'X') ? format[0] : 'd');
		char const* mindigbuf = mdstr.c_str();
		bool tempbool = false;
		switch(format[0])
		{
			case 'f':
				tempbool = true;
				[[fallthrough]];
			case 'd':
				if(arg%10000)
					tempbool = true;
				[[fallthrough]];
			case 'i':
			case 'p':
			{
				char argbuf[32] = {0};
				bool neg = arg < 0;
				if(mindig)
					sprintf(argbuf,mindigbuf,arg / 10000);
				else zc_itoa(arg / 10000, argbuf);
				
				if(tempbool) //add decimal places
				{
					arg = abs(arg);
					auto ind = strlen(argbuf);
					argbuf[ind++] = '.';
					for(int div = 1000; div > 0; div /= 10)
						argbuf[ind++] = '0' + (arg/div)%10;
					for(--ind; argbuf[ind]=='0' && argbuf[ind-1]!='-'; --ind)
					{
						argbuf[ind] = 0;
					}
				}
				
				if(neg && argbuf[0] != '-')
					ret = "-";
				ret += argbuf;
				return ret.c_str();
			}
			//
			case 'l':
			{
				char argbuf[32] = {0};
				if(mindig)
					sprintf(argbuf, mindigbuf, arg);
				else zc_itoa(arg, argbuf);
				
				ret = argbuf;
				return ret.c_str();
			}
			//
			case 's':
			{
				if(mindig)
					Z_scripterrlog("Cannot use minimum digits flag for '%%s'\n");
				if(arg)
				{
					int32_t strptr = arg;
					ArrayManager am(strptr);
					if(am.invalid())
						ret = "<INVALID STRING>";
					else ArrayH::getString(strptr, ret, MAX_ZC_ARRAY_SIZE);
				}
				else ret = "<NULL>";
				return ret.c_str();
			}
			case 'c':
			{
				if(mindig)
					Z_scripterrlog("Cannot use minimum digits flag for '%%c'\n");
				int32_t c = (arg / 10000);
				if ( (byte(c)) != c )
				{
					Z_scripterrlog("Illegal char value (%d) passed to sprintf as '%%c' arg\n", c);
					Z_scripterrlog("Value of invalid char will overflow.\n");
				}
				ret.push_back(byte(c));
				return ret.c_str();
			}
			//
			case 'X':
				tempbool = true;
				[[fallthrough]];
			case 'x':
			{
				char argbuf[32] = {0};
				if(mindig)
					sprintf(argbuf,mindigbuf,arg / 10000);
				else zc_itoa( (arg/10000), argbuf, 16 ); //base 16; hex
				
				for ( int32_t inx = 0; inx < 16; ++inx ) //set chosen caps
				{
					argbuf[inx] = ( tempbool ? toupper(argbuf[inx]) : tolower(argbuf[inx]) );
				}
				ret = "0x";
				ret += argbuf;
				return ret.c_str();
			}
			//
			case 'b': //int binary
				arg /= 10000;
				[[fallthrough]];
			case 'B': //long binary
			{
				char argbuf[33] = {0};
				int num_digits = mindig;
				for(int q = num_digits; q < 32; ++q)
					if(arg&(1<<q))
						num_digits = q+1;
				for(int q = 0; q < num_digits; ++q)
				{
					argbuf[q] = (arg&(1<<(num_digits-q-1)))
						? '1' : '0';
				}
				ret = argbuf;
				return ret.c_str();
			}
			case 'a': //array
			{
				if(arg)
				{
					if(!is_valid_format(format[1]))
					{
						Z_scripterrlog("Format '%%a%c' is invalid!\n",format[1]);
						break;
					}
					ArrayManager am(arg);
					ret = am.asString([&](int32_t val)
						{
							return zs_formatter(format+1, val, mindig, flags);
						}, 214748);
				}
				else ret = "{ NULL }";
				return ret.c_str();
			}
			default:
			{
				Z_scripterrlog("Error: '%%%c' is not a valid printf argument.\n",format[0]);
				return ret.c_str();
			}
		}
	}
	Z_scripterrlog("Error: No format parameter given for zs_formatter\n");
	return ret.c_str();
}

static int32_t zspr_varg_getter(int32_t,int32_t next_arg)
{
	return zs_vargs.at(next_arg);
}
static int32_t zspr_stack_getter(int32_t num_args, int32_t next_arg)
{
	return SH::read_stack(((ri->sp + num_args) - 1) - next_arg);
}
string zs_sprintf(char const* format, int32_t num_args, std::function<int32_t(int32_t,int32_t)> arg_getter)
{
	int32_t next_arg = 0;
	bool is_old_args = get_qr(qr_OLD_PRINTF_ARGS);
	ostringstream oss;
	while(format[0] != '\0')
	{
		int32_t arg_val = 0;
		if(next_arg < num_args)
		{
			arg_val = arg_getter(num_args,next_arg);
		}
		else if(get_qr(qr_PRINTF_NO_0FILL))
		{
			oss << format;
			return oss.str();
		}
		char buf[256] = {0};
		for ( int32_t q = 0; q < 256; ++q )
		{
			if(format[0] == '\0') //done
			{
				oss << buf;
				return oss.str();
			}
			else if(format[0] == '%')
			{
				++format;
				int32_t min_digits = 0;
				dword formatter_flags = 0;
				if(format[0] >= '0' && format[0] <= '9' && !is_old_args)
				{
					char argbuf[4] = {0};
					int32_t q = 0;
					if(format[0] == '0') //Leading 0 means to 0-fill, and gets eaten
						formatter_flags |= FORMATTER_FLAG_0FILL;
					else --format; //else don't eat
					while(q < 4)
					{
						++format;
						char c = format[0];
						if(c == '\0')
						{
							Z_scripterrlog("Cannot use minimum digits flag with no argument\n");
							oss << buf;
							return oss.str();
						}
						if(c >= '0' && c <= '9')
							argbuf[q++] = c;
						else
						{
							--format;
							break;
						}
					}
					++format;
					min_digits = atoi(argbuf);
					if(!min_digits)
					{
						Z_scripterrlog("Error formatting string: Invalid number '%s'\n", argbuf);
					}
				}
				bool bin = (format[0] == 'b' || format[0] == 'B');
				bool hex = (format[0] == 'x' || format[0] == 'X');
				if(bin)
				{
					if(min_digits > 32)
					{
						Z_scripterrlog("Min digits argument cannot be larger than 32!"
							" Value will be truncated to 32.");
						min_digits = 32;
					}
				}
				else if(min_digits > 10)
				{
					Z_scripterrlog("Min digits argument cannot be larger than 10!"
						" Value will be truncated to 10.");
					min_digits = 10;
				}
				
				bool tempbool = false;
				switch( format[0] )
				{
					case 'd':
					case 'f':
					case 'i': case 'p':
					case 'l':
					case 's':
					case 'c':
					case 'x': case 'X':
					case 'b':  case 'B':
					{
						++next_arg;
						oss << buf << zs_formatter(format,arg_val,min_digits,formatter_flags);
						q = 300; //break main loop
						break;
					}
					case 'a': //array print
					{
						++next_arg;
						oss << buf << zs_formatter(format,arg_val,min_digits,formatter_flags);
						while(format[0] == 'a')
						{
							if(is_valid_format(format[1]))
								++format;
							else break;
						}
						q = 300; //break main loop
						break;
					}
					case '%':
					{
						if(min_digits)
							Z_scripterrlog("Cannot use minimum digits flag for '%%%%'\n");
						buf[q] = '%';
						break;
					}
					default:
					{
						if(is_old_args)
							buf[q] = format[0];
						else
						{
							Z_scripterrlog("Error: '%%%c' is not a valid printf argument.\n",format[0]);
						}
						break;
					}
				}
				++format;
			}
			else
			{
				buf[q] = format[0];
				++format;
			}
			if(q == 255)
			{
				oss << buf;
				break;
			}
		}
	}
	return oss.str();
}

void FFScript::do_printf(const bool v, const bool varg)
{
	int32_t num_args, format_arrayptr;
	if(varg)
	{
		num_args = zs_vargs.size();
		format_arrayptr = SH::read_stack(ri->sp);
	}
	else
	{
		num_args = SH::get_arg(sarg1, v) / 10000;
		format_arrayptr = SH::read_stack(ri->sp + num_args);
	}
	ArrayManager fmt_am(format_arrayptr);
	if(!fmt_am.invalid())
	{
		string formatstr;
		ArrayH::getString(format_arrayptr, formatstr, MAX_ZC_ARRAY_SIZE);
		
		traceStr(zs_sprintf(formatstr.c_str(), num_args, varg ? zspr_varg_getter : zspr_stack_getter));
	}
	if(varg)
		zs_vargs.clear();
}
void FFScript::do_sprintf(const bool v, const bool varg)
{
	int32_t num_args, dest_arrayptr, format_arrayptr;
	if(varg)
	{
		num_args = zs_vargs.size();
		dest_arrayptr = SH::read_stack(ri->sp + 1);
		format_arrayptr = SH::read_stack(ri->sp);
	}
	else
	{
		num_args = SH::get_arg(sarg1, v) / 10000;
		dest_arrayptr = SH::read_stack(ri->sp + num_args + 1);
		format_arrayptr = SH::read_stack(ri->sp + num_args);
	}
	ArrayManager fmt_am(format_arrayptr);
	ArrayManager dst_am(dest_arrayptr);
	if(fmt_am.invalid() || dst_am.invalid())
		ri->d[rEXP1] = 0;
	else
	{
		string formatstr;
		ArrayH::getString(format_arrayptr, formatstr, MAX_ZC_ARRAY_SIZE);
		
		string output = zs_sprintf(formatstr.c_str(), num_args, varg ? zspr_varg_getter : zspr_stack_getter);
		if(ArrayH::setArray(dest_arrayptr, output, true) == SH::_Overflow)
		{
			Z_scripterrlog("Dest string supplied to 'sprintf()' not large enough and cannot be resized\n");
			ri->d[rEXP1] = ArrayH::strlen(dest_arrayptr);
		}
		else ri->d[rEXP1] = output.size();
	}
	if(varg)
		zs_vargs.clear();
}
void FFScript::do_printfarr()
{
	int32_t format_arrayptr = SH::read_stack(ri->sp + 1),
		args_arrayptr = SH::read_stack(ri->sp + 0);
	ArrayManager fmt_am(format_arrayptr);
	ArrayManager arg_am(args_arrayptr);
	if(!(fmt_am.invalid() || arg_am.invalid()))
	{
		auto num_args = arg_am.size();
		string formatstr;
		ArrayH::getString(format_arrayptr, formatstr, MAX_ZC_ARRAY_SIZE);
		
		traceStr(zs_sprintf(formatstr.c_str(), num_args,
			[&](int32_t,int32_t next_arg)
			{
				return arg_am.get(next_arg);
			}));
	}
}
void FFScript::do_sprintfarr()
{
	int32_t dest_arrayptr = SH::read_stack(ri->sp + 2),
		format_arrayptr = SH::read_stack(ri->sp + 1),
		args_arrayptr = SH::read_stack(ri->sp + 0);
	ArrayManager fmt_am(format_arrayptr);
	ArrayManager arg_am(args_arrayptr);
	ArrayManager dst_am(dest_arrayptr);
	if(fmt_am.invalid() || arg_am.invalid() || dst_am.invalid())
		ri->d[rEXP1] = 0;
	else
	{
		auto num_args = arg_am.size();
		string formatstr;
		ArrayH::getString(format_arrayptr, formatstr, MAX_ZC_ARRAY_SIZE);
		
		string output = zs_sprintf(formatstr.c_str(), num_args,
			[&](int32_t,int32_t next_arg)
			{
				return arg_am.get(next_arg);
			});
		
		if(ArrayH::setArray(dest_arrayptr, output, true) == SH::_Overflow)
		{
			Z_scripterrlog("Dest string supplied to 'sprintfa()' not large enough and cannot be resized\n");
			ri->d[rEXP1] = ArrayH::strlen(dest_arrayptr);
		}
		else ri->d[rEXP1] = output.size();
	}
}
void FFScript::do_varg_max()
{
	int32_t num_args = zs_vargs.size();
	int32_t val = 0;
	if (num_args > 0)
		val = zs_vargs.at(0);
	for(auto q = 1; q < num_args; ++q)
	{
		int32_t tval = zs_vargs.at(q);
		if(tval > val) val = tval;
	}
	zs_vargs.clear();
	ri->d[rEXP1] = val;
}
void FFScript::do_varg_min()
{
	int32_t num_args = zs_vargs.size();
	int32_t val = 0;
	if (num_args > 0)
		val = zs_vargs.at(0);
	for(auto q = 1; q < num_args; ++q)
	{
		int32_t tval = zs_vargs.at(q);
		if(tval < val) val = tval;
	}
	zs_vargs.clear();
	ri->d[rEXP1] = val;
}
void FFScript::do_varg_choose()
{
	int32_t num_args = zs_vargs.size();
	int32_t val = 0;
	if(num_args > 0)
	{
		int32_t choice = zc_rand(num_args-1);
		val = zs_vargs.at(choice);
	}
	zs_vargs.clear();
	ri->d[rEXP1] = val;
}
void FFScript::do_varg_makearray(ScriptType type, const uint32_t UID, script_object_type object_type)
{
	auto vargs = zs_vargs;
	zs_vargs.clear();

	size_t size = vargs.size();
	ri->d[rEXP1] = 0;

	if (ZScriptVersion::gc_arrays())
	{
		auto* array = script_arrays.create();
		if (!array)
			return;

		ZScriptArray &a = array->arr;
		a.Resize(size);
		a.setValid(true);
		a.setObjectType(object_type);

		for(size_t j = 0; j < size; ++j)
			a[j] = vargs[j]; //initialize array

		ri->d[rEXP1] = array->id;

		return;
	}

	dword ptrval;
	for(ptrval = 1; localRAM[ptrval].Valid(); ptrval++) ;
	
	if(ptrval >= NUM_ZSCRIPT_ARRAYS)
	{
		Z_scripterrlog("%d local arrays already in use, no more can be allocated\n", NUM_ZSCRIPT_ARRAYS-1);
		ptrval = 0;
	}
	else
	{
		ZScriptArray &a = localRAM[ptrval];
		
		a.Resize(size);
		a.setValid(true);
		
		for(size_t j = 0; j < size; ++j)
			a[j] = vargs[j]; //initialize array
		
		arrayOwner[ptrval].clear();
		arrayOwner[ptrval].reown(type, UID);
	}

	ri->d[rEXP1] = ptrval*10000;
}

void FFScript::do_breakpoint()
{
	// TODO: implement as `debugger;` statement when VS Code extension exists.
}

void FFScript::do_tracenl()
{
	safe_al_trace("\n");
	
	if ( console_enabled ) 
	{
		zscript_coloured_console.safeprint((CConsoleLoggerEx::COLOR_WHITE | 
			CConsoleLoggerEx::COLOR_BACKGROUND_BLACK),"\n");
	}
}


void FFScript::TraceScriptIDs(bool force_show_context)
{
	if(DEVTIMESTAMP)
	{
		CConsoleLoggerEx console = zscript_coloured_console;
		bool cond = console_enabled;
		
		char buf[256] = {0};
		//Calculate timestamp
		struct tm * tm_struct;
		time_t sysRTC;
		time (&sysRTC);
		tm_struct = localtime (&sysRTC);
		
		sprintf(buf, "[%d:%d:%d] ", tm_struct->tm_hour, tm_struct->tm_min, tm_struct->tm_sec);
		//
		
		al_trace("%s", buf);
		if ( cond ) {console.safeprint((CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY | 
			CConsoleLoggerEx::COLOR_BACKGROUND_BLACK),buf); }
	}

	bool show_context = force_show_context || (get_qr(qr_TRACESCRIPTIDS) || DEVLOGGING);
	if (show_context)
	{
		CConsoleLoggerEx console = zscript_coloured_console;
		bool cond = console_enabled;
		char buf[256] = {0};
		if(script_funcrun)
		{
			sprintf(buf, "Destructor(%d,%s): ", ri->thiskey, destructstr?destructstr->c_str():"UNKNOWN");
		}
		else switch(curScriptType)
		{
			case ScriptType::Global:
			{
				switch(curScriptNum)
				{
					case GLOBAL_SCRIPT_INIT:
						sprintf(buf, "Global Init(%s): ", globalmap[curScriptNum].scriptname.c_str());
						break;
					case GLOBAL_SCRIPT_GAME:
						sprintf(buf, "Global Active(%s): ", globalmap[curScriptNum].scriptname.c_str());
						break;
					case GLOBAL_SCRIPT_END:
						sprintf(buf, "Global Exit(%s): ", globalmap[curScriptNum].scriptname.c_str());
						break;
					case GLOBAL_SCRIPT_ONSAVELOAD:
						sprintf(buf, "Global SaveLoad(%s): ", globalmap[curScriptNum].scriptname.c_str());
						break;
					case GLOBAL_SCRIPT_ONLAUNCH:
						sprintf(buf, "Global Launch(%s): ", globalmap[curScriptNum].scriptname.c_str());
						break;
					case GLOBAL_SCRIPT_ONCONTGAME:
						sprintf(buf, "Global ContGame(%s): ", globalmap[curScriptNum].scriptname.c_str());
						break;
					case GLOBAL_SCRIPT_F6:
						sprintf(buf, "Global F6Menu(%s): ", globalmap[curScriptNum].scriptname.c_str());
						break;
					case GLOBAL_SCRIPT_ONSAVE:
						sprintf(buf, "Global Save(%s): ", globalmap[curScriptNum].scriptname.c_str());
						break;
				}
				break;
			}
			
			case ScriptType::Hero:
			{
				switch(curScriptNum)
				{
					case SCRIPT_HERO_INIT:
						sprintf(buf, "Hero Init(%s): ", playermap[curScriptNum-1].scriptname.c_str());
						break;
					case SCRIPT_HERO_ACTIVE:
						sprintf(buf, "Hero Active(%s): ", playermap[curScriptNum-1].scriptname.c_str());
						break;
					case SCRIPT_HERO_DEATH:
						sprintf(buf, "Hero Death(%s): ", playermap[curScriptNum-1].scriptname.c_str());
						break;
					case SCRIPT_HERO_WIN:
						sprintf(buf, "Hero Win(%s): ", playermap[curScriptNum-1].scriptname.c_str());
						break;
				}
				break;
			}
			
			case ScriptType::Lwpn:
				sprintf(buf, "LWeapon(%u, %s): ", curScriptNum,lwpnmap[curScriptNum-1].scriptname.c_str());
				break;
			
			case ScriptType::Ewpn:
				sprintf(buf, "EWeapon(%u, %s): ", curScriptNum,ewpnmap[curScriptNum-1].scriptname.c_str());
				break;
			
			case ScriptType::NPC:
				sprintf(buf, "NPC(%u, %s): ", curScriptNum,npcmap[curScriptNum-1].scriptname.c_str());
				break;
				
			case ScriptType::FFC:
				sprintf(buf, "FFC(%u, %s): ", curScriptNum,ffcmap[curScriptNum-1].scriptname.c_str());
				break;
				
			case ScriptType::Item:
				sprintf(buf, "Item(%u, %s): ", curScriptNum,itemmap[curScriptNum-1].scriptname.c_str());
				break;
			
			case ScriptType::OnMap:
				sprintf(buf, "DMapMap(%u, %s): ", curScriptNum,dmapmap[curScriptNum-1].scriptname.c_str());
				break;
			case ScriptType::ScriptedActiveSubscreen:
				sprintf(buf, "DMapASub(%u, %s): ", curScriptNum,dmapmap[curScriptNum-1].scriptname.c_str());
				break;
			case ScriptType::ScriptedPassiveSubscreen:
				sprintf(buf, "DMapPSub(%u, %s): ", curScriptNum,dmapmap[curScriptNum-1].scriptname.c_str());
				break;
			case ScriptType::DMap:
				sprintf(buf, "DMap(%u, %s): ", curScriptNum,dmapmap[curScriptNum-1].scriptname.c_str());
				break;
			
			case ScriptType::ItemSprite:
				sprintf(buf, "ItemSprite(%u, %s): ", curScriptNum,itemspritemap[curScriptNum-1].scriptname.c_str());
				break;
			
			case ScriptType::Screen:
				sprintf(buf, "Screen(%u, %s): ", curScriptNum,screenmap[curScriptNum-1].scriptname.c_str());
				break;
			
			case ScriptType::Combo:
				sprintf(buf, "Combo(%u, %s): ", curScriptNum,comboscriptmap[curScriptNum-1].scriptname.c_str());
				break;
				
			case ScriptType::Generic:
				sprintf(buf, "Generic(%u, %s): ", curScriptNum,genericmap[curScriptNum-1].scriptname.c_str());
				break;
				
			case ScriptType::GenericFrozen:
				sprintf(buf, "GenericFRZ(%u, %s): ", curScriptNum,genericmap[curScriptNum-1].scriptname.c_str());
				break;
				
			case ScriptType::EngineSubscreen:
				sprintf(buf, "Subscreen(%u, %s): ", curScriptNum,subscreenmap[curScriptNum-1].scriptname.c_str());
				break;
		}
		
		al_trace("%s", buf);
		if ( cond )
			console.safeprint((CConsoleLoggerEx::COLOR_GREEN|CConsoleLoggerEx::COLOR_INTENSITY|
				CConsoleLoggerEx::COLOR_BACKGROUND_BLACK),buf);
	}
}

void FFScript::do_cleartrace()
{
	zc_trace_clear();
	clearConsole();
}

string inttobase(word base, int32_t x, word mindigits)
{
	static const char coeff[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	
	string s2;
	word digits = zc_max(mindigits - 1, word(floor(log(double(x)) / log(double(base)))));
	
	for(int32_t i = digits; i >= 0; i--)
	{
		s2 += coeff[word(floor(x / pow(double(base), i))) % base];
	}
	
	return s2;
}

void FFScript::do_tracetobase()
{
	int32_t x = SH::read_stack(ri->sp + 2) / 10000;
	uint32_t base = vbound(SH::read_stack(ri->sp + 1) / 10000, 2, 36);
	uint32_t mindigits = zc_max(1, SH::read_stack(ri->sp) / 10000);
	
	string s2 = x < 0 ? "-": "";
	
	switch(base)
	{
	case 8:
		s2 += '0';
		break;
		
	case 16:
		s2 += "0x";
		break;
	}
	
	s2 += inttobase(base, int32_t(fabs(double(x))), mindigits);
	
	switch(base)
	{
	case 8:
	case 10:
	case 16:
		break;
		
	case 2:
		s2 += 'b';
		break;
		
	default:
		std::stringstream ss;
		ss << " (Base " << base << ')';
		s2 += ss.str();
		break;
	}
	TraceScriptIDs();
	s2 += "\n";
	al_trace("%s", s2.c_str());
	
	if ( console_enabled ) 
	{
		zscript_coloured_console.safeprint((CConsoleLoggerEx::COLOR_WHITE | 
			CConsoleLoggerEx::COLOR_BACKGROUND_BLACK),s2.c_str());
	}
}

//SRAM Functions
void FFScript::write_dmaps(PACKFILE *f, int32_t vers_id)
{
	word dmap_count=count_dmaps();
  
		dmap_count=zc_min(dmap_count, 512);
		dmap_count=zc_min(dmap_count, MAXDMAPS-0);
		
		//finally...  section data
		if(!p_iputw(dmap_count,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",5);
		}
		
		
		for(int32_t i=0; i<dmap_count; i++)
		{
			if(!p_putc(DMaps[i].map,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",6);
			}
			
			if(!p_iputw(DMaps[i].level,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",7);
			}
			
			if(!p_putc(DMaps[i].xoff,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",8);
			}
			
			if(!p_putc(DMaps[i].compass,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",9);
			}
			
			if(!p_iputw(DMaps[i].color,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",10);
			}
			
			if(!p_putc(DMaps[i].midi,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",11);
			}
			
			if(!p_putc(DMaps[i].cont,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",12);
			}
			
			if(!p_putc(DMaps[i].type,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",13);
			}
			
			for(int32_t j=0; j<8; j++)
			{
				if(!p_putc(DMaps[i].grid[j],f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",14);
				}
			}
			
			//16
			if(!pfwrite(&DMaps[i].name,sizeof(DMaps[0].name),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",15);
			}
			
			if(!p_putwstr(DMaps[i].title,f))
            {
                Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",16);
            }
			
			if(!pfwrite(&DMaps[i].intro,sizeof(DMaps[0].intro),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",17);
			}
			
			if(!p_iputl(DMaps[i].minimap_tile[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",18);
			}
			
			if(!p_putc(DMaps[i].minimap_cset[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",19);
			}
			
			if(!p_iputl(DMaps[i].minimap_tile[1],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",20);
			}
			
			if(!p_putc(DMaps[i].minimap_cset[1],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",21);
			}
			
			if(!p_iputl(DMaps[i].largemap_tile[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",22);
			}
			
			if(!p_putc(DMaps[i].largemap_cset[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",23);
			}
			
			if(!p_iputl(DMaps[i].largemap_tile[1],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",24);
			}
			
			if(!p_putc(DMaps[i].largemap_cset[1],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",25);
			}
			
			if(!pfwrite(&DMaps[i].tmusic,sizeof(DMaps[0].tmusic),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",26);
			}
			
			if(!p_putc(DMaps[i].tmusictrack,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",25);
			}
			
			if(!p_putc(DMaps[i].active_subscreen,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",26);
			}
			
			if(!p_putc(DMaps[i].passive_subscreen,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",27);
			}
			
			byte disabled[32];
			memset(disabled,0,32);
			
			for(int32_t j=0; j<MAXITEMS; j++)
			{
				if(DMaps[i].disableditems[j])
				{
					disabled[j/8] |= (1 << (j%8));
				}
			}
			
			if(!pfwrite(disabled,32,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",28);
			}
			
			if(!p_iputl(DMaps[i].flags,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",29);
			}
		if(!p_putc(DMaps[i].sideview,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",30);
			}
		if(!p_iputw(DMaps[i].script,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",31);
			}
		for ( int32_t q = 0; q < 8; q++ )
		{
		if(!p_iputl(DMaps[i].initD[q],f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",32);
		}
			
		}
		for ( int32_t q = 0; q < 8; q++ )
		{
			for ( int32_t w = 0; w < 65; w++ )
			{
			if (!p_putc(DMaps[i].initD_label[q][w],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",33);
			}
		}
		}
		}
}
void FFScript::read_dmaps(PACKFILE *f, int32_t vers_id)
{
	word dmap_count=count_dmaps();
  
		dmap_count=zc_min(dmap_count, 512);
		dmap_count=zc_min(dmap_count, MAXDMAPS-0);
		
		//finally...  section data
		if(!p_igetw(&dmap_count,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",5);
		}
		
		
		for(int32_t i=0; i<dmap_count; i++)
		{
			if(!p_getc(&DMaps[i].map,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",6);
			}
			
			if(!p_igetw(&DMaps[i].level,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",7);
			}
			
			if(!p_getc(&DMaps[i].xoff,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",8);
			}
			
			if(!p_getc(&DMaps[i].compass,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",9);
			}
			
			if(!p_igetw(&DMaps[i].color,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",10);
			}
			
			if(!p_getc(&DMaps[i].midi,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",11);
			}
			
			if(!p_getc(&DMaps[i].cont,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",12);
			}
			
			if(!p_getc(&DMaps[i].type,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",13);
			}
			
			for(int32_t j=0; j<8; j++)
			{
				if(!p_getc(&DMaps[i].grid[j],f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",14);
				}
			}
			
			//16
			if(!pfread((&DMaps[i].name),sizeof(DMaps[0].name),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",15);
			}
			
			if (!p_getwstr(&DMaps[i].title, f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",16);
			}
			
			if(!pfread((&DMaps[i].intro),sizeof(DMaps[0].intro),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",17);
			}
			
			if(!p_igetl(&DMaps[i].minimap_tile[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",18);
			}
			
			if(!p_getc(&DMaps[i].minimap_cset[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",19);
			}
			
			if(!p_igetl(&DMaps[i].minimap_tile[1],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",20);
			}
			
			if(!p_getc(&DMaps[i].minimap_cset[1],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",21);
			}
			
			if(!p_igetl(&DMaps[i].largemap_tile[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",22);
			}
			
			if(!p_getc(&DMaps[i].largemap_cset[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",23);
			}
			
			if(!p_igetl(&DMaps[i].largemap_tile[1],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",24);
			}
			
			if(!p_getc(&DMaps[i].largemap_cset[1],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",25);
			}
			
			if(!pfread((&DMaps[i].tmusic),sizeof(DMaps[0].tmusic),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",26);
			}
			
			if(!p_getc(&DMaps[i].tmusictrack,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",25);
			}
			
			if(!p_getc(&DMaps[i].active_subscreen,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",26);
			}
			
			if(!p_getc(&DMaps[i].passive_subscreen,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",27);
			}
			
			byte disabled[32];
			memset(disabled,0,32);
			
			for(int32_t j=0; j<MAXITEMS; j++)
			{
				if(&DMaps[i].disableditems[j])
				{
					disabled[j/8] |= (1 << (j%8));
				}
			}
			
			if(!pfread(disabled,32,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",28);
			}
			
			if(!p_igetl(&DMaps[i].flags,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",29);
			}
		if(!p_getc(&DMaps[i].sideview,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",30);
			}
		if(!p_igetw(&DMaps[i].script,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",31);
			}
		for ( int32_t q = 0; q < 8; q++ )
		{
		if(!p_igetl(&DMaps[i].initD[q],f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",32);
		}
			
		}
		for ( int32_t q = 0; q < 8; q++ )
		{
			for ( int32_t w = 0; w < 65; w++ )
			{
			if (!p_getc(&DMaps[i].initD_label[q][w],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",33);
			}
		}
		}
		}
}



void FFScript::read_combos(PACKFILE *f, int32_t version_id)
{
	
	word combos_used = 0;
	
		if(!p_igetw(&combos_used,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",5);
		}
		
		for(int32_t i=0; i<combos_used; i++)
		{
			if(!p_igetl(&combobuf[i].tile,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",6);
			}
			
			if(!p_getc(&combobuf[i].flip,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",7);
			}
			
			if(!p_getc(&combobuf[i].walk,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",8);
			}
			
			if(!p_getc(&combobuf[i].type,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",9);
			}
			
			if(!p_getc(&combobuf[i].csets,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",10);
			}
			
			if(!p_getc(&combobuf[i].frames,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",11);
			}
			
			if(!p_getc(&combobuf[i].speed,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",12);
			}
			
			if(!p_igetw(&combobuf[i].nextcombo,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",13);
			}
			
			if(!p_getc(&combobuf[i].nextcset,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",14);
			}
			
			if(!p_getc(&combobuf[i].flag,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",15);
			}
			
			if(!p_getc(&combobuf[i].skipanim,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",16);
			}
			
			if(!p_igetw(&combobuf[i].nexttimer,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",17);
			}
			
			if(!p_getc(&combobuf[i].skipanimy,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",18);
			}
			
			if(!p_getc(&combobuf[i].animflags,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",19);
			}
		
		for ( int32_t q = 0; q < NUM_COMBO_ATTRIBUTES; q++ )
		{
		if(!p_igetl(&combobuf[i].attributes[q],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",20);
		}
		}
		if(!p_igetl(&combobuf[i].usrflags,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",21);
		}	 
		if(combobuf[i].triggers.empty())
			combobuf[i].triggers.emplace_back();
		if(!p_getbitstr(&combobuf[i].triggers[0].trigger_flags,f))
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",22);
		
		if(!p_igetl(&combobuf[i].triggers[0].triggerlevel,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",23);
		}	
		for ( int32_t q = 0; q < 11; q++ ) 
		{
			if(!p_getc(&combobuf[i].label[q],f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",24);
			}
		}
		for ( int32_t q = 0; q < NUM_COMBO_ATTRIBUTES; q++ )
		{
			if(!p_getc(&combobuf[i].attribytes[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",25);
			}
		}
		if(!p_igetw(&combobuf[i].script,f))
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",26);
		for ( int32_t q = 0; q < 2; q++ )
		{
			if(!p_igetl(&combobuf[i].initd[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",27);
			}
		}
		if(!p_igetl(&combobuf[i].o_tile,f))
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",28);
		if(!p_getc(&combobuf[i].cur_frame,f))
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",29);
		if(!p_getc(&combobuf[i].aclk,f))
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",30);
		}

	combo_caches::refresh();
}

void FFScript::write_combos(PACKFILE *f, int32_t version_id)
{
	
	word combos_used = 0;
	
		//finally...  section data
		combos_used=count_combos()-0;
		combos_used=zc_min(combos_used, MAXCOMBOS);
		
		if(!p_iputw(combos_used,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",5);
		}
		
		for(int32_t i=0; i<combos_used; i++)
		{
			if(!p_iputl(combobuf[i].tile,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",6);
			}
			
			if(!p_putc(combobuf[i].flip,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",7);
			}
			
			if(!p_putc(combobuf[i].walk,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",8);
			}
			
			if(!p_putc(combobuf[i].type,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",9);
			}
			
			if(!p_putc(combobuf[i].csets,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",10);
			}
			
			if(!p_putc(combobuf[i].frames,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",11);
			}
			
			if(!p_putc(combobuf[i].speed,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",12);
			}
			
			if(!p_iputw(combobuf[i].nextcombo,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",13);
			}
			
			if(!p_putc(combobuf[i].nextcset,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",14);
			}
			
			if(!p_putc(combobuf[i].flag,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",15);
			}
			
			if(!p_putc(combobuf[i].skipanim,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",16);
			}
			
			if(!p_iputw(combobuf[i].nexttimer,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",17);
			}
			
			if(!p_putc(combobuf[i].skipanimy,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",18);
			}
			
			if(!p_putc(combobuf[i].animflags,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",19);
			}
		
		for ( int32_t q = 0; q < NUM_COMBO_ATTRIBUTES; q++ )
		{
		if(!p_iputl(combobuf[i].attributes[q],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",20);
		}
		}
		if(!p_iputl(combobuf[i].usrflags,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",21);
		}	 
		if(combobuf[i].triggers.empty())
			combobuf[i].triggers.emplace_back();
		if(!p_putbitstr(combobuf[i].triggers[0].trigger_flags,f));
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",22);
		
		if(!p_iputl(combobuf[i].triggers[0].triggerlevel,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",23);
		}	
		for ( int32_t q = 0; q < 11; q++ ) 
		{
			if(!p_putc(combobuf[i].label[q],f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",24);
			}
		}
		for ( int32_t q = 0; q < NUM_COMBO_ATTRIBUTES; q++ )
		{
			if(!p_putc(combobuf[i].attribytes[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",25);
			}
		}
		if(!p_iputw(combobuf[i].script,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",26);
		}
		for ( int32_t q = 0; q < 2; q++ )
		{
			if(!p_iputl(combobuf[i].initd[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",27);
			}
		}
		if(!p_iputl(combobuf[i].o_tile,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",28);
		}
		if(!p_putc(combobuf[i].cur_frame,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",29);
		}
		if(!p_putc(combobuf[i].aclk,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",30);
		}
			
		}
}
void FFScript::read_weaponsprtites(PACKFILE *f, int32_t vers_id)
{   
	for(int32_t i=0; i<MAXWPNS; i++)
	{
		word oldtile = 0;
		if(!p_igetw(&oldtile,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",6);
		}
			
		if(!p_getc(&wpnsbuf[i].misc,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",7);
		}
			
		if(!p_getc(&wpnsbuf[i].csets,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",8);
		}
			
		if(!p_getc(&wpnsbuf[i].frames,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",9);
		}
			
		if(!p_getc(&wpnsbuf[i].speed,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",10);
		}
			
		if(!p_getc(&wpnsbuf[i].type,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",11);
		}
		
		if(!p_igetw(&wpnsbuf[i].script,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",12);
		}

		if(!p_igetl(&wpnsbuf[i].tile,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",12);
		}
	}
}
void FFScript::write_weaponsprtites(PACKFILE *f, int32_t vers_id)
{   
	for(int32_t i=0; i<MAXWPNS; i++)
	{
		if(!p_iputw(wpnsbuf[i].tile,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",6);
		}
			
		if(!p_putc(wpnsbuf[i].misc,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",7);
		}
			
		if(!p_putc(wpnsbuf[i].csets,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",8);
		}
			
		if(!p_putc(wpnsbuf[i].frames,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",9);
		}
			
		if(!p_putc(wpnsbuf[i].speed,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",10);
		}
			
		if(!p_putc(wpnsbuf[i].type,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",11);
		}
		
		if(!p_iputw(wpnsbuf[i].script,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",12);
		}
		
		if(!p_iputl(wpnsbuf[i].tile,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",12);
		}
	}
}


void FFScript::read_enemies(PACKFILE *f, int32_t vers_id)
{
	if ( !f ) return;
	for(int32_t i=0; i<MAXGUYS; i++)
	{
			uint32_t flags1;
			uint32_t flags2;
			if (!p_igetl(&(flags1), f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d", 6);
			}
			if (!p_igetl(&(flags2), f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d", 7);;
			}
			guysbuf[i].flags = guy_flags(flags1) | guy_flags(uint64_t(flags2) << 32ULL);
			
			if(!p_igetl(&guysbuf[i].tile,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",8);
			}
			
			if(!p_getc(&guysbuf[i].width,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",9);
			}
			
			if(!p_getc(&guysbuf[i].height,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",10);
			}
			
			if(!p_igetl(&guysbuf[i].s_tile,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",11);
			}
			
			if(!p_getc(&guysbuf[i].s_width,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",12);
			}
			
			if(!p_getc(&guysbuf[i].s_height,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",13);
			}
			
			if(!p_igetl(&guysbuf[i].e_tile,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",14);
			}
			
			if(!p_getc(&guysbuf[i].e_width,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",15);
			}
			
			if(!p_getc(&guysbuf[i].e_height,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",16);
			}
			
			if(!p_igetw(&guysbuf[i].hp,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",17);
			}
			
			if(!p_igetw(&guysbuf[i].family,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",18);
			}
			
			if(!p_igetw(&guysbuf[i].cset,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",19);
			}
			
			if(!p_igetw(&guysbuf[i].anim,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",20);
			}
			
			if(!p_igetw(&guysbuf[i].e_anim,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",21);
			}
			
			if(!p_igetw(&guysbuf[i].frate,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",22);
			}
			
			if(!p_igetw(&guysbuf[i].e_frate,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",23);
			}
			
			if(!p_igetw(&guysbuf[i].dp,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",24);
			}
			
			if(!p_igetw(&guysbuf[i].wdp,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",25);
			}
			
			if(!p_igetw(&guysbuf[i].weapon,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",26);
			}
			
			if(!p_igetw(&guysbuf[i].rate,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",27);
			}
			
			if(!p_igetw(&guysbuf[i].hrate,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",28);
			}
			
			if(!p_igetw(&guysbuf[i].step,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",29);
			}
			
			if(!p_igetw(&guysbuf[i].homing,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",30);
			}
			
			if(!p_igetw(&guysbuf[i].grumble,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",31);
			}
			
			if(!p_igetw(&guysbuf[i].item_set,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",32);
			}
			//misc 1-10
			for (int q = 0; q < 10; ++q)
			{
				if (!p_igetl(&guysbuf[i].attributes[q], f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d", 33 + q);
				}
			}
			
			if(!p_igetw(&guysbuf[i].bgsfx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",43);
			}
			
			if(!p_igetw(&guysbuf[i].bosspal,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",44);
			}
			
			if(!p_igetw(&guysbuf[i].extend,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",45);
			}
			
			for(int32_t j=0; j < edefLAST; j++)
			{
			if(!p_getc(&guysbuf[i].defense[j],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",46);
			}
			}
			
			if(!p_getc(&guysbuf[i].hitsfx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",47);
			}
			
			if(!p_getc(&guysbuf[i].deadsfx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",48);
			}
			//misc 11-12
			for (int q = 0; q < 2; ++q)
			{
				if (!p_igetl(&guysbuf[i].attributes[10+q], f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d", 49 + q);
				}
			}
			
			//New 2.6 defences
			for(int32_t j=edefLAST; j < edefLAST255; j++)
			{
			if(!p_getc(&guysbuf[i].defense[j],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",51);
			}
			}
			
			//tilewidth, tileheight, hitwidth, hitheight, hitzheight, hitxofs, hityofs, hitzofs
			if(!p_igetl(&guysbuf[i].txsz,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",52);
			}
			if(!p_igetl(&guysbuf[i].tysz,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",53);
			}
			if(!p_igetl(&guysbuf[i].hxsz,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",54);
			}
			if(!p_igetl(&guysbuf[i].hysz,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",55);
			}
			if(!p_igetl(&guysbuf[i].hzsz,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",56);
			}
			// These are not fixed types, but ints, so they are safe to use here. 
			if(!p_igetl(&guysbuf[i].hxofs,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",57);
			}
			if(!p_igetl(&guysbuf[i].hyofs,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",58);
			}
			if(!p_igetl(&guysbuf[i].xofs,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",59);
			}
			if(!p_igetl(&guysbuf[i].yofs,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",60);
			}
			if(!p_igetl(&guysbuf[i].zofs,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",61);
			}
			if(!p_igetl(&guysbuf[i].wpnsprite,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",62);
			}
			if(!p_igetl(&guysbuf[i].SIZEflags,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",63);
			}
			if(!p_igetl(&guysbuf[i].frozentile,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",64);
			}
			if(!p_igetl(&guysbuf[i].frozencset,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",65);
			}
			if(!p_igetl(&guysbuf[i].frozenclock,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",66);
			}
			
			for ( int32_t q = 0; q < 10; q++ ) 
			{
			if(!p_igetw(&guysbuf[i].frozenmisc[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",67);
			}
			}
			if(!p_igetw(&guysbuf[i].firesfx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",68);
			}
			//misc 16->32
			for (int q = 0; q < 17; ++q)
			{
				if (!p_igetl(&guysbuf[i].attributes[15 + q], f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d", 69 + q);
				}
			}
			for ( int32_t q = 0; q < 32; q++ )
			{
				if(!p_igetl(&guysbuf[i].movement[q],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",86);
				}
			}
			for ( int32_t q = 0; q < 32; q++ )
			{
				if(!p_igetl(&guysbuf[i].new_weapon[q],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",87);
				}
			}
			if(!p_igetw(&guysbuf[i].script,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",88);
			}
			for ( int32_t q = 0; q < 8; q++ )
			{
			if(!p_igetl(&guysbuf[i].initD[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",89);
			}
			}
			if(!p_igetl(&guysbuf[i].editorflags,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",91);
			}
			//somehow forgot these in the older builds -Z
			for (int q = 0; q < 3; ++q)
			{
				if (!p_igetl(&guysbuf[i].attributes[12 + q], f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d", 92 + q);
				}
			}
			
			//Enemy Editor InitD[] labels
			for ( int32_t q = 0; q < 8; q++ )
			{
				for ( int32_t w = 0; w < 65; w++ )
				{
					if(!p_getc(&guysbuf[i].initD_label[q][w],f))
					{
						Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",95);
					} 
				}
				byte dummy;
				for ( int32_t w = 0; w < 65; w++ )
				{
					if(!p_getc(&dummy,f))
					{
						Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",96);
					} 
				}
			}
			if(!p_igetw(&guysbuf[i].weap_data.script,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",97);
			}
			//eweapon initD
			for ( int32_t q = 0; q < 8; q++ )
			{
			if(!p_igetl(&guysbuf[i].weap_data.initd[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",98);
			}
			}

			
	}
}

void FFScript::write_enemies(PACKFILE *f, int32_t vers_id)
{
	if ( !f ) return;
	for(int32_t i=0; i<MAXGUYS; i++)
	{
		uint32_t flags1 = uint32_t(guysbuf[i].flags);
		uint32_t flags2 = uint32_t(guysbuf[i].flags >> 32ULL);
		if (!p_iputl(flags1, f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d", 6);
		}
		if (!p_iputl(flags2, f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d", 7);
		}
		
		if(!p_iputl(guysbuf[i].tile,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",8);
		}
		
		if(!p_putc(guysbuf[i].width,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",9);
		}
		
		if(!p_putc(guysbuf[i].height,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",10);
		}
		
		if(!p_iputl(guysbuf[i].s_tile,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",11);
		}
		
		if(!p_putc(guysbuf[i].s_width,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",12);
		}
		
		if(!p_putc(guysbuf[i].s_height,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",13);
		}
		
		if(!p_iputl(guysbuf[i].e_tile,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",14);
		}
		
		if(!p_putc(guysbuf[i].e_width,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",15);
		}
		
		if(!p_putc(guysbuf[i].e_height,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",16);
		}
		
		if(!p_iputw(guysbuf[i].hp,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",17);
		}
		
		if(!p_iputw(guysbuf[i].family,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",18);
		}
		
		if(!p_iputw(guysbuf[i].cset,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",19);
		}
		
		if(!p_iputw(guysbuf[i].anim,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",20);
		}
		
		if(!p_iputw(guysbuf[i].e_anim,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",21);
		}
		
		if(!p_iputw(guysbuf[i].frate,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",22);
		}
		
		if(!p_iputw(guysbuf[i].e_frate,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",23);
		}
		
		if(!p_iputw(guysbuf[i].dp,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",24);
		}
		
		if(!p_iputw(guysbuf[i].wdp,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",25);
		}
		
		if(!p_iputw(guysbuf[i].weapon,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",26);
		}
		
		if(!p_iputw(guysbuf[i].rate,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",27);
		}
		
		if(!p_iputw(guysbuf[i].hrate,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",28);
		}
		
		if(!p_iputw(guysbuf[i].step,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",29);
		}
		
		if(!p_iputw(guysbuf[i].homing,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",30);
		}
		
		if(!p_iputw(guysbuf[i].grumble,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",31);
		}
		
		if(!p_iputw(guysbuf[i].item_set,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",32);
		}
		
		//misc 1-10
		for (int q = 0; q < 10; ++q)
		{
			if (!p_iputl(guysbuf[i].attributes[q], f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d", 33+q);
			}
		}
		
		if(!p_iputw(guysbuf[i].bgsfx,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",43);
		}
		
		if(!p_iputw(guysbuf[i].bosspal,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",44);
		}
		
		if(!p_iputw(guysbuf[i].extend,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",45);
		}
		
		for(int32_t j=0; j < edefLAST; j++)
		{
		if(!p_putc(guysbuf[i].defense[j],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",46);
		}
		}
		
		if(!p_putc(guysbuf[i].hitsfx,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",47);
		}
		
		if(!p_putc(guysbuf[i].deadsfx,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",48);
		}
		
		//misc 11-12
		for (int q = 0; q < 2; ++q)
		{
			if (!p_iputl(guysbuf[i].attributes[10+q], f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d", 49 + q);
			}
		}
		
		//New 2.6 defences
		for(int32_t j=edefLAST; j < edefLAST255; j++)
		{
		if(!p_putc(guysbuf[i].defense[j],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",51);
		}
		}
		
		//tilewidth, tileheight, hitwidth, hitheight, hitzheight, hitxofs, hityofs, hitzofs
		if(!p_iputl(guysbuf[i].txsz,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",52);
		}
		if(!p_iputl(guysbuf[i].tysz,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",53);
		}
		if(!p_iputl(guysbuf[i].hxsz,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",54);
		}
		if(!p_iputl(guysbuf[i].hysz,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",55);
		}
		if(!p_iputl(guysbuf[i].hzsz,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",56);
		}
		// These are not fixed types, but ints, so they are safe to use here. 
		if(!p_iputl(guysbuf[i].hxofs,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",57);
		}
		if(!p_iputl(guysbuf[i].hyofs,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",58);
		}
		if(!p_iputl(guysbuf[i].xofs,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",59);
		}
		if(!p_iputl(guysbuf[i].yofs,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",60);
		}
		if(!p_iputl(guysbuf[i].zofs,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",61);
		}
		if(!p_iputl(guysbuf[i].wpnsprite,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",62);
		}
		if(!p_iputl(guysbuf[i].SIZEflags,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",63);
		}
		if(!p_iputl(guysbuf[i].frozentile,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",64);
		}
		if(!p_iputl(guysbuf[i].frozencset,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",65);
		}
		if(!p_iputl(guysbuf[i].frozenclock,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",66);
		}
		
		for ( int32_t q = 0; q < 10; q++ ) 
		{
		if(!p_iputw(guysbuf[i].frozenmisc[q],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",67);
		}
		}

		if(!p_iputw(guysbuf[i].firesfx,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",68);
		}
		//misc 16->32
		for (int q=0; q < 17; ++q)
		{
			if (!p_iputl(guysbuf[i].attributes[15 + q], f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d", 69 + q);
			}
		}
		for ( int32_t q = 0; q < 32; q++ )
		{
			if(!p_iputl(guysbuf[i].movement[q],f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",86);
			}
		}
		for ( int32_t q = 0; q < 32; q++ )
		{
			if(!p_iputl(guysbuf[i].new_weapon[q],f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",87);
			}
		}
		if(!p_iputw(guysbuf[i].script,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",88);
		}
		for ( int32_t q = 0; q < 8; q++ )
		{
		if(!p_iputl(guysbuf[i].initD[q],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",89);
		}
		}
		if(!p_iputl(guysbuf[i].editorflags,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",91);
		}
		//misc 13-15
		for (int q = 0; q < 4; ++q)
		{
			if (!p_iputl(guysbuf[i].attributes[12 + q], f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d", 92 + q);
			}
		}
		
		//Enemy Editor InitD[] labels
		for ( int32_t q = 0; q < 8; q++ )
		{
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_putc(guysbuf[i].initD_label[q][w],f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",95);
				} 
			}
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_putc(0,f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",96);
				} 
			}
		}
		if(!p_iputw(guysbuf[i].weap_data.script,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",97);
		}
		//eweapon initD
		for ( int32_t q = 0; q < 8; q++ )
		{
		if(!p_iputl(guysbuf[i].weap_data.initd[q],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",98);
		}
		}
		
	}
}


void FFScript::write_items(PACKFILE *f, int32_t vers_id)
{
		for(int32_t i=0; i<MAXITEMS; i++)
		{
			if(!p_iputl(itemsbuf[i].tile,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",6);
			}
			
			if(!p_putc(itemsbuf[i].misc_flags,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",7);
			}
			
			if(!p_putc(itemsbuf[i].csets,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",8);
			}
			
			if(!p_putc(itemsbuf[i].frames,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",9);
			}
			
			if(!p_putc(itemsbuf[i].speed,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",10);
			}
			
			if(!p_putc(itemsbuf[i].delay,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",11);
			}
			
			if(!p_iputl(itemsbuf[i].ltm,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",12);
			}
			
			if(!p_iputl(itemsbuf[i].family,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",13);
			}
			
			if(!p_putc(itemsbuf[i].fam_type,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",14);
			}
			
			if(!p_iputl(itemsbuf[i].power,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",14);
			}
			
			if(!p_iputl(itemsbuf[i].flags,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",15);
			}
			
			if(!p_iputw(itemsbuf[i].script,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",16);
			}
			
			if(!p_putc(itemsbuf[i].count,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",17);
			}
			
			if(!p_iputw(itemsbuf[i].amount,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",18);
			}
			
			if(!p_iputw(itemsbuf[i].collect_script,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",19);
			}
			
			if(!p_iputw(itemsbuf[i].setmax,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",21);
			}
			
			if(!p_iputw(itemsbuf[i].max,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",22);
			}
			
			if(!p_putc(itemsbuf[i].playsound,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",23);
			}
			
			for(int32_t j=0; j<8; j++)
			{
				if(!p_iputl(itemsbuf[i].initiald[j],f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",24);
				}
			}
			
			if(!p_putc(itemsbuf[i].wpn,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",26);
			}
			
			if(!p_putc(itemsbuf[i].wpn2,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",27);
			}
			
			if(!p_putc(itemsbuf[i].wpn3,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",28);
			}
			
			if(!p_putc(itemsbuf[i].wpn4,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",29);
			}
			
			if(!p_putc(itemsbuf[i].wpn5,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",30);
			}
			
			if(!p_putc(itemsbuf[i].wpn6,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",31);
			}
			
			if(!p_putc(itemsbuf[i].wpn7,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",32);
			}
			
			if(!p_putc(itemsbuf[i].wpn8,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",33);
			}
			
			if(!p_putc(itemsbuf[i].wpn9,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",34);
			}
			
			if(!p_putc(itemsbuf[i].wpn10,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",35);
			}
			
			if(!p_putc(itemsbuf[i].pickup_hearts,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",36);
			}
			
			if(!p_iputl(itemsbuf[i].misc1,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",37);
			}
			
			if(!p_iputl(itemsbuf[i].misc2,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",38);
			}
			
			if(!p_putc(itemsbuf[i].cost_amount[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",39);
			}
			
			if(!p_iputl(itemsbuf[i].misc3,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",40);
			}
			
			if(!p_iputl(itemsbuf[i].misc4,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",41);
			}
			
			if(!p_iputl(itemsbuf[i].misc5,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",42);
			}
			
			if(!p_iputl(itemsbuf[i].misc6,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",43);
			}
			
			if(!p_iputl(itemsbuf[i].misc7,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",44);
			}
			
			if(!p_iputl(itemsbuf[i].misc8,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",45);
			}
			
			if(!p_iputl(itemsbuf[i].misc9,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",46);
			}
			
			if(!p_iputl(itemsbuf[i].misc10,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",47);
			}
			
			if(!p_putc(itemsbuf[i].usesound,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",48);
			}
			
			if(!p_putc(itemsbuf[i].usesound2,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",48);
			}
		
		//New itemdata vars -Z
		//! version 27
		
		if(!p_putc(itemsbuf[i].weap_data.imitate_weapon,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",49);
			}
		if(!p_putc(itemsbuf[i].weap_data.default_defense,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",50);
			}
		if(!p_iputl(itemsbuf[i].weaprange,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",51);
			}
		if(!p_iputl(itemsbuf[i].weapduration,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",52);
			}
		for ( int32_t q = 0; q < ITEM_MOVEMENT_PATTERNS; q++ ) {
			if(!p_iputl(itemsbuf[i].weap_pattern[q],f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",53);
			}
		}
		//version 28
		if(!p_iputl(itemsbuf[i].duplicates,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",54);
		}
		for ( int32_t q = 0; q < INITIAL_D; q++ )
		{
			if(!p_iputl(itemsbuf[i].weap_data.initd[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",55);
			}
		}

		if(!p_putc(itemsbuf[i].drawlayer,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",57);
		}


		if(!p_iputl(itemsbuf[i].hxofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",58);
		}
		if(!p_iputl(itemsbuf[i].hyofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",59);
		}
		if(!p_iputl(itemsbuf[i].hxsz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",60);
		}
		if(!p_iputl(itemsbuf[i].hysz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",61);
		}
		if(!p_iputl(itemsbuf[i].hzsz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",62);
		}
		if(!p_iputl(itemsbuf[i].xofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",63);
		}
		if(!p_iputl(itemsbuf[i].yofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",64);
		}
		if(!p_iputl(itemsbuf[i].weap_data.hxofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",65);
		}
		if(!p_iputl(itemsbuf[i].weap_data.hyofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",66);
		}
		if(!p_iputl(itemsbuf[i].weap_data.hxsz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",67);
		}
		if(!p_iputl(itemsbuf[i].weap_data.hysz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",68);
		}
		if(!p_iputl(itemsbuf[i].weap_data.hzsz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",69);
		}
		if(!p_iputl(itemsbuf[i].weap_data.xofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",70);
		}
		if(!p_iputl(itemsbuf[i].weap_data.yofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",71);
		}
		if(!p_iputw(itemsbuf[i].weap_data.script,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",72);
		}
		if(!p_iputl(itemsbuf[i].wpnsprite,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",73);
		}
		if(!p_iputl(itemsbuf[i].magiccosttimer[0],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",74);
		}
		if(!p_iputl(itemsbuf[i].overrideFLAGS,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",75);
		}
		if(!p_iputl(itemsbuf[i].tilew,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",76);
		}
		if(!p_iputl(itemsbuf[i].tileh,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",77);
		}
		if(!p_iputl(itemsbuf[i].weap_data.override_flags,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",78);
		}
		if(!p_iputl(itemsbuf[i].weap_data.tilew,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",79);
		}
		if(!p_iputl(itemsbuf[i].weap_data.tileh,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",80);
		}
		if(!p_iputl(itemsbuf[i].pickup,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",81);
		}
		if(!p_iputw(itemsbuf[i].pstring,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",82);
		}
		if(!p_iputw(itemsbuf[i].pickup_string_flags,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",83);
		}
		
		if(!p_putc(itemsbuf[i].cost_counter[0],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",84);
		}
		
		//InitD[] labels
		for ( int32_t q = 0; q < 8; q++ )
		{
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_putc(itemsbuf[i].initD_label[q][w],f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",85);
				} 
			}
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_putc(0,f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",86);
				} 
			}
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_putc(itemsbuf[i].sprite_initD_label[q][w],f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",87);
				} 
			}
			if(!p_iputl(itemsbuf[i].sprite_initiald[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",88);
			} 
		}

		if(!p_iputw(itemsbuf[i].sprite_script,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",90);
		} 
		
		
		}
}

void FFScript::read_items(PACKFILE *f, int32_t vers_id)
{
		for(int32_t i=0; i<MAXITEMS; i++)
		{
			if(!p_igetl(&itemsbuf[i].tile,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",6);
			}
			
			if(!p_getc(&itemsbuf[i].misc_flags,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",7);
			}
			
			if(!p_getc(&itemsbuf[i].csets,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",8);
			}
			
			if(!p_getc(&itemsbuf[i].frames,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",9);
			}
			
			if(!p_getc(&itemsbuf[i].speed,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",10);
			}
			
			if(!p_getc(&itemsbuf[i].delay,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",11);
			}
			
			if(!p_igetl(&itemsbuf[i].ltm,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",12);
			}
			
			if(!p_igetl(&itemsbuf[i].family,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",13);
			}
			
			if(!p_getc(&itemsbuf[i].fam_type,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",14);
			}
			
			if(!p_igetl(&itemsbuf[i].power,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",14);
			}
			
			if(!p_igetl(&itemsbuf[i].flags,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",15);
			}
			
			if(!p_igetw(&itemsbuf[i].script,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",16);
			}
			
			if(!p_getc(&itemsbuf[i].count,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",17);
			}
			
			if(!p_igetw(&itemsbuf[i].amount,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",18);
			}
			
			if(!p_igetw(&itemsbuf[i].collect_script,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",19);
			}
			
			if(!p_igetw(&itemsbuf[i].setmax,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",21);
			}
			
			if(!p_igetw(&itemsbuf[i].max,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",22);
			}
			
			if(!p_getc(&itemsbuf[i].playsound,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",23);
			}
			
			for(int32_t j=0; j<8; j++)
			{
				if(!p_igetl(&itemsbuf[i].initiald[j],f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",24);
				}
			}
			
			if(!p_getc(&itemsbuf[i].wpn,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",26);
			}
			
			if(!p_getc(&itemsbuf[i].wpn2,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",27);
			}
			
			if(!p_getc(&itemsbuf[i].wpn3,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",28);
			}
			
			if(!p_getc(&itemsbuf[i].wpn4,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",29);
			}
			
			if(!p_getc(&itemsbuf[i].wpn5,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",30);
			}
			
			if(!p_getc(&itemsbuf[i].wpn6,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",31);
			}
			
			if(!p_getc(&itemsbuf[i].wpn7,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",32);
			}
			
			if(!p_getc(&itemsbuf[i].wpn8,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",33);
			}
			
			if(!p_getc(&itemsbuf[i].wpn9,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",34);
			}
			
			if(!p_getc(&itemsbuf[i].wpn10,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",35);
			}
			
			if(!p_getc(&itemsbuf[i].pickup_hearts,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",36);
			}
			
			if(!p_igetl(&itemsbuf[i].misc1,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",37);
			}
			
			if(!p_igetl(&itemsbuf[i].misc2,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",38);
			}
			
			if(!p_getc(&itemsbuf[i].cost_amount[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",39);
			}
			
			if(!p_igetl(&itemsbuf[i].misc3,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",40);
			}
			
			if(!p_igetl(&itemsbuf[i].misc4,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",41);
			}
			
			if(!p_igetl(&itemsbuf[i].misc5,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",42);
			}
			
			if(!p_igetl(&itemsbuf[i].misc6,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",43);
			}
			
			if(!p_igetl(&itemsbuf[i].misc7,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",44);
			}
			
			if(!p_igetl(&itemsbuf[i].misc8,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",45);
			}
			
			if(!p_igetl(&itemsbuf[i].misc9,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",46);
			}
			
			if(!p_igetl(&itemsbuf[i].misc10,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",47);
			}
			
			if(!p_getc(&itemsbuf[i].usesound,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",48);
			}
			
			if(!p_getc(&itemsbuf[i].usesound2,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",48);
			}
		
		//New itemdata vars -Z
		//! version 27
		
		if(!p_getc(&itemsbuf[i].weap_data.imitate_weapon,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",49);
			}
		if(!p_getc(&itemsbuf[i].weap_data.default_defense,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",50);
			}
		if(!p_igetl(&itemsbuf[i].weaprange,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",51);
			}
		if(!p_igetl(&itemsbuf[i].weapduration,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",52);
			}
		for ( int32_t q = 0; q < ITEM_MOVEMENT_PATTERNS; q++ ) {
			if(!p_igetl(&itemsbuf[i].weap_pattern[q],f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",53);
			}
		}
		//version 28
		if(!p_igetl(&itemsbuf[i].duplicates,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",54);
		}
		for ( int32_t q = 0; q < INITIAL_D; q++ )
		{
			if(!p_igetl(&itemsbuf[i].weap_data.initd[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",55);
			}
		}

		if(!p_getc(&itemsbuf[i].drawlayer,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",57);
		}


		if(!p_igetl(&itemsbuf[i].hxofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",58);
		}
		if(!p_igetl(&itemsbuf[i].hyofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",59);
		}
		if(!p_igetl(&itemsbuf[i].hxsz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",60);
		}
		if(!p_igetl(&itemsbuf[i].hysz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",61);
		}
		if(!p_igetl(&itemsbuf[i].hzsz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",62);
		}
		if(!p_igetl(&itemsbuf[i].xofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",63);
		}
		if(!p_igetl(&itemsbuf[i].yofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",64);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.hxofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",65);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.hyofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",66);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.hxsz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",67);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.hysz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",68);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.hzsz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",69);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.xofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",70);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.yofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",71);
		}
		if(!p_igetw(&itemsbuf[i].weap_data.script,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",72);
		}
		if(!p_igetl(&itemsbuf[i].wpnsprite,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",73);
		}
		if(!p_igetl(&itemsbuf[i].magiccosttimer[0],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",74);
		}
		if(!p_igetl(&itemsbuf[i].overrideFLAGS,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",75);
		}
		if(!p_igetl(&itemsbuf[i].tilew,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",76);
		}
		if(!p_igetl(&itemsbuf[i].tileh,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",77);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.override_flags,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",78);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.tilew,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",79);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.tileh,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",80);
		}
		if(!p_igetl(&itemsbuf[i].pickup,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",81);
		}
		if(!p_igetw(&itemsbuf[i].pstring,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",82);
		}
		if(!p_igetw(&itemsbuf[i].pickup_string_flags,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",83);
		}
		
		if(!p_getc(&itemsbuf[i].cost_counter[0],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",84);
		}
		
		//InitD[] labels
		for ( int32_t q = 0; q < 8; q++ )
		{
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_getc(&itemsbuf[i].initD_label[q][w],f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",85);
				} 
			}
			byte dummy;
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_getc(&dummy,f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",86);
				} 
			}
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_getc(&itemsbuf[i].sprite_initD_label[q][w],f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",87);
				} 
			}
			if(!p_igetl(&itemsbuf[i].sprite_initiald[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",88);
			} 
		}

		if(!p_igetw(&itemsbuf[i].sprite_script,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",90);
		} 
		
		
		}
}
	
void FFScript::write_mapscreens(PACKFILE *f,int32_t vers_id)
{
	for(int32_t i=0; i<map_count && i<MAXMAPS; i++)
		{
		for(int32_t j=0; j<MAPSCRS; j++)
		{
			mapscr *m = &TheMaps[i*MAPSCRS+j];
			
			if(!p_putc(m->valid,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->guy,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			{
			if(!p_iputw(m->str,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			if(!p_putc(m->room,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->item,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->hasitem, f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_putc(m->tilewarptype[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			if(!p_iputw(m->door_combo_set,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_putc(m->warpreturnx[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_putc(m->warpreturny[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			if(!p_iputw(m->warpreturnc,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->stairx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->stairy,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->itemx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->itemy,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_iputw(m->color,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags11,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_putc(m->door[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_iputw(m->tilewarpdmap[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_putc(m->tilewarpscr[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			if(!p_putc(m->tilewarpoverlayflags,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->exitdir,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			for(int32_t k=0; k<10; k++)
			{
			{
				if(!p_iputw(m->enemy[k],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
			}
			}
			
			if(!p_putc(m->pattern,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_putc(m->sidewarptype[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			if(!p_putc(m->sidewarpoverlayflags,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->warparrivalx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->warparrivaly,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_putc(m->path[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_putc(m->sidewarpscr[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_iputw(m->sidewarpdmap[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			if(!p_putc(m->sidewarpindex,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_iputw(m->undercombo,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->undercset,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_iputw(m->catchall,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags2,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags3,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags4,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags5,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_iputw(m->noreset,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_iputl(m->nocarry,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags6,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags7,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags8,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags9,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags10,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->csensitive,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->oceansfx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->bosssfx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->secretsfx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->holdupsfx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			for(int32_t k=0; k<6; k++)
			{
			if(!p_putc(m->layermap[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<6; k++)
			{
			if(!p_putc(m->layerscreen[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<6; k++)
			{
			if(!p_putc(m->layeropacity[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			if(!p_iputw(m->timedwarptics,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->nextmap,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->nextscr,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			for(int32_t k=0; k<128; k++)
			{
			if(!p_iputw(m->secretcombo[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<128; k++)
			{
			if(!p_putc(m->secretcset[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<128; k++)
			{
			if(!p_putc(m->secretflag[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<176; k++)
			{
			try
			{
				if(!p_iputw(m->data[k],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
			}
			catch(std::out_of_range& )
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<176; k++)
			{
			try
			{
				if(!p_putc(m->sflag[k], f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
			}
			catch(std::out_of_range& )
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<176; k++)
			{
			try
			{
				if(!p_putc(m->cset[k],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
			}
			catch(std::out_of_range& )
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			if(!p_iputw(m->screen_midi,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->lens_layer,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			m->ensureFFC(32);
			for(int32_t k=0; k<32; k++)
			{
				ffcdata& ffc = m->ffcs[k];
				if(!p_iputw(ffc.data,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_putc(ffc.cset,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputw(ffc.delay,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputzf(ffc.x,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputzf(ffc.y,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputzf(ffc.vx,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputzf(ffc.vy,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputzf(ffc.ax,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputzf(ffc.ay,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_putc(ffc.link,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.hit_width,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.hit_height,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_putc(ffc.txsz,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_putc(ffc.tysz,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.flags,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputw(ffc.script,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.initd[0],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.initd[1],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.initd[2],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.initd[3],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.initd[4],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.initd[5],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.initd[6],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.initd[7],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
			
			}
			
			if(!p_iputw(m->script,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			} 
			for ( int32_t q = 0; q < 8; q++ )
			{
			if(!p_iputl(m->screeninitd[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			} 
				
			}
			if(!p_putc(m->preloadscript,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->hidelayers,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}  
			if(!p_putc(m->hidescriptlayers,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}    
				
			
		} //end mapscr for loop
	}
}
void FFScript::read_mapscreens(PACKFILE *f,int32_t vers_id)
{
	for(int32_t i=0; i<map_count && i<MAXMAPS; i++)
		{
		for(int32_t j=0; j<MAPSCRS; j++)
		{
			mapscr *m = &TheMaps[i*MAPSCRS+j];
			
			if(!p_getc(&(m->valid),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->guy),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			{
			if(!p_igetw(&(m->str),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			if(!p_getc(&(m->room),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->item),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->hasitem), f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_getc(&(m->tilewarptype[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			if(!p_igetw(&(m->door_combo_set),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_getc(&(m->warpreturnx[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_getc(&(m->warpreturny[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			if(!p_igetw(&(m->warpreturnc),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->stairx),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->stairy),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->itemx),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->itemy),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_igetw(&(m->color),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags11),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_getc(&(m->door[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_igetw(&(m->tilewarpdmap[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_getc(&(m->tilewarpscr[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			if(!p_getc(&(m->tilewarpoverlayflags),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->exitdir),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			for(int32_t k=0; k<10; k++)
			{
			{
				if(!p_igetw(&(m->enemy[k]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
			}
			}
			
			if(!p_getc(&(m->pattern),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_getc(&(m->sidewarptype[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			if(!p_getc(&(m->sidewarpoverlayflags),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->warparrivalx),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->warparrivaly),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_getc(&(m->path[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_getc(&(m->sidewarpscr[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_igetw(&(m->sidewarpdmap[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			if(!p_getc(&(m->sidewarpindex),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_igetw(&(m->undercombo),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->undercset),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_igetw(&(m->catchall),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags2),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags3),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags4),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags5),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_igetw(&(m->noreset),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_igetl(&(m->nocarry),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags6),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags7),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags8),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags9),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags10),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->csensitive),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->oceansfx),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->bosssfx),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->secretsfx),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->holdupsfx),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			for(int32_t k=0; k<6; k++)
			{
			if(!p_getc(&(m->layermap[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<6; k++)
			{
			if(!p_getc(&(m->layerscreen[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<6; k++)
			{
			if(!p_getc(&(m->layeropacity[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			if(!p_igetw(&(m->timedwarptics),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->nextmap),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->nextscr),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			for(int32_t k=0; k<128; k++)
			{
			if(!p_igetw(&(m->secretcombo[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<128; k++)
			{
			if(!p_getc(&(m->secretcset[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<128; k++)
			{
			if(!p_getc(&(m->secretflag[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<176; k++)
			{
			try
			{
				if(!p_igetw(&(m->data[k]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
			}
			catch(std::out_of_range& )
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<176; k++)
			{
			try
			{
				if(!p_getc(&(m->sflag[k]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
			}
			catch(std::out_of_range& )
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<176; k++)
			{
			try
			{
				if(!p_getc(&(m->cset[k]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
			}
			catch(std::out_of_range& )
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			if(!p_igetw(&(m->screen_midi),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->lens_layer),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}

			m->ensureFFC(32);
			word tempw;
			for(int32_t k=0; k<32; k++)
			{
				ffcdata& ffc = m->ffcs[k];
				if(!p_igetw(&tempw,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				zc_ffc_set(ffc, tempw);
				
				if(!p_getc(&(ffc.cset),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetw(&(ffc.delay),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetzf(&(ffc.x),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetzf(&(ffc.y),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetzf(&(ffc.vx),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetzf(&(ffc.vy),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetzf(&(ffc.ax),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetzf(&(ffc.ay),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_getc(&(ffc.link),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.hit_width),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.hit_height),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_getc(&(ffc.txsz),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_getc(&(ffc.tysz),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.flags),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetw(&(ffc.script),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.initd[0]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.initd[1]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.initd[2]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.initd[3]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.initd[4]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.initd[5]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.initd[6]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.initd[7]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
			
			}
			
			if(!p_igetw(&(m->script),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			} 
			for ( int32_t q = 0; q < 8; q++ )
			{
			if(!p_igetl(&(m->screeninitd[q]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			} 
				
			}
			if(!p_getc(&(m->preloadscript),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if ( vers_id >= 2 )
			{
			if(!p_getc(&(m->hidelayers),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}  
			if(!p_getc(&(m->hidescriptlayers),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}    
				
			}
			
			
		}//end mapscr all for loop
		
	}
}
/*
void FFScript::write_maps(PACKFILE *f, int32_t vers_id)
{
		for(int32_t i=0; i<map_count && i<MAXMAPS; i++)
		{
		for(int32_t j=0; j<MAPSCRS; j++)
		{
			if ( !(FFCore.write_mapscreen(f,i,j,vers_id)) )
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODE: %d",i*j);
			}
		}
		}
}

void FFScript::read_maps(PACKFILE *f, int32_t vers_id)
{
		for(int32_t i=0; i<map_count && i<MAXMAPS; i++)
		{
		for(int32_t j=0; j<MAPSCRS; j++)
		{
			if ( !(FFCore.read_mapscreen(f,i,j,vers_id)) )
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE: %d",i*j);
			}
		}
		}
}
*/


int32_t FFScript::getHeroOTile(int32_t index1, int32_t index2)
{
	{
		herospritetype lst = (herospritetype)index1;
		int32_t dir = index2;
		int32_t the_ret = 0;
		switch(lst)
		{
		case LSprwalkspr: the_ret = walkspr[dir][0]; break;
			case LSprstabspr: the_ret = stabspr[dir][0]; break;
			case LSprslashspr: the_ret = slashspr[dir][0]; break;
			case LSprrevslashspr: the_ret = revslashspr[dir][0]; break;
			case LSprfloatspr: the_ret = floatspr[dir][0]; break;
			case LSprswimspr: the_ret = swimspr[dir][0]; break;
			case LSprdivespr: the_ret = divespr[dir][0]; break;
			case LSprdrownspr: the_ret = drowningspr[dir][0]; break;
			case LSprsidedrownspr: the_ret = sidedrowningspr[dir][0]; break;
			case LSprlavadrownspr: the_ret = drowning_lavaspr[dir][0]; break;
			case LSprsideswimspr: the_ret = sideswimspr[dir][0]; break;
			case LSprsideswimslashspr: the_ret = sideswimslashspr[dir][0]; break;
			case LSprsideswimstabspr: the_ret = sideswimstabspr[dir][0]; break;
			case LSprsideswimpoundspr: the_ret = sideswimpoundspr[dir][0]; break;
			case LSprsideswimchargespr: the_ret = sideswimchargespr[dir][0]; break;
			case LSprpoundspr: the_ret = poundspr[dir][0]; break;
			case LSprjumpspr: the_ret = jumpspr[dir][0]; break;
			case LSprchargespr: the_ret = chargespr[dir][0]; break;
			case LSprcastingspr: the_ret = castingspr[0]; break;
			case LSprsideswimcastingspr: the_ret = sideswimcastingspr[0]; break;
			case LSprholdspr1: the_ret = holdspr[0][0][0]; break;
			case LSprholdspr2:  the_ret = holdspr[0][1][0]; break;
			case LSprholdsprw1: the_ret = holdspr[1][0][0]; break;
			case LSprholdsprw2: the_ret = holdspr[1][1][0]; break;
			case LSprholdsprSw1: the_ret = sideswimholdspr[0][0]; break;
			case LSprholdsprSw2: the_ret = sideswimholdspr[1][0]; break;
			default: the_ret = 0;
		}
	
	return the_ret*10000;
	}
}

defWpnSprite FFScript::getDefWeaponSprite(int32_t wpnid)
{
	switch(wpnid)
	{
		case wNone: return ws_0;
		case wSword: return ws_0;
		case wBeam: return wsBeam;
		case wBrang : return wsBrang;
		case wBomb: return wsBomb;
		case wSBomb: return wsSBomb;
		case wLitBomb: return wsBombblast;
		case wLitSBomb: return wsBombblast;
		case wArrow: return wsArrow;
		case wRefArrow: return wsArrow;
		case wFire: return wsFire;
		case wRefFire: return wsFire;
		case wRefFire2: return wsFire;
		case wWhistle: return wsUnused45;
		case wBait: return wsBait;
		case wWand: return wsWandHandle;
		case wMagic: return wsMagic;
		case wCatching: return wsUnused45;
		case wWind: return wsWind;
		case wRefMagic: return wsRefMagic;
		case wRefFireball: return wsRefFireball;
		case wRefRock: return wsRock;
		case wHammer: return wsHammer;
		case wHookshot: return wsHookshotHead;
		case wHSHandle: return wsHookshotHandle;
		case wHSChain: return wsHookshotChainH;
		case wSSparkle: return wsSilverSparkle;
		case wFSparkle: return wsGoldSparkle;
		case wSmack: return wsHammerSmack;
		case wPhantom: return wsUnused45;
		case wCByrna: return wsByrnaCane;
		case wRefBeam: return wsRefBeam;
		case wStomp: return wsUnused45;
		case lwMax: return wsUnused45;
		case wScript1: 
		case wScript2:
		case wScript3:
		case wScript4:
		case wScript5:
		case wScript6:
		case wScript7:
		case wScript8:
		case wScript9:
		case wScript10: return ws_0;
		case wIce: return wsIce; //new
		case wFlame: return wsEFire2; //new
		//not implemented; t/b/a
		case wSound:
		case wThrown: 
		case wPot:
		case wLit:
		case wBombos:
		case wEther:
		case wQuake:
		case wSword180:
		case wSwordLA:  return wsUnused45;
	
		case ewFireball: return wsFireball2;
		case ewArrow: return wsEArrow;
		case ewBrang: return wsBrang;
		case ewSword: return wsEBeam;
		case ewRock: return wsRock;
		case ewMagic: return wsEMagic;
		case ewBomb: return wsEBomb;
		case ewSBomb: return wsESbomb;
		case ewLitBomb: return wsEBombblast;
		case ewLitSBomb: return wsESbombblast;
		case ewFireTrail: return wsEFiretrail;
		case ewFlame: return wsEFire;
		case ewWind: return wsEWind;
		case ewFlame2: return wsEFire2;
		case ewFlame2Trail: return wsEFiretrail2;
		case ewIce: return wsIce;
		case ewFireball2: return wsFireball2;
		default: return wsUnused45;
	}
};

void FFScript::do_loadlweapon_by_script_uid(const bool v)
{
	int32_t uid = SH::get_arg(sarg1, v);
	if (ResolveLWeapon_checkSpriteList(uid))
		ri->lwpn = uid;
	else
	{
		ri->lwpn = 0;
	}
}

void FFScript::do_loadeweapon_by_script_uid(const bool v)
{
	int32_t uid = SH::get_arg(sarg1, v);
	if (ResolveEWeapon_checkSpriteList(uid))
		ri->ewpn = uid;
	else
	{
		ri->ewpn = 0;
	}
}


void FFScript::do_loadnpc_by_script_uid(const bool v)
{
	int32_t uid = SH::get_arg(sarg1, v);
	if (ResolveSprite<enemy>(uid, "enemy"))
		ri->guyref = uid;
	else
	{
		ri->guyref = 0;
	}
}

//Combo Scripts

void FFScript::clear_combo_scripts()
{
	combo_id_cache.clear();
	combo_id_cache.resize(region_num_rpos * 7);
	std::fill(combo_id_cache.begin(), combo_id_cache.end(), -1);
	FFCore.deallocateAllScriptOwnedOfType(ScriptType::Combo);
	FFCore.clear_script_engine_data_of_type(ScriptType::Combo);
}

void FFScript::clear_combo_script(const rpos_handle_t& rpos_handle)
{
	int32_t index = get_combopos_ref(rpos_handle);
	combo_id_cache[index] = -1;
	combopos_modified = index;
	clear_script_engine_data(ScriptType::Combo, index);
}

int32_t FFScript::combo_script_engine(const bool preload, const bool waitdraw)
{
	bool enabled[7];
	for (int32_t q = 0; q < 7; ++q)
	{
		enabled[q] = get_qr(qr_COMBOSCRIPTS_LAYER_0 + q);
	}

	auto& combo_cache = combo_caches::script;

	///non-scripted effects
	for_every_rpos([&](const rpos_handle_t& rpos_handle) {
		if (!enabled[rpos_handle.layer])
			return;

		int32_t combopos_ref = get_combopos_ref(rpos_handle);
		word cid = rpos_handle.data();
		if(combo_id_cache[combopos_ref] != cid)
		{
			combopos_modified = combopos_ref;
			combo_id_cache[combopos_ref] = cid;
			clear_script_engine_data(ScriptType::Combo, combopos_ref);
		}
		
		auto script = combo_cache.minis[cid].script;
		if (script)
		{
			auto& data = get_script_engine_data(ScriptType::Combo, combopos_ref);
			if (data.doscript)
			{
				if (waitdraw && !data.waitdraw) return; //waitdraw not set

				ZScriptVersion::RunScript(ScriptType::Combo, script, combopos_ref);
				if (waitdraw) data.waitdraw = true;
			}
		}
	});

	return 1;
}

int32_t FFScript::Distance(double x1, double y1, double x2, double y2) 
{
	double x = (x1-x2);
	double y = (y1-y2);
	double sum = (x*x)+(y*y);
	//if(((int32_t)sum) < 0)
	//{
	//	Z_scripterrlog("Distance() attempted to calculate square root of %ld!\n", ((int32_t)sum));
	//	return -10000;;
	//}
	sum *= 1000000.0;
	double total = sqrt(sum)*10;
	return int32_t(total);
}

int32_t FFScript::Distance(double x1, double y1, double x2, double y2, int32_t scale) 
{
	double x3 = x1+(x2-x1)/scale;
	double y3 = y1+(y2-y1)/scale;
	//double sum = (x*x)+(y*y);
	//if(((int32_t)sum) < 0)
	//{
	//	Z_scripterrlog("Distance() attempted to calculate square root of %ld!\n", ((int32_t)sum));
	//	return -10000;
	//}
	//sum *= 1000000.0;
	//double total = sqrt(sum)*10;
	//return int32_t(total*scale);
	return (FFCore.Distance(x1, y1, x3, y3)*scale);
}

int32_t FFScript::LongDistance(double x1, double y1, double x2, double y2) 
{
	double x = (x1-x2);
	double y = (y1-y2);
	double sum = (x*x)+(y*y);
	//if(((int32_t)sum) < 0)
	//{
	//	Z_scripterrlog("Distance() attempted to calculate square root of %ld!\n", ((int32_t)sum));
	//	return -10000;;
	//}
	double total = sqrt(sum);
	return int32_t(total);
}

int32_t FFScript::LongDistance(double x1, double y1, double x2, double y2, int32_t scale) 
{
	double x3 = x1+(x2-x1)/scale;
	double y3 = y1+(y2-y1)/scale;
	//double sum = (x*x)+(y*y);
	//if(((int32_t)sum) < 0)
	//{
	//	Z_scripterrlog("Distance() attempted to calculate square root of %ld!\n", ((int32_t)sum));
	//	return -10000;
	//}
	//sum *= 1000000.0;
	//double total = sqrt(sum)*10;
	//return int32_t(total*scale);
	return (FFCore.LongDistance(x1, y1, x3, y3)*scale);
}

bool command_is_wait(int command)
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

bool command_is_goto(int command)
{
	// GOTOR/return ops left out on purpose.
	switch (command)
	{
	case GOTO:
	case GOTOCMP:
	case GOTOLESS:
	case GOTOMORE:
	case GOTOTRUE:
	case GOTOFALSE:
		return true;
	}
	return false;
}

bool command_uses_comparison_result(int command)
{
	switch (command)
	{
	case GOTOTRUE:
	case GOTOFALSE:
	case GOTOMORE:
	case GOTOLESS:
	case GOTOCMP:
	case SETCMP:
	case SETTRUE:
	case SETTRUEI:
	case SETFALSE:
	case SETFALSEI:
	case SETMOREI:
	case SETLESSI:
	case SETMORE:
	case SETLESS:
	case STACKWRITEATVV_IF:
		return true;
	}
	return false;
}

bool command_writes_comparison_result(int command)
{
	switch (command)
	{
	case SETCMP:
	case SETTRUE:
	case SETTRUEI:
	case SETFALSE:
	case SETFALSEI:
	case SETMOREI:
	case SETLESSI:
	case SETMORE:
	case SETLESS:
		return true;
	}
	return false;
}

int command_to_cmp(int command, int arg)
{
	switch (command)
	{
		case SETCMP:
		case GOTOCMP:
			return arg;
		
		case GOTOTRUE:
			return CMP_EQ;
		case GOTOFALSE:
			return CMP_NE;
		case GOTOMORE:
			return CMP_GE;
		case GOTOLESS:
			return get_qr(qr_GOTOLESSNOTEQUAL) ? CMP_LE : CMP_LT;

		case SETTRUE:
			return CMP_EQ;
		case SETFALSE:
			return CMP_NE;
		case SETMORE:
			return CMP_GE;
		case SETLESS:
			return CMP_LE;
		
		case SETTRUEI:
			return CMP_SETI|CMP_EQ;
		case SETFALSEI:
			return CMP_SETI|CMP_NE;
		case SETMOREI:
			return CMP_SETI|CMP_GE;
		case SETLESSI:
			return CMP_SETI|CMP_LE;
	}

	ASSERT(false);
	return 0;
}

bool command_could_return_not_ok(int command)
{
	switch (command)
	{
	case 0xFFFF:
	case EWPNDEL:
	case GAMECONTINUE:
	case GAMEEND:
	case GAMEEXIT:
	case GAMERELOAD:
	case GAMESAVECONTINUE:
	case GAMESAVEQUIT:
	case ITEMDEL:
	case LWPNDEL:
	case NPCKICKBUCKET:
		return true;
	}
	return false;
}

bool command_is_pure(int command)
{
	switch (command)
	{
		case ABS:
		case ADDR:
		case ADDV:
		case ANDR:
		case ANDR32:
		case ANDV:
		case ANDV32:
		case ARCCOSR:
		case ARCCOSV:
		case ARCSINR:
		case ARCSINV:
		case BITNOT:
		case BITNOT32:
		case CASTBOOLF:
		case CEILING:
		case COMPAREV2:
		case COSR:
		case COSV:
		case DIVV2:
		case FACTORIAL:
		case FLOOR:
		case IPOWERR:
		case IPOWERV:
		case ISALLOCATEDBITMAP:
		case LOAD:
		case LOADD:
		case LOADI:
		case LOG10:
		case LOGE:
		case LSHIFTR:
		case LSHIFTR32:
		case LSHIFTV:
		case LSHIFTV32:
		case MAXR:
		case MAXV:
		case MAXVARG:
		case MINR:
		case MINV:
		case MINVARG:
		case MODR:
		case MODV:
		case MODV2:
		case NANDR:
		case NANDV:
		case NORR:
		case NORV:
		case NOT:
		case ORR:
		case ORR32:
		case ORV:
		case ORV32:
		case PEEK:
		case PEEKATV:
		case POWERR:
		case POWERV:
		case ROUND:
		case ROUNDAWAY:
		case RSHIFTR:
		case RSHIFTR32:
		case RSHIFTV:
		case RSHIFTV32:
		case SETCMP:
		case SETFALSE:
		case SETFALSEI:
		case SETLESS:
		case SETLESSI:
		case SETMORE:
		case SETMOREI:
		case SETR:
		case SETTRUE:
		case SETTRUEI:
		case SETV:
		case SINR:
		case SINV:
		case SUBR:
		case SUBV:
		case SUBV2:
		case TANR:
		case TANV:
		case TOBYTE:
		case TOINTEGER:
		case TOSHORT:
		case TOSIGNEDBYTE:
		case TOWORD:
		case TRUNCATE:
		case XNORR:
		case XNORV:
		case XORR:
		case XORR32:
		case XORV:
		case XORV32:
			return true;
	}

	return false;
}

int32_t get_combopos_ref(const rpos_handle_t& rpos_handle)
{
	return rpos_handle.layer * region_num_rpos + (int)rpos_handle.rpos;
}

int32_t get_combopos_ref(rpos_t rpos, int32_t layer)
{
	return layer * region_num_rpos + (int)rpos;
}

rpos_t combopos_ref_to_rpos(int32_t combopos_ref)
{
	return (rpos_t)(combopos_ref % region_num_rpos);
}

int32_t combopos_ref_to_layer(int32_t combopos_ref)
{
	return combopos_ref / region_num_rpos;
}

ScriptEngineData& get_ffc_script_engine_data(int index)
{
	return get_script_engine_data(ScriptType::FFC, index);
}

ScriptEngineData& get_item_script_engine_data(int index)
{
	return get_script_engine_data(ScriptType::Item, index);
}
