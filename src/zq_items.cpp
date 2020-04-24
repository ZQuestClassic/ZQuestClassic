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
#include "zdefs.h"
#include "items.h"

void movefairy(zfix&,zfix&,int)
{
    return;
}

void killfairy(int)
{
    return;
}

bool addfairy(zfix, zfix, int, int)
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

int select_dropitem(int)
{
    return 0;
}

int select_dropitem(int, int, int)
{
    return 0;
}

void getitem(int, bool)
{
    return;
}

int item::run_script(int)
{
	return RUNSCRIPT_OK;
}
/*** end of sprite.cc ***/


