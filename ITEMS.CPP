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

#include "items.h"
#include "maps.h"
#include "zelda.h"
#include "zdefs.h"

int fairy_cnt=0;

item::~item()
{
  if(id==iFairyMoving && misc>0)
    killfairy(misc);
}

bool item::animate(int index)
{
  //these are here to bypass compiler warnings about unused arguments
  index=index;

  if (is_side_view())
  {
    if (can_drop(x,y) && !(pickup & ipDUMMY))
    {
      y+=(int)fall/100;
      if (fall <= TERMINALV)
      {
        fall += GRAVITY;
      }
    }
    else
    {
      fall = -fall/2; // LA key bounce.
    }
  }
  else
  {
    z-=(int)fall/100;
    if (z<0)
    {
      z = 0;
      fall = -fall/2;
    }
    else if (fall <= TERMINALV)
    {
      fall += GRAVITY;
    }
  }

  if((++clk)>=0x8000)
  {
    clk=0x7000;
  }
  if(flash)
  {
    cs = o_cset;
    if(frame&8)
    {
      cs >>= 4;
    }
    else
    {
      cs &= 15;
    }
  }
  if(anim)
  {
    int spd = o_speed;
    if(aframe==0)
    {
      spd *= o_delay+1;
    }

    if(++aclk >= spd)
    {
      aclk=0;
      if(++aframe >= frames)
      {
        aframe=0;
      }
    }
    tile = o_tile + aframe;
  }

  if(id==iFairyMoving)
  {
    movefairy(x,y,misc);
  }

  if(fadeclk==0)
  {
    return true;
  }
  if(pickup&ipTIMER)
  {
    if(++clk2 == 512)
    {
      return true;
    }
  }
  return false;
}

void item::draw(BITMAP *dest)
{
  if(pickup&ipNODRAW)
    return;
  if(!(pickup&ipFADE) || fadeclk<0 || fadeclk&1)
  {
    if(clk2>32 || (clk2&2)==0 || id==iFairyMoving || id==iFairyStill)
    {
      sprite::draw(dest);
    }
  }
}

item::item(fix X,fix Y,fix Z,int i,int p,int c) : sprite()
{
  x=X; y=Y; z=Z; id=i; pickup=p; clk=c;
  misc=clk2=0;
  aframe=aclk=0;
  anim=flash=twohand=false;
  dummy_int[0]=-1;

  if(id<0 || id>iMax) //>, not >= for dummy items such as the HC Piece display in the subscreen
    return;

  o_tile = itemsbuf[id].tile;
  tile = itemsbuf[id].tile;
  cs = itemsbuf[id].csets&15;
  o_cset = itemsbuf[id].csets;
  o_speed = itemsbuf[id].speed;
  o_delay = itemsbuf[id].delay;
  frames = itemsbuf[id].frames;
  flip = itemsbuf[id].misc>>2;

  if(itemsbuf[id].misc&1)
    flash=true;
  if(itemsbuf[id].misc&2)
    twohand=true;

  anim = itemsbuf[id].frames>0;

  if(pickup&ipBIGRANGE)
  {
    hxofs=-8;
    hxsz=17;
    hyofs=-4;
    hysz=20;
  }
  else if(pickup&ipBIGTRI)
    {
      hxofs=-8;
      hxsz=28;
      hyofs=-4;
      hysz=20;
    }
    else
    {
      hxsz=1;
      hyofs=4;
      hysz=12;
    }

  if(id==iFairyMoving)
  {
    misc = ++fairy_cnt;
    addfairy(x, y, misc);
  }
  for(int j=0;j<8;j++)
  {
    if(j<2) a[j]=itemsbuf[id].initiala[j]*10000;
    d[i]=itemsbuf[id].initiald[j];
  }
}

// easy way to draw an item

void putitem(BITMAP *dest,int x,int y,int item_id)
{
  item temp((fix)x,(fix)y,(fix)0,item_id,0,0);
  temp.yofs=0;
  temp.animate(0);
  temp.draw(dest);
}

void putitem2(BITMAP *dest,int x,int y,int item_id, int &aclk, int &aframe, int flash)
{
  item temp((fix)x,(fix)y,(fix)0,item_id,0,0);
  temp.yofs=0;
  temp.aclk=aclk;
  temp.aframe=aframe;
  if (flash)
  {
    temp.flash=(flash != 0);
  }
  temp.animate(0);
  temp.draw(dest);
  aclk=temp.aclk;
  aframe=temp.aframe;
}

//some methods for dealing with items
int getItemFamily(itemdata* items, int item)
{
	return items[item].family;
}

void removeItemsOfFamily(gamedata *g, itemdata *items, int family)
{
	for(int i=0; i<MAXITEMS; i++)
	{
		if(items[i].family == family)
			g->set_item(i,false);
	}
}

void removeItemsOfFamily(zinitdata *z, itemdata *items, int family)
{
	for(int i=0; i<MAXITEMS; i++)
	{
		if(items[i].family == family)
			z->items[i]=false;
	}
}

int getHighestLevelOfFamily(zinitdata *source, itemdata *items, int family)
{
	int result = -1;
	int highestlevel = -1;
	for(int i=0; i<MAXITEMS; i++)
	{
		if(items[i].family == family && source->items[i])
		{
			if(items[i].fam_type >= highestlevel)
			{
				highestlevel = items[i].fam_type;
				result=i;
			}
		}
	}
	return result;
}

int getHighestLevelOfFamily(gamedata *source, itemdata *items, int family, bool checkenabled)
{
	int result = -1;
	int highestlevel = -1;
	for(int i=0; i<MAXITEMS; i++)
	{
		if(items[i].family == family && source->get_item(i) && (checkenabled?(!(source->items_off[i]) ):1) )
		{
			if(items[i].fam_type >= highestlevel)
			{
				highestlevel = items[i].fam_type;
				result=i;
			}
		}
	}
	return result;
}

void downgradeItemOfFamily(gamedata *g, itemdata *items, int family)
{
    // If you have the red potion, you don't have the blue potion,
    // so it has to be explicitly added
    if(family==itype_potion && g->get_item(iRPotion))
    {
        g->set_item(iRPotion,false);
        g->set_item(iBPotion,true);
        return;
    }

	int maxitem = getHighestLevelOfFamily(g, items, family);
	if(maxitem != -1)
	{
		g->set_item(maxitem,false);
	}
}

int getItemID(itemdata *items, int family, int level)
{
	for(int i=0; i<MAXITEMS; i++)
	{
		if(items[i].family == family && items[i].fam_type == level)
			return i;
	}
	return -1;
}

void addOldStyleFamily(zinitdata *dest, itemdata *items, int family, char levels)
{
	for(int i=0; i<8;i++)
	{
		if(levels & (1<<i))
		{
			int id = getItemID(items, family, i+1);
			if(id != -1)
				dest->items[id]=true;
		}
	}
}

int computeOldStyleBitfield(zinitdata *source, itemdata *items, int family)
{
	int rval=0;
	for(int i=0; i<MAXITEMS; i++)
	{
		if(items[i].family == family && source->items[i])
		{
			if(items[i].fam_type > 0)
				rval |= 1<<(items[i].fam_type-1);
		}
	}
	return rval;
}

char *old_item_string[OLDITEMCNT] =
{
  "Rupee (1)", "Rupee (5)", "Heart", "Bomb (Normal)", "Clock",
  "Sword 1 (Wooden)", "Sword 2 (White)", "Sword 3 (Magic)", "Shield 2 (Magic)", "Key (Normal)",
  "Candle 1 (Blue)", "Candle 2 (Red)", "Letter", "Arrow 1 (Wooden)", "Arrow 2 (Silver)",
  "Bow 1 (Short)", "Bait", "Ring 1 (Blue)", "Ring 2 (Red)", "Bracelet 2",
  "Triforce (Fragment)", "Map", "Compass", "Boomerang 1 (Wooden)", "Boomerang 2 (Magic)",
  "Wand", "Raft", "Ladder 1", "Heart Container", "Potion 1 (Blue)",
  "Potion 2 (Red)", "Whistle", "Magic Book", "Key (Magic)", "Fairy (Moving)",
  "Boomerang 3 (Fire)", "Sword 4 (Master)", "Shield 3 (Mirror)", "Rupee (20)", "Rupee (50)",
  "Rupee (200)", "Wallet 1 (500)", "Wallet 2 (999)", "Dust Pile",
  "Triforce (Whole)", "Selection (1)", "Misc 1", "Misc 2", "Bomb (Super)","Heart Container Piece",
  "Amulet 1", "Flippers", "Hookshot 1 (Short)", "Lens of Truth", "Hammer", "Boots",
  "Bracelet 3", "Arrow 3 (Golden)", "Magic Container", "Magic Jar 1 (Small)",
  "Magic Jar 2 (Large)", "Ring 3 (Golden)", "Kill All Enemies", "Amulet 2",
  "Din's Fire", "Farore's Wind", "Nayru's Love", "Key (Boss)", "Bow 2 (Long)", "Fairy (Stationary)",
  "Arrow Ammunition (1)", "Arrow Ammunition (5)", "Arrow Ammunition (10)", "Arrow Ammunition (30)",
  "Quiver 1 (Small)", "Quiver 2 (Medium)", "Quiver 3 (Large)", "Bomb Ammunition (1)", "Bomb Ammunition (4)", "Bomb Ammunition (8)", "Bomb Ammunition (30)",
  "Bomb Bag 1 (Small)", "Bomb Bag 2 (Medium)", "Bomb Bag 3 (Large)", "Key (Level Specific)", "Selection (2)", "Rupee (10)", "Rupee (100)", "Cane of Byrna",
  "Hookshot 2 (Long)", "Letter (used)", "Roc's Feather", "Hover Boots","Shield 1 (Small)","Scroll: Spin Attack", "Scroll: Cross Beams", "Scroll: Quake Hammer", "Scroll: Super Quake", "Scroll: Hurricane Spin", "Whisp Ring 1", "Whisp Ring 2", "Charge Ring 1", "Charge Ring 2", "Scroll: Peril Beam", "Wallet 3 (Magic)", "Quiver 4 (Magic)", "Bomb Bag 4 (Magic)", "Bracelet 1 (Worn-out Glove)", "Ladder 2 (Four-Way)", "Wealth Medal 1 (75%)", "Wealth Medal 2 (50%)", "Wealth Medal 3 (25%)"/*, "Scroll: Shield Attack"*/
};

char *item_string[ITEMCNT];

char *old_weapon_string[OLDWPNCNT] =
{
  "Sword 1 (Wooden)", "Sword 2 (White)", "Sword 3 (Magic)", "Sword 4 (Master)",
  "Boomerang 1 (Wooden)", "Boomerang 2 (Magic)", "Boomerang 3 (Fire)", "Bomb (Normal)",
  "Bomb (Super)", "Explosion (Normal)", "Arrow 1 (Wooden)", "Arrow 2 (Silver)", "Flame", "Whirlwind",
  "Bait", "Wand", "Magic (Normal)", "Fireball", "Rock", "Arrow (Enemy)", "Sword (Enemy)",
  "Magic (Enemy)", "MISC: Spawn", "MISC: Death", "MISC: <unused>", "Hammer",
  "Hookshot 1 (Short) Head", "Hookshot 1 (Short) Chain (Horizontal)", "Hookshot 1 (Short) Handle", "MISC: Arrow 2 (Silver) Sparkle",
  "MISC: Arrow 3 (Golden) Sparkle", "MISC: Boomerang 2 (Magic) Sparkle", "MISC: Boomerang 3 (Fire) Sparkle",
  "MISC: Hammer Impact", "Arrow 3 (Golden)", "Fire (Enemy)", "Whirlwind (Enemy)", "MISC: Magic Meter",
  "Din's Fire (Rising)", "Din's Fire (Falling)","Din's Fire Trail (Rising)", "Din's Fire Trail (Falling)", "Hookshot 1 (Short) Chain (Vertical)", "MISC: More...",
  "MISC: Boss Marker", "MISC: Link Slash", "Sword 1 (Wooden) Slash",
  "Sword 2 (White) Slash", "Sword 3 (Magic) Slash", "Sword 4 (Master) Slash", "MISC: Shadow (Small)",
  "MISC: Shadow (Large)", "MISC: Bush Leaves", "MISC: Flower Clippings",
  "MISC: Grass Clippings", "MISC: Tall Grass", "MISC: Ripples", "MISC: <unused>",
  "Nayru's Love (Left)", "Nayru's Love (Left, Returning)","Nayru's Love Trail (Left)", "Nayru's Love Trail (Left, Returning)",
  "Nayru's Love (Right)", "Nayru's Love (Right, Returning)","Nayru's Love Trail (Right)", "Nayru's Love Trail (Right, Returning)",
  "Nayru's Love Shield (Front)", "Nayru's Love Shield (Back)", "MISC: Subscreen Vine", "Cane of Byrna", "Cane of Byrna (Slash)",
  "Hookshot 2 (Long) Head", "Hookshot 2 (Long) Chain (Horizontal)", "Hookshot 2 (Long) Handle", "Hookshot 2 (Long) Chain (Vertical)",
  "Explosion (Super)", "Bomb (Enemy, Normal)", "Bomb (Enemy, Super)", "Explosion (Enemy, Normal)",  "Explosion (Enemy, Super)",
  "Fire Trail (Enemy)", "Fire 2 (Enemy)", "Fire 2 Trail (Enemy)", "Magic (Ice, Enemy)", "MISC: Hover Boots Glow", "Magic (Fire)"/*, "MISC: Sideview Ladder", "MISC: Sideview Raft"*/
};

char *weapon_string[WPNCNT];

/*char *lweapon_string[lwMax] =
{
	"(None)","Sword","Sword Beam","Boomerang","Bomb"*/


