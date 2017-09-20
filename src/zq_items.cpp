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
#include "weapons.h"

void movefairy(fix&,fix&,int)
{
    return;
}

void killfairy(int)
{
    return;
}

bool addfairy(fix, fix, int, int)
{
    return false;
}

bool can_drop(fix, fix)
{
    return false;
}

ItemDefinitionRef select_dropitem(int, int, int)
{
    return ItemDefinitionRef(); //?
}

bool is_side_view()
{
    return (Map.CurrScr()->flags7&fSIDEVIEW) != 0;
}

void getitem(const ItemDefinitionRef &itemref, bool)
{
    return;
}

extern sprite_list Lwpns;

int LwpnsIdCount(int id)
{
    int ret = 0;
    for (int i = 0; i < Lwpns.Count(); i++)
    {
        if (((weapon *)Lwpns.spr(i))->id == id)
            ret++;
    }
    return ret;
}

