//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zelda.cc
//
//  Main code for Zelda Classic. Originally written in
//  SPHINX C--, now rewritten in DJGPP with Allegro.
//
//--------------------------------------------------------

#include "precompiled.h" //always first

#include "gamedata.h"
#include <stdio.h>
#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "zelda.h"
#include "items.h"
#include "pal.h"
#include "base/util.h"
#include "ffscript.h"

using namespace util;
extern FFScript FFCore;
#ifndef IS_ZQUEST
extern portal* mirror_portal;
#endif
extern int32_t dlevel;
extern void flushItemCache();
extern zinitdata zinit;
extern void Z_eventlog(char *format,...);
extern void ringcolor(bool forceDefault);

// Debug variables: these log certain operations on gamedata when active.
// Should help me debug those item bugs.

// #define DEBUG_GD_ITEMS
// #define DEBUG_GD_COUNTERS
//#define DEBUG_GD_HCP

void gamedata::Clear()
{
    isclearing=true;
    std::fill(_name, _name+9, 0);
    _quest=0;
    _deaths=0;
    _cheat=0;
    std::fill(item, item+MAXITEMS, false);
    std::fill(items_off, items_off+MAXITEMS, 0);
    std::fill(_maxcounter, _maxcounter+32, 0);
    std::fill(_counter, _counter+32, 0);
    std::fill(_dcounter, _dcounter+32, 0);
    std::fill(version, version+9, 0);
    std::fill(title, title+65, 0);
    _hasplayed=false;
    _time=0;
    _timevalid=false;
    std::fill(lvlitems, lvlitems+MAXLEVELS, 0);
    std::fill(lvlkeys, lvlkeys+MAXLEVELS, 0);
    std::fill(lvlswitches, lvlswitches+MAXLEVELS, 0);
    std::fill(item_messages_played, item_messages_played+MAXITEMS, 0);
    _continue_scrn=0;
    _continue_dmap=0;
    std::fill(_generic, _generic+256, 0);
    std::fill(visited, visited+MAXDMAPS, 0);
    std::fill(bmaps, bmaps+MAXDMAPS*128, 0);
    std::fill(maps, maps+MAXMAPS2*MAPSCRSNORMAL, 0);
    std::fill(xstates, xstates+MAXMAPS2*MAPSCRSNORMAL, 0);
    std::fill(guys, guys+MAXMAPS2*MAPSCRSNORMAL, 0);
    std::fill(qstpath, qstpath+2048, 0);
    std::fill(icon, icon+128, 0);
    std::fill(pal, pal+48, 0);
    
    for(int32_t i=0; i<MAXDMAPS*MAPSCRSNORMAL; i++) std::fill(screen_d[i], screen_d[i]+8, 0);
    
    std::fill(global_d, global_d+MAX_SCRIPT_REGISTERS, 0);
    globalRAM.clear();
    awpn=0;
    bwpn=0;
    xwpn=0;
    ywpn=0;
    forced_awpn = -1; 
    forced_bwpn = -1;
    forced_xwpn = -1; 
    forced_ywpn = -1;
	
	memset(bottleSlots, 0, sizeof(bottleSlots));
	clear_portal();
	
	clear_genscript();
    std::fill(gswitch_timers, gswitch_timers+256, 0);
	user_objects.clear();
    isclearing=false;
}

void gamedata::Copy(const gamedata& g)
{
    for(byte i = 0; i < 9; i++)
        _name[i] = g._name[i];
        
    _quest = g._quest;
    _deaths = g._deaths;
    _cheat = g._cheat;
    
    for(word i = 0; i < MAXITEMS; i++)
    {
        item[i] = g.item[i];
        items_off[i] = g.items_off[i];
    }
    
    for(byte i = 0; i < 32; i++)
    {
        _maxcounter[i] = g._maxcounter[i];
        _counter[i] = g._counter[i];
        _dcounter[i] = g._dcounter[i];
    }
    
    for(byte i = 0; i < 9; i++)
        version[i] = g.version[i];
        
    for(byte i = 0; i < 65; i++)
        title[i] = g.title[i];
        
    _hasplayed = g._hasplayed;
    _time = g._time;
    _timevalid = g._timevalid;
    
    for(word i = 0; i < MAXLEVELS; i++)
    {
        lvlitems[i] = g.lvlitems[i];
        lvlkeys[i] = g.lvlkeys[i];
        lvlswitches[i] = g.lvlswitches[i];
    }
	
	for(auto q = 0; q < MAXITEMS; ++q)
		item_messages_played[q] = g.item_messages_played[q];
    
    _continue_scrn = g._continue_scrn;
    _continue_dmap = g._continue_dmap;
    
    for(word i = 0; i < 256; i++)
        _generic[i] = g._generic[i];
        
    for(word i = 0; i < MAXDMAPS; i++)
        visited[i] = g.visited[i];
        
    for(dword i = 0; i < MAXDMAPS*128; i++)
        bmaps[i] = g.bmaps[i];
        
    for(dword i = 0; i < MAXMAPS2*MAPSCRSNORMAL; i++)
    {
        maps[i] = g.maps[i];
        xstates[i] = g.xstates[i];
        guys[i] = g.guys[i];
    }
    
    for(word i = 0; i < 2048; i++)
        qstpath[i] = g.qstpath[i];
        
    for(byte i = 0; i < 128; i++)
        icon[i] = g.icon[i];
        
    for(byte i = 0; i < 48; i++)
        pal[i] = g.pal[i];
        
    for(dword i = 0; i < MAXDMAPS*MAPSCRSNORMAL; i++)
        for(byte j = 0; j < 8; j++)
            screen_d[i][j] = g.screen_d[i][j];
            
    for(word i = 0; i < MAX_SCRIPT_REGISTERS; i++)
        global_d[i] = g.global_d[i];
        
    awpn = g.awpn;
    bwpn = g.bwpn;
    xwpn = g.xwpn;
    ywpn = g.ywpn;
    
    forced_awpn = g.forced_awpn; 
    forced_bwpn = g.forced_bwpn;
    forced_xwpn = g.forced_xwpn; 
    forced_ywpn = g.forced_ywpn;
	
	for(size_t q = 0; q < 256; ++q)
		bottleSlots[q] = g.bottleSlots[q];
	
	portaldestdmap = g.portaldestdmap;
	portalsrcdmap = g.portalsrcdmap;
	portalscr = g.portalscr;
	portalx = g.portalx;
	portaly = g.portaly;
	portalsfx = g.portalsfx;
	portalwarpfx = g.portalwarpfx;
	portalspr = g.portalspr;
	
	memcpy(gen_doscript, g.gen_doscript, sizeof(gen_doscript));
	memcpy(gen_exitState, g.gen_exitState, sizeof(gen_exitState));
	memcpy(gen_reloadState, g.gen_reloadState, sizeof(gen_reloadState));
	memcpy(gen_eventstate, g.gen_eventstate, sizeof(gen_eventstate));
	memcpy(gen_initd, g.gen_initd, sizeof(gen_initd));
	memcpy(gen_dataSize, g.gen_dataSize, sizeof(gen_dataSize));
	for(size_t q = 0; q < NUMSCRIPTSGENERIC; ++q)
	{
		gen_data[q].clear();
		gen_data[q].resize(g.gen_data[q].size());
		gen_data[q] = g.gen_data[q];
	}
	for(size_t q = 0; q < 256; ++q)
	{
		gswitch_timers[q] = g.gswitch_timers[q];
	}
#ifndef IS_ZQUEST
	user_objects = g.user_objects;
#endif
}

void gamedata::save_user_objects()
{
	user_objects.clear();
#ifndef IS_ZQUEST
	for(int32_t q = 0; q < MAX_USER_OBJECTS; ++q)
	{
		user_object& obj = script_objects[q];
		if(obj.reserved && obj.isGlobal())
		{
			saved_user_object& save_obj = user_objects.emplace_back();
			save_obj.obj = obj;
			save_obj.object_index = q;
			obj.save_arrays(save_obj.held_arrays);
		}
	}
#endif
}
void gamedata::load_user_objects()
{
#ifndef IS_ZQUEST
	FFCore.user_objects_init();
	for(saved_user_object& obj : user_objects)
	{
		auto ind = obj.object_index;
		script_objects[ind] = obj.obj;
		script_objects[ind].load_arrays(obj.held_arrays);
	}
#endif
}

void gamedata::clear_genscript()
{
	memset(gen_doscript, 0, sizeof(gen_doscript));
	memset(gen_exitState, 0, sizeof(gen_exitState));
	memset(gen_reloadState, 0, sizeof(gen_reloadState));
	memset(gen_eventstate, 0, sizeof(gen_eventstate));
	memset(gen_initd, 0, sizeof(gen_initd));
	memset(gen_dataSize, 0, sizeof(gen_dataSize));
	for(size_t q = 0; q < NUMSCRIPTSGENERIC; ++q)
	{
		gen_data[q].clear();
		gen_data[q].resize(0);
	}
}

char *gamedata::get_name()
{
    return _name;
}
void gamedata::set_name(const char *n)
{
    strncpy(_name, n, 8);
    _name[8]='\0';
    return;
}

byte gamedata::get_quest()
{
    return _quest;
}
void gamedata::set_quest(byte q)
{
    _quest=q;
    return;
}
void gamedata::change_quest(int16_t q)
{
    _quest+=q;
    return;
}

word gamedata::get_counter(byte c)
{
    if(c>=MAX_COUNTERS)  // Sanity check
        return 0;
        
    return _counter[c];
}

void gamedata::set_counter(word change, byte c)
{
#ifdef DEBUG_GD_COUNTERS
    al_trace("Changing counter %i from %i to %i\n", c, _counter[c], change);
#endif
    
    if(c>=MAX_COUNTERS)  // Sanity check
        return;
        
    if(game!=NULL)
    {
        int32_t ringID=current_item_id(itype_ring, true);
        _counter[c]=zc_max(change, 0);
        
        // ringcolor is very slow, so make sure the ring has actually changed
        if(ringID!=current_item_id(itype_ring, true))
            ringcolor(false);
    }
    else
        _counter[c]=zc_max(change, 0);
        
    return;
}

void gamedata::change_counter(int16_t change, byte c)
{
#ifdef DEBUG_GD_COUNTERS
    al_trace("Changing counter %i from %i by %i\n", c, _counter[c], change);
#endif
    
    if(c>=MAX_COUNTERS)  // Sanity check
        return;
        
    if(game!=NULL)
    {
        int32_t ringID=current_item_id(itype_ring, true);
        _counter[c]=vbound(_counter[c]+change, 0, _maxcounter[c]);
        
        if(ringID!=current_item_id(itype_ring, true))
            ringcolor(false);
    }
    else
        _counter[c]=vbound(_counter[c]+change, 0, _maxcounter[c]);
        
    return;
}

word gamedata::get_maxcounter(byte c)
{
    if(c>=MAX_COUNTERS)  // Sanity check
        return 0;
        
    return _maxcounter[c];
}

void gamedata::set_maxcounter(word change, byte c)
{
#ifdef DEBUG_GD_COUNTERS

    if(c==0) al_trace("Changing max counter %i from %i to %i\n", c, _maxcounter[c], change);
    
#endif
    
    if(c==2)
    {
        set_maxbombs(change);
        return;
    }
    
    if(c>=MAX_COUNTERS)  // Sanity check
        return;
        
    _maxcounter[c]=change;
    return;
}

void gamedata::change_maxcounter(int16_t change, byte c)
{
#ifdef DEBUG_GD_COUNTERS
    al_trace("Changing max counter %i from %i by +%i\n", c, _maxcounter[c], change);
#endif
    
    if(c==2)
    {
        change_maxbombs(change);
        return;
    }
    
    if(c>=MAX_COUNTERS)  // Sanity check
        return;
        
    _maxcounter[c]=zc_max(0, _maxcounter[c]+change);
    return;
}

int16_t gamedata::get_dcounter(byte c)
{
    if(c>=MAX_COUNTERS)  // Sanity check
        return 0;
        
    return _dcounter[c];
}

void gamedata::set_dcounter(int16_t change, byte c)
{
#ifdef DEBUG_GD_COUNTERS

    if(c==0)
        al_trace("Changing D counter %i from %i to %i\n", c, _dcounter[c], change);
        
#endif
        
    if(c>=MAX_COUNTERS)  // Sanity check
        return;
        
    if(game!=NULL)
    {
        int32_t ringID=current_item_id(itype_ring, true);
        _dcounter[c]=change;
        
        if(ringID!=current_item_id(itype_ring, true))
            ringcolor(false);
    }
    else
        _dcounter[c]=change;
        
    return;
}

void gamedata::change_dcounter(int16_t change, byte c)
{
#ifdef DEBUG_GD_COUNTERS

    if(c==0) al_trace("Changing D counter %i from %i by %i\n", c, _dcounter[c], change);
    
#endif
    
    if(c>=MAX_COUNTERS)  // Sanity check
        return;
        
    if(game!=NULL)
    {
        int32_t ringID=current_item_id(itype_ring, true);
        _dcounter[c]+=change;
        
        if(ringID!=current_item_id(itype_ring, true))
            ringcolor(false);
    }
    else
        _dcounter[c]+=change;
        
    return;
}

int32_t gamedata::get_generic(byte c)
{
    return _generic[c];
}

void gamedata::set_generic(int32_t change, byte c)
{
    _generic[c]=change;
    return;
}

void gamedata::change_generic(int32_t change, byte c)
{
    _generic[c]+=change;
    return;
}

word gamedata::get_life()
{
    return get_counter(0);
}
void gamedata::set_life(word l)
{
    if(l <= 0) l = 0;
    
    set_counter(l, 0);
    return;
}
void gamedata::change_life(int16_t l)
{
    change_counter(l, 0);
    
    if(_dcounter[0] <= 0) _dcounter[0] = 0;
    
    return;
}

word gamedata::get_maxlife()
{
    return get_maxcounter(0);
}
void gamedata::set_maxlife(word m)
{
    set_maxcounter(m, 0);
    return;
}
void gamedata::change_maxlife(int16_t m)
{
    change_maxcounter(m, 0);
    return;
}

int16_t gamedata::get_drupy()
{
    return get_dcounter(1);
}
void gamedata::set_drupy(int16_t d)
{
    set_dcounter(d, 1);
    return;
}
void gamedata::change_drupy(int16_t d)
{
    change_dcounter(d, 1);
    return;
}

word gamedata::get_rupies()
{
    return get_counter(1);
}
word gamedata::get_spendable_rupies()
{
    if(get_bit(quest_rules, qr_SHOPCHEAT) || get_dcounter(1)>=0)
        return get_counter(1);
    else
        return get_counter(1)+get_dcounter(1);
}
void gamedata::set_rupies(word r)
{
    set_counter(r, 1);
    return;
}
void gamedata::change_rupies(int16_t r)
{
    change_counter(r, 1);
    return;
}

word gamedata::get_maxarrows()
{
    return get_maxcounter(3);
}
void gamedata::set_maxarrows(word a)
{
    set_maxcounter(a, 3);
}
void gamedata::change_maxarrows(int16_t a)
{
    change_maxcounter(a, 3);
    return;
}

word gamedata::get_arrows()
{
    return get_counter(3);
}
void gamedata::set_arrows(word a)
{
    set_counter(a, 3);
}
void gamedata::change_arrows(int16_t a)
{
    change_counter(a, 3);
    return;
}

word gamedata::get_deaths()
{
    return _deaths;
}
void gamedata::set_deaths(word d)
{
    _deaths=d;
    return;
}
void gamedata::change_deaths(int16_t d)
{
    _deaths+=d;
    return;
}

word gamedata::get_keys()
{
    return get_counter(5);
}
void gamedata::set_keys(word k)
{
    set_counter(k, 5);
    return;
}
void gamedata::change_keys(int16_t k)
{
    change_counter(k, 5);
    return;
}

word gamedata::get_bombs()
{
    return get_counter(2);
}
void gamedata::set_bombs(word k)
{
    set_counter(k, 2);
    return;
}
void gamedata::change_bombs(int16_t k)
{
    change_counter(k, 2);
    return;
}

word gamedata::get_maxbombs()
{
    return get_maxcounter(2);
}
void gamedata::set_maxbombs(word b, bool setSuperBombs)
{
    _maxcounter[2]=b;
    int32_t div = zinit.bomb_ratio;
    
    if(div != 0 && setSuperBombs)
        set_maxcounter(b/div,6);
        
    return;
}
void gamedata::change_maxbombs(int16_t b)
{
    _maxcounter[2]+=b;
    int32_t div = zinit.bomb_ratio;
    
    if(div != 0)
        change_maxcounter(b/div,6);
        
    return;
}

word gamedata::get_sbombs()
{
    return get_counter(6);
}
void gamedata::set_sbombs(word k)
{
    set_counter(k, 6);
    return;
}
void gamedata::change_sbombs(int16_t k)
{
    change_counter(k, 6);
    return;
}

word gamedata::get_wlevel()
{
    return get_generic(3);
}
void gamedata::set_wlevel(word l)
{
    set_generic(l, 3);
    return;
}
void gamedata::change_wlevel(int16_t l)
{
    change_generic(l, 3);
    return;
}

byte gamedata::get_cheat()
{
    return _cheat&(~DIDCHEAT_BIT);
}
void gamedata::set_cheat(byte c)
{
	_cheat = (_cheat&DIDCHEAT_BIT) | vbound(c,0,4);
    return;
}
void gamedata::did_cheat(bool set)
{
	SETFLAG(_cheat, DIDCHEAT_BIT, set);
}
bool gamedata::did_cheat()
{
	return (_cheat&DIDCHEAT_BIT)!=0;
}

byte gamedata::get_hasplayed()
{
    return _hasplayed;
}
void gamedata::set_hasplayed(byte p)
{
    _hasplayed=p;
    return;
}
void gamedata::change_hasplayed(int16_t p)
{
    _hasplayed+=p;
    return;
}

dword gamedata::get_time()
{
    return _time;
}
void gamedata::set_time(dword t)
{
    _time=t;
    return;
}
void gamedata::change_time(int64_t t)
{
    _time+=t;
    return;
}

byte gamedata::get_timevalid()
{
    return _timevalid;
}
void gamedata::set_timevalid(byte t)
{
    _timevalid=t;
    return;
}
void gamedata::change_timevalid(int16_t t)
{
    _timevalid+=t;
    return;
}

byte gamedata::get_HCpieces()
{
    return get_generic(0);
}
void gamedata::set_HCpieces(byte p)
{
#ifdef DEBUG_GD_HCP
    al_trace("Setting HCP to %i\n",p);
#endif
    
    set_generic(p, 0);
    return;
}
void gamedata::change_HCpieces(int16_t p)
{
#ifdef DEBUG_GD_HCP
    al_trace("Changing HCP by %d to %d\n",p, get_generic(0));
#endif
    change_generic(p, 0);
    return;
}

byte gamedata::get_continue_scrn()
{
    return _continue_scrn;
}
void gamedata::set_continue_scrn(byte s)
{
    if(!isclearing && _continue_scrn != s) Z_eventlog("Continue screen set to %x\n", s);
    
    _continue_scrn=s;
    return;
}
void gamedata::change_continue_scrn(int16_t s)
{
    if(!isclearing && s!=0) Z_eventlog("Continue screen set to %x\n", _continue_scrn+s);
    
    _continue_scrn+=s;
    return;
}

word gamedata::get_continue_dmap()
{
    return _continue_dmap;
}
void gamedata::set_continue_dmap(word d)
{
    if(!isclearing && _continue_dmap!=d) Z_eventlog("Continue DMap set to %d\n", d);
    
    _continue_dmap=d;
    return;
}
void gamedata::change_continue_dmap(int16_t d)
{
    if(!isclearing && d!=0) Z_eventlog("Continue DMap set to %d\n", _continue_dmap+d);
    
    _continue_dmap+=d;
    return;
}


word gamedata::get_maxmagic()
{
    return get_maxcounter(4);
}
void gamedata::set_maxmagic(word m)
{
    set_maxcounter(m, 4);
    return;
}
void gamedata::change_maxmagic(int16_t m)
{
    change_maxcounter(m, 4);
    return;
}

word gamedata::get_magic()
{
    return get_counter(4);
}
void gamedata::set_magic(word m)
{
    set_counter(m, 4);
    return;
}
void gamedata::change_magic(int16_t m)
{
    change_counter(m, 4);
    return;
}

int16_t gamedata::get_dmagic()
{
    return get_dcounter(4);
}
void gamedata::set_dmagic(int16_t d)
{
    set_dcounter(d, 4);
    return;
}
void gamedata::change_dmagic(int16_t d)
{
    change_dcounter(d, 4);
    return;
}

byte gamedata::get_magicdrainrate()
{
    return get_generic(1);
}
void gamedata::set_magicdrainrate(byte r)
{
    set_generic(r, 1);
    return;
}
void gamedata::change_magicdrainrate(int16_t r)
{
    change_generic((char)r, 1);
    return;
}

byte gamedata::get_canslash()
{
    return get_generic(2);
}
void gamedata::set_canslash(byte s)
{
    set_generic(s, 2);
    return;
}
void gamedata::change_canslash(int16_t s)
{
    change_generic(s, 2);
    return;
}

byte gamedata::get_lkeys()
{
    return lvlkeys[dlevel];
}

/*bool gamedata::get_item(int32_t id)
{
return item[id];
}*/

byte gamedata::get_hcp_per_hc()
{
    return get_generic(4);
}

void gamedata::set_hcp_per_hc(byte val)
{
    set_generic(val, 4);
}

byte gamedata::get_cont_hearts()
{
    return get_generic(5);
}

void gamedata::set_cont_hearts(byte val)
{
    set_generic(val, 5);
}

bool gamedata::get_cont_percent()
{
    return get_generic(6) != 0;
}

void gamedata::set_cont_percent(bool ispercent)
{
    set_generic(ispercent ? 1 : 0, 6);
}


byte gamedata::get_hp_per_heart()
{
	byte b = get_generic(genHP_PER_HEART);
	return b ? b : 16;
}
void gamedata::set_hp_per_heart(byte val)
{
	set_generic(val, genHP_PER_HEART);
}

byte gamedata::get_mp_per_block()
{
	byte b = get_generic(genMP_PER_BLOCK);
	return b ? b : 32;
}
void gamedata::set_mp_per_block(byte val)
{
	set_generic(val, genMP_PER_BLOCK);
}

byte gamedata::get_hero_dmgmult()
{
	byte b = get_generic(genHERO_DMG_MULT);
	return b ? b : 1;
}
void gamedata::set_hero_dmgmult(byte val)
{
	set_generic(val, genHERO_DMG_MULT);
}

byte gamedata::get_ene_dmgmult()
{
	byte b = get_generic(genENE_DMG_MULT);
	return b ? b : 1;
}
void gamedata::set_ene_dmgmult(byte val)
{
	set_generic(val, genENE_DMG_MULT);
}

byte gamedata::get_dither_type()
{
	return get_generic(genDITH_TYPE);
}
void gamedata::set_dither_type(byte val)
{
	set_generic(val, genDITH_TYPE);
}

byte gamedata::get_dither_arg()
{
	return get_generic(genDITH_ARG);
}
void gamedata::set_dither_arg(byte val)
{
	set_generic(val, genDITH_ARG);
}

byte gamedata::get_dither_perc()
{
	return zc_min(255, get_generic(genDITH_PERC));
}
void gamedata::set_dither_perc(byte val)
{
	set_generic(zc_min(255, val), genDITH_PERC);
}

byte gamedata::get_transdark_perc()
{
	return zc_min(255, get_generic(genTDARK_PERC));
}
void gamedata::set_transdark_perc(byte val)
{
	set_generic(zc_min(255, val), genTDARK_PERC);
}

byte gamedata::get_light_rad()
{
	return get_generic(genLIGHT_RAD);
}
void gamedata::set_light_rad(byte val)
{
	set_generic(val, genLIGHT_RAD);
}

byte gamedata::get_darkscr_color()
{
	return get_generic(genDARK_COL);
}
void gamedata::set_darkscr_color(byte val)
{
	set_generic(val, genDARK_COL);
}

int32_t gamedata::get_watergrav()
{
	return get_generic(genWATER_GRAV);
}
void gamedata::set_watergrav(int32_t val)
{
	set_generic(val, genWATER_GRAV);
}
int32_t gamedata::get_sideswim_up()
{
	return get_generic(genSIDESWIM_UP);
}
void gamedata::set_sideswim_up(int32_t val)
{
	set_generic(val, genSIDESWIM_UP);
}

int32_t gamedata::get_sideswim_side()
{
	return get_generic(genSIDESWIM_SIDE);
}
void gamedata::set_sideswim_side(int32_t val)
{
	set_generic(val, genSIDESWIM_SIDE);
}

int32_t gamedata::get_sideswim_down()
{
	return get_generic(genSIDESWIM_DOWN);
}
void gamedata::set_sideswim_down(int32_t val)
{
	set_generic(val, genSIDESWIM_DOWN);
}

int32_t gamedata::get_sideswim_jump()
{
	return get_generic(genSIDESWIM_JUMP);
}
void gamedata::set_sideswim_jump(int32_t val)
{
	set_generic(val, genSIDESWIM_JUMP);
}

int32_t gamedata::get_bunny_ltm()
{
	return get_generic(genBUNNY_LTM);
}
void gamedata::set_bunny_ltm(int32_t val)
{
	set_generic(val, genBUNNY_LTM);
}

byte gamedata::get_switchhookstyle()
{
	return get_generic(genSWITCHSTYLE);
}
void gamedata::set_switchhookstyle(byte val)
{
	set_generic(val, genSWITCHSTYLE);
}

void gamedata::set_item(int32_t id, bool value)
{
    set_item_no_flush(id, value);
    flushItemCache();
}

void gamedata::set_item_no_flush(int32_t id, bool value)
{
    if(!isclearing && !(value == item[id]))
        Z_eventlog("%sed item %i: %s\n", value ? "Gain" : "Remov", id, item_string[id]); //Gain + ed, Remov + ed. %s + ed.
        
    item[id]=value;
}
int32_t gamedata::fillBottle(byte val)
{
	bool temp[256] = {false};
	for(size_t q = 0; q < MAXITEMS; ++q)
	{
		if(get_item(q) && itemsbuf[q].family == itype_bottle)
		{
			size_t bind = itemsbuf[q].misc1;
			if(bind < 256)
			{
				temp[bind] = true;
			}
		}
	}
	for(size_t q = 0; q < 256; ++q)
	{
		if(!temp[q]) continue; //don't own bottle
		if(bottleSlots[q] == 0)
		{
			set_bottle_slot(q, val);
			return q;
		}
	}
	return -1;
}
bool gamedata::canFillBottle()
{
	bool temp[256] = {false};
	for(size_t q = 0; q < MAXITEMS; ++q)
	{
		if(get_item(q) && itemsbuf[q].family == itype_bottle)
		{
			size_t bind = itemsbuf[q].misc1;
			if(bind < 256)
			{
				temp[bind] = true;
			}
		}
	}
	for(size_t q = 0; q < 256; ++q)
	{
		if(!temp[q]) continue; //don't own bottle
		if(bottleSlots[q] == 0)
			return true;
	}
	return false;
}

void gamedata::set_portal(int16_t destdmap, int16_t srcdmap, byte scr, int32_t x, int32_t y, byte sfx, int32_t weffect, int16_t psprite)
{
	portaldestdmap = destdmap;
	portalsrcdmap = srcdmap;
	portalscr = scr;
	portalx = x;
	portaly = y;
	portalwarpfx = weffect;
	portalsfx = sfx;
	portalspr = psprite;
}

void gamedata::load_portal()
{
#ifndef IS_ZQUEST
	if(mirror_portal)
		delete mirror_portal;
	if(currdmap == portalsrcdmap && currscr == portalscr)
	{
		mirror_portal = new portal(portaldestdmap, portalscr+DMaps[portaldestdmap].xoff,
			portalwarpfx, portalsfx, portalspr);
		mirror_portal->x = zslongToFix(portalx);
		mirror_portal->y = zslongToFix(portaly);
	}
	else mirror_portal = NULL;
#endif
}

void gamedata::clear_portal()
{
	portalsrcdmap = -1;
	portaldestdmap = -1;
	portalscr = 0;
	portalx = 0;
	portaly = 0;
	portalsfx = 0;
	portalwarpfx = 0;
	portalspr = 0;
}

bool gamedata::should_show_time()
{
	// Drawing the time makes manually updating replays much more difficult.
	if (replay_is_active() && replay_is_debug())
		return false;

	return get_timevalid() && !did_cheat() && get_bit(quest_rules,qr_TIME);
}

/*** end of gamedata.cpp ***/

