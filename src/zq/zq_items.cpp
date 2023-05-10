//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  sprite.cc
//
//  Sprite classes:
//   - sprite:      base class for the guys and enemies in zelda.cc
//   - movingblock: the moving block class
//   - sprite_list: main container class for different groups of sprites
//   - item:        items class
//
//--------------------------------------------------------

/**********************************/
/**********  Item Class  **********/
/**********************************/

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include "zq_class.h"
#include "base/zdefs.h"
#include "items.h"

void movefairy(zfix&,zfix&,int32_t)
{
    return;
}

void killfairy(int32_t)
{
    return;
}

void killfairynew(item const&)
{
    return;
}

void movefairynew(zfix&, zfix&, item const&)
{
    return;
}

bool addfairy(zfix, zfix, int32_t, int32_t)
{
    return false;
}

bool addfairynew(zfix x, zfix y, int32_t misc3, item&)
{
	return false;
}

bool can_drop(zfix, zfix)
{
    return false;
}

void item_fall(zfix&, zfix&, zfix&)
{
	return;
}

int32_t item_pits(zfix&, zfix&, int32_t&)
{
	return 0;
}

int32_t select_dropitem(int32_t)
{
    return 0;
}

int32_t select_dropitem(int32_t, int32_t, int32_t)
{
    return 0;
}

void getitem(int32_t, bool, bool)
{
    return;
}

int32_t item::run_script(int32_t)
{
	return RUNSCRIPT_OK;
}

std::string bottle_name(size_t type)
{
	return "Empty";
}
std::string bottle_slot_name(size_t slot, std::string const& emptystr)
{
	return emptystr;
}

/*** end of sprite.cc ***/


