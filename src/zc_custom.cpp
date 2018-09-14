//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zc_custom.cc
//
//  Custom item, enemy, etc. for Zelda.
//
//--------------------------------------------------------

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include "zc_custom.h"
#include "zelda.h"
#include "zdefs.h"

//extern itemdata   *itemsbuf;
extern wpndata    *wpnsbuf;

int script_link_sprite = 0;
int script_link_flip = -1;

int old_floatspr, old_slashspr, linkspr;
int walkspr[4][3];                                   //dir,                    tile/flip/extend
int stabspr[4][3];                                   //dir,                    tile/flip/extend
int slashspr[4][3];                                  //dir,                    tile/flip/extend
int floatspr[4][3];                                  //dir,                    tile/flip/extend
int swimspr[4][3];                                   //dir,                    tile/flip/extend
int divespr[4][3];                                   //dir,                    tile/flip/extend
int poundspr[4][3];                                  //dir,                    tile/flip/extend
int jumpspr[4][3];                                   //dir,                    tile/flip/extend
int chargespr[4][3];                                 //dir,                    tile/flip/extend
int castingspr[3];                                   //                        tile/flip/extend
int holdspr[2][2][3];                                //     land/water, hands. tile/flip/extend
//int fallspr[3];                                     //     		       tile/flip/extend

void linktile(int *tile, int *flip, int *extend, int state, int dir, int style)
{
    switch(state)
    {
    case ls_float:
        *extend=floatspr[dir][spr_extend];
        break;
        
    case ls_swim:
        *extend=swimspr[dir][spr_extend];
        break;
        
    case ls_dive:
        *extend=divespr[dir][spr_extend];
        break;
        
    case ls_slash:
        *extend=slashspr[dir][spr_extend];
        break;
        
    case ls_walk:
        *extend=walkspr[dir][spr_extend];
        break;
        
    case ls_jump:
        *extend=jumpspr[dir][spr_extend];
        break;
        
    case ls_charge:
        *extend=chargespr[dir][spr_extend];
        break;
        
    case ls_stab:
        *extend=stabspr[dir][spr_extend];
        break;
        
    case ls_pound:
        *extend=poundspr[dir][spr_extend];
        break;
        
    case ls_cast:
        *extend=castingspr[spr_extend];
        break;
        
    case ls_landhold1:
        *extend=holdspr[spr_landhold][spr_hold1][spr_extend];
        break;
        
    case ls_landhold2:
        *extend=holdspr[spr_landhold][spr_hold2][spr_extend];
        break;
        
    case ls_waterhold1:
        *extend=holdspr[spr_waterhold][spr_hold1][spr_extend];
        break;
        
    case ls_waterhold2:
        *extend=holdspr[spr_waterhold][spr_hold2][spr_extend];
        break;
        
    default:
        *tile=0;
        *flip=0;
        *extend=0;
        return;
    }
    
    linktile(tile, flip, state, dir, style);
}
void setlinktile(int tile, int flip, int extend, int state, int dir)
{
    switch(state)
    {
    case ls_float:
        floatspr[dir][spr_tile] = tile;
        floatspr[dir][spr_flip] = flip;
        floatspr[dir][spr_extend] = extend;
        break;
        
    case ls_swim:
        swimspr[dir][spr_tile] = tile;
        swimspr[dir][spr_flip] = flip;
        swimspr[dir][spr_extend] = extend;
        break;
        
    case ls_dive:
        divespr[dir][spr_tile] = tile;
        divespr[dir][spr_flip] = flip;
        divespr[dir][spr_extend] = extend;
        break;
        
    case ls_slash:
        slashspr[dir][spr_tile] = tile;
        slashspr[dir][spr_flip] = flip;
        slashspr[dir][spr_extend] = extend;
        break;
        
    case ls_walk:
        walkspr[dir][spr_tile] = tile;
        walkspr[dir][spr_flip] = flip;
        walkspr[dir][spr_extend] = extend;
        break;
        
    case ls_stab:
        stabspr[dir][spr_tile] = tile;
        stabspr[dir][spr_flip] = flip;
        stabspr[dir][spr_extend] = extend;
        break;
        
    case ls_pound:
        poundspr[dir][spr_tile] = tile;
        poundspr[dir][spr_flip] = flip;
        poundspr[dir][spr_extend] = extend;
        break;
        
    case ls_jump:
        jumpspr[dir][spr_tile] = tile;
        jumpspr[dir][spr_flip] = flip;
        jumpspr[dir][spr_extend] = extend;
        break;
        
    case ls_charge:
        chargespr[dir][spr_tile] = tile;
        chargespr[dir][spr_flip] = flip;
        chargespr[dir][spr_extend] = extend;
        break;
        
    case ls_cast:
        castingspr[spr_tile] = tile;
        castingspr[spr_flip] = flip;
        castingspr[spr_extend] = extend;
        break;
        
    case ls_landhold1:
        holdspr[spr_landhold][spr_hold1][spr_tile] = tile;
        holdspr[spr_landhold][spr_hold1][spr_flip] = flip;
        holdspr[spr_landhold][spr_hold1][spr_extend] = extend;
        break;
        
    case ls_landhold2:
        holdspr[spr_landhold][spr_hold2][spr_tile] = tile;
        holdspr[spr_landhold][spr_hold2][spr_flip] = flip;
        holdspr[spr_landhold][spr_hold2][spr_extend] = extend;
        break;
        
    case ls_waterhold1:
        holdspr[spr_waterhold][spr_hold1][spr_tile] = tile;
        holdspr[spr_waterhold][spr_hold1][spr_flip] = flip;
        holdspr[spr_waterhold][spr_hold1][spr_extend] = extend;
        break;
        
    case ls_waterhold2:
        holdspr[spr_waterhold][spr_hold2][spr_tile] = tile;
        holdspr[spr_waterhold][spr_hold2][spr_flip] = flip;
        holdspr[spr_waterhold][spr_hold2][spr_extend] = extend;
        break;
        
    default:
        break;
    }
}

void linktile(int *tile, int *flip, int state, int dir, int)
{
	if ( script_link_sprite > 0 ) *tile = script_link_sprite;
	else
	{
	    switch(state)
	    {
	    case ls_float:
		*tile=floatspr[dir][spr_tile];
		break;
		
	    case ls_swim:
		*tile=swimspr[dir][spr_tile];
		break;
		
	    case ls_dive:
		*tile=divespr[dir][spr_tile];
		break;
		
	    case ls_slash:
		*tile=slashspr[dir][spr_tile];
		break;
		
	    case ls_walk:
		*tile=walkspr[dir][spr_tile];
		break;
		
	    case ls_stab:
		*tile=stabspr[dir][spr_tile];
		break;
		
	    case ls_pound:
		*tile=poundspr[dir][spr_tile];
		break;
		
	    case ls_jump:
		*tile=jumpspr[dir][spr_tile];
		break;
		
	    case ls_charge:
		*tile=chargespr[dir][spr_tile];
		break;
		
	    case ls_cast:
		*tile=castingspr[spr_tile];
		break;
		
	    case ls_landhold1:
		*tile=holdspr[spr_landhold][spr_hold1][spr_tile];
		break;
		
	    case ls_landhold2:
		*tile=holdspr[spr_landhold][spr_hold2][spr_tile];
		break;
		
	    case ls_waterhold1:
		*tile=holdspr[spr_waterhold][spr_hold1][spr_tile];
		break;
		
	    case ls_waterhold2:
		*tile=holdspr[spr_waterhold][spr_hold2][spr_tile];
		break;
		
	    default:
		*tile=0;
		break;
	    }
	}
	if ( script_link_flip > -1 ) *flip = script_link_flip;
	else
	{
	    switch(state)
	    {
	    case ls_float:
		*flip=floatspr[dir][spr_flip];
		break;
		
	    case ls_swim:
		*flip=swimspr[dir][spr_flip];
		break;
		
	    case ls_dive:
		*flip=divespr[dir][spr_flip];
		break;
		
	    case ls_slash:
		*flip=slashspr[dir][spr_flip];
		break;
		
	    case ls_walk:
		*flip=walkspr[dir][spr_flip];
		break;
		
	    case ls_stab:
		*flip=stabspr[dir][spr_flip];
		break;
		
	    case ls_pound:
		*flip=poundspr[dir][spr_flip];
		break;
		
	    case ls_jump:
		*flip=jumpspr[dir][spr_flip];
		break;
		
	    case ls_charge:
		*flip=chargespr[dir][spr_flip];
		break;
		
	    case ls_cast:
		*flip=castingspr[spr_flip];
		break;
		
	    case ls_landhold1:
		*flip=holdspr[spr_landhold][spr_hold1][spr_flip];
		break;
		
	    case ls_landhold2:
		*flip=holdspr[spr_landhold][spr_hold2][spr_flip];
		break;
		
	    case ls_waterhold1:
		*flip=holdspr[spr_waterhold][spr_hold1][spr_flip];
		break;
		
	    case ls_waterhold2:
		*flip=holdspr[spr_waterhold][spr_hold2][spr_flip];
		break;
		
	    default:
		*flip=0;
		break;
	    }
	}
}

void setuplinktiles(int style)
{
    old_floatspr = wpnsbuf[iwSwim].newtile;
    old_slashspr = wpnsbuf[iwLinkSlash].newtile;
    linkspr = 4;
    
    switch(style)
    {
    case 0:                                                 //normal
        walkspr[up][spr_tile]=linkspr+20;
        walkspr[up][spr_flip]=0;
        walkspr[up][spr_extend]=0;
        walkspr[down][spr_tile]=linkspr+18;
        walkspr[down][spr_flip]=0;
        walkspr[down][spr_extend]=0;
        walkspr[left][spr_tile]=linkspr+16;
        walkspr[left][spr_flip]=1;
        walkspr[left][spr_extend]=0;
        walkspr[right][spr_tile]=linkspr+16;
        walkspr[right][spr_flip]=0;
        walkspr[right][spr_extend]=0;
        
        stabspr[up][spr_tile]=linkspr+23;
        stabspr[up][spr_flip]=0;
        stabspr[up][spr_extend]=0;
        stabspr[down][spr_tile]=linkspr+22;
        stabspr[down][spr_flip]=0;
        stabspr[down][spr_extend]=0;
        stabspr[left][spr_tile]=linkspr+21;
        stabspr[left][spr_flip]=1;
        stabspr[left][spr_extend]=0;
        stabspr[right][spr_tile]=linkspr+21;
        stabspr[right][spr_flip]=0;
        stabspr[right][spr_extend]=0;
        
        slashspr[up][spr_tile]=old_slashspr;
        slashspr[up][spr_flip]=0;
        slashspr[up][spr_extend]=0;
        slashspr[down][spr_tile]=old_slashspr+1;
        slashspr[down][spr_flip]=0;
        slashspr[down][spr_extend]=0;
        slashspr[left][spr_tile]=old_slashspr+2;
        slashspr[left][spr_flip]=0;
        slashspr[left][spr_extend]=0;
        slashspr[right][spr_tile]=old_slashspr+3;
        slashspr[right][spr_flip]=0;
        slashspr[right][spr_extend]=0;
        
        floatspr[up][spr_tile]=old_floatspr + 4;
        floatspr[up][spr_flip]=0;
        floatspr[up][spr_extend]=0;
        floatspr[down][spr_tile]=old_floatspr + 2;
        floatspr[down][spr_flip]=0;
        floatspr[down][spr_extend]=0;
        floatspr[left][spr_tile]=old_floatspr;
        floatspr[left][spr_flip]=1;
        floatspr[left][spr_extend]=0;
        floatspr[right][spr_tile]=old_floatspr;
        floatspr[right][spr_flip]=0;
        floatspr[right][spr_extend]=0;
        
        swimspr[up][spr_tile]=old_floatspr + 4;
        swimspr[up][spr_flip]=0;
        swimspr[up][spr_extend]=0;
        swimspr[down][spr_tile]=old_floatspr + 2;
        swimspr[down][spr_flip]=0;
        swimspr[down][spr_extend]=0;
        swimspr[left][spr_tile]=old_floatspr;
        swimspr[left][spr_flip]=1;
        swimspr[left][spr_extend]=0;
        swimspr[right][spr_tile]=old_floatspr;
        swimspr[right][spr_flip]=0;
        swimspr[right][spr_extend]=0;
        
        divespr[up][spr_tile]=old_floatspr + 8;
        divespr[up][spr_flip]=0;
        divespr[up][spr_extend]=0;
        divespr[down][spr_tile]=old_floatspr + 8;
        divespr[down][spr_flip]=0;
        divespr[down][spr_extend]=0;
        divespr[left][spr_tile]=old_floatspr + 8;
        divespr[left][spr_flip]=0;
        divespr[left][spr_extend]=0;
        divespr[right][spr_tile]=old_floatspr + 8;
        divespr[right][spr_flip]=0;
        divespr[right][spr_extend]=0;
        
        poundspr[up][spr_tile]=linkspr+3;
        poundspr[up][spr_flip]=0;
        poundspr[up][spr_extend]=0;
        poundspr[down][spr_tile]=linkspr+2;
        poundspr[down][spr_flip]=0;
        poundspr[down][spr_extend]=0;
        poundspr[left][spr_tile]=linkspr+1;
        poundspr[left][spr_flip]=1;
        poundspr[left][spr_extend]=0;
        poundspr[right][spr_tile]=linkspr+1;
        poundspr[right][spr_flip]=0;
        poundspr[right][spr_extend]=0;
        
// Alter these when default sprites are created.
        jumpspr[up][spr_tile]=linkspr+24;
        jumpspr[up][spr_flip]=0;
        jumpspr[up][spr_extend]=0;
        jumpspr[down][spr_tile]=linkspr+19;
        jumpspr[down][spr_flip]=0;
        jumpspr[down][spr_extend]=0;
        jumpspr[left][spr_tile]=linkspr+16;
        jumpspr[left][spr_flip]=1;
        jumpspr[left][spr_extend]=0;
        jumpspr[right][spr_tile]=linkspr+16;
        jumpspr[right][spr_flip]=0;
        jumpspr[right][spr_extend]=0;
        
// Alter these when default sprites are created.
        chargespr[up][spr_tile]=linkspr+24;
        chargespr[up][spr_flip]=0;
        chargespr[up][spr_extend]=0;
        chargespr[down][spr_tile]=linkspr+19;
        chargespr[down][spr_flip]=0;
        chargespr[down][spr_extend]=0;
        chargespr[left][spr_tile]=linkspr+16;
        chargespr[left][spr_flip]=1;
        chargespr[left][spr_extend]=0;
        chargespr[right][spr_tile]=linkspr+16;
        chargespr[right][spr_flip]=0;
        chargespr[right][spr_extend]=0;
        
        castingspr[spr_tile]=linkspr;
        castingspr[spr_flip]=0;
        castingspr[spr_extend]=0;
        
        holdspr[spr_landhold][spr_hold1][spr_tile]=linkspr+30;
        holdspr[spr_landhold][spr_hold1][spr_flip]=0;
        holdspr[spr_landhold][spr_hold1][spr_extend]=0;
        holdspr[spr_landhold][spr_hold2][spr_tile]=linkspr+25;
        holdspr[spr_landhold][spr_hold2][spr_flip]=0;
        holdspr[spr_landhold][spr_hold2][spr_extend]=0;
        
        holdspr[spr_waterhold][spr_hold1][spr_tile]=old_floatspr+7;
        holdspr[spr_waterhold][spr_hold1][spr_flip]=0;
        holdspr[spr_waterhold][spr_hold1][spr_extend]=0;
        holdspr[spr_waterhold][spr_hold2][spr_tile]=old_floatspr+6;
        holdspr[spr_waterhold][spr_hold2][spr_flip]=0;
        holdspr[spr_waterhold][spr_hold2][spr_extend]=0;
        break;
        
    case 1:                                                 //BS
        walkspr[up][spr_tile]=linkspr+24;
        walkspr[up][spr_flip]=0;
        walkspr[up][spr_extend]=0;
        walkspr[down][spr_tile]=linkspr+19;
        walkspr[down][spr_flip]=0;
        walkspr[down][spr_extend]=0;
        walkspr[left][spr_tile]=linkspr+16;
        walkspr[left][spr_flip]=1;
        walkspr[left][spr_extend]=0;
        walkspr[right][spr_tile]=linkspr+16;
        walkspr[right][spr_flip]=0;
        walkspr[right][spr_extend]=0;
        
        stabspr[up][spr_tile]=linkspr+27;
        stabspr[up][spr_flip]=0;
        stabspr[up][spr_extend]=0;
        stabspr[down][spr_tile]=linkspr+23;
        stabspr[down][spr_flip]=0;
        stabspr[down][spr_extend]=0;
        stabspr[left][spr_tile]=linkspr+22;
        stabspr[left][spr_flip]=1;
        stabspr[left][spr_extend]=0;
        stabspr[right][spr_tile]=linkspr+22;
        stabspr[right][spr_flip]=0;
        stabspr[right][spr_extend]=0;
        
        slashspr[up][spr_tile]=old_slashspr;
        slashspr[up][spr_flip]=0;
        slashspr[up][spr_extend]=0;
        slashspr[down][spr_tile]=old_slashspr+1;
        slashspr[down][spr_flip]=0;
        slashspr[down][spr_extend]=0;
        slashspr[left][spr_tile]=old_slashspr+2;
        slashspr[left][spr_flip]=0;
        slashspr[left][spr_extend]=0;
        slashspr[right][spr_tile]=old_slashspr+3;
        slashspr[right][spr_flip]=0;
        slashspr[right][spr_extend]=0;
        
        floatspr[up][spr_tile]=old_floatspr+6;
        floatspr[up][spr_flip]=0;
        floatspr[up][spr_extend]=0;
        floatspr[down][spr_tile]=old_floatspr+3;
        floatspr[down][spr_flip]=0;
        floatspr[down][spr_extend]=0;
        floatspr[left][spr_tile]=old_floatspr;
        floatspr[left][spr_flip]=1;
        floatspr[left][spr_extend]=0;
        floatspr[right][spr_tile]=old_floatspr;
        floatspr[right][spr_flip]=0;
        floatspr[right][spr_extend]=0;
        
        swimspr[up][spr_tile]=old_floatspr+6;
        swimspr[up][spr_flip]=0;
        swimspr[up][spr_extend]=0;
        swimspr[down][spr_tile]=old_floatspr+3;
        swimspr[down][spr_flip]=0;
        swimspr[down][spr_extend]=0;
        swimspr[left][spr_tile]=old_floatspr;
        swimspr[left][spr_flip]=1;
        swimspr[left][spr_extend]=0;
        swimspr[right][spr_tile]=old_floatspr;
        swimspr[right][spr_flip]=0;
        swimspr[right][spr_extend]=0;
        
        divespr[up][spr_tile]=old_floatspr + 11;
        divespr[up][spr_flip]=0;
        divespr[up][spr_extend]=0;
        divespr[down][spr_tile]=old_floatspr + 11;
        divespr[down][spr_flip]=0;
        divespr[down][spr_extend]=0;
        divespr[left][spr_tile]=old_floatspr + 11;
        divespr[left][spr_flip]=0;
        divespr[left][spr_extend]=0;
        divespr[right][spr_tile]=old_floatspr + 11;
        divespr[right][spr_flip]=0;
        divespr[right][spr_extend]=0;
        
        poundspr[up][spr_tile]=linkspr+3;
        poundspr[up][spr_flip]=0;
        poundspr[up][spr_extend]=0;
        poundspr[down][spr_tile]=linkspr+2;
        poundspr[down][spr_flip]=0;
        poundspr[down][spr_extend]=0;
        poundspr[left][spr_tile]=linkspr+1;
        poundspr[left][spr_flip]=1;
        poundspr[left][spr_extend]=0;
        poundspr[right][spr_tile]=linkspr+1;
        poundspr[right][spr_flip]=0;
        poundspr[right][spr_extend]=0;
        
// Alter these when default sprites are created.
        jumpspr[up][spr_tile]=linkspr+24;
        jumpspr[up][spr_flip]=0;
        jumpspr[up][spr_extend]=0;
        jumpspr[down][spr_tile]=linkspr+19;
        jumpspr[down][spr_flip]=0;
        jumpspr[down][spr_extend]=0;
        jumpspr[left][spr_tile]=linkspr+16;
        jumpspr[left][spr_flip]=1;
        jumpspr[left][spr_extend]=0;
        jumpspr[right][spr_tile]=linkspr+16;
        jumpspr[right][spr_flip]=0;
        jumpspr[right][spr_extend]=0;
        
// Alter these when default sprites are created.
        chargespr[up][spr_tile]=linkspr+24;
        chargespr[up][spr_flip]=0;
        chargespr[up][spr_extend]=0;
        chargespr[down][spr_tile]=linkspr+19;
        chargespr[down][spr_flip]=0;
        chargespr[down][spr_extend]=0;
        chargespr[left][spr_tile]=linkspr+16;
        chargespr[left][spr_flip]=1;
        chargespr[left][spr_extend]=0;
        chargespr[right][spr_tile]=linkspr+16;
        chargespr[right][spr_flip]=0;
        chargespr[right][spr_extend]=0;
        
        castingspr[spr_tile]=linkspr;
        castingspr[spr_flip]=0;
        castingspr[spr_extend]=0;
        
        holdspr[spr_landhold][spr_hold1][spr_tile]=linkspr+29;
        holdspr[spr_landhold][spr_hold1][spr_flip]=0;
        holdspr[spr_landhold][spr_hold1][spr_extend]=0;
        holdspr[spr_landhold][spr_hold2][spr_tile]=linkspr+28;
        holdspr[spr_landhold][spr_hold2][spr_flip]=0;
        holdspr[spr_landhold][spr_hold2][spr_extend]=0;
        
        holdspr[spr_waterhold][spr_hold1][spr_tile]=old_floatspr+10;
        holdspr[spr_waterhold][spr_hold1][spr_flip]=0;
        holdspr[spr_waterhold][spr_hold1][spr_extend]=0;
        holdspr[spr_waterhold][spr_hold2][spr_tile]=old_floatspr+9;
        holdspr[spr_waterhold][spr_hold2][spr_flip]=0;
        holdspr[spr_waterhold][spr_hold2][spr_extend]=0;
        break;
        
    default:
        break;
    }
}
