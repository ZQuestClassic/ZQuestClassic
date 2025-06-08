#include "hero_tiles.h"
#include "base/zdefs.h"

extern wpndata    *wpnsbuf;
int32_t hero_animation_speed = 1;
int32_t liftspeed = 4;

int32_t script_hero_sprite = 0;
int32_t script_hero_flip = -1;
int32_t script_hero_cset = -1;

byte hero_defenses[wMax];

int32_t hammeroffsets[4];

int32_t old_floatspr, old_slashspr, herospr;
int32_t walkspr[4][3];                                   //dir,                           tile/flip/extend
int32_t stabspr[4][3];                                   //dir,                           tile/flip/extend
int32_t slashspr[4][3];                                  //dir,                           tile/flip/extend
int32_t revslashspr[4][3];                               //dir,                           tile/flip/extend
int32_t floatspr[4][3];                                  //dir,                           tile/flip/extend
int32_t swimspr[4][3];                                   //dir,                           tile/flip/extend
int32_t divespr[4][3];                                   //dir,                           tile/flip/extend
int32_t poundspr[4][3];                                  //dir,                           tile/flip/extend
int32_t jumpspr[4][3];                                   //dir,                           tile/flip/extend
int32_t chargespr[4][3];                                 //dir,                           tile/flip/extend
int32_t castingspr[3];                                   //                               tile/flip/extend
int32_t sideswimcastingspr[3];                           //                               tile/flip/extend
int32_t holdspr[2][3][3];                                //land/water, 1hand/2hand/sword, tile/flip/extend
int32_t frozenspr[4][3];                                 //dir,                           tile/flip/extend
int32_t frozen_waterspr[4][3];                           //dir,                           tile/flip/extend
int32_t onfirespr[4][3];                                 //dir,                           tile/flip/extend
int32_t onfire_waterspr[4][3];                           //dir,                           tile/flip/extend
int32_t diggingspr[4][3];                                //dir,                           tile/flip/extend
int32_t usingrodspr[4][3];                               //dir,                           tile/flip/extend
int32_t usingcanespr[4][3];                              //dir,                           tile/flip/extend
int32_t pushingspr[4][3];                                //dir,                           tile/flip/extend
int32_t liftingspr[4][4];                                //dir,                           tile/flip/extend/frames
int32_t liftingwalkspr[4][3];                            //dir,                           tile/flip/extend
int32_t stunnedspr[4][3];                                //dir,                           tile/flip/extend
int32_t stunned_waterspr[4][3];                          //dir,                           tile/flip/extend
int32_t drowningspr[4][3];                               //dir,                           tile/flip/extend
int32_t sidedrowningspr[4][3];                           //dir,                           tile/flip/extend
int32_t drowning_lavaspr[4][3];                          //dir,                           tile/flip/extend
int32_t sideswimspr[4][3];                               //dir,                           tile/flip/extend
int32_t sideswimslashspr[4][3];                          //dir,                           tile/flip/extend
int32_t sideswimstabspr[4][3];                           //dir,                           tile/flip/extend
int32_t sideswimpoundspr[4][3];                          //dir,                           tile/flip/extend
int32_t sideswimchargespr[4][3];                         //dir,                           tile/flip/extend
int32_t sideswimholdspr[3][3];                           //dir,                           tile/flip/extend
int32_t fallingspr[4][3];                                //dir,                           tile/flip/extend
int32_t shockedspr[4][3];                                //dir,                           tile/flip/extend
int32_t shocked_waterspr[4][3];                          //dir,                           tile/flip/extend
int32_t pullswordspr[4][3];                              //dir,                           tile/flip/extend
int32_t readingspr[4][3];                                //dir,                           tile/flip/extend
int32_t slash180spr[4][3];                               //dir,                           tile/flip/extend
int32_t slashZ4spr[4][3];                                //dir,                           tile/flip/extend
int32_t dashspr[4][3];                                   //dir,                           tile/flip/extend
int32_t bonkspr[4][3];                                   //dir,                           tile/flip/extend
int32_t medallionsprs[3][3];                             //medallion,                     tile/flip/extend

void herotile(int32_t *tile, int32_t *flip, int32_t *extend, int32_t state, int32_t dir, int32_t style)
{
	if (script_hero_sprite > 0)
	{
		herotile(tile, flip, state, dir, style);
		return;
	}

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
		
		case ls_drown:
			*extend=drowningspr[dir][spr_extend];
			break;
		
		case ls_sidedrown:
			*extend=sidedrowningspr[dir][spr_extend];
			break;
		
		case ls_lavadrown:
			*extend=drowning_lavaspr[dir][spr_extend];
			break;
		
		case ls_falling:
			*extend=fallingspr[dir][spr_extend];
			break;
			
		case ls_slash:
			*extend=slashspr[dir][spr_extend];
			break;
			
		case ls_revslash:
			*extend=revslashspr[dir][spr_extend];
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
		
		case ls_sideswimcast:
			*extend=sideswimcastingspr[spr_extend];
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
		
		case ls_sideswim:
			*extend=sideswimspr[dir][spr_extend];
			break;
		
		case ls_sideswimslash:
			*extend=sideswimslashspr[dir][spr_extend];
			break;
			
		case ls_sideswimstab:
			*extend=sideswimstabspr[dir][spr_extend];
			break;
			
		case ls_sideswimpound:
			*extend=sideswimpoundspr[dir][spr_extend];
			break;
		
		case ls_sideswimcharge:
			*extend=sideswimchargespr[dir][spr_extend];
			break;
		
		case ls_sidewaterhold1:
			*extend=sideswimholdspr[spr_hold1][spr_extend];
			break;
		
		case ls_sidewaterhold2:
			*extend=sideswimholdspr[spr_hold2][spr_extend];
			break;
		
		case ls_lifting:
			*extend=liftingspr[dir][spr_extend];
			break;
		
		case ls_liftwalk:
			*extend=liftingwalkspr[dir][spr_extend];
			break;
			
		default:
			*tile=0;
			*flip=0;
			*extend=0;
			return;
	}
	
	herotile(tile, flip, state, dir, style);
}
void setherotile(int32_t tile, int32_t flip, int32_t extend, int32_t state, int32_t dir)
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
			
		case ls_drown:
			drowningspr[dir][spr_tile] = tile;
			drowningspr[dir][spr_flip] = flip;
			drowningspr[dir][spr_extend] = extend;
			break;
		
		case ls_sidedrown:
			sidedrowningspr[dir][spr_tile] = tile;
			sidedrowningspr[dir][spr_flip] = flip;
			sidedrowningspr[dir][spr_extend] = extend;
			break;
		
		case ls_lavadrown:
			drowning_lavaspr[dir][spr_tile] = tile;
			drowning_lavaspr[dir][spr_flip] = flip;
			drowning_lavaspr[dir][spr_extend] = extend;
			break;
			
		case ls_falling:
			fallingspr[dir][spr_tile] = tile;
			fallingspr[dir][spr_flip] = flip;
			fallingspr[dir][spr_extend] = extend;
			break;
			
		case ls_slash:
			slashspr[dir][spr_tile] = tile;
			slashspr[dir][spr_flip] = flip;
			slashspr[dir][spr_extend] = extend;
			break;
		
		case ls_revslash:
			revslashspr[dir][spr_tile] = tile;
			revslashspr[dir][spr_flip] = flip;
			revslashspr[dir][spr_extend] = extend;
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
		
		case ls_sideswimcast:
			sideswimcastingspr[spr_tile] = tile;
			sideswimcastingspr[spr_flip] = flip;
			sideswimcastingspr[spr_extend] = extend;
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
		
		case ls_sidewaterhold1:
			sideswimholdspr[spr_hold1][spr_tile] = tile;
			sideswimholdspr[spr_hold1][spr_flip] = flip;
			sideswimholdspr[spr_hold1][spr_extend] = extend;
			break;
			
		case ls_sidewaterhold2:
			sideswimholdspr[spr_hold2][spr_tile] = tile;
			sideswimholdspr[spr_hold2][spr_flip] = flip;
			sideswimholdspr[spr_hold2][spr_extend] = extend;
			break;
		
		case ls_sideswim:
			sideswimspr[dir][spr_tile] = tile;
			sideswimspr[dir][spr_flip] = flip;
			sideswimspr[dir][spr_extend] = extend;
			break;
		
		case ls_sideswimslash:
			sideswimslashspr[dir][spr_tile] = tile;
			sideswimslashspr[dir][spr_flip] = flip;
			sideswimslashspr[dir][spr_extend] = extend;
			break;
		
		case ls_sideswimstab:
			sideswimstabspr[dir][spr_tile] = tile;
			sideswimstabspr[dir][spr_flip] = flip;
			sideswimstabspr[dir][spr_extend] = extend;
			break;
		
		case ls_sideswimpound:
			sideswimpoundspr[dir][spr_tile] = tile;
			sideswimpoundspr[dir][spr_flip] = flip;
			sideswimpoundspr[dir][spr_extend] = extend;
			break;
			
		case ls_sideswimcharge:
			sideswimchargespr[dir][spr_tile] = tile;
			sideswimchargespr[dir][spr_flip] = flip;
			sideswimchargespr[dir][spr_extend] = extend;
			break;
			
		case ls_lifting:
			liftingspr[dir][spr_tile] = tile;
			liftingspr[dir][spr_flip] = flip;
			liftingspr[dir][spr_extend] = extend;
			break;
			
		case ls_liftwalk:
			liftingwalkspr[dir][spr_tile] = tile;
			liftingwalkspr[dir][spr_flip] = flip;
			liftingwalkspr[dir][spr_extend] = extend;
			break;
			
		default:
			break;
	}
}

void herotile(int32_t *tile, int32_t *flip, int32_t state, int32_t dir, int32_t)
{
	if ( script_hero_sprite > 0 ) *tile = script_hero_sprite;
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
			
			case ls_drown:
				*tile=drowningspr[dir][spr_tile];
				break;
			
			case ls_sidedrown:
				*tile=sidedrowningspr[dir][spr_tile];
				break;
			
			case ls_lavadrown:
				*tile=drowning_lavaspr[dir][spr_tile];
				break;
			
			case ls_falling:
				*tile=fallingspr[dir][spr_tile];
				break;
			
			case ls_slash:
				*tile=slashspr[dir][spr_tile];
				break;
			
			case ls_revslash:
				*tile=revslashspr[dir][spr_tile];
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
			
			case ls_sideswimcast:
				*tile=sideswimcastingspr[spr_tile];
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
			
			case ls_sidewaterhold1:
				*tile=sideswimholdspr[spr_hold1][spr_tile];
				break;
			
			case ls_sidewaterhold2:
				*tile=sideswimholdspr[spr_hold2][spr_tile];
				break;

			case ls_sideswim:
				*tile=sideswimspr[dir][spr_tile];
				break;
			
			case ls_sideswimslash:
				*tile=sideswimslashspr[dir][spr_tile];
				break;
			
			case ls_sideswimstab:
				*tile=sideswimstabspr[dir][spr_tile];
				break;
			
			case ls_sideswimpound:
				*tile=sideswimpoundspr[dir][spr_tile];
				break;
			
			case ls_sideswimcharge:
				*tile=sideswimchargespr[dir][spr_tile];
				break;
			
			case ls_lifting:
				*tile=liftingspr[dir][spr_tile];
				break;
			
			case ls_liftwalk:
				*tile=liftingwalkspr[dir][spr_tile];
				break;
			
			default:
				*tile=0;
				break;
		}
	}
	if ( script_hero_flip > -1 ) *flip = script_hero_flip;
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
			
			case ls_drown:
				*flip=drowningspr[dir][spr_flip];
				break;
			
			case ls_sidedrown:
				*flip=sidedrowningspr[dir][spr_flip];
				break;
			
			case ls_lavadrown:
				*flip=drowning_lavaspr[dir][spr_flip];
				break;
			
			case ls_falling:
				*flip=fallingspr[dir][spr_flip];
				break;
			
			case ls_slash:
				*flip=slashspr[dir][spr_flip];
				break;
			
			case ls_revslash:
				*flip=revslashspr[dir][spr_flip];
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
			
			case ls_sideswimcast:
				*flip=sideswimcastingspr[spr_flip];
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
			
			case ls_sidewaterhold1:
				*flip=sideswimholdspr[spr_hold1][spr_flip];
				break;
			
			case ls_sidewaterhold2:
				*flip=sideswimholdspr[spr_hold2][spr_flip];
				break;
			
			case ls_sideswim:
				*flip=sideswimspr[dir][spr_flip];
				break;
			
			case ls_sideswimslash:
				*flip=sideswimslashspr[dir][spr_flip];
				break;
			
			case ls_sideswimstab:
				*flip=sideswimstabspr[dir][spr_flip];
				break;
			
			case ls_sideswimpound:
				*flip=sideswimpoundspr[dir][spr_flip];
				break;
			
			case ls_sideswimcharge:
				*flip=sideswimchargespr[dir][spr_flip];
				break;
			case ls_lifting:
				*flip=liftingspr[dir][spr_flip];
				break;
			case ls_liftwalk:
				*flip=liftingwalkspr[dir][spr_flip];
				break;
			
			default:
				*flip=0;
				break;
		}
	}
}

void setupherotiles(int32_t style)
{
    old_floatspr = wpnsbuf[iwSwim].tile;
    old_slashspr = wpnsbuf[iwHeroSlash].tile;
    herospr = 4;
    
    switch(style)
    {
    case 0:                                                 //normal
        walkspr[up][spr_tile]=herospr+20;
        walkspr[up][spr_flip]=0;
        walkspr[up][spr_extend]=0;
        walkspr[down][spr_tile]=herospr+18;
        walkspr[down][spr_flip]=0;
        walkspr[down][spr_extend]=0;
        walkspr[left][spr_tile]=herospr+16;
        walkspr[left][spr_flip]=1;
        walkspr[left][spr_extend]=0;
        walkspr[right][spr_tile]=herospr+16;
        walkspr[right][spr_flip]=0;
        walkspr[right][spr_extend]=0;
        
        stabspr[up][spr_tile]=herospr+23;
        stabspr[up][spr_flip]=0;
        stabspr[up][spr_extend]=0;
        stabspr[down][spr_tile]=herospr+22;
        stabspr[down][spr_flip]=0;
        stabspr[down][spr_extend]=0;
        stabspr[left][spr_tile]=herospr+21;
        stabspr[left][spr_flip]=1;
        stabspr[left][spr_extend]=0;
        stabspr[right][spr_tile]=herospr+21;
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
        
        poundspr[up][spr_tile]=herospr+3;
        poundspr[up][spr_flip]=0;
        poundspr[up][spr_extend]=0;
        poundspr[down][spr_tile]=herospr+2;
        poundspr[down][spr_flip]=0;
        poundspr[down][spr_extend]=0;
        poundspr[left][spr_tile]=herospr+1;
        poundspr[left][spr_flip]=1;
        poundspr[left][spr_extend]=0;
        poundspr[right][spr_tile]=herospr+1;
        poundspr[right][spr_flip]=0;
        poundspr[right][spr_extend]=0;
        
// Alter these when default sprites are created.
        jumpspr[up][spr_tile]=herospr+24;
        jumpspr[up][spr_flip]=0;
        jumpspr[up][spr_extend]=0;
        jumpspr[down][spr_tile]=herospr+19;
        jumpspr[down][spr_flip]=0;
        jumpspr[down][spr_extend]=0;
        jumpspr[left][spr_tile]=herospr+16;
        jumpspr[left][spr_flip]=1;
        jumpspr[left][spr_extend]=0;
        jumpspr[right][spr_tile]=herospr+16;
        jumpspr[right][spr_flip]=0;
        jumpspr[right][spr_extend]=0;
        
// Alter these when default sprites are created.
        chargespr[up][spr_tile]=herospr+24;
        chargespr[up][spr_flip]=0;
        chargespr[up][spr_extend]=0;
        chargespr[down][spr_tile]=herospr+19;
        chargespr[down][spr_flip]=0;
        chargespr[down][spr_extend]=0;
        chargespr[left][spr_tile]=herospr+16;
        chargespr[left][spr_flip]=1;
        chargespr[left][spr_extend]=0;
        chargespr[right][spr_tile]=herospr+16;
        chargespr[right][spr_flip]=0;
        chargespr[right][spr_extend]=0;
        
        castingspr[spr_tile]=herospr;
        castingspr[spr_flip]=0;
        castingspr[spr_extend]=0;
        
        holdspr[spr_landhold][spr_hold1][spr_tile]=herospr+30;
        holdspr[spr_landhold][spr_hold1][spr_flip]=0;
        holdspr[spr_landhold][spr_hold1][spr_extend]=0;
        holdspr[spr_landhold][spr_hold2][spr_tile]=herospr+25;
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
        walkspr[up][spr_tile]=herospr+24;
        walkspr[up][spr_flip]=0;
        walkspr[up][spr_extend]=0;
        walkspr[down][spr_tile]=herospr+19;
        walkspr[down][spr_flip]=0;
        walkspr[down][spr_extend]=0;
        walkspr[left][spr_tile]=herospr+16;
        walkspr[left][spr_flip]=1;
        walkspr[left][spr_extend]=0;
        walkspr[right][spr_tile]=herospr+16;
        walkspr[right][spr_flip]=0;
        walkspr[right][spr_extend]=0;
        
        stabspr[up][spr_tile]=herospr+27;
        stabspr[up][spr_flip]=0;
        stabspr[up][spr_extend]=0;
        stabspr[down][spr_tile]=herospr+23;
        stabspr[down][spr_flip]=0;
        stabspr[down][spr_extend]=0;
        stabspr[left][spr_tile]=herospr+22;
        stabspr[left][spr_flip]=1;
        stabspr[left][spr_extend]=0;
        stabspr[right][spr_tile]=herospr+22;
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
        
        poundspr[up][spr_tile]=herospr+3;
        poundspr[up][spr_flip]=0;
        poundspr[up][spr_extend]=0;
        poundspr[down][spr_tile]=herospr+2;
        poundspr[down][spr_flip]=0;
        poundspr[down][spr_extend]=0;
        poundspr[left][spr_tile]=herospr+1;
        poundspr[left][spr_flip]=1;
        poundspr[left][spr_extend]=0;
        poundspr[right][spr_tile]=herospr+1;
        poundspr[right][spr_flip]=0;
        poundspr[right][spr_extend]=0;
        
// Alter these when default sprites are created.
        jumpspr[up][spr_tile]=herospr+24;
        jumpspr[up][spr_flip]=0;
        jumpspr[up][spr_extend]=0;
        jumpspr[down][spr_tile]=herospr+19;
        jumpspr[down][spr_flip]=0;
        jumpspr[down][spr_extend]=0;
        jumpspr[left][spr_tile]=herospr+16;
        jumpspr[left][spr_flip]=1;
        jumpspr[left][spr_extend]=0;
        jumpspr[right][spr_tile]=herospr+16;
        jumpspr[right][spr_flip]=0;
        jumpspr[right][spr_extend]=0;
        
// Alter these when default sprites are created.
        chargespr[up][spr_tile]=herospr+24;
        chargespr[up][spr_flip]=0;
        chargespr[up][spr_extend]=0;
        chargespr[down][spr_tile]=herospr+19;
        chargespr[down][spr_flip]=0;
        chargespr[down][spr_extend]=0;
        chargespr[left][spr_tile]=herospr+16;
        chargespr[left][spr_flip]=1;
        chargespr[left][spr_extend]=0;
        chargespr[right][spr_tile]=herospr+16;
        chargespr[right][spr_flip]=0;
        chargespr[right][spr_extend]=0;
        
        castingspr[spr_tile]=herospr;
        castingspr[spr_flip]=0;
        castingspr[spr_extend]=0;
        
        holdspr[spr_landhold][spr_hold1][spr_tile]=herospr+29;
        holdspr[spr_landhold][spr_hold1][spr_flip]=0;
        holdspr[spr_landhold][spr_hold1][spr_extend]=0;
        holdspr[spr_landhold][spr_hold2][spr_tile]=herospr+28;
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

void setupherodefenses()
{
    //For now this just zeroes out Hero's defenses by default, set these to appropriate defaults if necessary if defense implementation is extended. -Jman
    for (int32_t i = 0; i < wMax; i++)
    {
        hero_defenses[i] = 0;
    }
    
}

void setupherooffsets()
{
	for (int32_t i = 0; i < 4; i++)
	{
		hammeroffsets[i] = 0;
	}
}
