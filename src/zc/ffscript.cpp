#include <cstdint>
#include <cctype>
#include <deque>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <sstream>
#include <math.h>
#include <cstdio>
#include <algorithm>
#include <ranges>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include "base/check.h"
#include "base/expected.h"
#include "core/handles.h"
#include "base/general.h"
#include "core/mapscr.h"
#include "core/qrs.h"
#include "core/dmap.h"
#include "core/msgstr.h"
#include "core/misctypes.h"
#include "core/initdata.h"
#include "base/version.h"
#include "new_subscr.h"
#include "components/zasm/debug_data.h"
#include "components/zasm/pc.h"
#include "zc/debugger/debugger.h"
#include "zc/maps.h"
#include "components/zasm/table.h"
#include "zc/replay.h"
#include "zc/scripting/array_manager.h"
#include "zc/scripting/arrays.h"
#include "zc/scripting/common.h"
#include "zc/scripting/script_object.h"
#include "zc/scripting/sram.h"
#include "zc/scripting/string_utils.h"
#include "zc/scripting/types.h"
#include "zc/scripting/types/genericdata.h"
#include "zc/scripting/types/mapdata.h"
#include "zc/scripting/types/portal.h"
#include "zc/scripting/types/savedportal.h"
#include "zc/scripting/types/subscreenwidget.h"
#include "zc/zc_ffc.h"
#include "zc/zc_sys.h"
#include "zc/jit.h"
#include "zc/script_debug.h"
#include "zc/script_timings.h"
#include "zalleg/zalleg.h"
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
#include "zalleg/colors.h"
#include "pal.h"
#include "zinfo.h"
#include "subscr.h"
#include "zc_list_data.h"
#include "music_playback.h"
#include "advanced_music.h"
#include "iter.h"
#include <sstream>

#include "zc/zelda.h"
#include "particles.h"
#include "zc/hero.h"
#include "zc/guys.h"
#include "gamedata.h"
#include "zc/zc_init.h"
#include "zalleg/zsys.h"
#include "core/misctypes.h"
#include "zc/title.h"
#include "zc/zscriptversion.h"

#include "pal.h"
#include "core/zdefs.h"
#include "zc/rendertarget.h"

#include "hero_tiles.h"
#include "core/qst.h"

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
	if (suppress_script_error_logging)
		return;

	// Bail before the debugger pause and the context/fmt work below - benchmarks
	// with every-frame script errors must not pay for messages nobody will see
	// (handle_trace would drop them anyway).
	if (disable_script_error_logs)
		return;

	if (auto debugger = zscript_debugger_get_if_open(); debugger && debugger->break_on_error)
		debugger->SetState(Debugger::State::Paused);

	if (current_zasm_context.empty())
	{
		std::vector<const char*> context;

		const char* command_string = scripting_get_zasm_command_context_string(current_zasm_command);
		if (command_string)
			context.push_back(command_string);

		const char* register_string = scripting_get_zasm_register_context_string(current_zasm_register);
		if (register_string)
			context.push_back(register_string);

		if (!current_zasm_extra_context.empty())
			context.push_back(current_zasm_extra_context.c_str());

		if (context.size())
			current_zasm_context = fmt::format("{}", fmt::join(context, ", "));
		else
		{
			bool is_error = true;
			FFCore.handle_trace(text + "\n", is_error);
			return;
		}
	}

	bool is_error = true;
	FFCore.handle_trace(fmt::format("{} | {}\n", current_zasm_context.c_str(), text.c_str()), is_error);

	current_zasm_context = "";
	current_zasm_extra_context = "";
}

// (type, index) => ScriptEngineData
std::map<std::pair<ScriptType, int>, ScriptEngineData> scriptEngineDatas;

uint8_t using_SRAM = 0;

int32_t hangcount = 0;
bool can_neg_array = true;

extern byte monochrome_console;

static std::map<script_id, ScriptDebugHandle> script_debug_handles;
ScriptDebugHandle* runtime_script_debug_handle;

CScriptDrawingCommands scriptdraws;
FFScript FFCore;

static UserDataContainer<script_array, 1000000> script_arrays = {script_object_type::array, "array"};
static UserDataContainer<user_paldata, MAX_USER_PALDATAS> user_paldatas = {script_object_type::paldata, "paldata"};
static UserDataContainer<user_rng, MAX_USER_RNGS> user_rngs = {script_object_type::rng, "rng"};
extern UserDataContainer<user_stack, MAX_USER_STACKS> user_stacks;
extern UserDataContainer<user_bitmap, MAX_USER_BITMAPS> user_bitmaps;
static UserDataContainer<user_weapondata, MAX_USER_WEAPONDATAS> user_weapondatas = {script_object_type::weapondata, "weapondata"};

weapon_data* checkWeaponData(int32_t ref, bool skipError)
{
	auto* ptr = user_weapondatas.check(ref, skipError);
	if (ptr)
	{
		auto old_suppress_script_error_logging =  suppress_script_error_logging;
		if (skipError)
			suppress_script_error_logging = true;
		auto* data = ptr->get_data();
		suppress_script_error_logging = old_suppress_script_error_logging;
		if (data)
			return data;
	}
	if(!skipError)
		scripting_log_error_with_context("Invalid {} using UID = {}", "weapondata", ref);
	return nullptr;
}

weapon_data* user_weapondata::get_data()
{
	switch (data_type)
	{
		case wdata_type::script:
			return inner_data;
		case wdata_type::npcdata:
			if (data_index >= MAXNPCS)
				break;
			return &guysbuf[data_index].weap_data;
		case wdata_type::npc:
			if (enemy* e = ResolveNpc(data_index))
				return &e->weap_data;
			break;
		case wdata_type::itemdata:
			if (invalid_item_id(data_index))
				break;
			return &itemsbuf[data_index].weap_data;
		case wdata_type::combodata_lift:
			if (data_index >= MAXCOMBOS)
				break;
			return &combobuf[data_index].lift_weap_data;
		case wdata_type::combodata_misc:
			if (data_index >= MAXCOMBOS)
				break;
			return &combobuf[data_index].misc_weap_data;
	}
	return nullptr;
}
static uint32_t make_new_user_weapondata(wdata_type data_type, dword data_index)
{
	uint32_t ret = 0;
	if (data_type != wdata_type::script)
	{
		user_weapondatas.foreach([&](uint32_t id, user_weapondata *wdata)
		{
			if (wdata->data_type == data_type && wdata->data_index == data_index)
			{
				ret = id;
				return true;
			}
			return false;
		});
	}
	if (!ret)
	{
		auto* wdata = user_weapondatas.create();
		if (wdata)
		{
			ret = wdata->id;
			wdata->data_type = data_type;
			wdata->data_index = data_index;
			if (data_type == wdata_type::script)
				wdata->inner_data = new weapon_data();
		}
	}
	return ret;
}

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

script_array* find_or_create_internal_script_array(script_array::internal_array_id internal_id)
{
	if (!zasm_array_supports(internal_id.zasm_var))
	{
		scripting_log_error_with_context("Invalid internal array id: {}", internal_id.zasm_var);
		return nullptr;
	}

	for (auto id : script_object_ids_by_type[script_arrays.type])
	{
		auto object = static_cast<script_array*>(get_script_object_checked(id));
		DCHECK(object);
		if (!object->internal_expired && object->internal_id.has_value() && object->internal_id.value() == internal_id)
			return object;
	}

	auto array = script_arrays.create();
	if (array)
	{
		array->arr.setValid(true);
		array->internal_id = internal_id;
		// Retain a reference until the underlying engine object is destroyed.
		script_object_ref_inc(array->id);
	}
	return array;
}

static void expire_internal_script_arrays(ScriptType scriptType, int ref)
{
	if (!ZScriptVersion::gc_arrays())
		return;

	// Expire internal arrays referring to this script object.
	std::vector<uint32_t> retained_ids;
	for (auto& script_object : script_objects | std::views::values)
	{
		if (script_object->type != script_object_type::array)
			continue;

		auto array = static_cast<script_array*>(script_object.get());
		if (!array->internal_id.has_value() || array->internal_expired)
			continue;

		if (array->internal_id->matches(scriptType, ref))
		{
			retained_ids.push_back(array->id);
			array->get_retained_ids(retained_ids);
			array->internal_expired = true;
		}
	}

	for (auto id : retained_ids)
		script_object_ref_dec(id);
}

static void expire_internal_script_arrays(ScriptType scriptType)
{
	if (!ZScriptVersion::gc_arrays())
		return;

	// Expire internal arrays referring to this script object.
	std::vector<uint32_t> retained_ids;
	for (auto& script_object : script_objects | std::views::values)
	{
		if (script_object->type != script_object_type::array)
			continue;

		auto array = static_cast<script_array*>(script_object.get());
		if (!array->internal_id.has_value() || array->internal_expired)
			continue;

		if (array->internal_id->matches(scriptType))
		{
			retained_ids.push_back(array->id);
			array->get_retained_ids(retained_ids);
			array->internal_expired = true;
		}
	}

	for (auto id : retained_ids)
		script_object_ref_dec(id);
}

// Releases everything a script's engine data owns, in preparation for that entry being
// zeroed (reset) or erased (cleared). This is the single chokepoint that frees:
// - script objects and arrays the script took ownership of via Own()
// - object references the script left on its stack (under GC)
// - internal arrays that refer to the script
// It must run while the entry's ref/stack still describe the finished script.
static void release_script_engine_data(ScriptType type, int index)
{
	FFScript::deallocateAllScriptOwned(type, index);
	expire_internal_script_arrays(type, index);
}

script_array* checkArray(uint32_t id, bool skipError)
{
	return script_arrays.check(id, skipError);
}

void script_bitmaps::update()
{
	// intentionally copying ids as script_object_ids_by_type may be mutated
	auto ids = script_object_ids_by_type[user_bitmaps.type];
	for (auto const& id : ids)
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

int32_t CScriptDrawingCommands::GetCount()
{
	al_trace("current number of draws is: %d\n", count);
	return count;
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
		case sstMAP:
			if(sub < subscreens_map.size())
				sbscr = &subscreens_map[sub];
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

//We gain some speed by not passing as arguments
int32_t sarg1;
int32_t sarg2;
int32_t sarg3;
vector<int32_t> *sargvec;
string *sargstr;
refInfo *ri;
script_data *curscript;
int32_t(*stack)[MAX_STACK_SIZE];
int32_t(*ret_stack)[MAX_CALL_FRAMES];
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
static vector<int32_t(*)[MAX_CALL_FRAMES]> ret_stack_cache;
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

void log_stack_overflow_error()
{
	scripting_log_error_with_context("Stack overflow!");
}

void log_call_limit_error()
{
	scripting_log_error_with_context("Function call limit reached! Too much recursion. Max nested function calls is {}", MAX_CALL_FRAMES);
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

int32_t get_mi(mapdata const& ref)
{
	if (ref.canonical())
	{
		if (ref.screen >= MAPSCRSNORMAL) return -1;
		return mapind(ref.scr->map, ref.screen);
	}
	else if (ref.current())
	{
		if (ref.screen >= MAPSCRSNORMAL) return -1;
		return mapind(cur_map, ref.screen);
	}
	else if (ref.scrolling())
	{
		if (ref.screen >= MAPSCRSNORMAL) return -1;
		return mapind(scrolling_map, ref.screen);
	}

	return -1;
}
int32_t get_mi(int32_t ref)
{
	return get_mi(decode_mapdata_ref(ref));
}

int32_t get_ref_map_index(int32_t ref)
{
	if (ref > 0)
		return ref - 1;

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

// For compat, get the first `combo_trigger` of the current `ri->combodataref`
combo_trigger* get_first_combo_trigger()
{
	int ref = GET_REF(combodataref);
	if(ref < 0 || ref > (MAXCOMBOS-1) )
		return nullptr;
	if(combobuf[ref].triggers.empty())
		return &(combobuf[ref].triggers.emplace_back());
	return &(combobuf[ref].triggers[0]);
}
// Get the combo trigger pointed to by `ref` (usually ri->combotriggerref)
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
// Get the combo ID of the trigger pointed to by `ref` (usually ri->combotriggerref)
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

ScriptEngineData& get_script_engine_data(ScriptType type, int index)
{
	if (type == ScriptType::DMap || type == ScriptType::OnMap || type == ScriptType::ScriptedPassiveSubscreen || type == ScriptType::ScriptedActiveSubscreen)
	{
		// `index` is used for dmapdataref, not for different script engine data.
		index = 0;
	}
	if (type == ScriptType::EngineSubscreen)
	{
		// `index` is used for subscreendataref, not for different script engine data.
		index = 0;
	}
	
	return scriptEngineDatas[{type, index}];
}

bool script_engine_data_exists(ScriptType type, int index)
{
	if (type == ScriptType::DMap || type == ScriptType::OnMap || type == ScriptType::ScriptedPassiveSubscreen || type == ScriptType::ScriptedActiveSubscreen)
	{
		// `index` is used for dmapdataref, not for different script engine data.
		index = 0;
	}
	if (type == ScriptType::EngineSubscreen)
	{
		// `index` is used for subscreendataref, not for different script engine data.
		index = 0;
	}

	return scriptEngineDatas.contains({type, index});
}

ScriptEngineData& get_script_engine_data(ScriptType type)
{
	return get_script_engine_data(type, 0);
}

void FFScript::clear_script_engine_data()
{
	scriptEngineDatas.clear();
}

void FFScript::clear_script_engine_data_for_continue()
{
	// Generic scripts should survive F6->Continue!
	for (auto it = scriptEngineDatas.begin(); it != scriptEngineDatas.end();)
	{
		if (it->first.first == ScriptType::Generic)
			++it;
		else it = scriptEngineDatas.erase(it);
	}
}

void FFScript::reset_script_engine_data(ScriptType type, int index)
{
	// Releasing first frees anything the previous occupant of this slot still owned,
	// so reusing or restarting the slot can never leak its objects.
	release_script_engine_data(type, index);
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
		// `index` is used for dmapdataref, not for different script engine data.
		index = 0;
	}
	if (type == ScriptType::EngineSubscreen)
	{
		// `index` is used for subscreendataref, not for different script engine data.
		index = 0;
	}

	release_script_engine_data(type, index);
	scriptEngineDatas.erase({type, index});
}

void FFScript::clear_script_engine_data_of_type(ScriptType type)
{
	deallocateAllScriptOwnedOfType(type);
	expire_internal_script_arrays(type);
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

template <typename T, std::size_t N>
static T get_script_safe(T (&arr)[N], int index)
{
	if (index < 0 || index >= N)
		return nullptr;

	return arr[index];
}

static script_data* get_script_safe(ScriptType type, int script)
{
	switch (type)
	{
		case ScriptType::FFC:
			return get_script_safe(ffscripts, script);

		case ScriptType::NPC:
			return get_script_safe(guyscripts, script);

		case ScriptType::Lwpn:
			return get_script_safe(lwpnscripts, script);

		case ScriptType::Ewpn:
			return get_script_safe(ewpnscripts, script);

		case ScriptType::ItemSprite:
			return get_script_safe(itemspritescripts, script);

		case ScriptType::Item:
			return get_script_safe(itemscripts, script);

		case ScriptType::Global:
			return get_script_safe(globalscripts, script);

		case ScriptType::Generic:
		case ScriptType::GenericFrozen:
			return get_script_safe(genericscripts, script);

		case ScriptType::Hero:
			return get_script_safe(playerscripts, script);

		case ScriptType::DMap:
		case ScriptType::OnMap:
		case ScriptType::ScriptedActiveSubscreen:
		case ScriptType::ScriptedPassiveSubscreen:
			return get_script_safe(dmapscripts, script);

		case ScriptType::EngineSubscreen:
			return get_script_safe(subscreenscripts, script);

		case ScriptType::Screen:
			return get_script_safe(screenscripts, script);

		case ScriptType::Combo:
			return get_script_safe(comboscripts, script);
	}

	return nullptr;
}

static void clear_script_variables()
{
	vector<int> ids_to_clear;
	for (uint32_t offset : ri->script_d_is_object)
	{
		ids_to_clear.push_back(ri->script_d[offset]);
	}
	ri->script_d.clear();
	ri->script_d_is_object.clear();
	for (auto id : ids_to_clear)
		script_object_ref_dec(id);
}
static void reset_script_variables()
{
	clear_script_variables();
	for (auto [id, val] : curscript->script_d_init.inner())
	{
		if (val)
			ri->script_d[id] = val;
	}
}
static void reset_script_variables(script_config const& cfg)
{
	reset_script_variables();
	for (auto [id, val] : cfg.inst_init)
		ri->script_d[id] = val;
	
	memcpy(ri->d, cfg.run_args.data(), 8 * sizeof(int32_t));
}

static void set_current_script_engine_data(ScriptEngineData& data, ScriptType type, int script, script_data* sd, int index)
{
	bool got_initialized = false;

	ri = &data.ref;
	stack = &data.stack;
	ret_stack = &data.ret_stack;
	curscript = sd;

	data.script_type = type;
	data.script_num = script;

	// By default, make `Screen->` refer to the top-left screen.
	// Will be set to something more specific for relevant script types.
	ri->screenref = cur_screen;

	switch (type)
	{
		case ScriptType::FFC:
		{
			ffcdata* ffc = get_ffc(index);

			if (!data.initialized)
			{
				got_initialized = true;
				mapscr* scr = get_scr(ffc->screen_spawned);
				reset_script_variables(scr->ffcs[index % 128].scrconfig);
				data.initialized = true;
			}

			ri->ffcref = ZScriptVersion::ffcRefIsSpriteId() ? ffc->getUID() : index;
			ri->screenref = ffc->screen_spawned;
			ri->script_d[0] = ri->ffcref;
		}
		break;
		
		case ScriptType::NPC:
		{
			enemy *spr = (enemy*)guys.getByUID(index);
			
			if (!data.initialized)
			{
				got_initialized = true;
				reset_script_variables(spr->scrconfig);
				data.initialized = 1;
			}
			
			ri->npcref = index;
			ri->screenref = spr->screen_spawned;
			ri->script_d[0] = ri->npcref;
		}
		break;
		
		case ScriptType::Lwpn:
		{
			weapon *spr = (weapon*)Lwpns.getByUID(index);
			
			if (!data.initialized)
			{
				got_initialized = true;
				reset_script_variables(spr->scrconfig);
				data.initialized = 1;
			}
			
			ri->lwpnref = index;
			ri->screenref = spr->screen_spawned;
			ri->script_d[0] = ri->lwpnref;
		}
		break;
		
		case ScriptType::Ewpn:
		{
			weapon *spr = (weapon*)Ewpns.getByUID(index);
			
			if (!data.initialized)
			{
				got_initialized = true;
				reset_script_variables(spr->scrconfig);
				data.initialized = 1;
			}
			
			ri->ewpnref = index;
			ri->screenref = spr->screen_spawned;
			ri->script_d[0] = ri->ewpnref;
		}
		break;
		
		case ScriptType::ItemSprite:
		{
			item *spr = (item*)items.getByUID(index);
			
			if (!data.initialized)
			{
				got_initialized = true;
				reset_script_variables(spr->scrconfig);
				data.initialized = 1;
			}
			
			ri->itemref = index;
			ri->screenref = spr->screen_spawned;
			ri->script_d[0] = ri->itemref;
		}
		break;
		
		case ScriptType::Item:
		{
			int32_t i = index;
			int32_t new_i = 0;
			bool collect = ( ( i < 1 ) || (i == COLLECT_SCRIPT_ITEM_ZERO) );
			new_i = ( collect ) ? (( i != COLLECT_SCRIPT_ITEM_ZERO ) ? (i * -1) : 0) : i;

			
			if (!data.initialized)
			{
				got_initialized = true;
				auto& idat = itemsbuf.get(new_i);
				reset_script_variables(collect ? idat.collect_scrconfig : idat.scrconfig);
				data.initialized = true;
			}			
			//'this' pointer
			ri->itemdataref = ( collect ) ? new_i : i;
			ri->script_d[0] = ri->itemdataref;
		}
		break;
		
		case ScriptType::Global:
		{
			if (!data.initialized)
			{
				got_initialized = true;

				// If this compat QR is on, scripts can run before ~Init and set global variables.
				// Before overwriting them with 0, get rid of object references held by global variables.
				if (get_qr(qr_OLD_INIT_SCRIPT_TIMING) && ZScriptVersion::gc() && script == GLOBAL_SCRIPT_INIT)
				{
					for (int i = 0; i < MAX_GLOBAL_VARIABLES; i++)
						script_object_ref_dec(game->global_d[i]);
				}
				
				reset_script_variables();
				data.initialized = 1;
			}
		}
		break;
		
		case ScriptType::Generic:
		{
			user_genscript& scr = user_genscript::get(script);
			scr.waitevent = false;
			if(!data.initialized)
			{
				got_initialized = true;
				reset_script_variables(scr.scrconfig);
				data.initialized = true;
			}
			ri->genericdataref = script;
			ri->script_d[0] = ri->genericdataref;
		}
		break;
		
		case ScriptType::GenericFrozen:
		{
			user_genscript& scr = user_genscript::get(script);
			if(!data.initialized)
			{
				got_initialized = true;
				reset_script_variables(scr.scrconfig);
				data.initialized = true;
			}
			ri->genericdataref = script;
			ri->script_d[0] = ri->genericdataref;
		}
		break;
		
		case ScriptType::Hero:
		{
			ri->screenref = Hero.current_screen;
			if (!data.initialized)
			{
				got_initialized = true;
				reset_script_variables();
				data.initialized = 1;
			}
		}
		break;
		
		case ScriptType::DMap:
		{
			ri->dmapdataref = index;
			//how do we clear initialised on dmap change?
			if ( !data.initialized )
			{
				got_initialized = true;
				reset_script_variables(DMaps[index].active_scrconfig);
				data.initialized = true;
			}
			ri->script_d[0] = ri->dmapdataref;
		}
		break;
		
		case ScriptType::OnMap:
		{
			ri->dmapdataref = index;
			if (!data.initialized)
			{
				got_initialized = true;
				reset_script_variables(DMaps[index].onmap_scrconfig);
				data.initialized = true;
			}
			ri->script_d[0] = ri->dmapdataref;
		}
		break;
		
		case ScriptType::ScriptedActiveSubscreen:
		{
			ri->dmapdataref = index;
			if (!data.initialized)
			{
				got_initialized = true;
				reset_script_variables(DMaps[index].active_sub_scrconfig);
				data.initialized = true;
			}
			ri->script_d[0] = ri->dmapdataref;
		}
		break;
		
		case ScriptType::ScriptedPassiveSubscreen:
		{
			ri->dmapdataref = index;
			if (!data.initialized)
			{
				got_initialized = true;
				reset_script_variables(DMaps[index].passive_sub_scrconfig);
				data.initialized = true;
			}
			ri->script_d[0] = ri->dmapdataref;
		}
		break;
		case ScriptType::EngineSubscreen:
		{
			ri->subscreendataref = get_subref(-1, index);
			auto [ptr,_ty] = load_subdata(ri->subscreendataref);
			
			if (ptr && !data.initialized)
			{
				got_initialized = true;
				reset_script_variables(ptr->scrconfig);
				data.initialized = true;
			}
			ri->script_d[0] = ri->subscreendataref;
		}
		break;
		
		case ScriptType::Screen:
		{
			if (!data.initialized)
			{
				got_initialized = true;
				mapscr* scr = get_scr(index);
				reset_script_variables(scr->scrconfig);
				data.initialized = true;
			}

			ri->screenref = index;
		}
		break;
		
		case ScriptType::Combo:
		{
			rpos_t rpos = combopos_ref_to_rpos(index);
			int32_t lyr = combopos_ref_to_layer(index);
			auto rpos_handle = get_rpos_handle(rpos, lyr);
			int32_t id = rpos_handle.data();
			if (!data.initialized)
			{
				got_initialized = true;
				reset_script_variables(combobuf[id].scrconfig);
				data.initialized = true;
			}

			ri->combodataref = id; //'this' pointer
			ri->comboposref = index; //used for X(), Y(), Layer(), and so forth.
			ri->screenref = rpos_handle.screen;
			ri->script_d[0] = ri->combodataref;
			break;
		}
	}
	
	if (got_initialized)
	{
		ri->pc = curscript->pc;

		if (!script_is_within_debugger_vm && curscript->valid())
		{
			// Note: the below works, but is more expensive than just setting the correct stack size when writing to D4.
			// int stack_size = zasm_debug_data.getFunctionAdditionalStackSize(zasm_debug_data.resolveFunctionScope(ri->pc));
			int stack_size = 0;
			ri->debugger_stack_frames.push_back(DebuggerStackFrame{
				.stack_frame_base = (uint16_t)(ri->sp - stack_size),
				.this_ptr = ri->thiskey,
			});

			if (auto debugger = zscript_debugger_get_if_open(); debugger && debugger->break_on_new_script)
				debugger->SetState(Debugger::State::Paused);
		}
	}
}

ffcdata* ResolveFFCWithID(ffc_id_t id)
{
	if (BC::checkFFC(id) != SH::_NoError)
		return nullptr;

	ffcdata* ffc = get_ffc(id);
	if (!ffc)
		scripting_log_error_with_context("Invalid ffc using ID = {}", id);

	return ffc;
}

ffcdata *ResolveFFC(int32_t ffcref)
{
	if (ZScriptVersion::ffcRefIsSpriteId())
		return ResolveSprite<ffcdata>(ffcref, "ffc");

	return ResolveFFCWithID(ffcref);
}

int32_t genscript_timing = SCR_TIMING_START_FRAME;
static word max_valid_genscript;

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
		gen.exitState = gd.gen_exitState.get(q);
		gen.reloadState = gd.gen_reloadState.get(q);
		gen.eventstate = gd.gen_eventstate.get(q);
		gen.data = gd.gen_data.get(q);
		auto const& run_args = gd.gen_initd.get(q);
		for (int q = 0; q < 8; ++q)
			gen.scrconfig.run_args[q] = run_args.get(q);
		gen.scrconfig.inst_init = gd.gen_inst_init.get(q);
	}
}
void load_genscript(const zinitdata& zd)
{
	for(size_t q = 0; q < NUMSCRIPTSGENERIC; ++q)
	{
		user_genscript& gen = user_genscript::get(q);
		gen.clear();
		gen.doscript() = zd.gen_doscript.get(q);
		gen.exitState = zd.gen_exitState.get(q);
		gen.reloadState = zd.gen_reloadState.get(q);
		gen.eventstate = zd.gen_eventstate.get(q);
		gen.data = zd.gen_data.get(q);
		auto const& run_args = zd.gen_initd.get(q);
		for (int q = 0; q < 8; ++q)
			gen.scrconfig.run_args[q] = run_args.get(q);
		gen.scrconfig.inst_init = zd.gen_inst_init.get(q);
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
		gd.gen_data[q] = gen.data;
		
		bounded_vec<byte,int32_t> run_args {8};
		for (int q = 0; q < 8; ++q)
		{
			if (int v = gen.scrconfig.run_args[q])
				run_args[q] = v;
		}
		gd.gen_initd[q] = run_args;
		
		gd.gen_inst_init[q] = gen.scrconfig.inst_init;
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
	if (scriptEngineDatas.contains({ScriptType::DMap, 0}))
		reset_script_engine_data(ScriptType::DMap, 0);
	if (scriptEngineDatas.contains({ScriptType::ScriptedPassiveSubscreen, 0}))
		reset_script_engine_data(ScriptType::ScriptedPassiveSubscreen, 0);
}

void FFScript::initZScriptSubscreenScript()
{
	// Cleared on every exit by ScopedScriptEngineDataClear in dosubscr.
	scriptEngineDatas[{ScriptType::EngineSubscreen, 0}] = ScriptEngineData();
}
void FFScript::initZScriptScriptedActiveSubscreen()
{
	// Cleared on every exit by ScopedScriptEngineDataClear in runScriptedActiveSubscreen.
	scriptEngineDatas[{ScriptType::ScriptedActiveSubscreen, 0}] = ScriptEngineData();
}

void FFScript::initZScriptOnMapScript()
{
	// Cleared on every exit by ScopedScriptEngineDataClear in runOnMapScriptEngine.
	scriptEngineDatas[{ScriptType::OnMap, 0}] = ScriptEngineData();
}

void FFScript::initZScriptHeroScripts()
{
	if (scriptEngineDatas.contains({ScriptType::Hero, 0}))
		reset_script_engine_data(ScriptType::Hero, 0);
}


void FFScript::initZScriptItemScripts()
{
	for (int32_t q = 0; q < MAXITEMS; ++q)
	{
		int c = q ? -q : COLLECT_SCRIPT_ITEM_ZERO;
		
		// Clear all the allocated memory for item scripts
		clear_script_engine_data(ScriptType::Item, q);
		clear_script_engine_data(ScriptType::Item, c);
		
		// only re-allocate memory for items this quest uses
		// IDATASCRIPT and IDATAPSCRIPT setters handle initializing these later,
		//     if scripts make previously unused items now used.
		if (q < itemsbuf.capacity())
		{
			auto const& itm = itemsbuf.get(q);
			if (itm.scrconfig.script)
			{
				auto& data = get_script_engine_data(ScriptType::Item, q);
				data.reset();
				data.doscript = ((itm.flags&item_passive_script) && game->get_item(q)) ? 1 : 0;
			}
			if (itm.collect_scrconfig.script)
			{
				auto& cdata = get_script_engine_data(ScriptType::Item, c);
				cdata.reset();
				cdata.doscript = 0;
			}
		}
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
	str.reserve(sz);
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
		// Pass along the source element's object type, so a destination untyped
		// array retains copied objects.
		am1.set(i,am2.get(i),am2.get_object_type(i));
	}
}
//Get element from array
int32_t ArrayH::getElement(const int32_t ptr, int32_t offset, const bool neg)
{
	ArrayManager am(ptr,neg);
	return am.get(offset);
}

//Set element in array
void ArrayH::setElement(const int32_t ptr, int32_t offset, const int32_t value, const bool neg, const script_object_type type)
{
	ArrayManager am(ptr,neg);
	am.set(offset, value, type);
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

void FFScript::release_sprite_owned_objects(int32_t sprite_id)
{
	std::vector<uint32_t> ids_to_clear;
	for (auto& script_object : script_objects | std::views::values)
	{
		if (script_object->sprite_own_clear(sprite_id))
		{
			ids_to_clear.push_back(script_object->id);
			script_object->disown();
		}
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
		for (int32_t i = 1; i < NUM_ZSCRIPT_ARRAYS; i++)
		{
			if (arrayOwner[i].sprite_own_clear(sprite_id))
				FFScript::deallocateArray(i);
		}
	}
}

// Called when the sprite object is being destroyed.
//
// - clears any object references retained by a sprite via "ownership".
//   See the comment above script_object_base::set_owned_by_script for more.
// - clears script engine data
// - invalidates any internal array references that refer to this sprite
void FFScript::destroySprite(sprite* sprite)
{
	DCHECK(sprite->get_scrtype().has_value());
	ScriptType scriptType = *sprite->get_scrtype();
	int32_t uid = sprite->getUID();
	FFCore.release_sprite_owned_objects(uid);
	FFCore.clear_script_engine_data(scriptType, uid);
}

vector<int> clear_vargs_back()
{
	// Return a copy of the vargs vector
	vector<int> vargs = ri->zs_vargs_stack.back();
	
	// Release the vargs' object references, but keep every object alive via the
	// autorelease pool, in case it is used after this (ex. returned by
	// `Choose(obj1, obj2)`, or retained by the array `do_varg_makearray` builds).
	// Each marked position holds one counted reference (see MARK_TYPE_VARG).
	for (auto pos : ri->zs_vargs_pos_is_object.back())
		script_object_transfer_ref_to_autorelease_pool(ri->zs_vargs_stack.back().at(pos));
	
	// now that we've handled copying and refcounting, clear the back vargs vectors
	ri->zs_vargs_stack.back().clear();
	ri->zs_vargs_pos_is_object.back().clear();
	
	// and finally return the copied vargs
	return vargs;
}

void FFScript::deallocateAllScriptOwned(ScriptType scriptType, const int32_t UID)
{
	std::vector<uint32_t> ids_to_clear;
	for (auto& script_object : script_objects | std::views::values)
	{
		if (script_object->script_own_clear(scriptType, UID))
		{
			ids_to_clear.push_back(script_object->id);
			script_object->disown();
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
		
		for (size_t q = 0; q < data.ref.zs_vargs_stack.size(); ++q)
			for (int i : data.ref.zs_vargs_pos_is_object[q])
				ids_to_clear.push_back(data.ref.zs_vargs_stack.at(q).at(i));
		data.ref.zs_vargs_stack = {{}};
		data.ref.zs_vargs_pos_is_object = {{}};
		
		for (uint32_t offset : data.ref.script_d_is_object)
		{
			uint32_t id = data.ref.script_d[offset];
			ids_to_clear.push_back(id);
		}
		data.ref.script_d_is_object.clear();
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
			if(arrayOwner[i].script_own_clear(scriptType,UID))
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
			script_object->disown();
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
			
			for (size_t q = 0; q < data.ref.zs_vargs_stack.size(); ++q)
				for (int i : data.ref.zs_vargs_pos_is_object[q])
					ids_to_clear.push_back(data.ref.zs_vargs_stack.at(q).at(i));
			data.ref.zs_vargs_stack = {{}};
			data.ref.zs_vargs_pos_is_object = {{}};
			
			for (uint32_t offset : data.ref.script_d_is_object)
			{
				uint32_t id = data.ref.script_d[offset];
				ids_to_clear.push_back(id);
			}
			data.ref.script_d_is_object.clear();
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
		if (script_object->script_own_clear_cont())
		{
			ids_to_clear.push_back(script_object->id);
			script_object->disown();
		}
	}

	if (ZScriptVersion::gc())
	{
		for (auto& [key, data] : scriptEngineDatas)
		{
			// Generic scripts may survive the continue with their engine data intact
			// (see clear_script_engine_data_for_continue), in which case their stacks
			// keep referencing objects. Generic scripts that instead exit or relaunch
			// release their references in user_genscript::quit, via
			// reset_script_engine_data.
			if (key.first == ScriptType::Generic)
				continue;

			for (uint32_t offset : data.ref.stack_pos_is_object)
			{
				uint32_t id = data.stack[offset];
				ids_to_clear.push_back(id);
			}
			data.ref.stack_pos_is_object.clear();
			
			for (size_t q = 0; q < data.ref.zs_vargs_stack.size(); ++q)
				for (int i : data.ref.zs_vargs_pos_is_object[q])
				{
					uint32_t id = data.ref.zs_vargs_stack.at(q).at(i);
					ids_to_clear.push_back(id);
				}
			data.ref.zs_vargs_stack = {{}};
			data.ref.zs_vargs_pos_is_object = {{}};
			
			for (uint32_t offset : data.ref.script_d_is_object)
			{
				uint32_t id = data.ref.script_d[offset];
				ids_to_clear.push_back(id);
			}
			data.ref.script_d_is_object.clear();
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
				if(arrayOwner[i].script_own_clear_cont())
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
		if (!skipError)
			scripting_log_error_with_context("Invalid combodata ID: {}", ref);
		return nullptr;
	}

	return &combobuf[ref];
}

static bool checkComboRef()
{
	if (GET_REF(combodataref) < 0 || GET_REF(combodataref) > (MAXCOMBOS-1))
	{
		scripting_log_error_with_context("Invalid combodata ID: {}", GET_REF(combodataref));
		return false;
	}

	return true;
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
	if (valid_item_id(ref))
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

sprite_data *checkSpriteData(int32_t ref)
{
	if(ref > 0 && ref < MAXSPRITES)
		return &sprite_data_buf[ref];

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

extern const std::string subscr_names[sstMAX];
static std::string subscr_type_names(std::set<int> const& req_tys)
{
	ostringstream oss;
	bool first = true;
	for (auto req_ty : req_tys)
	{
		if (first)
			first = false;
		else oss << ", ";
		oss << "'" << subscr_names[req_ty] << "'";
	}
	return oss.str();
}
ZCSubscreen *checkSubData(int32_t ref, std::set<int> const& req_tys)
{
	auto [ptr,ty] = load_subdata(ref);
	if(ptr)
	{
		if(req_tys.empty() || req_tys.contains(ty))
			return ptr;
		else
		{
			scripting_log_error_with_context("Wrong type of SubscreenData accessed! Expecting type [{}], but found '{}'",
				subscr_type_names(req_tys), subscr_names[ty]);
		}
	}
	else scripting_log_error_with_context("Script attempted to reference a nonexistent SubscreenData!");
	
	scripting_log_error_with_context("You were trying to reference an invalid SubscreenData with UID = {}", ref);
	return NULL;
}

SubscrPage *checkSubPage(int32_t ref, std::set<int> const& req_tys)
{
	auto [ptr,ty] = load_subpage(ref);
	if(ptr)
	{
		if(req_tys.empty() || req_tys.contains(ty))
			return ptr;
		else
		{
			scripting_log_error_with_context("Wrong type of Subscreen accessed! Expecting type [{}], but found '{}'",
				subscr_type_names(req_tys), subscr_names[ty]);
		}
	}
	else scripting_log_error_with_context("Script attempted to reference a nonexistent SubscreenPage!");
	
	scripting_log_error_with_context("You were trying to reference an invalid SubscreenPage with UID = {}", ref);
	return NULL;
}

SubscrWidget *checkSubWidg(int32_t ref, std::set<int> const& req_sub_tys, int req_widg_ty)
{
	auto [ptr,ty] = load_subwidg(ref);
	if(ptr)
	{
		if(req_sub_tys.empty() || req_sub_tys.contains(ty))
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
				subscr_type_names(req_sub_tys), subscr_names[ty]);
		}
	}
	else scripting_log_error_with_context("Script attempted to reference a nonexistent SubscreenWidget!");

	scripting_log_error_with_context("You were trying to reference an invalid SubscreenWidget with UID = {}", ref);
	return NULL;
}

bool scripting_use_8bit_colors;
int scripting_max_color_val;

int scripting_read_pal_color(int c)
{
	return scripting_use_8bit_colors ? c : c / 4;
}

int scripting_write_pal_color(int c)
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
		case qr_COOLSCROLL:
		case qr_FADEBLACKWIPE:
		case qr_OVALWIPE:
		case qr_SMASWIPE:
		case qr_TRIANGLEWIPE:
		{
			COOLSCROLL =
				(get_qr(qr_COOLSCROLL)!=0 ? 1 : 0) |
				(get_qr(qr_OVALWIPE)!=0 ? 2 : 0) |
				(get_qr(qr_TRIANGLEWIPE)!=0 ? 4 : 0) |
				(get_qr(qr_SMASWIPE)!=0 ? 8 : 0) |
				(get_qr(qr_FADEBLACKWIPE)!=0 ? 16 : 0);
			break;
		}
		case qr_BSZELDA:
		{
			BSZ = value;
			break;
		}
	}
}

static void apply_qr_rules()
{
	apply_qr_rule(qr_BSZELDA);
	apply_qr_rule(qr_COOLSCROLL);
	apply_qr_rule(qr_HIDE_BOTTOM_8_PIXELS);
	apply_qr_rule(qr_LTTPCOLLISION);
	apply_qr_rule(qr_LTTPWALK);
	apply_qr_rule(qr_SCRIPTS_6_BIT_COLOR);
	apply_qr_rule(qr_ZS_NO_NEG_ARRAY);
}

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
		case CLASS_THISKEY: return ri->thiskey;
		case CLASS_THISKEY2: return ri->thiskey2;
		case REFBITMAP: return ri->bitmapref;
		case REFBOTTLESHOP: return ri->bottleshopref;
		case REFBOTTLETYPE: return ri->bottletyperef;
		case REFCOMBODATA: return ri->combodataref;
		case REFCOMBOTRIGGER: return ri->combotriggerref;
		case REFDIRECTORY: return ri->directoryref;
		case REFDMAPDATA: return ri->dmapdataref;
		case REFDROPSETDATA: return ri->dropsetdataref;
		case REFEWPN: return ri->ewpnref;
		case REFFFC: return ri->ffcref;
		case REFFILE: return ri->fileref;
		case REFGENERICDATA: return ri->genericdataref;
		case REFITEM: return ri->itemref;
		case REFITEMDATA: return ri->itemdataref;
		case REFLWPN: return ri->lwpnref;
		case REFMAPDATA: return ri->mapdataref;
		case REFMSGDATA: return ri->msgdataref;
		case REFNPC: return ri->npcref;
		case REFNPCDATA: return ri->npcdataref;
		case REFPALDATA: return ri->paldataref;
		case REFPORTAL: return ri->portalref;
		case REFRNG: return ri->rngref;
		case REFSAVPORTAL: return ri->savportalref;
		case REFSCREEN: return ri->screenref;
		case REFSHOPDATA: return ri->shopdataref;
		case REFSPRITE: return ri->spriteref;
		case REFSPRITEDATA: return ri->spritedataref;
		case REFSTACK: return ri->stackref;
		case REFSUBSCREENDATA: return ri->subscreendataref;
		case REFSUBSCREENPAGE: return ri->subscreenpageref;
		case REFSUBSCREENWIDG: return ri->subscreenwidgref;
		case REFWEBSOCKET: return ri->websocketref;
		case REFSAVEMENU: return ri->savemenuref;
		case REFMUSIC: return ri->musicref;
		case REFWEAPDATA: return ri->weapdataref;

		default: NOTREACHED();
	}
}

int32_t earlyretval = -1;
int32_t get_register(int32_t arg)
{
	if (arg >= D(0) && arg <= D(7))
		return ri->d[arg - D(0)];

	if (arg >= GD(0) && arg < GD(MAX_GLOBAL_VARIABLES))
		return game->global_d[arg - GD(0)];

	if (arg >= SCRIPT_INST_VARS(0) && arg < SCRIPT_INST_VARS(MAX_SCRIPT_INST_VARIABLES))
		return ri->script_d[arg - SCRIPT_INST_VARS(0)];

	int32_t ret = 0;
	current_zasm_register = arg;

	if (zasm_array_supports(arg))
	{
		int ref_arg = get_register_ref_dependency(arg).value_or(0);
#ifdef DEBUG_REGISTER_DEPS
		if (ref_arg) debug_get_ref(ref_arg);
#endif
		int ref = ref_arg ? get_ref(ref_arg) : 0;
		ret = zasm_array_get(arg, ref, GET_D(rINDEX) / 10000);
	}
	else ret = scripting_engine_get_register(arg);

	current_zasm_register = 0;

	return ret;
}

void set_register(int32_t arg, int32_t value)
{
	if (arg >= D(0) && arg <= D(7))
	{
		ri->d[arg - D(0)] = value;
		return;
	}
	else if (arg >= GD(0) && arg < GD(MAX_GLOBAL_VARIABLES))
	{
		game->global_d[arg-GD(0)] = value;
		return;
	}
	else if (arg >= SCRIPT_INST_VARS(0) && arg < SCRIPT_INST_VARS(MAX_SCRIPT_INST_VARIABLES))
	{
		ri->script_d[arg-SCRIPT_INST_VARS(0)] = value;
		return;
	}

	current_zasm_register = arg;

	if (zasm_array_supports(arg))
	{
		int ref_arg = get_register_ref_dependency(arg).value_or(0);
#ifdef DEBUG_REGISTER_DEPS
		if (ref_arg) debug_get_ref(ref_arg);
#endif
		int ref = ref_arg ? get_ref(ref_arg) : 0;
		zasm_array_set(arg, ref, GET_D(rINDEX) / 10000, value, script_object_type::none);
	}
	else
	{
		scripting_engine_set_register(arg, value);
	}

	current_zasm_register = 0;
}

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
	if(ri->retsp >= MAX_CALL_FRAMES)
	{
		log_call_limit_error();
		ri->overflow = true;
		return;
	}

	(*ret_stack)[ri->retsp++] = val;

	if (!script_is_within_debugger_vm)
	{
		// Note: the below works, but is more expensive than just setting the correct stack size when writing to D4.
		// int stack_size = zasm_debug_data.getFunctionAdditionalStackSize(zasm_debug_data.resolveFunctionScope(ri->pc));
		int stack_size = 0;
		ri->debugger_stack_frames.push_back(DebuggerStackFrame{
			.stack_frame_base = (uint16_t)(ri->sp - stack_size),
			.this_ptr = ri->thiskey,
		});
	}
}

optional<int32_t> retstack_pop()
{
	if(!ri->retsp)
		return nullopt; //return from root, so, QUIT

	if (!script_is_within_debugger_vm)
		ri->debugger_stack_frames.pop_back();
	return (*ret_stack)[--ri->retsp];
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
	if (reg == D(4))
	{
		if (!script_is_within_debugger_vm)
			ri->debugger_stack_frames.back().stack_frame_base = value;
		set_register(reg, value);
		return;
	}

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
				if (auto ffc = ResolveFFC(GET_REF(ffcref)); ffc && ffc->index == whichUID)
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
				if(reg==IDATAPSCRIPT && ri->itemdataref==new_UID)
					allowed = false;
			}
			else if(reg==IDATASCRIPT && ri->itemdataref==new_UID)
				allowed = false;
			break;
		}
		
		case ScriptType::Lwpn:
			if(reg==LWPNSCRIPT && ri->lwpnref==whichUID)
				allowed = false;
			break;
			
		case ScriptType::NPC:
			if(reg==NPCSCRIPT && ri->npcref==whichUID)
				allowed = false;
			break;
		
		case ScriptType::Ewpn:
			if(reg==EWPNSCRIPT && ri->ewpnref==whichUID)
				allowed = false;
			break;
		
		case ScriptType::DMap:
			if(reg==DMAPSCRIPT && ri->dmapdataref==whichUID)
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
	ri->zs_vargs_stack.back().push_back(value);
}

void do_push_vargs(const bool v)
{
	if(sarg2 < 1) return;
	const int value = SH::get_arg(sarg1, v);
	auto& vec = ri->zs_vargs_stack.back();
	vec.insert(vec.end(), sarg2, value);
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
	const int32_t stackoffset = (sarg2+GET_D(rSFRAME)) / 10000;
	const int32_t value = SH::read_stack(stackoffset);
	set_register(sarg1, value);
}

void do_load()
{
	const int32_t stackoffset = GET_D(rSFRAME) + sarg2;
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
	const int32_t stackoffset = (sarg2+GET_D(rSFRAME)) / 10000;
	const int32_t value = SH::get_arg(sarg1, v);
	SH::write_stack(stackoffset, value);
}

void do_store(const bool v)
{
	const int32_t stackoffset = GET_D(rSFRAME) + sarg2;
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
	if (ri->stackPosHasObject(offset))
		script_object_ref_dec(id);
	else
		ri->stack_pos_is_object.insert(offset);

	SH::write_stack(offset, new_id);

	if (script_object_autorelease_pool_remove(new_id))
		script_object_ref_dec(new_id);
}

void do_store_object(const bool v)
{
	const int32_t stackoffset = GET_D(rSFRAME) + sarg2;
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

	if (!ri->stackPosHasObject(offset))
		return;

	uint32_t id = SH::read_stack(offset);
	script_object_ref_dec(id);
	ri->stack_pos_is_object.erase(offset);
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
	// Since 3.0, arrays are script objects: ownership adds a reference that is
	// released when the owning script ends, like every other Own() function.
	if (ZScriptVersion::gc_arrays())
	{
		if (auto* array = checkArray(arrindx))
		{
			if (array->internal_id.has_value())
				Z_scripterrlog_force_trace("Cannot 'OwnArray()' an internal array '%d'\n", arrindx);
			else
				own_script_object(array, scriptType, UID);
		}
		return;
	}

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

void do_own_array(int arrindx, sprite* spr)
{
	// See the note in the ScriptType overload above.
	if (ZScriptVersion::gc_arrays())
	{
		if (auto* array = checkArray(arrindx))
		{
			if (array->internal_id.has_value())
				Z_scripterrlog_force_trace("Cannot 'OwnArray()' an internal array '%d'\n", arrindx);
			else
				own_script_object(array, spr);
		}
		return;
	}

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
			arrayOwner[arrindx].reown(spr);
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

	// INT_MIN % -1 overflows the idiv instruction (the quotient is not
	// representable), but x % -1 is always 0.
	set_register(destreg, temp == -1 ? 0 : temp2 % temp);
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
		set_register(sarg1, int32_t(zc::math::ArcSin(temp) * 10000.0));
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
		set_register(sarg1, int32_t(zc::math::ArcCos(temp) * 10000.0));
	else
	{
		Z_scripterrlog("Script attempted to pass %f into ArcCos!\n",temp);
		set_register(sarg1, -10000);
	}
}

void do_arctan()
{
	double xpos = GET_D(rINDEX) / 10000.0;
	double ypos = GET_D(rINDEX2) / 10000.0;

	set_register(sarg1, int32_t(zc::math::ArcTan2(ypos, xpos) * 10000.0));
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
		set_register(sarg1, int32_t(zc::math::Log10(temp) * 10000.0));
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
		set_register(sarg1, int32_t(zc::math::Ln(temp) * 10000.0));
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
	SET_D(rEXP1, wrap_zslong_rad(SH::get_arg(sarg1, v)));
}
void do_wrap_deg(const bool v)
{
	SET_D(rEXP1, wrap_zslong_deg(SH::get_arg(sarg1, v)));
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

//Returns the system Real-Time-Clock value.
void FFScript::getRTC()
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
	
	set_register(destreg, int32_t(zc::math::Pow(temp2, temp) * 10000.0));
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
	
	set_register(destreg, int32_t(zc::math::Pow(temp2, temp)));
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
	
	set_register(sarg1, int32_t(zc::math::Pow(temp2, temp) * 10000.0));
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
	// Never implemented
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
	SET_D(rEXP1, text_height(get_zc_font(font))*10000);
}

void do_strwidth()
{
	int32_t strptr = get_register(sarg1);
	int32_t font = get_register(sarg2)/10000;
	string the_string;
	ArrayH::getString(strptr, the_string, 512);
	SET_D(rEXP1, text_length(get_zc_font(font), the_string.c_str())*10000);
}

void do_charwidth()
{
	char chr = get_register(sarg1)/10000;
	int32_t font = get_register(sarg2)/10000;
	char *cstr = new char[2];
	cstr[0] = chr;
	cstr[1] = '\0';
	SET_D(rEXP1, text_length(get_zc_font(font), cstr)*10000);
	delete[] cstr;
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
	int32_t x = int32_t(GET_D(rINDEX) / 10000);
	int32_t y = int32_t(GET_D(rINDEX2) / 10000);
	
	set_register(sarg1, (_walkflag(x, y, 1) ? 10000 : 0));
}

void do_mapdataissolid()
{
	int32_t x = int32_t(GET_D(rINDEX) / 10000);
	int32_t y = int32_t(GET_D(rINDEX2) / 10000);

	auto result = decode_mapdata_ref(GET_REF(mapdataref));
	if (!result.scr)
	{
		scripting_log_error_with_context("mapdata pointer is either invalid or uninitialised");
		set_register(sarg1,10000);
	}
	else
	{
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
			std::array<const mapscr*, 7> screens;
			for (int lyr = 0; lyr < 7; ++lyr)
			{
				screens[lyr] = GetScrollingMapscr(lyr, x, y);
				if (lyr && !screens[lyr]->valid)
					screens[lyr] = screens[0];
			}
			bool result = _walkflag_new(screens, x, y, 0_zf, true);
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
	int32_t x = int32_t(GET_D(rINDEX) / 10000);
	int32_t y = int32_t(GET_D(rINDEX2) / 10000);
	int32_t layer = int32_t(GET_D(rEXP1) / 10000);

	auto result = decode_mapdata_ref(GET_REF(mapdataref));
	if (!result.scr)
	{
		scripting_log_error_with_context("mapdata pointer is either invalid or uninitialised");
		set_register(sarg1,10000);
	}
	else
	{
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
	int32_t x = int32_t(GET_D(rINDEX) / 10000);
	int32_t y = int32_t(GET_D(rINDEX2) / 10000);
	int32_t layer = int32_t(GET_D(rEXP1) / 10000);

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
	
	mapscr* scr = get_scr(GET_REF(screenref));
		
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

	mapscr* scr = get_scr(GET_REF(screenref));
		
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
	
	set_register(sarg1, get_scr(GET_REF(screenref))->sidewarpdmap[warp]*10000);
}

void do_getsidewarpscr(const bool v)
{
	int32_t warp = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkBounds(warp, -1, 3) != SH::_NoError)
	{
		set_register(sarg1, -10000);
		return;
	}
	
	set_register(sarg1, get_scr(GET_REF(screenref))->sidewarpscr[warp]*10000);
}

void do_getsidewarptype(const bool v)
{
	int32_t warp = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkBounds(warp, -1, 3) != SH::_NoError)
	{
		set_register(sarg1, -10000);
		return;
	}
	
	set_register(sarg1, get_scr(GET_REF(screenref))->sidewarptype[warp]*10000);
}

void do_gettilewarpdmap(const bool v)
{
	int32_t warp = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkBounds(warp, -1, 3) != SH::_NoError)
	{
		set_register(sarg1, -10000);
		return;
	}
	
	set_register(sarg1, get_scr(GET_REF(screenref))->tilewarpdmap[warp]*10000);
}

void do_gettilewarpscr(const bool v)
{
	int32_t warp = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkBounds(warp, -1, 3) != SH::_NoError)
	{
		set_register(sarg1, -10000);
		return;
	}
	
	set_register(sarg1, get_scr(GET_REF(screenref))->tilewarpscr[warp]*10000);
}

void do_gettilewarptype(const bool v)
{
	int32_t warp = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkBounds(warp, -1, 3) != SH::_NoError)
	{
		set_register(sarg1, -10000);
		return;
	}
	
	set_register(sarg1, get_scr(GET_REF(screenref))->tilewarptype[warp]*10000);
}

void do_layerscreen()
{
	int32_t layer = (get_register(sarg2) / 10000) - 1;
	
	if(BC::checkBounds(layer, 0, 5) != SH::_NoError || get_scr(GET_REF(screenref))->layermap[layer] == 0)
		set_register(sarg1, -10000);
	else
		set_register(sarg1, get_scr(GET_REF(screenref))->layerscreen[layer] * 10000);
}

void do_layermap()
{
	int32_t layer = (get_register(sarg2) / 10000) - 1;
	
	if(BC::checkBounds(layer, 0, 5) != SH::_NoError || get_scr(GET_REF(screenref))->layermap[layer] == 0)
		set_register(sarg1, -10000);
	else
		set_register(sarg1, get_scr(GET_REF(screenref))->layermap[layer] * 10000);
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
	int32_t ref = (GET_D(rEXP1));
	int32_t xpos  = GET_D(rINDEX2) / 10000;
	int32_t ypos = (GET_D(rINDEX) / 10000);

	BITMAP *bitty = FFCore.GetScriptBitmap(ref, screen);

	const bool brokenOffset= ( (get_er(er_BITMAPOFFSET)!=0) || (get_qr(qr_BITMAPOFFSETFIX)!=0) );
	if(!brokenOffset && (ref-10) == -1 )
	{
		ypos += 56; //should this be -56?
	}
	else
	{
		ypos += 0;
	}

	if(!bitty)
	{
		bitty = scrollbuf;
	}

	// Note: getpixel will return -1 when out of bounds.
	if (!is_inside_bitmap(bitty, xpos, ypos, false))
		Z_scripterrlog("Invalid coordinate for getpixel. Bitmap: %dx%d, pixel: %dx%d\n", bitty->w, bitty->h, xpos, ypos);
	
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
	if(LwpnH::loadWeapon(GET_REF(lwpnref)) == SH::_NoError)
	{
		auto w = LwpnH::getWeapon();
		if (!w->angular)
		{
			double vx;
			double vy;
			switch(NORMAL_DIR(w->dir))
			{
				case l_up:
				case l_down:
				case left:
					vx = -1.0*w->step;
					break;
				case r_down:
				case r_up:
				case right:
					vx = w->step;
					break;
					
				default:
					vx = 0;
					break;
			}
			switch(NORMAL_DIR(w->dir))
			{
				case l_up:
				case r_up:
				case up:
					vy = -1.0*w->step;
					break;
				case l_down:
				case r_down:
				case down:
					vy = w->step;
					break;
					
				default:
					vy = 0;
					break;
			}
			w->angular = true;
			w->angle=zc::math::ArcTan2(vy, vx);
			w->step=FFCore.Distance(0, 0, vx, vy)/10000.0;
			w->doAutoRotate();
		}
	}
}

void do_lwpnmakedirectional()
{
	if(LwpnH::loadWeapon(GET_REF(lwpnref)) == SH::_NoError)
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
	if(EwpnH::loadWeapon(GET_REF(ewpnref)) == SH::_NoError)
	{
		auto w = EwpnH::getWeapon();
		if (!w->angular)
		{
			double vx;
			double vy;
			switch(NORMAL_DIR(w->dir))
			{
				case l_up:
				case l_down:
				case left:
					vx = -1.0*w->step;
					break;
				case r_down:
				case r_up:
				case right:
					vx = w->step;
					break;
					
				default:
					vx = 0;
					break;
			}
			switch(NORMAL_DIR(w->dir))
			{
				case l_up:
				case r_up:
				case up:
					vy = -1.0*w->step;
					break;
				case l_down:
				case r_down:
				case down:
					vy = w->step;
					break;
					
				default:
					vy = 0;
					break;
			}
			w->angular = true;
			w->angle=zc::math::ArcTan2(vy, vx);
			w->step=FFCore.Distance(0, 0, vx, vy)/10000.0;
			w->doAutoRotate();
		}
	}
}

void do_ewpnmakedirectional()
{
	if(EwpnH::loadWeapon(GET_REF(lwpnref)) == SH::_NoError)
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
		
	if(LwpnH::loadWeapon(GET_REF(lwpnref)) == SH::_NoError)
		LwpnH::getWeapon()->LOADGFX(ID);
}

void do_ewpnusesprite(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkWeaponMiscSprite(ID) != SH::_NoError)
		return;
		
	if(EwpnH::loadWeapon(GET_REF(ewpnref)) == SH::_NoError)
		EwpnH::getWeapon()->LOADGFX(ID);
}

void do_portalusesprite()
{
	int32_t ID = get_register(sarg1) / 10000;
	
	if(BC::checkWeaponMiscSprite(ID) != SH::_NoError)
		return;
	
	if(portal* p = checkPortal(GET_REF(portalref)))
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
		ri->lwpnref = 0; //MAX_DWORD; //Now NULL
	else
	{
		ri->lwpnref = Lwpns.spr(index)->getUID();
		// This is too trivial to log. -L
	}
}

void do_loadeweapon(const bool v)
{
	int32_t index = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkEWeaponIndex(index) != SH::_NoError)
		ri->ewpnref = 0; //MAX_DWORD; //Now NULL
	else
	{
		ri->ewpnref = Ewpns.spr(index)->getUID();
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
		ri->itemdataref = -1; //new null value
		return;
	}
	ri->itemdataref = ID;
}

void do_loadnpc(const bool v)
{
	int32_t index = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkGuyIndex(index) != SH::_NoError)
		ri->npcref = 0; // MAX_DWORD;
	else
	{
		ri->npcref = guys.spr(index)->getUID();
	}
}

void FFScript::do_loaddmapdata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if ( ID < 0 || ID > 511 )
	{
		Z_scripterrlog("Invalid DMap ID passed to Game->LoadDMapData(): %d\n", ID);
		ri->dmapdataref = MAX_DWORD;
	}
	else ri->dmapdataref = ID;
}

void FFScript::do_load_active_subscreendata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if(ID == -1 || (unsigned(ID) < subscreens_active.size() && unsigned(ID) < 256))
	{
		ri->subscreendataref = get_subref(ID, sstACTIVE);
	}
	else
	{
		Z_scripterrlog("Invalid Subscreen ID passed to Game->LoadASubData(): %d\n", ID);
		ri->subscreendataref = 0;
	}
	SET_D(rEXP1, ri->subscreendataref);
}
void FFScript::do_load_passive_subscreendata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if(ID == -1 || (unsigned(ID) < subscreens_passive.size() && unsigned(ID) < 256))
	{
		ri->subscreendataref = get_subref(ID, sstPASSIVE);
	}
	else
	{
		Z_scripterrlog("Invalid Subscreen ID passed to Game->LoadPSubData(): %d\n", ID);
		ri->subscreendataref = 0;
	}
	SET_D(rEXP1, ri->subscreendataref);
}
void FFScript::do_load_overlay_subscreendata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if(ID == -1 || (unsigned(ID) < subscreens_overlay.size() && unsigned(ID) < 256))
	{
		ri->subscreendataref = get_subref(ID, sstOVERLAY);
	}
	else
	{
		Z_scripterrlog("Invalid Subscreen ID passed to Game->LoadOSubData(): %d\n", ID);
		ri->subscreendataref = 0;
	}
	SET_D(rEXP1, ri->subscreendataref);
}
void FFScript::do_load_map_subscreendata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if(ID == -1 || (unsigned(ID) < subscreens_map.size() && unsigned(ID) < 256))
	{
		ri->subscreendataref = get_subref(ID, sstMAP);
	}
	else
	{
		Z_scripterrlog("Invalid Subscreen ID passed to Game->LoadMSubData(): %d\n", ID);
		ri->subscreendataref = 0;
	}
	SET_D(rEXP1, ri->subscreendataref);
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
		case sstMAP:
			do_load_map_subscreendata(v);
			break;
		default:
		{
			Z_scripterrlog("Invalid Subscreen Type passed to ???: %d\n", ty);
			ri->subscreendataref = 0;
			break;
		}
	}
	SET_D(rEXP1, ri->subscreendataref);
}

void FFScript::do_loadrng()
{
	auto rng = user_rngs.create();
	if (!rng)
	{
		SET_D(rEXP1, 0);
		return;
	}

	int q = script_object_ids_by_type[script_object_type::rng].size() - 1;
	rng->gen = &script_rnggens[q];
	ri->rngref = rng->id;
	SET_D(rEXP1, rng->id);
}

uint32_t FFScript::get_new_weapondata(wdata_type data_type, dword data_index)
{
	return (ri->weapdataref = make_new_user_weapondata(data_type, data_index));
}

void FFScript::do_loadstack()
{
	ri->stackref = user_stacks.get_free();
	SET_D(rEXP1, ri->stackref);
}

void FFScript::do_loaddropset(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if ( ID < 0 || ID > MAXITEMDROPSETS )
	{
		scripting_log_error_with_context("Invalid Dropset ID: {}", ID);
		ri->dropsetdataref = MAX_DWORD;
	}
		
	else ri->dropsetdataref = ID;
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
	SET_D(rEXP1, ri->paldataref);
}

void FFScript::do_create_paldata_clr()
{
	ri->paldataref = user_paldatas.get_free();
	if (ri->paldataref > 0)
	{
		user_paldata& pd = user_paldatas[GET_REF(paldataref)];
		int32_t clri = get_register(sarg1);

		RGB c = _RGB((clri >> 16) & 0xFF, (clri >> 8) & 0xFF, clri & 0xFF);

		c.r = vbound(c.r, 0, scripting_max_color_val);
		c.g = vbound(c.g, 0, scripting_max_color_val);
		c.b = vbound(c.b, 0, scripting_max_color_val);

		for (int32_t q = 0; q < 240; ++q)
			pd.set_color(q, c);
	}
	SET_D(rEXP1, ri->paldataref);
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

	SET_D(rEXP1, (r << 16) | (g << 8) | b);
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

	SET_D(rEXP1, (r << 16) | (g << 8) | b);
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

	SET_D(rEXP1, (r << 16) | (g << 8) | b);
}

void FFScript::do_convert_from_rgb()
{
	int32_t buf = SH::read_stack(ri->sp + 2);
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
	int32_t buf = SH::read_stack(ri->sp + 1);
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

	SET_D(rEXP1, (c.r << 16) | (c.g << 8) | c.b);
}

void FFScript::do_paldata_load_level()
{
	if (user_paldata* pd = checkPalData(GET_REF(paldataref)))
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
	if (user_paldata* pd = checkPalData(GET_REF(paldataref)))
	{
		int32_t page = get_register(sarg1) / 10000;

		int32_t pageoffset = 0;
		switch (page)
		{
		case 0: pageoffset += 0;  break;
		case 1: pageoffset += 15; break;
		default:
			scripting_log_error_with_context("Invalid page: {}. Valid pages are 0 or 1. Aborting.\n", page);
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
	if (user_paldata* pd = checkPalData(GET_REF(paldataref)))
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
	if (user_paldata* pd = checkPalData(GET_REF(paldataref)))
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
	if (user_paldata* pd = checkPalData(GET_REF(paldataref)))
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
			util::regulate_path(str);
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
	if (user_paldata* pd = checkPalData(GET_REF(paldataref)))
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
	if (user_paldata* pd = checkPalData(GET_REF(paldataref)))
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
	if (user_paldata* pd = checkPalData(GET_REF(paldataref)))
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
	if (user_paldata* pd = checkPalData(GET_REF(paldataref)))
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
	if (user_paldata* pd = checkPalData(GET_REF(paldataref)))
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
	if (user_paldata* pd = checkPalData(GET_REF(paldataref)))
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
	if (user_paldata* pd = checkPalData(GET_REF(paldataref)))
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
	if (user_paldata* pd = checkPalData(GET_REF(paldataref)))
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
	if (user_paldata* pd = checkPalData(GET_REF(paldataref)))
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
	if (user_paldata* pd = checkPalData(GET_REF(paldataref)))
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
	if (user_paldata* pd = checkPalData(GET_REF(paldataref)))
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
			return -1;
		}
		if (!get_bit(pd->colors_used, ind))
		{
			scripting_log_error_with_context("Tried to access unused color {}.", ind);
			return -1;
		}
		switch (c)
		{
			case 0:
				return pd->colors[ind].r;
			case 1:
				return pd->colors[ind].g;
			case 2:
				return pd->colors[ind].b;
		}
	}

	return -1;
}

void FFScript::do_paldata_setrgb(user_paldata* pd, int32_t index, int32_t val, int32_t c)
{
	if (pd)
	{
		int32_t ind = index;
		if (unsigned(ind) >= PALDATA_NUM_COLORS)
		{
			scripting_log_error_with_context("Invalid color index ({}). Valid indices are 0-255. Aborting.\n", ind);
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
	if (user_paldata* pd = checkPalData(GET_REF(paldataref)))
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
	if (user_paldata* pd = checkPalData(GET_REF(paldataref)))
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
		if (r > 0.04045) r = zc::math::Pow(((r + 0.055) / 1.055), 2.4);
		else r /= 12.92;
		if (g > 0.04045) g = zc::math::Pow(((g + 0.055) / 1.055), 2.4);
		else g /= 12.92;
		if (b > 0.04045) b = zc::math::Pow(((b + 0.055) / 1.055), 2.4);
		else b /= 12.92;

		double x = r * 0.4124 + g * 0.3576 + b * 0.1805;
		double y = r * 0.2126 + g * 0.7152 + b * 0.0722;
		double z = r * 0.0193 + g * 0.1192 + b * 0.9505;

		if (x > 0.008856) x = zc::math::Pow(x, 1.0 / 3.0);
		else x = (7.787 * x) + (16.0 / 116.0);
		if (y > 0.008856) y = zc::math::Pow(y, 1.0 / 3.0);
		else y = (7.787 * y) + (16.0 / 116.0);
		if (z > 0.008856) z = zc::math::Pow(z, 1.0 / 3.0);
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
		if (r > 0.04045) r = zc::math::Pow(((r + 0.055) / 1.055), 2.4);
		else r /= 12.92;
		if (g > 0.04045) g = zc::math::Pow(((g + 0.055) / 1.055), 2.4);
		else g /= 12.92;
		if (b > 0.04045) b = zc::math::Pow(((b + 0.055) / 1.055), 2.4);
		else b /= 12.92;

		double x = r * 0.4124 + g * 0.3576 + b * 0.1805;
		double y = r * 0.2126 + g * 0.7152 + b * 0.0722;
		double z = r * 0.0193 + g * 0.1192 + b * 0.9505;

		if (x > 0.008856) x = zc::math::Pow(x, 1.0 / 3.0);
		else x = (7.787 * x) + (16.0 / 116.0);
		if (y > 0.008856) y = zc::math::Pow(y, 1.0 / 3.0);
		else y = (7.787 * y) + (16.0 / 116.0);
		if (z > 0.008856) z = zc::math::Pow(z, 1.0 / 3.0);
		else z = (7.787 * z) + (16.0 / 116.0);

		double CIEL = (116 * y) - 16;
		double CIEa = 500 * (x - y);
		double CIEb = 200 * (y - z);

		double h = zc::math::ArcTan2(CIEb, CIEa);
		if (h > 0) h = (h / PI) * 180;
		else h = 360 - (abs(h) / PI) * 180;

		double CIEC = sqrt(zc::math::Pow(CIEa, 2) + zc::math::Pow(CIEb, 2));

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

		if (zc::math::Pow(var_x, 3) > 0.008856) var_x = zc::math::Pow(var_x, 3);
		else var_x = (var_x - 16.0 / 116.0) / 7.787;
		if (zc::math::Pow(var_y, 3) > 0.008856) var_y = zc::math::Pow(var_y, 3);
		else var_y = (var_y - 16.0 / 116.0) / 7.787;
		if (zc::math::Pow(var_z, 3) > 0.008856) var_z = zc::math::Pow(var_z, 3);
		else var_z = (var_z - 16.0 / 116.0) / 7.787;

		r = var_x * 3.2406 + var_y * -1.5372 + var_z * -0.4986;
		g = var_x * -0.9689 + var_y * 1.8758 + var_z * 0.0415;
		b = var_x * 0.0557 + var_y * -0.2040 + var_z * 1.0570;

		if (r > 0.0031308) r = 1.055 * zc::math::Pow(r, (1 / 2.4)) - 0.055;
		else r = 12.92 * r;
		if (g > 0.0031308) g = 1.055 * zc::math::Pow(g, (1 / 2.4)) - 0.055;
		else g = 12.92 * g;
		if (b > 0.0031308) b = 1.055 * zc::math::Pow(b, (1 / 2.4)) - 0.055;
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
		double CIEa = zc::math::Cos((arr[2] * PI) / 180.0) * arr[1];
		double CIEb = zc::math::Sin((arr[2] * PI) / 180.0) * arr[1];

		double var_y = (CIEL + 16) / 116.0;
		double var_x = CIEa / 500.0 + var_y;
		double var_z = var_y - CIEb / 200.0;

		if (zc::math::Pow(var_y, 3) > 0.008856) var_y = zc::math::Pow(var_y, 3);
		else var_y = (var_y - 16.0 / 116.0) / 7.787;
		if (zc::math::Pow(var_x, 3) > 0.008856) var_x = zc::math::Pow(var_x, 3);
		else var_x = (var_x - 16.0 / 116.0) / 7.787;
		if (zc::math::Pow(var_z, 3) > 0.008856) var_z = zc::math::Pow(var_z, 3);
		else var_z = (var_z - 16.0 / 116.0) / 7.787;

		r = var_x * 3.2406 + var_y * -1.5372 + var_z * -0.4986;
		g = var_x * -0.9689 + var_y * 1.8758 + var_z * 0.0415;
		b = var_x * 0.0557 + var_y * -0.2040 + var_z * 1.0570;

		if (r > 0.0031308) r = 1.055 * zc::math::Pow(r, (1 / 2.4)) - 0.055;
		else r = 12.92 * r;
		if (g > 0.0031308) g = 1.055 * zc::math::Pow(g, (1 / 2.4)) - 0.055;
		else g = 12.92 * g;
		if (b > 0.0031308) b = 1.055 * zc::math::Pow(b, (1 / 2.4)) - 0.055;
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
	int32_t ID = GET_REF(itemdataref);
	if(invalid_item_id(ID))
		return;
	int32_t arrayptr = get_register(sarg1);
	if(setter)
	{
		ArrayH::getString(arrayptr, itemsbuf[ID].display_name, 255);
	}
	else
	{
		if(ArrayH::setArray(arrayptr, itemsbuf.get(ID).display_name) == SH::_Overflow)
			Z_scripterrlog("Array supplied to 'itemdata->GetDisplayName()' not large enough\n");
	}
}
void item_shown_name()
{
	int32_t ID = GET_REF(itemdataref);
	if(invalid_item_id(ID))
		return;
	int32_t arrayptr = get_register(sarg1);
	if(ArrayH::setArray(arrayptr, itemsbuf.get(ID).get_name()) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'itemdata->GetShownName()' not large enough\n");
}

void FFScript::do_getDMapData_dmapname([[maybe_unused]] const bool v)
{
	int32_t ID = GET_REF(dmapdataref);
	int32_t arrayptr = get_register(sarg1);
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	if(ArrayH::setArray(arrayptr, string(DMaps[ID].name)) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'dmapdata->GetName()' not large enough\n");
}

void FFScript::do_setDMapData_dmapname([[maybe_unused]] const bool v)
{
	int32_t ID = GET_REF(dmapdataref);
	int32_t arrayptr = get_register(sarg1);

	string filename_str;
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
		
	ArrayH::getString(arrayptr, filename_str, 22);
	strncpy(DMaps[ID].name, filename_str.c_str(), 21);
	DMaps[ID].name[20]='\0';
}

void FFScript::do_getDMapData_dmaptitle([[maybe_unused]] const bool v)
{
	int32_t ID = GET_REF(dmapdataref);
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

void FFScript::do_setDMapData_dmaptitle([[maybe_unused]] const bool v)
{
	int32_t ID = GET_REF(dmapdataref);
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

void FFScript::do_getDMapData_dmapintro([[maybe_unused]] const bool v)
{
	int32_t ID = GET_REF(dmapdataref);
	int32_t arrayptr = get_register(sarg1);
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	if(ArrayH::setArray(arrayptr, string(DMaps[ID].intro)) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'dmapdata->GetIntro()' not large enough\n");
}

void FFScript::do_setDMapData_dmapintro([[maybe_unused]] const bool v)
{
	int32_t ID = GET_REF(dmapdataref);
	int32_t arrayptr = get_register(sarg1);
	string filename_str;
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
		
	ArrayH::getString(arrayptr, filename_str, 73);
	strncpy(DMaps[ID].intro, filename_str.c_str(), 72);
	DMaps[ID].intro[72]='\0';
}

void FFScript::do_getDMapData_music([[maybe_unused]] const bool v)
{
	int32_t ID = GET_REF(dmapdataref);
	int32_t arrayptr = get_register(sarg1);
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	string path;
	if (auto* amus = checkMusic(find_or_make_dmap_music(ID)))
		path = amus->enhanced.path;
	
	if(ArrayH::setArray(arrayptr, path) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'dmapdata->GetMusic()' not large enough\n");
}

void FFScript::do_setDMapData_music([[maybe_unused]] const bool v)
{
	int32_t ID = GET_REF(dmapdataref);
	int32_t arrayptr = get_register(sarg1);
	string filename_str;
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
	
	ArrayH::getString(arrayptr, filename_str, 256);
	if (auto* amus = checkMusic(find_or_make_dmap_music(ID)))
		amus->enhanced.path = filename_str;
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
		ri->msgdataref = MAX_DWORD;
	}
		
	else ri->msgdataref = ID;
}
//same syntax as loadmessage data
//the input is an array
void FFScript::do_messagedata_setstring([[maybe_unused]] const bool v)
{
	int32_t arrayptr = get_register(sarg1);
	int32_t ID = GET_REF(msgdataref);
	if(BC::checkMessage(ID) != SH::_NoError)
		return;
	
	std::string s;
	ArrayH::getString(arrayptr, s, MSG_NEW_SIZE);

	auto encoding_type = get_qr(qr_OLD_SCRIPTS_MESSAGE_DATA_BINARY_ENCODING) ?
		MsgStr::EncodingType::Binary :
		MsgStr::EncodingType::Ascii;
	MsgStrings[ID].set(s, encoding_type);
}
void FFScript::do_messagedata_getstring([[maybe_unused]] const bool v)
{
	int32_t ID = GET_REF(msgdataref);
	int32_t arrayptr = get_register(sarg1);
	
	if(BC::checkMessage(ID) != SH::_NoError)
		return;

	if (get_qr(qr_OLD_SCRIPTS_MESSAGE_DATA_BINARY_ENCODING))
		MsgStrings[ID].ensureLegacyEncoding();
	else
		MsgStrings[ID].ensureAsciiEncoding();

	if(ArrayH::setArray(arrayptr, MsgStrings[ID].s, true) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'messagedata->Get()' not large enough\n");
}

void FFScript::do_loadcombodata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if ( (unsigned)ID > (MAXCOMBOS-1) )
	{
		scripting_log_error_with_context("Invalid combodata ID: {}", ID);
		ri->combodataref = 0;
	}

	else ri->combodataref = ID;
}

void FFScript::do_loadmapdata_tempscr(const bool v)
{
	int32_t layer = SH::get_arg(sarg1, v) / 10000;

	if (BC::checkBounds(layer, 0, 6) != SH::_NoError)
	{
		ri->mapdataref = 0;
		set_register(sarg1, ri->mapdataref);
		return;
	}

	ri->mapdataref = create_mapdata_temp_ref(mapdata_type::TemporaryCurrentRegion, cur_screen, layer);
	set_register(sarg1, ri->mapdataref);
}

void FFScript::do_loadmapdata_tempscr2(const bool v)
{
	int32_t layer = SH::get_arg(sarg1, v) / 10000;
	int32_t screen = SH::get_arg(sarg2, v) / 10000;

	if (BC::checkBounds(layer, 0, 6) != SH::_NoError)
	{
		ri->mapdataref = 0;
		set_register(sarg1, ri->mapdataref);
		return;
	}

	if (!is_in_current_region(screen))
	{
		scripting_log_error_with_context("Must use a screen in the current region. got: {}", screen);
		ri->mapdataref = 0;
		set_register(sarg1, ri->mapdataref);
		return;
	}

	ri->mapdataref = create_mapdata_temp_ref(mapdata_type::TemporaryCurrentScreen, screen, layer);
	set_register(sarg1, ri->mapdataref);
}

static void do_loadtmpscrforcombopos(const bool v)
{
	int32_t layer = SH::get_arg(sarg1, v) / 10000;
	rpos_t rpos = (rpos_t)(SH::get_arg(sarg2, v) / 10000);

	if (BC::checkBoundsRpos(rpos, (rpos_t)0, region_max_rpos) != SH::_NoError)
	{
		ri->mapdataref = 0;
		set_register(sarg1, ri->mapdataref);
		return;
	}
	if (BC::checkBounds(layer, 0, 6) != SH::_NoError)
	{
		ri->mapdataref = 0;
		set_register(sarg1, ri->mapdataref);
		return;
	}

	set_register(sarg1, create_mapdata_temp_ref(mapdata_type::TemporaryCurrentScreen, get_screen_for_rpos(rpos), layer));
}

void FFScript::do_loadmapdata_scrollscr(const bool v)
{
	int32_t layer = SH::get_arg(sarg1, v) / 10000;

	if (BC::checkBounds(layer, 0, 6) != SH::_NoError)
	{
		ri->mapdataref = 0;
		set_register(sarg1, ri->mapdataref);
		return;
	}

	ri->mapdataref = create_mapdata_temp_ref(mapdata_type::TemporaryScrollingRegion, scrolling_hero_screen, layer);
	set_register(sarg1, ri->mapdataref);
}

void FFScript::do_loadmapdata_scrollscr2(const bool v)
{
	int32_t layer = SH::get_arg(sarg1, v) / 10000;
	int32_t screen = SH::get_arg(sarg2, v) / 10000;

	if (BC::checkBounds(layer, 0, 6) != SH::_NoError)
	{
		ri->mapdataref = 0;
		set_register(sarg1, ri->mapdataref);
		return;
	}

	if (!is_in_screenscrolling_region(screen))
	{
		scripting_log_error_with_context("Must use a screen in the current scrolling region. got: {}", screen);
		ri->mapdataref = 0;
		set_register(sarg1, ri->mapdataref);
		return;
	}

	ri->mapdataref = create_mapdata_temp_ref(mapdata_type::TemporaryScrollingScreen, screen, layer);
	set_register(sarg1, ri->mapdataref);
}
	
void FFScript::do_loadshopdata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if ( (unsigned)ID > 255 )
	{
		Z_scripterrlog("Invalid Shop ID passed to Game->LoadShopData: %d\n", ID);
		ri->shopdataref = 0;
	}	
	else ri->shopdataref = ID;
}


void FFScript::do_loadinfoshopdata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if ( (unsigned)ID > 255 )
	{
		Z_scripterrlog("Invalid Shop ID passed to Game->LoadShopData: %d\n", ID);
		ri->shopdataref = 0;
	}	
	else ri->shopdataref = ID+NUMSHOPS;
}

void FFScript::do_loadspritedata(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if ( (unsigned)ID > (MAXSPRITES-1) )
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
		weapon *w = (weapon*)Lwpns.spr(Lwpns.Count()-1); //last created
		ri->lwpnref = w->getUID();
		w->screen_spawned = ri->screenref;
		w->ScriptGenerated = 1;
		w->isLWeapon = 1;
		if(ID == wWind) w->specialinfo = 1;
		Z_eventlog("Script created lweapon %d with UID = %u\n", ID, ri->lwpnref);
	}
	else
	{
		ri->lwpnref = 0; // Now NULL
		Z_scripterrlog("Couldn't create lweapon %d, screen lweapon limit reached\n", ID);
	}
}

void do_createeweapon(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if (ID >= wMax && replay_is_active() && replay_get_meta_str("qst") == "terror_of_necromancy_demo5.qst")
		ID = wScript10;
		
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
			ri->ewpnref = w->getUID();
			Z_eventlog("Script created eweapon %d with UID = %u\n", ID, ri->ewpnref);
		}
		else
		{
			Z_scripterrlog("Couldn't create eweapon: Invalid ID/Type (%d) specified.\n", ID);
			return;
		}
	}
	else
	{
		ri->ewpnref = 0;
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
		Z_eventlog("Script created item \"%s\" with UID = %u\n", itemsbuf.get(ID).name.c_str(), ri->itemref);
	}
	else
	{
		ri->itemref = 0;
		Z_scripterrlog("Couldn't create item \"%s\", screen item limit reached\n", itemsbuf.get(ID).name.c_str());
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
		ri->npcref = 0;
		Z_scripterrlog("Couldn't create NPC \"%s\", screen NPC limit reached\n", guy_string[ID]);
	}
	else
	{
		word index = guys.Count() - numcreated; //Get the main enemy, not a segment
		ri->npcref = guys.spr(index)->getUID();
		
		for(; index<guys.Count(); index++)
		{
			enemy* e = ((enemy*)guys.spr(index));
			e->script_spawned = true;
			e->screen_spawned = ri->screenref;
		}
			
		Z_eventlog("Script created NPC \"%s\" with UID = %u\n", guy_string[ID], ri->npcref);
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
		msgstr::dismiss();
		blockpath = false;
		Hero.finishedmsg();
	}
	else
		msgstr::do_new(get_scr(GET_REF(screenref)), ID);
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
	script_drawing_commands[j].script_type = curScriptType;
	script_drawing_commands[j].script_num = curScriptNum;
	script_drawing_commands[j].script_index = curScriptIndex;
	script_drawing_commands[j].pc = ri->pc;
	script_drawing_commands[j].script_funcrun = script_funcrun;
	script_drawing_commands[j].thiskey = ri->thiskey;

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
				Z_scripterrlog("Invalid array pointer %d passed to Screen->PutPixels(). Aborting.\n", arrayptr);
				break;
			}
			int32_t sz = ArrayH::getSize(arrayptr);
			if(sz <= 0)
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
				Z_scripterrlog("Invalid array pointer %d passed to Screen->DrawTiles(). Aborting.\n", arrayptr);
				break;
			}
			int32_t sz = ArrayH::getSize(arrayptr);
			if(sz <= 0)
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
				Z_scripterrlog("Invalid array pointer %d passed to Screen->Lines(). Aborting.\n", arrayptr);
				break;
			}
			int32_t sz = ArrayH::getSize(arrayptr);
			if(sz <= 0)
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
				Z_scripterrlog("Invalid array pointer %d passed to Screen->DrawCombos(). Aborting.\n", arrayptr);
				break;
			}
			int32_t sz = ArrayH::getSize(arrayptr);
			if(sz <= 0)
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
				Z_scripterrlog("Invalid array pointer %d passed to Screen->Polygon(). Aborting.\n", arrayptr);
				break;
			}
			int32_t sz = ArrayH::getSize(arrayptr);
			if(sz <= 0)
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
			
		case DRAWMINITILE:
			set_drawing_command_args(j, 7);
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
		
		case DRAWMAPDATA:
			set_drawing_command_args(j, 5);
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
				Z_scripterrlog("Invalid array pointer %d passed to Screen->Polygon(). Aborting.\n", arrayptr);
				break;
			}
			int32_t sz = ArrayH::getSize(arrayptr);
			if(sz <= 0)
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
				util::regulate_path(user_path);
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
				util::regulate_path(user_path);
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
		case BMPDRAWMINITILE:	set_user_bitmap_command_args(j, 7); script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+7); break;
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
		case BMPDRAWMAPDATA:
			set_user_bitmap_command_args(j, 5);
			script_drawing_commands[j][DRAWCMD_BMP_TARGET] = SH::read_stack(ri->sp+5);
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
	zfix vol = vbound(zslongToFix(SH::read_stack(ri->sp + 3)), 0_zf, 100_zf);
	int32_t pan = vbound(SH::read_stack(ri->sp + 2)/10000 + 128, 0, 255);
	int32_t freq = SH::read_stack(ri->sp + 1);
	bool loop = SH::read_stack(ri->sp) / 10000;

	if (BC::checkSFXID(ID) != SH::_NoError)
		return;

	if (!restart && !sfx_is_allocated(ID))
		return;
	
	try
	{
		sfx_ex(ID, pan, loop, restart, vol, freq);
	}
	catch (zcsfx_exception const& e)
	{
		scripting_log_error_with_context("Failed to play SFX! Error: %s", e.what());
	}
}

static int get_sfx_completion()
{
	int32_t ID = get_register(sarg1) / 10000;
	if (unsigned(ID-1) >= quest_sounds.size())
		return -10000;
	auto& sound = quest_sounds[ID-1];
	if (!sound.is_playing())
		return -10000;

	uint sample_pos = sound.get_pos();
	uint32_t sample_length = sound.get_len();
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
	msgstr::clear_display_bmps();
	
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
	
	watch=loaded_guys=blockpath=false;
	freeze_holdup = false;

	activation_counters.fill({});
	for_every_base_screen_in_region([&](mapscr* scr, unsigned int, unsigned int) {
		get_screen_state(scr->screen).loaded_enemies = false;
	});

	sle_clk=0;
	
	if(usebombpal)
	{
		memcpy(RAMpal, tempbombpal, PAL_SIZE*sizeof(RGB));
		refreshpal=true;
		usebombpal=false;
	}

	clear_camera_effect();
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
		case warpEffectOpen:
			if(out) closescreen();
			else openscreen();
			break;

		// Why two? See https://discord.com/channels/876899628556091432/1356867212224368640
		case warpEffectNONE:
		case warpEffectNONE2:
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
	int32_t lx = 0, ly = 0;
	if ( warpDestX < 0 )
	{
		if(DEVLOGGING) zprint("WarpEx() was set to warp return point:%d\n", warpDestY); 

		region_t region;
		int rx, ry;
		calculate_region(dest_map, dest_screen, region, rx, ry);

		if ( (unsigned)warpDestY < 4 )
		{
			lx = m->warpreturnx[warpDestY];
			ly = m->warpreturny[warpDestY];

			wx = lx + rx * 256;
			wy = ly + ry * 176;

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
				lx = (int32_t)Hero.getX() % 256;
				ly = (int32_t)Hero.getY() % 176;

				wx = lx + rx * 256;
				wy = ly + ry * 176;
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
		lx = warpDestX;
		ly = warpDestY;
		wx = rx * 256 + lx;
		wy = ry * 176 + ly;
		if ((unsigned)wx < region.width && (unsigned)wy < region.height)
		{
			// success
		}
		else
		{
			Z_scripterrlog("Invalid pixel coordinates of x = %d, y = %d, supplied to Hero->WarpEx()\n", warpDestX, warpDestY);
			return false;
		}
	}
	Hero.finish_auto_walk();
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
				Hero.set_dive(olddiveclk);
			}
			if(wassideswim)
			{
				Hero.setAction(sideswimming); FFCore.setHeroAction(sideswimming);
				Hero.set_dive(0);
			}
			doWarpEffect(warpEffect, true);
			handle_region_unload_trigger();
			warp_setup_new_dmap(dmap, false, DMaps[cur_dmap].color);
			
			lightingInstant(); // Also sets naturaldark
			int prev_screen = Hero.current_screen;
			loadscr(cur_dmap, screen + DMaps[cur_dmap].xoff, -1, overlay);

			// In the case where we did not call ALLOFF, preserve the "enemies have spawned"
			// state for the new screen.
			if (warpFlags&warpFlagDONTCLEARSPRITES)
			{
				if (get_screen_state(prev_screen).loaded_enemies)
					get_screen_state(Hero.current_screen).loaded_enemies = true;
			}
			
			Hero.x = (zfix)wx;
			Hero.y = (zfix)wy;
			update_viewport();
			
			warp_hero_auto_face(lx, ly, heroFacesDir, false);
			
			markBmap(Hero.dir^1, Hero.current_screen);
			
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
			if (!(warpFlags&warpFlagFORCECONTINUEMUSIC)) playLevelMusic();
			warp_finish_setup();
			
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
			handle_region_unload_trigger();
			warp_setup_new_dmap(dmap, true);
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

			warp_hero_auto_face(lx, ly, heroFacesDir, true);
			
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
			
			markBmap(Hero.dir^1, Hero.current_screen);
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
			
			if (!(warpFlags&warpFlagFORCECONTINUEMUSIC))playLevelMusic();
			warp_finish_setup();
			
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
			warp_update_dmap_and_level(dmap);
			
			Hero.reset_hookshot();
			
			if(!intradmap)
			{
				warp_update_last_entrance(hero_scr, lx, ly, dmap, true);
			}
			
			if(DMaps[cur_dmap].color != c)
			{
				lighting(false, true);
			}
			
			if (!(warpFlags&warpFlagFORCECONTINUEMUSIC)) playLevelMusic();
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
		
	newscr_clk=global_frame;
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
			
			if(((enemy*)guys.spr(i))->type!=eeTRAP && ((enemy*)guys.spr(i))->type!=eeSPINTILE)
			guys.spr(i)->yofs += 2;
		}
	}
	else if(wasSideview && !tmpscr_is_sideview)
	{
		for(int32_t i=0; i<guys.Count(); i++)
		{
			if(((enemy*)guys.spr(i))->type!=eeTRAP && ((enemy*)guys.spr(i))->type!=eeSPINTILE)
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
		else if (unsigned(currmidi) < MAXMIDIS)
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
		jukebox(MIDI + MIDIOFFSET_ZSCRIPT);
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
		SET_D(rEXP1, ret ? 10000 : 0);
	}
	else
	{
		string filename_str;
		char filename_char[256];
		ArrayH::getString(arrayptr, filename_str, 256);
		strncpy(filename_char, filename_str.c_str(), 255);
		filename_char[255] = '\0';
		bool ret = play_enh_music_crossfade(filename_char, qstpath, track, get_emusic_volume(), fadeoutframes, fadeinframes, fademiddleframes, startpos, true);
		SET_D(rEXP1, ret ? 10000 : 0);
	}
}

bool FFScript::can_change_music_within_region()
{
	return music_update_cond == MUSIC_UPDATE_SCREEN;
}
bool FFScript::can_dmap_change_music(int32_t dm)
{
	switch (music_update_cond)
	{
		case MUSIC_UPDATE_SCREEN:
		case MUSIC_UPDATE_REGION:
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
	string path;
	if (auto* amus = checkMusic(find_or_make_dmap_music(ID)))
		path = amus->enhanced.path;
		
	if(ArrayH::setArray(arrayptr, path) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'Game->GetDMapMusicFilename' not large enough\n");
}

void do_get_enh_music_track(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
	
	if (auto* amus = checkMusic(find_or_make_dmap_music(ID)))
		set_register(sarg1, (amus->enhanced.track+1)*10000);
	else
		set_register(sarg1, 0);
}

void do_set_dmap_enh_music([[maybe_unused]] const bool v)
{
	int32_t ID   = SH::read_stack(ri->sp + 2) / 10000;
	int32_t arrayptr = SH::read_stack(ri->sp + 1);
	int32_t track = (SH::read_stack(ri->sp + 0) / 10000)-1;
	string filename_str;
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	ArrayH::getString(arrayptr, filename_str, 256);
	if (auto* amus = checkMusic(find_or_make_dmap_music(ID)))
	{
		amus->enhanced.path = filename_str;
		amus->enhanced.track = track;
	}
}






///----------------------------------------------------------------------------------------------------//
//Array & string related

void do_arraysize()
{
	int32_t arrayptr = get_register(sarg1);
	SET_D(rEXP1, ArrayH::getSize(arrayptr) * 10000);
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
	auto itmid = GET_REF(itemdataref);
	if(invalid_item_id(itmid))
	{
		scripting_log_error_with_context("Invalid itemdata access: {}", itmid);
		return;
	}
	
	if(ArrayH::setArray(arrayptr, itemsbuf.get(itmid).name) == SH::_Overflow)
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
	SET_D(rEXP1, 10000 * (unpackbuf[y * 16 + x] + csoffs));
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
	int32_t val = ArrayH::getElement(GET_D(rINDEX), indx, can_neg_array);
	set_register(sarg1, val);
}
void do_writepod(const bool v1, const bool v2)
{
	int32_t indx = SH::get_arg(sarg1, v1) / 10000;
	int32_t val = SH::get_arg(sarg2, v2);
	auto type = (script_object_type)sarg3;
	ArrayH::setElement(GET_D(rINDEX), indx, val, can_neg_array, type);
}

int32_t jit_pod_read(int32_t arrayptr, int32_t index, int32_t pc, int32_t no_neg)
{
	ri->pc = pc;
	return ArrayH::getElement(arrayptr, index, no_neg ? false : can_neg_array);
}
void jit_pod_write(int32_t arrayptr, int32_t index, int32_t value, int32_t type, int32_t pc, int32_t no_neg)
{
	ri->pc = pc;
	ArrayH::setElement(arrayptr, index, value, no_neg ? false : can_neg_array, (script_object_type)type);
}
int32_t jit_allocatemem(int32_t size, int32_t object_type, int32_t pc)
{
	ri->pc = pc;
	return allocatemem(size, true, curScriptType, curScriptIndex, (script_object_type)object_type);
}
void jit_writepodarr(int32_t id, int32_t pc)
{
	ri->pc = pc;
	auto vec = curscript->zasm_script->zasm[pc].vecptr;
	if (!vec) return;
	ArrayH::setArray(id, vec->size(), vec->data(), false);
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

sprite* get_own_sprite(ScriptType type)
{
	switch(type)
	{
		case ScriptType::None:
			return ResolveBaseSprite(GET_REF(spriteref));
		case ScriptType::Lwpn:
			return checkLWpn(GET_REF(lwpnref));
		case ScriptType::Ewpn:
			return checkEWpn(GET_REF(ewpnref));
		case ScriptType::ItemSprite:
			return checkItem(GET_REF(itemref));
		case ScriptType::NPC:
			return checkNPC(GET_REF(npcref));
		case ScriptType::FFC:
			return ResolveFFC(GET_REF(ffcref));
	}
	return nullptr;
}

sprite* get_own_sprite(refInfo* ri, ScriptType type)
{
	switch(type)
	{
		case ScriptType::None:
			return ResolveBaseSprite(ri->spriteref);
		case ScriptType::Lwpn:
			return checkLWpn(ri->lwpnref);
		case ScriptType::Ewpn:
			return checkEWpn(ri->ewpnref);
		case ScriptType::ItemSprite:
			return checkItem(ri->itemref);
		case ScriptType::NPC:
			return checkNPC(ri->npcref);
		case ScriptType::FFC:
			return ResolveFFC(ri->ffcref);
	}
	return nullptr;
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
	if (!(type >= 0 && type <= (int)script_object_type::last))
	{
		assert(false);
	}
	// Currently only marking globals / script instance vars as objects is supported.
	if (reg >= GD(0) && reg < GD(MAX_GLOBAL_VARIABLES))
	{
		int index = reg - GD(0);
		game->global_d_types[index] = (script_object_type)type;
	}
	else if (reg >= SCRIPT_INST_VARS(0) && reg < SCRIPT_INST_VARS(MAX_SCRIPT_INST_VARIABLES))
	{
		int index = reg - SCRIPT_INST_VARS(0);
		if (type)
			ri->script_d_is_object.insert(index);
		else
			ri->script_d_is_object.erase(index);
	}
	else assert(false);
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
				ffc->scrconfig.script = 0;
			auto& data = get_script_engine_data(type, i);
			data.doscript = false;
			data.clear_ref();
		}
		break;

		case ScriptType::Screen:
			get_scr(i)->scrconfig.script = 0;
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
				spr->scrconfig.script = 0;
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
				auto& itm = itemsbuf[i];
				if ((itm.flags & item_passive_script) && game->get_item(i))
					itm.scrconfig.script = 0; //Quit perpetual scripts, too.
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

	ScriptTimingScope _script_timing_scope; // -script-timings: measure scripting-engine time

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

	script_data* next_script_data = get_script_safe(type, script);
	if (!next_script_data)
	{
		al_trace("Invalid script. type: %d num: %d\n", (int)type, (int)script);
		return RUNSCRIPT_ERROR;
	}

	auto& data = get_script_engine_data(type, i);
	set_current_script_engine_data(data, type, script, next_script_data, i);

	// Because qst.cpp likes to write script_data without setting this.
	curscript->meta.script_type = type;

	// Attribute this outermost call's time to the script by name (-script-timings).
	if (script_timings_enabled && script_timings_is_outermost())
		script_timings_set_current_name(curscript->name());

	// If script isn't valid, we don't have a `pc` to start from... just exit.
	if(!curscript->valid())
	{
		script_exit_cleanup(false);
		return RUNSCRIPT_OK;
	}

	script_funcrun = false;

	// -jit-run-lo / -jit-run-hi: bisect tool. Each run_script call gets a monotonic index; only
	// calls with index in [lo, hi) are allowed to use the JIT, all others run interpreted.
	// Binary-searching this range isolates which run_script invocation must be jitted to
	// reproduce a JIT divergence. Default hi<0 disables the limit (normal behavior).
	//
	// A script is NOT freely interchangeable between backends mid-run: the wasm backend resumes
	// from its own instance state, not ri->pc, so it refuses to adopt a script that already ran
	// interpreter slices (see jit_create_script_impl) - with -jit-run-lo, such scripts simply
	// finish their run interpreted. The reverse (a jitted script forced to the interpreter by
	// -jit-run-hi) resumes from ri->pc, which is only fully correct when the script last yielded
	// at its top level (the JIT does not maintain ri->retsp unless stack traces are on).
	static long rs_call_index_counter = 0;
	long rs_call_index = rs_call_index_counter++;
	static int jit_run_lo = get_flag_int("-jit-run-lo").value_or(0);
	static int jit_run_hi = get_flag_int("-jit-run-hi").value_or(-1);
	bool jit_bisect_allows_this_call = jit_run_hi < 0 || (rs_call_index >= jit_run_lo && rs_call_index < jit_run_hi);

	// -jit-run-log-call N: report the frame (and script) a given run_script call runs on.
	// Lets jit_runtime_debug.py, after bisecting -jit-run-hi to the failing call, target the
	// per-instruction debug at that call's frame.
	static int jit_run_log_call = get_flag_int("-jit-run-log-call").value_or(-1);
	if (jit_run_log_call >= 0 && rs_call_index == jit_run_log_call)
	{
		al_trace("[jit-run-call] call=%ld frame=%d script=%s\n", rs_call_index, replay_get_frame(),
			curscript ? curscript->name().c_str() : "?");
		fflush(stdout);
	}

	JittedScriptInstance* j_instance = nullptr;
	// When the debugger is open, run scripts through the interpreter so every command passes through
	// the per-instruction hook in run_script_int (zscript_debugger_exec). The JIT executes most
	// commands as native code and only re-enters the interpreter for uncompiled commands, so
	// breakpoints and line stepping would otherwise only land on those few PCs.
	//
	// jit_can_start_script() guards nested script runs: on the wasm backend a script that runs
	// another script mid-execution (e.g. an FFC running an enemy's script) would need a second
	// asyncify unwind on top of the outer script's, which traps. Nested scripts run interpreted.
	if (jit_is_enabled() && !zscript_debugger_is_open() && jit_can_start_script() && jit_bisect_allows_this_call)
	{
		if (!data.j_instance)
			data.j_instance = std::shared_ptr<JittedScriptInstance>(jit_create_script_instance(curscript, ri));
		j_instance = data.j_instance.get();
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
	if (j_instance)
	{
		if (ri->waitframes)
		{
			--ri->waitframes;
			result = RUNSCRIPT_OK;
		}
		else
		{
			// Retain the script instance because if deleted while running, terrible things can happen (crash),
			// as the jit runtimes often write to it. Typically a script won't delete its own script handle,
			// but scripts can run nested, so lets capture a temporary retaining reference as part of the
			// call stack.
			auto retainer = data.j_instance;
			result = jit_run_script(j_instance);

			// The wasm JIT can't run a nested frozen generic script itself (its frame
			// can't be unwound by asyncify mid-execution), so RUNGENFRZSCR yields back
			// here instead, flagged by frozen_dest_reg != -1. Run the frozen script,
			// write its result to the destination register, then resume. (On other
			// backends frozen_dest_reg stays -1 and this never runs.)
			//
			// runGenericFrozenEngine below runs a whole frozen game frame (it advances
			// the frame and runs FFCs/enemies), so scripts run nested here - the path
			// that exposed the web-JIT nested-script crash. To reach this loop:
			//
			// python tests/run_replay_tests.py --build_folder build_emscripten/Release --filter terror_of_necromancy_demo6_05_of_54.zplay
			//
			// Without jit_can_start_script() (nested scripts run interpreted) that run
			// traps with "unreachable" around frame 7998.
			while (result == RUNSCRIPT_OK && j_instance->frozen_dest_reg != -1)
			{
				int32_t dest_reg = j_instance->frozen_dest_reg;
				j_instance->frozen_dest_reg = -1;
				bool r = FFCore.runGenericFrozenEngine(word(GET_REF(genericdataref)));
				set_register(dest_reg, r ? 10000L : 0L);
				result = jit_run_script(j_instance);
			}

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
			else if (result == RUNSCRIPT_JIT_QUIT)
			{
				script_exit_cleanup(false);
				result = RUNSCRIPT_STOPPED;
			}
		}
	}
	else
	{
		result = run_script_int();
	}

	if (ZScriptVersion::gc())
	{
		// Drain the autorelease pool.
		// Take the ids by value, since destructors can possibly
		// create objects and modify `script_object_autorelease_pool`.
		auto ids = script_object_autorelease_pool_take();
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

// Run [count] number of commands (unless something errors).
int32_t run_script_jit_sequence(JittedScriptInstance* j_instance, pc_t pc, uint32_t sp, int32_t count)
{
	ri->pc = pc;
	ri->sp = sp;
	j_instance->uncompiled_command_count = count;
	j_instance->sequence_mode = true;
	j_instance->should_wait = false;
	int r = run_script_int(j_instance);
	if (r != RUNSCRIPT_OK)
		return r;

	return j_instance->should_wait ? RUNSCRIPT_STOPPED : RUNSCRIPT_OK;
}

// Run a single command.
int32_t run_script_jit_one(JittedScriptInstance* j_instance, pc_t pc, uint32_t sp)
{
	ri->pc = pc;
	ri->sp = sp;
	j_instance->uncompiled_command_count = 1;
	j_instance->sequence_mode = true;
	j_instance->should_wait = false;
	int r = run_script_int(j_instance);
	if (r != RUNSCRIPT_OK)
		return r;

	return j_instance->should_wait ? RUNSCRIPT_STOPPED : RUNSCRIPT_OK;
}

// Runs the script until the next function call, return, wait frame, or error.
int32_t run_script_jit_until_call_or_return(JittedScriptInstance* j_instance, pc_t pc, uint32_t sp)
{
	ri->pc = pc;
	ri->sp = sp;
	j_instance->uncompiled_command_count = -1;
	j_instance->sequence_mode = false;
	j_instance->should_wait = false;
	int r = run_script_int(j_instance);
	if (r != RUNSCRIPT_OK)
		return r;

	return j_instance->should_wait ? RUNSCRIPT_STOPPED : RUNSCRIPT_OK;
}

bool script_is_within_debugger_vm;
bool suppress_script_error_logging;
bool disable_script_error_logs;

// Reads a script_object_type from an opcode argument, tolerating invalid values
// (quests compiled before these arguments existed always serialized 0/none).
static script_object_type get_object_type_from_sarg(int32_t arg)
{
	if (arg > 0 && arg <= (int)script_object_type::last)
		return (script_object_type)arg;
	return script_object_type::none;
}

// When j_instance is null, that means the interperter is fully in charge.
// Otherwise, the JIT may still call this function for the many commands that are not compiled, or
// during the period before a function is "hot" enough to have been compiled.
int32_t run_script_int(JittedScriptInstance* j_instance)
{
	bool is_jitted = j_instance;
	ScriptType type = curScriptType;
	word script = curScriptNum;
	int32_t i = curScriptIndex;

	current_zasm_command=(ASM_DEFINE)0; // this is actually SETV, but we never will print that as a context string, so it's fine.

	int commands_run = 0;
	bool old_script_funcrun = script_funcrun && curscript->meta.ffscript_v < 23;
	if(!is_jitted)
	{
		if(ri->waitframes)
		{
			--ri->waitframes;
			return RUNSCRIPT_OK;
		}
		
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

	// This is used to help debug differences w/ the JIT implementation. See scripts/jit_runtime_debug.py.
	bool is_debugging = script_debug_is_runtime_debugging() == 2;
	bool increment = true;
	static std::vector<ffscript> empty_zasm = {{0xFFFF}};
	const auto& zasm = curscript->valid() ? curscript->zasm_script->zasm : empty_zasm;
	word scommand = zasm[ri->pc].command;
	bool hit_invalid_zasm = false;
	bool no_dealloc = false;
	bool has_debugger = !script_is_within_debugger_vm && zscript_debugger_is_open();
	while(scommand != 0xFFFF)
	{
		if (has_debugger)
			zscript_debugger_exec(ri->pc);

		const auto& op = zasm[ri->pc];
		scommand = op.command;
		sarg1 = op.arg1;
		sarg2 = op.arg2;
		sarg3 = op.arg3;
		sargstr = op.strptr;
		sargvec = op.vecptr;

		current_zasm_command = (ASM_DEFINE)scommand;

		if (is_debugging && (!is_jitted || !j_instance->sequence_mode || commands_run > 0))
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
			if (script_is_within_debugger_vm)
			{
				ri->waitframes = 1;
				return RUNSCRIPT_ERROR;
			}
			if (is_jitted)
				j_instance->should_wait = true;
			break;
		}
		
		numInstructions++;
		if(numInstructions==hangcount) // No need to check frequently
		{
			numInstructions=0;
			poll_keyboard();
			checkQuitKeys();
			zscript_debugger_update();
			if(Quit)
				scommand=0xFFFF;
		}

		if (script_is_within_debugger_vm && commands_run > 10000000)
			return RUNSCRIPT_INFINITE_LOOP;

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
				if (is_debugging || is_jitted)
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

				// When is_jitted is true, GOTO can only be processed here when j_instance->sequence_mode is false.
				// Track back edges (jumps to a loop head). The JIT will compiled the current
				// function if this is called enough.
				if (is_jitted && ri->pc > sarg1)
					jit_profiler_increment_function_back_edge(j_instance, sarg1);

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
				int ret_pc = ri->pc+1;
				ri->pc = sarg1;
				retstack_push(ret_pc);
				if(sarg1 < 0 )
				{
					goto_err("CALLFUNC");
					scommand = 0xFFFF;
					break;
				}

				increment = false;

				// When is_jitted is true, CALLFUNC can only be processed here when j_instance->sequence_mode is false.
				// And when it is called, it marks the end of run_script_int.
				if (is_jitted)
					j_instance->uncompiled_command_count = commands_run + 1;
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

				if (script_is_within_debugger_vm && ri->retsp == 0)
					return RUNSCRIPT_OK;

				// When is_jitted is true, RETURNFUNC can only be processed here when j_instance->sequence_mode is false.
				// And when it is called, it marks the end of run_script_int.
				if (is_jitted)
					j_instance->uncompiled_command_count = commands_run + 1;
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
				if (script_funcrun && ri->sp >= MAX_STACK_SIZE)
				{
					ri->pc = MAX_PC;
					break; //handled below. 'RETURNFUNC' does this better now.
				}
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
				do_savegamestructs(false,false);
				using_SRAM = 0;
				break;
			case READGAMESTRUCTS:
				using_SRAM = 1;
				do_loadgamestructs(false,false);
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
				SET_D(rEXP1, 10000* do_msgwidth(get_register(sarg1)/10000));
				break;
			case MESSAGEHEIGHTR:
				SET_D(rEXP1, 10000* do_msgheight(get_register(sarg1)/10000));
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
			case GETMUSICDATABYNAME:	FFCore.do_getmusicdatabyname(); break;
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
			case PUSHVARGSTACK:
				if (ri->zs_vargs_stack.size() >= 128)
				{
					scripting_log_error_with_context("vargs stack overflow!");
					break;
				}
				ri->zs_vargs_stack.emplace_back();
				ri->zs_vargs_pos_is_object.emplace_back();
				break;
			case POPVARGSTACK:
				if (ri->zs_vargs_stack.size() < 2)
				{
					scripting_log_error_with_context("vargs stack underflow!");
					break;
				}
				ri->zs_vargs_stack.pop_back();
				ri->zs_vargs_pos_is_object.pop_back();
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
				FFCore.getRTC();
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
				do_sprintf(true, false);
				break;
				
			case PRINTFVARG:
				FFCore.do_printf(true, true);
				break;
			case SPRINTFVARG:
				do_sprintf(true, true);
				break;
			case PRINTFA:
				FFCore.do_printfarr();
				break;
			case SPRINTFA:
				do_sprintfarr();
				break;
			case ARRAYPUSH:
			{
				auto ptr = SH::read_stack(ri->sp + 2);
				auto val = SH::read_stack(ri->sp + 1);
				auto indx = SH::read_stack(ri->sp + 0) / 10000;
				// The compiler provides the pushed value's object type, so untyped arrays
				// can retain objects. Old quests always serialized 0 (none) here.
				auto type = get_object_type_from_sarg(sarg1);
				ArrayManager am(ptr);
				SET_D(rEXP1, am.push(val,indx,type) ? 10000 : 0);
				break;
			}
			case ARRAYPOP:
			{
				auto ptr = SH::read_stack(ri->sp + 1);
				auto indx = SH::read_stack(ri->sp + 0) / 10000;
				ArrayManager am(ptr);
				SET_D(rEXP1, am.pop(indx));
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
				int32_t sound = GET_D(rINDEX)/10000;
				pause_sfx(sound);
				
			}
			break;

			case RESUMESFX:
			{
				int32_t sound = GET_D(rINDEX)/10000;
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
				int32_t sound = GET_D(rINDEX)/10000;
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
				if (user_paldata* pd = checkPalData(GET_REF(paldataref), true))
				{
					free_script_object(pd->id);
				}
				break;
			case PALDATAOWN:
				if (user_paldata* pd = checkPalData(GET_REF(paldataref), false))
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
				do_triggersecrets(GET_REF(screenref));
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
			case DRAWMINITILE:
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
			case DRAWMAPDATA:
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
			case BMPDRAWMINITILE:
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
			case BMPDRAWMAPDATA:
				do_drawing_command(scommand, false);
				break;
			case READBITMAP:
			{
				uint32_t bitref = SH::read_stack(ri->sp+2);
				SET_D(rEXP2, bitref);
				if(checkBitmap(bitref,false,true))
					do_drawing_command(scommand, false);
				else //If the pointer isn't allocated, attempt to allocate it first
				{
					bitref = FFCore.get_free_bitmap();
					SET_D(rEXP2, bitref); //Return to ptr
					if(bitref) SH::write_stack(ri->sp+2,bitref); //Write the ref, for the drawing command to read
					else break; //No ref allocated; don't enqueue the drawing command.
					do_drawing_command(scommand, false);
				}
				break;
			}
			case REGENERATEBITMAP:
			{
				int ref = SH::read_stack(ri->sp+3);
				SET_D(rEXP2, ref);
				if(checkBitmap(ref,false,true))
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
					
					SET_D(rEXP2, FFCore.create_user_bitmap_ex(h,w)); //Return to ptr
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
				if(FFCore.isSystemBitref(GET_REF(bitmapref)))
					break; //Don't attempt to own system bitmaps!

				if (auto bitmap = checkBitmap(GET_REF(bitmapref), false))
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
				sprite* own_sprite = get_own_sprite(own_type);
				own_script_object(b, own_sprite);
				break;
			}
			case OBJ_OWN_PALDATA:
			{
				int palid = get_register(sarg1);
				user_paldata* pd = checkPalData(palid, false);
				if(!pd) break;
				ScriptType own_type = (ScriptType)sarg2;
				sprite* own_sprite = get_own_sprite(own_type);
				own_script_object(pd, own_sprite);
				break;
			}
			case OBJ_OWN_FILE:
			{
				int fileid = get_register(sarg1);
				user_file* f = checkFile(fileid, false);
				if(!f) break;
				ScriptType own_type = (ScriptType)sarg2;
				sprite* own_sprite = get_own_sprite(own_type);
				own_script_object(f, own_sprite);
				break;
			}
			case OBJ_OWN_DIR:
			{
				int dirid = get_register(sarg1);
				user_dir* dr = checkDir(dirid, false);
				if(!dr) break;
				ScriptType own_type = (ScriptType)sarg2;
				sprite* own_sprite = get_own_sprite(own_type);
				own_script_object(dr, own_sprite);
				break;
			}
			case OBJ_OWN_STACK:
			{
				int stackid = get_register(sarg1);
				user_stack* st = checkStack(stackid, false);
				if(!st) break;
				ScriptType own_type = (ScriptType)sarg2;
				sprite* own_sprite = get_own_sprite(own_type);
				own_script_object(st, own_sprite);
				break;
			}
			case OBJ_OWN_RNG:
			{
				int rngid = get_register(sarg1);
				user_rng* r = checkRNG(rngid, false);
				if(!r) break;
				ScriptType own_type = (ScriptType)sarg2;
				sprite* own_sprite = get_own_sprite(own_type);
				own_script_object(r, own_sprite);
				break;
			}
			case OBJ_OWN_ARRAY:
			{
				int arrid = get_register(sarg1);
				ScriptType own_type = (ScriptType)sarg2;
				sprite* own_sprite = get_own_sprite(own_type);
				do_own_array(arrid, own_sprite);
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
					Z_scripterrlog("Cannot End Game while reading or writing to SRAM. Aborting End.\n");
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
					Z_scripterrlog("Cannot Reload Game while reading or writing to SRAM. Aborting Reload.\n");
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
				SET_D(rEXP1, current_item_id(ity,checkcost,checkjinx,check_bunny) * 10000);
				break;
			}
			
			case GAMECONTINUE:
				if ( using_SRAM )
				{
					Z_scripterrlog("Cannot Continue Game while reading or writing to SRAM. Aborting Continue.\n");
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
					Z_scripterrlog("Cannot Save Game while reading or writing to SRAM. Aborting Save.\n");
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
					Z_scripterrlog("Cannot Save Game while reading or writing to SRAM. Aborting Save.\n");
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
				SET_D(rEXP1, scriptloadenemies(GET_REF(screenref)) ? 10000 : 0);
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
			case FFCTRIGGER:
			{
				int32_t idx = get_register(sarg1) / 10000;
				ffcdata* ffc = ResolveFFC(GET_REF(ffcref));
				if (ffc)
					set_register(sarg1, do_trigger_combo(get_ffc_handle(ffc), idx) ? 10000 : 0);
				else
					set_register(sarg1, 0);
				break;
			}
			
			case SWITCHNPC:
			{
				byte effect = vbound(get_register(sarg1)/10000, 0, 255);
				set_register(sarg1,0);
				if(Hero.switchhookclk) break; //Already switching!
				if(GuyH::loadNPC(GET_REF(npcref)) == SH::_NoError)
				{
					switching_object = guys.getByUID(GET_REF(npcref));
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
				if(ItemH::loadItem(GET_REF(itemref)) == SH::_NoError)
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
				if(LwpnH::loadWeapon(GET_REF(lwpnref)) == SH::_NoError)
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
				if(EwpnH::loadWeapon(GET_REF(ewpnref)) == SH::_NoError)
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
				FFCore.do_warp_ex_array();
				break;
			}
			case WARPEX:
			{
				FFCore.do_warp_ex();
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
				SET_D(rEXP1, Hero.movexy(dx, dy, kb, ign_sv, shove) ? 10000 : 0);
				break;
			}
			case HEROCANMOVEXY:
			{
				zfix dx = zslongToFix(SH::read_stack(ri->sp + 4));
				zfix dy = zslongToFix(SH::read_stack(ri->sp + 3));
				bool kb = SH::read_stack(ri->sp + 2)!=0;
				bool ign_sv = SH::read_stack(ri->sp + 1)!=0;
				bool shove = SH::read_stack(ri->sp + 0)!=0;
				SET_D(rEXP1, Hero.can_movexy(dx, dy, kb, ign_sv, shove) ? 10000 : 0);
				break;
			}
			case HEROMOVEATANGLE:
			{
				zfix degrees = zslongToFix(SH::read_stack(ri->sp + 4));
				zfix pxamnt = zslongToFix(SH::read_stack(ri->sp + 3));
				bool kb = SH::read_stack(ri->sp + 2)!=0;
				bool ign_sv = SH::read_stack(ri->sp + 1)!=0;
				bool shove = SH::read_stack(ri->sp + 0)!=0;
				SET_D(rEXP1, Hero.moveAtAngle(degrees, pxamnt, kb, ign_sv, shove) ? 10000 : 0);
				break;
			}
			case HEROCANMOVEATANGLE:
			{
				zfix degrees = zslongToFix(SH::read_stack(ri->sp + 4));
				zfix pxamnt = zslongToFix(SH::read_stack(ri->sp + 3));
				bool kb = SH::read_stack(ri->sp + 2)!=0;
				bool ign_sv = SH::read_stack(ri->sp + 1)!=0;
				bool shove = SH::read_stack(ri->sp + 0)!=0;
				SET_D(rEXP1, Hero.can_moveAtAngle(degrees, pxamnt, kb, ign_sv, shove) ? 10000 : 0);
				break;
			}
			case HEROMOVE:
			{
				int dir = SH::read_stack(ri->sp + 4)/10000;
				zfix pxamnt = zslongToFix(SH::read_stack(ri->sp + 3));
				bool kb = SH::read_stack(ri->sp + 2)!=0;
				bool ign_sv = SH::read_stack(ri->sp + 1)!=0;
				bool shove = SH::read_stack(ri->sp + 0)!=0;
				SET_D(rEXP1, Hero.moveDir(dir, pxamnt, kb, ign_sv, shove) ? 10000 : 0);
				break;
			}
			case HEROCANMOVE:
			{
				int dir = SH::read_stack(ri->sp + 4)/10000;
				zfix pxamnt = zslongToFix(SH::read_stack(ri->sp + 3));
				bool kb = SH::read_stack(ri->sp + 2)!=0;
				bool ign_sv = SH::read_stack(ri->sp + 1)!=0;
				bool shove = SH::read_stack(ri->sp + 0)!=0;
				SET_D(rEXP1, Hero.can_moveDir(dir, pxamnt, kb, ign_sv, shove) ? 10000 : 0);
				break;
			}
			case HEROLIFTRELEASE:
			{
				if(Hero.lift_wpn)
				{
					SET_D(rEXP1, Hero.lift_wpn->getUID());
					Lwpns.add(Hero.lift_wpn);
					Hero.lift_wpn = nullptr;
				}
				else SET_D(rEXP1, 0);
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
				SET_D(rEXP1, Hero.is_hitflickerframe() ? 10000 : 0);
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
				ri->portalref = SET_D(rEXP1, val);
				break;
			}
			case CREATEPORTAL:
			{
				portal* p = new portal();
				if(portals.add(p))
					ri->portalref = SET_D(rEXP1, p->getUID());
				else
				{
					ri->portalref = SET_D(rEXP1, 0);
					Z_scripterrlog("Unable to create new portal! Limit reached!\n");
				}
				break;
			}
			case LOADSAVPORTAL:
			{
				auto val = get_register(sarg1)/10000;
				savedportal* prt = checkSavedPortal(val);
				ri->savportalref = SET_D(rEXP1, prt ? val : 0);
				break;
			}
			case CREATESAVPORTAL:
			{
				if(game->user_portals.size() >= MAX_SAVED_PORTALS)
				{
					ri->savportalref = SET_D(rEXP1, 0);
					Z_scripterrlog("Cannot create any more Saved Portals! Remove some first!\n");
					break;
				}
				savedportal& ref = game->user_portals.emplace_back();
				ri->savportalref = SET_D(rEXP1, ref.getUID());
				break;
			}
			case PORTALREMOVE:
			{
				extern portal mirror_portal;

				if(portal* p = checkPortal(GET_REF(portalref), true))
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
				if(savedportal* sp = checkSavedPortal(GET_REF(savportalref), true))
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
				if(savedportal* sp = checkSavedPortal(GET_REF(savportalref)))
				{
					retval = getPortalFromSaved(sp);
					if(!retval)
					{
						if(portal* p = loadportal(*sp))
							if(portals.add(p))
								retval = p->getUID();
					}
				}
				SET_D(rEXP1, retval);
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
					if(GuyH::loadNPC(GET_REF(npcref)) == SH::_NoError)
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
					if(ItemH::loadItem(GET_REF(itemref)) == SH::_NoError)
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
					if(LwpnH::loadWeapon(GET_REF(lwpnref)) == SH::_NoError)
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
					if(EwpnH::loadWeapon(GET_REF(ewpnref)) == SH::_NoError)
					{
						EwpnH::getWeapon()->explode(mode);
					}
				}
				break;
			}
				
			case BOTTLENAMEGET:
			{
				int32_t arrayptr = get_register(sarg1);
				int32_t id = GET_REF(bottletyperef)-1;
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
				int32_t id = GET_REF(bottletyperef)-1;
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
				int32_t id = GET_REF(bottleshopref)-1;
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
				int32_t id = GET_REF(bottleshopref);
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
				int32_t itemid = GET_REF(itemdataref);
				if(unsigned(itemid) > itemsbuf.capacity()) break;
				auto const& itm = itemsbuf.get(itemid);
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
						if ( itm.scrconfig.script != 0 )
						{
							if ( !data.doscript ) 
							{
								data.clear_ref();
								data.doscript = 1;
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
						if (itm.scrconfig.script != 0)
						{
							if (data.doscript != 2)
								data.doscript = 2;
						}
						break;
					}
				}
				break;
			}
			
			//Game over Screen
			case SETCONTINUESCREEN: FFScript::FFChangeSubscreenText(); break;
			case SETCONTINUESTRING: FFScript::FFSetSaveScreenSetting(); break;
			
			case LWPNDEL:
			{
				if(type == ScriptType::Lwpn && ri->lwpnref == i)
				{
					FFCore.do_lweapon_delete();
					earlyretval = RUNSCRIPT_SELFDELETE;
					break;
				}

				FFCore.do_lweapon_delete();
				break;
			}
			case EWPNDEL:
			{
				if(type == ScriptType::Ewpn && ri->ewpnref == i)
				{
					FFCore.do_eweapon_delete();
					earlyretval = RUNSCRIPT_SELFDELETE;
					break;
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
			
			//Stack
			case STACKFREE:
			{
				if(user_stack* st = checkStack(GET_REF(stackref), true))
				{
					free_script_object(st->id);
				}
				break;
			}
			case STACKOWN:
			{
				if(user_stack* st = checkStack(GET_REF(stackref)))
				{
					own_script_object(st, type, i);
				}
				break;
			}
			case STACKCLEAR:
			{
				if(user_stack* st = checkStack(GET_REF(stackref)))
				{
					std::vector<uint32_t> ids;
					st->get_retained_ids(ids);
					// Empty the stack before releasing: a release can run a ZScript
					// destructor that touches this same stack, and it must not see (and
					// re-release) the entries this snapshot already accounts for.
					st->clearStack();
					for (uint32_t id : ids)
						script_object_ref_dec(id);
				}
				break;
			}
			case STACKGET:
			{
				if(user_stack* st = checkStack(GET_REF(stackref), true))
				{
					int32_t indx = get_register(sarg1); //NOT /10000
					set_register(sarg1, st->get(indx)); //NOT *10000
				}
				else set_register(sarg1, 0L);
				break;
			}
			case STACKSET:
			{
				if(user_stack* st = checkStack(GET_REF(stackref), true))
				{
					int32_t indx = get_register(sarg1); //NOT /10000
					int32_t val = get_register(sarg2); //NOT /10000
					// The compiler provides the written value's object type, so stacks can
					// retain objects. Old quests always serialized 0 (none) here.
					auto type = get_object_type_from_sarg(sarg3);
					bool old_is_object = st->holds_object(indx);
					int32_t old_val = st->get(indx);
					if (st->set(indx, val, type)) //NOT *10000
					{
						// Increase, then decrease, to handle self-assignment of a last reference.
						if (type != script_object_type::none)
							script_object_ref_inc(val);
						if (old_is_object)
							script_object_ref_dec(old_val);
					}
				}
				break;
			}
			case STACKPOPBACK:
			{
				if(user_stack* st = checkStack(GET_REF(stackref), true))
				{
					bool was_object = st->holds_object(st->size() - 1);
					int32_t val = st->pop_back();
					// Transfer the stack's reference to the autorelease pool, so that popping
					// the last reference doesn't return an already-deleted object.
					if (was_object)
						script_object_transfer_ref_to_autorelease_pool(val);
					set_register(sarg1, val); //NOT *10000
				}
				else set_register(sarg1, 0L);
				break;
			}
			case STACKPOPFRONT:
			{
				if(user_stack* st = checkStack(GET_REF(stackref), true))
				{
					bool was_object = st->holds_object(0);
					int32_t val = st->pop_front();
					// See STACKPOPBACK.
					if (was_object)
						script_object_transfer_ref_to_autorelease_pool(val);
					set_register(sarg1, val); //NOT *10000
				}
				else set_register(sarg1, 0L);
				break;
			}
			case STACKPEEKBACK:
			{
				if(user_stack* st = checkStack(GET_REF(stackref), true))
				{
					set_register(sarg1, st->peek_back()); //NOT *10000
				}
				else set_register(sarg1, 0L);
				break;
			}
			case STACKPEEKFRONT:
			{
				if(user_stack* st = checkStack(GET_REF(stackref), true))
				{
					set_register(sarg1, st->peek_front()); //NOT *10000
				}
				else set_register(sarg1, 0L);
				break;
			}
			case STACKPUSHBACK:
			{
				if(user_stack* st = checkStack(GET_REF(stackref), true))
				{
					int32_t val = get_register(sarg1); //NOT /10000
					// See STACKSET.
					auto type = get_object_type_from_sarg(sarg2);
					if (st->push_back(val, type) && type != script_object_type::none)
						script_object_ref_inc(val);
				}
				break;
			}
			case STACKPUSHFRONT:
			{
				if(user_stack* st = checkStack(GET_REF(stackref), true))
				{
					int32_t val = get_register(sarg1); //NOT /10000
					// See STACKSET.
					auto type = get_object_type_from_sarg(sarg2);
					if (st->push_front(val, type) && type != script_object_type::none)
						script_object_ref_inc(val);
				}
				break;
			}
			
			//Module
			case MODULEGETIC:
			{
				int32_t buf_pointer = SH::get_arg(sarg1, false);
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
				if(user_rng* r = checkRNG(GET_REF(rngref)))
				{
					SET_D(rEXP1, r->rand(214748, -214748)*10000L);
				}
				else SET_D(rEXP1, -10000L);
				break;
			case RNGRAND2:
				if(user_rng* r = checkRNG(GET_REF(rngref)))
				{
					set_register(sarg1,r->rand(get_register(sarg1)/10000L)*10000L);
				}
				else set_register(sarg1,-10000L);
				break;
			case RNGRAND3:
				if(user_rng* r = checkRNG(GET_REF(rngref)))
				{
					set_register(sarg1,r->rand(get_register(sarg1)/10000L, get_register(sarg2)/10000L)* 10000L);
				}
				else set_register(sarg1,-10000L);
				break;
			case RNGLRAND1:
				if(user_rng* r = checkRNG(GET_REF(rngref)))
				{
					SET_D(rEXP1, r->rand());
				}
				else SET_D(rEXP1, -10000L);
				break;
			case RNGLRAND2:
				if(user_rng* r = checkRNG(GET_REF(rngref)))
				{
					SET_D(rEXP1, r->rand(get_register(sarg1)));
				}
				else SET_D(rEXP1, -10000L);
				break;
			case RNGLRAND3:
				if(user_rng* r = checkRNG(GET_REF(rngref)))
				{
					SET_D(rEXP1, r->rand(get_register(sarg1), get_register(sarg2)));
				}
				else SET_D(rEXP1, -10000L);
				break;
			case RNGSEED:
				if(user_rng* r = checkRNG(GET_REF(rngref)))
				{
					r->srand(get_register(sarg1));
				}
				break;
			case RNGRSEED:
				if(user_rng* r = checkRNG(GET_REF(rngref)))
				{
					SET_D(rEXP1, r->srand());
				}
				else SET_D(rEXP1, -10000);
				break;
			case RNGFREE:
				if(user_rng* r = checkRNG(GET_REF(rngref), true))
				{
					free_script_object(r->id);
				}
				break;
			case RNGOWN:
				if(user_rng* r = checkRNG(GET_REF(rngref), false))
				{
					own_script_object(r, type, i);
				}
				break;
			//}
			case LOADSAVEMENU:
			{
				auto val = get_register(sarg1)/10000;
				if (unsigned(val-1) >= NUM_SAVE_MENUS)
				{
					Z_scripterrlog("Tried to load invalid save_menu index '%d'\n", val);
				}
				ri->savemenuref = SET_D(rEXP1, val);
				break;
			}
			case LOADMUSICDATA:
			{
				auto val = get_register(sarg1)/10000;
				if (val && unsigned(val-1) >= quest_music.size())
				{
					Z_scripterrlog("Tried to load invalid musicdata index '%d'\n", val);
					val = 0; // return null pointer
				}
				ri->musicref = SET_D(rEXP1, val);
				break;
			}
			case LOADGENERICDATA:
				FFCore.do_loadgenericdata(false); break;
			case RUNGENFRZSCR:
			{
				bool r = FFCore.runGenericFrozenEngine(word(GET_REF(genericdataref)));
				set_register(sarg1, r ? 10000L : 0L);
				break;
			}
			
			///----------------------------------------------------------------------------------------------------//
			
			case SUBDATA_OPEN:
			{
				if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
				{
					FFCore.queued_subscreen = sub;
				}
				break;
			}
			case SUBDATA_CLOSE:
			{
				if(checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
				{
					if (type != ScriptType::EngineSubscreen
						|| ri->subscreendataref != get_subref(-1, i))
					{
						scripting_log_error_with_context("This function can only be used on the currently-open Active or Map subscreen.");
					}
					else earlyretval = RUNSCRIPT_SELFDELETE; // exit the subscreen itself
				}
				break;
			}
			case SUBDATA_GET_NAME:
			{
				if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref)))
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
				if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref)))
				{
					auto aptr = get_register(sarg1);
					ArrayH::getString(aptr, sub->name);
				}
				break;
			}
			case SUBDATA_SWAP_PAGES:
			{
				ri->subscreendataref = SH::read_stack(ri->sp+2);
				if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref)))
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
				ri->subscreenpageref = SH::read_stack(ri->sp+2);
				if(SubscrPage* pg = checkSubPage(GET_REF(subscreenpageref)))
				{
					int p1 = SH::read_stack(ri->sp+1) / 10000;
					int p2 = SH::read_stack(ri->sp+0) / 10000;
					if(unsigned(p1) >= pg->size())
						Z_scripterrlog("Invalid first widget index '%d' passed to subscreenpage->SwapWidgets()\n", p1);
					else if(unsigned(p2) >= pg->size())
						Z_scripterrlog("Invalid second widget index '%d' passed to subscreenpage->SwapWidgets()\n", p2);
					else pg->swap_widg(p1,p2);
				}
				break;
			}
			case SUBPAGE_FIND_WIDGET:
			{
				SET_D(rEXP1, 0);

				ri->subscreenpageref = SH::read_stack(ri->sp+1);
				if(SubscrPage* pg = checkSubPage(GET_REF(subscreenpageref), {sstACTIVE, sstMAP}))
				{
					int cursorpos = SH::read_stack(ri->sp+0) / 10000;
					if(auto* widg = pg->get_widg_pos(cursorpos,false))
					{
						auto q = pg->widget_index(widg);
						if(q > -1)
						{
							auto [sub,ty,pgid,_ind] = from_subref(GET_REF(subscreenpageref));
							SET_D(rEXP1, get_subref(sub,ty,pgid,q));
						}
					}
				}
				break;
			}
			case SUBPAGE_FIND_WIDGET_BY_LABEL:
			{
				SET_D(rEXP1, 0);

				ri->subscreenpageref = SH::read_stack(ri->sp+1);
				if(SubscrPage* pg = checkSubPage(GET_REF(subscreenpageref)))
				{
					int aptr = SH::read_stack(ri->sp+0);
					std::string lbl;
					ArrayH::getString(aptr, lbl);
					if(lbl.size())
					{
						auto q = pg->find_label_index(lbl);
						if(q > -1)
						{
							auto [sub,ty,pgid,_ind] = from_subref(GET_REF(subscreenpageref));
							SET_D(rEXP1, get_subref(sub,ty,pgid,q));
						}
					}
				}
				break;
			}
			case SUBPAGE_MOVE_SEL:
			{
				#define SUBSEL_FLAG_NO_NONEQUIP 0x01
				#define SUBSEL_FLAG_NEED_ITEM 0x02

				SET_D(rEXP1, 0);

				ri->subscreenpageref = SH::read_stack(ri->sp+3);
				if(SubscrPage* pg = checkSubPage(GET_REF(subscreenpageref)))
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
					SET_D(rEXP1, 10000*newpos);
				}
				break;
			}
			case SUBPAGE_NEW_WIDG:
			{
				SET_D(rEXP1, 0);

				ri->subscreenpageref = SH::read_stack(ri->sp+1);
				if(SubscrPage* pg = checkSubPage(GET_REF(subscreenpageref)))
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
						auto [sub,ty,pgid,_ind] = from_subref(GET_REF(subscreenpageref));
						SET_D(rEXP1, get_subref(sub,ty,pgid,pg->size()-1));
					}
					else Z_scripterrlog("Invalid type %d passed to subscreenpage->CreateWidget()\n",ty);
				}
				break;
			}
			case SUBPAGE_DELETE:
			{
				if(SubscrPage* pg = checkSubPage(GET_REF(subscreenpageref)))
				{
					auto [sub,_ty] = load_subdata(GET_REF(subscreenpageref));
					sub->delete_page(pg->getIndex());
				}
				break;
			}
			case SUBWIDG_GET_SELTEXT_OVERRIDE:
			{
				if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
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
				if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				{
					auto aptr = get_register(sarg1);
					ArrayH::getString(aptr, widg->override_text);
				}
				break;
			}
			case SUBWIDG_GET_LABEL:
			{
				if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
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
				if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				{
					auto aptr = get_register(sarg1);
					ArrayH::getString(aptr, widg->label);
				}
				break;
			}
			case SUBWIDG_CHECK_CONDITIONS:
			{
				if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				{
					set_register(sarg1, widg->check_conditions() ? 10000 : 0);
				}
				break;
			}
			case SUBWIDG_CHECK_VISIBLE:
			{
				if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				{
					extern int current_subscr_pos;
					set_register(sarg1, widg->visible(current_subscr_pos, game->should_show_time()) ? 10000 : 0);
				}
				break;
			}
			case SUBWIDG_TY_GETTEXT:
			{
				if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				{
					optional<string> str;
					byte ty = widg->getType();
					switch(ty)
					{
						case widgTEXT:
							str = ((SW_Text*)widg)->text;
							break;
						case widgTEXTBOX:
							str = ((SW_TextBox*)widg)->text;
							break;
						case widgITMCOOLDOWNTEXT:
							str = ((SW_ItemCooldownText*)widg)->get_text();
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
				if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
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
				if(checkComboRef())
				{
					auto aptr = get_register(sarg1);
					string name;
					ArrayH::getString(aptr, name, 256);
					newcombo const& cmb = combobuf[GET_REF(combodataref)];
					int32_t ret = 0;
					for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
					{
						if(cmb.triggers[idx].label == name)
						{
							ret = dword(GET_REF(combodataref)) | (dword(idx)<<24);
							break;
						}
					}
					
					set_register(sarg1, ret);
				}
				break;
			}
			case CMBTRIG_GET_LABEL:
			{
				if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				{
					auto aptr = get_register(sarg1);
					if(ArrayH::setArray(aptr, trig->label, true) == SH::_Overflow)
						Z_scripterrlog("Array supplied to 'combotrigger->GetLabel()' not large enough,"
							" and couldn't be resized!\n");
				}
				break;
			}
			case CMBTRIG_SET_LABEL:
			{
				if (auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				{
					auto aptr = get_register(sarg1);
					ArrayH::getString(aptr, trig->label);
				}
				break;
			}
			
			case REF_INC:
			{
				int thiskey = GET_REF(thiskey);

				if (sarg1 == -1)
				{
					script_object_ref_inc(thiskey);
					break;
				}

				int offset = GET_D(rSFRAME) + sarg1;
				if (!ri->stackPosHasObject(offset))
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
				int thiskey = GET_REF(thiskey);

				if (sarg1 == -1)
				{
					script_object_ref_dec(thiskey);
					break;
				}

				int offset = GET_D(rSFRAME) + sarg1;
				if (!ri->stackPosHasObject(offset))
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
				if (id && !script_object_autorelease_pool_contains(id))
				{
					script_object_autorelease_pool_add(id);
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
				int offset = GET_D(rSFRAME) + sarg2;
				if (offset < 0 || offset >= MAX_STACK_SIZE)
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
			case MARK_TYPE_VARG:
			{
				auto& vec = ri->zs_vargs_stack.back();
				auto& obj_set = ri->zs_vargs_pos_is_object.back();
				auto offset = vec.size()-1;
				bool already_obj = obj_set.contains(offset);
				if (sarg1)
					obj_set.insert(offset);
				else
					obj_set.erase(offset);
				
				auto id = vec.at(offset);
				if (already_obj && !sarg1)
					script_object_ref_dec(id);
				else if (sarg1 && !already_obj)
					script_object_ref_inc(id);
				break;
			}
			case REF_REMOVE:
			{
				int offset = GET_D(rSFRAME) + sarg1;
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
				int value = get_register(sarg2);
				if (sarg1 >= GD(0) && sarg1 < GD(MAX_GLOBAL_VARIABLES))
				{
					int index = sarg1-GD(0);
					assert(game->global_d_types[index] != script_object_type::none);
					script_object_ref_inc(value);
					script_object_ref_dec(game->global_d[index]);
					game->global_d[index] = value;
				}
				else if (sarg1 >= SCRIPT_INST_VARS(0) && sarg1 < SCRIPT_INST_VARS(MAX_SCRIPT_INST_VARIABLES))
				{
					int index = sarg1-SCRIPT_INST_VARS(0);
					assert(ri->script_d_is_object.contains(index));
					script_object_ref_inc(value);
					script_object_ref_dec(ri->script_d[index]);
					ri->script_d[index] = value;
				}
				else if (zasm_array_supports(sarg1))
				{
					auto type = (script_object_type)sarg3;

					int zasm_var = sarg1;
					int index = GET_D(rINDEX) / 10000;
					int ref_arg = get_register_ref_dependency(zasm_var).value_or(0);
					int ref = ref_arg ? get_ref(ref_arg) : 0;
					zasm_array_set(zasm_var, ref, index, value, type);
				}
				else NOTREACHED();
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
#ifdef _DEBUG
				Z_error_fatal("Invalid ZASM command: %d\n", scommand);
#endif
				break;
			}
		}
		if(earlyretval == RUNSCRIPT_SELFDELETE)
		{
			// The JIT uses command_could_return_not_ok to decide whether to check a command's
			// return code. A self-deleting command that isn't registered there is silently
			// ignored by the JIT (it diverges from the interpreter), so catch that here.
			CHECK(command_could_return_not_ok(scommand));
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
		if(old_script_funcrun && ri->pc == MAX_PC)
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
			[[maybe_unused]] auto v = earlyretval;
			earlyretval = -1;
			return earlyretval;
		}

		// If running a JIT compiled script, we're only here to do a few commands.
		commands_run += 1;
		if (is_jitted && commands_run == j_instance->uncompiled_command_count)
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

	if (is_jitted && commands_run == j_instance->uncompiled_command_count)
		return RUNSCRIPT_OK;

	ri->pc++;

	return RUNSCRIPT_OK;
}

script_data* load_scrdata(ScriptType type, word script, int32_t i)
{
	switch(type)
	{
		case ScriptType::FFC:
			return ffscripts[script];
		case ScriptType::NPC:
			return guyscripts[guys.getByUID(i)->scrconfig.script];
		case ScriptType::Lwpn:
			return lwpnscripts[Lwpns.getByUID(i)->scrconfig.script];
		case ScriptType::Ewpn:
			return ewpnscripts[Ewpns.getByUID(i)->scrconfig.script];
		case ScriptType::ItemSprite:
			return itemspritescripts[items.getByUID(i)->scrconfig.script];
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
		for_every_base_screen_in_region([&](mapscr* scr, unsigned int, unsigned int) {
			if ((preload && scr->preloadscript) || !preload)
			{
				if (scr->scrconfig.script > 0 && FFCore.doscript(ScriptType::Screen, scr->screen))
				{
					ZScriptVersion::RunScript(ScriptType::Screen, scr->scrconfig.script, scr->screen);
				}
			}
			});
	}

	if (!FFCore.system_suspend[susptFFCSCRIPTS])
	{
		//intentional it's for compatability
		if (FFCore.getQuestHeaderInfo(vZelda) >= 0x255 && get_qr(qr_ZS_OLD_SUSPEND_FFC))
		{
			for_every_base_screen_in_region([&](mapscr* scr, unsigned int, unsigned int) {
				if ((preload && scr->preloadscript) || !preload)
				{
					if (scr->scrconfig.script > 0 && FFCore.doscript(ScriptType::Screen, scr->screen))
					{
						ZScriptVersion::RunScript(ScriptType::Screen, scr->scrconfig.script, scr->screen);
					}
				}
				});
		}

		for_every_ffc([&](const ffc_handle_t& ffc_handle) {
			if(ffc_handle.ffc->scrconfig.script == 0)
				return;
				
			if(preload && !(ffc_handle.ffc->flags&ffc_preload))
				return;
				
			if((ffc_handle.ffc->flags&ffc_ignoreholdup)==0 && Hero.getHoldClk()>0)
				return;
			
			if (ffc_handle.ffc->is_beyond_viewport_suspend_range())
				return;

			ZScriptVersion::RunScript(ScriptType::FFC, ffc_handle.ffc->scrconfig.script, ffc_handle.ffc_id);
		});
	}
	
	
	return 0;
}



///----------------------------------------------------------------------------------------------------

void FFScript::user_files_init()
{
	files_init();
}

void FFScript::user_dirs_init()
{
	dirs_init();
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

void FFScript::user_weapondata_init()
{
	user_weapondatas.clear();
}

void FFScript::script_arrays_init()
{
	script_arrays.clear();
}

///----------------------------------------------------------------------------------------------------

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
	int32_t w = (GET_D(rINDEX2) / 10000);
	int32_t h = (GET_D(rINDEX)/10000);
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

	if(isSystemBitref(GET_REF(bitmapref)))
	{
		return; //Don't attempt to deallocate system bitmaps!
	}

	// Bitmaps are not deallocated right away, but deferred until the next call to scb.update()
	if (auto b = checkBitmap(GET_REF(bitmapref), false, true))
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
int32_t FFScript::GetQuestSectionVersion([[maybe_unused]] int32_t section)
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
	int32_t map = (GET_D(rINDEX) / 10000);
	int32_t screen = (GET_D(rINDEX2)/10000);
	int32_t indx = (zc_max((map)-1,0) * MAPSCRS + screen);
	 if ( map < 1 || map > map_count )
	{
		Z_scripterrlog("Invalid Map ID passed to Game->LoadMapData: %d\n", map);
		ri->mapdataref = 0;
	}
	else if ( screen < 0 || screen > 129 ) //0x00 to 0x81 -Z
	{
		Z_scripterrlog("Invalid Screen Index passed to Game->LoadMapData: %d\n", screen);
		ri->mapdataref = 0;
	}
	else ri->mapdataref = indx + 1;
	return ri->mapdataref;
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
	
	return game->screen_d.get(index1).get(index2);
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
		al_trace("The index supplied to Game->SetSubscreenText() is invalid. The index specified was: %d\n", index);
		return;
	}

	string filename_str;
	ArrayH::getString(arrayptr, filename_str, 73);
	ChangeSubscreenText(index,filename_str.c_str());
}

void FFScript::SetItemMessagePlayed(int32_t itm)
{
	if (invalid_item_id(itm)) return;
	game->item_messages_played.set(itm, true);
}
bool FFScript::GetItemMessagePlayed(int32_t itm)
{
	if (invalid_item_id(itm)) return false;
	return game->item_messages_played.get(itm);
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

void FFScript::init(bool for_continue)
{
	apply_qr_rules();
	eventData.clear();
	countGenScripts();
	// Some scripts can run even before ~Init (but only if qr_OLD_INIT_SCRIPT_TIMING is on), so figure out
	// the global register types ahead of time.
	markGlobalRegisters();
	for ( int32_t q = 0; q < wexLast; q++ ) warpex[q] = 0;
	temp_no_stepforward = 0;
	queued_subscreen = nullptr;
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
	//clearRunningItemScripts();
	ScrollingScreensAll.clear();
	memset(ScrollingData, 0, sizeof(int32_t) * SZ_SCROLLDATA);
	ScrollingData[SCROLLDATA_DIR] = -1;
	user_rng_init();
	if (for_continue)
		clear_script_engine_data_for_continue();
	else clear_script_engine_data();
	script_debug_handles.clear();
	runtime_script_debug_handle = nullptr;
	show_zasm_stack_traces = zc_get_config("ZSCRIPT", "show_zasm_stack_traces", false);
}

void FFScript::shutdown()
{
	scriptEngineDatas.clear();
	objectRAM.clear();
	script_objects.clear();
	script_object_ids_by_type.clear();
	next_script_object_id_freelist.clear();
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

void FFScript::do_greyscale([[maybe_unused]] const bool v)
{
	// This has been removed.
}

void FFScript::do_monochromatic([[maybe_unused]] const bool v)
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
		// Kill leading '/'
		size_t first = user_path.find_first_not_of('/');
		if (first != string::npos)
			user_path = user_path.substr(first, string::npos);

		// Kill trailing '/'
		size_t last = user_path.find_last_not_of('/');
		if (last != string::npos)
			user_path = user_path.substr(0, last + 1);

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

static void warp_ex(int args[wexLast])
{
	if ( ((unsigned)args[1]) >= MAXDMAPS )
		Z_scripterrlog("Invalid DMap ID (%d) passed to WarpEx(). Aborting.\n", args[1]);
	else if ( ((unsigned)args[2]) >= MAPSCRS )
		Z_scripterrlog("Invalid Screen Index (%d) passed to WarpEx(). Aborting.\n", args[2]);
	else if ( map_screen_index(DMaps[args[1]].map, args[2] + DMaps[args[1]].xoff) >= (int32_t)TheMaps.size() )
		Z_scripterrlog("Invalid destination passed to WarpEx(). Aborting.\n");
	else
	{
		if(get_qr(qr_OLD_BROKEN_WARPEX_MUSIC))
		{
			SETFLAG(args[wexFlags],warpFlagFORCECONTINUEMUSIC,args[wexFlags]&warpFlagFORCERESETMUSIC);
			TOGGLEFLAG(args[wexFlags],warpFlagFORCERESETMUSIC);
		}
		memcpy(FFCore.warpex, args, sizeof(FFCore.warpex));
		FFCore.warpex[wexActive] = 1;
	}
}

void FFScript::do_warp_ex()
{
	int num_args = sarg1 / 10000;
	if (num_args < 8 || unsigned(num_args) > wexActive)
	{
		scripting_log_error_with_context("Invalid parameter count %d!", num_args);
		return;
	}
	int args[wexLast] = {0};
	for (int q = 0; q < num_args; ++q)
		args[q] = SH::read_stack(ri->sp + (num_args - q - 1)) / 10000;
	warp_ex(args);
}

void FFScript::do_warp_ex_array()
{
	int32_t zscript_array_ptr = get_register(sarg1);
	ArrayManager am(zscript_array_ptr);
	if(am.invalid()) return;
	int32_t zscript_array_size = am.size();
	switch(zscript_array_size)
	{
		case 8: // {int32_t type, int32_t dmap, int32_t screen, int32_t x, int32_t y, int32_t effect, int32_t sound, int32_t flags}
		case 9: // {int32_t type, int32_t dmap, int32_t screen, int32_t x, int32_t y, int32_t effect, int32_t sound, int32_t flags, int32_t dir}
		{
			int args[wexLast] = {0};
			for ( int32_t q = 0; q < 8; q++ )
				args[q] = (am.get(q)/10000);
			args[wexDir] = zscript_array_size < 9 ? -1 : (am.get(8)/10000);
			warp_ex(args);
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
		if(DMaps[cur_dmap].passive_sub_scrconfig.script != 0)
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[cur_dmap].passive_sub_scrconfig.script, cur_dmap);
		if (waitdraw(ScriptType::ScriptedPassiveSubscreen) && DMaps[cur_dmap].passive_sub_scrconfig.script != 0 && doscript(ScriptType::ScriptedPassiveSubscreen))
		{
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[cur_dmap].passive_sub_scrconfig.script, cur_dmap);
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
			ZScriptVersion::RunScript(ScriptType::DMap, DMaps[cur_dmap].active_scrconfig.script,cur_dmap);
			FFCore.waitdraw(ScriptType::DMap) = false;
		}
		if ( (!( FFCore.system_suspend[susptDMAPSCRIPT] )) && FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 )
		{
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[cur_dmap].passive_sub_scrconfig.script,cur_dmap);
			FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) = false;
		}
		//no doscript check here, becauseb of preload? Do we want to write doscript here? -Z 13th July, 2019
		if (FFCore.getQuestHeaderInfo(vZelda) >= 0x255 && !FFCore.system_suspend[susptSCREENSCRIPTS])
		{
			for_every_base_screen_in_region([&](mapscr* scr, unsigned int, unsigned int) {
				if (scr->scrconfig.script != 0 && FFCore.waitdraw(ScriptType::Screen, scr->screen) && scr->preloadscript)
				{
					ZScriptVersion::RunScript(ScriptType::Screen, scr->scrconfig.script, scr->screen);  
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
			ZScriptVersion::RunScript(ScriptType::DMap, DMaps[cur_dmap].active_scrconfig.script,cur_dmap);
		}
		if ( (!( FFCore.system_suspend[susptDMAPSCRIPT] )) && FFCore.doscript(ScriptType::ScriptedPassiveSubscreen) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) 
		{
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[cur_dmap].passive_sub_scrconfig.script,cur_dmap);
		}
		if (FFCore.getQuestHeaderInfo(vZelda) >= 0x255 && !FFCore.system_suspend[susptSCREENSCRIPTS])
		{
			for_every_base_screen_in_region([&](mapscr* scr, unsigned int, unsigned int) {
				if (scr->scrconfig.script != 0 && scr->preloadscript)
				{
					ZScriptVersion::RunScript(ScriptType::Screen, scr->scrconfig.script, scr->screen);
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
			ScopedScriptEngineDataClear engine_data_guard{ScriptType::Global, GLOBAL_SCRIPT_F6};
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
		else if (unsigned(QMisc.savemenu_f6 - 1) < NUM_SAVE_MENUS && QMisc.save_menus[QMisc.savemenu_f6 - 1].is_valid())
		{
			clear_bitmap(f6_menu_buf);
			blit(framebuf, f6_menu_buf, 0, 0, 0, 0, framebuf->w, framebuf->h);

			GameFlags &= ~GAMEFLAG_TRYQUIT;
			GameFlags |= GAMEFLAG_F6SCRIPT_ACTIVE;
			QMisc.save_menus[QMisc.savemenu_f6 - 1].run();
			GameFlags &= ~GAMEFLAG_F6SCRIPT_ACTIVE;
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
	ScopedScriptEngineDataClear engine_data_guard{ScriptType::Hero, 0};
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
	ScopedScriptEngineDataClear engine_data_guard{ScriptType::Global, GLOBAL_SCRIPT_ONLAUNCH};
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
			scr.scrconfig.run_args[q] = init_data[q];
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
	ScopedScriptEngineDataClear engine_data_guard{ScriptType::GenericFrozen, local_i};
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
	bool skipdraws = FFCore.skipscriptdraws;
	FFCore.skipscriptdraws = false;
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
	FFCore.skipscriptdraws = skipdraws;
	//script_drawing_commands.push_commands(tmpDrawCommands);
	//clear
	GameFlags &= ~GAMEFLAG_SCRIPTMENU_ACTIVE;
	if(fl)
	{
		GameFlags |= fl;
		destroy_bitmap(script_menu_buf);
		script_menu_buf = tmpbuf;
	}
	--gen_frozen_index;
	//Restore script refinfo
	pop_ri();
	return true;
}

bool FFScript::runScriptedActiveSubscreen()
{
	word activesubscript = DMaps[cur_dmap].active_sub_scrconfig.script;
	if(!activesubscript || !dmapscripts[activesubscript]->valid()) return false; //No script to run
	word passivesubscript = DMaps[cur_dmap].passive_sub_scrconfig.script;
	word dmapactivescript = DMaps[cur_dmap].active_scrconfig.script;
	clear_bitmap(script_menu_buf);
	blit(framebuf, script_menu_buf, 0, 0, 0, 0, framebuf->w, framebuf->h);
	initZScriptScriptedActiveSubscreen();
	ScopedScriptEngineDataClear engine_data_guard{ScriptType::ScriptedActiveSubscreen, 0};
	GameFlags |= GAMEFLAG_SCRIPTMENU_ACTIVE;
	word script_dmap = cur_dmap;
	//auto tmpDrawCommands = script_drawing_commands.pop_commands();
	pause_all_sfx();
	auto& data = get_script_engine_data(ScriptType::ScriptedActiveSubscreen);
	bool skipdraws = FFCore.skipscriptdraws;
	FFCore.skipscriptdraws = false;
	while (data.doscript && !Quit)
	{
		script_drawing_commands.Clear();
		load_control_state();
		if(get_qr(qr_DMAP_ACTIVE_RUNS_DURING_ACTIVE_SUBSCRIPT) && DMaps[script_dmap].active_scrconfig.script != 0 && doscript(ScriptType::DMap))
		{
			ZScriptVersion::RunScript(ScriptType::DMap, dmapactivescript, script_dmap);
		}
		if(get_qr(qr_PASSIVE_SUBSCRIPT_RUNS_DURING_ACTIVE_SUBSCRIPT)!=0 && DMaps[script_dmap].passive_sub_scrconfig.script != 0 && FFCore.doscript(ScriptType::ScriptedPassiveSubscreen))
		{
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, passivesubscript, script_dmap);
		}
		ZScriptVersion::RunScript(ScriptType::ScriptedActiveSubscreen, activesubscript, script_dmap);
		if(waitdraw(ScriptType::DMap) && (get_qr(qr_DMAP_ACTIVE_RUNS_DURING_ACTIVE_SUBSCRIPT) && DMaps[script_dmap].active_scrconfig.script != 0 && doscript(ScriptType::DMap)))
		{
			ZScriptVersion::RunScript(ScriptType::DMap, dmapactivescript, script_dmap);
			waitdraw(ScriptType::DMap) = false;
		}
		if(waitdraw(ScriptType::ScriptedPassiveSubscreen) && (get_qr(qr_PASSIVE_SUBSCRIPT_RUNS_DURING_ACTIVE_SUBSCRIPT)!=0 && DMaps[script_dmap].passive_sub_scrconfig.script != 0 && FFCore.doscript(ScriptType::ScriptedPassiveSubscreen)))
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
			activesubscript = DMaps[cur_dmap].active_sub_scrconfig.script;
			if(!activesubscript || !dmapscripts[activesubscript]->valid()) return true; //No script to run
			passivesubscript = DMaps[cur_dmap].passive_sub_scrconfig.script;
			dmapactivescript = DMaps[cur_dmap].active_scrconfig.script;
			script_dmap = cur_dmap;
			//Reset the background image
			game_loop();
			clear_bitmap(script_menu_buf);
			blit(framebuf, script_menu_buf, 0, 0, 0, 0, framebuf->w, framebuf->h);
			//Now loop without advancing frame, so that the subscreen script can draw immediately.
		}
	}
	FFCore.skipscriptdraws = skipdraws;
	resume_all_sfx();
	script_drawing_commands.Clear();
	//script_drawing_commands.push_commands(tmpDrawCommands);
	GameFlags &= ~GAMEFLAG_SCRIPTMENU_ACTIVE;
	GameFlags |= GAMEFLAG_RESET_GAME_LOOP;
	return true;
}
bool FFScript::runOnMapScriptEngine()
{
	word onmap_script = DMaps[cur_dmap].onmap_scrconfig.script;
	if(!onmap_script || !dmapscripts[onmap_script]->valid()) return false; //No script to run
	clear_bitmap(script_menu_buf);
	blit(framebuf, script_menu_buf, 0, 0, 0, 0, framebuf->w, framebuf->h);
	initZScriptOnMapScript();
	ScopedScriptEngineDataClear engine_data_guard{ScriptType::OnMap, 0};
	GameFlags |= GAMEFLAG_SCRIPTMENU_ACTIVE;
	word script_dmap = cur_dmap;
	//auto tmpDrawCommands = script_drawing_commands.pop_commands();
	pause_all_sfx();

	bool skipdraws = FFCore.skipscriptdraws;
	FFCore.skipscriptdraws = false;
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
			onmap_script = DMaps[cur_dmap].onmap_scrconfig.script;
			if(!onmap_script || !dmapscripts[onmap_script]->valid()) return true; //No script to run
			script_dmap = cur_dmap;
			//Reset the background image
			game_loop();
			clear_bitmap(script_menu_buf);
			blit(framebuf, script_menu_buf, 0, 0, 0, 0, framebuf->w, framebuf->h);
			//Now loop without advancing frame, so that the subscreen script can draw immediately.
		}
	}
	FFCore.skipscriptdraws = skipdraws;
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
		ScopedScriptEngineDataClear engine_data_guard{ScriptType::Global, GLOBAL_SCRIPT_ONSAVE};
		ZScriptVersion::RunScript(ScriptType::Global, GLOBAL_SCRIPT_ONSAVE, GLOBAL_SCRIPT_ONSAVE);
		//
		pop_ri();
		Quit = tQuit;
	}
}

bool FFScript::itemScriptEngine()
{
	if ( FFCore.system_suspend[susptITEMSCRIPTENGINE] ) return false;
	for (int32_t q = 0; q < itemsbuf.capacity(); ++q)
	{
		auto const& itm = itemsbuf.get(q);
		if ( itm.scrconfig.script <= 0 || itm.scrconfig.script > NUMSCRIPTITEM ) continue; // > NUMSCRIPTITEM as someone could force an invaid script slot!
		
		auto& data = get_script_engine_data(ScriptType::Item, q);
		if ( data.doscript < 1 ) continue;
		
		//Passive items
		if (((itm.flags&item_passive_script)))
		{
			if(game->get_item(q) && (get_qr(qr_ITEMSCRIPTSKEEPRUNNING)))
			{
				if(get_qr(qr_PASSIVE_ITEM_SCRIPT_ONLY_HIGHEST)
					&& current_item(itm.type) > itm.level)
					data.doscript = 0;
				else ZScriptVersion::RunScript(ScriptType::Item, itm.scrconfig.script, q);
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
				ZScriptVersion::RunScript(ScriptType::Item, itm.scrconfig.script, q);
			}
			else if (data.doscript == 3) //Run via itemdata->RunScript
			{
				if ( (get_qr(qr_ITEMSCRIPTSKEEPRUNNING)) ) 
				{
					data.doscript = 2; //Reduce to normal run status
				}
				else 
				{
					ZScriptVersion::RunScript(ScriptType::Item, itm.scrconfig.script, q);
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
	for (int32_t q = 0; q < itemsbuf.capacity(); ++q)
	{
		auto const& itm = itemsbuf.get(q);
		if ( itm.scrconfig.script <= 0 || itm.scrconfig.script > NUMSCRIPTITEM ) continue; // > NUMSCRIPTITEM as someone could force an invaid script slot!
		
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
		if ((itm.flags&item_passive_script))
		{
			if(game->get_item(q) && (get_qr(qr_ITEMSCRIPTSKEEPRUNNING)))
			{
				if(get_qr(qr_PASSIVE_ITEM_SCRIPT_ONLY_HIGHEST)
					&& current_item(itm.type) > itm.level)
					data.doscript = 0;
				else ZScriptVersion::RunScript(ScriptType::Item, itm.scrconfig.script, q);
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
				ZScriptVersion::RunScript(ScriptType::Item, itm.scrconfig.script, q);
			}
			else if (data.doscript == 3) //Run via itemdata->RunScript
			{
				if ( (get_qr(qr_ITEMSCRIPTSKEEPRUNNING)) ) 
				{
					data.doscript = 2; //Reduce to normal run status
				}
				else 
				{
					ZScriptVersion::RunScript(ScriptType::Item, itm.scrconfig.script, q);
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
	if(auto s=checkLWpn(GET_REF(lwpnref)))
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
	if(auto s=checkEWpn(GET_REF(ewpnref)))
	{
		Ewpns.del(s);
	}
}

void FFScript::updateIncludePaths()
{
	// TODO: remove this from FFScript - must also remove ffscript.h being used in ./src/zq
}

void FFScript::initIncludePaths()
{
	// TODO: remove this from FFScript - must also remove ffscript.h being used in ./src/zq
}

bool FFScript::checkExtension(std::string &filename, const std::string &extension)
//inline bool checkExtension(std::string filename, std::string extension)
{
	int32_t dot = filename.find_last_of(".");
	std::string exten = (dot == std::string::npos ? "" : filename.substr(dot, filename.length() - dot));
	return exten == extension;
}

void FFScript::do_strcmp()
{
	int32_t arrayptr_a = GET_D(rINDEX);
	int32_t arrayptr_b = GET_D(rINDEX2);
	string strA;
	string strB;
	ArrayH::getString(arrayptr_a, strA);
	ArrayH::getString(arrayptr_b, strB);
	set_register(sarg1, (vbound(strcmp(strA.c_str(), strB.c_str()), -1, 1) * 10000));
}

void FFScript::do_stricmp()
{
	int32_t arrayptr_a = GET_D(rINDEX);
	int32_t arrayptr_b = GET_D(rINDEX2);
	string strA;
	string strB;
	ArrayH::getString(arrayptr_a, strA);
	ArrayH::getString(arrayptr_b, strB);
	set_register(sarg1, (vbound(stricmp(strA.c_str(), strB.c_str()), -1, 1) * 10000));
}

void FFScript::do_LowerToUpper([[maybe_unused]] const bool v)
{
	int32_t arrayptr_a = get_register(sarg1);
	string strA;
	ArrayH::getString(arrayptr_a, strA);
	for (char& c : strA)
		c = std::toupper(c); 
	ArrayH::setArray(arrayptr_a, strA);
	set_register(sarg1, 10000); // used to return 0 if string was empty.
}

void FFScript::do_UpperToLower([[maybe_unused]] const bool v)
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
	ArrayH::getString(arrayptr, the_string, 512);
	
	for (int32_t q = 0; q < itemsbuf.capacity(); ++q)
	{
		if (the_string == itemsbuf.get(q).name)
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
void FFScript::do_getmusicdatabyname()
{
	int32_t arrayptr = get_register(sarg1);
	string the_string;
	int32_t num = -1;
	ArrayH::getString(arrayptr, the_string, 512);
	
	if (!the_string.empty())
	{
		for (word q = 0; q < quest_music.size(); ++q)
		{
			if (the_string == quest_music[q].name)
			{
				num = q;
				break;
			}
		}
	}
	set_register(sarg1, num+1);
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
void FFScript::do_ConvertCase([[maybe_unused]] const bool v)
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
	double val = util::zc_xtoi(const_cast<char*>(str.c_str()));
	set_register(sarg1, (int32_t)(val) * 10000);
}
void FFScript::do_xtoi2() 
{
	int32_t arrayptr_a = GET_D(rINDEX);
	string strA;
	ArrayH::getString(arrayptr_a, strA);
	set_register(sarg1, (util::zc_xtoi(strA.c_str()) * 10000));
}

// Calculates log2 of number.  
double FFScript::Log2( double n )  
{  
    // log(n)/log(2) is log2.
    return zc::math::Ln( (double)n ) / zc::math::Ln( (double)2 );
}

//xtoa, convert hex number to hex ascii
void FFScript::do_xtoa()
{
	int32_t arrayptr_a = get_register(sarg1);
	int32_t number = get_register(sarg2) / 10000;//GET_D(rEXP2)/10000; //why are you not in sarg2?!!
	
	bool isneg = false;
	if ( number < 0 ) 
	{
		isneg = true; 
		number *= -1;
	}
	double num = number;
	int32_t digits = num ? floor(FFCore.LogToBase(num, 16) + 1) : 1;
	
	ostringstream oss;
	if ( isneg )
		oss << "-";
	oss << "0x";

	int32_t alphaoffset = 'A' - 0xA;
	for(int32_t i = 0; i < digits; ++i)
	{
		int32_t coeff = ((int32_t)floor((double)(((double)number) / zc::math::Pow(0x10, digits - i - 1))) % 0x10);
		oss << char(coeff < 0xA ? coeff + '0' : coeff + alphaoffset);
	}
	string str = oss.str();
	if(ArrayH::setArray(arrayptr_a, str) == SH::_Overflow)
	{
		scripting_log_error_with_context("Dest string parameter not large enough");
		set_register(sarg1, 0);
	}
	else set_register(sarg1, str.size() * 10000);
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
	
	int32_t arrayptr_a = GET_D(rINDEX);
	int32_t arrayptr_b = GET_D(rINDEX2);
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
	
	int32_t arrayptr_a = GET_D(rINDEX);
	int32_t arrayptr_b = GET_D(rINDEX2);
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
	
	int32_t arrayptr_a = GET_D(rINDEX);
	int32_t arrayptr_b = GET_D(rINDEX2);
	string strA;
	string strB;
	ArrayH::getString(arrayptr_a, strA);
	ArrayH::getString(arrayptr_b, strB);
	set_register(sarg1, (strspn(strA.c_str(), strB.c_str()) * 10000));
}

void FFScript::do_strcspn()
{
	
	int32_t arrayptr_a = GET_D(rINDEX);
	int32_t arrayptr_b = GET_D(rINDEX2);
	string strA;
	string strB;
	ArrayH::getString(arrayptr_a, strA);
	ArrayH::getString(arrayptr_b, strB);
	set_register(sarg1, (strcspn(strA.c_str(), strB.c_str()) * 10000));
}

void FFScript::do_strchr()
{
	
	int32_t arrayptr_a = GET_D(rINDEX);
	char chr_to_find = (GET_D(rINDEX2)/10000);
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
	int32_t arrayptr_a = GET_D(rINDEX);
	char chr_to_find = (GET_D(rINDEX2)/10000);
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
	int32_t arrayptr_a = GET_D(rINDEX);
	string strA;
	ArrayH::getString(arrayptr_a, strA);
}
//Bookmark
void FFScript::do_atoi2()
{
	//not implemented; atoi does not take 2 params
	int32_t arrayptr_a = GET_D(rINDEX);
	string strA;
	ArrayH::getString(arrayptr_a, strA);
}
void FFScript::do_ilen2()
{
	//not implemented, ilen not found
	int32_t arrayptr_a = GET_D(rINDEX);
	string strA;
	ArrayH::getString(arrayptr_a, strA);
}
void FFScript::do_xlen2()
{
	//not implemented, xlen not found
	int32_t arrayptr_a = GET_D(rINDEX);
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
		strB[pos + ret + i] = ((int32_t)floor((double)(num / zc::math::Pow(10, digits - i - 1))) % 10) + '0';
	
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
	int32_t arrayptr_a = GET_D(rINDEX);
	int32_t arrayptr_b = GET_D(rEXP2);
	int32_t len = GET_D(rEXP1)/10000;
	string strA;
	string strB;
	ArrayH::getString(arrayptr_a, strA);
	ArrayH::getString(arrayptr_b, strB);
	set_register(sarg1, (vbound(strncmp(strA.c_str(), strB.c_str(), len), -1, 1) * 10000));
}

void FFScript::do_strnicmp()
{
	int32_t arrayptr_a = GET_D(rINDEX);
	int32_t arrayptr_b = GET_D(rEXP2);
	int32_t len = GET_D(rEXP1)/10000;
	string strA;
	string strB;
	ArrayH::getString(arrayptr_a, strA);
	ArrayH::getString(arrayptr_b, strB);
	set_register(sarg1, (vbound(ustrnicmp(strA.c_str(), strB.c_str(), len), -1, 1) * 10000));
}

/////////////////////
/// MATHS HELPERS ///
/////////////////////

//Returns the log of val to the base 10. Any value <= 0 will return 0.
int32_t FFScript::Log10(double temp)
{
	int32_t ret = 0;
	if(temp > 0)
		ret = int32_t(zc::math::Log10(temp) * 10000.0);
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
		return zc::math::Ln(temp);
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

std::optional<StackFrame> FFScript::get_script_stack_frame(int pc)
{
	auto [source_file, line] = zasm_debug_data.resolveLocationSourceFile(pc);
	if (line > 0)
	{
		const DebugScope* scope = zasm_debug_data.resolveFunctionScope(pc);
		std::string fn_name = scope ? zasm_debug_data.getFullScopeName(scope) : "?";

		StackFrame frame{};
		frame.source_file = source_file;
		frame.line = line;
		frame.pc = pc;
		frame.function_name = fn_name;
		return frame;
	}
	else if (show_zasm_stack_traces)
	{
		// It's often useful to know which zasm instruction stuff happened at for development,
		// but that's never useful for typical users.
		StackFrame frame{};
		frame.line = pc;
		frame.pc = pc;
		frame.extra = curscript->zasm_script->name + ".zasm";
		return frame;
	}

	return std::nullopt;
}

void FFScript::handle_trace(const std::string& s, bool is_error, bool no_prefix)
{
	// -experimental-disable-script-error-logs: skip script error logging (and the
	// expensive stack-trace creation below). Scripts that error every frame (e.g.
	// divide-by-zero) otherwise dominate a benchmark. Engine diagnostics (al_trace)
	// and non-error Trace() output are unaffected.
	if (disable_script_error_logs && is_error)
		return;

	bool user_visible_trace = true;
	if (is_error)
		user_visible_trace = get_qr(qr_SCRIPTERRLOG) || DEVLEVEL > 0; // TODO: consider removing this, always logging errors.
	bool do_replay_comment = replay_is_active() && replay_get_meta_bool("script_trace");
	if (!do_replay_comment && !user_visible_trace)
		return;

	std::optional<StackTrace> stack_trace;
	std::string stack_trace_string;

	if (!s.empty() && s.back() == '\n')
		stack_trace = create_stack_trace(ri);
	if (stack_trace)
		stack_trace_string = stack_trace->to_string() + "\n";

	if (user_visible_trace)
	{
		if (!no_prefix)
		{
			bool force_context = is_error;
			PrintTracePrefix(force_context, is_error);
		}
		safe_al_trace(s);
		if (!stack_trace_string.empty())
			safe_al_trace(stack_trace_string);
	}

	if (do_replay_comment)
	{
		if (s.size() != 1 || s[0] != '\n')
			replay_step_comment(fmt::format("{}: {}", is_error ? "Error" : "Trace", s));
		if (stack_trace)
		{
			for (const auto& frame : stack_trace->frames)
			{
				std::string frame_normalized = frame.to_string();
				util::replstr(frame_normalized, "../", "");
				replay_step_comment(frame_normalized);
			}
		}
	}

	if (console_enabled && user_visible_trace)
	{
		int colors = is_error ?
			(CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_INTENSITY | CConsoleLoggerEx::COLOR_BACKGROUND_BLACK) :
			(CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY | CConsoleLoggerEx::COLOR_BACKGROUND_BLACK);
		zscript_coloured_console.safeprint(colors, s.c_str());

		if (!stack_trace_string.empty())
		{
			colors = CConsoleLoggerEx::COLOR_WHITE | CConsoleLoggerEx::COLOR_BACKGROUND_BLACK;
			zscript_coloured_console.safeprint(colors, stack_trace_string.c_str());
		}
	}

	if (auto debugger = zscript_debugger_get_if_open())
	{
		if (stack_trace)
			debugger->AddConsoleMessageWithStackTrace(std::move(s), std::move(stack_trace.value()));
		else
			debugger->AddConsoleMessage(std::move(s));
	}
}

std::string StackFrame::to_string() const
{
	if (source_file)
		return fmt::format("  at {}:{}", source_file->path, line);
	else
		return fmt::format("  at {}", extra);
}

std::string StackFrame::to_short_string() const
{
	if (source_file)
	{
		size_t pos = source_file->path.find_last_of("/\\") + 1;
		return fmt::format("{}:{}", source_file->path.substr(pos), line);
	}

	return extra;
}

std::string StackTrace::to_string() const
{
	std::vector<std::string> parts;
	for (auto& frame : frames)
		parts.push_back(frame.to_string());

	return fmt::format("{}", fmt::join(parts, "\n"));
}

bool FFScript::should_display_stack_traces()
{
	return !zasm_debug_data.debug_lines_encoded.empty() || show_zasm_stack_traces;
}

std::optional<StackTrace> FFScript::create_stack_trace(const refInfo* ri)
{
	if (!should_display_stack_traces())
		return std::nullopt;

	StackTrace result{};
	std::vector<pc_t> frames;

	frames.push_back(ri->pc);

	for (int i = ri->retsp - 1; i >= 0; i--)
	{
		pc_t pc = (*ret_stack)[i];
		frames.push_back(pc - 1);
	}

	result.frames.reserve(frames.size());

	pc_t last_pc = -1;
	int repeated_count = 0;
	for (int i = 0; i < frames.size(); i++)
	{
		pc_t pc = frames[i];
		if (last_pc == pc)
			repeated_count++;
		else
			repeated_count = 1;

		if (auto frame = get_script_stack_frame(pc))
			result.frames.push_back(*frame);

		// Elide repeated frames.
		if (repeated_count == 2)
		{
			int j = i + 1;
			int lookahead_count = 0;
			while (j < frames.size() && frames[j] == last_pc)
			{
				j++;
				lookahead_count++;
			}

			if (lookahead_count > 0)
			{
				result.frames.push_back(StackFrame{.extra = fmt::format("  ... (x{})", lookahead_count)});
				i = j - 1;
			}

			repeated_count = 0;
		}

		last_pc = pc;
	}

	if (result.frames.empty())
		return std::nullopt;

	return result;
}

void FFScript::do_trace(bool v)
{
	int32_t temp = SH::get_arg(sarg1, v);
	
	char tmp[100];
	snprintf(tmp, sizeof(tmp), (temp < 0 ? "%06d" : "%05d"), temp);
	string s2(tmp);
	s2 = s2.substr(0, s2.size() - 4) + "." + s2.substr(s2.size() - 4, 4) + "\n";
	handle_trace(s2);
}
void FFScript::do_tracel(bool v)
{
	int32_t temp = SH::get_arg(sarg1, v);

	char tmp[32];
	snprintf(tmp, sizeof(tmp), "%d\n", temp);
	handle_trace(tmp);
}

void FFScript::do_tracebool(const bool v)
{
	int32_t temp = SH::get_arg(sarg1, v);

	handle_trace(temp ? "true\n" : "false\n");
}

void FFScript::do_tracestring()
{
	int32_t arrayptr = get_register(sarg1);
	string str;
	ArrayH::getString(arrayptr, str, 512);
	handle_trace(str);
}

static int32_t zspr_varg_getter(int32_t,int32_t next_arg)
{
	return ri->zs_vargs_stack.back().at(next_arg);
}
static int32_t zspr_stack_getter(int32_t num_args, int32_t next_arg)
{
	return SH::read_stack(((ri->sp + num_args) - 1) - next_arg);
}

void FFScript::do_printf(const bool v, const bool varg)
{
	int32_t num_args, format_arrayptr;
	if(varg)
	{
		num_args = ri->zs_vargs_stack.back().size();
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
		handle_trace(zs_sprintf(formatstr.c_str(), num_args, varg ? zspr_varg_getter : zspr_stack_getter));
	}
	if(varg)
		clear_vargs_back();
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
		handle_trace(zs_sprintf(formatstr.c_str(), num_args,
			[&](int32_t,int32_t next_arg)
			{
				return arg_am.get(next_arg);
			}));
	}
}

void FFScript::do_varg_max()
{
	auto vargs = clear_vargs_back();
	int32_t num_args = vargs.size();
	int32_t val = std::numeric_limits<int32_t>::min();
	if (num_args > 0)
		val = vargs.at(0);
	for(auto q = 1; q < num_args; ++q)
	{
		int32_t tval = vargs.at(q);
		if(tval > val) val = tval;
	}
	SET_D(rEXP1, val);
}
void FFScript::do_varg_min()
{
	auto vargs = clear_vargs_back();
	int32_t num_args = vargs.size();
	int32_t val = std::numeric_limits<int32_t>::max();
	if (num_args > 0)
		val = vargs.at(0);
	for(auto q = 1; q < num_args; ++q)
	{
		int32_t tval = vargs.at(q);
		if(tval < val) val = tval;
	}
	SET_D(rEXP1, val);
}
void FFScript::do_varg_choose()
{
	auto vargs = clear_vargs_back();
	int32_t num_args = vargs.size();
	int32_t val = 0;
	if(num_args > 0)
	{
		int32_t choice = zc_rand(num_args-1);
		val = vargs.at(choice);
	}
	SET_D(rEXP1, val);
}
void FFScript::do_varg_makearray(ScriptType type, const uint32_t UID, script_object_type object_type)
{
	// Which varg positions hold objects, captured before clear_vargs_back()
	// wipes it. Needed for untyped arrays, where only some elements are objects.
	auto obj_positions = ri->zs_vargs_pos_is_object.back();
	auto vargs = clear_vargs_back();

	size_t size = vargs.size();
	SET_D(rEXP1, 0);

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

		// The array must retain its object elements, mirroring the ref_inc done
		// by the normal array-write paths (ArrayManager::push, etc.).
		if (a.HoldsObjects())
		{
			// Typed object array: every element is an object.
			for (size_t j = 0; j < size; ++j)
				script_object_ref_inc(vargs[j]);
		}
		else if (a.MaybeHoldsObjects())
		{
			// Untyped array: only some positions hold objects.
			for (int pos : obj_positions)
			{
				if (pos < 0 || (size_t)pos >= size)
					continue;
				uint32_t id = vargs[pos];
				script_object_ref_inc(id);
				if (auto obj = get_script_object(id))
					array->set_type_in_untyped_array(pos, obj->type);
			}
		}

		SET_D(rEXP1, array->id);

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

	SET_D(rEXP1, ptrval*10000);
}

void FFScript::do_breakpoint()
{
	// TODO: implement as `debugger;` statement when VS Code extension exists.
}

void FFScript::do_tracenl()
{
	handle_trace("\n", false, true);
}

std::string FFScript::GetScriptName(ScriptType script_type, int script_num)
{
	switch(script_type)
	{
		case ScriptType::Global:
		{
			switch(script_num)
			{
				case GLOBAL_SCRIPT_INIT:
					return globalmap[script_num].scriptname;
				case GLOBAL_SCRIPT_GAME:
					return globalmap[script_num].scriptname;
				case GLOBAL_SCRIPT_END:
					return globalmap[script_num].scriptname;
				case GLOBAL_SCRIPT_ONSAVELOAD:
					return globalmap[script_num].scriptname;
				case GLOBAL_SCRIPT_ONLAUNCH:
					return globalmap[script_num].scriptname;
				case GLOBAL_SCRIPT_ONCONTGAME:
					return globalmap[script_num].scriptname;
				case GLOBAL_SCRIPT_F6:
					return globalmap[script_num].scriptname;
				case GLOBAL_SCRIPT_ONSAVE:
					return globalmap[script_num].scriptname;
			}
			break;
		}
		
		case ScriptType::Hero:
		{
			switch(script_num)
			{
				case SCRIPT_HERO_INIT:
					return playermap[script_num-1].scriptname;
				case SCRIPT_HERO_ACTIVE:
					return playermap[script_num-1].scriptname;
				case SCRIPT_HERO_DEATH:
					return playermap[script_num-1].scriptname;
				case SCRIPT_HERO_WIN:
					return playermap[script_num-1].scriptname;
			}
			break;
		}

		case ScriptType::Lwpn:
			return lwpnmap[script_num-1].scriptname;

		case ScriptType::Ewpn:
			return ewpnmap[script_num-1].scriptname;

		case ScriptType::NPC:
			return npcmap[script_num-1].scriptname;

		case ScriptType::FFC:
			return ffcmap[script_num-1].scriptname;

		case ScriptType::Item:
			return itemmap[script_num-1].scriptname;

		case ScriptType::OnMap:
			return dmapmap[script_num-1].scriptname;
		case ScriptType::ScriptedActiveSubscreen:
			return dmapmap[script_num-1].scriptname;
		case ScriptType::ScriptedPassiveSubscreen:
			return dmapmap[script_num-1].scriptname;
		case ScriptType::DMap:
			return dmapmap[script_num-1].scriptname;
		
		case ScriptType::ItemSprite:
			return itemspritemap[script_num-1].scriptname;
		
		case ScriptType::Screen:
			return screenmap[script_num-1].scriptname;
		
		case ScriptType::Combo:
			return comboscriptmap[script_num-1].scriptname;
			
		case ScriptType::Generic:
			return genericmap[script_num-1].scriptname;
			
		case ScriptType::GenericFrozen:
			return genericmap[script_num-1].scriptname;
			
		case ScriptType::EngineSubscreen:
			return subscreenmap[script_num-1].scriptname;
	}

	return "";
}

std::string FFScript::GetScriptDataName(ScriptType script_type, int script_num)
{
	char buf[256] = {0};

	switch(script_type)
	{
		case ScriptType::Global:
		{
			switch(script_num)
			{
				case GLOBAL_SCRIPT_INIT:
					snprintf(buf, sizeof(buf), "Global Init(%s)", globalmap[script_num].scriptname.c_str());
					break;
				case GLOBAL_SCRIPT_GAME:
					snprintf(buf, sizeof(buf), "Global Active(%s)", globalmap[script_num].scriptname.c_str());
					break;
				case GLOBAL_SCRIPT_END:
					snprintf(buf, sizeof(buf), "Global Exit(%s)", globalmap[script_num].scriptname.c_str());
					break;
				case GLOBAL_SCRIPT_ONSAVELOAD:
					snprintf(buf, sizeof(buf), "Global SaveLoad(%s)", globalmap[script_num].scriptname.c_str());
					break;
				case GLOBAL_SCRIPT_ONLAUNCH:
					snprintf(buf, sizeof(buf), "Global Launch(%s)", globalmap[script_num].scriptname.c_str());
					break;
				case GLOBAL_SCRIPT_ONCONTGAME:
					snprintf(buf, sizeof(buf), "Global ContGame(%s)", globalmap[script_num].scriptname.c_str());
					break;
				case GLOBAL_SCRIPT_F6:
					snprintf(buf, sizeof(buf), "Global F6Menu(%s)", globalmap[script_num].scriptname.c_str());
					break;
				case GLOBAL_SCRIPT_ONSAVE:
					snprintf(buf, sizeof(buf), "Global Save(%s)", globalmap[script_num].scriptname.c_str());
					break;
			}
			break;
		}
		
		case ScriptType::Hero:
		{
			switch(script_num)
			{
				case SCRIPT_HERO_INIT:
					snprintf(buf, sizeof(buf), "Hero Init(%s)", playermap[script_num-1].scriptname.c_str());
					break;
				case SCRIPT_HERO_ACTIVE:
					snprintf(buf, sizeof(buf), "Hero Active(%s)", playermap[script_num-1].scriptname.c_str());
					break;
				case SCRIPT_HERO_DEATH:
					snprintf(buf, sizeof(buf), "Hero Death(%s)", playermap[script_num-1].scriptname.c_str());
					break;
				case SCRIPT_HERO_WIN:
					snprintf(buf, sizeof(buf), "Hero Win(%s)", playermap[script_num-1].scriptname.c_str());
					break;
			}
			break;
		}
		
		case ScriptType::Lwpn:
			snprintf(buf, sizeof(buf), "LWeapon(%u, %s)", script_num,lwpnmap[script_num-1].scriptname.c_str());
			break;
		
		case ScriptType::Ewpn:
			snprintf(buf, sizeof(buf), "EWeapon(%u, %s)", script_num,ewpnmap[script_num-1].scriptname.c_str());
			break;
		
		case ScriptType::NPC:
			snprintf(buf, sizeof(buf), "NPC(%u, %s)", script_num,npcmap[script_num-1].scriptname.c_str());
			break;
			
		case ScriptType::FFC:
			snprintf(buf, sizeof(buf), "FFC(%u, %s)", script_num,ffcmap[script_num-1].scriptname.c_str());
			break;
			
		case ScriptType::Item:
			snprintf(buf, sizeof(buf), "Item(%u, %s)", script_num,itemmap[script_num-1].scriptname.c_str());
			break;
		
		case ScriptType::OnMap:
			snprintf(buf, sizeof(buf), "DMapMap(%u, %s)", script_num,dmapmap[script_num-1].scriptname.c_str());
			break;
		case ScriptType::ScriptedActiveSubscreen:
			snprintf(buf, sizeof(buf), "DMapASub(%u, %s)", script_num,dmapmap[script_num-1].scriptname.c_str());
			break;
		case ScriptType::ScriptedPassiveSubscreen:
			snprintf(buf, sizeof(buf), "DMapPSub(%u, %s)", script_num,dmapmap[script_num-1].scriptname.c_str());
			break;
		case ScriptType::DMap:
			snprintf(buf, sizeof(buf), "DMap(%u, %s)", script_num,dmapmap[script_num-1].scriptname.c_str());
			break;
		
		case ScriptType::ItemSprite:
			snprintf(buf, sizeof(buf), "ItemSprite(%u, %s)", script_num,itemspritemap[script_num-1].scriptname.c_str());
			break;
		
		case ScriptType::Screen:
			snprintf(buf, sizeof(buf), "Screen(%u, %s)", script_num,screenmap[script_num-1].scriptname.c_str());
			break;
		
		case ScriptType::Combo:
			snprintf(buf, sizeof(buf), "Combo(%u, %s)", script_num,comboscriptmap[script_num-1].scriptname.c_str());
			break;
			
		case ScriptType::Generic:
			snprintf(buf, sizeof(buf), "Generic(%u, %s)", script_num,genericmap[script_num-1].scriptname.c_str());
			break;
			
		case ScriptType::GenericFrozen:
			snprintf(buf, sizeof(buf), "GenericFRZ(%u, %s)", script_num,genericmap[script_num-1].scriptname.c_str());
			break;
			
		case ScriptType::EngineSubscreen:
			snprintf(buf, sizeof(buf), "Subscreen(%u, %s)", script_num,subscreenmap[script_num-1].scriptname.c_str());
			break;
	}

	return buf;
}

void FFScript::PrintTracePrefix(bool force_show_context, bool is_error)
{
	std::vector<std::string> parts;

	if(DEVTIMESTAMP)
	{
		char buf[256] = {0};
		//Calculate timestamp
		struct tm * tm_struct;
		time_t sysRTC;
		time (&sysRTC);
		tm_struct = localtime (&sysRTC);

		snprintf(buf, sizeof(buf), "[%d:%d:%d] ", tm_struct->tm_hour, tm_struct->tm_min, tm_struct->tm_sec);

		parts.push_back(buf);
	}

	bool show_context = force_show_context || (get_qr(qr_TRACESCRIPTIDS) || DEVLOGGING);
	if (show_context)
	{
		if(script_funcrun)
		{
			char buf[256] = {0};
			snprintf(buf, sizeof(buf), "Destructor(%d,%s)", ri->thiskey, destructstr?destructstr->c_str():"UNKNOWN");
			parts.push_back(buf);
		}
		else
		{
			parts.push_back(GetScriptDataName(curScriptType, curScriptNum));
		}
	}

	std::string prefix = fmt::format("{}: ", fmt::join(parts, " "));

	al_trace("%s", prefix.c_str());

	CConsoleLoggerEx console = zscript_coloured_console;
	if (console_enabled)
	{
		int colors = is_error ?
			(CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_INTENSITY | CConsoleLoggerEx::COLOR_BACKGROUND_BLACK) :
			(CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY | CConsoleLoggerEx::COLOR_BACKGROUND_BLACK);
		zscript_coloured_console.safeprint(colors, prefix.c_str());
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
	word digits = zc_max(mindigits - 1, word(floor(zc::math::Ln(double(x)) / zc::math::Ln(double(base)))));
	
	for(int32_t i = digits; i >= 0; i--)
	{
		s2 += coeff[word(floor(x / zc::math::Pow(double(base), i))) % base];
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

	s2 += "\n";
	handle_trace(s2);
}

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
		ri->lwpnref = uid;
	else
	{
		ri->lwpnref = 0;
	}
}

void FFScript::do_loadeweapon_by_script_uid(const bool v)
{
	int32_t uid = SH::get_arg(sarg1, v);
	if (ResolveEWeapon_checkSpriteList(uid))
		ri->ewpnref = uid;
	else
	{
		ri->ewpnref = 0;
	}
}


void FFScript::do_loadnpc_by_script_uid(const bool v)
{
	int32_t uid = SH::get_arg(sarg1, v);
	if (ResolveSprite<enemy>(uid, "enemy"))
		ri->npcref = uid;
	else
	{
		ri->npcref = 0;
	}
}

//Combo Scripts

void FFScript::clear_combo_scripts()
{
	combo_id_cache.clear();
	combo_id_cache.resize(region_num_rpos * 7);
	std::fill(combo_id_cache.begin(), combo_id_cache.end(), -1);
	FFCore.clear_script_engine_data_of_type(ScriptType::Combo);
}

void FFScript::clear_combo_script(const rpos_handle_t& rpos_handle)
{
	int32_t index = get_combopos_ref(rpos_handle);
	combo_id_cache[index] = -1;
	combopos_modified = index;
	clear_script_engine_data(ScriptType::Combo, index);
}

int32_t FFScript::combo_script_engine([[maybe_unused]] const bool preload, const bool waitdraw)
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
	case SUBDATA_CLOSE:
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
		case ARCSINR:
		case BITNOT:
		case BITNOT32:
		case CASTBOOLF:
		case CEILING:
		case COMPAREV2:
		case COSR:
		case COSV:
		case DIVR:
		case DIVV:
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
		case LPOWERR:
		case LPOWERV:
		case LPOWERV2:
		case LSHIFTR:
		case LSHIFTR32:
		case LSHIFTV:
		case LSHIFTV32:
		case MAXR:
		case MAXV:
		case MINR:
		case MINV:
		case MODR:
		case MODV:
		case MODV2:
		case MULTR:
		case MULTV:
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
		case POWERV2:
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
		case SQROOTR:
		case SQROOTV:
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

#ifdef DEBUG_REGISTER_DEPS

static std::array<int, 8> debug_deps_cur_regs;
static int debug_ref;

#define REG_R ((int)ARGTY::READ_REG)
#define REG_W ((int)ARGTY::WRITE_REG)

int debug_get_d(int r)
{
	CHECK(!(debug_deps_cur_regs[r] & REG_W));
	debug_deps_cur_regs[r] |= REG_R;
	return ri->d[r];
}

int debug_set_d(int r, int)
{
	debug_deps_cur_regs[r] |= REG_W;
	return ri->d[r];
}

int debug_get_ref(std::string reg_name)
{
	util::upperstr(reg_name);
	util::replstr(reg_name, "REF", "");
	reg_name = "REF" + reg_name;
	int r = get_script_variable(reg_name).value();
	return debug_get_ref(r);
}

int debug_get_ref(int r)
{
	if (r == debug_ref) return get_ref(r);

	CHECK(!debug_ref);
	debug_ref = r;
	return get_ref(r);
}

static const char* get_d_reg_name(int r)
{
	if (r == 0) return "rINDEX";
	if (r == 1) return "rINDEX2";
	if (r == 2) return "rEXP1";
	if (r == 3) return "rEXP2";
	if (r == 4) return "rSFRAME";
	if (r == 5) return "rNUL";
	if (r == 6) return "rSFTEMP";
	if (r == 7) return "rWHAT_NO_7";
	NOTREACHED();
}

static void reset_test_ri(refInfo* ri)
{
	*ri = {};
	ri->sp = MAX_STACK_SIZE - 100;
	ri->screenref = cur_screen;
	ri->msgdataref = -1;
}

// This matches "Sort lines" command in VS Code.
static bool natural_less(const std::string& a, const std::string& b)
{
	auto it_a = a.begin();
	auto it_b = b.begin();

	while (it_a != a.end() && it_b != b.end())
	{
		if (std::isdigit(static_cast<unsigned char>(*it_a)) && std::isdigit(static_cast<unsigned char>(*it_b)))
		{
			// Parse numbers
			std::string num_a_str, num_b_str;
			while (it_a != a.end() && std::isdigit(static_cast<unsigned char>(*it_a)))
				num_a_str += *it_a++;
			while (it_b != b.end() && std::isdigit(static_cast<unsigned char>(*it_b)))
				num_b_str += *it_b++;

			if (num_a_str.length() != num_b_str.length())
				return num_a_str.length() < num_b_str.length();

			if (num_a_str != num_b_str)
				return num_a_str < num_b_str;
		}
		else
		{
			char ca = static_cast<char>(std::tolower(static_cast<unsigned char>(*it_a)));
			char cb = static_cast<char>(std::tolower(static_cast<unsigned char>(*it_b)));
			if (ca != cb)
				return ca < cb;
			++it_a;
			++it_b;
		}
	}

	if (it_a == a.end() && it_b == b.end())
		return false;
	if (it_a == a.end())
		return *it_b != '_';
	if (it_b == b.end())
		return *it_a == '_';

	return it_a == a.end() && it_b != b.end();
}

void print_d_register_deps()
{
	refInfo testRi;
	ri = &testRi;
	std::array<int32_t, MAX_STACK_SIZE> testStack{};
	testStack.fill(10000);
	stack = (int32_t (*)[MAX_STACK_SIZE])testStack.data();

	// value -> case labels
	std::map<std::string, std::vector<std::string>> value_to_labels;
	std::map<std::string, std::vector<std::string>> value_to_labels2;

	for (int i = 0; i < NUMVARIABLES; i++)
	{
		auto [sv, _] = get_script_variable(i);
		if (!sv) continue;

		reset_test_ri(&testRi);
		for (int j = 0; j < 8; j++) ri->d[j] = i == 4891 ? 10000 : 0;

		debug_deps_cur_regs = {};
		debug_ref = 0;
		get_register(i);

		bool any = false;
		for (int j = 0; j < 8; j++)
		{
			if (debug_deps_cur_regs[j])
				any = true;
		}

		if (any)
		{
			std::vector<std::string> reg_names;
			for (int j = 0; j < 8; j++)
			{
				CHECK(!(debug_deps_cur_regs[j] & REG_W));
				if (!debug_deps_cur_regs[j])
					continue;

				reg_names.push_back(get_d_reg_name(j));
			}

			std::string value = fmt::format("{{{}}}", fmt::join(reg_names, ", "));
			if (auto* labels = util::find(value_to_labels, value))
				labels->push_back(sv->name);
			else
				value_to_labels[value] = {sv->name};
		}

		if (debug_ref)
		{
			std::string value = get_script_variable(debug_ref).first->name;
			if (auto* labels = util::find(value_to_labels2, value))
				labels->push_back(sv->name);
			else
				value_to_labels2[value] = {sv->name};
		}
	}

	fmt::println("static std::vector<int> _get_register_dependencies(int reg)");
	fmt::println("{{");
	fmt::println("\tswitch (reg)");
	fmt::println("\t{{");

	for (auto& [value, labels] : value_to_labels | std::views::reverse)
	{
		std::sort(labels.begin(), labels.end(), natural_less);
		for (auto& label : labels)
			fmt::println("\t\tcase {}:", label);
		fmt::println("\t\t{{");
		fmt::println("\t\t\treturn {};", value);
		fmt::println("\t\t}}");
		fmt::println("");
	}
	value_to_labels.clear();

	fmt::println("\t}}");
	fmt::println("");
	fmt::println("\treturn {{}};");
	fmt::println("}}");

	fmt::println("std::optional<int> get_register_ref_dependency(int reg)");
	fmt::println("{{");
	fmt::println("\tswitch (reg)");
	fmt::println("\t{{");

	for (auto& [value, labels] : value_to_labels2)
	{
		std::sort(labels.begin(), labels.end(), natural_less);
		for (auto& label : labels)
			fmt::println("\t\tcase {}:", label);
		fmt::println("\t\t\treturn {};", value);
		fmt::println("");
	}

	fmt::println("\t}}");
	fmt::println("");
	fmt::println("\treturn {{}};");
	fmt::println("}}");

	curscript = new script_data(ScriptType::None, 0);
	curscript->zasm_script = std::make_shared<zasm_script>();

	JittedScriptInstance j_instance{};
	j_instance.script = curscript;

	for (int i = 0; i < NUMCOMMANDS; i++)
	{
		if (command_is_goto(i) || command_is_wait(i) || command_uses_comparison_result(i) || command_writes_comparison_result(i))
			continue;

		switch (i)
		{
			case CALLFUNC:
			case CLOSEWIPE:
			case CLOSEWIPESHAPE:
			case DEALLOCATEMEMR:
			case FXWAVYR:
			case FXWAVYV:
			case FXZAPR:
			case FXZAPV:
			case GAMECONTINUE:
			case GAMEEND:
			case GAMEEXIT:
			case GAMERELOAD:
			case GAMESAVECONTINUE:
			case GAMESAVEQUIT:
			case GOTOR:
			case LOAD_INTERNAL_ARRAY_REF:
			case LOAD_INTERNAL_ARRAY:
			case MARK_TYPE_REG:
			case OBJ_OWN_ARRAY:
			case OBJ_OWN_BITMAP:
			case OBJ_OWN_DIR:
			case OBJ_OWN_FILE:
			case OBJ_OWN_PALDATA:
			case OBJ_OWN_RNG:
			case OBJ_OWN_STACK:
			case OPENWIPE:
			case OPENWIPESHAPE:
			case POP:
			case POPARGS:
			case PUSHARGSR:
			case PUSHARGSV:
			case PUSHR:
			case PUSHV:
			case QUIT:
			case RETURN:
			case RETURNFUNC:
			case SAVE:
			case SAVEQUITSCREEN:
			case SAVESCREEN:
			case SET_OBJECT:
			case SHOWF6SCREEN:
			case STARTDESTRUCTOR:
			case WAVYIN:
			case WAVYOUT:
			case ZAPIN:
			case ZAPOUT:
			case ZCLASS_CONSTRUCT:
			case ZCLASS_MARK_TYPE:
				continue;
		}

		if (!get_script_command(i))
			continue;

		curscript->zasm_script->zasm.push_back({(word)i});
	}

	curscript->zasm_script->zasm.push_back({NOP});
	curscript->zasm_script->size = curscript->end_pc = curscript->zasm_script->zasm.size();

	for (int i = 0; i < curscript->zasm_script->size; i++)
	{
		reset_test_ri(&testRi);

		debug_deps_cur_regs = {};
		debug_ref = 0;

		int command = curscript->zasm_script->zasm[i].command;
		auto sc = get_script_command(command);

		switch (command)
		{
			case REF_DEC:
			case REF_INC:
			{
				debug_deps_cur_regs[rSFRAME] = REG_R;
				break;
			}

			default:
			{
				run_script_jit_one(&j_instance, i, MAX_STACK_SIZE - 100);
			}
		}

		bool any = false;
		for (int j = 0; j < 8; j++)
		{
			if (debug_deps_cur_regs[j])
				any = true;
			if (debug_deps_cur_regs[j] & REG_W)
				CHECK(!command_is_pure(command));
		}
		if (!any)
			continue;

		std::vector<std::string> parts;
		for (int j = 0; j < 8; j++)
		{
			if (!debug_deps_cur_regs[j])
				continue;

			bool r = debug_deps_cur_regs[j] & REG_R;
			bool w = debug_deps_cur_regs[j] & REG_W;
			std::string value;
			if (r && w) value = "REG_RW";
			else if (r) value = "REG_R";
			else if (w) value = "REG_W";

			parts.push_back(fmt::format("{{{}, {}}}", get_d_reg_name(j), value));
		}

		std::string value = fmt::format("{{{}}}", fmt::join(parts, ", "));
		if (auto* labels = util::find(value_to_labels, value))
			labels->push_back(sc->name);
		else
			value_to_labels[value] = {sc->name};
	}

	value_to_labels["{{CLASS_THISKEY, REG_W}}"] = {"ZCLASS_MARK_TYPE"};
	value_to_labels["{{rEXP1, REG_R}, {CLASS_THISKEY, REG_W}}"] = {"ZCLASS_CONSTRUCT"};
	value_to_labels["{{SP, REG_RW}, {SP2, REG_RW}}"] = {"POP", "POPARGS", "PUSHARGSR", "PUSHARGSV", "PUSHR", "PUSHV"};

	fmt::println("std::initializer_list<CommandDependency> get_command_implicit_dependencies(int command)");
	fmt::println("{{");
	fmt::println("\ttypedef std::initializer_list<CommandDependency> T;");
	fmt::println("");
	fmt::println("\tswitch (command)");
	fmt::println("\t{{");

	for (auto& [value, labels] : value_to_labels | std::views::reverse)
	{
		std::sort(labels.begin(), labels.end(), natural_less);
		for (auto& label : labels)
			fmt::println("\t\tcase {}:", label);
		fmt::println("\t\t{{");
		fmt::println("\t\t\tstatic T r = {};", value);
		fmt::println("\t\t\treturn r;");
		fmt::println("\t\t}}");
		fmt::println("");
	}
	value_to_labels.clear();

	fmt::println("\t}}");
	fmt::println("");
	fmt::println("\treturn {{}};");
	fmt::println("}}");

	zc_exit(0);
}

#endif
