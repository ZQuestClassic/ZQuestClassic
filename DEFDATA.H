//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  defdata.cc
//
//  Default item, weapon, and enemy data for Zelda Classic.
//
//  This data was originally hard coded in the item class,
//  enemy classes, etc. I exctracted it to make generic
//  classes, thus allowing custom items and enemies.
//
//--------------------------------------------------------

#ifndef _DEFDATA_H_
#define _DEFDATA_H_

#include "items.h"
#include "guys.h"
extern itemdata default_items[MAXITEMS];
extern guydata default_guys[eMAXGUYS];
#endif
