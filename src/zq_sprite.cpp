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

#include "precompiled.h" //always first

#include "sprite.h"

int32_t fadeclk=-1;
int32_t frame=8;
bool BSZ=false;
int32_t conveyclk=0;
bool freeze_guys=false;

void sprite::handle_sprlighting()
{
	return;
}

void sprite::check_conveyor()
{
    return;
}

void movingblock::push(zfix bx,zfix by,int32_t d2,int32_t f)
{
    //these are here to bypass compiler warnings about unused arguments
    bx=bx;
    by=by;
    d2=d2;
    f=f;
    
    return;
}

bool movingblock::animate(int32_t index)
{
    //these are here to bypass compiler warnings about unused arguments
    index=index;
    
    return false;
}

