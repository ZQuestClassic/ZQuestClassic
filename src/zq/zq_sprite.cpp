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

#include "sprite.h"

int32_t fadeclk=-1;
int32_t frame=8;
bool BSZ=false;
int32_t conveyclk=0;
byte newconveyorclk=0;
bool freeze_guys=false;

void sprite::handle_sprlighting()
{
	return;
}

void sprite::check_conveyor()
{
    return;
}

void movingblock::push(zfix,zfix,int32_t,int32_t)
{
    return;
}

bool movingblock::animate(int32_t)
{
    return false;
}

