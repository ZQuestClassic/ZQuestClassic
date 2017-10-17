//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zc_custom.cc
//
//  Custom item, enemy, etc. for Zelda.
//
//--------------------------------------------------------

#include "zc_custom.h"
#include "zdefs.h"

//extern itemdata   *itemsbuf;
extern wpndata    *wpnsbuf;

int swimspr, slashspr, linkspr;
int walkspr[4][2];                                          //dir, tile/flip
int stabspr[4][2];                                          //dir, tile/flip
int slashspr2[4][2];                                        //dir, tile/flip
int swimspr2[4][2];                                         //dir, tile/flip
int divespr[4][2];                                          //dir, tile/flip
int poundspr[4][2];                                         //dir, tile/flip
int castingspr;
int holdspr[2][2];                                          //stand/swim, tile/flip

void linktile(int *tile, int *flip, int state, int dir, int style)
{
  switch (state)
  {
    case ls_swim:
      *tile=swimspr2[dir][spr_tile];
      *flip=swimspr2[dir][spr_flip];
      break;
    case ls_dive:
      *tile=divespr[dir][spr_tile];
      *flip=divespr[dir][spr_flip];
      break;
    case ls_slash:
      *tile=slashspr2[dir][spr_tile];
      *flip=slashspr2[dir][spr_flip];
      break;
    case ls_walk:
      *tile=walkspr[dir][spr_tile];
      *flip=walkspr[dir][spr_flip];
      break;
    case ls_stab:
      *tile=stabspr[dir][spr_tile];
      *flip=stabspr[dir][spr_flip];
      break;
    case ls_pound:
      *tile=poundspr[dir][spr_tile];
      *flip=poundspr[dir][spr_flip];
      break;
    case ls_cast:
      *tile=castingspr;
      *flip=0;
      break;
    case ls_hold1:
      *tile=holdspr[spr_standhold][spr_hold1];
      *flip=0;
      break;
    case ls_hold2:
      *tile=holdspr[spr_standhold][spr_hold2];
      *flip=0;
      break;
    case ls_swimhold1:
      *tile=holdspr[spr_swimhold][spr_hold1];
      *flip=0;
      break;
    case ls_swimhold2:
      *tile=holdspr[spr_swimhold][spr_hold2];
      *flip=0;
      break;
    default:
      *tile=0;
      *flip=0;
      break;
  }
}

void setuplinktiles(int style)
{
  swimspr = wpnsbuf[iwSwim].tile;
  slashspr = wpnsbuf[iwLinkSlash].tile;
  linkspr = 4;
  switch (style)
  {
    case 0:                                                 //normal
      walkspr[up][spr_tile]=linkspr+20;
      walkspr[up][spr_flip]=0;
      walkspr[down][spr_tile]=linkspr+18;
      walkspr[down][spr_flip]=0;
      walkspr[left][spr_tile]=linkspr+16;
      walkspr[left][spr_flip]=1;
      walkspr[right][spr_tile]=linkspr+16;
      walkspr[right][spr_flip]=0;
      stabspr[up][spr_tile]=linkspr+23;
      stabspr[up][spr_flip]=0;
      stabspr[down][spr_tile]=linkspr+22;
      stabspr[down][spr_flip]=0;
      stabspr[left][spr_tile]=linkspr+21;
      stabspr[left][spr_flip]=1;
      stabspr[right][spr_tile]=linkspr+21;
      stabspr[right][spr_flip]=0;
      slashspr2[up][spr_tile]=slashspr;
      slashspr2[up][spr_flip]=0;
      slashspr2[down][spr_tile]=slashspr+1;
      slashspr2[down][spr_flip]=0;
      slashspr2[left][spr_tile]=slashspr+2;
      slashspr2[left][spr_flip]=0;
      slashspr2[right][spr_tile]=slashspr+3;
      slashspr2[right][spr_flip]=0;
      swimspr2[up][spr_tile]=swimspr + 4;
      swimspr2[up][spr_flip]=0;
      swimspr2[down][spr_tile]=swimspr + 2;
      swimspr2[down][spr_flip]=0;
      swimspr2[left][spr_tile]=swimspr;
      swimspr2[left][spr_flip]=1;
      swimspr2[right][spr_tile]=swimspr;
      swimspr2[right][spr_flip]=0;
      divespr[up][spr_tile]=swimspr + 8;
      divespr[up][spr_flip]=0;
      divespr[down][spr_tile]=swimspr + 8;
      divespr[down][spr_flip]=0;
      divespr[left][spr_tile]=swimspr + 8;
      divespr[left][spr_flip]=0;
      divespr[right][spr_tile]=swimspr + 8;
      divespr[right][spr_flip]=0;
      poundspr[up][spr_tile]=linkspr+3;
      poundspr[up][spr_flip]=0;
      poundspr[down][spr_tile]=linkspr+2;
      poundspr[down][spr_flip]=0;
      poundspr[left][spr_tile]=linkspr+1;
      poundspr[left][spr_flip]=1;
      poundspr[right][spr_tile]=linkspr+1;
      poundspr[right][spr_flip]=0;
      castingspr=linkspr;
      holdspr[spr_standhold][spr_hold1]=linkspr+30;
      holdspr[spr_standhold][spr_hold2]=linkspr+25;
      holdspr[spr_swimhold][spr_hold1]=swimspr+7;
      holdspr[spr_swimhold][spr_hold2]=swimspr+6;
      break;
    case 1:                                                 //BS
      walkspr[up][spr_tile]=linkspr+24;
      walkspr[up][spr_flip]=0;
      walkspr[down][spr_tile]=linkspr+19;
      walkspr[down][spr_flip]=0;
      walkspr[left][spr_tile]=linkspr+16;
      walkspr[left][spr_flip]=1;
      walkspr[right][spr_tile]=linkspr+16;
      walkspr[right][spr_flip]=0;
      stabspr[up][spr_tile]=linkspr+27;
      stabspr[up][spr_flip]=0;
      stabspr[down][spr_tile]=linkspr+23;
      stabspr[down][spr_flip]=0;
      stabspr[left][spr_tile]=linkspr+22;
      stabspr[left][spr_flip]=1;
      stabspr[right][spr_tile]=linkspr+22;
      stabspr[right][spr_flip]=0;
      slashspr2[up][spr_tile]=slashspr;
      slashspr2[up][spr_flip]=0;
      slashspr2[down][spr_tile]=slashspr+1;
      slashspr2[down][spr_flip]=0;
      slashspr2[left][spr_tile]=slashspr+2;
      slashspr2[left][spr_flip]=0;
      slashspr2[right][spr_tile]=slashspr+3;
      slashspr2[right][spr_flip]=0;
      swimspr2[up][spr_tile]=swimspr+6;
      swimspr2[up][spr_flip]=0;
      swimspr2[down][spr_tile]=swimspr+3;
      swimspr2[down][spr_flip]=0;
      swimspr2[left][spr_tile]=swimspr;
      swimspr2[left][spr_flip]=1;
      swimspr2[right][spr_tile]=swimspr;
      swimspr2[right][spr_flip]=0;
      divespr[up][spr_tile]=swimspr + 11;
      divespr[up][spr_flip]=0;
      divespr[down][spr_tile]=swimspr + 11;
      divespr[down][spr_flip]=0;
      divespr[left][spr_tile]=swimspr + 11;
      divespr[left][spr_flip]=0;
      divespr[right][spr_tile]=swimspr + 11;
      divespr[right][spr_flip]=0;
      poundspr[up][spr_tile]=linkspr+3;
      poundspr[up][spr_flip]=0;
      poundspr[down][spr_tile]=linkspr+2;
      poundspr[down][spr_flip]=0;
      poundspr[left][spr_tile]=linkspr+1;
      poundspr[left][spr_flip]=1;
      poundspr[right][spr_tile]=linkspr+1;
      poundspr[right][spr_flip]=0;
      castingspr=linkspr;
      holdspr[spr_standhold][spr_hold1]=linkspr+29;
      holdspr[spr_standhold][spr_hold2]=linkspr+28;
      holdspr[spr_swimhold][spr_hold1]=swimspr+10;
      holdspr[spr_swimhold][spr_hold2]=swimspr+9;
      break;
    default:
      break;
  }
}
