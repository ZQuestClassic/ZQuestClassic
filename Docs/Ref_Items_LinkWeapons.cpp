// Link Items and Weapons Reference


/* Item Class Variables
	The following are the present values that can be passed to items in weapons.cpp or link.cpp, 
	from the item editor in ZQUest.
	All examples use 'curitem' as their pointer, for example. curitem.family. 
	They vurther use a legal assign, as an example.
*/

int var;

//Item Class, ->Family in ZScript
var = itemsbuf[curitem].family;

//Level, ->Level in ZScript
var = itemsbuf[curitem].fam_type;

//Keep old, ->Keep in ZScript
var = itemsbuf[curitem].flags & ITEM_GAMEDATA;

//The 'Increase Amount' value in the item editor, ->Increase in ZScript 
var = itemsbuf[curitem].amount;


//The 'Full Max' value in the item editor, ->Max in ZScript. 
var = itemsbuf[curitem].max;

/*
	In conjunction with MaxIncrement (see below) this value controls how the maximum 
	value of the counter of this item (see Counter below) is modified when the item 
	is picked up. 
	
	If MaxIncrement is nonzero at that time, the counter's new maximum value is at 
	that time set to the minimum of its current value plus MaxIncrement, Max.
	
	If Max is less than the current maximum of the counter, Max is ignored and that 
	maximum is used instead.
	
	Notice that as a special case, if Max = MaxIncrement, the counter's maximum value 
	will be forced equal to Max.
*/

//The '+Max' value in the item editor, ->MaxIncrement in ZScript. 
var = itemsbuf[curitem].setmax;

/*
 	In conjunction with Max (see above) this value controls how the maximum value of the 
	counter of this item (see Counter below) is modified when the item is picked up. 
	
	If MaxIncrement is nonzero at that time, the counter's new maximum value is at that 
	time set to the minimum of its current value plus MaxIncrement, and Max.
	
	If Max is less than the current maximum of the counter, Max is ignored and that 
	maximum is used instead.
	
*/

//Counter Reference, ->Counter in ZScript
var = itemsbuf[curitem].count;

//UseSound, ->UseSound in ZScript
var = itemsbuf[curitem].usesound; 
	//Called as SFX(itemsbuf[curitem].usesound)
	
//Damage of the weapon, ->Power in ZScript
var = itemsbuf[curitem].power;

//ID : From the ZScript Parser, ->ID in ZScript; otherwise 'parentitem' or 'itemid'
int id = ri->idata; //this is the stack reference to the item being used.

//->Attributes[] : These are variable purpose indices, used by some items, and unused (at present) by others.
//For example Arrows use 'misc1' (Attributes[0]) for their lifespan. 
var = itemsbuf[curitem].misc1; //Attribute[0]
var = itemsbuf[curitem].misc2; //Attribute[1]
var = itemsbuf[curitem].misc3; //Attribute[2]
var = itemsbuf[curitem].misc4; //Attribute[3]
var = itemsbuf[curitem].misc5; //Attribute[4]
var = itemsbuf[curitem].misc6; //Attribute[5]
var = itemsbuf[curitem].misc7; //Attribute[6]
var = itemsbuf[curitem].misc8; //Attribute[7]
var = itemsbuf[curitem].misc9; //Attribute[8]
var = itemsbuf[curitem].misc10; //Attribute[9]

//->Sprites[] used by the item. THere are ten of these available, used for specific internal purposes.
//Some items use one, other use more. Spells such as 'Din's Fire' use all ten.
var = itemsbuf[curitem].wpn; //Sprites[0]
var = itemsbuf[curitem].wpn2; //Sprites[1]
var = itemsbuf[curitem].wpn3; //Sprites[2]
var = itemsbuf[curitem].wpn4; //Sprites[3]
var = itemsbuf[curitem].wpn5; //Sprites[4]
var = itemsbuf[curitem].wpn6; //Sprites[5]
var = itemsbuf[curitem].wpn7; //Sprites[6]
var = itemsbuf[curitem].wpn8; //Sprites[7]
var = itemsbuf[curitem].wpn9; //Sprites[8]
var = itemsbuf[curitem].wpn10; //Sprites[9]

//Link Tile Modifier, ->Modifier in ZScript
var = itemsbuf[curitem].ltm;

//Action Script, ->Script in ZScript
var = itemsbuf[curitem].script; //it would be possible to set a default script to items. 

//Pickup Script, ->PScript in ZScript
var = itemsbuf[curitem].collect_script;

//Magic Cost, ->MagicCost in ZScript
var = itemsbuf[curitem].magic;

//Minimum Hearts to Pick Up Item (e.g. Z1 swords), ->MinHearts in ZScript.
var = itemsbuf[curitem].pickup_hearts;

//Tile of the item, ->Tile in ZScript. 
var = itemsbuf[curitem].tile;

//Flash Value; seems to be used as a flagset holding the 'Behind' and 'Flip' values, too. ?
//->Flash in ZScript. 
//! Needs verification.
var = itemsbuf[curitem].misc; //Poorly named. 

//CSet, ->CSet in ZScript. 
var = itemsbuf[curitem].csets;

//Number of frames in the item's animation, ->AFrames in ZScript.
var = itemsbuf[curitem].frames;

//Animation Speed of the anim frames, ->ASpeed in ZScript. 
var = itemsbuf[curitem].speed;

//Animation delay, ->Delay in ZScript. 
var = itemsbuf[curitem].delay;

//Upgrade when collected twice, ->Combine in ZScript. 
var = itemsbuf[curitem].flags & ITEM_COMBINE;

//Downgrade to an item one level lower, when used, ->Downgrade in ZScript. 
var = itemsbuf[curitem].flags & ITEM_DOWNGRADE;

//->Flags[] : Five flags are defined for multipurpose use. 
var = itemsbuf[curitem].flags & ITEM_FLAG1; //Flags[0]
var = itemsbuf[curitem].flags & ITEM_FLAG2; //Flags[1]
var = itemsbuf[curitem].flags & ITEM_FLAG3; //Flags[2]
var = itemsbuf[curitem].flags & ITEM_FLAG4; //Flags[3]
var = itemsbuf[curitem].flags & ITEM_FLAG5; //Flags[4]

//When collecting an upgrade, keep the old item too, ->KeepOld in ZScript. 
var = itemsbuf[curitem].flags & ITEM_KEEPOLD;

//Use Rupees instead of Magic, ->RupeeCost in ZScript. 
var = itemsbuf[curitem].flags & ITEM_RUPEE_MAGIC; 

//Can be eaten by enemies, ->Edible in ZScript. 
var = itemsbuf[curitem].flags & ITEM_EDIBLE;

//Unused. I plan to steal this for 'Magic Use is in Seconds, Not Frames'.
//! DO NOT USE for ANYTHING!
var = itemsbuf[curitem].flags & ITEM_UNUSED;

//When collecting an item, gain all lower-level items in its class, ->GainLower in ZScript. 
var = itemsbuf[curitem].flags & ITEM_GAINOLD;

//Ten D# arguments, eight of them, ->InitD[n] in ZScript. 
var = itemsbuf[curitem].initiald[0]; //D0
var = itemsbuf[curitem].initiald[1]; //D1
var = itemsbuf[curitem].initiald[2]; //D2
var = itemsbuf[curitem].initiald[3]; //D3
var = itemsbuf[curitem].initiald[4]; //D4
var = itemsbuf[curitem].initiald[5]; //D5
var = itemsbuf[curitem].initiald[6]; //D6
var = itemsbuf[curitem].initiald[7]; //D7

/* The following are INTERNAL to the items and weapons classes. */
int wpn.power; //The damage value
int wpn.type; //Wepaon type ID, such as wArrow (LW_ARROW). 
int wpn.dead; //The deadstate.
int wpn.misc2; //A general purpose variable for internal use.
int wpn.ignorecombo; //Weapon will not trigger combos. Used for dummy status wepaons. 
bool wpn isLit. //A single bit that can be set if the weapon lights the screen.
int wpn.clk2; wpn->clk2; //A general purpose timer.
int wpn.parentid; //The ID of an NPC that generates an EWeapon. -1 if created by script.
int wpn.parentitem; //The ID of an item that creates an LWeapon. -1 if created by script.
int wpn.dragging; //
fix wpn.step; //The speed at which the weapon moves.
	//Don;t forget to typecast int to fix!
bool wpn.bounce; //Determines if boomerang class weapons are bouncing back to Link.
		 //Note: It is assigned as follows:  bounce=ignoreLink=false; in weapon::weapon(...)
int wpn.cs; //The CSet of the weapon. Assigned by LOADGFX() by cs = wpn.csets&15;
bool wpn.ignoreLink; // Weapon will not have collision with Link. Used for dummy status. 
word wpn.wid; //A 16-bit value that is meant to hold the current tile of the item's animation. 
	//Holds the default weapon tile, too. When calling LOADGFX(int gfx), 'wid' is assigned to 'gfx'.
word wpn.flash; //A 16-bit value that is meant to hold the flash value from itemdata.
		//When calling LOADGFX(wpn), this is set as follows: flash = wpnsbuf[wid].misc&3;
word wpn.aframe; //A 16-bit value that holds the present frame NUMBER of the item's animation.
word wpn.csclk; //A 16-bit value that holds the timer for cset flashing.
int wpn.o_tile; //THe original tile of the item, corresponding to idata.tile.
int wpn.o_cset; //The original cset of the item, corresponding to idata.cset.
int wpn.o_speed; //The original animation speed, corresponding to idata.speed.
int wpn.o_type; //The original ID (weapon type) of the item, corrsponding to idata.type;
	//idata.type has no ZScript instruction, it seems?
int wpn.o_frames; //The number of the frames inof animation, corresponding ti idata.frames.
int wpn.flip; //The present flip state of the item's sprite. Corresponds to *Weapon->Flip in ZScript.
int wpn.temp1; //Used by fire weapons and reads the tile of wFIRE when calling LOADGFX(). 
bool wpn.behind; //A bit to set to determine if the weapon is drawn underneath Link and other sprites.
	//Seems to be assigned oddly: wpnsbuf[wid].misc&WF_BEHIND

/////////////////
/// Functions ///
/////////////////

//Create a weapon pointer. 
weapon(weapon const & other);

//Create a weapon with specific attributes.
weapon(fix X,fix Y,fix Z,int Id,int Type,int pow,int Dir, int Parentid, int prntid, bool isDummy=false);
	//isDummy is used to set a default item ID to a weapon. 

//Set the sprite of a weapon.
void LOADGFX(int wpn);

//Kill a weapon.
bool Dead();
    
//Determine of a weapon 'clips' the hitbox of a sprite. 
bool clip();
    
//Determine of a weapon is blocked.
bool blocked();
    
//As blocked90, but with offsets.
virtual bool blocked(int xOffset, int yOffset);
    
//Runs the item's movement.
virtual bool animate(int index);
    
//If the weapon hits a valid target...
	//Allows setting if the weapon may 'clip' a sprite. 
virtual void onhit(bool clipped);
	//additionally, 'special' determines " " , and 'linkdir' determines " " .
virtual void onhit(bool clipped, int special, int linkdir);
    
//Perform a hit based on a sprite, or a collision rect
virtual bool hit(sprite *s);
	//Need to verify, but these params should be:
	//tx = x1, ty = y1, tz = z1, txsz = x2, tysz = y2, tzsz = z2
virtual bool hit(int tx,int ty,int tz,int txsz,int tysz,int tzsz);

//Render the weapon to the screen, or any arbitrary bitmap.
virtual void draw(BITMAP *dest);
 
 //Handle weapon animations. 
virtual void update_weapon_frame(int change, int orig);

//Add a weapon to the screen
void Lwpns.add(*ptr);
void Lwpns.add(*ptr, fix x, fix y, fix z, int weapon_type, int weapon_family, int weapon_damage, int direction, int itemid, int uid)

