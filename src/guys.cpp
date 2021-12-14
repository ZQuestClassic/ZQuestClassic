//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  guys.cc
//
//  "Guys" code (and other related stuff) for zelda.cc
//
//--------------------------------------------------------

#include "precompiled.h" //always first

#include <string.h>
#include <stdio.h>
#include "zc_alleg.h"
#include "guys.h"
#include "zelda.h"
#include "zsys.h"
#include "maps.h"
#include "link.h"
#include "subscr.h"
#include "ffscript.h"
#include "gamedata.h"
#include "defdata.h"
#include "mem_debug.h"
#include "zscriptversion.h"
#include "particles.h"
extern particle_list particles;

extern FFScript FFCore;
extern word item_doscript[256];
extern refInfo itemScriptData[256];
extern int32_t item_stack[256][MAX_SCRIPT_REGISTERS];
extern ZModule zcm;
extern LinkClass   Link;
extern sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations;
extern zinitdata    zinit;

int32_t repaircharge=0;
bool adjustmagic=false;
bool learnslash=false;
int32_t wallm_load_clk=0;
int32_t sle_x,sle_y,sle_cnt,sle_clk;
int32_t vhead=0;
int32_t guycarryingitem=0;

char *guy_string[eMAXGUYS];

void never_return(int32_t index);
void playLevelMusic();

// If an enemy is this far out of the playing field, just remove it.
#define OUTOFBOUNDS ((int32_t)y>((isSideViewGravity() && canfall(id))?192:352) || y<-176 || x<-256 || x > 512)
//#define NEWOUTOFBOUNDS ((int32_t)y>32767 || y<-32767 || x<-32767 || x > 32767)
#define IGNORE_SIDEVIEW_PLATFORMS (editorflags & ENEMY_FLAG14)
#define OFFGRID_ENEMY (editorflags & ENEMY_FLAG15)

void do_fix(zfix& coord, int32_t val, bool nearest_half = false)
{
	int32_t c = coord.getInt();
	if(nearest_half)
	{
		c += (val/2);
	}
	c -= c % val;
	coord = c;
}

bool NEWOUTOFBOUNDS(zfix x, zfix y, zfix z)
{
	return 
	(
		(((int32_t)y) > FFCore.enemy_removal_point[spriteremovalY2]) 
		|| (((int32_t)y) < FFCore.enemy_removal_point[spriteremovalY1]) 
		|| (((int32_t)x) < FFCore.enemy_removal_point[spriteremovalX1]) 
		|| (((int32_t)x) > FFCore.enemy_removal_point[spriteremovalX2]) 
		|| (((int32_t)z) < FFCore.enemy_removal_point[spriteremovalZ1]) 
		|| (((int32_t)z) > FFCore.enemy_removal_point[spriteremovalZ2])
	);
}

namespace
{
	int32_t trapConstantHorizontalID;
	int32_t trapConstantVerticalID;
	int32_t trapLOSHorizontalID;
	int32_t trapLOSVerticalID;
	int32_t trapLOS4WayID;
	
	int32_t cornerTrapID;
	int32_t centerTrapID;
	
	int32_t rockID;
	int32_t zoraID;
	int32_t statueID;
}

void identifyCFEnemies()
{
	trapConstantHorizontalID=-1;
	trapConstantVerticalID=-1;
	trapLOSHorizontalID=-1;
	trapLOSVerticalID=-1;
	trapLOS4WayID=-1;
	cornerTrapID=-1;
	centerTrapID=-1;
	rockID=-1;
	zoraID=-1;
	statueID=-1;
	
	for(int32_t i=0; i<eMAXGUYS; i++)
	{
		if((guysbuf[i].flags2&cmbflag_trph) && trapLOSHorizontalID==-1)
			trapLOSHorizontalID=i;
		if((guysbuf[i].flags2&cmbflag_trpv) && trapLOSVerticalID==-1)
			trapLOSVerticalID=i;
		if((guysbuf[i].flags2&cmbflag_trp4) && trapLOS4WayID==-1)
			trapLOS4WayID=i;
		if((guysbuf[i].flags2&cmbflag_trplr) && trapConstantHorizontalID==-1)
			trapConstantHorizontalID=i;
		if((guysbuf[i].flags2&cmbflag_trpud) && trapConstantVerticalID==-1)
			trapConstantVerticalID=i;
		
		if((guysbuf[i].flags2&eneflag_trap) && cornerTrapID==-1)
			cornerTrapID=i;
		if((guysbuf[i].flags2&eneflag_trp2) && centerTrapID==-1)
			centerTrapID=i;
		
		if((guysbuf[i].flags2&eneflag_rock) && rockID==-1)
			rockID=i;
		if((guysbuf[i].flags2&eneflag_zora) && zoraID==-1)
			zoraID=i;
		
		if((guysbuf[i].flags2 & eneflag_fire) && statueID==-1)
			statueID=i;
	}
}

int32_t random_layer_enemy()
{
	int32_t cnt=count_layer_enemies();
	
	if(cnt==0)
	{
		return eNONE;
	}
	
	int32_t ret=zc_oldrand()%cnt;
	cnt=0;
	
	for(int32_t i=0; i<6; ++i)
	{
		if(tmpscr->layermap[i]!=0)
		{
			mapscr *layerscreen=&TheMaps[(tmpscr->layermap[i]-1)*MAPSCRS]+tmpscr->layerscreen[i];
			
			for(int32_t j=0; j<10; ++j)
			{
				if(layerscreen->enemy[j]>0&&layerscreen->enemy[j]<MAXGUYS)
				{
					if(cnt==ret)
					{
						return layerscreen->enemy[j];
					}
					
					++cnt;
				}
			}
		}
	}
	
	return eNONE;
}

int32_t count_layer_enemies()
{
	int32_t cnt=0;
	
	for(int32_t i=0; i<6; ++i)
	{
		if(tmpscr->layermap[i]!=0)
		{
			mapscr *layerscreen=&TheMaps[(tmpscr->layermap[i]-1)*MAPSCRS]+tmpscr->layerscreen[i];
			
			for(int32_t j=0; j<10; ++j)
			{
				if(layerscreen->enemy[j]!=0)
				{
					++cnt;
				}
			}
		}
	}
	
	return cnt;
}

int32_t link_on_wall()
{
	zfix lx = Link.getX();
	zfix ly = Link.getY();
	
	
	//zprint2("link_on_wall x is: %d\n", lx);
	//zprint2("link_on_wall y is: %d\n", ly);
	
	if(lx>=48 && lx<=192)
	{
		if(ly==32)  return up+1;
		
		if(ly==128) return down+1;
	}
	
	if(ly>=48 && ly<=112)
	{
		if(lx==32)  return left+1;
		
		if(lx==208) return right+1;
	}
	
	return 0;
}

bool tooclose(int32_t x,int32_t y,int32_t d)
{
	return (abs(int32_t(LinkX())-x)<d && abs(int32_t(LinkY())-y)<d);
}

bool enemy::overpit(enemy *e)
{
	for ( int32_t q = 0; q < hxsz; ++q )
	{
		for ( int32_t q = 0; q < hysz; ++q )
		{
			//check every pixel of the hitbox
			if ( ispitfall(x+q+hxofs, y+q+hyofs) )
			{
				//if the hitbox is over a pit, we can't land
				return true;
			}
		}
	}
	return false;
}

bool enemy::shadow_overpit(enemy *e)
{
	for ( int32_t q = 0; q < hxsz; ++q )
	{
		for ( int32_t q = 0; q < hysz; ++q )
		{
			//check every pixel of the hitbox
			if ( ispitfall(x+q+hxofs, y+q+hyofs+hysz-2) )
			{
				//if the hitbox is over a pit, we can't land
				return true;
			}
		}
	}
	return false;
}

// Returns true iff a combo type or flag precludes enemy movement.
bool enemy::groundblocked(int32_t dx, int32_t dy, bool isKB)
{
	int32_t c = COMBOTYPE(dx,dy);
	bool pit_blocks = (!(moveflags & FLAG_CAN_PITWALK) && (!(moveflags & FLAG_CAN_PITFALL) || !isKB));
	bool water_blocks = (!(moveflags & FLAG_CAN_WATERWALK) && (!(moveflags & FLAG_CAN_WATERDROWN) || !isKB) && get_bit(quest_rules,qr_DROWN));
	return c==cPIT || c==cPITB || c==cPITC ||
		   c==cPITD || c==cPITR || (pit_blocks && ispitfall(dx,dy)) ||
		   // Block enemies type and block enemies flags
		   combo_class_buf[c].block_enemies&1 ||
		   MAPFLAG(dx,dy)==mfNOENEMY || MAPCOMBOFLAG(dx,dy)==mfNOENEMY ||
		   MAPFLAG(dx,dy)==mfNOGROUNDENEMY || MAPCOMBOFLAG(dx,dy)==mfNOGROUNDENEMY ||
		   // Check for ladder-only combos which aren't dried water
		   (combo_class_buf[c].ladder_pass&1 && !iswater_type(c)) ||
		   // Check for drownable water
		   (water_blocks && !(isSideViewGravity()) && (iswaterex(MAPCOMBO(dx,dy), currmap, currscr, -1, dx, dy, false, false, true)));
}

// Returns true iff enemy is floating and blocked by a combo type or flag.
bool enemy::flyerblocked(int32_t dx, int32_t dy, int32_t special, bool isKB)
{
	bool pit_blocks = (!(moveflags & FLAG_CAN_PITWALK) && (!(moveflags & FLAG_CAN_PITFALL) || !isKB));
	bool water_blocks = (!(moveflags & FLAG_CAN_WATERWALK) && (!(moveflags & FLAG_CAN_WATERDROWN) || !isKB));
	return ((special==spw_floater)&&
			((COMBOTYPE(dx,dy)==cNOFLYZONE)||
			 (combo_class_buf[COMBOTYPE(dx,dy)].block_enemies&4)||
			 (MAPFLAG(dx,dy)==mfNOENEMY)||
			 (MAPCOMBOFLAG(dx,dy)==mfNOENEMY)||
			 (water_blocks && iswaterex(MAPCOMBO(dx, dy), currmap, currscr, -1, dx,dy, false, false, true)) ||
			 (pit_blocks && ispitfall(dx,dy))));
}
// Returns true iff a combo type or flag precludes enemy movement.
bool groundblocked(int32_t dx, int32_t dy, guydata const& gd)
{
	int32_t c = COMBOTYPE(dx,dy);
	bool pit_blocks = !(gd.moveflags & FLAG_CAN_PITWALK);
	bool water_blocks = !(gd.moveflags & FLAG_CAN_WATERWALK) && get_bit(quest_rules,qr_DROWN);
	return c==cPIT || c==cPITB || c==cPITC ||
		   c==cPITD || c==cPITR || (pit_blocks && ispitfall(dx,dy)) ||
		   // Block enemies type and block enemies flags
		   combo_class_buf[c].block_enemies&1 ||
		   MAPFLAG(dx,dy)==mfNOENEMY || MAPCOMBOFLAG(dx,dy)==mfNOENEMY ||
		   MAPFLAG(dx,dy)==mfNOGROUNDENEMY || MAPCOMBOFLAG(dx,dy)==mfNOGROUNDENEMY ||
		   // Check for ladder-only combos which aren't dried water
		   (combo_class_buf[c].ladder_pass&1 && !iswater_type(c)) ||
		   // Check for drownable water
		   (water_blocks && !(isSideViewGravity()) && (iswaterex(MAPCOMBO(dx,dy), currmap, currscr, -1, dx, dy, false, false, true)));
}

// Returns true iff enemy is floating and blocked by a combo type or flag.
bool flyerblocked(int32_t dx, int32_t dy, int32_t special, guydata const& gd)
{
	bool pit_blocks = (!(gd.moveflags & FLAG_CAN_PITWALK) && !(gd.moveflags & FLAG_CAN_PITFALL));
	bool water_blocks = !(gd.moveflags & FLAG_CAN_WATERWALK);
	return ((special==spw_floater)&&
			((COMBOTYPE(dx,dy)==cNOFLYZONE)||
			 (combo_class_buf[COMBOTYPE(dx,dy)].block_enemies&4)||
			 (MAPFLAG(dx,dy)==mfNOENEMY)||
			 (MAPCOMBOFLAG(dx,dy)==mfNOENEMY)||
			 (water_blocks && iswaterex(MAPCOMBO(dx,dy), currmap, currscr, -1, dx, dy, false, false, true)) ||
			 (pit_blocks && ispitfall(dx,dy))));
}

/**********************************/
/*******  Enemy Base Class  *******/
/**********************************/

/* ROM data flags

  */

eFire::eFire(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	clk4(clk4),
	shield(shield)

{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eOther::eOther(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	clk4(clk4),
	shield(shield)

{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}




eScript::eScript(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	clk4(clk4),
	shield(shield)

{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eFriendly::eFriendly(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	clk4(clk4),
	shield(shield)

{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eGhini::eGhini(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	clk4(clk4),
	ox(ox),
	oy(oy),
	c(c)

{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eTektite::eTektite(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	old_y(old_y),
	clk2start(clk2start),
	cstart(cstart),
	c(c)

{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eItemFairy::eItemFairy(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

ePeahat::ePeahat(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	ox(ox),
	oy(oy),
	c(c)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eLeever::eLeever(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	temprule(temprule)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eWallM::eWallM(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eStalfos::eStalfos(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	clk4(clk4),
	clk5(clk5),
	fired(fired),
	shield(shield),
	dashing(dashing),
	multishot(multishot),
	fy(fy),
	shadowdistance(shadowdistance)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eZora::eZora(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eSpinTile::eSpinTile(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eNPC::eNPC(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eTrigger::eTrigger(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eProjectile::eProjectile(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	minRange(minRange)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eBoulder::eBoulder(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eRock::eRock(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eTrap2::eTrap2(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eTrap::eTrap(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	ox(ox),
	oy(oy)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}




eKeese::eKeese(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	ox(ox),
	c(c),
	clk4(clk4),
	oy(oy)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eWizzrobe::eWizzrobe(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	charging(charging),
	firing(firing),
	fclk(fclk)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eDodongo::eDodongo(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eDodongo2::eDodongo2(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	previous_dir(previous_dir)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eAquamentus::eAquamentus(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	fbx(fbx),
	clk4(clk4)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eGohma::eGohma(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	clk4(clk4)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eLilDig::eLilDig(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eBigDig::eBigDig(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eGanon::eGanon(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	Stunclk(Stunclk)

{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eMoldorm::eMoldorm(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	segcnt(segcnt),
	segid(segid)

{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

esMoldorm::esMoldorm(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	parentclk(parentclk)
{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}
/*
eManhandla::eManhandla(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	armcnt(armcnt),
	adjusted(adjusted),
	arm[0](arm[0]),
	arm[1](arm[1]),
	arm[2](arm[2]),
	arm[3](arm[3]),
	arm[4](arm[4]),
	arm[5](arm[5]),
	arm[6](arm[6]),
	arm[7](arm[7])
{
	
	//arrays
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

esManhandla::esManhandla(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other)
{
	
	//arrays
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eGleeok::eGleeok(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	flameclk(flameclk),
	flamehead(flamehead),
	necktile(necktile)

{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

esGleeok::esGleeok(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	headtile(headtile),
	flyingheadtile(flyingheadtile),
	necktile(necktile),
	xoffset(xoffset),
	yoffset(yoffset),
	nx(nx),
	ny(ny),
	nxoffset(nxoffset),
	nyoffset(nyoffset),
	parent(parent)

{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	//for ( int32_t q = 0; q < 255; q++ ) 
	//{
	//	nx[q] = other.nx[q];
	//	ny[q] = other.ny[q];
	//	nxoffset[q] = other.nxoffset[q];
	//	nyoffset[q] = other.nyoffset[q];
	//}
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

ePatra::ePatra(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	flycnt(flycnt),
	flycnt2(flycnt2),
	loopcnt(loopcnt),
	lookat(lookat),
	circle_x(circle_x),
	circle_y(circle_y),
	temp_x(temp_x),
	temp_y(temp_y),
	adjusted(adjusted)

{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

ePatraBS::ePatraBS(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	flycnt(flycnt),
	flycnt2(flycnt2),
	loopcnt(loopcnt),
	lookat(lookat),
	adjusted(adjusted)

{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

esPatra::esPatra(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other)

{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for ( int32_t q = 0; q < 255; q++ ) 
	{
		nx[q] = other.nx[q];
		ny[q] = other.ny[q];
		nxoffset[q] = other.nxoffset[q];
		nyoffset[q] = other.nyoffset[q];
	}
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

esPatraBS::esPatraBS(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other)

{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for ( int32_t q = 0; q < 255; q++ ) 
	{
		nx[q] = other.nx[q];
		ny[q] = other.ny[q];
		nxoffset[q] = other.nxoffset[q];
		nyoffset[q] = other.nyoffset[q];
	}
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

*/

enemy::enemy(zfix X,zfix Y,int32_t Id,int32_t Clk) : sprite()
{
	x=X;
	y=Y;
	id=Id;
	clk=Clk;
	floor_y=y;
	ceiling=false;
	fading = misc = clk2 = clk3 = stunclk = hclk = sclk = superman = 0;
	grumble = movestatus = posframe = timer = ox = oy = 0;
	yofs = playing_field_offset - ((isSideViewGravity()) ? 0 : 2);
	did_armos=true;
	script_spawned=false;
	
	d = guysbuf + (id & 0xFFF);
	hp = d->hp;
	starting_hp = hp;
//  cs = d->cset;
//d variables

	flags=d->flags;
	flags2=d->flags2;
	s_tile=d->s_tile; //secondary (additional) tile(s)
	family=d->family;
	dcset=d->cset;
	cs=dcset;
	anim=get_bit(quest_rules,qr_NEWENEMYTILES)?d->e_anim:d->anim;
	dp=d->dp;
	wdp=d->wdp;
	wpn=d->weapon;
	wpnsprite = d-> wpnsprite; //2.6 -Z
	rate=d->rate;
	hrate=d->hrate;
	dstep=d->step;
	homing=d->homing;
	dmisc1=d->misc1;
	dmisc2=d->misc2;
	dmisc3=d->misc3;
	dmisc4=d->misc4;
	dmisc5=d->misc5;
	dmisc6=d->misc6;
	dmisc7=d->misc7;
	dmisc8=d->misc8;
	dmisc9=d->misc9;
	dmisc10=d->misc10;
	dmisc11=d->misc11;
	dmisc12=d->misc12;
	dmisc13=d->misc13;
	dmisc14=d->misc14;
	dmisc15=d->misc15;
	dmisc16=d->misc16;
	dmisc17=d->misc17;
	dmisc18=d->misc18;
	dmisc19=d->misc19;
	dmisc20=d->misc20;
	dmisc21=d->misc21;
	dmisc22=d->misc22;
	dmisc23=d->misc23;
	dmisc24=d->misc24;
	dmisc25=d->misc25;
	dmisc26=d->misc26;
	dmisc27=d->misc27;
	dmisc28=d->misc28;
	dmisc29=d->misc29;
	dmisc30=d->misc30;
	dmisc31=d->misc31;
	dmisc32=d->misc32;
	if (get_bit(quest_rules, qr_BROKEN_ATTRIBUTE_31_32))
	{
		dmisc31 = dmisc32;
		dmisc32 = 0;
	}
	spr_shadow=d->spr_shadow;
	spr_death=d->spr_death;
	spr_spawn=d->spr_spawn;
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=d->defense[i];
		
	bgsfx=d->bgsfx;
	hitsfx=d->hitsfx;
	deadsfx=d->deadsfx;
	bosspal=d->bosspal;
	parent_script_UID = 0;
	
	frozentile = d->frozentile;
	
	frozencset = d->frozencset;
	frozenclock = 0;
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = d->frozenmisc[q];
   
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = 0;
	//firesfx = 0; //t.b.a -Z
	isCore = true; //t.b.a
	parentCore = 0; //t.b.a
	script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	
	firesfx = d->firesfx;
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = d->movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = d->new_weapon[q];
	
	script = (d->script >= 0) ? d->script : 0; //Dont assign invalid data. 
	waitdraw = 0;
	weaponscript = (d->weaponscript >= 0) ? d->weaponscript : 0; //Dont assign invalid data. 
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = d->initD[q];
		//Z_scripterrlog("(enemy::enemy(zfix)): Loading weapon InitD[%d] to an enemy with a value of (%d)\n",q,d->weap_initiald[q]);
		weap_initiald[q] = d->weap_initiald[q];
		//al_trace("Guys.cpp: Assigning guy.initD[%d]: %d\n",q, d->initD.initD[q]);
		//al_trace("Guys.cpp: Assigning guy.initD[%d] from d->initD[%d]: %d\n",q,q, d->initD[q]);
		//al_trace("Guys.cpp: guy.initD[%d] is: %d\n",q, initD[q]);
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = d->initA[q];
		weap_initiala[q] = d->weap_initiala[q];
	}
	
	stickclk = 0;
	submerged = 0;
	hitdir = -1;
	dialogue_str = 0; //set by spawn flags. 
	editorflags = d->editorflags; //set by Enemy Editor 
	//Set the drawing flag for this sprite.
	if ( (editorflags&ENEMY_FLAG12) ) { drawflags |= sprdrawflagALWAYSOLDDRAWS; }
	
	
	if(bosspal>-1)
	{
		loadpalset(csBOSS,pSprite(bosspal));
	}
	
	if(bgsfx>-1)
	{
		cont_sfx(bgsfx);
	}
	
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
		o_tile=d->e_tile;
		frate = d->e_frate;
	}
	else
	{
		o_tile=d->tile;
		frate = d->frate;
	}
	
	tile=0; //init to 0 here, but set it later.
	
	scripttile = -1;
	scriptflip = -1;
	do_animation = 1;
	immortal = false;
	noSlide = false;
	
	haslink=false;
	
	// If they forgot the invisibility flag, here's another failsafe:
	if(o_tile==0 && family!=eeSPINTILE)
		flags |= guy_invisible;
		
//  step = d->step/100.0;
	// To preserve the odd step values for Keese & Gleeok heads. -L
	if(dstep==62.0) dstep+=0.5;
	else if(dstep==89) dstep-=1/9;
	
	step = zslongToFix(dstep*100);
	
	
	item_set = d->item_set;
	grumble = d->grumble;
	
	if(frate == 0)
		frate = 256;
		
	leader = itemguy = dying = scored = false;
	canfreeze = count_enemy = true;
	mainguy = !(flags & guy_doesntcount);
	dir = zc_oldrand()&3;
	
	//2.6 Enemy Editor Hit and TIle Sizes
	if ( ((d->SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && d->txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", d->txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((d->SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && d->tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((d->SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && d->hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((d->SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && d->hysz >= 0 ) hysz = d->hysz;
	if ( ((d->SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && d->hzsz >= 0  ) hzsz = d->hzsz;
	if ( (d->SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (d->SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (d->SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = d->hzofs;
	if (  (d->SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)d->xofs;
	if ( (d->SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (d->SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)d->zofs;
	
	SIZEflags = d->SIZEflags;
	
	if((wpn==ewBomb || wpn==ewSBomb) && family!=eeOTHER && family!=eeFIRE && (family!=eeWALK || dmisc2 != e2tBOMBCHU))
		wpn = 0;
	
	//tile should never be 0 after init --Z (failsafe)
	if (tile <= 0 && FFCore.getQuestHeaderInfo(vZelda) >= 0x255) {tile = o_tile;}
	
	//Moveflags; for gravity and pit interaction
	moveflags = d->moveflags;
	if(!can_pitfall(false))
	{
		//Some enemies must not interact with pits. Force their flags, for sanity's sake.
		moveflags &= ~FLAG_CAN_PITFALL;
		moveflags &= ~FLAG_CAN_WATERDROWN;
	}
}

//base clone constructor

enemy::enemy(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	sprite(other),
	//x(other.x), 		//int32_t
	//y(other.y), 			//int32_t
	//id(other.id),			//int32_t
	//clk(other.clk),			//int32_t
	floor_y(other.floor_y),			//int32_t
	fading(other.fading),			//int32_t
	//misc(other.misc),			//int32_t
	clk2(other.clk2),			//int32_t
	clk3(other.clk3),			//int32_t
	stunclk(other.stunclk),			//int32_t
	hclk(other.hclk),			//int32_t
	sclk(other.sclk),			//int32_t
	superman(other.superman),			//int32_t
	//grumble(other.grumble),			//int32_t
	movestatus(other.movestatus),			//int32_t
	posframe(other.posframe),			//int32_t
	timer(other.timer),			//int32_t
	ox(other.ox),			//int32_t
	oy(other.oy),			//int32_t
	//yofs(other.yofs),			//int32_t
	did_armos(other.did_armos),			//int32_t
	script_spawned(other.script_spawned),			//int32_t
	d(other.d),			//int32_t
	hp(other.hp),			//int32_t
	starting_hp(other.starting_hp),			//int32_t
	//flags(other.flags),			//int32_t
	
	flags2(other.flags2),			//int32_t
	s_tile(other.s_tile),			//int32_t
	family(other.family),			//int32_t
	dcset(other.dcset),			//int32_t
	//cs(other.cs),			//int32_t
	anim(other.anim),			//int32_t
	dp(other.dp),			//int32_t
	wdp(other.wdp),			//int32_t
	wpnsprite(other.wpnsprite),			//int32_t
	rate(other.rate),			//int32_t
	hrate(other.hrate),			//int32_t
	dstep(other.dstep),			//int32_t
	
	homing(other.homing),			//int32_t
	dmisc1(other.dmisc1),			//int32_t
	dmisc2(other.dmisc2),			//int32_t
	dmisc3(other.dmisc3),			//int32_t
	dmisc4(other.dmisc4),			//int32_t
	dmisc5(other.dmisc5),			//int32_t
	dmisc6(other.dmisc6),			//int32_t
	dmisc7(other.dmisc7),			//int32_t
	dmisc8(other.dmisc8),			//int32_t
	dmisc9(other.dmisc9),			//int32_t
	dmisc10(other.dmisc10),			//int32_t
	dmisc11(other.dmisc11),			//int32_t
	dmisc12(other.dmisc12),			//int32_t
	dmisc13(other.dmisc13),			//int32_t
	dmisc14(other.dmisc14),			//int32_t
	dmisc15(other.dmisc15),			//int32_t
	dmisc16(other.dmisc16),			//int32_t
	dmisc17(other.dmisc17),			//int32_t
	dmisc18(other.dmisc18),			//int32_t
	dmisc19(other.dmisc19),			//int32_t
	dmisc20(other.dmisc20),			//int32_t
	dmisc21(other.dmisc21),			//int32_t
	dmisc22(other.dmisc22),			//int32_t
	dmisc23(other.dmisc23),			//int32_t
	dmisc24(other.dmisc24),			//int32_t
	dmisc25(other.dmisc25),			//int32_t
	dmisc26(other.dmisc26),			//int32_t
	dmisc27(other.dmisc27),			//int32_t
	dmisc28(other.dmisc28),			//int32_t
	dmisc29(other.dmisc29),			//int32_t
	dmisc30(other.dmisc30),			//int32_t
	dmisc31(other.dmisc31),			//int32_t
	dmisc32(other.dmisc32),			//int32_t
	bgsfx(other.bgsfx),			//int32_t
	hitsfx(other.hitsfx),			//int32_t
	deadsfx(other.deadsfx),			//int32_t
	bosspal(other.bosspal),			//int32_t
	parent_script_UID(other.parent_script_UID),			//int32_t
	frozentile(other.frozentile),			//int32_t
	frozencset(other.frozencset),			//int32_t
	frozenclock(other.frozenclock),			//int32_t
	isCore(other.isCore),			//int32_t
	parentCore(other.parentCore),			//int32_t
	script_UID(other.script_UID),			//int32_t
	firesfx(other.firesfx),			//int32_t
	//script(other.script),			//int32_t
	//waitdraw(other.waitdraw),			//int32_t
	weaponscript(other.weaponscript),			//int32_t
	stickclk(other.stickclk),			//int32_t
	hitdir(other.hitdir),			//int32_t
	submerged(other.submerged),			//int32_t
	
	dialogue_str(other.dialogue_str),			//int32_t
	editorflags(other.editorflags),			//int32_t
	//drawflags(other.drawflags),			//int32_t
	o_tile(other.o_tile),			//int32_t
	frate(other.frate),			//int32_t
	//tile(other.tile),			//int32_t
	//scripttile(other.scripttile),			//int32_t
	//scriptflip(other.scriptflip),			//int32_t
	//do_animation(other.do_animation),			//int32_t
	immortal(other.immortal),			//int32_t
	noSlide(other.noSlide),			//int32_t
	flags(other.flags),			//int32_t
	step(other.step),			//int32_t
	
	item_set(other.item_set),			//int32_t
	grumble(other.grumble),			//int32_t
	leader(other.leader),			//int32_t
	itemguy(other.itemguy),			//int32_t
	dying(other.dying),			//int32_t
	scored(other.scored),			//int32_t
	//canfreeze(other.canfreeze),			//int32_t
	count_enemy(other.count_enemy),			//int32_t
	mainguy(other.mainguy),			//int32_t
	//dir(other.dir),			//int32_t
	
	//txsz(other.txsz),			//int32_t
	//tysz(other.tysz),			//int32_t
	//hxsz(other.hxsz),			//int32_t
	//hysz(other.hysz),			//int32_t
	//hzsz(other.hzsz),			//int32_t
	//hxofs(other.hxofs),			//int32_t
	//hxofs(other.hxofs),			//int32_t
	//xofs(other.xofs),			//int32_t
	//yofs(other.yofs),			//int32_t
	//hzofs(other.hzofs),			//int32_t
	//zofs(other.zofs),			//int32_t
   
	wpn(other.wpn),			//int32_t
	SIZEflags(other.SIZEflags),			//int32_t
	haslink(haslink)

{
	
	//arrays
	
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
		
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else scrmem = NULL;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));
	
	for(int32_t i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int32_t q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];
	
	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs. 
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int32_t q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int32_t q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];
	
	for ( int32_t q = 0; q < 8; q++ ) 
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int32_t q = 0; q < 2; q++ ) 
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)zofs;
	
	
	
	
}

/*
void enemy::explode(int32_t type)
{
	al_trace("Trying to explode enemy tile: %d\n",o_tile);
	tiledata *temptilebuf = NULL;
	memset(temptilebuf, 0, sizeof(temptilebuf));
	static int32_t tempx, tempy;
	static byte linktilebuf[256];
	int32_t ltile=0;
	int32_t lflip=0;
	unpack_tile(temptilebuf, tile, flip, true);
	//unpack_tile(temptilebuf, tile, flip, true);
	//unpack_tile(temptilebuf, o_tile, 0, true);
	memcpy(linktilebuf, temptilebuf, 256);
	tempx=x;
	tempy=y;
	for(int32_t i=0; i<16; ++i)
	{
				for(int32_t j=0; j<16; ++j)
				{
					if(linktilebuf[i*16+j])
					{
						if(type==0)  // Twilight
						{
							particles.add(new pTwilight((zfix)x+j, (zfix)y-(zfix)z+i, 5, 0, 0, (zc_oldrand()%8)+i*4));
							int32_t k=particles.Count()-1;
							particle *p = (particles.at(k));
							p->step=3;
						}
						else if(type ==1)  // Sands of Hours
						{
							particles.add(new pTwilight((zfix)x+j, (zfix)y-(zfix)z+i, 5, 1, 2, (zc_oldrand()%16)+i*2));
							int32_t k=particles.Count()-1;
							particle *p = (particles.at(k));
							p->step=4;
							
							if(zc_oldrand()%10 < 2)
							{
								p->color=1;
								p->cset=0;
							}
						}
						else
						{
							particles.add(new pFaroresWindDust((zfix)x+j, (zfix)y-(zfix)z+i, 5, 6, linktilebuf[i*16+j], zc_oldrand()%96));
							
							int32_t k=particles.Count()-1;
							particle *p = (particles.at(k));
							p->angular=true;
							p->angle=zc_oldrand();
							p->step=(((double)j)/8);
							p->yofs=0;//yofs;
						}
					}
				}
	}
}
*/


int32_t enemy::getScriptUID() { return script_UID; }
void enemy::setScriptUID(int32_t new_id) { script_UID = new_id; }
enemy::~enemy()
{
	FFCore.deallocateAllArrays(SCRIPT_NPC, getUID());
}

// Handle pitfalls
bool enemy::do_falling(int32_t index)
{
	if(fallclk > 0)
	{
		if(fallclk == PITFALL_FALL_FRAMES && fallCombo) sfx(combobuf[fallCombo].attribytes[0], pan(x.getInt()));
		if(!--fallclk)
		{
			if(immortal) //Keep alive forever
				++fallclk; //force another frame of falling.... forever.
			else if(dying) //Give 1 frame for script revival
			{
				if(flags&guy_neverret)
					never_return(index);
				
				if(leader)
					kill_em_all();
				
				//leave_item(); //Don't drop items in pits!
				stop_bgsfx(index);
				return true;
			}
			else
			{
				try_death(true); //Force death
				++fallclk; //force another frame of falling
			}
		}
		
		wpndata& spr = wpnsbuf[QMisc.sprites[sprFALL]];
		cs = spr.csets & 0xF;
		int32_t fr = spr.frames ? spr.frames : 1;
		int32_t spd = spr.speed ? spr.speed : 1;
		int32_t animclk = (PITFALL_FALL_FRAMES-fallclk);
		tile = spr.newtile + zc_min(animclk / spd, fr-1);
	}
	return false;
}

// Handle drowning in water
bool enemy::do_drowning(int32_t index)
{
	if(drownclk > 0)
	{
		//if(drownclk == WATER_DROWN_FRAMES && drownCombo) sfx(combobuf[drownCombo].attribytes[0], pan(x.getInt()));
		//!TODO: Drown SFX
		if(!--drownclk)
		{
			if(immortal) //Keep alive forever
				++drownclk; //force another frame of falling.... forever.
			else if(dying) //Give 1 frame for script revival
			{
				if(flags&guy_neverret)
					never_return(index);
				
				if(leader)
					kill_em_all();
				
				//leave_item(); //Don't drop items in pits!
				stop_bgsfx(index);
				return true;
			}
			else
			{
				try_death(true); //Force death
				++drownclk; //force another frame of falling
			}
		}
		
		if (drownCombo && combobuf[drownCombo].usrflags&cflag1) 
		{
			wpndata &spr = wpnsbuf[QMisc.sprites[sprLAVADROWN]];
			cs = spr.csets & 0xF;
			int32_t fr = spr.frames ? spr.frames : 1;
			int32_t spd = spr.speed ? spr.speed : 1;
			int32_t animclk = (WATER_DROWN_FRAMES-drownclk);
			tile = spr.newtile + zc_min((animclk % (spd*fr))/spd, fr-1);
		}
		else 
		{
			wpndata &spr = wpnsbuf[QMisc.sprites[sprDROWN]];
			cs = spr.csets & 0xF;
			int32_t fr = spr.frames ? spr.frames : 1;
			int32_t spd = spr.speed ? spr.speed : 1;
			int32_t animclk = (WATER_DROWN_FRAMES-drownclk);
			tile = spr.newtile + zc_min((animclk % (spd*fr))/spd, fr-1);
		}
	}
	return false;
}

// Supplemental animation code that all derived classes should call
// as a return value for animate().
// Handles the death animation and returns true when enemy is finished.
bool enemy::Dead(int32_t index)
{
	if(immortal)
	{
		dying = false;
		return false;
	}
	if(dying)
	{
		--clk2;
		
		if((get_bit(quest_rules,qr_HARDCODED_ENEMY_ANIMS) && clk2==12)
		   && hp>-1000) // not killed by ringleader
			death_sfx();
			
		if(clk2==0)
		{
			if(flags&guy_neverret)
				never_return(index);
				
			if(leader)
				kill_em_all();
				
			leave_item();
		}
		
		stop_bgsfx(index);
		return (clk2==0);
	}
	
	return false;
}

// Basic animation code that all derived classes should call.
// The one with an index is the one that is called by
// the guys sprite list; index is the enemy's index in the list.
bool enemy::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(sclk <= 0) hitdir = -1;
	if(do_falling(index)) return true;
	else if(fallclk)
	{
		//clks
		if(hclk>0)
			--hclk;
		if(stunclk>0)
			--stunclk;
		if ( frozenclock > 0 ) 
			--frozenclock;
		if(haslink)
		{
			Link.setX(x);
			Link.setY(y);
			Link.fallCombo = fallCombo;
			Link.fallclk = fallclk;
			haslink = false; //Let Link go if falling
		}
		run_script(MODE_NORMAL);
		return false;
	}
	if(do_drowning(index)) return true;
	else if(drownclk)
	{
		//clks
		if(hclk>0)
			--hclk;
		if(stunclk>0)
			--stunclk;
		if ( frozenclock > 0 ) 
			--frozenclock;
		if(haslink)
		{
			Link.setX(x);
			Link.setY(y);
			Link.drownclk = drownclk;
			haslink = false; //Let Link go if falling
		}
		run_script(MODE_NORMAL);
		return false;
	}
	int32_t nx = real_x(x);
	int32_t ny = real_y(y);
	
	if(ox!=nx || oy!=ny)
	{
		posframe=(posframe+1)%(get_bit(quest_rules,qr_NEWENEMYTILES)?4:2);
	}
	
	ox = nx;
	oy = ny;
	
	// Maybe they fell off the bottom in sideview, or were moved by a script.
	
	//Check offscreen settings. I wrote it this way for clarity and to simplify testing. -Z
	if ( immortal )
	{
	//skip, as it can go out of bounds, from immortality
	}
	else if (   ( (get_bit(quest_rules, qr_OUTOFBOUNDSENEMIES)) != (editorflags&ENEMY_FLAG11) ) && !NEWOUTOFBOUNDS(x,y,z)   )
	{
	//skip, it can go out of bounds, from a quest rule, or from the enemy editor (but not both!)
	}
	else if ( (OUTOFBOUNDS) )
	{
		hp=-1000; //kill it, as it is not immortal, and no quest bit or rule is enabled
	}
	//fall down
	if((enemycanfall(id) || (moveflags & FLAG_OBEYS_GRAV) )&& fading != fade_flicker && clk>=0)
	{
		if(isSideViewGravity())
		{
			if(!isOnSideviewPlatform())
			{
				bool willHitSVPlatform = false;
				int32_t usewid = (SIZEflags&guyflagOVERRIDE_HIT_WIDTH)?hxsz:16;
				int32_t usehei = (SIZEflags&guyflagOVERRIDE_HIT_HEIGHT)?hysz:16;
				for(int32_t nx = x+4; nx < x+usewid; nx+=16)
				{
					if(fall > 0 && !IGNORE_SIDEVIEW_PLATFORMS && checkSVLadderPlatform(x+4,y+(fall/100)+usehei-1) && (((int32_t(y)+(int32_t(fall)/100)+usehei-1)&0xF0)!=((int32_t(y)+usehei-1)&0xF0)))
					{
						willHitSVPlatform = true;
						break;
					}
				}
				if(willHitSVPlatform)
				{
					y+=fall/100;
					//y-=int32_t(y)%16; //Fix to top of SV Ladder
					do_fix(y, 16); //Fix to top of SV Ladder
					fall = 0;
				}
				else
				{
					y+=fall/100;
					if(fall <= (int32_t)zinit.terminalv)
						fall += (zinit.gravity2/100);
				}
			}
			else
			{
				if(fall!=0)   // Only fix pos once
				{
					//y-=(int32_t)y%8; // Fix position
					do_fix(y, 8); //Fix position
				}
					
				fall = 0;
			}
		}
		else
		{
			if(fall!=0)
				z-=(fall/100);
				
			if(z<0)
				z = fall = 0;
			else if(fall <= (int32_t)zinit.terminalv)
				fall += (zinit.gravity2/100);
			
		}
	}
	if(!isSideViewGravity() && (moveflags & FLAG_CAN_PITFALL))
	{
		if(can_pitfall() && ((z <= 0 && !isflier(id)) || stunclk) && !superman)
		{
			fallCombo = check_pits();
		}
	}
	if(!isSideViewGravity() && (moveflags & FLAG_CAN_WATERDROWN))
	{
		if(can_pitfall() && ((z <= 0 && !isflier(id)) || stunclk) && !superman)
		{
			drownCombo = check_water();
		}
	}
	
	runKnockback(); //scripted knockback handling
	
	// clk is incremented here
	if(++clk >= frate)
		clk=0;
		
	// hit and death handling
	if(hclk>0)
		--hclk;
		
	if(stunclk>0)
		--stunclk;
	if ( frozenclock > 0 ) 
		--frozenclock;
		
	if(ceiling && z<=0)
		ceiling = false;
	
	try_death();
	
	scored=false;
	
	++c_clk;
	
	//Run its script
	if (run_script(MODE_NORMAL)==RUNSCRIPT_SELFDELETE)
	{
		return 0; //Avoid NULLPO if this object deleted itself
	}
	
	// returns true when enemy is defeated
	return Dead(index);
}
bool enemy::m_walkflag_old(int32_t dx,int32_t dy,int32_t special, int32_t x, int32_t y)
{
	int32_t yg = (special==spw_floater)?8:0;
	int32_t nb = get_bit(quest_rules, qr_NOBORDER) ? 16 : 0;
	
	if(dx<16-nb || dy<zc_max(16-yg-nb,0) || dx>=240+nb || dy>=160+nb)
		return true;
		
	bool isInDungeon = isdungeon();
	if(isInDungeon || special==spw_wizzrobe)
	{
		if((x>=32 && dy<32-yg) || (y>-1000 && y<=144 && dy>=144))
			return true;
			
		if((x>=32 && dx<32) || (x>-1000 && x<224 && dx>=224))
			if(special!=spw_door) // walk in door way
				return true;
	}
	
	if(!(moveflags & FLAG_CAN_PITWALK) && !(moveflags & FLAG_CAN_PITFALL)) //Don't walk into pits (knockback doesn't call this func)
	{
		if(ispitfall(dx,dy) || ispitfall(dx+8,dy)
		   || ispitfall(dx,dy+8) || ispitfall(dx+8,dy+8))
		   return true;
	}
	
	switch(special)
	{
	case spw_clipbottomright:
		if(dy>=128 || dx>=208) return true;
		break;
	case spw_clipright:
		break; //if(x>=208) return true; break;
		
	case spw_wizzrobe: // fall through
	case spw_floater: // Special case for fliers and wizzrobes - hack!
		{
			if(isInDungeon)
			{
				if(dy < 32-yg || dy >= 144) return true;
				if(dx < 32 || dx >= 224) return true;
			}
			return false;
		}
	}
	
	dx&=(special==spw_halfstep)?(~7):(~15);
	dy&=(special==spw_halfstep || isSideViewGravity())?(~7):(~15);
	
	if(special==spw_water)
		return (water_walkflag(dx,dy+8,1) || water_walkflag(dx+8,dy+8,1));
		
	return _walkflag(dx,dy+8,1) || _walkflag(dx+8,dy+8,1) ||
		   groundblocked(dx,dy+8) || groundblocked(dx+8,dy+8);
}

bool enemy::m_walkflag_simple(int32_t dx,int32_t dy)
{
	bool kb = false;
	int32_t nb = get_bit(quest_rules, qr_NOBORDER) ? 16 : 0;
	
	if(dx<16-nb || dy<zc_max(16-nb,0) || dx>=240+nb || dy>=160+nb)
		return true;
		
	if(isdungeon())
	{
		if((dy<32) || (dy>=144))
			return true;
			
		if((dx<32) || (dx>=224))
			return true;
	}
	
	if(!(moveflags & FLAG_CAN_PITWALK) && (!(moveflags & FLAG_CAN_PITFALL))) //Don't walk into pits, unless being knocked back
	{
		if(ispitfall(dx,dy) || ispitfall(dx+8,dy)
		   || ispitfall(dx,dy+8) || ispitfall(dx+8,dy+8))
		   return true;
	}
	
	if(get_bit(quest_rules,qr_ENEMY_BROKEN_TOP_HALF_SOLIDITY))
	{
		return _walkflag(dx,dy+8,1) || _walkflag(dx+8,dy+8,1) ||
			   groundblocked(dx,dy+8,kb) || groundblocked(dx+8,dy+8,kb);
	}
	else
	{
		return _walkflag(dx,dy,1) || _walkflag(dx+8,dy,1) ||
			   _walkflag(dx,dy+8,1) || _walkflag(dx+8,dy+8,1) ||
			   groundblocked(dx,dy,kb) || groundblocked(dx+8,dy,kb) ||
			   groundblocked(dx,dy+8,kb) || groundblocked(dx+8,dy+8,kb);
	}
}

bool enemy::m_walkflag(int32_t dx,int32_t dy,int32_t special, int32_t dir, int32_t input_x, int32_t input_y, bool kb)
{
	int32_t yg = (special==spw_floater)?8:0;
	int32_t nb = get_bit(quest_rules, qr_NOBORDER) ? 16 : 0;
	switch(dir)
	{
		case l_down:
		case r_down:
		case down:
		case 11: //r_down
		case 12: //down
		case 13: //l_down
		{
			if ( ((unsigned)(id&0xFFF)) < MAXGUYS )
			{
				if ( SIZEflags&guyflagOVERRIDE_HIT_HEIGHT && !isflier(id) )
				{
					//Small enemies are treated as 16x16, for the purposes of m_walkflag!
					dy += zc_max(hysz-16,0);
				}
			}
			break;
		}
	}
	switch(dir)
	{
		case r_up:
		case r_down:
		case right:
		case 9: //r_up
		case 10: //right
		case 11: //r_down
		{
			if ( ((unsigned)(id&0xFFF)) < MAXGUYS )
			{
				if ( SIZEflags&guyflagOVERRIDE_HIT_WIDTH && !isflier(id) )
				{
					//Small enemies are treated as 16x16, for the purposes of m_walkflag!
					dx += zc_max(hxsz-16,0);
				}
			}
			break;
		}
	}
	//Z_eventlog("Checking x,y %d,%d\n",dx,dy);
	
	if(dx<16-nb || dy<zc_max(16-yg-nb,0) || dx>=240+nb || dy>=160+nb)
		return true;
		
	bool isInDungeon = isdungeon();
	if(isInDungeon || special==spw_wizzrobe)
	{
		if((input_x>=32 && dy<32-yg) || (input_y>-1000 && input_y<=144 && dy>=144))
			return true;
			
		if((input_x>=32 && dx<32) || (input_x>-1000 && input_x<224 && dx>=224))
			if(special!=spw_door) // walk in door way
				return true;
	}
	
	if(!(moveflags & FLAG_CAN_PITWALK) && (!(moveflags & FLAG_CAN_PITFALL) || !kb)) //Don't walk into pits, unless being knocked back
	{
		if(ispitfall(dx,dy) || ispitfall(dx+8,dy)
		   || ispitfall(dx,dy+8) || ispitfall(dx+8,dy+8))
		   return true;
	}
	
	switch(special)
	{
		case spw_clipbottomright:
			if(dy>=128 || dx>=208) return true;
			break;
		case spw_clipright:
			break; //if(input_x>=208) return true; break;
			
		case spw_wizzrobe: // fall through
		case spw_floater: // Special case for fliers and wizzrobes - hack!
			{
				if(isInDungeon)
				{
					if(dy < 32-yg || dy >= 144) return true;
					if(dx < 32 || dx >= 224) return true;
				}
				return false;
			}
	}
	
	dx&=(special==spw_halfstep)?(~7):(~15);
	dy&=(special==spw_halfstep || isSideViewGravity())?(~7):(~15);
	
	if(special==spw_water)
		return (water_walkflag(dx,dy+8,1) || water_walkflag(dx+8,dy+8,1));
	
	if(get_bit(quest_rules,qr_ENEMY_BROKEN_TOP_HALF_SOLIDITY))
	{
		return _walkflag(dx,dy+8,1) || _walkflag(dx+8,dy+8,1) ||
			   groundblocked(dx,dy+8,kb) || groundblocked(dx+8,dy+8,kb);
	}
	else
	{
		return _walkflag(dx,dy,1) || _walkflag(dx+8,dy,1) ||
			   _walkflag(dx,dy+8,1) || _walkflag(dx+8,dy+8,1) ||
			   groundblocked(dx,dy,kb) || groundblocked(dx+8,dy,kb) ||
			   groundblocked(dx,dy+8,kb) || groundblocked(dx+8,dy+8,kb);
	}
}

bool enemy::isOnSideviewPlatform()
{
	int32_t usewid = (SIZEflags&guyflagOVERRIDE_HIT_WIDTH) ? hxsz : 16;
	int32_t usehei = (SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) ? hysz : 16;
	if(y + usehei >= 176 && currscr>=0x70 && !(tmpscr->flags2&wfDOWN)) return true; //Bottom of the map
	for(int32_t nx = x+4; nx < x + usewid; nx+=16)
	{
		if(_walkflag(nx,y+usehei,0)) return true;
		if(IGNORE_SIDEVIEW_PLATFORMS || ((int32_t(y)+usehei)%16)!=0) continue;
		if(checkSVLadderPlatform(nx,y+usehei)) return true;
		if(checkSVLadderPlatform(nx+8,y+usehei)) return true;
	}
	return false;
}

// Stops playing the given sound only if there are no enemies left to play it
void enemy::stop_bgsfx(int32_t index)
{
	if(bgsfx<=0)
		return;
		
	// Look for other enemies with the same bgsfx
	for(int32_t i=0; i<guys.Count(); i++)
	{
		if(i!=index && ((enemy*)guys.spr(i))->bgsfx==bgsfx)
			return;
	}
	
	stop_sfx(bgsfx);
}


// to allow for different sfx on defeating enemy
void enemy::death_sfx()
{
	if(deadsfx > 0) sfx(deadsfx,pan(int32_t(x)));
}

void enemy::move(zfix dx,zfix dy)
{
	/*if(FFCore.getQuestHeaderInfo(vZelda) >= 0x255 && FFCore.getQuestHeaderInfo(vBuild) >= 50 )
	{
	switch(family)
	{
		case eeFIRE:
		case eeOTHER:
			return;
		default: break;
	}
	if(family >= eeSCRIPT01 && family <= eeFFRIENDLY10 ) return;
	}
	*/
	if(!watch && (!(isSideViewGravity()) || isOnSideviewPlatform() || !(moveflags & FLAG_OBEYS_GRAV) || !enemycanfall(id)))
	{
		x+=dx;
		y+=dy;
	}
}

void enemy::move(zfix s)
{
	/*if(FFCore.getQuestHeaderInfo(vZelda) >= 0x255 && FFCore.getQuestHeaderInfo(vBuild) >= 50 )
	{
	switch(family)
	{
		case eeFIRE:
		case eeOTHER:
			return;
		default: break;
	}
	if(family >= eeSCRIPT01 && family <= eeFFRIENDLY10 ) return;
	}*/
	if(!watch && (!(isSideViewGravity()) || isOnSideviewPlatform() || !enemycanfall(id)) || !(moveflags & FLAG_OBEYS_GRAV))
		sprite::move(s);
}

void enemy::leave_item()
{
	int32_t drop_item = select_dropitem(item_set, x, y);
	
	if(drop_item!=-1&&((itemsbuf[drop_item].family!=itype_fairy)||!m_walkflag(x,y,0,dir)))
	{
		if(extend >= 3) items.add(new item(x+(txsz-1)*8,y+(tysz-1)*8,(zfix)0,drop_item,ipBIGRANGE+ipTIMER,0));
		else items.add(new item(x,y,(zfix)0,drop_item,ipBIGRANGE+ipTIMER,0));
	}
}

// auomatically kill off enemy (for rooms with ringleaders)
void enemy::kickbucket()
{
	if(!superman)
		hp=-1000;                                               // don't call death_sfx()
}

bool enemy::isSubmerged()
{
	return submerged;
}

void enemy::FireBreath(bool seeklink)
{
	if(wpn==wNone)
		return;
	
	if(wpn==ewFireTrail)
	{
		dmisc1 = e1tEACHTILE;
		FireWeapon();
		return;
	}
	
	float fire_angle=0.0;
	int32_t wx=0, wy=0, wdir=dir;
	
	if(!seeklink)
	{
		switch(dir)
		{
		case down:
			fire_angle=PI*(int64_t(zc_oldrand()%20)+10)/40;
			wx=x;
			wy=y+8;
			break;
			
		case -1:
		case up:
			fire_angle=PI*(int64_t(zc_oldrand()%20)+50)/40;
			wx=x;
			wy=y-8;
			break;
			
		case left:
			fire_angle=PI*(int64_t(zc_oldrand()%20)+30)/40;
			wx=x-8;
			wy=y;
			break;
			
		case right:
			fire_angle=PI*(int64_t(zc_oldrand()%20)+70)/40;
			wx=x+8;
			wy=y;
			break;
		}
		
		if(wpn==ewFlame || wpn==ewFlame2)
		{
			if(fire_angle==-PI || fire_angle==PI) wdir=left;
			else if(fire_angle==-PI/2) wdir=up;
			else if(fire_angle==PI/2) wdir=down;
			else if(fire_angle==0) wdir=right;
			else if(fire_angle<-PI/2) wdir=l_up;
			else if(fire_angle<0) wdir=r_up;
			else if(fire_angle<(PI/2)) wdir=r_down;
			else if(fire_angle<PI) wdir=l_down;
		}
	}
	else
	{
		wx = x;
		wy = y;
	}
	
	addEwpn(wx,wy,z,wpn,2,wdp,seeklink ? 0xFF : wdir, getUID());
	sfx(wpnsfx(wpn),pan(int32_t(x)));
	
	int32_t i=Ewpns.Count()-1;
	weapon *ew = (weapon*)(Ewpns.spr(i));
	ew->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
	
	if(!seeklink && (zc_oldrand()&4))
	{
		ew->angular=true;
		ew->angle=fire_angle;
	}
	
	if(wpn==ewFlame && wpnsbuf[ewFLAME].frames>1)
	{
		ew->aframe=zc_oldrand()%wpnsbuf[ewFLAME].frames;
		if ( ew->do_animation ) ew->tile+=ew->aframe;
	}
	
	for(int32_t j=Ewpns.Count()-1; j>0; j--)
	{
		Ewpns.swap(j,j-1);
	}
}

void enemy::FireWeapon()
{
	/*
	 * Type:
	 * 0x01: Boss fireball
	 * 0x02: Seeks Link
	 * 0x04: Fast projectile
	 * 0x00-0x30: If 0x02, slants toward (type>>3)-1
	 */
	
	if (wpn < 1) return;
	if(wpn<wEnemyWeapons && dmisc1!=9 && dmisc1!=10 && (wpn < wScript1 && wpn > wScript10) )  // Summoning doesn't require weapons
		return;
		
	if(wpn==ewFireTrail && dmisc1>=e1t3SHOTS && dmisc1<=e1t8SHOTS)
		dmisc1 = e1tEACHTILE;
	
	int32_t xoff = 0;
	int32_t yoff = 0;
	if ( SIZEflags&guyflagOVERRIDE_HIT_WIDTH )
	{
		xoff += (hxsz/2)-8;   
		//Z_scripterrlog("width flag enabled. xoff = %d\n", xoff);
	}
	if ( SIZEflags&guyflagOVERRIDE_HIT_HEIGHT )
	{
		yoff += (hysz/2)-8;   
		//Z_scripterrlog("width flag enabled. yoff = %d\n", yoff);
	}
		
	switch(dmisc1)
	{
	case e1t5SHOTS: //BS-Aquamentus
		Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,2+(((dir^left)+5)<<3),wdp,dir,-1, getUID(),false));
		Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,2+(((dir^right)+5)<<3),wdp,dir,-1, getUID(),false));
		
		[[fallthrough]];
	case e1t3SHOTSFAST:
	case e1t3SHOTS: //Aquamentus
		Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,2+(((dir^left)+1)<<3)+(dmisc1==e1t3SHOTSFAST ? 4:0),wdp,dir,-1, getUID(),false));
		Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,2+(((dir^right)+1)<<3)+(dmisc1==e1t3SHOTSFAST ? 4:0),wdp,dir,-1, getUID(),false));

		[[fallthrough]];
	default:
		Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,2+(dmisc1==e1t3SHOTSFAST || dmisc1==e1tFAST ? 4:0),wdp,wpn==ewFireball2 || wpn==ewFireball ? 0:dir,-1, getUID(),false));
		sfx(wpnsfx(wpn),pan(int32_t(x)));
		break;
		
	case e1tSLANT:
	{
		int32_t slant = 0;
		
		if(((Link.x-x) < -8 && dir==up) || ((Link.x-x) > 8 && dir==down) || ((Link.y-y) < -8 && dir==left) || ((Link.y-y) > 8 && dir==right))
			slant = left;
		else if(((Link.x-x) > 8 && dir==up) || ((Link.x-x) < -8 && dir==down) || ((Link.y-y) > 8 && dir==left) || ((Link.y-y) < -8 && dir==right))
			slant = right;
			
		Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,2+(((dir^slant)+1)<<3),wdp,wpn==ewFireball2 || wpn==ewFireball ? 0:dir,-1, getUID(),false));
		sfx(wpnsfx(wpn),pan(int32_t(x)));
		break;
	}
	
	case e1t8SHOTS: //Fire Wizzrobe
		Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,0,wdp,l_up,-1, getUID(),false));
		((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
		Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,0,wdp,l_down,-1, getUID(),false));
		((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
		Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,0,wdp,r_up,-1, getUID(),false));
		((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
		Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,0,wdp,r_down,-1, getUID(),false));
		((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits

		[[fallthrough]];
	case e1t4SHOTS: //Stalfos 3
		Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,0,wdp,up,-1, getUID(),false));
		((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
		Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,0,wdp,down,-1, getUID(),false));
		((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
		Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,0,wdp,left,-1, getUID(),false));
		((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
		Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,0,wdp,right,-1, getUID(),false));
		((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
		sfx(wpnsfx(wpn),pan(int32_t(x)));
		break;
		
	case e1tSUMMON: // Bat Wizzrobe
	{
		//al_trace("Summon Bats\n");
		//zprint2("Summon Bats\n");
		if(dmisc4==0) break;  // Summon 0
		
		int32_t bc=0;
		
		for(int32_t gc=0; gc<guys.Count(); gc++)
		{
			if((((enemy*)guys.spr(gc))->id) == dmisc3)
			{
				++bc;
			}
		}
		
		if(bc<=40)  // Not too many enemies
		{
			int32_t kids = guys.Count();
			int32_t bats=(zc_oldrand()%zc_max(1,dmisc4))+1;
			
			for(int32_t i=0; i<bats; i++)
			{
			//zprint2("summon\n");
			//al_trace("summon\n");
				if(addchild(x,y,dmisc3,-10, this->script_UID))
		{
					((enemy*)guys.spr(kids+i))->count_enemy = false;
			//((enemy*)guys.spr(guys.Count()-1))->parent_script_UID = this->script_UID;
			//zprint2("Summoner Script UID: %d\n",this->script_UID);
			
		}
			}
			
			sfx(get_bit(quest_rules,qr_MORESOUNDS) ? WAV_ZN1SUMMON : WAV_FIRE,pan(int32_t(x)));
		}
		
		break;
	}
	
	case e1tSUMMONLAYER: // Summoner
	{
		if(count_layer_enemies()==0)
		{
			break;
		}
		
		int32_t kids = guys.Count();
		
		if(kids<40)
		{
			int32_t newguys=(zc_oldrand()%3)+1;
			bool summoned=false;
			
			for(int32_t i=0; i<newguys; i++)
			{
				int32_t id2=vbound(random_layer_enemy(),eSTART,eMAXGUYS-1);
				int32_t x2=0;
				int32_t y2=0;
				
				for(int32_t k=0; k<20; ++k)
				{
					x2=16*((zc_oldrand()%12)+2);
					y2=16*((zc_oldrand()%7)+2);
					
					if((!m_walkflag(x2,y2,0,dir))&&((abs(x2-Link.getX())>=32)||(abs(y2-Link.getY())>=32)))
					{
				//zprint2("summon\n");
				//al_trace("summon\n");
						if(addchild(x2,y2,get_bit(quest_rules,qr_ENEMIESZAXIS) ? 64 : 0,id2,-10, this->script_UID))
			{
							((enemy*)guys.spr(kids+i))->count_enemy = false;
							//((enemy*)guys.spr(guys.Count()-1))->parent_script_UID = this->script_UID;
			}
							
						summoned=true;
						break;
					}
				}
			}
			
			if(summoned)
			{
				sfx(get_bit(quest_rules,qr_MORESOUNDS) ? WAV_ZN1SUMMON : WAV_FIRE,pan(int32_t(x)));
			}
		}
		
		break;
	}
	}
}


// Hit the shield(s)?
// Apparently, this function is only used for hookshots...
bool enemy::hitshield(int32_t wpnx, int32_t wpny, int32_t xdir)
{
	if(!(family==eeWALK || family==eeFIRE || family==eeOTHER))
		return false;
		
	bool ret = false;
	
	// TODO: There must be some bitwise operations that can simplify this...
	if(wpny > y) ret = ((flags&inv_front && xdir==down) || (flags&inv_back && xdir==up) || (flags&inv_left && xdir==left) || (flags&inv_right && xdir==right));
	else if(wpny < y) ret = ((flags&inv_front && xdir==up) || (flags&inv_back && xdir==down) || (flags&inv_left && xdir==right) || (flags&inv_right && xdir==left));
	
	if(wpnx < x) ret = ret || ((flags&inv_front && xdir==left) || (flags&inv_back && xdir==right) || (flags&inv_left && xdir==up) || (flags&inv_right && xdir==down));
	else if(wpnx > x) ret = ret || ((flags&inv_front && xdir==right) || (flags&inv_back && xdir==left) || (flags&inv_left && xdir==down) || (flags&inv_right && xdir==up));
	
	return ret;
}


//! Weapon Editor for 2.6
//To hell with this. I'm writing new functions to resolve weapon type and defence. -Z


//converts a wqeapon ID to its defence index. 
int32_t weaponToDefence(int32_t wid)
{
	switch(wid)
	{
		case wNone: return -1;
		case wSword: return edefSWORD;
		case wBeam: return edefBEAM;
		case wBrang: return edefBRANG;
		case wBomb: return edefBOMB;
		case wSBomb: return edefSBOMB;
		case wLitBomb: return edefBOMB;
		case wLitSBomb: return edefSBOMB;
		case wArrow: return edefARROW;
		case wFire: return edefFIRE;
		case wWhistle:
		{
			//al_trace("Weapon resolved as a whistle, using edef: %s\n", "edefWhistle");
			return edefWhistle;
		}
		case wBait: return edefBAIT;
		case wWand: return edefWAND;
		case wMagic: return edefMAGIC;
		case wCatching: return -1;
		case wWind: return edefWIND;
		case wRefMagic: return edefREFMAGIC;
		case wRefFireball: return edefREFBALL;
		case wRefRock: return edefREFROCK;
		case wHammer: return edefHAMMER;
		case wHookshot: return edefHOOKSHOT;
		case wHSHandle: return edefHOOKSHOT;
		case wHSChain: return edefHOOKSHOT;
		case wSSparkle: return edefSPARKLE;
		case wFSparkle:  return edefSPARKLE;
		case wSmack: return -1; // is this the candle object?
		case wPhantom:  return -1; //engine created visual effects. 
		case wCByrna: return edefBYRNA;
		case wRefBeam:  return edefREFBEAM;
		case wStomp: return edefSTOMP;
		case wScript1:  return edefSCRIPT01;
		case wScript2:  return edefSCRIPT02;
		case wScript3:  return edefSCRIPT03;
		case wScript4: return edefSCRIPT04;
		case wScript5:  return edefSCRIPT05;
		case wScript6:  return edefSCRIPT06;
		case wScript7:  return edefSCRIPT07;
		case wScript8: return edefSCRIPT08;
		case wScript9:  return edefSCRIPT09;
		case wScript10:  return edefSCRIPT10;
		case wIce:  return edefICE;
		case wSound: return edefSONIC;
		//case wThrowRock: return edefTHROWNROCK;
		//case wPot: return edefPOT;
//		case wLitZap: return edefELECTRIC;
//		case wZ3Sword: return edefZ3SWORD;
//		case wLASWord: return edefLASWORD;
//		case wSpinAttk: return edefSPINATTK;
//		case wShield: return edefSHIELD;
//		case wTrowel: return edefTROWEL;
		
		default: return -1;
	}
}

int32_t getDefType(weapon *w)
{
	int32_t id = getWeaponID(w);
	int32_t edef = weaponToDefence(id);
	if(edef == edefHOOKSHOT)
	{
		if(w->family_class == itype_switchhook)
			return edefSwitchHook;
	}
	return edef;
}

int32_t getWeaponID(weapon *w)
{
	int32_t wpnID = w->id;
	//al_trace("getWeaponID(), initial wpnID is: %d\n", wpnID);
	
	if ( w->parentitem < 0 ) 
	{
		//al_trace("enemy::getWeaponID(*w), Step 1B, checking parentitem; parentitem == -1, eturning w->id: %d\n", w->id);
		//Z_message("enemy::getWeaponID(*w), Step 1B, checking parentitem; parentitem == -1, eturning w->id: %d\n", w->id);
		return w->id;
		
	}
	if ( w->parentitem > -1 ) 
	{
		//al_trace("enemy::getWeaponID(*w), Step 1B, checking parentitem; parentitem > -1, and is: %d\n", w->parentitem);
		//Z_message("enemy::getWeaponID(*w), Step 1B, checking parentitem; parentitem > -1, and is: %d\n", w->parentitem);
	
		int32_t usewpn = itemsbuf[w->parentitem].useweapon;
		al_trace("getWeaponID() usewpn is %d\n",usewpn);
		//al_trace("enemy::getWeaponID(*w), Step 2, getting itemsbuf[w->parentitem].useweapon; usewpn is: %d\n", usewpn);
		//Z_message("enemy::getWeaponID(*w), Step 2, getting itemsbuf[w->parentitem].useweapon; usewpn is: %d\n", usewpn);
	
		if ( usewpn > 0 ) 
		{ 
			//al_trace("Assigning weapon id: %d\n", usewpn);
			wpnID = usewpn; //Not forwarding to the weapon sprite?
			//al_trace("enemy::getWeaponID(*w), Step 3A, useweapon > 0; setting wpnID = useweapon; wpnID is: %d\n", wpnID);
			//Z_message("enemy::getWeaponID(*w), Step 3A, useweapon > 0; setting wpnID = useweapon; wpnID is: %d\n", wpnID);
		}
		else 
		{
			wpnID = w->id;
			//al_trace("weaponToDefence() w->id is: %d\n", w->id);
			//al_trace("enemy::getWeaponID(*w), Step 3A, useweapon !> 0; setting wpnID = w->id; wpnID is: %d\n", wpnID);
			//Z_message("enemy::getWeaponID(*w), Step 3A, useweapon !> 0; setting wpnID = w->id; wpnID is: %d\n", wpnID);
			
		}
		return wpnID;
	}
	else if ( w->parentitem == -1 && w->ScriptGenerated ) 
	{
		//al_trace("enemy::getWeaponID(*w), Step 1B, checking parentitem; parentitem > -1, and is: %d\n", w->parentitem);
		//Z_message("enemy::getWeaponID(*w), Step 1B, checking parentitem; parentitem > -1, and is: %d\n", w->parentitem);
		
		int32_t usewpn = w->useweapon;
		//al_trace("weaponToDefence() usewpn is %d\n",usewpn);
		//al_trace("enemy::getWeaponID(*w), Step 2, getting itemsbuf[w->parentitem].useweapon; usewpn is: %d\n", usewpn);
		//Z_message("enemy::getWeaponID(*w), Step 2, getting itemsbuf[w->parentitem].useweapon; usewpn is: %d\n", usewpn);
	
		if ( usewpn > 0 ) 
		{ 
			//al_trace("Assigning weapon id: %d\n", usewpn);
			wpnID = usewpn; //Not forwarding to the weapon sprite?
			//al_trace("enemy::getWeaponID(*w), Step 3A, useweapon > 0; setting wpnID = useweapon; wpnID is: %d\n", wpnID);
			//Z_message("enemy::getWeaponID(*w), Step 3A, useweapon > 0; setting wpnID = useweapon; wpnID is: %d\n", wpnID);
		}
		else 
		{
			wpnID = w->id;
			//al_trace("weaponToDefence() w->id is: %d\n", w->id);
			//al_trace("enemy::getWeaponID(*w), Step 3A, useweapon !> 0; setting wpnID = w->id; wpnID is: %d\n", wpnID);
			//Z_message("enemy::getWeaponID(*w), Step 3A, useweapon !> 0; setting wpnID = w->id; wpnID is: %d\n", wpnID);
			
		}
		return wpnID;
	}
	else 
	{
		return w->id;
	}

}

int32_t enemy::resolveEnemyDefence(weapon *w)
{
	//sword edef is 9, but we're reading it at 0
	//, 
	int32_t weapondef = 0;
	int32_t wid = getWeaponID(w);
	int32_t wtype = (w->useweapon > 0 ? w->useweapon : wid);
	int32_t wdeftype = getDefType(w);
	int32_t usedef = w->usedefence;
	
	if ( usedef > 0 && (wdeftype < 0 || wdeftype >= edefLAST255 || defense[wdeftype] == 0)) 
	{
		//zprint2("Using a default defence of: %d\n", usedef);
		weapondef = usedef*-1;
	}
	else if(wdeftype < 0 || wdeftype >= edefLAST255)
	{
		//zprint2("Invalid wdeftype %d, using no defense\n");
		weapondef = 0;
	}
	else
	{
		//zprint2("Using an engine defence of: %d\n", wdeftype);
		weapondef = wdeftype;
	}
	/*
	if ( defense[weaponToDefence(wid)] == 0 ) {
		weapondef = usedef;
		//al_trace("enemy::resolveEnemyDefence(), Step 4A, defense[wid] == 0; edef = usedef; edef is: %d\n", weapondef);
		//Z_message("enemy::resolveEnemyDefence(), Step 4A, defense[wid] == 0; edef = usedef; edef is: %d\n", weapondef);
	}
	else weapondef = defense[weaponToDefence(wid)]; //defense] is not in the same order as weapon id enum, is it?
	*/
	return weapondef;
}

byte get_def_ignrflag(int32_t edef)
{
	switch(edef)
	{
		case edIGNORE:
		case edIGNOREL1:
		case edSTUNORIGNORE:
			return WPNUNB_IGNR;
		case edSTUNORCHINK:
		case edCHINK:
		case edCHINKL1:
		case edCHINKL2:
		case edCHINKL4:
		case edCHINKL6:
		case edCHINKL8:
		case edCHINKL10:
		case edLEVELCHINK2:
		case edLEVELCHINK3:
		case edLEVELCHINK4:
		case edLEVELCHINK5:
			return WPNUNB_BLOCK;
	}
	return 0;
}

int32_t conv_edef_unblockable(int32_t edef, byte unblockable)
{
	if(!(unblockable&get_def_ignrflag(edef))) return edef;
	switch(edef)
	{
		case edIGNORE:
		case edIGNOREL1:
		case edCHINK:
		case edCHINKL1:
		case edCHINKL2:
		case edCHINKL4:
		case edCHINKL6:
		case edCHINKL8:
		case edCHINKL10:
		case edLEVELCHINK2:
		case edLEVELCHINK3:
		case edLEVELCHINK4:
		case edLEVELCHINK5:
			return edNORMAL;
		case edSTUNORIGNORE:
		case edSTUNORCHINK:
			return edSTUNONLY;
	}
	return edef;
}

// Do we do damage?
// 0: takehit returns 0
// 1: takehit returns 1
// -1: do damage
//The input from resolveEnemyDefence() for the param 'edef' is negative if a specific defence RESULT is being used.
int32_t enemy::defendNew(int32_t wpnId, int32_t *power, int32_t edef, byte unblockable) //May need *wpn to set return on brangs and hookshots
{
	if(switch_hooked) return 0;
	int32_t tempx = x;
	int32_t tempy = y;
	int32_t the_defence = 0;
	if ( edef < 0 ) //we are using a specific base default defence for a weapon
	{
		the_defence = edef*-1; //A specific defence type. 
	}
	else the_defence = defense[edef];
	
	the_defence = conv_edef_unblockable(the_defence, unblockable);
	
	if(shieldCanBlock && !(unblockable&WPNUNB_SHLD))
	{
		switch(the_defence)
		{
			case edIGNORE:
				return 0;
			case edIGNOREL1:
			case edSTUNORIGNORE:
				if(*power <= 0)
					return 0;
		}
		sfx(WAV_CHINK,pan(int32_t(x)));
		return 1;
	}
	
	int32_t new_id = id;
	int32_t effect_type = dmisc15;
	int32_t delay_timer = 90;
	enemy *gleeok = NULL;
	enemy *ptra = NULL;
	int32_t c = 0;
	
	switch(the_defence)
	{
		case edREPLACE:
		{
			sclk = 0;
			if ( dmisc16 > 0 ) new_id = dmisc16;
			else new_id = id+1; 
			if ( new_id > 511 ) new_id = id; //Sanity bound to legal enemy IDs.
			if ( dmisc17 > 0 ) delay_timer = dmisc17;
			//if ( dmisc18 > 0 ) dummy_wpn_id = dmisc18;
			
			//Z_scripterrlog("new id is %d\n", new_id);
				switch(guysbuf[new_id&0xFFF].family)
				{
					//Fixme: possible enemy memory leak. (minor)
					case eeWALK:
					{
					enemy *e = new eStalfos(x,y,new_id,clk);
					guys.add(e);
					}
					break;
				
					case eeLEV:
					{
					enemy *e = new eLeever(x,y,new_id,clk);
					guys.add(e);
					}
					break;
				
					case eeTEK:
					{
					enemy *e = new eTektite(x,y,new_id,clk);
					guys.add(e);
					}
					break;
				
					case eePEAHAT:
					{
					enemy *e = new ePeahat(x,y,new_id,clk);
					guys.add(e);
					}
					break;
				
					case eeZORA:
					{
					enemy *e = new eZora(x,y,new_id,clk);
					guys.add(e);
					}
					break;
				
					case eeGHINI:
					{
					enemy *e = new eGhini(x,y,new_id,clk);
					guys.add(e);
					}
					break;
					
					case eeKEESE:
					{
					enemy *e = new eKeese(x,y,new_id,clk);
					guys.add(e);
					}
					break;
					
					case eeWIZZ:
					{
					enemy *e = new eWizzrobe(x,y,new_id,clk);
					guys.add(e);
					}
					break;
					
					case eePROJECTILE:
					{
					enemy *e = new eProjectile(x,y,new_id,clk);
					guys.add(e);
					}
					break;
					
					case eeWALLM:
					{
					enemy *e = new eWallM(x,y,new_id,clk);
					guys.add(e);
					}
					break;
					
					case eeAQUA:
					{
					enemy *e = new eAquamentus(x,y,new_id,clk);
					guys.add(e);
						e->x = x;
						e->y = y;
					}
					break;
					
					case eeMOLD:
					{
					enemy *e = new eMoldorm(x,y,new_id,zc_max(1,zc_min(254,guysbuf[new_id&0xFFF].misc1)));
					guys.add(e);
						e->x = x;
						e->y = y;
					}
					break;
					
					case eeMANHAN:
					{
					enemy *e = new eManhandla(x,y,new_id,clk);
					guys.add(e);
						e->x = x;
						e->y = y;
					}
					break;
					
					case eeGLEEOK:
					{
						*power = 0; 
						gleeok = new eGleeok(x,y,new_id,guysbuf[new_id&0xFFF].misc1);
						guys.add(gleeok);
						((enemy*)guys.spr(guys.Count()-1))->hclk = delay_timer;
						//((enemy*)guys.spr(guys.Count()-1))->stunclk = delay_timer;
						new_id &= 0xFFF;
						int32_t head_cnt = zc_max(1,zc_min(254,guysbuf[new_id&0xFFF].misc1));
							Z_scripterrlog("Gleeok head count is %d\n",head_cnt);
						for(int32_t i=0; i<head_cnt; i++)
						{
							//enemy *e = new esGleeok(x,y,new_id+0x1000,clk,gleeok);
							if(!guys.add(new esGleeok((zfix)x,(zfix)y,new_id+0x1000,c, gleeok)))
							{
							al_trace("Gleeok head %d could not be created!\n",i+1);
							
							for(int32_t j=0; j<i+1; j++)
							{
								guys.del(guys.Count()-1);
							}
							
							break;
							}
							else
							{
							((enemy*)guys.spr(guys.Count()-1))->hclk = delay_timer;
							//((enemy*)guys.spr(guys.Count()-1))->stunclk = delay_timer;
							}
							
							c-=guysbuf[new_id].misc4;
							//gleeok->x = x;
							//gleeok->y = y;
							//gleeok = e;
						}
						return 1;
					}
					
					case eeGHOMA:
					{
					enemy *e = new eGohma(x,y,new_id,clk);
					guys.add(e);
						e->x = x;
						e->y = y;
					}
					break;
					
					case eeLANM:
					{
					enemy *e = new eLanmola(x,y,new_id,zc_max(1,zc_min(253,guysbuf[new_id&0xFFF].misc1)));
					guys.add(e);
						e->x = x;
						e->y = y;
					}
					break;
					
					case eeGANON:
					{
					enemy *e = new eGanon(x,y,new_id,clk);
					guys.add(e);
						e->x = x;
						e->y = y;
					}
					break;
					
					case eeFAIRY:
					{
					enemy *e = new eItemFairy(x,y,new_id+0x1000*clk,clk);
					guys.add(e);
						e->x = x;
						e->y = y;
					}
					break;
					
					case eeFIRE:
					{
					enemy *e = new eFire(x,y,new_id,clk);
					guys.add(e);
						e->x = x;
						e->y = y;
					}
					break;
					
					case eeOTHER: 
					{
					enemy *e = new eOther(x,y,new_id,clk);
					guys.add(e);
						e->x = x;
						e->y = y;
					}
					break;
					
					case eeSPINTILE:
					{
					enemy *e = new eSpinTile(x,y,new_id,clk);
					guys.add(e);
						e->x = x;
						e->y = y;
					}
					break;
					
					// and these enemies use the misc10/misc2 value
					case eeROCK:
					{
						switch(guysbuf[new_id&0xFFF].misc10)
						{
							case 1:
							{
							enemy *e = new eBoulder(x,y,new_id,clk);
							guys.add(e);
								e->x = x;
								e->y = y;
							}
							break;
						
							case 0:
							default:
							{
							enemy *e = new eRock(x,y,new_id,clk);
							guys.add(e);
								e->x = x;
								e->y = y;
							}
							break;
						}
					
						break;
					}
					
					case eeTRAP:
					{
						switch(guysbuf[new_id&0xFFF].misc2)
						{
							case 1:
							{
							enemy *e = new eTrap2(x,y,new_id,clk);
							guys.add(e);
								e->x = x;
								e->y = y;
							}
							break;
						
							case 0:
							default:
							{
							enemy *e = new eTrap(x,y,new_id,clk);
							guys.add(e);
								e->x = x;
								e->y = y;
							}
							break;
						}
					
						break;
					}
					
					case eeDONGO:
					{
						switch(guysbuf[new_id&0xFFF].misc10)
						{
							case 1:
							{
							enemy *e = new eDodongo2(x,y,new_id,clk);
							guys.add(e);
								e->x = x;
								e->y = y;
							}
							break;
						
							case 0:
							default:
							{
							enemy *e = new eDodongo(x,y,new_id,clk);
							guys.add(e);
								e->x = x;
								e->y = y;
							}
							break;
						}
					
						break;
					}
					
					case eeDIG:
					{
						switch(guysbuf[new_id&0xFFF].misc10)
						{
							case 1:
							{
							enemy *e = new eLilDig(x,y,new_id,clk);
							guys.add(e);
								e->x = x;
								e->y = y;
							}
							break;
							
							case 0:
							default:
							{
							enemy *e = new eBigDig(x,y,new_id,clk);
							guys.add(e);
								e->x = x;
								e->y = y;
							}
							break;
						}
					
						break;
					}
					
					case eePATRA:
					{
						switch(guysbuf[new_id&0xFFF].misc10)
						{
							case 1:
							{
								if (get_bit(quest_rules,qr_HARDCODED_BS_PATRA))
								{
									enemy *e = new ePatraBS(x,y,new_id,clk);
									guys.add(e);
									e->x = x;
									e->y = y;
									break;
								}
							}
							[[fallthrough]];
							case 0:
							default:
							{
								enemy *e = new ePatra(x,y,new_id,clk);
								guys.add(e);
								e->x = x;
								e->y = y;
							}
							break;
						}
					
						break;
					}
					
					case eeGUY:
					{
						switch(guysbuf[new_id&0xFFF].misc10)
						{
							case 1:
							{
							enemy *e = new eTrigger(x,y,new_id,clk);
							guys.add(e);
							}
							break;
						
							case 0:
							default:
							{
							enemy *e = new eNPC(x,y,new_id,clk);
							guys.add(e);
							}
							break;
						}
					
						break;
					}
					
						case eeSCRIPT01: 
						case eeSCRIPT02: 
						case eeSCRIPT03: 
						case eeSCRIPT04: 
						case eeSCRIPT05: 
						case eeSCRIPT06: 
						case eeSCRIPT07: 
						case eeSCRIPT08: 
						case eeSCRIPT09: 
						case eeSCRIPT10: 
						case eeSCRIPT11: 
						case eeSCRIPT12: 
						case eeSCRIPT13: 
						case eeSCRIPT14: 
						case eeSCRIPT15: 
						case eeSCRIPT16: 
						case eeSCRIPT17: 
						case eeSCRIPT18: 
						case eeSCRIPT19: 
						case eeSCRIPT20: 
					{
						enemy *e = new eScript(x,y,new_id,clk);
						guys.add(e);
						e->x = x;
						e->y = y;
						break;
					}
					
					
					case eeFFRIENDLY01:
					case eeFFRIENDLY02:
					case eeFFRIENDLY03:
					case eeFFRIENDLY04:
					case eeFFRIENDLY05:
					case eeFFRIENDLY06:
					case eeFFRIENDLY07:
					case eeFFRIENDLY08:
					case eeFFRIENDLY09:
					case eeFFRIENDLY10:
					{
						enemy *e = new eFriendly(x,y,new_id,clk);
						guys.add(e);
						e->x = x;
						e->y = y;
						break;
					}
					
					
					default: break;
				}
				
					// add segments of segmented enemies
				int32_t c=0;
				
				switch(guysbuf[new_id&0xFFF].family)
				{
					case eeMOLD:
					{
					byte is=((enemy*)guys.spr(guys.Count()-1))->item_set;
					new_id &= 0xFFF;
					
					for(int32_t i=0; i<zc_max(1,zc_min(254,guysbuf[new_id].misc1)); i++)
					{
						//christ this is messy -DD
						int32_t segclk = -i*((int32_t)(8.0/(zslongToFix(guysbuf[new_id&0xFFF].step*100))));
						
						if(!guys.add(new esMoldorm((zfix)x,(zfix)y,new_id+0x1000,segclk)))
						{
						al_trace("Moldorm segment %d could not be created!\n",i+1);
						
						for(int32_t j=0; j<i+1; j++)
							guys.del(guys.Count()-1);
							
						return 0;
						}
						
						if(i>0)
						((enemy*)guys.spr(guys.Count()-1))->item_set=is;
						
						
					}
					
					break;
					}
					
					case eeLANM:
					{
					new_id &= 0xFFF;
					int32_t shft = guysbuf[new_id].misc2;
					byte is=((enemy*)guys.spr(guys.Count()-1))->item_set;
					enemy *e = new esLanmola((zfix)x,(zfix)y,new_id+0x1000,0);
						
					if(!guys.add(e))
					{
						al_trace("Lanmola segment 1 could not be created!\n");
						guys.del(guys.Count()-1);
						return 0;
					}
					e->x = x;
					e->y = y;
					
					
					
					for(int32_t i=1; i<zc_max(1,zc_min(253,guysbuf[new_id&0xFFF].misc1)); i++)
					{
						enemy *e2 = new esLanmola((zfix)x,(zfix)y,new_id+0x2000,-(i<<shft));
						if(!guys.add(e2))
						{
							al_trace("Lanmola segment %d could not be created!\n",i+1);
							
							for(int32_t j=0; j<i+1; j++)
								guys.del(guys.Count()-1);
								
							return 0;
						}
						e2->x = x;
						e2->y = y;
						
						((enemy*)guys.spr(guys.Count()-1))->item_set=is;
						
					}
					}
					break;
					
					case eeMANHAN:
					new_id &= 0xFFF;
					
					for(int32_t i=0; i<((!(guysbuf[new_id].misc2))?4:8); i++)
					{
						if(!guys.add(new esManhandla((zfix)x,(zfix)y,new_id+0x1000,i)))
						{
						al_trace("Manhandla head %d could not be created!\n",i+1);
						
						for(int32_t j=0; j<i+1; j++)
						{
							guys.del(guys.Count()-1);
						}
						
						return 0;
						}
						
						
						((enemy*)guys.spr(guys.Count()-1))->frate=guysbuf[new_id].misc1;
					}
					
					break;
					
					case eeGLEEOK:
					{
					/*
					new_id &= 0xFFF;
					int32_t head_cnt = zc_max(1,zc_min(254,guysbuf[new_id&0xFFF].misc1));
						Z_scripterrlog("Gleeok head count is %d\n",head_cnt);
					for(int32_t i=0; i<head_cnt; i++)
					{
						//enemy *e = new esGleeok(x,y,new_id+0x1000,clk,gleeok);
						if(!guys.add(new esGleeok((zfix)x,(zfix)y,new_id+0x1000,c, gleeok)))
						{
						al_trace("Gleeok head %d could not be created!\n",i+1);
						
						for(int32_t j=0; j<i+1; j++)
						{
							guys.del(guys.Count()-1);
						}
						
						break;
						}
						
						c-=guysbuf[new_id].misc4;
						*/
						
					// }
					}
					break;
					
					
					case eePATRA:
					{
					new_id &= 0xFFF;
					int32_t outeyes = 0;
						ptra = new ePatraBS((zfix)x,(zfix)y,id,clk);
					
					for(int32_t i=0; i<zc_min(254,guysbuf[new_id&0xFFF].misc1); i++)
					{
						if(!((guysbuf[new_id].misc10&&get_bit(quest_rules,qr_HARDCODED_BS_PATRA))?guys.add(new esPatraBS((zfix)x,(zfix)y,new_id+0x1000,i,ptra)):guys.add(new esPatra((zfix)x,(zfix)y,new_id+0x1000,i,ptra))))
						{
						al_trace("Patra outer eye %d could not be created!\n",i+1);
						
						for(int32_t j=0; j<i+1; j++)
							guys.del(guys.Count()-1);
							
						return 0;
						}
						else
						outeyes++;
						
						
					}
					
					for(int32_t i=0; i<zc_min(254,guysbuf[new_id&0xFFF].misc2); i++)
					{
						if(!guys.add(new esPatra((zfix)x,(zfix)y,new_id+0x1000,i,ptra)))
						{
						al_trace("Patra inner eye %d could not be created!\n",i+1);
						
						for(int32_t j=0; j<i+1+zc_min(254,outeyes); j++)
							guys.del(guys.Count()-1);
							
						return 0;
						}
						
						
					}
					delete ptra;
					break;
					}
				}
				
				
			
			((enemy*)guys.spr(guys.Count()-1))->count_enemy = true;
			((enemy*)guys.spr(guys.Count()-1))->stunclk = delay_timer;
			((enemy*)guys.spr(guys.Count()-1))->dir = this->dir;
			((enemy*)guys.spr(guys.Count()-1))->scale = this->scale;
			((enemy*)guys.spr(guys.Count()-1))->angular = this->angular;
			((enemy*)guys.spr(guys.Count()-1))->angle = this->angle;
			((enemy*)guys.spr(guys.Count()-1))->rotation = this->rotation;
			//((enemy*)guys.spr(guys.Count()-1))->mainguy = this->mainguy; //This might mean that it is a core. 
			((enemy*)guys.spr(guys.Count()-1))->itemguy = this->itemguy;
			((enemy*)guys.spr(guys.Count()-1))->leader = this->leader;
			((enemy*)guys.spr(guys.Count()-1))->hclk = delay_timer;
			((enemy*)guys.spr(guys.Count()-1))->script_spawned = this->script_spawned;
			((enemy*)guys.spr(guys.Count()-1))->script_UID = this->script_UID;
			((enemy*)guys.spr(guys.Count()-1))->sclk = 0;
			
			
			item_set = 0; //Do not make a drop. 
			
			switch(effect_type)
			{
				case -7:
				{
					weapon *w = new weapon(x,y,z,wBomb,0,wdp,0,-1,getUID(),false, 0);
					Lwpns.add(w);
					break;
				}
				case -6:
				{
					weapon *w = new weapon(x,y,z,wSBomb,0,wdp,0,-1,getUID(),false, 0);
					Lwpns.add(w);
					break;
				}
				case -5: 
				{
					weapon *w = new weapon(x,y,z,wBomb,effect_type,0,0,Link.getUID(), txsz, tysz);
					Lwpns.add(w);
					break;
				}
				case -4:
				{
					weapon *w = new weapon(x,y,z,wSBomb,effect_type,0,0,Link.getUID(), txsz, tysz);
					Lwpns.add(w);
					break;
				}
				case -3: explode(1); break;
				case -2: explode(2); break;
				case -1: explode(0); break;
				case 0: break;
				
				default:
				{
					//Dummy weapon function
					if ( effect_type > 255 ) effect_type = 0; //Sanity bound the sprite ID.
					//weapon *w = new weapon(x,y,z,dummy_wpn_id,effect_type,0,0,Link.getUID(), txsz, tysz);
					weapon *w = new weapon(x,y,z,wSSparkle,effect_type,0,0,Link.getUID(), txsz, tysz,0,0,0,0,0,0,0);
					Lwpns.add(w);
					break;
				}
			}
			
			
			yofs = -32768;
			switch(guysbuf[new_id&0xFFF].family)
			{
				case eeGLEEOK:
				{
					Z_scripterrlog("Replacing a gleeok.\n");
					enemy *tempenemy = (enemy *) guys.getByUID(parentCore);
					hp = -999;
					tempenemy->hp = -999; 
					break;
					
				}
				default:
					hp = -1000; break;
			}
			++game->guys[(currmap*MAPSCRSNORMAL)+currscr];
			return 1;
			
		}
		case edSPLIT:
		{
			//int32_t ex = x; int32_t ey = y;
			//al_trace("edSplit dmisc3: %d\n", dmisc3);
			//al_trace("edSplit dmisc4: %d\n", dmisc4);
			/*
			if ( txsx > 1 ) 
			{
				ex += ( txsz-1 ) * 8; //from its middle
			}
			if ( tysx > 1 ) 
			{
				ey += ( tysz-1 ) * 8; //from its middle
			}
			*/
			for ( int32_t q = 0; q < dmisc4; q++ )
			{
				
				//addenemy((x+(txsz*16)/2),(y+(tysz*16)/2),dmisc3+0x1000,-15);
				addenemy(
					//ex,ey,
					x,y,
						dmisc3+0x1000,-15);
				//addenemy(ex,ey,dmisc3,0);
				
			}
			item_set = 0; //Do not make a drop. 
			hp = -1000;
			return -1;
			
		}
		case edSUMMON: 
		{
			
			
			//al_trace("edSplit dmisc3: %d\n", dmisc3);
			//al_trace("edSplit dmisc4: %d\n", dmisc4);
			int32_t summon_count = (zc_oldrand()%dmisc4)+1;
			for ( int32_t q = 0; q < summon_count; q++ )
			{
				int32_t x2=16*((zc_oldrand()%12)+2);
				int32_t y2=16*((zc_oldrand()%7)+2);
				addenemy(
					//(x+(txsz*16)/2),(y+(tysz*16)/2)
					x2,y2,
						dmisc3+0x1000,-15);
				//addenemy(ex,ey,dmisc3,0);
				
			}
			sfx(get_bit(quest_rules,qr_MORESOUNDS) ? WAV_ZN1SUMMON : WAV_FIRE,pan(int32_t(x)));
			return -1;
			
		}
		
		case edEXPLODESMALL:
		{
			weapon *ew=new weapon(x,y,z, ewBomb, 0, dmisc4, dir,-1,getUID(),false);
			Ewpns.add(ew);
			item_set = 0; //Should we make a drop?
			hp = -1000;
			return -1;
		}
		
		
		case edEXPLODEHARMLESS:
		{
			weapon *ew=new weapon(x,y,z, ewSBomb, 0, dmisc4, dir,-1,getUID(),false);
			Ewpns.add(ew);
			ew->hyofs = -32768;
			item_set = 0; //Should we make a drop?
			hp = -1000;
			return -1;
		}
		
		
		case edEXPLODELARGE:
		{
			weapon *ew=new weapon(x,y,z, ewSBomb, 0, dmisc4, dir,-1,getUID(),false);
			Ewpns.add(ew);
			
			hp = -1000;
			return -1;
		}
		
		
		case edTRIGGERSECRETS:
		{
			hidden_entrance(0, true, false, -4);
			return -1;
		}
		
		case edSTUNORCHINK:
			if (stunclk && get_bit(quest_rules, qr_NO_STUNLOCK))
			{
				sfx(WAV_CHINK,pan(int32_t(x)));
				return 1;
			}
			else if(*power <= 0)
			{
			//al_trace("defendNew() is at: %s\n", "returning edSTUNORCHINK");
				sfx(WAV_CHINK,pan(int32_t(x)));
				return 1;
			}
			[[fallthrough]];
			
		case edSTUNORIGNORE:
			if (stunclk && get_bit(quest_rules, qr_NO_STUNLOCK))
			{
				sfx(WAV_CHINK,pan(int32_t(x)));
				return 1;
			}
			else if(*power <= 0)
				return 0;
			[[fallthrough]];
				
		case edSTUNONLY:
			if((wpnId==wFire || wpnId==wBomb || wpnId==wSBomb || wpnId==wHookshot || wpnId==wSword) && stunclk>=159)
			{
				//al_trace("enemy::defend(), edSTUNONLY found a weapon of type FIRE, BOMB, SBOMB, HOOKSHOT, or SWORD:, with wpnId:  \n", wpnId);
			   // Z_message("enemy::defend(), edSTUNONLY found a weapon of type FIRE, BOMB, SBOMB, HOOKSHOT, or SWORD:, with wpnId:  \n", wpnId);
					return 1;
			}
			if (stunclk && get_bit(quest_rules, qr_NO_STUNLOCK))
			{
				sfx(WAV_CHINK,pan(int32_t(x)));
				return 1;
			}
			else
			{
				stunclk=160;
				sfx(WAV_EHIT,pan(int32_t(x)));
				
				return 1;
			}
			
		case edCHINKL1:
			if(*power >= 1*game->get_hero_dmgmult()) break;
			[[fallthrough]];
		case edCHINKL2:
			if(*power >= 2*game->get_hero_dmgmult()) break;
			[[fallthrough]];
		case edCHINKL4:
			if(*power >= 4*game->get_hero_dmgmult()) break;
			[[fallthrough]];
		case edCHINKL6:
			if(*power >= 6*game->get_hero_dmgmult()) break;
			[[fallthrough]];
		case edCHINKL8:
			if(*power >= 8*game->get_hero_dmgmult()) break;
			[[fallthrough]];
		case edCHINKL10:
			if(*power >= 10*game->get_hero_dmgmult()) break;
			[[fallthrough]];
		case edCHINK:
			//al_trace("defendNew() is at: %s\n", "returning edCHINK");
			sfx(WAV_CHINK,pan(int32_t(x)));
			return 1;
			
		case edIGNOREL1:
			if(*power > 0)  break;
			[[fallthrough]];
			
		case edIGNORE:
			return 0;
			
		case ed1HKO:
			*power = hp;
			return -2;
			
		case ed2x:
		{
			*power = zc_max(1,*power*2);
		//int32_t pow = *power;
			//*power = vbound((pow*2),0,214747);
		return -1; 
		}
		case ed3x:
		{
			*power = zc_max(1,*power*3);
		//int32_t pow = *power;
			//*power = vbound((pow*3),0,214747);
		return -1;
		}
		
		case ed4x:
		{
			*power = zc_max(1,*power*4);
		//int32_t pow = *power;
			//*power = vbound((pow*4),0,214747);
		return -1;
		}
		
		
		case edHEAL:
		{ //Probably needs its own function, or  routine in the damage functuon to heal if power is negative. 
		//int32_t pow = *power;
			//*power = vbound((pow*-1),0,214747);
		//break;
			*power = zc_min(0,*power*-1);
			return -1;
		}
		/*
		case edLEVELDAMAGE: 
		{
		int32_t pow = *power;
		int32_t lvl  = *level;
			*power = vbound((pow*lvl),0,214747);
		break;
		}
		case edLEVELREDUCTION:
		{
		int32_t pow = *power;
		int32_t lvl  = *level;
			*power = vbound((pow/lvl),0,214747);
		break;
		}
		*/
		
		case edQUARTDAMAGE:
			*power = zc_max(1,*power/2);

			[[fallthrough]];
		case edHALFDAMAGE:
			*power = zc_max(1,*power/2);
			break;
		
		case edSWITCH:
		{
			if(Link.switchhookclk) return 0; //Already switching!
			switch(family)
			{
				case eeAQUA: case eeMOLD: case eeDONGO: case eeMANHAN: case eeGLEEOK:
				case eeDIG: case eeGHOMA: case eeLANM: case eePATRA: case eeGANON:
					return 0;
			}
			hooked_combopos = -1;
			hooked_layerbits = 0;
			switching_object = this;
			switch_hooked = true;
			Link.doSwitchHook(game->get_switchhookstyle());
			sfx(QMisc.miscsfx[sfxSWITCHED],int32_t(x));
			return 1;
		}
		
		case 0:
		{
			if(edef == edefSwitchHook)
				return -1;
			if (stunclk && get_bit(quest_rules, qr_NO_STUNLOCK) && *power == 0)
			{
				sfx(WAV_CHINK,pan(int32_t(x)));
				return 1;
			}
			
		}
	}
	
	return -1;
}


int32_t enemy::defenditemclassNew(int32_t wpnId, int32_t *power, weapon *w)
{
	int32_t wid = getWeaponID(w);

	int32_t edef = resolveEnemyDefence(w);
	if(QHeader.zelda_version > 0x250)
		return defendNew(wid, power,  edef, w->unblockable);
	switch(wid)
	{
		case wScript1: case wScript2: case wScript3: case wScript4: case wScript5:
		case wScript6: case wScript7: case wScript8: case wScript9: case wScript10:
			return defend(wpnId, power,  edefSCRIPT);

		case wWhistle:
			return -1;

		default:
			return defendNew(wid, power,  edef, w->unblockable);
	}
}


// Check defenses without actually acting on them.
bool enemy::candamage(int32_t power, int32_t edef, byte unblockable)
{
	switch(defense[edef])
	{
	case edSTUNONLY:
		return false;
	case edSTUNORCHINK:
	case edCHINK:
		return unblockable&WPNUNB_BLOCK;
	case edSTUNORIGNORE:
	case edIGNORE:
		return unblockable&WPNUNB_IGNR;
		
	case edIGNOREL1:
		return (unblockable&WPNUNB_IGNR) || power >= 1*game->get_hero_dmgmult();
	case edCHINKL1:
		return (unblockable&WPNUNB_BLOCK) || power >= 1*game->get_hero_dmgmult();
		
	case edCHINKL2:
		return (unblockable&WPNUNB_BLOCK) || power >= 2*game->get_hero_dmgmult();
		
	case edCHINKL4:
		return (unblockable&WPNUNB_BLOCK) || power >= 4*game->get_hero_dmgmult();
		
	case edCHINKL6:
		return (unblockable&WPNUNB_BLOCK) || power >= 6*game->get_hero_dmgmult();
		
	case edCHINKL8:
		return (unblockable&WPNUNB_BLOCK) || power >= 8*game->get_hero_dmgmult();
	}
	
	return true;
}

// Do we do damage?
// 0: takehit returns 0
// 1: takehit returns 1
// -1: do damage
int32_t enemy::defend(int32_t wpnId, int32_t *power, int32_t edef)
{
	if(shieldCanBlock)
	{
		switch(defense[edef])
		{
		case edIGNORE:
			return 0;
		case edIGNOREL1:
		case edSTUNORIGNORE:
			if(*power <= 0)
				return 0;
		}
		
		sfx(WAV_CHINK,pan(int32_t(x)));
		return 1;
	}
	
	switch(defense[edef])
	{
	case edSTUNORCHINK:
		if(*power <= 0)
		{
			sfx(WAV_CHINK,pan(int32_t(x)));
			return 1;
		}

		[[fallthrough]];
	case edSTUNORIGNORE:
		if(*power <= 0)
			return 0;

		[[fallthrough]];
	case edSTUNONLY:
		if((wpnId==wFire || wpnId==wBomb || wpnId==wSBomb || wpnId==wHookshot || wpnId==wSword) && stunclk>=159)
			return 1;
			
		stunclk=160;
		sfx(WAV_EHIT,pan(int32_t(x)));
		return 1;
	
	case edFREEZE:
		frozenclock=-1;
		//sfx(WAV_FREEZE,pan(int32_t(x)));
		return 1;
		
	case edCHINKL1:
		if(*power >= 1*game->get_hero_dmgmult()) break;
		[[fallthrough]];
	case edCHINKL2:
		if(*power >= 2*game->get_hero_dmgmult()) break;
		[[fallthrough]];
	case edCHINKL4:
		if(*power >= 4*game->get_hero_dmgmult()) break;
		[[fallthrough]];
	case edCHINKL6:
		if(*power >= 6*game->get_hero_dmgmult()) break;
		[[fallthrough]];
	case edCHINKL8:
		if(*power >= 8*game->get_hero_dmgmult()) break;
		[[fallthrough]];
	case edCHINKL10:
		if(*power >= 10*game->get_hero_dmgmult()) break;
		[[fallthrough]];
	case edCHINK:
		sfx(WAV_CHINK,pan(int32_t(x)));
		return 1;
	case edTRIGGERSECRETS:
		hidden_entrance(0, true, false, -4);
		break;
		
	case edIGNOREL1:
		if(*power > 0)  break;
		[[fallthrough]];
	case edIGNORE:
		return 0;
		
	case ed1HKO:
		*power = hp;
		return -2;
	
	case ed2x:
	 {
 	    *power = zc_max(1,*power*2);
 	//int32_t pow = *power;
		 //*power = vbound((pow*2),0,214747);
 	return -1; 
	 }
	 case ed3x:
	 {
 	    *power = zc_max(1,*power*3);
 	//int32_t pow = *power;
		 //*power = vbound((pow*3),0,214747);
 	return -1;
	 }
	 
	 case ed4x:
	 {
 	    *power = zc_max(1,*power*4);
 	//int32_t pow = *power;
		 //*power = vbound((pow*4),0,214747);
 	return -1;
	 }
	 
	 
	 case edHEAL:
	 { //Probably needs its own function, or  routine in the damage functuon to heal if power is negative. 
 	//int32_t pow = *power;
		 //*power = vbound((pow*-1),0,214747);
 	//break;
 	    *power = zc_min(0,*power*-1);
 	    return -1;
	 }
	 /*
	 case edLEVELDAMAGE: 
	 {
 	int32_t pow = *power;
 	int32_t lvl  = *level;
		 *power = vbound((pow*lvl),0,214747);
 	break;
	 }
	 case edLEVELREDUCTION:
	 {
 	int32_t pow = *power;
 	int32_t lvl  = *level;
		 *power = vbound((pow/lvl),0,214747);
 	break;
	 }
	 */
	 
		
	case edQUARTDAMAGE:
		*power = zc_max(1,*power/2);

		[[fallthrough]];
	case edHALFDAMAGE:
		*power = zc_max(1,*power/2);
		break;
	}
	
	return -1;
}

// Defend against a particular item class.
int32_t enemy::defenditemclass(int32_t wpnId, int32_t *power)
{
	int32_t def=-1;
	
	switch(wpnId)
	{
		// These first 2 are only used by Gohma... enemy::takehit() has complicated stun-calculation code for these.
	case wBrang:
		def = defend(wpnId, power, edefBRANG);
		break;
		
	case wHookshot:
		def = defend(wpnId, power, edefHOOKSHOT);
		break;
		
		// Anyway...
	case wBomb:
		def = defend(wpnId, power, edefBOMB);
		break;
		
	case wSBomb:
		def = defend(wpnId, power, edefSBOMB);
		break;
		
	case wArrow:
		def = defend(wpnId, power, edefARROW);
		break;
		
	case wFire:
		def = defend(wpnId, power, edefFIRE);
		break;
		
	case wWand:
		def = defend(wpnId, power, edefWAND);
		break;
		
	case wMagic:
		def = defend(wpnId, power, edefMAGIC);
		break;
		
	case wHammer:
		def = defend(wpnId, power, edefHAMMER);
		break;
		
	case wSword:
		def = defend(wpnId, power, edefSWORD);
		break;
		
	case wBeam:
		def = defend(wpnId, power, edefBEAM);
		break;
		
	case wRefBeam:
		def = defend(wpnId, power, edefREFBEAM);
		break;
		
	case wRefMagic:
		def = defend(wpnId, power, edefREFMAGIC);
		break;
		
	case wRefFireball:
		def = defend(wpnId, power, edefREFBALL);
		break;
		
	case wRefRock:
		def = defend(wpnId, power, edefREFROCK);
		break;
		
	case wStomp:
		def = defend(wpnId, power, edefSTOMP);
		break;
		
	case wCByrna:
		def = defend(wpnId, power, edefBYRNA);
		break;
		
	case wScript1:
		if(QHeader.zelda_version > 0x250) def = defend(wpnId, power,  edefSCRIPT01);
		else def = defend(wpnId, power,  edefSCRIPT);
		break;
	
	case wScript2:
		if(QHeader.zelda_version > 0x250) def = defend(wpnId, power,  edefSCRIPT02);
		else def = defend(wpnId, power,  edefSCRIPT);
		break;
	
	case wScript3:
		if(QHeader.zelda_version > 0x250) def = defend(wpnId, power,  edefSCRIPT03);
		else def = defend(wpnId, power,  edefSCRIPT);
		break;
	
	case wScript4:
		if(QHeader.zelda_version > 0x250) def = defend(wpnId, power,  edefSCRIPT04);
		else def = defend(wpnId, power,  edefSCRIPT);
		break;
	
	case wScript5:
		if(QHeader.zelda_version > 0x250) def = defend(wpnId, power,  edefSCRIPT05);
		else def = defend(wpnId, power,  edefSCRIPT);
		break;
	
	case wScript6:
		if(QHeader.zelda_version > 0x250) def = defend(wpnId, power,  edefSCRIPT06);
		else def = defend(wpnId, power,  edefSCRIPT);
		break;
	
	case wScript7:
		if(QHeader.zelda_version > 0x250) def = defend(wpnId, power,  edefSCRIPT07);
		else def = defend(wpnId, power,  edefSCRIPT);
		break;
	
	case wScript8:
		if(QHeader.zelda_version > 0x250) def = defend(wpnId, power,  edefSCRIPT08);
		else def = defend(wpnId, power,  edefSCRIPT);
		break;
	
	case wScript9:
		if(QHeader.zelda_version > 0x250) def = defend(wpnId, power,  edefSCRIPT09);
		else def = defend(wpnId, power,  edefSCRIPT);
		break;
	
	case wScript10:
		if(QHeader.zelda_version > 0x250) def = defend(wpnId, power,  edefSCRIPT10);
		else def = defend(wpnId, power,  edefSCRIPT);
		break;
	
	case wWhistle:
		if(QHeader.zelda_version > 0x250) def = defend(wpnId, power,  edefWhistle);
		else break;
		break;
	
	
	//!ZoriaRPG : We need some special cases here, to ensure that old script defs don;t break. 
	//Probably best to do this from the qest file, loading the values of Script(generic) into each
	//of the ten if the quest version is lower than N. 
	//Either that, or we need a boolean flag to set int32_t he enemy editor, or by ZScript that changes this behaviour. 
	//such as bool UseSeparatedScriptDefences. hah.
	default:
		//if(wpnId>=wScript1 && wpnId<=wScript10)
		// {
		 //   def = defend(wpnId, power, edefSCRIPT);
		// }
		// }
		
		break;
	}
	
	return def;
}

// take damage or ignore it
// -1: damage (if any) dealt
// 1: blocked
// 0: weapon passes through unhindered
int32_t enemy::takehit(weapon *w)
{
	if(fallclk||drownclk) return 0;
	int32_t wpnId = w->id;
	//al_trace("takehit() wpnId is %d\n",wpnId);
	//if ( wpnId == wWhistle ) al_trace("Whistle weapon in %s\n", "takehit");
	int32_t power = w->power;
	int32_t wpnx = w->x;
	int32_t wpny = w->y;
	int32_t enemyHitWeapon = w->parentitem;
	int32_t wpnDir;
	int32_t parent_item = w->parentitem;
	
	//if ( parent_item > -1 )
	//{
	//	if ( itemsbuf[parent_item].useweapon > 0 /*&& wpnId != wWhistle*/ )
	//	{
	//		wpnId = itemsbuf[parent_item].useweapon;
	//	}
		
	//}
	//if ( parent_item == -1 && w->ScriptGenerated )
	//{
	//	if ( w->useweapon > 0 /*&& wpnId != wWhistle*/ )
	//	{
	//		wpnId = w->useweapon;
	//	}
		
	//}
	//al_trace("takehit wpnId is: %d\n",wpnId);
   
	//Shoud be set from idata from the weapon::weaon constructor. -Z
	if ( w->useweapon > 0 /*&& wpnId != wWhistle*/ )
	{
		wpnId = w->useweapon;
	}
	
	//al_trace("takehit() useweapon is %d\n",itemsbuf[parent_item].useweapon);
	
	//Weapon Editor -Z
	
	
	// If it's a boomerang that just bounced, use the opposite direction;
	// otherwise, it might bypass a shield. This probably won't handle
	// every case correctly, but it's better than having shields simply
	// not work against boomerangs.
	if(w->id==wBrang && w->misc==1 && w->clk2>=256 && w->clk2<264)
		wpnDir = oppositeDir[w->dir];
	else
		wpnDir = w->dir;
		
	if(dying || clk<0 || hclk>0 || superman)
	{
		return 0;
	}
	
	//Prevent boomerang from writing to hitby[] for more than one frame.
	//This also prevents stunlock.
	//if ( stunclk > 0 ) return 0; 
	//this needs a rule for boomerangs that cannot stunlock!
	//further, bouncing weapons should probably SFX_CHINK and bounce here.
	//sigh.
	
	int32_t ret = -1;
	
	// This obscure quest rule...
	if(get_bit(quest_rules,qr_BOMBDARKNUTFIX) && (wpnId==wBomb || wpnId==wSBomb))
	{
		double ddir=atan2(double(wpny-y),double(x-wpnx));
		wpnDir=zc_oldrand()&3;
		
		if((ddir<=(((-1)*PI)/4))&&(ddir>(((-3)*PI)/4)))
		{
			wpnDir=down;
		}
		else if((ddir<=(((1)*PI)/4))&&(ddir>(((-1)*PI)/4)))
		{
			wpnDir=right;
		}
		else if((ddir<=(((3)*PI)/4))&&(ddir>(((1)*PI)/4)))
		{
			wpnDir=up;
		}
		else
		{
			wpnDir=left;
		}
	}
	
	int32_t xdir = dir;
	shieldCanBlock=false;
	
	//if (family==eeFLOAT && flags&(inv_front|inv_back_inv_left|inv_right)) xdir=down;
	if(!(w->unblockable&WPNUNB_BLOCK)&&((wpnId==wHookshot && hitshield(wpnx, wpny, xdir))
			|| ((flags&inv_front && wpnDir==(xdir^down)) || (flags&inv_back && wpnDir==(xdir^up)) || (flags&inv_left && wpnDir==(xdir^left)) || (flags&inv_right && wpnDir==(xdir^right))))
	  )
		// The hammer should already be dealt with by subclasses (Walker etc.)
	{
		switch(wpnId)
		{
			// Weapons which shields protect against
		case wSword:
		case wWand:
			if(Link.getCharging()>0)
				Link.setAttackClk(Link.getAttackClk()+1); //Cancel charging

			[[fallthrough]];
		case wHookshot:
		case wHSHandle:
		case wBrang:
			shieldCanBlock=true;
			break;
			
		case wBeam:
		case wRefBeam:
			// Mirror shielded enemies!
#if 0
			if(false /*flags2&guy_mirror*/ && !get_bit(quest_rules,qr_SWORDMIRROR))
			{
				if(wpnId>wEnemyWeapons)
					return 0;
					
				sfx(WAV_CHINK,pan(int32_t(x)));
				return 1;
			}
			
#endif

			[[fallthrough]];
		case wRefRock:
		case wRefFireball:
		case wMagic:
#if 0
			if(false /*flags2&guy_mirror*/ && (wpnId!=wRefRock || get_bit(quest_rules,qr_REFLECTROCKS)))
			{
				sfx(WAV_CHINK,pan(int32_t(x)));
				return 3;
			}
			
#endif
			
			if(wpnId>wEnemyWeapons)
				return 0;

			[[fallthrough]];
		default:
			shieldCanBlock=true;
			break;
			
			// Bombs
		case wSBomb:
		case wBomb:
			if (!get_bit(quest_rules,qr_TRUEFIXEDBOMBSHIELD)) goto hitclock;
			else if (!get_bit(quest_rules,qr_BOMBSPIERCESHIELD)) 
			{
				sfx(WAV_CHINK,pan(int32_t(x)));
				return 0;
			}
			else break;
			
			// Weapons which ignore shields
		case wWhistle:
		case wHammer:
			break;
			
			// Weapons which shouldn't be removed by shields
		case wLitBomb:
		case wLitSBomb:
		case wWind:
		case wPhantom:
		case wSSparkle:
		case wBait:
			return 0;

			[[fallthrough]];
		case wFire:
#if 0
			if(false /*flags2&guy_mirror*/)
			{
				sfx(WAV_CHINK,pan(int32_t(x)));
				return 1;
			}
			
#endif
			;
		}
	}
	
	switch(wpnId)
	{
	case wWhistle: //No longer completely ignore whistle weapons! -Z
	{
		
		if ( ((itemsbuf[parent_item].flags & ITEM_FLAG2) == 0) ||  ( parent_item == -1 )  )  //if the flag is set, or the weapon is scripted
		{
		//al_trace("Whistle weapon in %s\n", "takehit flag == 0");
		return 0; break;
		}
		else 
		{
		//al_trace("Whistle weapon in %s\n", "takehit flag != 0");
		w->power = itemsbuf[parent_item].misc5;
			
		//int32_t def = defendNew(wWhistle, &power,  resolveEnemyDefence(w));
		int32_t def = defendNew(wpnId, &power,  resolveEnemyDefence(w), w->unblockable);
		//int32_t def = defend(wWhistle, &power,  edefWhistle);
			
		//al_trace("whistle def is: %d\n", def);
		//al_trace("edefWhistle: %d\n", edefWhistle);
		//al_trace("whistle weapon defence resolution is %d\n", resolveEnemyDefence(w));
		//al_trace("Whistle Defence: %i\n", def);
		//al_trace("Whistle Damage Flag: %i\n", (itemsbuf[parent_item].flags & ITEM_FLAG2) ? 1 : 0);

		if(def <= 0) 
		{
			if ( def == -2 ) hp -= hp;
			else hp -= w->power;
			//al_trace("Whistle Defence: %i\n", def);
			return def;
		}
		break;
		}
		break;
	}
		
	case wPhantom:
		return 0;
		
	case wLitBomb:
	case wLitSBomb:
	case wBait:
	case wWind:
	case wSSparkle:
		return 0;
		
	case wFSparkle:
	
		// Only take sparkle damage if the sparkle's parent item is not
		// defended against.
		if(enemyHitWeapon > -1)
		{
			int32_t p = 0;
			int32_t f = itemsbuf[enemyHitWeapon].family;
			
			switch(f)
			{
			case itype_arrow:
				if(!candamage(p, edefARROW, w->unblockable)) return 0;
				
				break;
				
			case itype_cbyrna:
				if(!candamage(p, edefBYRNA, w->unblockable)) return 0;
				
				break;
				
			case itype_brang:
				if(!candamage(p, edefBRANG, w->unblockable)) return 0;
				
				break;
				
			default:
				return 0;
			}
		}
		
		wpnId = wSword;
		power = game->get_hero_dmgmult()>>1;
		goto fsparkle;
		break;
		
	case wBrang:
	{
		//int32_t def = defendNew(wpnId, &power, edefBRANG, w);
		int32_t def = defendNew(wpnId, &power,  resolveEnemyDefence(w), w->unblockable);
		//preventing stunlock might be best, here. -Z
		if(def >= 0) return def;
		
		// Not hurt by 0-damage weapons
		if(!(flags & guy_bhit))
		{
			stunclk=160;
			
			if(enemyHitWeapon>-1 ? itemsbuf[enemyHitWeapon].power : current_item_power(itype_brang))
			{
				hp -= (enemyHitWeapon>-1 ? itemsbuf[enemyHitWeapon].power : current_item_power(itype_brang))*game->get_hero_dmgmult();
				goto hitclock;
			}
			
			break;
		}
		
		if(!power)
			hp-=(enemyHitWeapon>-1 ? itemsbuf[enemyHitWeapon].fam_type : current_item(itype_brang))*game->get_hero_dmgmult();
		else
			hp-=power;
			
		goto hitclock;
	}
	
	case wHookshot:
	{
		//int32_t def = defendNew(wpnId, &power, edefHOOKSHOT,w);
		int32_t def = defendNew(wpnId, &power,  resolveEnemyDefence(w), w->unblockable);
		
		if(def >= 0) return def;
		
		if(!(flags & guy_bhit))
		{
			if(!switch_hooked && w->family_class != itype_switchhook)
				stunclk=160;
			
			if(enemyHitWeapon>-1 ? itemsbuf[enemyHitWeapon].power : current_item_power(itype_hookshot))
			{
				hp -= (enemyHitWeapon>-1 ? itemsbuf[enemyHitWeapon].power : current_item_power(itype_hookshot))*game->get_hero_dmgmult();
				goto hitclock;
			}
			
			break;
		}
		
		if(!power) hp-=(enemyHitWeapon>-1 ? itemsbuf[enemyHitWeapon].fam_type : current_item(itype_hookshot))*game->get_hero_dmgmult();
		else
			hp-=power;
			
		goto hitclock;
	}
	break;
	
	case wHSHandle:
	{
		if(itemsbuf[enemyHitWeapon>-1 ? enemyHitWeapon : current_item_id(itype_hookshot)].flags & ITEM_FLAG1)
			return 0;
			
		bool ignorehookshot = ((defense[edefHOOKSHOT] == edIGNORE) || ((defense[edefHOOKSHOT] == edIGNOREL1 || defense[edefHOOKSHOT] == edSTUNORIGNORE)
							   && (enemyHitWeapon>-1 ? itemsbuf[enemyHitWeapon].power : current_item_power(itype_hookshot)) <= 0));
							   
		// Peahats, Darknuts, Aquamentuses, Pols Voices, Wizzrobes, Manhandlas
		if(!(family==eePEAHAT || family==eeAQUA || family==eeMANHAN || (family==eeWIZZ && !ignorehookshot)
				|| (family==eeWALK && dmisc9==e9tPOLSVOICE) || (family==eeWALK && flags&(inv_back|inv_front|inv_left|inv_right))))
			return 0;
			
		power = game->get_hero_dmgmult();
	}
	
fsparkle:

	[[fallthrough]];
	default:
		// Work out the defenses!
	{
		int32_t def = defenditemclassNew(wpnId, &power, w); 
		
		if(def >= 0)
			return def;
		else if(def == -2)
		{
			ret = 0;
		}
	}
	
	if(!power)
	{
		if(flags & guy_bhit)
			hp-=1;
		else
		{
			// Don't make a long chain of 'stun' hits
			if((wpnId==wFire || wpnId==wBomb || wpnId==wSBomb || wpnId==wSword) && stunclk>0)
				return 1;
			
			
			if(!switch_hooked)
				stunclk=160;
			break;
		}
	}
	else hp-=power;
	
hitclock:
	hclk=33;
	
	// Use w->dir instead of wpnDir to make sure boomerangs don't push enemies the wrong way
	if((dir&2)==(w->dir&2))
	{
		sclk=(w->dir<<8)+16;
	}
	}
	
	if(((wpnId==wBrang) || (get_bit(quest_rules,qr_NOFLASHDEATH))) && (hp<=0 && !immortal))
	{
		fading=fade_blue_poof;
	}
	
   
	/*
	if( hitsfx > 0 ) //user set hit sound. 
	{
	if ( !dying ) //Don't play the hit sound when dying. 
		sfx(hitsfx, pan(int32_t(x)));
	}
	else sfx(WAV_EHIT, pan(int32_t(x))); //Don't play this one if the user sets a custom sound!
*/
/*
	if( hitsfx > 0 ) //A sound is set. 
	{
	if ( !dying ) //Don't play the hit sound when dying. 
		sfx(hitsfx, pan(int32_t(x)));
	}
*/
	 if ( FFCore.getQuestHeaderInfo(vZelda) > 0x250 || ( FFCore.getQuestHeaderInfo(vZelda) == 0x250 && FFCore.getQuestHeaderInfo(vBuild) > 31 )) //2.53 Gamma 2 and later
	{
		if( hitsfx > 0 ) //user-set hit sound. 
		{
			if (!dying) //don't play the hit sound on death! -Z
			sfx(hitsfx, pan(int32_t(x)));
		}
		else sfx(WAV_EHIT, pan(int32_t(x))); //Don't play the hardcoded sound if the user sets a custom one. 
	}
	else //2.50.2 or earlier
	{
	sfx(WAV_EHIT, pan(int32_t(x)));
	sfx(hitsfx, pan(int32_t(x)));
	}
	if(family==eeGUY)
		sfx(WAV_EDEAD, pan(int32_t(x)));
		
	// Penetrating weapons
	if((wpnId==wArrow || wpnId==wBeam) && !cannotpenetrate())
	{
		int32_t item=enemyHitWeapon;
		
		if(wpnId==wArrow)
		{
			//If we use an arrow type for the item's Weapon type, the flags differ, so we need to rely on the flags from an arrow class. 
			if(item>=0 && (itemsbuf[item].flags&ITEM_FLAG1) && (itemsbuf[parent_item].family == itype_arrow))
				return 0;
			else if(get_bit(quest_rules,qr_ARROWS_ALWAYS_PENETRATE)) return 0;
			//if(item<0)
			else
				item=current_item_id(itype_arrow);	
		}
		
		else
		{

			//If we use an swordbeam type for the item's Weapon type, the flags differ, so we need to rely on the flags from an arrow class. 
			if(item>=0 && (itemsbuf[item].flags&ITEM_FLAG3) && (itemsbuf[parent_item].family == itype_sword))
				return 0;
			
			else if(get_bit(quest_rules,qr_SWORDBEAMS_ALWAYS_PENETRATE)) return 0;
			else
			//if(item<0)
				item=current_item_id(itype_sword);
		}
	}
	
	return ret;
}

bool enemy::dont_draw()
{
	if(fading==fade_invisible || (((flags2&guy_blinking)||(fading==fade_flicker)) && (clk&1)))
		return true;
		
	if(flags&guy_invisible)
		return true;
		
	if(flags&lens_only && !lensclk)
		return true;
		
	if(lensclk && (itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG6) && !(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG7) &&
	!((flags&lens_only) && (get_bit(quest_rules,qr_LENSSEESENEMIES) || (itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG5))))
		return true;
		
	return false;
}

#define DRAW_NORMAL 2
#define DRAW_CLOAKED 1
#define DRAW_INVIS 0
// base drawing function to be used by all derived classes instead of
// sprite::draw()
void enemy::draw(BITMAP *dest)
{
	if(fading==fade_invisible || (((flags2&guy_blinking)||(fading==fade_flicker)) && (clk&1))) 
		return;
	if(flags&guy_invisible)
		return;
	if(lensclk && (itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG6) && !(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG7) && !(flags&lens_only))
		return;
	
	//We did the normal don't_draw stuff here so we can make exceptions; specifically the lens check (which should make enemies
	// be cloaked if they have "invisible displays as cloaked" checked.
	
	byte canSee = DRAW_NORMAL;
	//Enemy specific stuff
	if ( editorflags & ENEMY_FLAG1 )
	{
		canSee = DRAW_INVIS;
		if (editorflags & ENEMY_FLAG4) canSee = DRAW_CLOAKED;
		if (dmisc13 >= 0 && (editorflags & ENEMY_FLAG2))
		{
			if (game->item[dmisc13])
			{
				canSee = DRAW_NORMAL;
			}
			//else if ( lensclk && getlensid.flags SHOWINVIS )
			//{
			//
			//}
			//else
			//{
			//	if ( (editorflags & ENEMY_FLAG4) ) canSee = DRAW_CLOAKED;
			//	//otherwisem invisible
			//}
		}	
	}
	//Room specific
	if (tmpscr->flags3&fINVISROOM)
	{
		if (canSee == DRAW_NORMAL && !(current_item(itype_amulet)) && 
		!((itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG5) && lensclk) && family!=eeGANON) canSee = DRAW_CLOAKED;
	}
	//Lens check
	if (lensclk)
	{
		if((itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG6) && !(flags&lens_only))
		{
			if (canSee == DRAW_NORMAL) 
			{
				if (itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG7) canSee = DRAW_CLOAKED;
				else canSee = DRAW_INVIS; //Should never happen cause dont_draw should catch this, but just in case.
			}
		}
		if(flags&lens_only)
		{
			if (canSee == DRAW_INVIS) canSee = DRAW_NORMAL;
		}
	}
	else
	{
		if(flags&lens_only)
			canSee = DRAW_INVIS;
	}
	if (canSee == DRAW_INVIS && (editorflags & ENEMY_FLAG4)) canSee = DRAW_CLOAKED;
	if (canSee == DRAW_NORMAL && (editorflags & ENEMY_FLAG16)) canSee = DRAW_CLOAKED;
	
	if (canSee == DRAW_INVIS)
		return;
	
	if(fallclk||drownclk)
	{
		if (canSee == DRAW_CLOAKED)
		{
			sprite::drawcloaked(dest);
		}
		else if (canSee == DRAW_NORMAL)
		{
			sprite::draw(dest);
		}
		return;
	}
	int32_t cshold=cs;
	
	if(dying)
	{
		if(clk2>=19)
		{
			if(!(clk2&2))
			{
				//if the enemy isn't totally invisible, or if it is, but Link has the item needed to reveal it, draw it.
				if (canSee == DRAW_CLOAKED)
				{
					sprite::drawcloaked(dest);
				}
				else if (canSee == DRAW_NORMAL)
				{
					sprite::draw(dest);
				}
			}	
			return;
		}
		
		flip = 0;
		tile = wpnsbuf[spr_death].newtile;
		//The scale of this tile shouldx be based on the enemy size. -Z
		if ( do_animation ) 
		{
			if(!get_bit(quest_rules,qr_HARDCODED_ENEMY_ANIMS))
			{
				if(clk2 > 2)
				{
					spr_death_anim_clk=0;
					clk2=1;
					death_sfx();
				}
				if(clk2==1 && spr_death_anim_clk>-1)
				{
					++clk2;
					spr_death_anim_frm=(spr_death_anim_clk/zc_max(wpnsbuf[spr_death].speed,1));
					if(++spr_death_anim_clk >= (zc_max(wpnsbuf[spr_death].speed,1) * zc_max(wpnsbuf[spr_death].frames,1)))
					{
						spr_death_anim_clk=-1;
						clk2=1;
					}
				}
				tile += spr_death_anim_frm;
			}
			else if(BSZ)
				tile += zc_min((15-clk2)/3,4);
			else if(clk2>6 && clk2<=12)
				++tile;
		}
		
		if(!get_bit(quest_rules,qr_HARDCODED_ENEMY_ANIMS) || BSZ || fading==fade_blue_poof)
			cs = wpnsbuf[spr_death].csets&15;
		else
			cs = (((clk2+5)>>1)&3)+6;
	}
	else if(hclk>0)
	{
		if(family==eeGANON)
			cs=(((hclk-1)>>1)&3)+6;
		else if(hclk<33 && !get_bit(quest_rules,qr_ENEMIESFLICKER))
			cs=(((hclk-1)>>1)&3)+6;
	}
	//draw every other frame for flickering enemies
	if((frame&1)==1 || !(family !=eeGANON && hclk>0 && get_bit(quest_rules,qr_ENEMIESFLICKER)))
	{
		if (canSee == DRAW_CLOAKED)
		{
			sprite::drawcloaked(dest);
		}
		else if (canSee == DRAW_NORMAL)
		{
			if ( frozenclock < 0 )
			{
				if ( frozentile > 0 ) tile = frozentile;
				loadpalset(csBOSS,frozencset);
			}
			sprite::draw(dest);
		}
	}
	cs=cshold;
}

//old zc bosses
void enemy::drawzcboss(BITMAP *dest)
{
	if(dont_draw())
		return;
		
	int32_t cshold=cs;
	
	if(dying)
	{
		if(clk2>=19)
		{
			if(!(clk2&2))
				sprite::drawzcboss(dest);
				
			return;
		}
		
		flip = 0;
		tile = wpnsbuf[spr_death].newtile;
		
		if ( do_animation ) 
		{
			if(!get_bit(quest_rules,qr_HARDCODED_ENEMY_ANIMS))
			{
				if(clk2 > 2)
				{
					spr_death_anim_clk=0;
					clk2=1;
					death_sfx();
				}
				if(clk2==1 && spr_death_anim_clk>-1)
				{
					++clk2;
					spr_death_anim_frm=(spr_death_anim_clk/zc_max(wpnsbuf[spr_death].speed,1));
					if(++spr_death_anim_clk >= (zc_max(wpnsbuf[spr_death].speed,1) * zc_max(wpnsbuf[spr_death].frames,1)))
					{
						spr_death_anim_clk=-1;
						clk2=1;
					}
				}
				tile += spr_death_anim_frm;
			}
			else if(BSZ)
				tile += zc_min((15-clk2)/3,4);
			else if(clk2>6 && clk2<=12)
				++tile;
		}
		
		if(!get_bit(quest_rules,qr_HARDCODED_ENEMY_ANIMS) || BSZ || fading==fade_blue_poof)
			cs = wpnsbuf[spr_death].csets&15;
		else
			cs = (((clk2+5)>>1)&3)+6;
	}
	else if(hclk>0)
	{
		if(family==eeGANON)
			cs=(((hclk-1)>>1)&3)+6;
		else if(hclk<33 && !get_bit(quest_rules,qr_ENEMIESFLICKER))
			cs=(((hclk-1)>>1)&3)+6;
	}
	
	if((tmpscr->flags3&fINVISROOM) &&
			!(current_item(itype_amulet)) &&
			!(get_bit(quest_rules,qr_LENSSEESENEMIES) &&
			  lensclk) && family!=eeGANON)
	{
		sprite::drawcloaked(dest);
	}
	else
	{
		if(family !=eeGANON && hclk>0 && get_bit(quest_rules,qr_ENEMIESFLICKER))
		{
			if((frame&1)==1)
				sprite::drawzcboss(dest);
		}
		else
			sprite::drawzcboss(dest);
	}
	
	cs=cshold;
}


// similar to the overblock function--can do up to a 32x32 sprite
//will this play nicely with scripttile, solely using the modifications in sprite::draw()?
void enemy::drawblock(BITMAP *dest,int32_t mask)
{
	int32_t thold=tile;
	int32_t t1=tile;
	int32_t t2=tile+20;
	int32_t t3=tile+1;
	int32_t t4=tile+21;
	
	switch(mask)
	{
	case 1:
		enemy::drawzcboss(dest);
		break;
		
	case 3:
		if(flip&2)
			zc_swap(t1,t2);
			
		tile=t1;
		enemy::drawzcboss(dest);
		tile=t2;
		yofs+=16;
		enemy::drawzcboss(dest);
		yofs-=16;
		break;
		
	case 5:
		t2=tile+1;
		
		if(flip&1)
			zc_swap(t1,t2);
			
		tile=t1;
		enemy::drawzcboss(dest);
		tile=t2;
		xofs+=16;
		enemy::drawzcboss(dest);
		xofs-=16;
		break;
		
	case 15:
		if(flip&1)
		{
			zc_swap(t1,t3);
			zc_swap(t2,t4);
		}
		
		if(flip&2)
		{
			zc_swap(t1,t2);
			zc_swap(t3,t4);
		}
		
		tile=t1;
		enemy::drawzcboss(dest);
		tile=t2;
		yofs+=16;
		enemy::drawzcboss(dest);
		yofs-=16;
		tile=t3;
		xofs+=16;
		enemy::drawzcboss(dest);
		tile=t4;
		yofs+=16;
		enemy::drawzcboss(dest);
		xofs-=16;
		yofs-=16;
		break;
	}
	
	tile=thold;
}

void enemy::drawshadow(BITMAP *dest, bool translucent)
{
	if(dont_draw() || isSideViewGravity())
	{
		return;
	}
	
	if(dying)
	{
		return;
	}
	
	if(((tmpscr->flags3&fINVISROOM)&& !(current_item(itype_amulet)))||
			(darkroom))
	{
		return;
	}
	else
	{
		/*   if (enemycanfall(id) && z>0)
			 shadowtile = wpnsbuf[spr_shadow].tile;
		   sprite::drawshadow(dest,translucent);
		   if (z==0)
			 shadowtile = 0;*/
		// a bad idea, as enemies do their own setting of the shadow tile (since some use the
		// 2x2 tiles, shadows animate, etc.) -DD
		
		//this hack is in place as not all enemies that should use the z axis while in the air
		//(ie rocks, boulders) actually do. To be removed when the enemy revamp is complete -DD
		if(enemycanfall(id) && shadowtile == 0)
			shadowtile = wpnsbuf[spr_shadow].newtile;
			
		if(z>0 || !enemycanfall(id))
		{
			if(!shadow_overpit(this))
			sprite::drawshadow(dest,translucent);
		}
	}
}

void enemy::masked_draw(BITMAP *dest,int32_t mx,int32_t my,int32_t mw,int32_t mh)
{
	BITMAP *sub=create_sub_bitmap(dest,mx,my,mw,mh);
	
	if(sub!=NULL)
	{
		xofs-=mx;
		yofs-=my;
		enemy::draw(sub);
		xofs+=mx;
		yofs+=my;
		destroy_bitmap(sub);
	}
	else
		enemy::draw(dest);
}

// override hit detection to check for invicibility, stunned, etc
bool enemy::hit(sprite *s)
{
	if(!(s->scriptcoldet&1) || s->fallclk || s->drownclk) return false;
	
	return (dying || hclk>0) ? false : sprite::hit(s);
}

bool enemy::hit(int32_t tx,int32_t ty,int32_t tz,int32_t txsz2,int32_t tysz2,int32_t tzsz2)
{
	return (dying || hclk>0) ? false : sprite::hit(tx,ty,tz,txsz2,tysz2,tzsz2);
}

bool enemy::hit(weapon *w)
{
	if(!(w->scriptcoldet&1) || w->fallclk || w->drownclk) return false;
	
	return (dying || hclk>0) ? false : sprite::hit(w);
}

bool enemy::can_pitfall(bool checkspawning)
{
	if((fading||isspawning)&&checkspawning) return false; //Don't fall during spawn.
	switch(guysbuf[id&0xFFF].family)
	{
		case eeAQUA:
		case eeDIG:
		case eeDONGO:
		case eeFAIRY:
		case eeGANON:
		case eeGHOMA:
		case eeGLEEOK:
		case eeGUY:
		case eeLANM:
		case eeMANHAN:
		case eeMOLD:
		case eeNONE:
		case eePATRA:
		case eeZORA:
			return false; //Disallowed types
		default:
			return true;
	}
}
//Handle death
void enemy::try_death(bool force_kill)
{
	if(!dying && (force_kill || (hp<=0 && !immortal)))
	{
		if(itemguy && (hasitem&2)!=0)
		{
			for(int32_t i=0; i<items.Count(); i++)
			{
				if(((item*)items.spr(i))->pickup&ipENEMY)
				{
					items.spr(i)->x = x;
					items.spr(i)->y = y - 2;
				}
			}
		}
		
		dying=true;
		
		if(fading==fade_flash_die)
			clk2=19+18*4;
		else
		{
			clk2 = BSZ ? 15 : 19;
			
			if(fading!=fade_blue_poof)
				fading=0;
		}
		
		if(itemguy)
		{
			hasitem&=~2;
			item_set=0;
		}
		
		if(currscr<128 && count_enemy && !script_spawned)
			game->guys[(currmap<<7)+currscr]-=1;
	}
}

//                         --==**==--

//   Movement routines that can be used by derived classes as needed

//                         --==**==--

void enemy::fix_coords(bool bound)
{
	if ((get_bit(quest_rules,qr_OUTOFBOUNDSENEMIES) ? 1 : 0) ^ ((editorflags&ENEMY_FLAG11)?1:0)) return;
	
	
	
	if(bound)
	{
		if ( ((unsigned)(id&0xFFF)) < MAXGUYS )
		{
		x=vbound(x, 0, (( guysbuf[id].SIZEflags&guyflagOVERRIDE_TILE_WIDTH && !isflier(id) ) ? (256-((txsz-1)*16)) : 240));
		y=vbound(y, 0,(( guysbuf[id].SIZEflags&guyflagOVERRIDE_TILE_HEIGHT && !isflier(id) ) ? (176-((txsz-1)*16)) : 160));
		}
		else
		{
		   x=vbound(x, 0,240); 
			y=vbound(y, 0,160);
		}
	}
	
	if(!OUTOFBOUNDS)
	{
		/*x=((int32_t(x)&0xF0)+((int32_t(x)&8)?16:0));
		
		if(isSideViewGravity())
			y=((int32_t(y)&0xF8)+((int32_t(y)&4)?8:0));
		else
			y=((int32_t(y)&0xF0)+((int32_t(y)&8)?16:0));
		*/
		do_fix(x, 16, true);
		if(isSideViewGravity())
			do_fix(y,8,true);
		else do_fix(y,16,true);
	}
}
bool enemy::cannotpenetrate()
{
	return (family == eeAQUA || family == eeMANHAN || family == eeGHOMA);
}

bool enemy::canmove_old(int32_t ndir,zfix s,int32_t special,int32_t dx1,int32_t dy1,int32_t dx2,int32_t dy2)
{
	bool ok;
	int32_t dx = 0, dy = 0;
	int32_t sv = 8;
	
	//Why is this here??? Why is it needed???
	s += 0.5; // Make the ints round; doesn't seem to cause any problems.
	
	switch(ndir)
	{
	case 8:
	case up:
		if(canfall(id) && isSideViewGravity())
			return false;
			
		dy = dy1-s;
		special = (special==spw_clipbottomright)?spw_none:special;
		ok = !m_walkflag_old(x,y+dy,special, x, y) && !flyerblocked(x,y+dy, special);
		break;
		
	case 12:
	case down:
		if(canfall(id) && isSideViewGravity())
			return false;
			
		dy = dy2+s;
		ok = !m_walkflag_old(x,y+dy,special, x, y) && !flyerblocked(x,y+dy, special);
		break;
		
	case 14:
	case left:
		dx = dx1-s;
		sv = ((isSideViewGravity())?7:8);
		special = (special==spw_clipbottomright||special==spw_clipright)?spw_none:special;
		ok = !m_walkflag_old(x+dx,y+sv,special, x, y) && !flyerblocked(x+dx,y+8, special);
		break;
		
	case 10:
	case right:
		dx = dx2+s;
		sv = ((isSideViewGravity())?7:8);
		ok = !m_walkflag_old(x+dx,y+sv,special, x, y) && !flyerblocked(x+dx,y+8, special);
		break;
		
	case 9:
	case r_up:
		dx = dx2+s;
		dy = dy1-s;
		ok = !m_walkflag_old(x,y+dy,special, x, y) && !m_walkflag_old(x+dx,y+sv,special, x, y) &&
			 !flyerblocked(x,y+dy, special) && !flyerblocked(x+dx,y+8, special);
		break;
		
	case 11:
	case r_down:
		dx = dx2+s;
		dx = dy2+s;
		ok = !m_walkflag_old(x,y+dy,special, x, y) && !m_walkflag_old(x+dx,y+sv,special, x, y) &&
			 !flyerblocked(x,y+dy, special) && !flyerblocked(x+dx,y+8, special);
		break;
		
	case 13:
	case l_down:
		dx = dx1-s;
		dy = dy2+s;
		ok = !m_walkflag_old(x,y+dy,special, x, y) && !m_walkflag_old(x+dx,y+sv,special, x, y) &&
			 !flyerblocked(x,y+dy, special) && !flyerblocked(x+dx,y+8, special);
		break;
		
	case 15:
	case l_up:
		dx = dx1-s;
		dy = dy1-s;
		ok = !m_walkflag_old(x,y+dy,special, x, y) && !m_walkflag_old(x+dx,y+sv,special, x, y) &&
			 !flyerblocked(x,y+dy, special) && !flyerblocked(x+dx,y+8, special);
		break;
		
	default:
		db=99;
		return true;
	}
	
	return ok;
}




// returns true if next step is ok, false if there is something there
bool enemy::canmove(int32_t ndir,zfix s,int32_t special,int32_t dx1,int32_t dy1,int32_t dx2,int32_t dy2, bool kb)
{
	bool ok = false; //initialise the var, son't just declare it
	int32_t dx = 0, dy = 0;
	int32_t sv = 8;
	int32_t tries = 2; int32_t try_x = 0; int32_t try_y = 0;
	//Why is this here??? Why is it needed???
	s += 0.5; // Make the ints round; doesn't seem to cause any problems.
	int32_t usexoffs = (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) ? hxofs : 0;
	int32_t useyoffs = (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) ? hyofs : 0;
	int32_t usewid = (SIZEflags&guyflagOVERRIDE_HIT_WIDTH) ? hxsz : 16;
	int32_t usehei = (SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) ? hysz : 16;
	bool offgrid = OFFGRID_ENEMY;
	if(!offgrid)
	{
		//Enemies smaller than 1-tile must act as 1-tile large, if off-grid movement is disabled.
		if(usehei<16)usehei=16;
		if(usewid<16)usewid=16;
	}
	switch(ndir) //need to check every 8 pixels between two points
	{
		case 8:
		case up:
		{
			if(enemycanfall(id) && isSideViewGravity())
				return false;
				
			dy = dy1-s;
			special = (special==spw_clipbottomright)?spw_none:special;
			tries = usewid/(offgrid ? 8 : 16);
			//Z_eventlog("Trying move UP, dy=%d,usewid=%d,usehei=%d\n",int32_t(dy),usewid,usehei);
			for ( ; tries > 0; --tries )
			{
				ok = !m_walkflag(x+usexoffs+try_x,y+useyoffs+dy,special, ndir, x+usexoffs+try_x, y+useyoffs, kb) && !flyerblocked(x+usexoffs+try_x,y+useyoffs+dy, special,kb);
				try_x += (offgrid ? 8 : 16);
				if (!ok) break;
			}
			if(!ok) break;
			if((usewid%16)>0) //Uneven width
			{
				ok = !m_walkflag(x+usexoffs+usewid-1,y+useyoffs+dy,special, ndir, x+usexoffs+usewid-1, y+useyoffs, kb) && !flyerblocked(x+usexoffs+usewid-1,y+useyoffs+dy, special,kb);
			}
			break;
		}
		case 12:
		case down:
		{
			if(enemycanfall(id) && isSideViewGravity())
				return false;
				
			dy = dy2+s;
			tries = usewid/(offgrid ? 8 : 16);
			//Z_eventlog("Trying move DOWN, dy=%d,usewid=%d,usehei=%d\n",int32_t(dy),usewid,usehei);
			for ( ; tries > 0; --tries )
			{
				ok = !m_walkflag(x+usexoffs+try_x,y+useyoffs+dy,special, ndir, x+usexoffs+try_x, y+useyoffs, kb) && !flyerblocked(x+usexoffs+try_x,y+useyoffs+dy+zc_max(usehei-16,0), special,kb);
				try_x += (offgrid ? 8 : 16);
				if (!ok) break;
			}
			if(!ok) break;
			if((usewid%16)>0) //Uneven width
			{
				ok = !m_walkflag(x+usexoffs+usewid-1,y+useyoffs+dy,special, ndir, x+usexoffs+usewid-1, y+useyoffs, kb) && !flyerblocked(x+usexoffs+usewid-1,y+useyoffs+dy+zc_max(usehei-16,0), special,kb);
			}
			break;
		}
		case 14:
		case left:
		{
			dx = dx1-s;
			sv = ((isSideViewGravity())?7:0);
			special = (special==spw_clipbottomright||special==spw_clipright)?spw_none:special;
			tries = usehei/(offgrid ? 8 : 16);
			//Z_eventlog("Trying move LEFT, dx=%d,usewid=%d,usehei=%d\n",int32_t(dx),usewid,usehei);
			for ( ; tries > 0; --tries )
			{
				ok = !m_walkflag(x+usexoffs+dx,y+useyoffs+try_y+sv,special, ndir, x+usexoffs, y+useyoffs+try_y, kb) && !flyerblocked(x+usexoffs+dx,y+8+useyoffs+try_y, special,kb);
				try_y += (offgrid ? 8 : 16);
				if (!ok) break;
			}
			if(!ok) break;
			if((usehei%16)>0) //Uneven height
			{
				ok = !m_walkflag(x+usexoffs+dx,y+useyoffs+usehei-1+sv,special, ndir, x+usexoffs, y+useyoffs+usehei-1, kb) && !flyerblocked(x+usexoffs+dx,y+8+useyoffs+usehei-1, special,kb);
			}
			break;
		}
		case 10:
		case right:
		{
			dx = dx2+s;
			sv = ((isSideViewGravity())?7:0);
			tries = usehei/(offgrid ? 8 : 16);
			//Z_eventlog("Trying move RIGHT, dx=%d,usewid=%d,usehei=%d\n",int32_t(dx),usewid,usehei);
			for ( ; tries > 0; --tries )
			{
				ok = !m_walkflag(x+usexoffs+dx,y+useyoffs+try_y+sv,special, ndir, x+usexoffs, y+useyoffs+try_y, kb) && !flyerblocked(x+usexoffs+dx+zc_max(usewid-16,0),y+8+useyoffs+try_y, special,kb);
				try_y += (offgrid ? 8 : 16);
				if (!ok) break;
			}
			if(!ok) break;
			if((usehei%16)>0) //Uneven height
			{
				ok = !m_walkflag(x+usexoffs+dx,y+useyoffs+usehei-1+sv,special, ndir, x+usexoffs, y+useyoffs+usehei-1, kb) && !flyerblocked(x+usexoffs+dx+zc_max(usewid-16,0),y+8+useyoffs+usehei-1, special,kb);
			}
			break;
		}
		case 9:
		case r_up:
		{
			dx = dx2+s;
			dy = dy1-s;
			int32_t tries_x = usewid/(offgrid ? 8 : 16);
			sv = ((isSideViewGravity())?7:0);
			for ( ; tries_x > 0; --tries_x )
			{
				int32_t tries_y = usehei/(offgrid ? 8 : 16);
				try_y = 0;
				for ( ; tries_y > 0; --tries_y )
				{
					ok = !m_walkflag(x+usexoffs+try_x,y+useyoffs+dy+try_y,special,ndir, x+usexoffs+try_x, y+useyoffs+try_y, kb) && !m_walkflag(x+usexoffs+dx+try_x,y+useyoffs+sv+try_y,special,ndir, x+usexoffs+try_x, y+useyoffs+try_y, kb) &&
					!flyerblocked(x+usexoffs+try_x,y+useyoffs+dy+try_y, special,kb) && !flyerblocked(x+usexoffs+dx+try_x,y+useyoffs+8+try_y, special,kb);
					try_y += (offgrid ? 8 : 16);
					if (!ok) break;
				}
				if (!ok) break;
				if((usehei%16)>0) //Uneven height
				{
					ok = !m_walkflag(x+usexoffs+try_x,y+useyoffs+dy+usehei-1,special,ndir, x+usexoffs+try_x, y+useyoffs+usehei-1, kb) && !m_walkflag(x+usexoffs+dx+try_x,y+useyoffs+sv+usehei-1,special,ndir, x+usexoffs+try_x, y+useyoffs+usehei-1, kb) &&
						 !flyerblocked(x+usexoffs+try_x,y+useyoffs+dy+usehei-1, special,kb) && !flyerblocked(x+usexoffs+dx+try_x,y+useyoffs+8+usehei-1, special,kb);
				}
				try_x += (offgrid ? 8 : 16);
			}
			if(!ok) break;
			if((usewid%16)>0) //Uneven width
			{
				int32_t tries_y = usehei/(offgrid ? 8 : 16);
				try_y = 0;
				for ( ; tries_y > 0; --tries_y )
				{
					ok = !m_walkflag(x+usexoffs+usewid-1,y+useyoffs+dy+try_y,special,ndir, x+usexoffs+usewid-1, y+useyoffs+try_y, kb) && !m_walkflag(x+usexoffs+dx+usewid-1,y+useyoffs+sv+try_y,special,ndir, x+usexoffs+usewid-1, y+useyoffs+try_y, kb) &&
					!flyerblocked(x+usexoffs+usewid-1,y+useyoffs+dy+try_y, special,kb) && !flyerblocked(x+usexoffs+dx+usewid-1,y+useyoffs+8+try_y, special,kb);
					try_y += (offgrid ? 8 : 16);
					if (!ok) break;
				}
				if (!ok) break;
				if((usehei%16)>0) //Uneven height
				{
					ok = !m_walkflag(x+usexoffs+usewid-1,y+useyoffs+dy+usehei-1,special,ndir, x+usexoffs+usewid-1, y+useyoffs+usehei-1, kb) && !m_walkflag(x+usexoffs+dx+usewid-1,y+useyoffs+sv+usehei-1,special,ndir, x+usexoffs+usewid-1, y+useyoffs+usehei-1, kb) &&
						 !flyerblocked(x+usexoffs+usewid-1,y+useyoffs+dy+usehei-1, special,kb) && !flyerblocked(x+usexoffs+dx+usewid-1,y+useyoffs+8+usehei-1, special,kb);
				}
			}
			break;
		}
		case 11:
		case r_down:
		{
			dx = dx2+s;
			dx = dy2+s;
			int32_t tries_x = usewid/(offgrid ? 8 : 16);
			//sv = ((isSideViewGravity())?7:0);
			for ( ; tries_x > 0; --tries_x )
			{
				int32_t tries_y = usehei/(offgrid ? 8 : 16);
				try_y = 0;
				for ( ; tries_y > 0; --tries_y )
				{
					ok = !m_walkflag(x+usexoffs+try_x,y+useyoffs+dy+try_y,special,ndir, x+usexoffs+try_x, y+useyoffs+try_y, kb) && !m_walkflag(x+usexoffs+dx+try_x,y+useyoffs+sv+try_y,special,ndir, x+usexoffs+try_x, y+useyoffs+try_y, kb) &&
					!flyerblocked(x+usexoffs+try_x,y+useyoffs+dy+try_y, special,kb) && !flyerblocked(x+usexoffs+dx+try_x,y+useyoffs+8+try_y, special,kb);
					try_y += (offgrid ? 8 : 16);
					if (!ok) break;
				}
				if (!ok) break;
				if((usehei%16)>0) //Uneven height
				{
					ok = !m_walkflag(x+usexoffs+try_x,y+useyoffs+dy+usehei-1,special,ndir, x+usexoffs+try_x, y+useyoffs+usehei-1, kb) && !m_walkflag(x+usexoffs+dx+try_x,y+useyoffs+sv+usehei-1,special,ndir, x+usexoffs+try_x, y+useyoffs+usehei-1, kb) &&
						 !flyerblocked(x+usexoffs+try_x,y+useyoffs+dy+usehei-1, special,kb) && !flyerblocked(x+usexoffs+dx+try_x,y+useyoffs+8+usehei-1, special,kb);
				}
				try_x += (offgrid ? 8 : 16);
			}
			if(!ok) break;
			if((usewid%16)>0) //Uneven width
			{
				int32_t tries_y = usehei/(offgrid ? 8 : 16);
				try_y = 0;
				for ( ; tries_y > 0; --tries_y )
				{
					ok = !m_walkflag(x+usexoffs+usewid-1,y+useyoffs+dy+try_y,special,ndir, x+usexoffs+usewid-1, y+useyoffs+try_y, kb) && !m_walkflag(x+usexoffs+dx+usewid-1,y+useyoffs+sv+try_y,special,ndir, x+usexoffs+usewid-1, y+useyoffs+try_y, kb) &&
					!flyerblocked(x+usexoffs+usewid-1,y+useyoffs+dy+try_y, special,kb) && !flyerblocked(x+usexoffs+dx+usewid-1,y+useyoffs+8+try_y, special,kb);
					try_y += (offgrid ? 8 : 16);
					if (!ok) break;
				}
				if (!ok) break;
				if((usehei%16)>0) //Uneven height
				{
					ok = !m_walkflag(x+usexoffs+usewid-1,y+useyoffs+dy+usehei-1,special,ndir, x+usexoffs+usewid-1, y+useyoffs+usehei-1, kb) && !m_walkflag(x+usexoffs+dx+usewid-1,y+useyoffs+sv+usehei-1,special,ndir, x+usexoffs+usewid-1, y+useyoffs+usehei-1, kb) &&
						 !flyerblocked(x+usexoffs+usewid-1,y+useyoffs+dy+usehei-1, special,kb) && !flyerblocked(x+usexoffs+dx+usewid-1,y+useyoffs+8+usehei-1, special,kb);
				}
			}
			break;
		}
		case 13:
		case l_down:
		{
			dx = dx1-s;
			dy = dy2+s;
			int32_t tries_x = usewid/(offgrid ? 8 : 16);
			//sv = ((isSideViewGravity())?7:0);
			for ( ; tries_x > 0; --tries_x )
			{
				int32_t tries_y = usehei/(offgrid ? 8 : 16);
				try_y = 0;
				for ( ; tries_y > 0; --tries_y )
				{
					ok = !m_walkflag(x+usexoffs+try_x,y+useyoffs+dy+try_y,special,ndir, x+usexoffs+try_x, y+useyoffs+try_y, kb) && !m_walkflag(x+usexoffs+dx+try_x,y+useyoffs+sv+try_y,special,ndir, x+usexoffs+try_x, y+useyoffs+try_y, kb) &&
					!flyerblocked(x+usexoffs+try_x,y+useyoffs+dy+try_y, special,kb) && !flyerblocked(x+usexoffs+dx+try_x,y+useyoffs+8+try_y, special,kb);
					try_y += (offgrid ? 8 : 16);
					if (!ok) break;
				}
				if (!ok) break;
				if((usehei%16)>0) //Uneven height
				{
					ok = !m_walkflag(x+usexoffs+try_x,y+useyoffs+dy+usehei-1,special,ndir, x+usexoffs+try_x, y+useyoffs+usehei-1, kb) && !m_walkflag(x+usexoffs+dx+try_x,y+useyoffs+sv+usehei-1,special,ndir, x+usexoffs+try_x, y+useyoffs+usehei-1, kb) &&
						 !flyerblocked(x+usexoffs+try_x,y+useyoffs+dy+usehei-1, special,kb) && !flyerblocked(x+usexoffs+dx+try_x,y+useyoffs+8+usehei-1, special,kb);
				}
				try_x += (offgrid ? 8 : 16);
			}
			if(!ok) break;
			if((usewid%16)>0) //Uneven width
			{
				int32_t tries_y = usehei/(offgrid ? 8 : 16);
				try_y = 0;
				for ( ; tries_y > 0; --tries_y )
				{
					ok = !m_walkflag(x+usexoffs+usewid-1,y+useyoffs+dy+try_y,special,ndir, x+usexoffs+usewid-1, y+useyoffs+try_y, kb) && !m_walkflag(x+usexoffs+dx+usewid-1,y+useyoffs+sv+try_y,special,ndir, x+usexoffs+usewid-1, y+useyoffs+try_y, kb) &&
					!flyerblocked(x+usexoffs+usewid-1,y+useyoffs+dy+try_y, special,kb) && !flyerblocked(x+usexoffs+dx+usewid-1,y+useyoffs+8+try_y, special,kb);
					try_y += (offgrid ? 8 : 16);
					if (!ok) break;
				}
				if (!ok) break;
				if((usehei%16)>0) //Uneven height
				{
					ok = !m_walkflag(x+usexoffs+usewid-1,y+useyoffs+dy+usehei-1,special,ndir, x+usexoffs+usewid-1, y+useyoffs+usehei-1, kb) && !m_walkflag(x+usexoffs+dx+usewid-1,y+useyoffs+sv+usehei-1,special,ndir, x+usexoffs+usewid-1, y+useyoffs+usehei-1, kb) &&
						 !flyerblocked(x+usexoffs+usewid-1,y+useyoffs+dy+usehei-1, special,kb) && !flyerblocked(x+usexoffs+dx+usewid-1,y+useyoffs+8+usehei-1, special,kb);
				}
			}
			break;
		}
		case 15:
		case l_up:
		{
			dx = dx1-s;
			dy = dy1-s;
			int32_t tries_x = usewid/(offgrid ? 8 : 16);
			sv = ((isSideViewGravity())?7:0);
			for ( ; tries_x > 0; --tries_x )
			{
				int32_t tries_y = usehei/(offgrid ? 8 : 16);
				try_y = 0;
				for ( ; tries_y > 0; --tries_y )
				{
					ok = !m_walkflag(x+usexoffs+try_x,y+useyoffs+dy+try_y,special,ndir, x+usexoffs+try_x, y+useyoffs+try_y, kb) && !m_walkflag(x+usexoffs+dx+try_x,y+useyoffs+sv+try_y,special,ndir, x+usexoffs+try_x, y+useyoffs+try_y, kb) &&
					!flyerblocked(x+usexoffs+try_x,y+useyoffs+dy+try_y, special,kb) && !flyerblocked(x+usexoffs+dx+try_x,y+useyoffs+8+try_y, special,kb);
					try_y += (offgrid ? 8 : 16);
					if (!ok) break;
				}
				if (!ok) break;
				if((usehei%16)>0) //Uneven height
				{
					ok = !m_walkflag(x+usexoffs+try_x,y+useyoffs+dy+usehei-1,special,ndir, x+usexoffs+try_x, y+useyoffs+usehei-1, kb) && !m_walkflag(x+usexoffs+dx+try_x,y+useyoffs+sv+usehei-1,special,ndir, x+usexoffs+try_x, y+useyoffs+usehei-1, kb) &&
						 !flyerblocked(x+usexoffs+try_x,y+useyoffs+dy+usehei-1, special,kb) && !flyerblocked(x+usexoffs+dx+try_x,y+useyoffs+8+usehei-1, special,kb);
				}
				try_x += (offgrid ? 8 : 16);
			}
			if(!ok) break;
			if((usewid%16)>0) //Uneven width
			{
				int32_t tries_y = usehei/(offgrid ? 8 : 16);
				try_y = 0;
				for ( ; tries_y > 0; --tries_y )
				{
					ok = !m_walkflag(x+usexoffs+usewid-1,y+useyoffs+dy+try_y,special,ndir, x+usexoffs+usewid-1, y+useyoffs+try_y, kb) && !m_walkflag(x+usexoffs+dx+usewid-1,y+useyoffs+sv+try_y,special,ndir, x+usexoffs+usewid-1, y+useyoffs+try_y, kb) &&
					!flyerblocked(x+usexoffs+usewid-1,y+useyoffs+dy+try_y, special,kb) && !flyerblocked(x+usexoffs+dx+usewid-1,y+useyoffs+8+try_y, special,kb);
					try_y += (offgrid ? 8 : 16);
					if (!ok) break;
				}
				if (!ok) break;
				if((usehei%16)>0) //Uneven height
				{
					ok = !m_walkflag(x+usexoffs+usewid-1,y+useyoffs+dy+usehei-1,special,ndir, x+usexoffs+usewid-1, y+useyoffs+usehei-1, kb) && !m_walkflag(x+usexoffs+dx+usewid-1,y+useyoffs+sv+usehei-1,special,ndir, x+usexoffs+usewid-1, y+useyoffs+usehei-1, kb) &&
						 !flyerblocked(x+usexoffs+usewid-1,y+useyoffs+dy+usehei-1, special,kb) && !flyerblocked(x+usexoffs+dx+usewid-1,y+useyoffs+8+usehei-1, special,kb);
				}
			}
			break;
		}
		default:
			db=99;
			return true;
	}
	//Z_eventlog("\n");
	return ok;
}


bool enemy::canmove(int32_t ndir,zfix s,int32_t special, bool kb)
{
	int32_t usewid = (SIZEflags&guyflagOVERRIDE_HIT_WIDTH) ? hxsz : 16;
	int32_t usehei = (SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) ? hysz : 16;
	if (usewid % 16 != 0) usewid += (16 - (usewid%16));
	if (usehei % 16 != 0) usehei += (16 - (usehei%16));
	--usewid;
	--usehei;
	return canmove(ndir,s,special,0,-8,usewid,usehei,kb);
}

bool enemy::canmove(int32_t ndir,int32_t special, bool kb)
{
	bool dodongo_move=true; //yes, it's an ugly hack, but we're going to rewrite everything later anyway - DN
	
	if(special==spw_clipright&&ndir==right)
	{
		dodongo_move=canmove(ndir,(zfix)1,special,0,-8,31,15,kb);
	}
	int32_t usewid = (SIZEflags&guyflagOVERRIDE_HIT_WIDTH) ? hxsz : 16;
	int32_t usehei = (SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) ? hysz : 16;
	if (usewid % 16 != 0) usewid += (16 - (usewid%16));
	if (usehei % 16 != 0) usehei += (16 - (usehei%16));
	--usewid;
	--usehei;
	return canmove(ndir,(zfix)1,special,0,-8,usewid,usehei,kb)&&dodongo_move;
}

bool enemy::canmove(int32_t ndir, bool kb)
{
	return canmove(ndir,(zfix)1,spw_none,0,-8,15,15,kb);
}

// 8-directional
void enemy::newdir_8_old(int32_t newrate,int32_t newhoming,int32_t special,int32_t dx1,int32_t dy1,int32_t dx2,int32_t dy2)
{
	int32_t ndir=0;
	
	// can move straight, check if it wants to turn
	if(canmove_old(dir,step,special,dx1,dy1,dx2,dy2))
	{
		if(grumble && (zc_oldrand()&4)<grumble) //Homing
		{
			int32_t w = Lwpns.idFirst(wBait);
			
			if(w>=0)
			{
				int32_t bx = Lwpns.spr(w)->x;
				int32_t by = Lwpns.spr(w)->y;
				
				ndir = (bx<x) ? left : (bx!=x) ? right : 0;
				
				if(abs(int32_t(y)-by)>14)
				{
					if(ndir>0)  // Already left or right
					{
						// Making the diagonal directions
						ndir += (by<y) ? 2 : 4;
					}
					else
					{
						ndir = (by<y) ? up : down;
					}
				}
				
				if(canmove(ndir,special,false))
				{
					dir=ndir;
					return;
				}
			}
		}
		
		// Homing added.
		if(newhoming && (zc_oldrand()&255)<newhoming)
		{
			ndir = lined_up(8,true);
			
			if(ndir>=0 && canmove(ndir,special,false))
			{
				dir=ndir;
			}
			
			return;
		}
		
		int32_t r=zc_oldrand();
		
		if(newrate>0 && !(r%newrate))
		{
			ndir = ((dir+((r&64)?-1:1))&7)+8;
			int32_t ndir2=((dir+((r&64)?1:-1))&7)+8;
			
			if(canmove(ndir,step,special,dx1,dy1,dx2,dy2,false))
				dir=ndir;
			else if(canmove(ndir2,step,special,dx1,dy1,dx2,dy2,false))
				dir=ndir2;
				
			if(dir==ndir && (newrate>=4)) // newrate>=4, otherwise movement is biased toward upper-left
				// due to numerous lost fractional components. -L
			{
				x.doFloor();
				y.doFloor();
			}
		}
		
		return;
	}
	
	// can't move straight, must turn
	int32_t i=0;
	
	for(; i<32; i++)  // Try random dir
	{
		ndir=(zc_oldrand()&7)+8;
		
		if(canmove(ndir,step,special,dx1,dy1,dx2,dy2,false))
			break;
	}
	
	if(i==32)
	{
		for(ndir=8; ndir<16; ndir++)
		{
			if(canmove(ndir,step,special,dx1,dy1,dx2,dy2,false))
				goto ok;
		}
		
		ndir = (isSideViewGravity()) ? (zc_oldrand()&1 ? left : right) : -1;  // Sideview enemies get trapped if their dir becomes -1
	}
	
ok:
	dir=ndir;
	x.doFloor();
	y.doFloor();
}

void enemy::newdir_8(int32_t newrate,int32_t newhoming,int32_t special,int32_t dx1,int32_t dy1,int32_t dx2,int32_t dy2)
{
	int32_t ndir=0;
	
	// can move straight, check if it wants to turn
	if(canmove(dir,step,special,dx1,dy1,dx2,dy2,false))
	{
		if(grumble && (zc_oldrand()&4)<abs(grumble)) //Homing
		{
			int32_t i = Lwpns.idFirst(wBait);
			if(i >= 0) //idfirst returns -1 if it can't find any
			{
				weapon *w  = (weapon*)Lwpns.spr(i);
				if (get_bit(quest_rules, qr_FIND_CLOSEST_BAIT))
				{
					int32_t currentrange;
					if (distance(x, y, w->x, w->y) < w->misc2 || w->misc2 == 0) currentrange = distance(x, y, w->x, w->y);
					else currentrange = -1;
					int curid = i;
					++i; //increment beforehand cause we just checked the first bait weapon and all others must be after it. ...otherwise it wouldn't be the first. -Deedee
					for(; i<Lwpns.Count(); ++i)
					{
						weapon *lw = (weapon*)Lwpns.spr(i);
						if (lw->id == wBait && distance(x, y, lw->x, lw->y) < currentrange && (distance(x, y, lw->x, lw->y) < lw->misc2 || lw->misc2 == 0)) 
						{
							currentrange = distance(x, y, lw->x, lw->y);
							curid = i;
						}
					}
					i = curid;
					if (currentrange == -1) i = -1;
				}
				else
				{
					if (!(distance(x, y, w->x, w->y) < w->misc2 || w->misc2 == 0)) i = -1;
				}
				if(i>=0)
				{
					int32_t bx = Lwpns.spr(i)->x;
					int32_t by = Lwpns.spr(i)->y;
					
					ndir = (bx<x) ? left : (bx!=x) ? right : 0;
					
					if(abs(int32_t(y)-by)>14)
					{
						if(ndir>0)  // Already left or right
						{
							// Making the diagonal directions
							ndir += (by<y) ? 2 : 4;
						}
						else
						{
							ndir = (by<y) ? up : down;
						}
					}
					if (grumble < 0 || (itemsbuf[((weapon*)Lwpns.spr(i))->parentitem].flags & ITEM_FLAG1)) ndir = oppositeDir[ndir];
					if(canmove(ndir,special,false))
					{
						dir=ndir;
						return;
					}
				}
			}
		}
		
		// Homing added.
		if(newhoming && (zc_oldrand()&255)<abs(newhoming))
		{
			ndir = lined_up(8,true);
			if (newhoming < 0 && ndir >= 0) ndir = oppositeDir[ndir];
			if(ndir>=0 && canmove(ndir,special,false))
			{
				dir=ndir;
			}
			
			return;
		}
		
		int32_t r=zc_oldrand();
		
		if(newrate>0 && !(r%newrate))
		{
			ndir = ((dir+((r&64)?-1:1))&7)+8;
			int32_t ndir2=((dir+((r&64)?1:-1))&7)+8;
			
			if(canmove(ndir,step,special,dx1,dy1,dx2,dy2,false))
				dir=ndir;
			else if(canmove(ndir2,step,special,dx1,dy1,dx2,dy2,false))
				dir=ndir2;
				
			if(dir==ndir && (newrate>=4)) // newrate>=4, otherwise movement is biased toward upper-left
				// due to numerous lost fractional components. -L
			{
				x.doFloor();
				y.doFloor();
			}
		}
		
		return;
	}
	
	// can't move straight, must turn
	int32_t i=0;
	
	for(; i<32; i++)  // Try random dir
	{
		ndir=(zc_oldrand()&7)+8;
		
		if(canmove(ndir,step,special,dx1,dy1,dx2,dy2,false))
			break;
	}
	
	if(i==32)
	{
		for(ndir=8; ndir<16; ndir++)
		{
			if(canmove(ndir,step,special,dx1,dy1,dx2,dy2,false))
				goto ok;
		}
		
		ndir = (isSideViewGravity()) ? (zc_oldrand()&1 ? left : right) : -1;  // Sideview enemies get trapped if their dir becomes -1
	}
	
ok:
	dir=ndir;
	x.doFloor();
	y.doFloor();
}

void enemy::newdir_8(int32_t newrate,int32_t newhoming,int32_t special)
{
	newdir_8(newrate,newhoming,special,0,-8,15,15);
}

void enemy::newdir_8_old(int32_t newrate,int32_t newhoming,int32_t special)
{
	newdir_8_old(newrate,newhoming,special,0,-8,15,15);
}

// makes the enemy slide backwards when hit
// sclk: first byte is clk, second byte is dir
// makes the enemy slide backwards when hit
// sclk: first byte is clk, second byte is dir
int32_t enemy::slide()
{
	if(script_knockback_clk!=0) //scripted knockback
	{
		sclk = 0;
		return 1; //scripted knockback ran
	}
	if(sclk==0 || (hp<=0 && !immortal))
		return 0;
		
	if(knockbackflags & FLAG_NOSLIDE)
	{
		sclk = 0;
		if(!OFFGRID_ENEMY)
		{
			//Fix to grid
			//x = (int32_t(x)+8)-((int32_t(x)+8)%16);
			//y = (int32_t(y)+8)-((int32_t(y)+8)%16);
			do_fix(x, 16, true);
			do_fix(y, 16, true);
		}
		return 0;
	}
	if((sclk&255)==16 && (get_bit(quest_rules,qr_OLD_ENEMY_KNOCKBACK_COLLISION) || knockbackSpeed!=4 ? !canmove(sclk>>8,(zfix) (dmisc2==e2tSPLITHIT ? 1 : 12),0,true) : !canmove(sclk>>8,(zfix) (dmisc2==e2tSPLITHIT ? 1 : knockbackSpeed),0,0,0,15,15,true)))
	{
		sclk=0;
		return 0;
	}
	
	--sclk;
	
	switch(sclk>>8)
	{
		case up:
		{
		if(y<=(dmisc2==e2tSPLITHIT ? 0 : (get_bit(quest_rules,qr_OLD_ENEMY_KNOCKBACK_COLLISION)?16:0))) //vires
		{
			sclk=0;
			return 0;
		}
		if ( dmisc2==e2tSPLITHIT && !canmove(sclk>>8,(zfix)(4),0,true) ) { sclk=0; return 0; } //vires
		
		break;
		}
		case down:
		{
		if(y>=(dmisc2==e2tSPLITHIT ? 150 : 160)) //was 160 --changed for vires bug. 
		{
			sclk=0;
			return 0;
		}
		if ( dmisc2==e2tSPLITHIT && !canmove(sclk>>8,(zfix)(4),0,true) ) { sclk=0; return 0; } //vires
		
		break;
		}
		case left:
		{
		if(x<=(dmisc2==e2tSPLITHIT ? 0 : (get_bit(quest_rules,qr_OLD_ENEMY_KNOCKBACK_COLLISION)?16:0)))
		{
			sclk=0;
			return 0;
		}
		if ( dmisc2==e2tSPLITHIT && !canmove(sclk>>8,(zfix)(4),0,true) ) { sclk=0; return 0; }
		
		break;
		}
		case right:
		{
		if(x>=(dmisc2==e2tSPLITHIT ? 255 : 240)) //vires
		{
			sclk=0;
			return 0;
		}
		if ( dmisc2==e2tSPLITHIT && !canmove(sclk>>8,(zfix)(4),0,true) ) { sclk=0; return 0; } //vires
		break;
		}
	}
	
	int32_t move = knockbackSpeed;
	while(move>0)
	{
		int32_t thismove = zc_min(8, move);
		move -= thismove;
		hitdir = (sclk>>8);
		switch(sclk>>8)
		{
		case up:
			y-=thismove;
			break;
			
		case down:
			y+=thismove;
			break;
			
		case left:
			x-=thismove;
			break;
			
		case right:
			x+=thismove;
			break;
		}
		if(!canmove(sclk>>8,(zfix)0,0,true))
		{
			switch(sclk>>8)
			{
			case up:
			case down:
				if(y < 0)
					y = 0;
				else if((int32_t(y)&15) > 7)
					y=(int32_t(y)&0xF0)+16;
				else
					y=(int32_t(y)&0xF0);
					
				break;
				
			case left:
			case right:
				if(x < 0)
					x = 0;
				else if((int32_t(x)&15) > 7)
					x=(int32_t(x)&0xF0)+16;
				else
					x=(int32_t(x)&0xF0);
					
				break;
			}
			
			sclk=0;
			clk3=0;
			break;
		}
	}
	
	if((sclk&255)==0)
	{
		//hitdir = -1;
		sclk=0;
	}
	return 2;
}

bool enemy::can_slide()
{
	if(sclk==0 || (hp<=0 && !immortal))
		return false;
		
	if((sclk&255)==16 && (get_bit(quest_rules,qr_OLD_ENEMY_KNOCKBACK_COLLISION) || knockbackSpeed!=4 ? !canmove(sclk>>8,(zfix) (dmisc2==e2tSPLITHIT ? 1 : 12),0,true) : !canmove(sclk>>8,(zfix) (dmisc2==e2tSPLITHIT ? 1 : knockbackSpeed),0,true)))
	{
		return false;
	}
	
	return true;
}

bool enemy::fslide()
{
	if(sclk==0 || (hp<=0 && !immortal))
		return false;
		
	if((sclk&255)==16 && !canmove(sclk>>8,(zfix)12,spw_floater,true))
	{
		sclk=0;
		return false;
	}
	
	--sclk;
	
	switch(sclk>>8)
	{
	case up:
		if(y<=16)
		{
			sclk=0;
			return false;
		}
		
		break;
		
	case down:
		if(y>=160)
		{
			sclk=0;
			return false;
		}
		
		break;
		
	case left:
		if(x<=16)
		{
			sclk=0;
			return false;
		}
		
		break;
		
	case right:
		if(x>=240)
		{
			sclk=0;
			return false;
		}
		
		break;
	}
	hitdir = (sclk>>8);
	switch(sclk>>8)
	{
	case up:
		y-=4;
		break;
		
	case down:
		y+=4;
		break;
		
	case left:
		x-=4;
		break;
		
	case right:
		x+=4;
		break;
	}
	
	if(!canmove(sclk>>8,(zfix)0,spw_floater,true))
	{
		switch(sclk>>8)
		{
		case up:
		case down:
			if((int32_t(y)&15) > 7)
				y=(int32_t(y)&0xF0)+16;
			else
				y=(int32_t(y)&0xF0);
				
			break;
			
		case left:
		case right:
			if((int32_t(x)&15) > 7)
				x=(int32_t(x)&0xF0)+16;
			else
				x=(int32_t(x)&0xF0);
				
			break;
		}
		
		sclk=0;
		clk3=0;
	}
	
	if((sclk&255)==0)
		sclk=0;
		
	return true;
}

bool enemy::knockback(int32_t time, int32_t dir, int32_t speed)
{
	if((hp<=0 && !immortal)) return false; //No knocking back dead/mid-knockback enemies
	if(!canmove(dir,(zfix)speed,0,0,0,15,15,true)) return false; //from slide(); collision check
	bool ret = sprite::knockback(time, dir, speed);
	if(ret) sclk = 0; //kill engine knockback if interrupted
	//! Perhaps also set hitdir here, if needed for timing? -Z
	return ret;
}

bool enemy::runKnockback()
{
	if((script_knockback_clk&0xFF)==0)
	{
		script_knockback_clk = 0;
		return false;
	}
	if(knockbackflags & FLAG_NOSCRIPTKNOCKBACK)
	{
		return false;
	}
	int32_t move = script_knockback_speed;
	int32_t kb_dir = script_knockback_clk>>8;
	--script_knockback_clk;
	while(move>0)
	{
		int32_t thismove = zc_min(8, move);
		move -= thismove;
		hitdir = kb_dir;
		switch(kb_dir)
		{
			case r_up:
			case l_up:
			case up:
				y-=thismove;
				break;
				
			case r_down:
			case l_down:
			case down:
				y+=thismove;
				break;
		}
		switch(kb_dir)
		{
			case l_up:
			case l_down:
			case left:
				x-=thismove;
				break;
			
			case r_up:
			case r_down:
			case right:
				x+=thismove;
				break;
		}
		if(!canmove(kb_dir,(zfix)0,0,true))
		{
			script_knockback_clk=0;
			clk3=0;
			//Fix to grid
			switch(kb_dir)
			{
				case up:
				case down:
					break;
				default:
					if(x < 0)
						x = 0;
					else if((int32_t(x)&15) > 7)
						x=(int32_t(x)&0xF0)+16;
					else
						x=(int32_t(x)&0xF0);
					break;
			}
			switch(kb_dir)
			{
				case left:
				case right:
					break;
				default:
					if(y < 0)
						y = 0;
					else if((int32_t(y)&15) > 7)
						y=(int32_t(y)&0xF0)+16;
					else
						y=(int32_t(y)&0xF0);
					break;
			}
			break;
		}
	}
	return true;
}
// changes enemy's direction, checking restrictions
// rate:   0 = no random changes, 16 = always random change
// homing: 0 = none, 256 = always
// grumble 0 = none, 4 = strongest appetite
void enemy::newdir(int32_t newrate,int32_t newhoming,int32_t special)
{
	int32_t ndir=-1;
	
	if(grumble != 0 && (zc_oldrand()&3)<abs(grumble)) //yes, I know checking if grumble is equal to if grumble == 0, but the latter makes the intention more clear to less experienced coders who might join.
	{
		int32_t i = Lwpns.idFirst(wBait);
		if(i >= 0) //idfirst returns -1 if it can't find any
		{
			weapon *w  = (weapon*)Lwpns.spr(i);
			if (get_bit(quest_rules, qr_FIND_CLOSEST_BAIT))
			{
				int32_t currentrange;
				if (distance(x, y, w->x, w->y) < w->misc2 || w->misc2 == 0) currentrange = distance(x, y, w->x, w->y);
				else currentrange = -1;
				int curid = i;
				++i; //increment beforehand cause we just checked the first bait weapon and all others must be after it. ...otherwise it wouldn't be the first. -Deedee
				for(; i<Lwpns.Count(); ++i)
				{
					weapon *lw = (weapon*)Lwpns.spr(i);
					if (lw->id == wBait && distance(x, y, lw->x, lw->y) < currentrange && (distance(x, y, lw->x, lw->y) < lw->misc2 || lw->misc2 == 0)) 
					{
						currentrange = distance(x, y, lw->x, lw->y);
						curid = i;
					}
				}
				i = curid;
				if (currentrange == -1) i = -1;
			}
			else
			{
				if (!(distance(x, y, w->x, w->y) < w->misc2 || w->misc2 == 0)) i = -1;
			}
			if (i >= 0)
			{
				int32_t bx = Lwpns.spr(i)->x;
				int32_t by = Lwpns.spr(i)->y;
				
				if(abs(int32_t(y)-by)>14)
				{
					ndir = (by<y) ? up : down;
					if (grumble < 0 || (itemsbuf[((weapon*)Lwpns.spr(i))->parentitem].flags & ITEM_FLAG1)) ndir = oppositeDir[ndir];
					if(canmove(ndir,special,false))
					{
						dir=ndir;
						return;
					}
				}
				
				ndir = (bx<x) ? left : right;
				if (grumble < 0 || (itemsbuf[((weapon*)Lwpns.spr(i))->parentitem].flags & ITEM_FLAG1)) ndir = oppositeDir[ndir];
				if(canmove(ndir,special,false))
				{
					dir=ndir;
					return;
				}
			}
		}
	}
	
	if((zc_oldrand()&255)<abs(newhoming))
	{
		ndir = lined_up(8,false);
		if (newhoming < 0 && ndir >= 0) ndir = oppositeDir[ndir];
		if(ndir>=0 && canmove(ndir,special,false))
		{
			dir=ndir;
			return;
		}
	}
	
	int32_t i=0;
	
	for(; i<32; i++)
	{
		int32_t r=zc_oldrand();
		
		if((r&15)<newrate)
			ndir=(r>>4)&3;
		else
			ndir=dir;
			
		if(canmove(ndir,special,false))
			break;
	}
	
	if(i==32)
	{
		for(ndir=0; ndir<4; ndir++)
		{
			if(canmove(ndir,special,false))
				goto ok;
		}
		
		ndir = (isSideViewGravity()) ? (zc_oldrand()&1 ? left : right) : -1; // Sideview enemies get trapped if their dir becomes -1
		//...Isn't that the point? I'm not sure I understand. Certainly beats phasing through walls... -Dimi
	}
	
ok:
	dir = ndir;
}

void enemy::newdir()
{
	newdir(4,0,spw_none);
}

zfix enemy::distance_left()
{
	int32_t a2=x.getInt();
	int32_t b2=y.getInt();
	
	switch(dir)
	{
	case up:
		return (zfix)(b2&0xF);
		
	case down:
		return (zfix)(16-(b2&0xF));
		
	case left:
		return (zfix)(a2&0xF);
		
	case right:
		return (zfix)(16-(a2&0xF));
	}
	
	return (zfix)0;
}

// keeps walking around
void enemy::constant_walk(int32_t newrate,int32_t newhoming,int32_t special)
{
	if(slide())
		return;
		
	if(clk<0 || dying || stunclk || watch || ceiling || frozenclock )
		return;
		
	if(clk3<=0)
	{
		fix_coords(true);
		newdir(newrate,newhoming,special);
		
		if(step==0)
			clk3=0;
		else
			clk3=int32_t(16.0/step);
	}
	else if(scored)
	{
		dir^=1;
		clk3=int32_t(16.0/step)-clk3;
	}
	
	--clk3;
	move(step);
}

void enemy::constant_walk()
{
	constant_walk(4,0,spw_none);
}

int32_t enemy::pos(int32_t newx,int32_t newy)
{
	return (newy<<8)+newx;
}

// for variable step rates
void enemy::variable_walk(int32_t newrate,int32_t newhoming,int32_t special)
{
	if(slide())
		return;
		
	if(clk<0 || dying || stunclk || watch || step == 0 || ceiling || frozenclock )
		return;
		
	zfix dx = (zfix)0;
	zfix dy = (zfix)0;
	
	switch(dir)
	{
	case 8:
	case up:
		dy-=step;
		break;
		
	case 12:
	case down:
		dy+=step;
		break;
		
	case 14:
	case left:
		dx-=step;
		break;
		
	case 10:
	case right:
		dx+=step;
		break;
		
	case 15:
	case l_up:
		dx-=step;
		dy-=step;
		break;
		
	case 9:
	case r_up:
		dx+=step;
		dy-=step;
		break;
		
	case 13:
	case l_down:
		dx-=step;
		dy+=step;
		break;
		
	case 11:
	case r_down:
		dx+=step;
		dy+=step;
		break;
	}
	
	if(((int32_t(x)&15)==0 && (int32_t(y)&15)==0 && clk3!=pos(x,y)) ||
			m_walkflag(int32_t(x+dx),int32_t(y+dy), spw_halfstep, dir))
	{
		fix_coords();
		newdir(newrate,newhoming,special);
		clk3=pos(x,y);
	}
	
	move(step);
}

// pauses for a while after it makes a complete move (to a new square)
void enemy::halting_walk(int32_t newrate,int32_t newhoming,int32_t special,int32_t newhrate, int32_t haltcnt)
{
	if(sclk && clk2)
	{
		clk3=0;
	}
	
	if(slide() || clk<0 || dying || stunclk || watch || ceiling || frozenclock)
	{
		return;
	}
	
	if(clk2>0)
	{
		--clk2;
		return;
	}
	
	if(clk3<=0)
	{
		fix_coords(true);
		newdir(newrate,newhoming,special);
		clk3=int32_t(16.0/step);
		
		if(clk2<0)
		{
			clk2=0;
		}
		else if((zc_oldrand()&15)<newhrate)
		{
			clk2=haltcnt;
			return;
		}
	}
	else if(scored)
	{
		dir^=1;
		
		clk3=int32_t(16.0/step)-clk3;
	}
	
	--clk3;
	move(step);
}

// 8-directional movement, aligns to 8 pixels
void enemy::constant_walk_8(int32_t newrate,int32_t newhoming,int32_t special)
{
	if(clk<0 || dying || stunclk || watch || ceiling || frozenclock)
		return;
		
	if(clk3<=0)
	{
		newdir_8(newrate,newhoming,special);
		clk3=int32_t(8.0/step);
	}
	
	--clk3;
	move(step);
}
// 8-directional movement, aligns to 8 pixels
void enemy::constant_walk_8_old(int32_t newrate,int32_t newhoming,int32_t special)
{
	if(clk<0 || dying || stunclk || watch || ceiling || frozenclock)
		return;
		
	if(clk3<=0)
	{
		newdir_8(newrate,newhoming,special);
		clk3=int32_t(8.0/step);
	}
	
	--clk3;
	move(step);
}

void enemy::halting_walk_8(int32_t newrate,int32_t newhoming, int32_t newclk,int32_t special,int32_t newhrate, int32_t haltcnt)
{
	if(clk<0 || dying || stunclk || watch || frozenclock)
		return;
		
	if(!canmove(dir,step,special,false))
		clk3=0;
		
	if(clk2>0)
	{
		--clk2;
		return;
	}
	
	if(clk3<=0)
	{
		newdir_8(newrate,newhoming,special);
		clk3=newclk;
		
		if(clk2<0)
		{
			clk2=0;
		}
		else if((zc_oldrand()&15)<newhrate)
		{
			newdir_8(newrate,newhoming,special);
			clk2=haltcnt;
			return;
		}
	}
	
	--clk3;
	move(step);
}

// 8-directional movement, no alignment
void enemy::variable_walk_8(int32_t newrate,int32_t newhoming, int32_t newclk,int32_t special)
{
	if(clk<0 || dying || stunclk || watch || ceiling || frozenclock)
		return;
		
	if(!canmove(dir,step,special,false))
		clk3=0;
		
	if(clk3<=0)
	{
		newdir_8(newrate,newhoming,special);
		clk3=newclk;
	}
	
	--clk3;
	move(step);
}

// same as above but with variable enemy size
void enemy::variable_walk_8(int32_t newrate,int32_t newhoming, int32_t newclk,int32_t special,int32_t dx1,int32_t dy1,int32_t dx2,int32_t dy2)
{
	if(clk<0 || dying || stunclk || watch || ceiling || frozenclock)
		return;
		
	if(!canmove(dir,step,special,dx1,dy1,dx2,dy2,false))
		clk3=0;
		
	if(clk3<=0)
	{
		newdir_8(newrate,newhoming,special,dx1,dy1,dx2,dy2);
		clk3=newclk;
	}
	
	--clk3;
	move(step);
}

// the variable speed floater movement
// ms is max speed
// ss is step speed
// s is step count
// p is pause count
// g is graduality :)
//floater_walk(rate,hrate,dstep/100,(zfix)0,10,dmisc16,dmisc17);
void enemy::floater_walk(int32_t newrate,int32_t newclk,zfix ms,zfix ss,int32_t s,int32_t p, int32_t g)
{
	++clk2;
	byte over_pit = overpit(this);
	
	if(dmisc1 && over_pit) p = 0;
	switch(movestatus)
	{
		//! This needs a case 4 (landing)....if we want to halt, we move to case 4, and 
		//! if the conditions prevent it, we jump back to case 2.
	case 0:                                                 // paused
		if(clk2>=p)
		{
			movestatus=1;
			clk2=0;
		}
		
		break;
		
	case 1:                                                 // speeding up
		if(clk2<g*s)
		{
			if(!((clk2-1)%g))
				step+=ss;
		}
		else
		{
			movestatus=2;
			clk2=0;
		}
		
		break;
		
	case 2:                                                 // normal
		step=ms;
		
		if(clk2>48 && !(zc_oldrand()%768))
		{
			step=ss*s;
			movestatus=3;
			clk2=0;
		}
		
		break;
		
	case 3:                                                 // slowing down
		if(clk2<=g*s)
		{
			{ //don't slow down over pits
				
				if(over_pit) 
				{
					if(dmisc1)
					{
						step=ms;
					}
				}
				else //can slow down
				{
					if(!(clk2%g) && !dmisc1)
						step-=ss;
				}
			}
			
			
		}
		else
		{
			//if((moveflags&FLAG_CAN_PITFALL)) //don't check pits if the enemy ignores them
			//this doesn't help keese, as they have a z of 0. 
			//they always nee to run this check.
			{
				if(over_pit &&!dmisc1) 
				{
					--clk2; //if over a pit, don't land, and revert clock change
				}
				else //can land safely
				{
					movestatus=0;
					if(dmisc1&&!over_pit)
						step=0;
					clk2=0;
				}
			}
			
		}
		
		break;
	}
	
	variable_walk_8(movestatus==2?newrate:0,homing,newclk,spw_floater);
}

void enemy::floater_walk(int32_t newrate,int32_t newclk,zfix s)
{
	floater_walk(newrate,newclk,s,(zfix)0.125,3,80,32);
}

// Checks if enemy is lined up with Link. If so, returns direction Link is
// at as compared to enemy. Returns -1 if not lined up. Range is inclusive.
int32_t enemy::lined_up(int32_t range, bool dir8)
{
	int32_t lx = Link.getX();
	int32_t ly = Link.getY();
	
	if(abs(lx-int32_t(x))<=range)
	{
		if(ly<y)
		{
			return up;
		}
		
		return down;
	}
	
	if(abs(ly-int32_t(y))<=range)
	{
		if(lx<x)
		{
			return left;
		}
		
		return right;
	}
	
	if(dir8)
	{
	if(abs(lx-x)-abs(ly-y)<=range)
		//if(abs(lx-x)-abs(ly-y)<=range && abs(ly-y)-abs(lx-x)<=range) //Fix floating enemies not seeking link. -Tamamo
		{
			if(ly<y)
			{
				if(lx<x)
				{
					return l_up;
				}
				else
				{
					return r_up;
				}
			}
			else
			{
				if(lx<x)
				{
					return l_down;
				}
				else
				{
					return r_down;
				}
			}
		}
	}
	
	return -1;
}

// returns true if Link is within 'range' pixels of the enemy
bool enemy::LinkInRange(int32_t range)
{
	int32_t lx = Link.getX();
	int32_t ly = Link.getY();
	return abs(lx-int32_t(x))<=range && abs(ly-int32_t(y))<=range;
}

// place the enemy in line with Link (red wizzrobes)
void enemy::place_on_axis(bool floater, bool solid_ok)
{
	int32_t lx=zc_min(zc_max(int32_t(Link.getX())&0xF0,32),208);
	int32_t ly=zc_min(zc_max(int32_t(Link.getY())&0xF0,32),128);
	int32_t pos2=zc_oldrand()%23;
	int32_t tried=0;
	bool last_resort,placed=false;
	
	
	do
	{
		if(pos2<14)
		{
			x=(pos2<<4)+16;
			y=ly;
		}
		else
		{
			x=lx;
			y=((pos2-14)<<4)+16;
		}
		
		// Don't commit to a last resort if position is out of bounds.
		last_resort= !(x<32 || y<32 || x>=224 || y>=144);
		
		if(abs(lx-int32_t(x))>16 || abs(ly-int32_t(y))>16)
		{
			// Red Wizzrobes should be able to appear on water, but not other
			// solid combos; however, they could appear on solid combos in 2.10,
			// and some quests depend on that.
			if((solid_ok || !m_walkflag(x,y,floater ? spw_water : spw_door, dir))
					&& !flyerblocked(x,y,floater ? spw_floater : spw_door))
				placed=true;
		}
		
		if(!placed && tried>=22 && last_resort)
		{
			placed=true;
		}
		
		++tried;
		pos2=(pos2+3)%23;
	}
	while(!placed);
	
	if(y==ly)
		dir=(x<lx)?right:left;
	else
		dir=(y<ly)?down:up;
		
	clk2=tried;
}

void enemy::n_frame_n_dir(int32_t frames, int32_t ndir, int32_t f4)
{
	int32_t t = o_tile;
	
	// Darknuts, but also Wizzrobes and Wallmasters
	switch(family)
	{
	case eeWALK:
		if(dmisc9==e9tPOLSVOICE && clk2>=0 && do_animation)
		{
			tile=s_tile;
			t=s_tile;
		}
		
		break;
		
	case eeTRAP:
		if(dummy_int[1] && guysbuf[id].flags2 & eneflag_trp2 && do_animation)  // Just to make sure
		{
			tile=s_tile;
			t=s_tile;
		}
		
		break;
		
	case eeSPINTILE:
		if(misc>=96 && do_animation)
		{
			tile=o_tile+frames*ndir;
			t=tile;
		}
		
		break;
	}
	if ( do_animation ) 
	{
		if(ndir!=0) switch(frames)
		{
		case 2:
			tiledir_small(dir,ndir==4);
			break;
			
		case 3:
			tiledir_three(dir);
			break;
			
		case 4:
			tiledir(dir,ndir==4);
			break;
		}

		if(family==eeWALK)
		tile=zc_min(tile+f4, t+frames*(zc_max(dir, 0)+1)-1);
		else
		tile+=f4;
	}
}

void enemy::tiledir_three(int32_t ndir)
{
	if ( !do_animation ) return;
	flip=0;
	
	switch(ndir)
	{
	case right:
		tile+=3;
		[[fallthrough]];
		
	case left:
		tile+=3;
		[[fallthrough]];
		
	case down:
		tile+=3;
		[[fallthrough]];
		
	case up:
		break;
	}
}

void enemy::tiledir_small(int32_t ndir, bool fourdir)
{
	if ( !do_animation ) return;
	flip=0;
	
	switch(ndir)
	{
	case 8:
	case up:
		break;
		
	case 12:
	case down:
		tile+=2;
		break;
		
	case 14:
	case left:
		tile+=4;
		break;
		
	case 10:
	case right:
		tile+=6;
		break;
		
	case 9:
	case r_up:
		if(fourdir)
			break;
			
		tile+=10;
		break;
		
	case 11:
	case r_down:
		if(fourdir)
			tile+=2;
		else
			tile+=14;
			
		break;
		
	case 13:
	case l_down:
		if(fourdir)
			tile+=2;
		else
			tile+=12;
			
		break;
		
	case 15:
	case l_up:
		if(fourdir)
			break;
			
		tile+=8;
		break;
		
	default:
		//dir=(zc_oldrand()*100)%8;
		//tiledir_small(dir);
		//      flip=zc_oldrand()&3;
		//      tile=(zc_oldrand()*100000)%NEWMAXTILES;
		break;
	}
}

void enemy::tiledir(int32_t ndir, bool fourdir)
{
	if ( !do_animation ) return;
	flip=0;
	
	switch(ndir)
	{
	case 8:
	case up:
		break;
		
	case 12:
	case down:
		tile+=4;
		break;
		
	case 14:
	case left:
		tile+=8;
		break;
		
	case 10:
	case right:
		tile+=12;
		break;
		
	case 9:
	case r_up:
		if(fourdir)
			break;
		else
			tile+=24;
			
		break;
		
	case 11:
	case r_down:
		if(fourdir)
			tile+=4;
		else
			tile+=32;
			
		break;
		
	case 13:
	case l_down:
		if(fourdir)
			tile+=4;
		else
			tile+=28;
			
		break;
		
	case 15:
	case l_up:
		if(fourdir)
			break;
		else
			tile+=20;
			
		break;
		
	default:
		//dir=(zc_oldrand()*100)%8;
		//tiledir(dir);
		//      flip=zc_oldrand()&3;
		//      tile=(zc_oldrand()*100000)%NEWMAXTILES;
		break;
	}
}

void enemy::tiledir_big(int32_t ndir, bool fourdir)
{
	if ( !do_animation ) return;
	flip=0;
	
	switch(ndir)
	{
	case 8:
	case up:
		break;
		
	case 12:
	case down:
		tile+=8;
		break;
		
	case 14:
	case left:
		tile+=40;
		break;
		
	case 10:
	case right:
		tile+=48;
		break;
		
	case 9:
	case r_up:
		if(fourdir)
			break;
			
		tile+=88;
		break;
		
	case 11:
	case r_down:
		if(fourdir)
			tile+=8;
		else
			tile+=128;
			
		break;
		
	case 13:
	case l_down:
		if(fourdir)
			tile+=8;
		else
			tile+=120;
			
		break;
		
	case 15:
	case l_up:
		if(fourdir)
			break;
			
		tile+=80;
		break;
		
	default:
		//dir=(zc_oldrand()*100)%8;
		//tiledir_big(dir);
		//      flip=zc_oldrand()&3;
		//      tile=(zc_oldrand()*100000)%NEWMAXTILES;
		break;
	}
}

void enemy::update_enemy_frame()
{
	if(fallclk||drownclk) return;
	if (!do_animation)
		return;
	if(get_bit(quest_rules,qr_ANONE_NOANIM)
		&& anim == aNONE && family != eeGUY)
		return;
	int32_t newfrate = zc_max(frate,4);
	int32_t f4=abs(clk/(newfrate/4)); // casts clk to [0,1,2,3]
	int32_t f2=abs(clk/(newfrate/2)); // casts clk to [0,1]
	int32_t fx = get_bit(quest_rules, qr_NEWENEMYTILES) ? f4 : f2;
	tile = o_tile;
	int32_t tilerows = 1; // How many rows of tiles? The Extend code needs to know.
	bool ignore_extend = false;
	switch(anim)
	{
	
	case aDONGO:
	{
		int32_t fr = stunclk>0 ? 16 : 8;
		
		if(!dying && clk2>0 && clk2<=64)
		{
			// bloated
			switch(dir)
			{
			case up:
				tile+=9;
				flip=0;
				xofs=0;
				dummy_int[1]=0; //no additional tiles
				break;
				
			case down:
				tile+=7;
				flip=0;
				xofs=0;
				dummy_int[1]=0; //no additional tiles
				break;
				
			case left:
				flip=1;
				tile+=4;
				xofs=16;
				dummy_int[1]=1; //second tile is next tile
				break;
				
			case right:
				flip=0;
				tile+=5;
				xofs=16;
				dummy_int[1]=-1; //second tile is previous tile
				break;
			}
		}
		else if(!dying || clk2>19)
		{
			// normal
			switch(dir)
			{
			case up:
				tile+=8;
				flip=(clk&fr)?1:0;
				xofs=0;
				dummy_int[1]=0; //no additional tiles
				break;
				
			case down:
				tile+=6;
				flip=(clk&fr)?1:0;
				xofs=0;
				dummy_int[1]=0; //no additional tiles
				break;
				
			case left:
				flip=1;
				tile+=(clk&fr)?2:0;
				xofs=16;
				dummy_int[1]=1; //second tile is next tile
				break;
				
			case right:
				flip=0;
				tile+=(clk&fr)?3:1;
				xofs=16;
				dummy_int[1]=-1; //second tile is next tile
				break;
			}
		}
	}
	break;
	
	case aNEWDONGO:
	{
		int32_t fr4=0;
		
		if(!dying && clk2>0 && clk2<=64)
		{
			// bloated
			if(clk2>=0)
			{
				fr4=3;
			}
			
			if(clk2>=16)
			{
				fr4=2;
			}
			
			if(clk2>=32)
			{
				fr4=1;
			}
			
			if(clk2>=48)
			{
				fr4=0;
			}
			
			switch(dir)
			{
			case up:
				xofs=0;
				tile+=8+fr4;
				dummy_int[1]=0; //no additional tiles
				break;
				
			case down:
				xofs=0;
				tile+=12+fr4;
				dummy_int[1]=0; //no additional tiles
				break;
				
			case left:
				tile+=29+(2*fr4);
				xofs=16;
				dummy_int[1]=-1; //second tile is previous tile
				break;
				
			case right:
				tile+=49+(2*fr4);
				xofs=16;
				dummy_int[1]=-1; //second tile is previous tile
				break;
			}
		}
		else if(!dying || clk2>19)
		{
			// normal
			switch(dir)
			{
			case up:
				xofs=0;
				tile+=((clk&12)>>2);
				dummy_int[1]=0; //no additional tiles
				break;
				
			case down:
				xofs=0;
				tile+=4+((clk&12)>>2);
				dummy_int[1]=0; //no additional tiles
				break;
				
			case left:
				tile+=21+((clk&12)>>1);
				xofs=16;
				dummy_int[1]=-1; //second tile is previous tile
				break;
				
			case right:
				flip=0;
				tile+=41+((clk&12)>>1);
				xofs=16;
				dummy_int[1]=-1; //second tile is previous tile
				break;
			}
		}
	}
	break;
	
	case aDONGOBS:
	{
		int32_t fr4=0;
		
		if(!dying && clk2>0 && clk2<=64)
		{
			// bloated
			if(clk2>=0)
			{
				fr4=3;
			}
			
			if(clk2>=16)
			{
				fr4=2;
			}
			
			if(clk2>=32)
			{
				fr4=1;
			}
			
			if(clk2>=48)
			{
				fr4=0;
			}
			
			switch(dir)
			{
			case up:
				tile+=28+fr4;
				yofs+=8;
				dummy_int[1]=-20; //second tile change
				dummy_int[2]=0;   //new xofs change
				dummy_int[3]=-16; //new xofs change
				break;
				
			case down:
				tile+=12+fr4;
				yofs-=8;
				dummy_int[1]=20; //second tile change
				dummy_int[2]=0;  //new xofs change
				dummy_int[3]=16; //new xofs change
				break;
				
			case left:
				tile+=49+(2*fr4);
				xofs+=8;
				dummy_int[1]=-1; //second tile change
				dummy_int[2]=-16; //new xofs change
				dummy_int[3]=0;  //new xofs change
				break;
				
			case right:
				tile+=69+(2*fr4);
				xofs+=8;
				dummy_int[1]=-1; //second tile change
				dummy_int[2]=-16; //new xofs change
				dummy_int[3]=0;  //new xofs change
				break;
			}
		}
		else if(!dying || clk2>19)
		{
			// normal
			switch(dir)
			{
			case up:
				tile+=20+((clk&24)>>3);
				yofs+=8;
				dummy_int[1]=-20; //second tile change
				dummy_int[2]=0;   //new xofs change
				dummy_int[3]=-16; //new xofs change
				break;
				
			case down:
				tile+=4+((clk&24)>>3);
				yofs-=8;
				dummy_int[1]=20; //second tile change
				dummy_int[2]=0;  //new xofs change
				dummy_int[3]=16; //new xofs change
				break;
				
			case left:
				xofs=-8;
				tile+=40+((clk&24)>>2);
				dummy_int[1]=1; //second tile change
				dummy_int[2]=16; //new xofs change
				dummy_int[3]=0; //new xofs change
				break;
				
			case right:
				tile+=60+((clk&24)>>2);
				xofs=-8;
				dummy_int[1]=1; //second tile change
				dummy_int[2]=16; //new xofs change
				dummy_int[3]=0; //new xofs change
				break;
			}
		}
	}
	break;
	
	case aWIZZ:
	{
//      if(d->misc1)
		if(dmisc1)
		{
			if(clk&8)
			{
				++tile;
			}
		}
		else
		{
			if(frame&4)
			{
				++tile;
			}
		}
		
		switch(dir)
		{
		case 9:
		case 15:
		case up:
			tile+=2;
			break;
			
		case down:
			break;
			
		case 13:
		case left:
			flip=1;
			break;
			
		default:
			flip=0;
			break;
		}
	}
	break;
	
	case aNEWWIZZ:
	{
		tiledir(dir,true);
		
//      if(d->misc1)                                            //walking wizzrobe
		if(dmisc1)                                            //walking wizzrobe
		{
			if(clk&8)
			{
				tile+=2;
			}
			
			if(clk&4)
			{
				tile+=1;
			}
			
			if(!(dummy_bool[1]||dummy_bool[2]))                               //should never be charging or firing for these wizzrobes
			{
				if(dummy_int[1]>0)
				{
					tile+=40;
				}
			}
		}
		else
		{
			if(dummy_bool[1]||dummy_bool[2])
			{
				tile+=20;
				
				if(dummy_bool[2])
				{
					tile+=20;
				}
			}
			
			tile+=((frame>>1)&3);
		}
	}
	break;
	
	case a3FRM:
	{
		n_frame_n_dir(3, 0, (f4==3) ? 1 : f4);
	}
	break;
	
	case a3FRM4DIR:
	{
		n_frame_n_dir(3, 4, (f4==3) ? 1 : f4);
	}
	break;
	
	case aVIRE:
	{
		if(dir==up)
		{
			tile+=2;
		}
		
		tile+=fx;
	}
	break;
	
	case aROPE:
	{
		tile+=(1-fx);
		flip = dir==left ? 1:0;
	}
	break;
	
	case aZORA:
	{
		int32_t dl;
		
		if(clk<36)
		{
			dl=clk+5;
			goto waves2;
		}
		
		if(clk<36+66)
			tile=(dir==up)?o_tile+1:o_tile;
		else
		{
			dl=clk-36-66;
waves2:
			tile=((dl/11)&1)+s_tile;
		}
	}
	break;
	
	case aNEWZORA:
	{
		f4=(clk/16)%4;
		
		tiledir(dir,true);
		int32_t dl;
		
		if((clk>35)&&(clk<36+67))                               //surfaced
		{
			if((clk>=(35+10))&&(clk<(38+56)))                     //mouth open
			{
				tile+=80;
			}                                                     //mouth closed
			else
			{
				tile+=40;
			}
			
			tile+=f4;
		}
		else
		{
			if(clk<36)
			{
				dl=clk+5;
			}
			else
			{
				dl=clk-36-66;
			}
			
			tile+=((dl/5)&3);
		}
	}
	break;
	
	case a4FRM4EYE:
	case a2FRM4EYE:
	case a4FRM8EYE:
	case a4FRM8EYEB: //big version
	case a4FRM4EYEB:
	{
		tilerows = 2;
		int fakex = x + 8*(zc_max(1,txsz)-1);
		int fakey = y + 8*(zc_max(1,tysz)-1);
		double ddir=atan2(double(fakey-(Link.y)),double(Link.x-fakex));
		int32_t lookat=zc_oldrand()&15;
		
		if((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
		{
			lookat=l_down;
		}
		else if((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
		{
			lookat=down;
		}
		else if((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
		{
			lookat=r_down;
		}
		else if((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
		{
			lookat=right;
		}
		else if((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
		{
			lookat=r_up;
		}
		else if((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
		{
			lookat=up;
		}
		else if((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
		{
			lookat=l_up;
		}
		else
		{
			lookat=left;
		}
		
		int32_t dir2 = dir;
		dir = lookat;
		if (anim != a4FRM8EYEB && anim != a4FRM4EYEB) n_frame_n_dir(anim==a2FRM4EYE ? 2:4, anim==a4FRM8EYE ? 8 : 4, anim==a2FRM4EYE ? (f2&1):f4);
		else 
		{
			tiledir_big(dir,(anim == a4FRM4EYEB));
			tile+=2*f4;
			ignore_extend = true;
		}
		dir = dir2;
	}
	break;
	
	case aFLIP:
	{
		flip = f2&1;
	}
	break;
	
	case a2FRM:
	{
		tile += (1-f2);
	}
	break;
	
	case a2FRMB:
	{
		tile+= 2*(1-f2);
		ignore_extend = true;
	}
	break;
	
	case a2FRM4DIR:
	{
		n_frame_n_dir(2, 4, f2&1);
	}
	break;
	
	case a4FRM4DIRF:
	{
		n_frame_n_dir(4,4,f4);
		
		if(clk2>0)                                              //stopped to fire
		{
			tile+=20;
			
			if(clk2<17)                                           //firing
			{
				tile+=20;
			}
		}
	}
	break;
	
	case a4FRM4DIR:
	{
		n_frame_n_dir(4,4,f4);
	}
	break;
	
	case a4FRM8DIRF:
	{
		tilerows = 2;
		n_frame_n_dir(4,8,f4);
		
		if(clk2>0)                                              //stopped to fire
		{
			tile+=40;
			
			if(clk2<17)                                           //firing
			{
				tile+=40;
			}
		}
	}
	break;
	
	case a4FRM8DIRB:
	case a4FRM8DIRFB:
	{
		tilerows = 2;
		tiledir_big(dir,false);
		tile+=2*f4;
		if(clk2>0 && anim == a4FRM8DIRFB)                         //stopped to fire
		{
			tile+=80;
			
			if(clk2<17)                                           //firing
			{
				tile+=80;
			}
		}
		ignore_extend = true;
	}
	break;
	
	case a4FRM4DIRB:
	case a4FRM4DIRFB:
	{
		tilerows = 2;
		tiledir_big(dir,true);
		tile+=2*f4;
		if(clk2>0 && anim == a4FRM4DIRFB)                         //stopped to fire
		{
			tile+=40;
			
			if(clk2<17)                                           //firing
			{
				tile+=40;
			}
		}
		ignore_extend = true;
	}
	break;
	
	case aOCTO:
	{
		switch(dir)
		{
		case up:
			flip = 2;
			break;
			
		case down:
			flip = 0;
			break;
			
		case left:
			flip = 0;
			tile += 2;
			break;
			
		case right:
			flip = 1;
			tile += 2;
			break;
		}
		
		tile+=f2;
	}
	break;
	
	case aWALK:
	{
		switch(dir)
		{
		case up:
			tile+=3;
			flip = f2;
			break;
			
		case down:
			tile+=2;
			flip = f2;
			break;
			
		case left:
			flip=1;
			tile += f2;
			break;
			
		case right:
			flip=0;
			tile += f2;
			break;
		}
	}
	break;
	
	case aDWALK:
	{
		if((get_bit(quest_rules,qr_BRKNSHLDTILES)) && (dummy_bool[1]==true))
		{
			tile=s_tile;
		}
		
		switch(dir)
		{
		case up:
			tile+=2;
			flip=f2;
			break;
			
		case down:
			flip=0;
			tile+=(1-f2);
			break;
			
		case left:
			flip=1;
			tile+=(3+f2);
			break;
			
		case right:
			flip=0;
			tile+=(3+f2);
			break;
		}
	}
	break;
	
	case aTEK:
	{
		if(misc==0)
		{
			tile += f2;
		}
		else if(misc!=1)
		{
			++tile;
		}
	}
	break;
	
	case aNEWTEK:
	{
		if(step<0)                                              //up
		{
			switch(clk3)
			{
			case left:
				flip=0;
				tile+=20;
				break;
				
			case right:
				flip=0;
				tile+=24;
				break;
			}
		}
		else if(step==0)
		{
			switch(clk3)
			{
			case left:
				flip=0;
				tile+=8;
				break;
				
			case right:
				flip=0;
				tile+=12;
				break;
			}
		}                                                       //down
		else
		{
			switch(clk3)
			{
			case left:
				flip=0;
				tile+=28;
				break;
				
			case right:
				flip=0;
				tile+=32;
				break;
			}
		}
		
		if(misc==0)
		{
			tile+=f4;
		}
		else if(misc!=1)
		{
			tile+=2;
		}
	}
	break;
	
	case aARMOS:
	{
		if(!fading)
		{
			tile += fx;
			
			if(dir==up)
				tile += 2;
		}
	}
	break;
	
	case aARMOS4:
	{
		switch(dir)
		{
		case up:
			flip=0;
			break;
			
		case down:
			flip=0;
			tile+=4;
			break;
			
		case left:
			flip=0;
			tile+=8;
			break;
			
		case right:
			flip=0;
			tile+=12;
			break;
		}
		
		if(!fading)
		{
			tile+=f4;
		}
	}
	break;
	
	case aGHINI:
	{
		switch(dir)
		{
		case 8:
		case 9:
		case up:
			++tile;
			flip=0;
			break;
			
		case 15:
			++tile;
			flip=1;
			break;
			
		case 10:
		case 11:
		case right:
			flip=1;
			break;
			
		default:
			flip=0;
			break;
		}
	}
	break;
	
	case a2FRMPOS:
	{
		tile+=posframe;
	}
	break;
	
	case a4FRMPOS4DIR:
	{
		n_frame_n_dir(4,4,0);
		//        tile+=f2;
		tile+=posframe;
	}
	break;
	
	case a4FRMPOS4DIRF:
	{
		n_frame_n_dir(4,4,0);
		
		if(clk2>0)                                              //stopped to fire
		{
			tile+=20;
			
			if(clk2<17)                                           //firing
			{
				tile+=20;
			}
		}
		
		//        tile+=f2;
		tile+=posframe;
	}
	break;
	
	case a4FRMPOS8DIR:
	{
		tilerows = 2;
		int32_t n = tile;
		n_frame_n_dir(4,8,0);
		//        tile+=f2;
		tile+=posframe;
	}
	break;
	
	case a4FRMPOS8DIRF:
	{
		tilerows = 2;
		n_frame_n_dir(4,8,0);
		
		if(clk2>0)                                              //stopped to fire
		{
			tile+=40;
			
			if(clk2<17)                                           //firing
			{
				tile+=40;
			}
		}
		
		tile+=posframe;
	}
	break;
	
	case aNEWLEV:
	{
		tiledir(dir,true);
		
		switch(misc)
		{
		case -1:
		case 0:
			return;
			
		case 1:
		
//        case 5: cs = d->misc2; break;
		case 5:
			cs = dmisc2;
			break;
			
		case 2:
		case 4:
			tile += 20;
			break;
			
		case 3:
			tile += 40;
			break;
		}
		
		tile+=f4;
	}
	break;
	
	case aLEV:
	{
		f4 = ((clk/5)&1);
		
		switch(misc)
		{
		case -1:
		case 0:
			return;
			
		case 1:
		
//        case 5: tile += (f2) ? 1 : 0; cs = d->misc2; break;
		case 5:
			tile += (f2) ? 1 : 0;
			cs = dmisc2;
			break;
			
		case 2:
		case 4:
			tile += 2;
			break;
			
		case 3:
			tile += (f4) ? 4 : 3;
			break;
		}
	}
	break;
	
	case aWALLM:
	{
		if(!dummy_bool[1])
		{
			tile += f2;
		}
	}
	break;
	
	case aNEWWALLM:
	{
		int32_t tempdir=0;
		
		switch(misc)
		{
		case 1:
		case 2:
			tempdir=clk3;
			break;
			
		case 3:
		case 4:
		case 5:
			tempdir=dir;
			break;
			
		case 6:
		case 7:
			tempdir=clk3^1;
			break;
		}
		
		tiledir(tempdir,true);
		
		if(!dummy_bool[1])
		{
			tile+=f4;
		}
	}
	break;
	
	case a4FRMNODIR:
	{
		tile+=f4;
	}
	break;
	
	}                                                         // switch(d->anim)
	
	// flashing
//  if(d->flags2 & guy_flashing)
	if(flags2 & guy_flashing)
	{
		cs = (frame&3) + 6;
	}
	
	if(flags2&guy_transparent)
	{
		drawstyle=1;
	}
	
	int32_t change = tile-o_tile;
	
	if(extend > 2 && (!ignore_extend || get_bit(quest_rules, qr_BROKEN_BIG_ENEMY_ANIMATION)))
	{
		if(o_tile/TILES_PER_ROW==(o_tile+((txsz*change)/tilerows))/TILES_PER_ROW)
		{
			tile=o_tile+txsz*change;
		}
		else
		{
			tile=o_tile+(txsz*change)+((tysz-1)*TILES_PER_ROW)*(((o_tile+txsz*change)/TILES_PER_ROW)-(o_tile/TILES_PER_ROW));
		}
	}
	else
	{
		tile=o_tile+change;
	}
}

int32_t wpnsfx(int32_t wpn)
{
	switch(wpn)
	{
	case ewFireTrail:
	case ewFlame:
	case ewFlame2Trail:
	case ewFlame2:
		return WAV_FIRE;
		
	case ewWind:
	case ewMagic:
		return WAV_WAND;
		
	case ewIce:
		return WAV_ZN1ICE;
		
	case ewRock:
		if(get_bit(quest_rules,qr_MORESOUNDS)) return WAV_ZN1ROCK;
		break;
		
	case ewFireball2:
	case ewFireball:
		if(get_bit(quest_rules,qr_MORESOUNDS)) return WAV_ZN1FIREBALL;
	}
	
	return -1;
}

int32_t enemy::run_script(int32_t mode)
{
	if (script <= 0 || !doscript || FFCore.getQuestHeaderInfo(vZelda) < 0x255 || FFCore.system_suspend[susptNPCSCRIPTS])
		return RUNSCRIPT_OK;
	int32_t ret = RUNSCRIPT_OK;
	alloc_scriptmem();
	switch(mode)
	{
		case MODE_NORMAL:
			return ZScriptVersion::RunScript(SCRIPT_NPC, script, getUID());
		case MODE_WAITDRAW:
			if(waitdraw)
			{
				ret = ZScriptVersion::RunScript(SCRIPT_NPC, script, getUID());
				waitdraw = 0;
			}
			break;
	}
	return ret;
}

/********************************/
/*********  Guy Class  **********/
/********************************/

// good guys, fires, fairy, and other non-enemies
// based on enemy class b/c guys in dungeons act sort of like enemies
// also easier to manage all the guys this way
guy::guy(zfix X,zfix Y,int32_t Id,int32_t Clk,bool mg) : enemy(X,Y,Id,Clk)
{
	mainguy=mg;
	canfreeze=false;
	dir=down;
	yofs=playing_field_offset;
	hxofs=2;
	hzsz=8;
	hxsz=12;
	hysz=17;
	
	if(!superman && (!isdungeon() || id==gFAIRY || id==gFIRE || id==gZELDA))
	{
		superman = 1;
		hxofs=1000;
	}
}

bool guy::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(mainguy && clk==0 && misc==0)
	{
		setupscreen();
		misc = 1;
	}
	
	if(mainguy && fadeclk==0)
		return true;
		
	hp=256;                                                   // good guys never die...
	
	if(hclk && !clk2)
	{
		// but if they get hit...
		++clk2;                                                 // only do this once
		
		if(!get_bit(quest_rules,qr_NOGUYFIRES))
		{
			addenemy(BSZ?64:72,68,eSHOOTFBALL,0);
			addenemy(BSZ?176:168,68,eSHOOTFBALL,0);
		}
	}
	
	return enemy::animate(index);
}

void guy::draw(BITMAP *dest)
{
	update_enemy_frame();
	
	if(!mainguy || fadeclk<0 || fadeclk&1)
		enemy::draw(dest);
}

/*******************************/
/*********   Enemies   *********/
/*******************************/

eFire::eFire(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	clk4=0;
	shield= (flags&(inv_left | inv_right | inv_back |inv_front)) != 0;
	// Spawn type
	if(flags & guy_fadeflicker)
	{
		clk=0;
		superman = 1;
		fading=fade_flicker;
		if (!(editorflags&ENEMY_FLAG3)) count_enemy=false;
		dir=down;
		
		if(!canmove(down,(zfix)8,spw_none,false))
			clk3=int32_t(13.0/step);
	}
	else if(flags & guy_fadeinstant)
	{
		clk=0;
	}
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) d->zofs = (int32_t)zofs;
}

bool eFire::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(fallclk||drownclk) return enemy::animate(index);
	if(fading)
	{
		if(++clk4 > 60)
		{
			clk4=0;
			superman=0;
			fading=0;
			
			if(flags2&cmbflag_armos && z==0)
				removearmos(x,y);
				
			clk2=0;
			
			if(!canmove(down,(zfix)8,spw_none,false))
			{
				dir=0;
				y = y.getInt() & 0xF0;
			}
			
			return Dead(index);
		}
		else if(flags2&cmbflag_armos && z==0 && clk==0)
			removearmos(x,y);
	}
	
	return enemy::animate(index);
}

void eFire::draw(BITMAP *dest)
{
	update_enemy_frame();
	enemy::draw(dest);
}

int32_t eFire::takehit(weapon *w)
{
	int32_t wpnId = w->id;
	int32_t wpnDir = w->dir;
	
	if(wpnId==wHammer && shield && (flags & guy_bkshield)
			&& ((flags&inv_front && wpnDir==(dir^down)) || (flags&inv_back && wpnDir==(dir^up))
				|| (flags&inv_left && wpnDir==(dir^left)) || (flags&inv_right && wpnDir==(dir^right))))
	{
		shield = false;
		flags &= ~(inv_left|inv_right|inv_back|inv_front);
		
		if(get_bit(quest_rules,qr_BRKNSHLDTILES))
			o_tile=s_tile;
	}
	
	int32_t ret = enemy::takehit(w);
	return ret;
}

void eFire::break_shield()
{
	if(!shield)
		return;
		
	flags&=~(inv_front | inv_back | inv_left | inv_right);
	shield=false;
	
	if(get_bit(quest_rules,qr_BRKNSHLDTILES))
		o_tile=s_tile;
}

eOther::eOther(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	//zprint2("npct other::other\n");
	clk4=0;
	shield= (flags&(inv_left | inv_right | inv_back |inv_front)) != 0;
	
	// Spawn type
	if(flags & guy_fadeflicker)
	{
		clk=0;
		superman = 1;
		fading=fade_flicker;
		if (!(editorflags&ENEMY_FLAG3)) count_enemy=false;
		dir=down;
		
		if(!canmove(down,(zfix)8,spw_none,false))
			clk3=int32_t(13.0/step);
	}
	else if(flags & guy_fadeinstant)
	{
		clk=0;
	}
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)d->zofs;
}

bool eOther::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(fallclk||drownclk) return enemy::animate(index);
	//zprint2("npct other::animate\n");
	if(fading)
	{
		if(++clk4 > 60)
		{
			clk4=0;
			superman=0;
			fading=0;
			
			if(flags2&cmbflag_armos && z==0)
				removearmos(x,y);
				
			clk2=0;
			
			if(!canmove(down,(zfix)8,spw_none,false))
			{
				dir=0;
				y = y.getInt() & 0xF0;
			}
			
			return Dead(index);
		}
		else if(flags2&cmbflag_armos && z==0 && clk==0)
			removearmos(x,y);
	}
	
	return enemy::animate(index);
}

void eOther::draw(BITMAP *dest)
{
	update_enemy_frame();
	enemy::draw(dest);
}

int32_t eOther::takehit(weapon *w)
{
	int32_t wpnId = w->id;
	int32_t wpnDir = w->dir;
	
	if(wpnId==wHammer && shield && (flags & guy_bkshield)
			&& ((flags&inv_front && wpnDir==(dir^down)) || (flags&inv_back && wpnDir==(dir^up))
				|| (flags&inv_left && wpnDir==(dir^left)) || (flags&inv_right && wpnDir==(dir^right))))
	{
		shield = false;
		flags &= ~(inv_left|inv_right|inv_back|inv_front);
		
		if(get_bit(quest_rules,qr_BRKNSHLDTILES))
			o_tile=s_tile;
	}
	
	int32_t ret = enemy::takehit(w);
	return ret;
}

void eOther::break_shield()
{
	if(!shield)
		return;
		
	flags&=~(inv_front | inv_back | inv_left | inv_right);
	shield=false;
	
	if(get_bit(quest_rules,qr_BRKNSHLDTILES))
		o_tile=s_tile;
}


eScript::eScript(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	clk4=0;
	shield= (flags&(inv_left | inv_right | inv_back |inv_front)) != 0;
	
	// Spawn type
	if(flags & guy_fadeflicker)
	{
		clk=0;
		superman = 1;
		fading=fade_flicker;
		if (!(editorflags&ENEMY_FLAG3)) count_enemy=false;
		dir=down;
		
		if(!canmove(down,(zfix)8,spw_none,false))
			clk3=int32_t(13.0/step);
	}
	else if(flags & guy_fadeinstant)
	{
		clk=0;
	}
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)d->zofs;
}

bool eScript::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(fallclk||drownclk) return enemy::animate(index);
	if(fading)
	{
		if(++clk4 > 60)
		{
			clk4=0;
			superman=0;
			fading=0;
			
			if(flags2&cmbflag_armos && z==0)
				removearmos(x,y);
				
			clk2=0;
			
			if(!canmove(down,(zfix)8,spw_none,false))
			{
				dir=0;
				y = y.getInt() & 0xF0;
			}
			
			return Dead(index);
		}
		else if(flags2&cmbflag_armos && z==0 && clk==0)
			removearmos(x,y);
	}
	
	return enemy::animate(index);
}

void eScript::draw(BITMAP *dest)
{
	update_enemy_frame();
	enemy::draw(dest);
}

int32_t eScript::takehit(weapon *w)
{
	int32_t wpnId = w->id;
	int32_t wpnDir = w->dir;
	
	if(wpnId==wHammer && shield && (flags & guy_bkshield)
			&& ((flags&inv_front && wpnDir==(dir^down)) || (flags&inv_back && wpnDir==(dir^up))
				|| (flags&inv_left && wpnDir==(dir^left)) || (flags&inv_right && wpnDir==(dir^right))))
	{
		shield = false;
		flags &= ~(inv_left|inv_right|inv_back|inv_front);
		
		if(get_bit(quest_rules,qr_BRKNSHLDTILES))
			o_tile=s_tile;
	}
	
	int32_t ret = enemy::takehit(w);
	return ret;
}

void eScript::break_shield()
{
	if(!shield)
		return;
		
	flags&=~(inv_front | inv_back | inv_left | inv_right);
	shield=false;
	
	if(get_bit(quest_rules,qr_BRKNSHLDTILES))
		o_tile=s_tile;
}


eFriendly::eFriendly(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	clk4=0;
	hyofs = -32768; //No hitbox initially.
	shield= (flags&(inv_left | inv_right | inv_back |inv_front)) != 0;
	
	// Spawn type
	if(flags & guy_fadeflicker)
	{
		clk=0;
		superman = 1;
		fading=fade_flicker;
		if (!(editorflags&ENEMY_FLAG3)) count_enemy=false;
		dir=down;
		
		if(!canmove(down,(zfix)8,spw_none,false))
			clk3=int32_t(13.0/step);
	}
	else if(flags & guy_fadeinstant)
	{
		clk=0;
	}
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)d->zofs;
}

bool eFriendly::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(fallclk||drownclk) return enemy::animate(index);
	if(fading)
	{
		if(++clk4 > 60)
		{
			clk4=0;
			superman=0;
			fading=0;
			
			if(flags2&cmbflag_armos && z==0)
				removearmos(x,y);
				
			clk2=0;
			
			if(!canmove(down,(zfix)8,spw_none,false))
			{
				dir=0;
				y = y.getInt() & 0xF0;
			}
			
			return Dead(index);
		}
		else if(flags2&cmbflag_armos && z==0 && clk==0)
			removearmos(x,y);
	}
	
	return enemy::animate(index);
}

void eFriendly::draw(BITMAP *dest)
{
	update_enemy_frame();
	enemy::draw(dest);
}

int32_t eFriendly::takehit(weapon *w)
{
	int32_t wpnId = w->id;
	int32_t wpnDir = w->dir;
	
	if(wpnId==wHammer && shield && (flags & guy_bkshield)
			&& ((flags&inv_front && wpnDir==(dir^down)) || (flags&inv_back && wpnDir==(dir^up))
				|| (flags&inv_left && wpnDir==(dir^left)) || (flags&inv_right && wpnDir==(dir^right))))
	{
		shield = false;
		flags &= ~(inv_left|inv_right|inv_back|inv_front);
		
		if(get_bit(quest_rules,qr_BRKNSHLDTILES))
			o_tile=s_tile;
	}
	
	int32_t ret = enemy::takehit(w);
	return ret;
}

void eFriendly::break_shield()
{
	if(!shield)
		return;
		
	flags&=~(inv_front | inv_back | inv_left | inv_right);
	shield=false;
	
	if(get_bit(quest_rules,qr_BRKNSHLDTILES))
		o_tile=s_tile;
}


void enemy::removearmos(int32_t ax,int32_t ay)
{
	if(did_armos)
	{
		return;
	}
	
	did_armos=true;
	ax&=0xF0;
	ay&=0xF0;
	int32_t cd = (ax>>4)+ay;
	int32_t f = MAPFLAG(ax,ay);
	int32_t f2 = MAPCOMBOFLAG(ax,ay);
	
	if(combobuf[tmpscr->data[cd]].type!=cARMOS)
	{
		return;
	}
	
	tmpscr->data[cd] = tmpscr->undercombo;
	tmpscr->cset[cd] = tmpscr->undercset;
	tmpscr->sflag[cd] = 0;
	
	if(f == mfARMOS_SECRET || f2 == mfARMOS_SECRET)
	{
		tmpscr->data[cd] = tmpscr->secretcombo[sSTAIRS];
		tmpscr->cset[cd] = tmpscr->secretcset[sSTAIRS];
		tmpscr->sflag[cd]=tmpscr->secretflag[sSTAIRS];
		sfx(tmpscr->secretsfx);
	}
	
	if(f == mfARMOS_ITEM || f2 == mfARMOS_ITEM)
	{
		if(!getmapflag((currscr < 128 && get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)) ? mITEM : mBELOW) || (tmpscr->flags9&fBELOWRETURN))
		{
			additem(ax,ay,tmpscr->catchall, (ipONETIME2 + ipBIGRANGE) | ((tmpscr->flags3&fHOLDITEM) ? ipHOLDUP : 0) | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0));
			sfx(tmpscr->secretsfx);
		}
	}
	
	putcombo(scrollbuf,ax,ay,tmpscr->data[cd],tmpscr->cset[cd]);
}

eGhini::eGhini(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	fading=fade_flicker;
	if (!(editorflags&ENEMY_FLAG3)) count_enemy=false;
	dir=12;
	movestatus=1;
	step=0;
	clk=0;
	clk4=0;
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)d->zofs;
}

bool eGhini::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(fallclk||drownclk) return enemy::animate(index);
	if(dying)
		return Dead(index);
		
	if(dmisc1)
	{
		if(misc)
		{
			if(clk4>160)
				misc=2;
				
			floater_walk((misc==1)?0:rate,hrate,zslongToFix(dstep*100),zslongToFix(dstep*10),10,dmisc16,dmisc17); //120,10);
			removearmos(x,y);
		}
		else if(clk4>=60)
		{
			misc=1;
			clk3=32;
			fading=0;
			guygrid[(int32_t(y)&0xF0)+(int32_t(x)>>4)]=0;
			removearmos(x,y);
		}
	}
	
	clk4++;
	
	return enemy::animate(index);
}

void eGhini::draw(BITMAP *dest)
{
	update_enemy_frame();
	enemy::draw(dest);
}

void eGhini::kickbucket()
{
	hp=-1000;                                                 // don't call death_sfx()
}

eTektite::eTektite(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	old_y=y;
	dir=down;
	misc=1;
	clk=-15;
	
	if(!BSZ)
		clk*=zc_oldrand()%3+1;
		
	// avoid divide by 0 errors
	if(dmisc1 == 0)
		dmisc1 = 24;
		
	if(dmisc2 == 0)
		dmisc2 = 3;
		
	//nets+760;
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)d->zofs;
}

bool eTektite::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(fallclk||drownclk) return enemy::animate(index);
	if(dying)
		return Dead(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	if(get_bit(quest_rules,qr_ENEMIESZAXIS))
	{
		y=floor_y;
	}
	
	if(clk>=0 && !stunclk && !frozenclock && (!watch || misc==0))
	{
		switch(misc)
		{
		case 0:                                               // normal
			if(!(zc_oldrand()%dmisc1))
			{
				misc=1;
				clk2=32;
			}
			
			break;
			
		case 1:                                               // waiting to pounce
			if(--clk2<=0)
			{
				int32_t r=zc_oldrand();
				misc=2;
				step=0-(zslongToFix(dstep*100));                           // initial speed
				clk3=(r&1)+2;                                       // left or right
				clk2start=clk2=(r&31)+10;                           // flight time
				
				if(y<32)  clk2+=2;                                  // make them come down from top of screen
				
				if(y>112) clk2-=2;                                  // make them go back up
				
				cstart=c = 9-((r&31)>>3);                           // time before gravity kicks in
			}
			
			break;
			
		case 2:                                                 // in flight
			move(step);
			
			if(step>0)                                            //going down
			{
				if(COMBOTYPE(x+8,y+16)==cNOJUMPZONE)
				{
					step=0-step;
				}
				else if(COMBOTYPE(x+8,y+16)==cNOENEMY)
				{
					step=0-step;
				}
				else if(ispitfall(x+8,y+16))
				{
					step=0-step;
				}
				else if(MAPFLAG(x+8,y+16)==mfNOENEMY)
				{
					step=0-step;
				}
				else if(MAPCOMBOFLAG(x+8,y+16)==mfNOENEMY)
				{
					step=0-step;
				}
			}
			else if(step<0)
			{
				if(COMBOTYPE(x+8,y)==cNOJUMPZONE)
				{
					step=0-step;
				}
				else if(COMBOTYPE(x+8,y)==cNOENEMY)
				{
					step=0-step;
				}
				else if(ispitfall(x+8,y))
				{
					step=0-step;
				}
				else if(MAPFLAG(x+8,y)==mfNOENEMY)
				{
					step=0-step;
				}
				else if(MAPCOMBOFLAG(x+8,y)==mfNOENEMY)
				{
					step=0-step;
				}
			}
			
			if(clk3==left)
			{
				if(COMBOTYPE(x,y+8)==cNOJUMPZONE)
				{
					clk3^=1;
				}
				else if(COMBOTYPE(x,y+8)==cNOENEMY)
				{
					clk3^=1;
				}
				else if(ispitfall(x,y+8))
				{
					clk3^=1;
				}
				else if(MAPFLAG(x,y+8)==mfNOENEMY)
				{
					clk3^=1;
				}
				else if(MAPCOMBOFLAG(x,y+8)==mfNOENEMY)
				{
					clk3^=1;
				}
			}
			else
			{
				if(COMBOTYPE(x+16,y+8)==cNOJUMPZONE)
				{
					clk3^=1;
				}
				else if(COMBOTYPE(x+16,y+8)==cNOENEMY)
				{
					clk3^=1;
				}
				else if(ispitfall(x+16,y+8))
				{
					clk3^=1;
				}
				else if(MAPFLAG(x+16,y+8)==mfNOENEMY)
				{
					clk3^=1;
				}
				else if(MAPCOMBOFLAG(x+16,y+8)==mfNOENEMY)
				{
					clk3^=1;
				}
			}
			
			--c;
			
			if(c<0 && step<zslongToFix(dstep*100))
			{
				step+=zslongToFix(dmisc3*100);
			}
			
			int32_t nb=get_bit(quest_rules,qr_NOBORDER) ? 16 : 0;
			
			if(x<=16-nb)  clk3=right;
			
			if(x>=224+nb) clk3=left;
			
			x += (clk3==left) ? -1 : 1;
			
			if((--clk2<=0 && y>=16-nb) || y>=144+nb)
			{
				if(y>=144+nb && get_bit(quest_rules,qr_ENEMIESZAXIS))
				{
					step=0-step;
					y--;
				}
				else if(zc_oldrand()%dmisc2)                                 //land and wait
				{
					clk=misc=0;
				}                                                   //land and jump again
				else
				{
					misc=1;
					clk2=0;
				}
			}
			
			break;
		}                                                         // switch
	}
	
	if(get_bit(quest_rules,qr_ENEMIESZAXIS) && misc==2)
	{
		int32_t tempy = floor_y;
		z=zc_max(0,zc_min(clk2start-clk2,clk2));
		floor_y = y;
		y=tempy-z;
		old_y = y;
	}
	
	if(stunclk && (clk&31)==1)
		clk=0;
		
	return enemy::animate(index);
}

void eTektite::drawshadow(BITMAP *dest,bool translucent)
{
	if(z<1 && get_bit(quest_rules,qr_ENEMIESZAXIS))
		return;
		
	int32_t tempy=yofs;
	int32_t fdiv = frate/4;
	int32_t efrate = fdiv == 0 ? 0 : clk/fdiv;
	int32_t f2=get_bit(quest_rules,qr_NEWENEMYTILES)?
		   efrate:((clk>=(frate>>1))?1:0);
	flip = 0;
	shadowtile = wpnsbuf[spr_shadow].newtile;
	
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
		if(misc==0)
		{
			shadowtile+=f2;
		}
		else if(misc!=1)
			shadowtile+=2;
	}
	else
	{
		if(misc==0)
		{
			shadowtile += f2 ? 1 : 0;
		}
		else if(misc!=1)
		{
			++shadowtile;
		}
	}
	
	yofs+=8;
	
	if(!get_bit(quest_rules,qr_ENEMIESZAXIS) && misc==2)
	{
		yofs+=zc_max(0,zc_min(clk2start-clk2,clk2));
	}
	if(!shadow_overpit(this))
		enemy::drawshadow(dest,translucent);
	yofs=tempy;
}

void eTektite::draw(BITMAP *dest)
{
	update_enemy_frame();
	enemy::draw(dest);
}

eItemFairy::eItemFairy(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	step=zslongToFix(guysbuf[id&0xFFF].step*100);
	superman=1;
	dir=8;
	hxofs=1000;
	mainguy=false;
	count_enemy=false;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)zofs;
}

bool eItemFairy::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(dying)
		return Dead(index);
		
	//if(clk>32)
	misc=1;
	bool w=watch;
	watch=false;
	variable_walk_8(misc?3:0,0,8,spw_floater);
	watch=w;
	
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	return enemy::animate(index);
}

void eItemFairy::draw(BITMAP *dest)
{
	//these are here to bypass compiler warnings about unused arguments
	dest=dest;
}

ePeahat::ePeahat(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	//floater_walk(int32_t rate,int32_t newclk,zfix ms,zfix ss,int32_t s,int32_t p, int32_t g)
	floater_walk(misc?rate:0,      hrate, zslongToFix(dstep*100),zslongToFix(dstep*10), 10, dmisc16,dmisc17); // 80, 16);
	dir=8;
	movestatus=1;
	clk=0;
	step=0;
	//nets+720;
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)d->zofs;
}

bool ePeahat::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(fallclk||drownclk) return enemy::animate(index);
	if(slide())
	{
		return false;
	}
	
	if(dying)
		return Dead(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	if(stunclk==0 && clk>96)
		misc=1;
	
	if(!watch)
		floater_walk(misc?rate:0,      hrate, zslongToFix(dstep*100),zslongToFix(dstep*10), 10,  80, 16);
	
	if(get_bit(quest_rules,qr_ENEMIESZAXIS) && !(isSideViewGravity()))
	{
		z=int32_t(step*1.1/((zslongToFix(dstep*10))*1.1));
	}
	
	if(watch && get_bit(quest_rules,qr_PEAHATCLOCKVULN))
		superman=0;
	else
	superman=(movestatus && !get_bit(quest_rules,qr_ENEMIESZAXIS)) ? 1 : 0;
	//stunclk=0; //Not sure what was going on here, or what was intended. Why was this set to 0? -Z
	if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x250 )
	{
	if ( stunclk ) --stunclk;
	}
	else stunclk = 0; //Was probably this way in 2.10 quests. if not, then we never need to clear it. -Z
	//Pretty sure this was always an error. -Z ( 14FEB2019 )
	
	
	if(x<16) dir=right; //this is ugly, but so is moving or creating these guys with scripts.
	
	return enemy::animate(index);
}

void ePeahat::drawshadow(BITMAP *dest, bool translucent)
{
	int32_t tempy=yofs;
	flip = 0;
	shadowtile = wpnsbuf[spr_shadow].newtile+posframe;
	
	if(!get_bit(quest_rules,qr_ENEMIESZAXIS))
	{
		yofs+=8;
		yofs+=int32_t(step/zslongToFix(dstep*10));
	}
	if(!shadow_overpit(this))
		enemy::drawshadow(dest,translucent);
	yofs=tempy;
}

void ePeahat::draw(BITMAP *dest)
{
	update_enemy_frame();
	enemy::draw(dest);
}

int32_t ePeahat::takehit(weapon *w)
{
	int32_t wpnId = w->id;
	int32_t enemyHitWeapon = w->parentitem;
	
	if(dying || clk<0 || hclk>0)
		return 0;
		
	if(superman && !(wpnId==wSBomb)            // vulnerable to super bombs
			// fire boomerang, for nailing peahats
			&& !(wpnId==wBrang && (enemyHitWeapon>-1 ? itemsbuf[enemyHitWeapon].power : current_item_power(itype_brang))>0))
		return 0;
		
	// Time for a kludge...
	int32_t s = superman;
	superman = 0;
	int32_t ret = enemy::takehit(w);
	superman = s;
	
	// Anyway...
	if(stunclk == 160)
	{
		clk2=0;
		movestatus=0;
		misc=0;
		clk=0;
		step=0;
	}
	
	return ret;
}

// auomatically kill off enemy (for rooms with ringleaders)
void ePeahat::kickbucket()
{
	hp=-1000;                                               // don't call death_sfx()
}

eLeever::eLeever(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
//  if(d->misc1==0) { misc=-1; clk-=16; } //Line of Sight leevers
	if(dmisc1==0)
	{
		misc=-1;    //Line of Sight leevers
		clk-=16;
	}
	//nets+1460;
	temprule=(get_bit(quest_rules,qr_NEWENEMYTILES)) != 0;
	submerged = 0;
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)d->zofs;
}

bool eLeever::isSubmerged()
{
	Z_scripterrlog("misc is: %d\n", misc);
	return misc <= 0;
	
}

bool eLeever::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(fallclk||drownclk)
	{
		return enemy::animate(index);
	}
	if(dying)
		return Dead(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	if(clk>=0 && !slide())
	{
//    switch(d->misc1)
		switch(dmisc1)
		{
		case 0:      //line of sight
		case 2:
			switch(misc) //is this leever active
			{
			case -1:  //submerged
			{
		
				if((dmisc1==2)&&(zc_oldrand()&255))
				{
					break;
				}
				
				int32_t active=0;
				
				for(int32_t i=0; i<guys.Count(); i++)
				{
					if(guys.spr(i)->id==id && (((enemy*)guys.spr(i))->misc>=0))
					{
						++active;
					}
				}
				
				if(active<((dmisc1==2)?1:2))
				{
					misc=0; //activate this one
				}
			}
			break;
			
			case 0:
			{
		
				int32_t s=0;
				
				for(int32_t i=0; i<guys.Count(); i++)
				{
					if(guys.spr(i)->id==id && ((enemy*)guys.spr(i))->misc==1)
					{
						++s;
					}
				}
				
				if(s>0)
				{
					break;
				}
				
				int32_t d2=zc_oldrand()&1;
				
				if(LinkDir()>=left)
				{
					d2+=2;
				}
				
				if(canplace(d2) || canplace(d2^1))
				{
					misc=1;
					clk2=0;
					clk=0;
				}
			}
			break;
			
			case 1:
		
				if(++clk2>16) misc=2;
				
				break;
				
			case 2:
		
				if(++clk2>24) misc=3;
				
				break;
				
//        case 3: if(stunclk) break; if(scored) dir^=1; if(!canmove(dir,false)) misc=4; else move((zfix)(d->step/100.0)); break;
			case 3:
		
				if(stunclk || frozenclock) break;
				
				if(scored) dir^=1;
				
				if(!canmove(dir,false)) misc=4;
				else move(zslongToFix(dstep*100));
				
				break;
				
			case 4:
		
				if(--clk2==16)
				{
					misc=5;
					clk=8;
				}
				
				break;
				
			case 5:
		
				if(--clk2==0)  misc=((dmisc1==2)?-1:0);
				
				break;
			}                                                       // switch(misc)
			
			break;
			
		default:  //random
//      step=d->misc3/100.0;
	   
			step=zslongToFix(dmisc3*100);
			++clk2;
			
			if(clk2<32)    misc=1;
			else if(clk2<48)    misc=2;
			else if(clk2<300)
			{
				/*if(misc==2 && (int32_t)(dmisc3*0.48)%8)
				{
				  fix_coords();
				}*/
				misc=3;
				step = zslongToFix(dstep*100);
			}
			else if(clk2<316)   misc=2;
			else if(clk2<412)   misc=1;
			else if(clk2<540)
			{
				misc=0;
				step=0;
			}
			else clk2=0;
			
			if(clk2==48) clk=0;
			
//      variable_walk(d->rate, d->homing, 0);
			variable_walk(rate, homing, 0);
		}                                                         // switch(dmisc1)
	}
	
	hxofs=(misc>=2)?0:1000;
	return enemy::animate(index);
}

bool eLeever::canplace(int32_t d2)
{
	int32_t nx=LinkX();
	int32_t ny=LinkY();
	
	if(d2<left) ny&=0xF0;
	else       nx&=0xF0;
	
	switch(d2)
	{
//    case up:    ny-=((d->misc1==0)?32:48); break;
//    case down:  ny+=((d->misc1==0)?32:48); if(ny-LinkY()<32) ny+=((d->misc1==0)?16:0); break;
//    case left:  nx-=((d->misc1==0)?32:48); break;
//    case right: nx+=((d->misc1==0)?32:48); if(nx-LinkX()<32) nx+=((d->misc1==0)?16:0); break;
	case up:
		ny-=((dmisc1==0||dmisc1==2)?32:48);
		break;
		
	case down:
		ny+=((dmisc1==0||dmisc1==2)?32:48);
		
		if(ny-LinkY()<32) ny+=((dmisc1==0||dmisc1==2)?16:0);
		
		break;
		
	case left:
		nx-=((dmisc1==0||dmisc1==2)?32:48);
		break;
		
	case right:
		nx+=((dmisc1==0||dmisc1==2)?32:48);
		
		if(nx-LinkX()<32) nx+=((dmisc1==0||dmisc1==2)?16:0);
		
		break;
	}
	
	if(m_walkflag(nx,ny,spw_halfstep, dir)||m_walkflag(nx,ny-8,spw_halfstep, dir))                         /*none*/
		return false;
		
	if(d2>=left)
		if(m_walkflag(LinkX(),LinkY(),spw_halfstep, dir)||m_walkflag(LinkX(),LinkY()-8,spw_halfstep, dir))                         /*none*/
			return false;
			
	x=nx;
	y=ny;
	dir=d2^1;
	return true;
}

void eLeever::draw(BITMAP *dest)
{
//  cs=d->cset;
	cs=dcset;
	update_enemy_frame();
	if(!fallclk&&!drownclk)
	{
		switch(misc)
		{
		case -1:
		case 0:
			return;
		}
	}
	
	enemy::draw(dest);
}

eWallM::eWallM(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	//zprint2("eWallM::eWallM\n");
	haslink=false;
	//nets+1000;
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)d->zofs;
}

bool eWallM::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(fallclk||drownclk)
	{
		return enemy::animate(index);
	}
	if(dying)
		return Dead(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	hxofs=1000;
	if(misc==0) //inside wall, ready to spawn?
	{
	//zprint2("Wallmaster is ready to spawn, clk is: %d\n",clk);
	//zprint2("frame is: %d\n",frame);
	//zprint2("wallm_load_clk is: %d\n",wallm_load_clk);
		if(frame-wallm_load_clk>80 && clk>=0)
		{
		//zprint2("getting wall\n");
			int32_t wall=link_on_wall();
		//zprint2("Wallmaster wall is %d\n",wall);
			int32_t wallm_cnt=0;
			
			for(int32_t i=0; i<guys.Count(); i++)
				if(((enemy*)guys.spr(i))->family==eeWALLM)
				{
					int32_t m=((enemy*)guys.spr(i))->misc;
					
					if(m && ((enemy*)guys.spr(i))->clk3==(wall^1))
					{
						++wallm_cnt;
					}
				}
				
			if(wall>0)
			{
				--wall;
				misc=1; //emerging from the wall?
		//zprint2("Wallmaster is emerging\n");
				clk2=0;
				clk3=wall^1;
				wallm_load_clk=frame;
				
				if(wall<=down)
				{
					if(LinkDir()==left)
						dir=right;
					else
						dir=left;
				}
				else
				{
					if(LinkDir()==up)
						dir=down;
					else
						dir=up;
				}
				
				switch(wall)
				{
				case up:
					y=0;
					break;
					
				case down:
					y=160;
					break;
					
				case left:
					x=0;
					break;
					
				case right:
					x=240;
					break;
				}
		
		//zprint2("Wallmaster (p1) x is %d\n",x);
		//zprint2("Wallmaster (p1) y is %d\n",y);
				
				switch(dir)
				{
				case up:
					y=(LinkY()+48-(wallm_cnt&1)*12);
					flip=wall&1;
					break;
					
				case down:
					y=(LinkY()-48+(wallm_cnt&1)*12);
					flip=((wall&1)^1)+2;
					break;
					
				case left:
					x=(LinkX()+48-(wallm_cnt&1)*12);
					flip=(wall==up?2:0)+1;
					break;
					
				case right:
					x=(LinkX()-48+(wallm_cnt&1)*12);
					flip=(wall==up?2:0);
					break;
				}
		
		//zprint2("Wallmaster (p2) x is %d\n",x);
		//zprint2("Wallmaster (p2) y is %d\n",y);
			}
		}
	}
	else
		wallm_crawl();
		
	return enemy::animate(index);
}

void eWallM::wallm_crawl()
{
	bool w=watch;
	hxofs=0;
	
	if(slide())
	{
		return;
	}
	
	//  if(dying || watch || (!haslink && stunclk))
	if(dying || (!haslink && ( stunclk || frozenclock )))
	{
		return;
	}
	
	watch=false;
	++clk2;
	// Misc1: slightly different movement
	//zprint2("wallmaster crawl\n");
	//zprint2("wallmaster tmpdstep is %d\n",tmpdstep);
	float tmpmisc3 = ((40.0/(int32_t)dstep)*40);
	
	//int32_t tmpmisc = int32_t((40.0/dstep)*40);
	//zprint2("wallmaster crawl tmpmisc is: %d\n", tmpmisc);
	//zprint2("wallmaster crawl tmpmisc4 is: %d\n", tmpmisc4);
	misc=(clk2/(dmisc1==1?40:(int32_t)tmpmisc3))+1;
	//zprint2("wallmaster crawl misc is: %d\n", misc);
	if(w&&misc>=3&&misc<=5)
	{
		--clk2;
	}
	
	switch(misc)
	{
	case 1:
	case 2:
		zc_swap(dir,clk3);
		move(step);
		zc_swap(dir,clk3);
		break;
		
	case 3:
	case 4:
	case 5:
		if(w)
		{
			watch=w;
			return;
		}
		
		move(step);
		break;
		
	case 6:
	case 7:
		zc_swap(dir,clk3);
		dir^=1;
		move(step);
		dir^=1;
		zc_swap(dir,clk3);
		break;
		
	default:
		misc=0;
		break;
	}
	
	watch=w;
}

void eWallM::grablink()
{
	haslink=true;
	superman=1;
}

void eWallM::draw(BITMAP *dest)
{
	dummy_bool[1]=haslink;
	update_enemy_frame();
	
	if(misc>0 || fallclk||drownclk)
	{
		masked_draw(dest,16,playing_field_offset+16,224,144);
	}
	
	//    enemy::draw(dest);
	//    tile = clk&8 ? 128:129;
}

bool eWallM::isSubmerged()
{
	return ( !misc );
}

eTrap::eTrap(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	ox=x;                                                     //original x
	oy=y;                                                     //original y
	if(get_bit(quest_rules,qr_TRAPPOSFIX))
	{
		yofs = playing_field_offset;
	}
	
	mainguy=false;
	if (!(editorflags&ENEMY_FLAG3)) count_enemy=false;
	//nets+420;
	dummy_int[1]=0;
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)d->zofs;
}

bool eTrap::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(fallclk||drownclk) return enemy::animate(index);
	if(clk<0)
		return enemy::animate(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	if(misc==0)                                               // waiting
	{
		ox = x;
		oy = y;
		double ddir=atan2(double(y-(Link.y)),double(Link.x-x));
		
		if((ddir<=(((-1)*PI)/4))&&(ddir>(((-3)*PI)/4)))
		{
			dir=down;
		}
		else if((ddir<=(((1)*PI)/4))&&(ddir>(((-1)*PI)/4)))
		{
			dir=right;
		}
		else if((ddir<=(((3)*PI)/4))&&(ddir>(((1)*PI)/4)))
		{
			dir=up;
		}
		else
		{
			dir=left;
		}
		
		int32_t d2=lined_up(15,true);
		
		if(((d2<left || d2 > right) && (dmisc1==1)) ||
				((d2>down) && (dmisc1==2)) ||
				((d2>right) && (!dmisc1)) ||
				((d2<l_up) && (dmisc1==4)) ||
				((d2!=r_up) && (d2!=l_down) && (dmisc1==6)) ||
				((d2!=l_up) && (d2!=r_down) && (dmisc1==8)))
		{
			d2=-1;
		}
		
		if(d2!=-1 && trapmove(d2))
		{
			dir=d2;
			misc=1;
			clk2=(dir==down)?3:0;
		}
	}
	
	if(misc==1)                                               // charging
	{
		clk2=(clk2+1)&3;
		step=(clk2==3)?1:2;
		
		if(!trapmove(dir) || clip())
		{
			misc=2;
			
			if(dir<l_up)
			{
				dir=dir^1;
			}
			else
			{
				dir=dir^3;
			}
		}
		else
		{
			sprite::move(step);
		}
	}
	
	if(misc==2)                                               // retreating
	{
		step=(++clk2&1)?1:0;
		
		switch(dir)
		{
		case up:
			if(int32_t(y)<=oy) goto trap_rest;
			else sprite::move(step);
			
			break;
			
		case left:
			if(int32_t(x)<=ox) goto trap_rest;
			else sprite::move(step);
			
			break;
			
		case down:
			if(int32_t(y)>=oy) goto trap_rest;
			else sprite::move(step);
			
			break;
			
		case right:
			if(int32_t(x)>=ox) goto trap_rest;
			else sprite::move(step);
			
			break;
			
		case l_up:
			if(int32_t(x)<=ox && int32_t(y)<=oy) goto trap_rest;
			else sprite::move(step);
			
			break;
			
		case r_up:
			if(int32_t(x)>=ox && int32_t(y)<=oy) goto trap_rest;
			else sprite::move(step);
			
			break;
			
		case l_down:
			if(int32_t(x)<=ox && int32_t(y)>=oy) goto trap_rest;
			else sprite::move(step);
			
			break;
			
		case r_down:
			if(int32_t(x)>=ox && int32_t(y)>=oy) goto trap_rest;
			else sprite::move(step);
			
			break;
trap_rest:
			{
				x=ox;
				y=oy;
				misc=0;
			}
		}
	}
	
	return enemy::animate(index);
}

bool eTrap::trapmove(int32_t ndir)
{
	if(get_bit(quest_rules,qr_MEANTRAPS))
	{
		if(tmpscr->flags2&fFLOATTRAPS)
			return canmove(ndir,(zfix)1,spw_floater, 0, 0, 15, 15,false);
			
		return canmove(ndir,(zfix)1,spw_water, 0, 0, 15, 15,false);
	}
	
	if(oy==80 && !(ndir==left || ndir == right))
		return false;
		
	if(ox==128 && !(ndir==up || ndir==down))
		return false;
		
	if(oy<80 && ndir==up)
		return false;
		
	if(oy>80 && ndir==down)
		return false;
		
	if(ox<128 && ndir==left)
		return false;
		
	if(ox>128 && ndir==right)
		return false;
		
	if(ox<128 && oy<80 && ndir==l_up)
		return false;
		
	if(ox<128 && oy>80 && ndir==l_down)
		return false;
		
	if(ox>128 && oy<80 && ndir==r_up)
		return false;
		
	if(ox>128 && oy>80 && ndir==r_down)
		return false;
		
	return true;
}

bool eTrap::clip()
{
	if(get_bit(quest_rules,qr_MEANPLACEDTRAPS))
	{
		switch(dir)
		{
		case up:
			if(y<=0)           return true;
			
			break;
			
		case down:
			if(y>=160)         return true;
			
			break;
			
		case left:
			if(x<=0)           return true;
			
			break;
			
		case right:
			if(x>=240)         return true;
			
			break;
			
		case l_up:
			if(y<=0||x<=0)     return true;
			
			break;
			
		case l_down:
			if(y>=160||x<=0)   return true;
			
			break;
			
		case r_up:
			if(y<=0||x>=240)   return true;
			
			break;
			
		case r_down:
			if(y>=160||x>=240) return true;
			
			break;
		}
		
		return false;
	}
	else
	{
		switch(dir)
		{
		case up:
			if(oy>80 && y<=86) return true;
			
			break;
			
		case down:
			if(oy<80 && y>=80) return true;
			
			break;
			
		case left:
			if(ox>128 && x<=124) return true;
			
			break;
			
		case right:
			if(ox<120 && x>=116) return true;
			
			break;
			
		case l_up:
			if(oy>80 && y<=86 && ox>128 && x<=124) return true;
			
			break;
			
		case l_down:
			if(oy<80 && y>=80 && ox>128 && x<=124) return true;
			
			break;
			
		case r_up:
			if(oy>80 && y<=86 && ox<120 && x>=116) return true;
			
			break;
			
		case r_down:
			if(oy<80 && y>=80 && ox<120 && x>=116) return true;
			
			break;
		}
		
		return false;
	}
}

void eTrap::draw(BITMAP *dest)
{
	update_enemy_frame();
	enemy::draw(dest);
}

int32_t eTrap::takehit(weapon*)
{
	return 0;
}

eTrap2::eTrap2(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	lasthit=-1;
	lasthitclk=0;
	mainguy=false;
	if (!(editorflags&ENEMY_FLAG3)) count_enemy=false;
	step=2;
	if(dmisc1==1 || (dmisc1==0 && zc_oldrand()&2))
	{
		dir=(x<=112)?right:left;
	}
	else
	{
		dir=(y<=72)?down:up;
	}
	
	if(get_bit(quest_rules,qr_TRAPPOSFIX))
	{
		yofs = playing_field_offset;
	}
	
	//nets+((id==eTRAP_LR)?540:520);
	dummy_int[1]=0;
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)d->zofs;
}

bool eTrap2::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(fallclk||drownclk) return enemy::animate(index);
	if(clk<0)
		return enemy::animate(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	if(!get_bit(quest_rules,qr_PHANTOMPLACEDTRAPS))
	{
		if(lasthitclk>0)
		{
			--lasthitclk;
		}
		else
		{
			lasthit=-1;
		}
		
		bool hitenemy=false;
		
		for(int32_t j=0; j<guys.Count(); j++)
		{
			if((j!=index) && (lasthit!=j))
			{
				if(hit(guys.spr(j)))
				{
					lasthit=j;
					lasthitclk=10;
					hitenemy=true;
					guys.spr(j)->lasthit=index;
					guys.spr(j)->lasthitclk=10;
//          guys.spr(j)->dir=guys.spr(j)->dir^1;
				}
			}
		}
		
		if(!trapmove(dir) || clip() || hitenemy)
		{
			if(!trapmove(dir) || clip())
			{
				lasthit=-1;
				lasthitclk=0;
			}
			
			if(get_bit(quest_rules,qr_MORESOUNDS))
				sfx(WAV_ZN1TAP,pan(int32_t(x)));
				
			dir=dir^1;
		}
		
		sprite::move(step);
	}
	else
	{
		if(!trapmove(dir) || clip())
		{
			if(get_bit(quest_rules,qr_MORESOUNDS))
				sfx(WAV_ZN1TAP,pan(int32_t(x)));
				
			dir=dir^1;
		}
		
		sprite::move(step);
	}
	
	return enemy::animate(index);
}

bool eTrap2::trapmove(int32_t ndir)
{
	if(tmpscr->flags2&fFLOATTRAPS)
		return canmove(ndir,(zfix)1,spw_floater, 0, 0, 15, 15,false);
		
	return canmove(ndir,(zfix)1,spw_water, 0, 0, 15, 15,false);
}

bool eTrap2::clip()
{
	switch(dir)
	{
	case up:
		if(y<=0) return true;
		
		break;
		
	case down:
		if(y>=160) return true;
		
		break;
		
	case left:
		if(x<=0) return true;
		
		break;
		
	case right:
		if(x>=240) return true;
		
		break;
	}
	
	return false;
}

void eTrap2::draw(BITMAP *dest)
{
	update_enemy_frame();
	enemy::draw(dest);
}

int32_t eTrap2::takehit(weapon*)
{
	return 0;
}

eRock::eRock(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	//do not show "enemy appering" anim -DD
	clk=0;
	mainguy=false;
	clk2=-14;
	//Enemy Editor Size Tab
	if (  (d->SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	else hxofs = -2;
	if (  (d->SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
	else hyofs = -2;
	if ( ((d->SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && d->hxsz >= 0 ) hxsz = d->hxsz;
	else hxsz = 20;
	if ( ((d->SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && d->hysz >= 0 ) hysz = d->hysz;
	else hysz=20;
	
	if ( ((d->SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && d->txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
		if ( ((d->SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && d->tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
		if ( ((d->SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && d->hzsz >= 0  ) hzsz = d->hzsz;    
		if (  (d->SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)d->xofs;
		if ( (d->SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
		{
		yofs = (int32_t)d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
		}
  
		if (  (d->SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)d->zofs;																
	//nets+1640;
}

bool eRock::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(fallclk||drownclk) return enemy::animate(index);
	if(dying)
		return Dead(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	if(++clk2==0)                                             // start it
	{
		x=zc_oldrand()&0xF0;
		y=0;
		clk3=0;
		clk2=zc_oldrand()&15;
	}
	
	if(clk2>16)                                               // move it
	{
		if(clk3<=0)                                             // start bounce
		{
			dir=zc_oldrand()&1;
			
			if(x<32)  dir=1;
			
			if(x>208) dir=0;
		}
		
		if(clk3<13+16)
		{
			x += dir ? 1 : -1;                                    //right, left
			dummy_int[1]=dir;
			
			if(clk3<2)
			{
				y-=2;    //up
				dummy_int[2]=(dummy_int[1]==1)?r_up:l_up;
			}
			else if(clk3<5)
			{
				y--;    //up
				dummy_int[2]=(dummy_int[1]==1)?r_up:l_up;
			}
			else if(clk3<8)
			{
				dummy_int[2]=(dummy_int[1]==1)?right:left;
			}
			else if(clk3<11)
			{
				y++;   //down
				dummy_int[2]=(dummy_int[1]==1)?r_down:l_down;
			}
			else
			{
				y+=2; //down
				dummy_int[2]=(dummy_int[1]==1)?r_down:l_down;
			}
			
			++clk3;
		}
		else if(y<176)
			clk3=0;                                               // next bounce
		else
			clk2 = -(zc_oldrand()&63);                                  // back to top
	}
	
	return enemy::animate(index);
}

void eRock::drawshadow(BITMAP *dest, bool translucent)
{
	if(clk2>=0)
	{
		int32_t tempy=yofs;
		flip = 0;
		int32_t fdiv = frate/4;
		int32_t efrate = fdiv == 0 ? 0 : clk/fdiv;
		int32_t f2=get_bit(quest_rules,qr_NEWENEMYTILES)?
			   efrate:((clk>=(frate>>1))?1:0);
		shadowtile = wpnsbuf[spr_shadow].newtile+f2;
		
		yofs+=8;
		yofs+=zc_max(0,zc_min(29-clk3,clk3));
		if(!shadow_overpit(this))
			enemy::drawshadow(dest, translucent);
		yofs=tempy;
	}
}

void eRock::draw(BITMAP *dest)
{
	if(clk2>=0 || fallclk||drownclk)
	{
		int32_t tempdir=dir;
		dir=dummy_int[2];
		update_enemy_frame();
		enemy::draw(dest);
		dir=tempdir;
	}
}

int32_t eRock::takehit(weapon*)
{
	return 0;
}

eBoulder::eBoulder(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	clk=0;
	mainguy=false;
	clk2=-14;
	if ( (d->SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	else hxofs= -10; 
	if (  (d->SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
	else hyofs=-10;
	if ( ((d->SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && d->hxsz >= 0 ) hxsz = d->hxsz;
	else hxsz=36;
	if ( ((d->SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && d->hysz >= 0 ) hysz = d->hysz;
	else hysz=36;
	if ( ((d->SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && d->hzsz >= 0  ) hzsz = d->hzsz;
	else hzsz=16; //can't be jumped
	
	if ( ((d->SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && d->txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	if ( ((d->SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && d->tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((d->SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && d->hxsz >= 0 ) hxsz = d->hxsz;
	if (  (d->SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)d->xofs;
	if ( (d->SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (d->SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)d->zofs;
	//nets+1680;
}

bool eBoulder::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(fallclk||drownclk) return enemy::animate(index);
	if(dying)
		return Dead(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	zfix *vert;
	vert = get_bit(quest_rules,qr_ENEMIESZAXIS) ? &z : &y;
	
	if(++clk2==0)                                             // start it
	{
		x=zc_oldrand()&0xF0;
		y=-32;
		clk3=0;
		clk2=zc_oldrand()&15;
	}
	
	if(clk2>16)                                               // move it
	{
		if(clk3<=0)                                             // start bounce
		{
			dir=zc_oldrand()&1;
			
			if(x<32)  dir=1;
			
			if(x>208) dir=0;
		}
		
		if(clk3<13+16)
		{
			x += dir ? 1 : -1;                                    //right, left
			dummy_int[1]=dir;
			
			if(clk3<2)
			{
				y-=2;    //up
				dummy_int[2]=(dummy_int[1]==1)?r_up:l_up;
			}
			else if(clk3<5)
			{
				y--;    //up
				dummy_int[2]=(dummy_int[1]==1)?r_up:l_up;
			}
			else if(clk3<8)
			{
				dummy_int[2]=(dummy_int[1]==1)?right:left;
			}
			else if(clk3<11)
			{
				y++;     //down
				dummy_int[2]=(dummy_int[1]==1)?r_down:l_down;
			}
			else
			{
				y+=2; //down
				dummy_int[2]=(dummy_int[1]==1)?r_down:l_down;
			}
			
			++clk3;
		}
		else if(y<176)
			clk3=0;                                               // next bounce
		else
			clk2 = -(zc_oldrand()&63);                                  // back to top
	}
	
	return enemy::animate(index);
}

void eBoulder::drawshadow(BITMAP *dest, bool translucent)
{
	if(clk2>=0)
	{
		int32_t tempy=yofs;
		flip = 0;
		int32_t f2=((clk<<2)/frate)<<1;
		shadowtile = wpnsbuf[spr_shadow].newtile+f2;
		yofs+=zc_max(0,zc_min(29-clk3,clk3));
		
		yofs+=8;
		xofs-=8;
		if(!shadow_overpit(this))
			enemy::drawshadow(dest, translucent);
		xofs+=16;
		++shadowtile;
		if(!shadow_overpit(this))
			enemy::drawshadow(dest, translucent);
		yofs+=16;
		shadowtile+=20;
		if(!shadow_overpit(this))
			enemy::drawshadow(dest, translucent);
		xofs-=16;
		--shadowtile;
		if(!shadow_overpit(this))
			enemy::drawshadow(dest, translucent);
		xofs+=8;
		yofs=tempy;
	}
}

void eBoulder::draw(BITMAP *dest)
{
	if(clk2>=0 || fallclk||drownclk)
	{
		int32_t tempdir=dir;
		dir=dummy_int[2];
		update_enemy_frame();
		dir=tempdir;
		xofs-=8;
		yofs-=8;
		drawblock(dest,15);
		xofs+=8;
		yofs+=8;
		//    enemy::draw(dest);
	}
}

int32_t eBoulder::takehit(weapon*)
{
	return 0;
}

eProjectile::eProjectile(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk),
	minRange(get_bit(quest_rules, qr_BROKENSTATUES) ? 0 : Clk)
{
	/* fixing
	  hp=1;
	  */
	mainguy=false;
	if (!(editorflags&ENEMY_FLAG3)) count_enemy=false;
	hclk=clk;                                                 // the "no fire" range
	clk=0;
	clk3=96;
	timer=0;
	if(o_tile==0)
	{
		superman=1;
		hxofs=1000;
	}
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = d->zofs;
}

bool eProjectile::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(fallclk||drownclk) return enemy::animate(index);
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	double ddir=atan2(double(y-(Link.y)),double(Link.x-x));
	
	if((ddir<=(((-1)*PI)/4))&&(ddir>(((-3)*PI)/4)))
	{
		dir=down;
	}
	else if((ddir<=(((1)*PI)/4))&&(ddir>(((-1)*PI)/4)))
	{
		dir=right;
	}
	else if((ddir<=(((3)*PI)/4))&&(ddir>(((1)*PI)/4)))
	{
		dir=up;
	}
	else
	{
		dir=left;
	}
	
	if(++clk3>80)
	{
		if(dmisc1==9) // Breath type
		{
			if(timer==0)
			{
				unsigned r=zc_oldrand();
				
				if(!(r&63))
				{
					timer=zc_oldrand()%50+50;
				}
			}
			
			if(timer>0)
			{
				if(timer%4==0)
				{
					FireBreath(false);
				}
				
				if(--timer==0)
				{
					clk3=0;
				}
			}
		}
		
		else // Not breath type
		{
			unsigned r=zc_oldrand();
			
			if(!(r&63) && !LinkInRange(minRange))
			{
				FireWeapon();
				
				if(get_bit(quest_rules, qr_BROKENSTATUES)==0 &&
				  ((wpn==ewFireball || wpn==ewFireball2) || dmisc1==e1tNORMAL))
				{
					if(!((r>>7)&15))
					{
						x-=4;
						FireWeapon();
						x+=4;
					}
				}
				
				clk3=0;
			}
		}
	}
	
	return enemy::animate(index);
}

void eProjectile::draw(BITMAP *dest)
{
	update_enemy_frame();
	enemy::draw(dest);
}

eTrigger::eTrigger(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	hxofs=1000;
}

void eTrigger::draw(BITMAP *dest)
{
	update_enemy_frame();
	enemy::draw(dest);
}

void eTrigger::death_sfx()
{
	//silent death
}

eNPC::eNPC(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	o_tile+=wpnsbuf[iwNPCs].newtile;
	if (!(editorflags&ENEMY_FLAG3)) count_enemy=false;
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = d->zofs;
}

bool eNPC::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(dying)
		return Dead(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	switch(dmisc2)
	{
	case 0:
	{
		double ddir=atan2(double(y-(Link.y)),double(Link.x-x));
		
		if((ddir<=(((-1)*PI)/4))&&(ddir>(((-3)*PI)/4)))
		{
			dir=down;
		}
		
		else if((ddir<=(((1)*PI)/4))&&(ddir>(((-1)*PI)/4)))
		{
			dir=right;
		}
		else if((ddir<=(((3)*PI)/4))&&(ddir>(((1)*PI)/8)))
		{
			dir=up;
		}
		else
		{
			dir=left;
		}
	}
	break;
	
	case 1:
		halting_walk(rate, homing, 0, hrate, 48);
		
		if(clk2==1 && (misc < dmisc1) && !(zc_oldrand()&15))
		{
			newdir(rate, homing, 0);
			clk2=48;
			++misc;
		}
		
		if(clk2==0)
			misc=0;
			
		break;
	}
	
	return enemy::animate(index);
}

void eNPC::draw(BITMAP *dest)
{
	update_enemy_frame();
	enemy::draw(dest);
}

int32_t eNPC::takehit(weapon*)
{
	return 0;
}

eSpinTile::eSpinTile(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	if(clk>0)  // clk>0 when created by a Spinning Tile combo
	{
		o_tile=clk;
		cs=id>>12;
	}
	
	id=id&0xFFF;
	clk=0;
	step=0;
	mainguy=false;
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = d->zofs;
}

void eSpinTile::facelink()
{
	if(Link.x-x==0)
	{
		if (Link.y + 8 < y)
			dir = up;
		else
			dir = down;
	}
	else
	{
		double ddir=atan2(double(y-(Link.y)),double(Link.x-x));
		
		if((ddir <= -5.0*PI/8.0) && (ddir > -7.0*PI/8.0))
		{
			dir=l_down;
		}
		else if ((ddir <= -3.0*PI / 8.0) && (ddir > -5.0*PI / 8.0))
		{
			dir=down;
		}
		else if ((ddir <= -1.0*PI / 8.0) && (ddir > -3.0*PI / 8.0))
		{
			dir=r_down;
		}
		else if ((ddir <= 1.0*PI / 8.0) && (ddir > -1.0*PI / 8.0))
		{
			dir=right;
		}
		else if ((ddir <= 3.0*PI / 8.0) && (ddir > 1.0*PI / 8.0))
		{
			dir=r_up;
		}
		else if ((ddir <= 5.0*PI / 8.0) && (ddir > 3.0*PI / 8.0))
		{
			dir=up;
		}
		else if ((ddir <= 7.0*PI / 8.0) && (ddir > 5.0*PI / 8.0))
		{
			dir=l_up;
		}
		else
		{
			dir=left;
		}
	}
}


bool eSpinTile::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(fallclk||drownclk) return enemy::animate(index);
	if(dying)
	{
		return Dead(index);
	}
	
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	++misc;
	
	if(misc==96)
	{
		facelink();
		double ddir=atan2(double((Link.y)-y),double(Link.x-x));
		angular=true;
		angle=ddir;
		step=zslongToFix(dstep*100);
	}
	
	if(y>186 || y<=-16 || x>272 || x<=-16)
		kickbucket();
		
	sprite::move(step);
	return enemy::animate(index);
}

void eSpinTile::draw(BITMAP *dest)
{
	update_enemy_frame();
	y-=(misc>>4);
	yofs+=2;
	enemy::draw(dest);
	yofs-=2;
	y+=(misc>>4);
}

void eSpinTile::drawshadow(BITMAP *dest, bool translucent)
{
	flip = 0;
	shadowtile = wpnsbuf[spr_shadow].newtile+(clk%4);
	yofs+=4;
	if(!shadow_overpit(this))
		enemy::drawshadow(dest, translucent);
	yofs-=4;
}

eZora::eZora(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,0)
{
	//these are here to bypass compiler warnings about unused arguments
	Clk=Clk;
	mainguy=false;
	if (!(editorflags&ENEMY_FLAG3)) count_enemy=false;
	/*if((x>-17 && x<0) && iswaterex(tmpscr->data[(((int32_t)y&0xF0)+((int32_t)x>>4))]))
	{
	  clk=1;
	}*/
	//nets+880;
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = d->zofs;
}

void eZora::facelink()
{
	if(Link.x-x==0)
	{
		dir=(Link.y+8<y)?up:down;
	}
	else
	{
		double ddir=atan2(double(y-(Link.y)),double(Link.x-x));
		
		if((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
		{
			dir=l_down;
		}
		else if((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
		{
			dir=down;
		}
		else if((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
		{
			dir=r_down;
		}
		else if((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
		{
			dir=right;
		}
		else if((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
		{
			dir=r_up;
		}
		else if((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
		{
			dir=up;
		}
		else if((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
		{
			dir=l_up;
		}
		else
		{
			dir=left;
		}
	}
}

bool eZora::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(dying)
		return Dead(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	if(watch)
	{
		++clock_zoras[id];
		return true;
	}
	
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
		facelink();
	}
	
	switch(clk)
	{
	case 0:                                                 // reposition him
	{
		int32_t t=0;
		int32_t pos2=zc_oldrand()%160 + 16;
		bool placed=false;
		
		while(!placed && t<160)
		{
			int32_t watertype = iswaterex(tmpscr->data[pos2], currmap, currscr, -1, ((pos2)%16*16), ((pos2)&0xF0), false, true, true, (bool)(editorflags & ENEMY_FLAG7));
			if(watertype && ((editorflags & ENEMY_FLAG6) || 
			((combobuf[watertype].usrflags&cflag1) && (editorflags & ENEMY_FLAG5))
			|| (!(combobuf[watertype].usrflags&cflag1) && !(editorflags & ENEMY_FLAG5))) && (pos2&15)>0 && (pos2&15)<15)
			{
				x=(pos2&15)<<4;
				y=pos2&0xF0;
				if (!(editorflags & ENEMY_FLAG8)) hp=guysbuf[id&0xFFF].hp;       // refill life each time, unless the flag is checked.
				hxofs=1000;                                                      // avoid hit detection
				stunclk=0;
				placed=true;
			}
			
			pos2+=19;
			
			if(pos2>=176)
				pos2-=160;
				
			++t;
		}
		
		if(!placed || whistleclk>=88)                         // can't place him, he's gone
			return true;
			
	}
	break;
	
	case 35:
		if(!get_bit(quest_rules,qr_NEWENEMYTILES))
		{
			dir=(Link.y+8<y)?up:down;
		}
		
		hxofs=0;
		break;
		
//    case 35+19: addEwpn(x,y,z,ewFireball,0,d->wdp,0); break;
	case 35+19:
		addEwpn(x,y,z,wpn,2,wdp,dir,getUID());
		sfx(wpnsfx(wpn),pan(int32_t(x)));
		break;
		
	case 35+66:
		hxofs=1000;
		break;
		
	case 198:
		clk=-1;
		break;
	}
	
	return enemy::animate(index);
}

void eZora::draw(BITMAP *dest)
{
	if(clk<3)
		return;
		
	update_enemy_frame();
	enemy::draw(dest);
}

bool eZora::isSubmerged()
{
	return ( clk < 3 );
}

eStalfos::eStalfos(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	multishot= timer = fired = dashing = 0;
	haslink = false;
	dummy_bool[0]=false;
	shield= (flags&(inv_left | inv_right | inv_back |inv_front)) != 0;
	if(dmisc9==e9tARMOS && zc_oldrand()&1)
	{
		step=zslongToFix(dmisc10*100);
		
		if(anim==aARMOS4) o_tile+=20;
	}
	
	if(flags & guy_fadeflicker)
	{
		clk=0;
		superman = 1;
		fading=fade_flicker;
		if (!(editorflags&ENEMY_FLAG3)) count_enemy=false;
		dir=down;
		
		if(!canmove(down,(zfix)8,spw_none,false))
			clk3=int32_t(13.0/step);
	}
	else if(flags & guy_fadeinstant)
	{
		clk=0;
	}
	
	shadowdistance = 0;
	clk4 = clk5 = 0;
	//nets+2380;
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = d->zofs;
}

bool eStalfos::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(fallclk||drownclk)
	{
		return enemy::animate(index);
	}
	if(dying)
	{
		if(haslink)
		{
			Link.setEaten(0);
			haslink=false;
		}
		
		if(dmisc9==e9tROPE && dmisc2==e2tBOMBCHU && !fired && (hp<=0 && !immortal) && hp>-1000 && wpn>wEnemyWeapons)
		{
			hp=-1000;
//        weapon *ew=new weapon(x,y,z, ewBomb, 0, d->wdp, dir);
			weapon *ew=new weapon(x,y,z, wpn, 0, dmisc4, dir,-1,getUID(),false);
			Ewpns.add(ew);
			
			if(wpn==ewSBomb || wpn==ewBomb)
			{
				ew->step=0;
				ew->id=wpn;
				ew->misc=50;
				ew->clk=48;
			}
			
			fired=true;
		}
		else if(wpn && wpn!=ewBrang && dmisc2==e2tFIREOCTO)  // Fire Octo
		{
			if(!dummy_bool[0])
			{
				int32_t wpn2 = wpn+dmisc3;
				
				if(wpn2 <= wEnemyWeapons || wpn2 >= wMax)
				{
					wpn2=wpn;
				}
				
				dummy_bool[0]=true;
				addEwpn(x,y,z,wpn2,0,dmisc4,up, getUID());
				((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
				addEwpn(x,y,z,wpn2,0,dmisc4,down, getUID());
				((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
				addEwpn(x,y,z,wpn2,0,dmisc4,left, getUID());
				((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
				addEwpn(x,y,z,wpn2,0,dmisc4,right, getUID());
				((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
				addEwpn(x,y,z,wpn2,0,dmisc4,l_up, getUID());
				((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
				addEwpn(x,y,z,wpn2,0,dmisc4,r_up, getUID());
				((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
				addEwpn(x,y,z,wpn2,0,dmisc4,l_down, getUID());
				((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
				addEwpn(x,y,z,wpn2,0,dmisc4,r_down, getUID());
				((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
				sfx(wpnsfx(wpn2),pan(int32_t(x)));
			}
		}
		
		KillWeapon();
		return Dead(index);
	}
	//vire split
	//2.10 checked !fslide(), but nothing uses that now anyway. -Z
	//Perhaps the problem occurs when vires die because they have < 0 HP, in this check?
	else if(((hp<=0 && !immortal) && dmisc2==e2tSPLIT) || (dmisc2==e2tSPLITHIT && hp>0 && hp<guysbuf[id&0xFFF].hp && !slide() && (sclk&255)<=1))  //Split into enemies
	{
		stop_bgsfx(index);
		int32_t kids = guys.Count();
		int32_t id2=dmisc3;
		for(int32_t i=0; i < dmisc4; i++)
		{
//	    if (addenemy(x,y,id2+(guysbuf[id2].family==eeKEESE ? 0 : ((i+1)<<12)),-21-(i%4)))
			if(addenemy(x,y,id2+(guysbuf[id2].family==eeKEESE ? 0 : ((editorflags & ENEMY_FLAG5) ? 0 : (i<<12))),-21-(i%4)))
				((enemy*)guys.spr(kids+i))->count_enemy = false;
		}
		
		if(itemguy) // Hand down the carried item
		{
			guycarryingitem = guys.Count()-1;
			((enemy*)guys.spr(guycarryingitem))->itemguy = true;
			itemguy = false;
		}
		
		if(haslink)
		{
			Link.setEaten(0);
			haslink=false;
		}
		
		if(deadsfx > 0 && dmisc2==e2tSPLIT)
			sfx(deadsfx,pan(int32_t(x)));
			
		return true;
	}
	/*
	else if((dmisc2==e2tSPLITHIT && (hp<=0 && !immortal) &&!slide()))  //Possible vires fix; or could cause goodness knows what. -Z
	{
		stop_bgsfx(index);
		int32_t kids = guys.Count();
		int32_t id2=dmisc3;
		
		for(int32_t i=0; i < dmisc4; i++)
		{
//	    if (addenemy(x,y,id2+(guysbuf[id2].family==eeKEESE ? 0 : ((i+1)<<12)),-21-(i%4)))
			if(addenemy(x,y,id2+(guysbuf[id2].family==eeKEESE ? 0 : (i<<12)),-21-(i%4)))
				((enemy*)guys.spr(kids+i))->count_enemy = false;
		}
		
		if(itemguy) // Hand down the carried item
		{
			guycarryingitem = guys.Count()-1;
			((enemy*)guys.spr(guycarryingitem))->itemguy = true;
			itemguy = false;
		}
		
		if(haslink)
		{
			Link.setEaten(0);
			haslink=false;
		}
					
		return true;
	}
	*/
	if(fading)
	{
		if(++clk4 > 60)
		{
			clk4=0;
			superman=0;
			fading=0;
			
			if(flags2&cmbflag_armos && z==0)
		{
		//if a custom size (not 16px by 16px)
			
		//if a custom size (not 16px by 16px)
		if (txsz > 1 || tysz > 1 || (SIZEflags&guyflagOVERRIDE_HIT_WIDTH) || (SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) )//remove more than one combo based on enemy size
		{
			//zprint("spawn big enemy from armos\n");
			 //if removing a block, then adjust y by -1 as the enemy spawns at y+1
			for(int32_t dx = 0; dx < tysz; dx ++)
			{
				for(int32_t dy = 0; dy < tysz; dy++)
				{
					removearmos((int32_t)x+(dx*16),(int32_t)y+(dy*16)+1);
					did_armos = false;
				}
				removearmos((int32_t)x+(dx*16), (int32_t)y+((tysz-1)*16)+1);
				did_armos = false;
			}
			for(int32_t dy = 0; dy < tysz; dy ++)
			{
				removearmos((int32_t)x+((txsz-1)*16), (int32_t)y+(dy*16)+1);
				did_armos = false;
			}
			removearmos((int32_t)x+((txsz-1)*16), (int32_t)y+((tysz-1)*16)+1);
		}
				else removearmos(x,y); 
		/*
		if (txsz > 1 || tysz > 1 || (SIZEflags&guyflagOVERRIDE_HIT_WIDTH) || (SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) )//remove more than one combo based on enemy size
		{
			 //if removing a block, then adjust y by -1 as the enemy spawns at y+1
			for(int32_t dx = 0; dx < hxsz; dx += 16)
			{
				for(int32_t dy = 0; dy < hysz; dy += 16)
				{
					removearmos((int32_t)x+dx+hxofs,(int32_t)y+dy+hyofs+1);
					did_armos = false;
				}
				removearmos((int32_t)x+dx+hxofs, (int32_t)y+hyofs+(hysz-1)-1);
				did_armos = false;
			}
			for(int32_t dy = 0; dy < hysz; dy += 16)
			{
				removearmos((int32_t)x+hxofs+(hxsz-1), (int32_t)y+dy+hyofs-1);
				did_armos = false;
			}
			removearmos((int32_t)x+hxofs+(hxsz-1), (int32_t)y+hyofs+(hysz-1)-1);
		}
				else removearmos(x,y);
		*/		
	   
		}
				
			clk2=0;
			
			newdir();
		}
		else return enemy::animate(index);
	}
	else if(flags2&cmbflag_armos && z==0 && clk==0)
		removearmos(x,y);
		
	
	if(haslink)
	{
		Link.setX(x);
		Link.setY(y);
		++clk2;
		
		if(clk2==(dmisc8==0 ? 95 : dmisc8))
		{
			switch(dmisc7)
			{
			case e7tEATITEMS:
			{
				for(int32_t i=0; i<MAXITEMS; i++)
				{
					if(itemsbuf[i].flags&ITEM_EDIBLE)
						game->set_item(i, false);
				}
				
				break;
			}
			
			case e7tEATMAGIC:
				game->change_dmagic(-1*game->get_magicdrainrate());
				break;
				
			case e7tEATRUPEES:
				game->change_drupy(-1);
				break;
			}
			
			clk2=0;
		}
		
		if((clk&0x18)==8)                                       // stop its animation on the middle frame
			--clk;
	}
	else if(!(wpn==ewBrang && WeaponOut()))  //WeaponOut uses misc
	{
		// Movement engine
		if(clk>=0) switch(id>>12)
			{
			case 0: // Normal movement
			
				/*
				if((dmisc9==e9tLEEVER || dmisc9==e9tZ3LEEVER) && !slide()) //Leever
				{
				  // Overloading clk4 (Tribble clock) here...
				  step=17/100.0;
				  if(clk4<32)    misc=1;
				  else if(clk4<48)    misc=2;
				  else if(clk4<300) { misc=3; step = dstep/100.0; }
				  else if(clk4<316)   misc=2;
				  else if(clk4<412)   misc=1;
				  else if(clk4<540) { misc=0; step=0; }
				  else clk4=0;
				  if(clk4==48) clk=0;
				  hxofs=(misc>=2)?0:1000;
				  if (dmisc9==e9tLEEVER)
					variable_walk(rate, homing, 0);
				  else
					variable_walk_8(rate, homing, 4, 0);
				  break;
				}
				*/
				if(dmisc9==e9tVIRE || dmisc9==e9tPOLSVOICE) //Vire
				{
					vire_hop();
					break;
				}
				else if(dmisc9==e9tROPE) //Rope charge
				{
					if(!fired && dashing && !stunclk && !watch && !frozenclock)
					{
						if(dmisc2==e2tBOMBCHU && LinkInRange(16) && wpn+dmisc3 > wEnemyWeapons) //Bombchu
						{
				
							if (  get_bit(quest_rules,qr_BOMBCHUSUPERBOMB) ) 
							{
								hp=-1000;
										
								if(wpn+dmisc3 > wEnemyWeapons && wpn+dmisc3 < wMax)
								{
								weapon *ew=new weapon(x,y,z, wpn+dmisc3, 0, dmisc4, dir,-1,getUID());
								Ewpns.add(ew);
								
								if(wpn==ewSBomb || wpn==ewBomb)
								{
									ew->step=0;
									ew->id=wpn+dmisc3;
									ew->misc=50;
									ew->clk=48;
								}
								
								fired=true;
								}
								else
								{
								weapon *ew=new weapon(x,y,z, wpn, 0, dmisc4, dir,-1,getUID());
								Ewpns.add(ew);
								
								if(wpn==ewSBomb || wpn==ewBomb)
								{
									ew->step=0;
									ew->id=wpn;
									ew->misc=50;
									ew->clk=48;
								}
								
								fired=true;
								}
							}
							
							else
							{
								hp=-1000;
								
								int32_t wpn2;
								if(wpn+dmisc3 > wEnemyWeapons && wpn+dmisc3 < wMax)
								wpn2=wpn;
								else
								wpn2=wpn;
								
								weapon *ew=new weapon(x,y,z, wpn2, 0, dmisc4, dir,-1,getUID());
								Ewpns.add(ew);
								
								if(wpn2==ewSBomb || wpn2==ewBomb)
								{
								ew->step=0;
								ew->id=wpn2;
								ew->misc=50;
								ew->clk=48;
								}
								
								fired=true;
							}
						}
					}
					
					charge_attack();
					break;
				}
				/*
				 * Boomerang-throwers have a halt count of 1
				 * Zols have a halt count of (zc_oldrand()&7)<<4
				 * Gels have a halt count of ((zc_oldrand()&7)<<3)+2
				 * Everything else has 48
				 */
				else
				{
					if(wpn==ewBrang) // Goriya
					{
						halting_walk(rate,homing,0,hrate, 1);
					}
					else if(dmisc9==e9tNORMAL && wpn==0)
					{
						if(dmisc2==e2tSPLITHIT) // Zol
						{
							halting_walk(rate,homing,0,hrate,(zc_oldrand()&7)<<4);
						}
						else if(frate<=8 && starting_hp==1) // Gel
						{
							halting_walk(rate,homing,0,hrate,((zc_oldrand()&7)<<3)+2);
						}
						else // Other
						{
							halting_walk(rate,homing,0,hrate, 48);
						}
					}
					else // Other
					{
						halting_walk(rate,homing,0,hrate, 48);
					}
				}
				
				//if not in midair, and Link's swinging sword is nearby, jump.
				/*if (dmisc9==e9tZ3STALFOS && z==0 && (!(isSideViewGravity()) || !_walkflag(x,y+16,0))
				  && Link.getAttackClk()==5 && Link.getAttack()==wSword && distance(x,y,Link.getX(),Link.getY())<32)
					{
					  facelink(false);
					  sclk=16+((dir^1)<<8);
					fall=-FEATHERJUMP;
					  sfx(WAV_ZN1JUMP,pan(int32_t(x)));
					}*/
				break;
				
				// Following cases are for just after creation-by-splitting.
			case 1:
				if(misc==1)
				{
					dir=up;
					step=8;
				}
				
				if(misc<=2)
				{
					move(step);
					
					if(!canmove(dir,(zfix)0,0,false))
						dir=down;
				}
				
				if(misc==3)
				{
					if(canmove(right,(zfix)16,0,false))
						x+=16;
				}
				
				++misc;
				break;
				
			case 2:
				if(misc==1)
				{
					dir=down;
					step=8;
				}
				
				if(misc<=2)
				{
					move(step);
					/*
							  if(!canmove(dir,(zfix)0,0,false))
								dir=up;
					*/
				}
				
				if(misc==3)
				{
					if(canmove(left,(zfix)16,0,false))
						x-=16;
				}
				
				++misc;
				break;
				
			default:
				if(misc==1)
				{
					dir=(zc_oldrand()%4);
					step=8;
				}
				
				if(misc<=2)
				{
					move(step);
					
					if(!canmove(dir,(zfix)0,0,false))
						dir=dir^1;
				}
				
				if(misc==3)
				{
					if(dir >= left && canmove(dir,(zfix)16,0,false))
						x+=(dir==left ? -16 : 16);
				}
				
				++misc;
				break;
			}
			
		if(id>>12 && misc>=4) //recently spawned by a split enemy
		{
			id&=0xFFF;
			step = zslongToFix(dstep*100);
			
			if(x<32) x=32;
			
			if(x>208) x=208;
			
			if(y<32) y=32;
			
			if(y>128) y=128;
			
			misc=3;
		}
	}
	else
	{
		//sfx(wpnsfx(wpn),pan(int32_t(x)));
		if(clk2>2) clk2--;
	}
	
	// Fire Zol
	if(wpn && dmisc1==e1tEACHTILE && clk2==1 && !hclk)
	{
		addEwpn(x,y,z,wpn,0,wdp,dir, getUID());
		sfx(wpnsfx(wpn),pan(int32_t(x)));
		
		int32_t i=Ewpns.Count()-1;
		weapon *ew = (weapon*)(Ewpns.spr(i));
		
		if(wpn==ewFIRETRAIL && wpnsbuf[ewFIRETRAIL].frames>1)
		{
			ew->aframe=zc_oldrand()%wpnsbuf[ewFIRETRAIL].frames;
			if ( ew->do_animation ) ew->tile+=ew->aframe;
		}
	}
	// Goriya
	else if(wpn==ewBrang && clk2==1 && sclk==0 && !stunclk && !frozenclock && !watch && wpn && !WeaponOut())
	{
		misc=index+100;
		Ewpns.add(new weapon(x,y,z,wpn,misc,wdp,dir, -1,getUID(),false));
		((weapon*)Ewpns.spr(Ewpns.Count()-1))->dummy_bool[0]=false;
		
		if(dmisc1==2)
		{
			int32_t ndir=dir;
			
			if(Link.x-x==0)
			{
				ndir=(Link.y+8<y)?up:down;
			}
			else //turn to face Link
			{
				double ddir=atan2(double(y-(Link.y)),double(Link.x-x));
				
				if((ddir<=(((-2)*PI)/8))&&(ddir>(((-6)*PI)/8)))
				{
					ndir=down;
				}
				else if((ddir<=(((2)*PI)/8))&&(ddir>(((-2)*PI)/8)))
				{
					ndir=right;
				}
				else if((ddir<=(((6)*PI)/8))&&(ddir>(((2)*PI)/8)))
				{
					ndir=up;
				}
				else
				{
					ndir=left;
				}
			}
			
			((weapon*)Ewpns.spr(Ewpns.Count()-1))->dummy_bool[0]=true;
			
			if(canmove(ndir,false))
			{
				dir=ndir;
			}
		}
	}
	else if((clk2==16 || dmisc1==e1tCONSTANT) &&  dmisc1!=e1tEACHTILE && wpn && wpn!=ewBrang && sclk==0 && !stunclk && !frozenclock && !watch)
		switch(dmisc1)
		{
		case e1tCONSTANT: //Deathnut
		{
			// Overloading clk5 (Like Like clock) to avoid making another clock just for this attack...
			if(clk5>64)
			{
				clk5=0;
				fired=false;
			}
			
			clk5+=(zc_oldrand()&3);
			
			if((clk5>24)&&(clk5<52))
			{
				if ( do_animation )tile+=20;                                         //firing
				
				if(!fired&&(clk5>=38))
				{
					Ewpns.add(new weapon(x,y,z, wpn, 0, wdp, dir, -1,getUID(),false));
					sfx(wpnsfx(wpn),pan(int32_t(x)));
					fired=true;
				}
			}
			
			break;
		}
		
		case e1tFIREOCTO: //Fire Octo
			timer=zc_oldrand()%50+50;
			break;
			
		default:
			FireWeapon();
			break;
		}
		
	/* Fire again if:
	 * - clk2 about to run out
	 * - not already double-firing (dmisc1 is 1)
	 * - not carrying Link
	 * - one in 0xF chance
	 */
	if(clk2==1 && (multishot < dmisc6) && dmisc1 != e1tEACHTILE && !haslink && !(zc_oldrand()&15))
	{
#if 1
		newdir(rate, homing, grumble);
#else
		dir^=2;
#endif
		clk2=28;
		++multishot;
	}
	
	if(clk2==0)
	{
		multishot = 0;
	}
	
	if(timer)  //Fire Octo
	{
		clk2=15; //this keeps the octo in place until he's done firing
		
		if(!(timer%4))
		{
			FireBreath(false);
		}
		
		--timer;
	}
	
	if(dmisc2==e2tTRIBBLE)
		++clk4;
		
	if(clk4==(dmisc5 ? dmisc5 : 256) && (dmisc2==e2tTRIBBLE) && dmisc3 && dmisc4)
	{
		int32_t kids = guys.Count();
		int32_t id2=dmisc3;
		
		for(int32_t i=0; i<dmisc4; i++)
		{
			if(addenemy(x,y,id2,-24))
			{
				if(itemguy) // Hand down the carried item
				{
					guycarryingitem = guys.Count()-1;
					((enemy*)guys.spr(guycarryingitem))->itemguy = true;
					itemguy = false;
				}
				
				((enemy*)guys.spr(kids+i))->count_enemy = false;
			}
		}
		
		if(haslink)
		{
			Link.setEaten(0);
			haslink=false;
		}
		
		stop_bgsfx(index);
		return true;
	}
	
	return enemy::animate(index);
}

void eStalfos::draw(BITMAP *dest)
{
	/*if ((dmisc9==e9tLEEVER || dmisc9==e9tZ3LEEVER) && misc<=0) //Submerged
	{
	  clk4--; //Kludge
	  return;
	}*/
	
	/*if ((dmisc9==e9tLEEVER || dmisc9==e9tZ3LEEVER) && misc>1)
	{
	  cs = dcset;
	}*/
	update_enemy_frame();
	
	if(!fallclk&&!drownclk&&(dmisc2==e2tBOMBCHU)&&dashing)
	{
		if ( do_animation )tile+=20;
	}
	
	enemy::draw(dest);
}

void eStalfos::drawshadow(BITMAP *dest, bool translucent)
{
	int32_t tempy=yofs;
	
	/*
	  if (clk6 && dir>=left && !get_bit(quest_rules,qr_ENEMIESZAXIS)) {
		flip = 0;
		int32_t f2=get_bit(quest_rules,qr_NEWENEMYTILES)?
		  (clk/(frate/4)):((clk>=(frate>>1))?1:0);
		shadowtile = wpnsbuf[spr_shadow].tile+f2;
		yofs+=(((int32_t)y+17)&0xF0)-y;
		yofs+=8;
	  }
	*/
	if((dmisc9 == e9tPOLSVOICE || dmisc9==e9tVIRE) && !get_bit(quest_rules,qr_ENEMIESZAXIS))
	{
		flip = 0;
		int32_t fdiv = frate/4;
		int32_t efrate = fdiv == 0 ? 0 : clk/fdiv;
		
		int32_t f2=get_bit(quest_rules,qr_NEWENEMYTILES)?
			   efrate:((clk>=(frate>>1))?1:0);
		shadowtile = wpnsbuf[spr_shadow].newtile;
		
		if(get_bit(quest_rules,qr_NEWENEMYTILES))
		{
			shadowtile+=f2;
		}
		else
		{
			shadowtile+=f2?1:0;
		}
		
		yofs+=shadowdistance;
		yofs+=8;
	}
	if(!shadow_overpit(this))
		enemy::drawshadow(dest, translucent);
	yofs=tempy;
}

int32_t eStalfos::takehit(weapon *w)
{
	int32_t wpnId = w->id;
	int32_t wpnDir = w->dir;
	
	if(wpnId==wHammer && shield && (flags & guy_bkshield)
			&& ((flags&inv_front && wpnDir==(dir^down)) || (flags&inv_back && wpnDir==(dir^up))
				|| (flags&inv_left && wpnDir==(dir^left)) || (flags&inv_right && wpnDir==(dir^right))))
	{
		shield = false;
		flags &= ~(inv_left|inv_right|inv_back|inv_front);
		
		if(get_bit(quest_rules,qr_BRKNSHLDTILES))
			o_tile=s_tile;
	}
	
	int32_t ret = enemy::takehit(w);
	
	if(sclk && dmisc2==e2tSPLITHIT)
		sclk+=128; //Fuck these arbitrary values with no explanation. Fuck vires, too. -Z
		
	return ret;
}

void eStalfos::charge_attack()
{
	if(slide())
		return;
		
	if(clk<0 || dir<0 || stunclk || watch || ceiling || frozenclock )
		return;
		
	if(clk3<=0)
	{
		fix_coords(true);
		
		if(!dashing)
		{
			int32_t ldir = lined_up(7,false);
			
			if(ldir!=-1 && canmove(ldir,false))
			{
				dir=ldir;
				dashing=true;
				step=zslongToFix(dstep*100)+1;
			}
			else newdir(4,0,0);
		}
		
		if(!canmove(dir,false))
		{
			step=zslongToFix(dstep*100);
			newdir();
			dashing=false;
		}
		
		zfix div = step;
		
		if(div == 0)
			div = 1;
			
		clk3=(int32_t)(16.0/div);
		return;
	}
	
	move(step);
	--clk3;
}

void eStalfos::vire_hop()
{
	//if ( sclk > 0 ) return; //Don't hop during knockback.
  
//    if(dmisc9!=e9tPOLSVOICE)
//    {
//        //if( slide() /*sclk!=0*/ && dmisc2==e2tSPLIT) //Vires with split on hit, only! -Z
//        if( sclk!=0 && dmisc2==e2tSPLIT) //Vires with split on hit, only! -Z
//            return; //the enemy should split if it is sliding!
//        //else sclk=0; //might need this here, too. -Z
//    }
	if(dmisc9!=e9tPOLSVOICE)
	{
		if(sclk!=0)
		{
			if (dmisc2==e2tSPLITHIT) return;
			//return;
		}
	}
	else sclk=0;
	
	if(clk<0 || dying || stunclk || watch || ceiling || frozenclock)
		return;
		
	int32_t jump_width = (dmisc9==e9tPOLSVOICE) ? 2 : 1;
	int32_t jump_height = (dmisc9==e9tPOLSVOICE) ? 27 : 16;
	
	y=floor_y;
	
	if(clk3<=0)
	{
		fix_coords();
		
		//z=0;
		//if we're not in the middle of a jump or if we can't complete the current jump in the current direction
		//if(clk2<=0 || !canmove(dir,(zfix)1,spw_floater,false) || (isSideViewGravity() && isOnSideviewPlatform()))
		if(clk2<=0 || !canmove(dir,(zfix)1,spw_floater,false) || (isSideViewGravity() && (isOnSideviewPlatform() || !(moveflags & FLAG_OBEYS_GRAV)))) //Vires in old quests 
			newdir(rate,homing,dmisc9==e9tPOLSVOICE ? spw_floater : spw_none);
			
		if(clk2<=0)
		{
			//z=0;
			if(!canmove(dir,(zfix)2,spw_none,false) || m_walkflag(x,y,spw_none, dir) || (zc_oldrand()&15)>=hrate)
			{
                
				clk2=(wpn==ewBrang ? 1 : int32_t((16.0*jump_width)/step.getFloat()));
				/*if (dmisc9==e9tPOLSVOICE )
				{	
					zprint2("polsvoice jump_width is: %d\n", jump_width);
					zprint2("polsvoice raw step is: %d\n", step);
					zprint2("polsvoice step.getInt() is: %d\n", step.getInt());
					zprint2("setting clk2 on polsvoice to: %d\n", clk2);
				}
				else 
				{
					zprint2("vire jump_width is: %d\n", jump_width);
					zprint2("vire raw step is: %d\n", step);
					zprint2("vire step.getInt() is: %d\n", step.getInt());
					zprint2("setting clk2 on vire to: %d\n", clk2);
				}
				*/
			}
		}
		
		if(dmisc9!=e9tPOLSVOICE && dir>=left) //if we're moving left or right
		{
			clk2=int32_t((16.0*jump_width)/step.getFloat());
		}
		
		clk3=int32_t(16.0/step.getFloat());
	}
	
	--clk3;
	
	if(dmisc9==e9tPOLSVOICE || clk2>0)
		move(step);
		
	floor_y=y;
	clk2--;
	
	//if we're in the middle of a jump
	if(clk2>0 && (dir>=left || dmisc9==e9tPOLSVOICE))
	{
		int32_t h = fixtoi(fixsin(itofix(clk2*128*step/(16*jump_width)))*jump_height);
		
		if(get_bit(quest_rules,qr_ENEMIESZAXIS) && !(isSideViewGravity()))
		{
			z=h;
		}
		else
		{
			//y+=fixtoi(fixsin(itofix((clk2+1)*128*step/(16*jump_width)))*jump_height);
			//y-=h;
			y=floor_y-h;
			shadowdistance=h;
		}
	}
	else
		shadowdistance = 0;
}

void eStalfos::eatlink()
{
	if(!haslink && Link.getEaten()==0 && Link.getAction() != hopping && Link.getAction() != swimming)
	{
		haslink=true;
		y=floor_y;
		z=0;
		
		if(Link.isSwimming())
		{
			Link.setX(x);
			Link.setY(y);
		}
		else
		{
			x=Link.getX();
			y=Link.getY();
		}
		
		clk2=0;
	}
}

bool eStalfos::WeaponOut()
{
	for(int32_t i=0; i<Ewpns.Count(); i++)
	{
		if(((weapon*)Ewpns.spr(i))->parentid==getUID() && Ewpns.spr(i)->id==ewBrang)
		{
			return true;
		}
		
		/*if (bgsfx > 0 && guys.idCount(id) < 2) // count self
		  stop_sfx(bgsfx);
		*/
	}
	
	return false;
}

void eStalfos::KillWeapon()
{
	for(int32_t i=0; i<Ewpns.Count(); i++)
	{
		if(((weapon*)Ewpns.spr(i))->type==misc && Ewpns.spr(i)->id==ewBrang)
		{
			//only kill this Goriya's boomerang -DD
			if(((weapon *)Ewpns.spr(i))->parentid == getUID())
			{
				Ewpns.del(i);
			}
		}
	}
	
	if(wpn==ewBrang && !Ewpns.idCount(ewBrang))
	{
		stop_sfx(WAV_BRANG);
	}
}

void eStalfos::break_shield()
{
	if(!shield)
		return;
		
	flags&=~(inv_front | inv_back | inv_left | inv_right);
	shield=false;
	
	if(get_bit(quest_rules,qr_BRKNSHLDTILES))
		o_tile=s_tile;
}

eKeese::eKeese(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	dir=(zc_oldrand()&7)+8;
	step=0;
	movestatus=1;
	c=0;
	SIZEflags = d->SIZEflags;
	if ( !(SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) ) hxofs=2;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	
	if ( !(SIZEflags&guyflagOVERRIDE_HIT_WIDTH) ) hxsz=12;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && d->hxsz >= 0 ) hxsz = d->hxsz;
	
	if ( !(SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) ) hyofs=4;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
	
	if ( !(SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) ) hysz=8;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && d->hysz >= 0 ) hysz = d->hysz;
	
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && d->txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", d->txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && d->tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	
	
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && d->hzsz >= 0  ) hzsz = d->hzsz;
	
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)d->zofs;
	clk4=0;
	//nets;
	dummy_int[1]=0;
}

bool eKeese::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(fallclk||drownclk) return enemy::animate(index);
	if(dying)
		return Dead(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	if(dmisc1) //Walk style. 0 is keese, 1 is bat.
	{
		floater_walk(rate,hrate,dstep/100,(zfix)0,10,dmisc16,dmisc17);
	}
	else
	{
		floater_walk(rate,hrate,dstep/100,dstep/1000,10,dmisc16,dmisc17);
	}
	
	if(dmisc2 == e2tKEESETRIB)
	{
		if(++clk4==256)
		{
			if(!m_walkflag(x,y,0, dir))
			{
				int32_t kids = guys.Count();
				bool success = false;
				int32_t id2=dmisc3;
				success = 0 != addenemy((zfix)x,(zfix)y,id2,-24);
				
				if(success)
				{
					if(itemguy) // Hand down the carried item
					{
						guycarryingitem = guys.Count()-1;
						((enemy*)guys.spr(guycarryingitem))->itemguy = true;
						itemguy = false;
					}
					
					((enemy*)guys.spr(kids))->count_enemy = count_enemy;
				}
				
				stop_bgsfx(index);
				return true;
			}
			else
			{
				clk4=0;
			}
		}
	}
	// Keese Tribbles stay on the ground, so there's no problem when they transform.
	else if(get_bit(quest_rules,qr_ENEMIESZAXIS) && !(isSideViewGravity()))
	{
		z=int32_t(step/zslongToFix(dstep*100));
		// Some variance in keese flight heights when away from Link
		z+=int32_t(step*zc_max(0,(distance(x,y,LinkX(),LinkY())-128)/10));
	}
	
	return enemy::animate(index);
}

void eKeese::drawshadow(BITMAP *dest, bool translucent)
{
	int32_t tempy=yofs;
	flip = 0;
	shadowtile = wpnsbuf[spr_shadow].newtile+posframe;
	yofs+=8;
	
	if(!get_bit(quest_rules,qr_ENEMIESZAXIS))
		yofs+=int32_t(step/zslongToFix(dstep*10));
	
	if(!shadow_overpit(this))
		enemy::drawshadow(dest, translucent);
	yofs=tempy;
}

void eKeese::draw(BITMAP *dest)
{
	update_enemy_frame();
	enemy::draw(dest);
}

eWizzrobe::eWizzrobe(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
//  switch(d->misc1)
	switch(dmisc1)
	{
	case 0:
		hxofs=1000;
		fading=fade_invisible;
		// Set clk to just before the 'reappear' threshold
		clk=zc_min(clk+(146+zc_max(0,dmisc5))+14,(146+zc_max(0,dmisc5))-1);
		break;
		
	default:
		dir=(loadside==right)?right:left;
		misc=-3;
		break;
	}
	
	//netst+2880;
	charging=false;
	firing=false;
	fclk=0;
	if(!dmisc1) frate=1200+146; //1200 = 20 seconds
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = d->zofs;
}

bool eWizzrobe::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(fallclk||drownclk) return enemy::animate(index);
	if(dying)
	{
		return Dead(index);
	}
	
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	if(dmisc1) // Floating
	{
		wizzrobe_attack();
	}
	else // Teleporting
	{
		if(watch)
		{
			fading=0;
			hxofs=0;
		}
		else switch(clk)
			{
			case 0:
				if(!dmisc2)
				{
			// Wizzrobe Misc4 controls whether wizzrobes can teleport on top of solid combos,
			// but should not appear on dungeon walls.	
					if ( FFCore.getQuestHeaderInfo(vZelda) <= 0x190 ) place_on_axis(true, false); //1.84, and probably 1.90 wizzrobes should NEVER appear in dungeon walls.-Z (1.84 confirmed, 15th January, 2019 by Chris Miller).
					else if (editorflags&ENEMY_FLAG5) 
			{
				//2.10 Windrobe
				//randomise location and face Link
			int32_t t=0;
			bool placed=false;
					
			while(!placed && t<160)
			{
				if(isdungeon())
				{
					x=((zc_oldrand()%12)+2)*16;
					y=((zc_oldrand()%7)+2)*16;
				}
				else
				{
					x=((zc_oldrand()%14)+1)*16;
					y=((zc_oldrand()%9)+1)*16;
				}
						
				if(!m_walkflag(x,y,spw_door, dir)&&((abs(x-Link.getX())>=32)||(abs(y-Link.getY())>=32)))
				{
					placed=true;
				}
						
				++t;
			}
					
			if(abs(x-Link.getX())<abs(y-Link.getY()))
			{
				if(y<Link.getY())
				{
					dir=down;
				}
				else
				{
					dir=up;
				}
			}
			else
			{
				if(x<Link.getX())
				{
					dir=right;
				}
				else
				{
					dir=left;
				}
			}
					
			if(!placed)                                       // can't place him, he's gone
				return true;
				
				
			//wizzrobe_attack(); //COmplaint about 2.10 Windrobes not behaving as they did in 2.10. Let's try it this way. -Z
			//wizzrobe_attack_for_real(); //doing this makes them fire twice. The rest is correct.
			}
			else place_on_axis(true, dmisc4!=0);
				}
				else
				{
					int32_t t=0;
					bool placed=false;
					
					while(!placed && t<160)
					{
						if(isdungeon())
						{
							x=((zc_oldrand()%12)+2)*16;
							y=((zc_oldrand()%7)+2)*16;
						}
						else
						{
							x=((zc_oldrand()%14)+1)*16;
							y=((zc_oldrand()%9)+1)*16;
						}
						
						if(!m_walkflag(x,y,spw_door, dir)&&((abs(x-Link.getX())>=32)||(abs(y-Link.getY())>=32)))
						{
							placed=true;
						}
						
						++t;
					}
					
					if(abs(x-Link.getX())<abs(y-Link.getY()))
					{
						if(y<Link.getY())
						{
							dir=down;
						}
						else
						{
							dir=up;
						}
					}
					else
					{
						if(x<Link.getX())
						{
							dir=right;
						}
						else
						{
							dir=left;
						}
					}
					
					if(!placed)                                       // can't place him, he's gone
						return true;
				}
				
				fading=fade_flicker;
				hxofs=0;
				break;
				
			case 64:
				fading=0;
				charging=true;
				break;
				
			case 73:
				charging=false;
				firing=40;
				break;
				
			case 83:
				wizzrobe_attack_for_real();
				break;
				
			case 119:
				firing=false;
				charging=true;
				break;
				
			case 128:
				fading=fade_flicker;
				charging=false;
				break;
				
			case 146:
				fading=fade_invisible;
				hxofs=1000;

				[[fallthrough]];
			default:
				if(clk>=(146+zc_max(0,dmisc5)))
					clk=-1;
					
				break;
			}
	}
	
	return enemy::animate(index);
}

void eWizzrobe::wizzrobe_attack_for_real()
{
	if(wpn==0)  // Edited enemies
		return;
		
	if(dmisc2 == 0)  //normal weapon
	{
		addEwpn(x,y,z,wpn,0,wdp,dir,getUID());
		sfx(WAV_WAND,pan(int32_t(x)));
	}
	else if(dmisc2 == 1) // ring of fire
	{
		addEwpn(x,y,z,wpn,0,wdp,up,getUID());
		((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
		addEwpn(x,y,z,wpn,0,wdp,down,getUID());
		((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
		addEwpn(x,y,z,wpn,0,wdp,left,getUID());
		((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
		addEwpn(x,y,z,wpn,0,wdp,right,getUID());
		((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
		addEwpn(x,y,z,wpn,0,wdp,l_up,getUID());
		((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
		addEwpn(x,y,z,wpn,0,wdp,r_up,getUID());
		((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
		addEwpn(x,y,z,wpn,0,wdp,l_down,getUID());
		((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
		addEwpn(x,y,z,wpn,0,wdp,r_down,getUID());
		((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
		sfx(WAV_FIRE,pan(int32_t(x)));
	if (get_bit(quest_rules, qr_8WAY_SHOT_SFX)) sfx(WAV_FIRE,pan(int32_t(x))); 
	else
	{
		switch(wpn)
		{
			case ewFireball: sfx(40,pan(int32_t(x))); break;
				
			case ewArrow: sfx(1,pan(int32_t(x))); break; //Ghost.zh has 0?
			case ewBrang: sfx(4,pan(int32_t(x))); break; //Ghost.zh has 0?
			case ewSword: sfx(20,pan(int32_t(x))); break; //Ghost.zh has 0?
			case ewRock: sfx(51,pan(int32_t(x))); break;
			case ewMagic: sfx(32,pan(int32_t(x))); break;
			case ewBomb: sfx(3,pan(int32_t(x))); break; //Ghost.zh has 0?
			case ewSBomb: sfx(3,pan(int32_t(x))); break; //Ghost.zh has 0?
			case ewLitBomb: sfx(21,pan(int32_t(x))); break; //Ghost.zh has 0?
			case ewLitSBomb:  sfx(21,pan(int32_t(x))); break; //Ghost.zh has 0?
			case ewFireTrail:  sfx(13,pan(int32_t(x))); break;
			case ewFlame: sfx(13,pan(int32_t(x))); break;
			case ewWind: sfx(32,pan(int32_t(x))); break;
			case ewFlame2: sfx(13,pan(int32_t(x))); break;
			case ewFlame2Trail: sfx(13,pan(int32_t(x))); break;
			case ewIce: sfx(44,pan(int32_t(x))); break;
			case ewFireball2: sfx(40,pan(int32_t(x))); break; //fireball (rising)
			default: sfx(WAV_FIRE,pan(int32_t(x)));  break;
			
		}
	}
	}
	else if(dmisc2==2)  // summons specific enemy
	{
		int32_t bc=0;
		
		for(int32_t gc=0; gc<guys.Count(); gc++)
		{
			if((((enemy*)guys.spr(gc))->id) == dmisc3)
			{
				++bc;
			}
		}
		
		if(bc<=40)
		{
			int32_t kids = guys.Count();
			int32_t bats=(zc_oldrand()%3)+1;
			
			for(int32_t i=0; i<bats; i++)
			{
				// Summon bats (or anything)
				if(addchild(x,y,dmisc3,-10, this->script_UID))
					((enemy*)guys.spr(kids+i))->count_enemy = false;
			}
			
			sfx(WAV_FIRE,pan(int32_t(x)));
		}
	}
	else if(dmisc2==3)  //summon from layer
	{
		if(count_layer_enemies()==0)
		{
			return;
		}
		
		int32_t kids = guys.Count();
		
		if(kids<200)
		{
			int32_t newguys=(zc_oldrand()%3)+1;
			bool summoned=false;
			
			for(int32_t i=0; i<newguys; i++)
			{
				int32_t id2=vbound(random_layer_enemy(),eSTART,eMAXGUYS-1);
				int32_t x2=0;
				int32_t y2=0;
				
				for(int32_t k=0; k<20; ++k)
				{
					x2=16*((zc_oldrand()%12)+2);
					y2=16*((zc_oldrand()%7)+2);
					
					if(!m_walkflag(x2,y2,0, dir) && (abs(x2-Link.getX())>=32 || abs(y2-Link.getY())>=32))
					{
						if(addchild(x2,y2,get_bit(quest_rules,qr_ENEMIESZAXIS) ? 64 : 0,id2,-10, this->script_UID))
							((enemy*)guys.spr(kids+i))->count_enemy = false;
							
						summoned=true;
						break;
					}
				}
			}
			
			if(summoned)
			{
				sfx(get_bit(quest_rules,qr_MORESOUNDS) ? WAV_ZN1SUMMON : WAV_FIRE,pan(int32_t(x)));
			}
		}
	}
}


void eWizzrobe::wizzrobe_attack()
{
	if(clk<0 || dying || stunclk || watch || ceiling || frozenclock)
		return;
		
	if(clk3<=0 || ((clk3&31)==0 && !canmove(dir,(zfix)1,spw_door,false) && !misc))
	{
		fix_coords();
		
		switch(misc)
		{
		case 1:                                               //walking
			if(!m_walkflag(x,y,spw_door, dir))
				misc=0;
			else
			{
				clk3=16;
				
				if(!canmove(dir,(zfix)1,spw_wizzrobe,false))
				{
					wizzrobe_newdir(0);
				}
			}
			
			break;
			
		case 2:                                               //phasing
		{
			int32_t jx=x;
			int32_t jy=y;
			int32_t jdir=-1;
			
			switch(zc_oldrand()&7)
			{
			case 0:
				jx-=32;
				jy-=32;
				jdir=15;
				break;
				
			case 1:
				jx+=32;
				jy-=32;
				jdir=9;
				break;
				
			case 2:
				jx+=32;
				jy+=32;
				jdir=11;
				break;
				
			case 3:
				jx-=32;
				jy+=32;
				jdir=13;
				break;
			}
			
			if(jdir>0 && jx>=32 && jx<=208 && jy>=32 && jy<=128)
			{
				misc=3;
				clk3=32;
				dir=jdir;
				break;
			}
		}
		[[fallthrough]];
		case 3:
			dir&=3;
			misc=0;
			[[fallthrough]];
		case 0:
			wizzrobe_newdir(64);
			[[fallthrough]];
		default:
			if(!canmove(dir,(zfix)1,spw_door,false))
			{
				if(canmove(dir,(zfix)15,spw_wizzrobe,false))
				{
					misc=1;
					clk3=16;
				}
				else
				{
					wizzrobe_newdir(64);
					misc=0;
					clk3=32;
				}
			}
			else
			{
				clk3=32;
			}
			
			break;
		}
		
		if(misc<0)
			++misc;
	}
	
	--clk3;
	
	switch(misc)
	{
	case 1:
	case 3:
		step=1;
		break;
		
	case 2:
		step=0;
		break;
		
	default:
		step=0.5;
		break;
		
	}
	
	move(step);
	
//  if(d->misc1 && misc<=0 && clk3==28)
	if(dmisc1 && misc<=0 && clk3==28)
	{
		if(dmisc2 != 1)
		{
			if(lined_up(8,false) == dir)
			{
//        addEwpn(x,y,z,wpn,0,wdp,dir,getUID());
//        sfx(WAV_WAND,pan(int32_t(x)));
				wizzrobe_attack_for_real();
				fclk=30;
			}
		}
		else
		{
			if((zc_oldrand()%500)>=400)
			{
				wizzrobe_attack_for_real();
				fclk=30;
			}
		}
	}
	
	if(misc==0 && (zc_oldrand()&127)==0)
		misc=2;
		
	if(misc==2 && clk3==4)
		fix_coords();
		
	if(!(charging||firing))                               //should never be charging or firing for these wizzrobes
	{
		if(fclk>0)
		{
			--fclk;
		}
	}
	
}

void eWizzrobe::wizzrobe_newdir(int32_t homing)
{
	// Wizzrobes shouldn't move to the edge of the screen;
	// if they're already there, they should move toward the center
	if(x<32)
		dir=right;
	else if(x>=224)
		dir=left;
	else if(y<32)
		dir=down;
	else if(y>=144)
		dir=up;
	else
		newdir(4,homing,spw_wizzrobe);
}

void eWizzrobe::draw(BITMAP *dest)
{
//  if(d->misc1 && (misc==1 || misc==3) && (clk3&1) && hp>0 && !watch && !stunclk)                          // phasing
	if(dmisc1 && (misc==1 || misc==3) && (clk3&1) && hp>0 && !watch && !stunclk && !frozenclock)                          // phasing
		return;
		
	int32_t tempint=dummy_int[1];
	bool tempbool1=dummy_bool[1];
	bool tempbool2=dummy_bool[2];
	dummy_int[1]=fclk;
	dummy_bool[1]=charging;
	dummy_bool[2]=firing;
	update_enemy_frame();
	dummy_int[1]=tempint;
	dummy_bool[1]=tempbool1;
	dummy_bool[2]=tempbool2;
	enemy::draw(dest);
}

/*********************************/
/**********   Bosses   ***********/
/*********************************/

eDodongo::eDodongo(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	fading=fade_flash_die;
	//nets+5120;
	if(dir==down&&y>=128)
	{
		dir=up;
	}
	
	if(dir==right&&x>=208)
	{
		dir=left;
	}
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)zofs;
}

bool eDodongo::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(dying)
	{
		return Dead(index);
	}
	
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	if(clk2)                                                  // ate a bomb
	{
		if(--clk2==0)
			hp-=misc;                                             // store bomb's power in misc
	}
	else
		constant_walk(rate,homing,spw_clipright);
		
	hxsz = (dir<=down) ? 16 : 32;
	//    hysz = (dir>=left) ? 16 : 32;
	
	return enemy::animate(index);
}

void eDodongo::draw(BITMAP *dest)
{
	tile=o_tile;
	
	if(clk<0)
	{
		enemy::drawzcboss(dest);
		return;
	}
	
	update_enemy_frame();
	enemy::drawzcboss(dest);
	
	if(dummy_int[1]!=0)  //additional tiles
	{
		tile+=dummy_int[1]; //second tile is previous tile
		xofs-=16;           //new xofs change
		enemy::drawzcboss(dest);
		xofs+=16;
	}
	
}

int32_t eDodongo::takehit(weapon *w)
{
	int32_t wpnId = w->id;
	int32_t power = w->power;
	int32_t wpnx = w->x;
	int32_t wpny = w->y;
	
	if(dying || clk<0 || clk2>0 || (superman && !(superman>1 && wpnId==wSBomb)))
		return 0;
		
	switch(wpnId)
	{
	case wPhantom:
		return 0;
		
	case wFire:
	case wBait:
	case wWhistle:
	case wWind:
	case wSSparkle:
	case wFSparkle:
		return 0;
		
	case wLitBomb:
	case wLitSBomb:
		if(abs(wpnx-((dir==right)?x+16:x)) > 7 || abs(wpny-y) > 7)
			return 0;
			
		clk2=96;
		misc=power;
		
		if(wpnId==wLitSBomb)
			item_set=isSBOMB100;
			
		return 1;
		
	case wBomb:
	case wSBomb:
		if(abs(wpnx-((dir==right)?x+16:x)) > 8 || abs(wpny-y) > 8)
			return 0;
			
		stunclk=160;
		misc=wpnId;                                           // store wpnId
		return 1;
		
	case wSword:
		if(stunclk)
		{
			sfx(WAV_EHIT,pan(int32_t(x)));
			hp=0;
			item_set = (misc==wSBomb) ? isSBOMB100 : isBOMB100;
			fading=0;                                           // don't flash
			return 1;
		}

		[[fallthrough]];
	default:
		sfx(WAV_CHINK,pan(int32_t(x)));
	}
	
	return 1;
}

eDodongo2::eDodongo2(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	fading=fade_flash_die;
	//nets+5180;
	previous_dir=-1;
	if(dir==down&&y>=128)
	{
		dir=up;
	}
	
	if(dir==right&&x>=208)
	{
		dir=left;
	}
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)zofs;
}

bool eDodongo2::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(dying)
	{
		return Dead(index);
	}
	
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	if(clk2)                                                  // ate a bomb
	{
		if(--clk2==0)
			hp-=misc;                                             // store bomb's power in misc
	}
	else
		constant_walk(rate,homing,spw_clipbottomright);
		
	hxsz = (dir<=down) ? 16 : 32;
	hysz = (dir>=left) ? 16 : 32;
	hxofs=(dir>=left)?-8:0;
	hyofs=(dir<left)?-8:0;
	
	return enemy::animate(index);
}

void eDodongo2::draw(BITMAP *dest)
{
	if(clk<0)
	{
		enemy::drawzcboss(dest);
		return;
	}
	
	int32_t tempx=xofs;
	int32_t tempy=yofs;
	update_enemy_frame();
	enemy::drawzcboss(dest);
	tile+=dummy_int[1]; //second tile change
	xofs+=dummy_int[2]; //new xofs change
	yofs+=dummy_int[3]; //new yofs change
	enemy::drawzcboss(dest);
	xofs=tempx;
	yofs=tempy;
}

int32_t eDodongo2::takehit(weapon *w)
{
	int32_t wpnId = w->id;
	int32_t power = w->power;
	int32_t wpnx = w->x;
	int32_t wpny = w->y;
	
	if(dying || clk<0 || clk2>0 || superman)
		return 0;
		
	switch(wpnId)
	{
	case wPhantom:
		return 0;
		
	case wFire:
	case wBait:
	case wWhistle:
	case wWind:
	case wSSparkle:
	case wFSparkle:
		return 0;
		
	case wLitBomb:
	case wLitSBomb:
		switch(dir)
		{
		case up:
			if(abs(wpnx-x) > 7 || abs(wpny-(y-8)) > 7)
				return 0;
				
			break;
			
		case down:
			if(abs(wpnx-x) > 7 || abs(wpny-(y+8)) > 7)
				return 0;
				
			break;
			
		case left:
			if(abs(wpnx-(x-8)) > 7 || abs(wpny-y) > 7)
				return 0;
				
			break;
			
		case right:
			if(abs(wpnx-(x+8)) > 7 || abs(wpny-y) > 7)
				return 0;
				
			break;
		}
		
		//          if(abs(wpnx-((dir==right)?x+8:(dir==left)?x-8:0)) > 7 || abs(wpny-((dir==down)?y+8:(dir==up)?y-8:0)) > 7)
		//            return 0;
		clk2=96;
		misc=power;
		
		if(wpnId==wLitSBomb)
			item_set=isSBOMB100;
			
		return 1;
		
	case wBomb:
	case wSBomb:
		switch(dir)
		{
		case up:
			if(abs(wpnx-x) > 7 || abs(wpny-(y-8)) > 7)
				return 0;
				
			break;
			
		case down:
			if(abs(wpnx-x) > 7 || abs(wpny-(y+8)) > 7)
				return 0;
				
			break;
			
		case left:
			if(abs(wpnx-(x-8)) > 7 || abs(wpny-y) > 7)
				return 0;
				
			break;
			
		case right:
			if(abs(wpnx-(x+8)) > 7 || abs(wpny-y) > 7)
				return 0;
				
			break;
		}
		
		stunclk=160;
		misc=wpnId;                                           // store wpnId
		return 1;
		
	case wSword:
		if(stunclk)
		{
			sfx(WAV_EHIT,pan(int32_t(x)));
			hp=0;
			item_set = (misc==wSBomb) ? isSBOMB100 : isBOMB100;
			fading=0;                                           // don't flash
			return 1;
		}

		[[fallthrough]];
	default:
		sfx(WAV_CHINK,pan(int32_t(x)));
	}
	
	return 1;
}

eAquamentus::eAquamentus(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)//enemy((zfix)176,(zfix)64,Id,Clk)
{
	//these are here to bypass compiler warnings about unused arguments
	if ( !(editorflags & ENEMY_FLAG5) )
	{
		x = dmisc1 ? 64 : 176;
		y = 64;
	}
	else { x = X; y = Y; }
	
	//nets+5940;
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
	}
	else
	{
		if(dmisc1)
		{
			flip=1;
		}
	}
	
	yofs=playing_field_offset+1;
	clk3=32;
	clk2=0;
	clk4=clk;
	dir=left;
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = d->zofs;
}

bool eAquamentus::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(dying)
		return Dead(index);
		
	//  fbx=x+((id==eRAQUAM)?4:-4);
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	fbx=x;
	
	/*
	  if (get_bit(quest_rules,qr_NEWENEMYTILES)&&id==eLAQUAM)
	  {
	  fbx+=16;
	  }
	  */
	if(--clk3==0)
	{
//    addEwpn(fbx,y,z,ewFireball,0,d->wdp,up+1);
//    addEwpn(fbx,y,z,ewFireball,0,d->wdp,0);
//    addEwpn(fbx,y,z,ewFireball,0,d->wdp,down+1);
		addEwpn(fbx,y,z,wpn,2,wdp,up,getUID());
		addEwpn(fbx,y,z,wpn,2,wdp,8,getUID());
		addEwpn(fbx,y,z,wpn,2,wdp,down,getUID());
		sfx(wpnsfx(wpn),pan(int32_t(x)));
	}
	
	if(clk3<-80 && !(zc_oldrand()&63))
	{
		clk3=32;
	}
	
	if(!((clk4+1)&63))
	{
		int32_t d2=(zc_oldrand()%3)+1;
		
		if(d2>=left)
		{
			dir=d2;
		}
		
		if(dmisc1)
		{
			if(x<=40)
			{
				dir=right;
			}
			
			if(x>=104)
			{
				dir=left;
			}
		}
		else
		{
			if(x<=136)
			{
				dir=right;
			}
			
			if(x>=200)
			{
				dir=left;
			}
		}
	}
	
	if(clk4>=-1 && !((clk4+1)&7))
	{
		if(dir==left)
		{
			x-=1;
		}
		else
		{
			x+=1;
		}
	}
	
	clk4=(clk4+1)%256;
	
	return enemy::animate(index);
}

void eAquamentus::draw(BITMAP *dest)
{
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
		xofs=(dmisc1?-16:0);
		if ( do_animation ) tile=o_tile+((clk&24)>>2)+(clk3>-32?(clk3>0?40:80):0);
		
		if(dying)
		{
			xofs=0;
			enemy::draw(dest);
		}
		else
		{
			drawblock(dest,15);
		}
	}
	else
	{
		int32_t xblockofs=((dmisc1)?-16:16);
		xofs=0;
		
		if(clk<0 || dying)
		{
			enemy::draw(dest);
			return;
		}
		if ( do_animation ) 
	{
		// face (0=firing, 2=resting)
		tile=o_tile+((clk3>0)?0:2);
		enemy::draw(dest);
		// tail (
		tile=o_tile+((clk&16)?1:3);
		xofs=xblockofs;
		enemy::draw(dest);
		// body
		yofs+=16;
		xofs=0;
		tile=o_tile+((clk&16)?20:22);
		enemy::draw(dest);
		xofs=xblockofs;
		tile=o_tile+((clk&16)?21:23);
		enemy::draw(dest);
		yofs-=16;
	}
	else enemy::draw(dest);
	}
}

bool eAquamentus::hit(weapon *w)
{
	if(!(w->scriptcoldet&1) || w->fallclk || w->drownclk) return false;
	
	switch(w->id)
	{
	case wBeam:
	case wRefBeam:
	case wMagic:
		hysz=32;
	}
	
	bool ret = (dying || hclk>0) ? false : sprite::hit(w);
	hysz=16;
	return ret;
	
}

eGohma::eGohma(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)  // enemy((zfix)128,(zfix)48,Id,0)
{
	
	if ( !(editorflags & ENEMY_FLAG5) )
	{
		x = 128;
		y = 48;
	}
	else { x = X; y = Y; }
	
	Clk=Clk;
	if(flags & guy_fadeflicker)
	{
		clk=0;
		superman = 1;
		fading=fade_flicker;
		if (!(editorflags&ENEMY_FLAG3)) count_enemy=false;
	}
	else if(flags & guy_fadeinstant)
	{
		clk=0;
	}
	hxofs=-16;
	hxsz=48;
	clk4=0;
	yofs=playing_field_offset+1;
	dir=zc_oldrand()%3+1;
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)zofs;
	
	//nets+5340;
}

bool eGohma::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(dying)
		return Dead(index);
		
	if(fading)
	{
		if(++clk4 > 60)
		{
			clk4=0;
			superman=0;
			fading=0;
			clk=0;
			
		}
		else return enemy::animate(index);
	}
		
	if(clk==0)
	{
		removearmos(zc_max(x-16, 0),y);
		did_armos = false;
		removearmos(x,y);
		did_armos = false;
		removearmos(zc_min(x+16, 255),y);
	}
	
	if(clk<0) return enemy::animate(index);
	
	// Movement clk must be separate from animation clk because of the Clock item
	if(!watch)
		clk4++;
		
	if((clk4&63)==0)
	{
		if(clk4&64)
			dir^=1;
		else
			dir=zc_oldrand()%3+1;
	}
	
	if((clk&63)==3)
	{
		switch(dmisc1)
		{
		case 1:
			addEwpn(x,y+2,z,wpn,3,wdp,left,getUID());
			addEwpn(x,y+2,z,wpn,3,wdp,8,getUID());
			addEwpn(x,y+2,z,wpn,3,wdp,right,getUID());
			sfx(wpnsfx(wpn),pan(int32_t(x)));
			break;
			
		default:
			if(dmisc1 != 1 && dmisc1 != 2)
			{
				addEwpn(x,y+2,z,wpn,3,wdp,8,getUID());
				sfx(wpnsfx(wpn),pan(int32_t(x)));
				sfx(wpnsfx(wpn),pan(int32_t(x)));
			}
			
			break;
		}
	}
	
	if((dmisc1 == 2)&& clk3>=16 && clk3<116)
	{
		if(!(clk3%8))
		{
			FireBreath(true);
		}
	}
	
	if(clk4&1)
		move((zfix)1);
		
	if(++clk3>=400)
		clk3=0;
		
	return enemy::animate(index);
}

void eGohma::draw(BITMAP *dest)
{
	tile=o_tile;
	
	if(clk<0 || dying)
	{
		enemy::drawzcboss(dest);
		return;
	}
	
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
	///if ( do_animation ) 
		//Yuck. Gohma can just not have this capability right now. 
		// left side
		xofs=-16;
		flip=0;
		//      if(clk&16) tile=180;
		//      else { tile=182; flip=1; }
		tile+=(3*((clk&48)>>4));
		enemy::drawzcboss(dest);
		
		// right side
		xofs=16;
		//      tile=(180+182)-tile;
		tile=o_tile;
		tile+=(3*((clk&48)>>4))+2;
		enemy::drawzcboss(dest);
		
		// body
		xofs=0;  //Gohma may need more adjustments for SIZEflags. -Z 14 Aug 2020
		tile=o_tile;
		
		//      tile+=(3*((clk&24)>>3))+2;
		if(clk3<16)
			tile+=7;
		else if(clk3<116)
			tile+=10;
		else if(clk3<132)
			tile+=7;
		else
			tile+=((clk3-132)&24)?4:1;
			
		enemy::drawzcboss(dest);
		
	}
	else
	{
		// left side
		xofs=-16;
		flip=0;
		
		if(!(clk&16))
		{
			tile+=2;
			flip=1;
		}
		
		enemy::draw(dest);
		
		// right side
		tile=o_tile;
		xofs=16;
		
		if((clk&16)) tile+=2;
		
		//      tile=(180+182)-tile;
		enemy::draw(dest);
		
		// body
		tile=o_tile;
		xofs=0;
		
		if(clk3<16)
			tile+=4;
		else if(clk3<116)
			tile+=5;
		else if(clk3<132)
			tile+=4;
		else tile+=((clk3-132)&8)?3:1;
		
		enemy::draw(dest);
		
	}
}

int32_t eGohma::takehit(weapon *w)
{
	int32_t wpnId = w->id;
	int32_t power = w->power;
	int32_t wpnx = w->x;
	int32_t wpnDir = w->dir;
	int32_t def = defenditemclassNew(wpnId, &power, w);
	
	if(def < 0)
	{
		if(!((wpnDir==up || wpnDir==l_up || wpnDir==r_up) && abs(int32_t(x)-wpnx)<=8 && clk3>=16 && clk3<116))
		{
			sfx(WAV_CHINK,pan(int32_t(x)));
			return 1;
		}
	}
	
	return enemy::takehit(w);
}

eLilDig::eLilDig(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	count_enemy=(id==(id&0xFFF));
	//nets+4360+(((id&0xFF)-eDIGPUP2)*40);
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = d->zofs;
}

bool eLilDig::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(dying)
		return Dead(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	if(misc<=128)
	{
		if(!(++misc&31))
			step+=0.25;
	}
	
	variable_walk_8(rate,homing,hrate,spw_floater);
	return enemy::animate(index);
}

void eLilDig::draw(BITMAP *dest)
{
	tile = o_tile;
	//    tile = 160;
	int32_t fdiv = frate/4;
	int32_t efrate = fdiv == 0 ? 0 : clk/fdiv;
	int32_t f2=get_bit(quest_rules,qr_NEWENEMYTILES)?
		   efrate:((clk>=(frate>>1))?1:0);
		   
	if ( do_animation ) 
	{
		if(get_bit(quest_rules,qr_NEWENEMYTILES))
		{
		switch(dir-8)                                           //directions get screwed up after 8.  *shrug*
		{
		case up:                                              //u
			flip=0;
			break;
			
		case l_up:                                            //d
			flip=0;
			tile+=4;
			break;
			
		case l_down:                                          //l
			flip=0;
			tile+=8;
			break;
			
		case left:                                            //r
			flip=0;
			tile+=12;
			break;
			
		case r_down:                                          //ul
			flip=0;
			tile+=20;
			break;
			
		case down:                                            //ur
			flip=0;
			tile+=24;
			break;
			
		case r_up:                                            //dl
			flip=0;
			tile+=28;
			break;
			
		case right:                                           //dr
			flip=0;
			tile+=32;
			break;
		}
		
		tile+=f2;
		}
		else
		{
		tile+=(clk>=6)?1:0;
		}
	}
	
	enemy::draw(dest);
}

eBigDig::eBigDig(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	superman=1;
	
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	else hxsz=32;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	else hysz=32;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	else hzsz=16; // hard to jump.
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	else hxofs=-8;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
	else hyofs=-8;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = d->zofs;
	
	
}

bool eBigDig::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(dying)
		return Dead(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	switch(misc)
	{
	case 0:
		variable_walk_8(rate,homing,hrate,spw_floater,-8,-16,23,23);
		break;
		
	case 1:
		++misc;
		break;
		
	case 2:
		for(int32_t i=0; i<dmisc5; i++)
		{
			addenemy(x,y,dmisc1+0x1000,-15);
		}
		
		for(int32_t i=0; i<dmisc6; i++)
		{
			addenemy(x,y,dmisc2+0x1000,-15);
		}
		
		for(int32_t i=0; i<dmisc7; i++)
		{
			addenemy(x,y,dmisc3+0x1000,-15);
		}
		
		for(int32_t i=0; i<dmisc8; i++)
		{
			addenemy(x,y,dmisc4+0x1000,-15);
		}
		
		if(itemguy) // Hand down the carried item
		{
			guycarryingitem = guys.Count()-1;
			((enemy*)guys.spr(guycarryingitem))->itemguy = true;
			itemguy = false;
		}
		
		stop_bgsfx(index);
		
		if(deadsfx > 0) sfx(deadsfx,pan(int32_t(x)));
		
		return true;
	}
	
	return enemy::animate(index);
}

void eBigDig::draw(BITMAP *dest)
{
	if(anim!=aDIG)
	{
		update_enemy_frame();
		xofs-=8;
		yofs-=8;
		drawblock(dest,15);
		xofs+=8;
		yofs+=8;
		return;
	}
	
	tile = o_tile;
	int32_t fdiv = frate/4;
	int32_t efrate = fdiv == 0 ? 0 : clk/fdiv;
	
	int32_t f2=get_bit(quest_rules,qr_NEWENEMYTILES)?
		   efrate:((clk>=(frate>>1))?1:0);
	   
	if ( do_animation ) 
	{	    
		if(get_bit(quest_rules,qr_NEWENEMYTILES))
		{
		switch(dir-8)                                           //directions get screwed up after 8.  *shrug*
		{
		case up:                                              //u
			flip=0;
			break;
			
		case l_up:                                            //d
			flip=0;
			tile+=8;
			break;
			
		case l_down:                                          //l
			flip=0;
			tile+=40;
			break;
			
		case left:                                            //r
			flip=0;
			tile+=48;
			break;
			
		case r_down:                                          //ul
			flip=0;
			tile+=80;
			break;
			
		case down:                                            //ur
			flip=0;
			tile+=88;
			
			break;
			
		case r_up:                                            //dl
			flip=0;
			tile+=120;
			break;
			
		case right:                                           //dr
			flip=0;
			tile+=128;
			break;
		}
		
		tile+=(f2*2);
		}
		else
		{
		tile+=(f2)?0:2;
		flip=(clk&1)?1:0;
		}
	}
	
	xofs-=8;
	yofs-=8;
	drawblock(dest,15);
	xofs+=8;
	yofs+=8;
}

int32_t eBigDig::takehit(weapon *w)
{
	int32_t wpnId = w->id;
	
	if(wpnId==wWhistle && misc==0)
		misc=1;
		
	return 0;
}

/*
eGanon::eGanon(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	hxofs=hyofs=8;
	hzsz=16; //can't be jumped.
	clk2=70;
	misc=-1;
	mainguy=!getmapflag();
}

bool eGanon::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(dying)
	
		return Dead(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	switch(misc)
	{
	case -1:
		misc=0;
		
	case 0:
		if(++clk2>72 && !(zc_oldrand()&3))
		{
			addEwpn(x,y,z,wpn,3,wdp,dir,getUID());
			sfx(wpnsfx(wpn),pan(int32_t(x)));
			clk2=0;
		}
		
		Stunclk=0;
		constant_walk(rate,homing,spw_none);
		break;
		
	case 1:
	case 2:
		if(--Stunclk<=0)
		{
			int32_t r=zc_oldrand();
			
			if(r&1)
			{
				y=96;
				
				if(r&2)
					x=160;
				else
					x=48;
					
				if(tooclose(x,y,48))
					x=208-x;
			}
			
		//if ( editorflags & ENEMY_FLAG15 && current_item_id(itype_amulet,false) >= 2 ) //visible to Amulet 2
		//{
	//	loadpalset(9,pSprite(spBROWN)); //make Ganon visible?
	//    }
		//    else
	//    {
		loadpalset(csBOSS,pSprite(d->bosspal));
	//    }
			misc=0;
		}
		
		break;
		
	case 3:
	{
		if(hclk>0)
			break;
			
		misc=4;
		clk=0;
		hxofs=1000;
		loadpalset(9,pSprite(spPILE));
		music_stop();
		stop_sfx(WAV_ROAR);
		
		if(deadsfx>0) sfx(deadsfx,pan(int32_t(x)));
		
		sfx(WAV_GANON);
		//Ganon's dustpile; fall in sideview. -Z
			item *dustpile = new item(x+8,y+8,(zfix)0,iPile,ipDUMMY,0);
		dustpile->linked_parent = eeGANON;
			setmapflag();
		//items.add(new item(x+8,y+8,(zfix)0,iPile,ipDUMMY,0));
		break;
	}
		
	case 4:
		if(clk>=80)
		{
			misc=5;
			
			if(getmapflag())
			{
				game->lvlitems[dlevel]|=liBOSS;
				//play_DmapMusic();
				playLevelMusic();
				return true;
			}
			
			sfx(WAV_CLEARED);
			items.add(new item(x+8,y+8,(zfix)0,iBigTri,ipBIGTRI,0));
			setmapflag();
		}
		
		break;
	}
	
	//if ( editorflags & ENEMY_FLAG15 ) //visible to Amulet 2
	//{
	//if ( current_item_id(itype_amulet,false) >= 2 )
	//{
	///	loadpalset(9,pSprite(spBROWN)); //make Ganon visible?
	//}
	//}
	
	
	return enemy::animate(index);
}


int32_t eGanon::takehit(weapon *w)
{
	//these are here to bypass compiler warnings about unused arguments
	int32_t wpnId = w->id;
	int32_t power = w->power;
	int32_t enemyHitWeapon = w->parentitem;
	
	switch(misc)
	{
		case 0:
		{
			//if we're not using the editor defences, and Ganon isn't hit by a sword, return.
		if(wpnId!=wSword && !(editorflags & ENEMY_FLAG14))
			return 0;
		
		//if we are not using the new defences, just reduce his HP
		if (!(editorflags & ENEMY_FLAG14)) 
		{
			hp-=power;
			if(hp>0)
			{
				misc=1;
				Stunclk=64;
			}
			else
			{
				loadpalset(csBOSS,pSprite(spBROWN));
				misc=2;
				Stunclk=284;
				hp=guysbuf[id&0xFFF].hp;                              //16*game->get_hero_dmgmult();
			}
			
			sfx(WAV_EHIT,pan(int32_t(x)));
			
			if(hitsfx>0) sfx(hitsfx,pan(int32_t(x)));
			
			return 1;
		}
		//otherwise, resolve his defence. 
		else 
		{
				int32_t def = enemy::takehit(w); //This works, but it instantly kills him if it does enough damage.
			if(hp>0)
			{
				misc=1;
				Stunclk=64;
			}
			else
			{
				loadpalset(csBOSS,pSprite(spBROWN));
				misc=2;
				Stunclk=284;
				hp=guysbuf[id&0xFFF].hp;                              //16*game->get_hero_dmgmult();
			}
			
			sfx(WAV_EHIT,pan(int32_t(x)));
			
			if(hitsfx>0) sfx(hitsfx,pan(int32_t(x)));
			
			
			return 1;
		}
		} 
		case 2:
		{
		if 
		(
			( dmisc14 > 0 && !enemyHitWeapon == dmisc14 ) //special weapon needed to kill ganon specified in editor
			|| //or nothing specified, use silver arrows+
			( dmisc14 <= 0 && (wpnId!=wArrow || (enemyHitWeapon>-1 ? itemsbuf[enemyHitWeapon].power : current_item_power(itype_arrow))<4))
		)
		return 0;
		{
			misc=3;
			hclk=81;
			loadpalset(9,pSprite(spBROWN));
			return 1;
		}
		
	   }
	}
	
	return 0;
}

void eGanon::draw(BITMAP *dest)
{
	switch(misc)
	{
	case 0:
		if((clk&3)==3)
			tile=(zc_oldrand()%5)*2+o_tile;
			
		if(db!=999)
			break;
			
	case 2:
		if(Stunclk<64 && (Stunclk&1) )
	{
		if 
		(
		( (editorflags & ENEMY_FLAG1) && current_item_power(itype_amulet) >= 2 && (editorflags & ENEMY_FLAG15) )
		||
		( (editorflags & ENEMY_FLAG2) && (game->item[dmisc13]) && (editorflags & ENEMY_FLAG15) )
		)
		{
			goto ganon_draw; //draw his weapons if we can see him
		}
			break;
		}
			
	case -1:
		tile=o_tile;
		
		//fall through
	case 1:
	case 3:
	ganon_draw:
		drawblock(dest,15);
		break;
		
	case 4:
		draw_guts(dest);
		draw_flash(dest);
		break;
	}
	
	if ( editorflags & ENEMY_FLAG1 ) //visible to Amulet 2
	{
	if 
	(
		( (editorflags & ENEMY_FLAG1) && current_item_power(itype_amulet) >= 2 && (editorflags & ENEMY_FLAG15) )
		||
		( (editorflags & ENEMY_FLAG2) && (game->item[dmisc13]) && (editorflags & ENEMY_FLAG15) )
	)
	{
		draw_guts(dest); //makes his shots visible, but not him
		draw_flash(dest);
	}
	}
}

void eGanon::draw_guts(BITMAP *dest)
{
	int32_t c = zc_min(clk>>3,8);
	tile = clk<24 ? 74 : 75;
	overtile16(dest,tile,x+8,y+c+playing_field_offset,9,0);
	overtile16(dest,tile,x+8,y+16-c+playing_field_offset,9,0);
	overtile16(dest,tile,x+c,y+8+playing_field_offset,9,0);
	overtile16(dest,tile,x+16-c,y+8+playing_field_offset,9,0);
	overtile16(dest,tile,x+c,y+c+playing_field_offset,9,0);
	overtile16(dest,tile,x+16-c,y+c+playing_field_offset,9,0);
	overtile16(dest,tile,x+c,y+16-c+playing_field_offset,9,0);
	overtile16(dest,tile,x+16-c,y+16-c+playing_field_offset,9,0);
}

void eGanon::draw_flash(BITMAP *dest)
{

	int32_t c = clk-(clk>>2);
	cs = (frame&3)+6;
	overtile16(dest,194,x+8,y+8-clk+playing_field_offset,cs,0);
	overtile16(dest,194,x+8,y+8+clk+playing_field_offset,cs,2);
	overtile16(dest,195,x+8-clk,y+8+playing_field_offset,cs,0);
	overtile16(dest,195,x+8+clk,y+8+playing_field_offset,cs,1);
	overtile16(dest,196,x+8-c,y+8-c+playing_field_offset,cs,0);
	overtile16(dest,196,x+8+c,y+8-c+playing_field_offset,cs,1);
	overtile16(dest,196,x+8-c,y+8+c+playing_field_offset,cs,2);
	overtile16(dest,196,x+8+c,y+8+c+playing_field_offset,cs,3);
}
*/

eGanon::eGanon(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	hxofs=hyofs=8;
	if (editorflags & ENEMY_FLAG3)
	{
		hxofs = 4;
		hyofs = 4;
		hxsz = 24;
		hysz = 24;
		SIZEflags|=guyflagOVERRIDE_HIT_WIDTH;
		SIZEflags|=guyflagOVERRIDE_HIT_HEIGHT;
	}
	hzsz=16; //can't be jumped.
	clk2=70;
	misc=-1;
	mainguy=(!getmapflag((currscr < 128 && get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)) ? mITEM : mBELOW) || (tmpscr->flags9&fBELOWRETURN));
}

bool eGanon::animate(int32_t index) //DO NOT ADD a check for do_animation to this version of GANON!! -Z
{
	if(dying)
	
		return Dead(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
   
	switch(misc)
	{
	case -1:
		misc=0;
		[[fallthrough]];
	case 0:
		if(++clk2>72 && !(zc_oldrand()&3))
		{
			addEwpn(x,y,z,wpn,3,wdp,dir,getUID());
			sfx(wpnsfx(wpn),pan(int32_t(x)));
			clk2=0;
		}
		
		Stunclk=0;
		constant_walk(rate,homing,spw_none);
		break;
		
	case 1:
	case 2:
		if(--Stunclk<=0)
		{
			int32_t r=zc_oldrand();
			
			if(r&1)
			{
				y=96;
				
				if(r&2)
					x=160;
				else
					x=48;
					
				if(tooclose(x,y,48))
					x=208-x;
			}
			
			loadpalset(csBOSS,pSprite(d->bosspal));
			misc=0;
		}
		
		break;
		
	case 3:
	{
		if(hclk>0)
			break;
			
		misc=4;
		clk=0;
		hxofs=1000;
		loadpalset(9,pSprite(spPILE));
		music_stop();
		stop_sfx(WAV_ROAR);
		
		if(deadsfx>0) sfx(deadsfx,pan(int32_t(x)));
		
		sfx(WAV_GANON);
		//Ganon's dustpile; fall in sideview. -Z
			//item *dustpile = new item(x+8,y+8,(zfix)0,iPile,ipDUMMY,0);
		//dustpile->miscellaneous[31] = eeGANON;
		items.add(new item(x+8,y+8,(zfix)0,iPile,ipDUMMY,0));
		item *dustpile = NULL;
		//dustpile = (item *)items.spr(items.Count() - 1)->getUID();
		dustpile = (item *)items.spr(items.Count() - 1);
		dustpile->linked_parent = eeGANON; //was miscellaneous[31]
			//setmapflag(); //Could be why the Triforce doesn't drop. Disabling this now. -Z ( 6th March, 2019 )
		//items.add(new item(x+8,y+8,(zfix)0,iPile,ipDUMMY,0));
		break;
	}
		
	case 4:
		if(clk>=80)
		{
			misc=5;
			
			if(getmapflag((currscr < 128 && get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)) ? mITEM : mBELOW) && !(tmpscr->flags9&fBELOWRETURN))
			{
				game->lvlitems[dlevel]|=liBOSS;
				//play_DmapMusic();
				playLevelMusic();
				return true;
			}
			
			sfx(WAV_CLEARED);
		//Add the big TF over the ashes!
		for(word q = 0; q < items.Count(); q++)
		{
			item *ashes = (item*)items.spr(q);
			if ( ashes->linked_parent == eeGANON )
			{
				//Z_scripterrlog("Found correct dustpile!\n");
				items.add(new item(ashes->x,ashes->y,(zfix)0,iBigTri,ipBIGTRI,0));
			}
		}
		setmapflag((currscr < 128 && get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)) ? mITEM : mBELOW);
		//game->lvlitems[dlevel]|=liBOSS; // if we had more rule bits, we could mark him dead so that he does not respawn. -Z
		}
		
		break;
	}
	
	return enemy::animate(index);
}


int32_t eGanon::takehit(weapon *w)
{
	//these are here to bypass compiler warnings about unused arguments
	int32_t wpnId = w->id;
	int32_t power = w->power;
	int32_t enemyHitWeapon = w->parentitem;
	
	switch(misc)
	{
	case 0:
		if(wpnId!=wSword)
			return 0;
			
		hp-=power;
		
		if(hp>0)
		{
			misc=1;
			Stunclk=64;
		}
		else
		{
			loadpalset(csBOSS,pSprite(spBROWN));
			misc=2;
			Stunclk=284;
			hp=guysbuf[id&0xFFF].hp;                              //16*game->get_hero_dmgmult();
		}
		
		sfx(WAV_EHIT,pan(int32_t(x)));
		
		if(hitsfx>0) sfx(hitsfx,pan(int32_t(x)));
		
		return 1;
		
	case 2:
		if(wpnId!=wArrow || (enemyHitWeapon>-1 ? itemsbuf[enemyHitWeapon].power : current_item_power(itype_arrow))<4)
			return 0;
			
		misc=3;
		hclk=81;
		loadpalset(9,pSprite(spBROWN));
		return 1;
	}
	
	return 0;
}

void eGanon::draw(BITMAP *dest)
{
	switch(misc)
	{
	case 0:
		if((clk&3)==3)
			tile=(zc_oldrand()%5)*2+o_tile;
			
		if ( (editorflags & ENEMY_FLAG1) && current_item_power(itype_amulet) >= 2 ) //ganon is visible to level 2 amulet
		{
		
			if ( editorflags & ENEMY_FLAG16 ) //draw cloaked
			{
				int odraw = drawstyle;
				drawstyle = 2;
				drawblock(dest,15);
				drawstyle = odraw;
			}
			else
			{
				drawblock(dest,15);
			}
			break;
		
		}
		else if ( (editorflags & ENEMY_FLAG2) && (game->item[dmisc13]) )
		{
			if ( editorflags & ENEMY_FLAG16 ) //draw cloaked
			{
				int odraw = drawstyle;
				drawstyle = 2;
				drawblock(dest,15);
				drawstyle = odraw;
			}
			else
			{
				drawblock(dest,15);
			}
			break;
		}
		if(db!=999)
			break;
		[[fallthrough]];
	case 2:
		if(Stunclk<64 && (Stunclk&1))
			break;
		[[fallthrough]];
	case -1:
		tile=o_tile;
		
		[[fallthrough]];
	case 1:
	case 3:
		drawblock(dest,15);
		break;
		
	case 4:
		draw_guts(dest);
		draw_flash(dest);
		break;
	}
}

void eGanon::draw_guts(BITMAP *dest)
{
	int32_t c = zc_min(clk>>3,8);
	tile = clk<24 ? 74 : 75;
	overtile16(dest,tile,x+8,y+c+playing_field_offset,9,0);
	overtile16(dest,tile,x+8,y+16-c+playing_field_offset,9,0);
	overtile16(dest,tile,x+c,y+8+playing_field_offset,9,0);
	overtile16(dest,tile,x+16-c,y+8+playing_field_offset,9,0);
	overtile16(dest,tile,x+c,y+c+playing_field_offset,9,0);
	overtile16(dest,tile,x+16-c,y+c+playing_field_offset,9,0);
	overtile16(dest,tile,x+c,y+16-c+playing_field_offset,9,0);
	overtile16(dest,tile,x+16-c,y+16-c+playing_field_offset,9,0);
}

void eGanon::draw_flash(BITMAP *dest)
{

	int32_t c = clk-(clk>>2);
	cs = (frame&3)+6;
	overtile16(dest,194,x+8,y+8-clk+playing_field_offset,cs,0);
	overtile16(dest,194,x+8,y+8+clk+playing_field_offset,cs,2);
	overtile16(dest,195,x+8-clk,y+8+playing_field_offset,cs,0);
	overtile16(dest,195,x+8+clk,y+8+playing_field_offset,cs,1);
	overtile16(dest,196,x+8-c,y+8-c+playing_field_offset,cs,0);
	overtile16(dest,196,x+8+c,y+8-c+playing_field_offset,cs,1);
	overtile16(dest,196,x+8-c,y+8+c+playing_field_offset,cs,2);
	overtile16(dest,196,x+8+c,y+8+c+playing_field_offset,cs,3);
}

void getBigTri(int32_t id2)
{
	/*
	  *************************
	  * BIG TRIFORCE SEQUENCE *
	  *************************
	  0 BIGTRI out, WHITE flash in
	  4 WHITE flash out, PILE cset white
	  8 WHITE in
	  ...
	  188 WHITE out
	  191 PILE cset red
	  200 top SHUTTER opens
	  209 bottom SHUTTER opens
	  */
	sfx(itemsbuf[id2].playsound);
	guys.clear();
	
	if(itemsbuf[id2].flags & ITEM_GAMEDATA)
	{
		game->lvlitems[dlevel]|=liTRIFORCE;
	}
	
	draw_screen(tmpscr);
	
	for(int32_t f=0; f<24*8 && !Quit; f++)
	{
		if(f==4)
		{
			for(int32_t i=1; i<16; i++)
			{
				RAMpal[CSET(9)+i]=_RGB(63,63,63);
			}
		}
		
		if((f&7)==0)
		{
			for(int32_t cs=2; cs<5; cs++)
			{
				for(int32_t i=1; i<16; i++)
				{
					RAMpal[CSET(cs)+i]=_RGB(63,63,63);
				}
			}
			
			refreshpal=true;
		}
		
		if((f&7)==4)
		{
			if(currscr<128) loadlvlpal(DMaps[currdmap].color);
			else loadlvlpal(0xB);
		}
		
		if(f==191)
		{
			loadpalset(9,pSprite(spPILE));
		}
		
		advanceframe(true);
	}
	
	//play_DmapMusic();
	playLevelMusic();
	
	if(itemsbuf[id2].flags & ITEM_FLAG1 && currscr < 128)
	{
		Link.dowarp(1,0); //side warp
	}
}

/**********************************/
/***  Multiple-Segment Enemies  ***/
/**********************************/


//! No. I am not adding SIZEflags to Moldorm and Lanmola. -Z 12 Aug 2020
eMoldorm::eMoldorm(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	if( !(editorflags & ENEMY_FLAG5) )
	{
		x=128;
		y=48;
	}
	//else { x = X; y = Y; }
	dir=(zc_oldrand()&7)+8;
	superman=1;
	fading=fade_invisible;
	hxofs=1000;
	segcnt=clk;
	segid=Id|0x1000;
	clk=0;
	id=guys.Count();
	yofs=playing_field_offset;
	tile=o_tile;
	hitdir = -1;
	stickclk = 0;
	
	/*
	  if (get_bit(quest_rules,qr_NEWENEMYTILES))
	  {
		tile=nets+1220;
	  }
	  else
	  {
		tile=57;
	  }
	*/
}

bool eMoldorm::animate(int32_t index)
{
	if(switch_hooked) return false;
	int32_t max_y = isdungeon() ? 100 : 100+28; //warning: Ugly hack. -Z
	if ( y > (max_y) )
	{
		++stickclk; //Keep Moldorm from pacinn the bottom row or leaving the screen via the bottom edge. -Z 8th Sept, 2019
		//Z_scripterrlog("Stickclk is %d\n", stickclk);
	}
	if ( stickclk > 45 )
	{
		stickclk = 0;
		newdir_8_old(rate,homing,spw_floater); //chage dir to keep from getting stuck.
	}
	

	if(clk==0)
	{
		removearmos(x,y);
	}
	
	if(clk2)
	{
		if(--clk2 == 0)
		{
			if(flags&guy_neverret)
				never_return(index);
				
			if(!dmisc2 || (editorflags & ENEMY_FLAG6))
				leave_item();
				
			stop_bgsfx(index);
			return true;
		}
	}
	else
	{
		if(stunclk>0)
			stunclk=0;
		constant_walk_8_old(rate,homing,spw_floater);
	
		
		misc=dir;
		
		// If any higher-numbered segments were killed, segcnt can be too high,
		// leading to a crash
		if(index+segcnt>=guys.Count())
			segcnt=guys.Count()-index-1;
		
		for(int32_t i=index+1; i<index+segcnt+1; i++)
		{
			enemy* segment=((enemy*)guys.spr(i));
			
			// More validation - if segcnt was wrong, this may not
			// actually be a Moldorm segment
			if(segment->id!=segid)
			{
				segcnt=i-index-1;
				break;
			}
			
			if(i==index+1)
			{
				x=segment->x;
				y=segment->y;
			}
			
			segment->o_tile=tile; //I refuse to fuck with adding scripttile to segmented enemies. -Z
		//Script your own blasted segmented bosses!! -Z
			segment->parent_script_UID = this->script_UID;
			if((i==index+segcnt)&&(i!=index+1))                   //tail
			{
				segment->dummy_int[1]=2;
			}
			else
			{
				segment->dummy_int[1]=1;
			}
			
			if(i==index+1)                                        //head
			{
				segment->dummy_int[1]=0;
			}
			
			if(segment->hp <= 0)
			{
				int32_t offset=1;
				
				for(int32_t j=i; j<index+segcnt; j++)
				{
					// Triple-check
					if(((enemy*)guys.spr(j+1))->id!=segid)
					{
						segcnt=j-index+1; // Add 1 because of --segcnt below
						break;
					}
					zc_swap(((enemy*)guys.spr(j))->hp,((enemy*)guys.spr(j+1))->hp);
					zc_swap(((enemy*)guys.spr(j))->hclk,((enemy*)guys.spr(j+1))->hclk);
				}
				
				segment->hclk=33;
				--segcnt;
				--i; // Recheck the same index in case multiple segments died at once
			}
		}
		
		if(segcnt==0)
		{
			clk2=19;
			
			x=guys.spr(index+1)->x;
			y=guys.spr(index+1)->y;
		}
	}
	
	return false;
}

esMoldorm::esMoldorm(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	if( !(editorflags & ENEMY_FLAG5) )
	{
		x=128;
		y=48;
	}
	
	yofs=playing_field_offset;
	hyofs=4;
	hxsz=hysz=8;
	hxofs=1000;
	mainguy=count_enemy=false;
	parentclk = 0;
	bgsfx=-1;
	flags&=~guy_neverret;
	//deadsfx = WAV_EDEAD;
	isCore = false;
}

bool esMoldorm::animate(int32_t index)
{
	if(switch_hooked) return false;
	// Shouldn't be possible, but better to be sure
	if(index==0)
		dying=true;
	
	if(dying)
	{
		if(!dmisc2)
			item_set=0;
			
		return Dead(index);
	}
	
	if(clk>=0)
	{
		hxofs=4;
		step=((enemy*)guys.spr(index-1))->step;
		
		if(parentclk == 0)
		{
			misc=dir;
			dir=((enemy*)guys.spr(index-1))->misc;
			//do alignment, as in parent's animation :-/ -DD
			x.doFloor();
			y.doFloor();
		}
		
		parentclk=(parentclk+1)%((int32_t)(8.0/step));
		
		if(!watch)
		{
			sprite::move(step);
		}
	}
	
	return enemy::animate(index);
}

int32_t esMoldorm::takehit(weapon *w)
{
	if(enemy::takehit(w))
		return (w->id==wSBomb) ? 1 : 2;                         // force it to wait a frame before checking sword attacks again
		
	return 0;
}

void esMoldorm::draw(BITMAP *dest)
{
	tile=o_tile;
	int32_t fdiv = frate/4;
	int32_t efrate = fdiv == 0 ? 0 : clk/fdiv;
	
	int32_t f2=get_bit(quest_rules,qr_NEWENEMYTILES)?
		   efrate:((clk>=(frate>>1))?1:0);
		   
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
		tile+=dummy_int[1]*40;
		
		if(dir<8)
		{
			flip=0;
			tile+=4*zc_max(dir, 0); // dir is -1 if trapped
			
			if(dir>3) // Skip to the next row for diagonals
				tile+=4;
		}
		else
		{
			switch(dir-8)                                           //directions get screwed up after 8.  *shrug*
			{
			case up:                                              //u
				flip=0;
				break;
				
			case l_up:                                            //d
				flip=0;
				tile+=4;
				break;
				
			case l_down:                                          //l
				flip=0;
				tile+=8;
				break;
				
			case left:                                            //r
				flip=0;
				tile+=12;
				break;
				
			case r_down:                                          //ul
				flip=0;
				tile+=20;
				break;
				
			case down:                                            //ur
				flip=0;
				tile+=24;
				break;
				
			case r_up:                                            //dl
				flip=0;
				tile+=28;
				break;
				
			case right:                                           //dr
				flip=0;
				tile+=32;
				break;
			}
		}
		
		tile+=f2;
	}
	
	if(clk>=0)
		enemy::draw(dest);
}

eLanmola::eLanmola(zfix X,zfix Y,int32_t Id,int32_t Clk) : eBaseLanmola(X,Y,Id,Clk)
{
	if( !(editorflags & ENEMY_FLAG5) )
	{
		x=64;
		y=80;
	}
	//else { x = X; y = Y; }
	//zprint2("lanmola index is %d\n", index);
	//byte legaldirs = 0;
	int32_t incr = 16;
	//int32_t possiiblepos = 0;
	//int32_t positions[8] = {0};
	
	//Don't spawn in pits. 
	if ( m_walkflag_simple(x, y) )
	{
		//zprint2("Can't spawn here.\n");
		for ( ; incr < 240; incr += 16 )
		{
			//move if we spawn over a pit
			//check each direction
			if ( !m_walkflag_simple(x-incr, y) ) //legaldirs |= 0x1; //left
			{
				//zprint2("Spawn adjustment: -x (%d)\n", incr);
				x-=incr; break;
			}
			else if ( !m_walkflag_simple(x+incr, y) ) //legaldirs |= 0x2; //right
			{
				//zprint2("Spawn adjustment: +x (%d)\n", incr);
				x+=incr; break;
			}
			else if ( !m_walkflag_simple(x-incr, y-incr) ) //legaldirs |= 0x4; //left-up
			{
				//zprint2("Spawn adjustment: -x (%d), -y (%d)\n", incr, incr);
				x-=incr; y-=incr; break;
			}
			else if ( !m_walkflag_simple(x+incr, y-incr) ) //legaldirs |= 0x8; //right-up
			{
				//zprint2("Spawn adjustment: +x (%d), -y (%d)\n", incr, incr);
				x+=incr; y-=incr; break;
			}
			else if ( !m_walkflag_simple(x, y-incr) ) // legaldirs |= 0x10; //up
			{
				//zprint2("Spawn adjustment: -y (%d)\n", incr);
				y -= incr; break;
			}
			else if ( !m_walkflag_simple(x, y+incr) ) //legaldirs |= 0x20; //down
			{
				//zprint2("Spawn adjustment: +y (%d)\n", incr);
				y+=incr; break;
			}
			else if ( !m_walkflag_simple(x-incr, y+incr) ) //legaldirs |= 0x40; //left-down
			{
				//zprint2("Spawn adjustment: -x (%d), +y (%d)\n", incr, incr);
				x-=incr; y+=incr; break;
			}
			else if ( !m_walkflag_simple(x+incr, y+incr) ) //legaldirs |= 0x80; //right-down
			{
				//zprint2("Spawn adjustment: +x (%d), +y (%d)\n", incr, incr);
				x+=incr; y+=incr; break;
			}
			else continue;
			
		}
		
	}
	
	dir=up;
	superman=1;
	fading=fade_invisible;
	hxofs=1000;
	segcnt=clk;
	clk=0;
	//set up move history
	for(int32_t i=0; i <= (1<<dmisc2); i++)
		prevState.push_back(std::pair<std::pair<zfix, zfix>, int32_t>(std::pair<zfix,zfix>(x,y), dir));
}

bool eLanmola::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	if(clk2)
	{
		if(--clk2 == 0)
		{
			if(!dmisc3) //This checks if  "segments drop items" isn't true, because if they don't drop items, then only killing the whole thing drops an item.
				leave_item();
				
			stop_bgsfx(index);
			return true;
		}
		
		return false;
	}
	
	
	//this animation style plays ALL KINDS of havoc on the Lanmola segments, since it causes
	//the direction AND x,y position of the lanmola to vary in uncertain ways.
	//I've added a complete movement history to this enemy to compensate -DD
	constant_walk(rate,homing,spw_none);
	prevState.pop_front();
	prevState.push_front(std::pair<std::pair<zfix, zfix>, int32_t>(std::pair<zfix, zfix>(x,y), dir));
	
	// This could cause a crash with Moldorms. I didn't see the same problem
	// with Lanmolas, but it looks like it ought to be possible, so here's
	// the same solution. - Saf
	if(index+segcnt>=guys.Count())
		segcnt=guys.Count()-index-1;
	
	for(int32_t i=index+1; i<index+segcnt+1; i++)
	{
		enemy* segment=((enemy*)guys.spr(i));
		
		// More validation in case segcnt is wrong
		if((segment->id&0xFFF)!=(id&0xFFF))
		{
			segcnt=i-index-1;
			break;
		}
		
		segment->o_tile=o_tile;
		segment->parent_script_UID = this->script_UID;
		if((i==index+segcnt)&&(i!=index+1))
		{
			segment->dummy_int[1]=1;                //tail
		}
		else
		{
			segment->dummy_int[1]=0;
		}
		
		if(segment->hp <= 0)
		{
			for(int32_t j=i; j<index+segcnt; j++)
			{
				// Triple-check
				if((((enemy*)guys.spr(j+1))->id&0xFFF)!=(id&0xFFF))
				{
					segcnt=j-index+1; // Add 1 because of --segcnt below
					break;
				}
				zc_swap(((enemy*)guys.spr(j))->hp,((enemy*)guys.spr(j+1))->hp);
				zc_swap(((enemy*)guys.spr(j))->hclk,((enemy*)guys.spr(j+1))->hclk);
			}
			
			((enemy*)guys.spr(i))->hclk=33;
			--segcnt;
			--i; // Recheck the same index in case multiple segments died at once
		}
	}
	
	if(segcnt==0)
	{
		clk2=19;
		x=guys.spr(index+1)->x;
		y=guys.spr(index+1)->y;
		setmapflag(mTMPNORET);
	}
	
	//this enemy is invincible.. BUT scripts don't know that, and can "kill" it by setting the hp negative.
	//which is... disastrous.
	hp = 1;
	return enemy::animate(index);
}

esLanmola::esLanmola(zfix X,zfix Y,int32_t Id,int32_t Clk) : eBaseLanmola(X,Y,Id,Clk)
{
	if( !(editorflags & ENEMY_FLAG5) )
	{
		x=64;
		y=80;
	}
	int32_t incr = 16;
	//Don't spawn in pits. 
	if ( m_walkflag_simple(x, y) )
	{
		//zprint2("Can't spawn here.\n");
		for ( ; incr < 240; incr += 16 )
		{
			//move if we spawn over a pit
			//check each direction
			if ( !m_walkflag_simple(x-incr, y) ) //legaldirs |= 0x1; //left
			{
				//zprint2("Spawn adjustment: -x (%d)\n", incr);
				x-=incr; break;
			}
			else if ( !m_walkflag_simple(x+incr, y) ) //legaldirs |= 0x2; //right
			{
				//zprint2("Spawn adjustment: +x (%d)\n", incr);
				x+=incr; break;
			}
			else if ( !m_walkflag_simple(x-incr, y-incr) ) //legaldirs |= 0x4; //left-up
			{
				//zprint2("Spawn adjustment: -x (%d), -y (%d)\n", incr, incr);
				x-=incr; y-=incr; break;
			}
			else if ( !m_walkflag_simple(x+incr, y-incr) ) //legaldirs |= 0x8; //right-up
			{
				//zprint2("Spawn adjustment: +x (%d), -y (%d)\n", incr, incr);
				x+=incr; y-=incr; break;
			}
			else if ( !m_walkflag_simple(x, y-incr) ) // legaldirs |= 0x10; //up
			{
				//zprint2("Spawn adjustment: -y (%d)\n", incr);
				y -= incr; break;
			}
			else if ( !m_walkflag_simple(x, y+incr) ) //legaldirs |= 0x20; //down
			{
				//zprint2("Spawn adjustment: +y (%d)\n", incr);
				y+=incr; break;
			}
			else if ( !m_walkflag_simple(x-incr, y+incr) ) //legaldirs |= 0x40; //left-down
			{
				//zprint2("Spawn adjustment: -x (%d), +y (%d)\n", incr, incr);
				x-=incr; y+=incr; break;
			}
			else if ( !m_walkflag_simple(x+incr, y+incr) ) //legaldirs |= 0x80; //right-down
			{
				//zprint2("Spawn adjustment: +x (%d), +y (%d)\n", incr, incr);
				x+=incr; y+=incr; break;
			}
			else continue;
			
		}
		
	}
	
	hxofs=1000;
	hxsz=8;
	mainguy=false;
	count_enemy=(id<0x2000)?true:false;
	
	//set up move history
	for(int32_t i=0; i <= (1<<dmisc2); i++)
		prevState.push_back(std::pair<std::pair<zfix, zfix>, int32_t>(std::pair<zfix,zfix>(x,y), dir));
		
	bgsfx = -1;
	isCore = false;
	flags&=~guy_neverret;
}

bool esLanmola::animate(int32_t index)
{
	if(switch_hooked) return false;
	// Shouldn't be possible, but who knows
	if(index==0)
		dying=true;
	
	if(dying)
	{
		xofs=0;
		
		if(!dmisc3)
			item_set=0;
			
		return Dead(index);
	}
	
	if(clk>=0)
	{
		hxofs=4;
		
		if(!watch)
		{
			std::pair<std::pair<zfix, zfix>, int32_t> newstate = ((eBaseLanmola*)guys.spr(index-1))->prevState.front();
			prevState.pop_front();
			prevState.push_back(newstate);
			x = newstate.first.first;
			y = newstate.first.second;
			dir = newstate.second;
		}
	}
	
	return enemy::animate(index);
}

int32_t esLanmola::takehit(weapon *w)
{
	if(enemy::takehit(w))
		return (w->id==wSBomb) ? 1 : 2;                         // force it to wait a frame before checking sword attacks again
		
	return 0;
}

void esLanmola::draw(BITMAP *dest)
{
	tile=o_tile;
	int32_t fdiv = frate/4;
	int32_t efrate = fdiv == 0 ? 0 : clk/fdiv;
	
	int32_t f2=get_bit(quest_rules,qr_NEWENEMYTILES)?
		   efrate:((clk>=(frate>>1))?1:0);
		   
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
		if(id>=0x2000)
		{
			tile+=20;
			
			if(dummy_int[1]==1)
			{
				tile+=20;
			}
		}
		
		switch(dir)
		{
		case up:
			flip=0;
			break;
			
		case down:
			flip=0;
			tile+=4;
			break;
			
		case left:
			flip=0;
			tile+=8;
			break;
			
		case right:
			flip=0;
			tile+=12;
			break;
		}
		
		tile+=f2;
	}
	else
	{
		if(id>=0x2000)
		{
			tile+=1;
		}
	}
	
	if(clk>=0)
		enemy::draw(dest);
}

eManhandla::eManhandla(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,0)
{
	//these are here to bypass compiler warnings about unused arguments
	Clk=Clk;
	superman=1;
	dir=(zc_oldrand()&7)+8;
	armcnt=dmisc2?8:4;//((id==eMANHAN)?4:8);
	
	for(int32_t i=0; i<armcnt; i++)
		arm[i]=i;
		
	fading=fade_blue_poof;
	//nets+4680;
	adjusted=false;
	SIZEflags = d->SIZEflags; //Probably will be buggy. -Z 12 AUG 2020
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = d->yofs+playing_field_offset; 
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = d->zofs;
}

bool eManhandla::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(dying)
		return Dead(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	
	// check arm status, move dead ones to end of group
	for(int32_t i=0; i<armcnt; i++)
	{
		if(!adjusted)
		{
			if(!dmisc2)
			{
				((enemy*)guys.spr(index+i+1))->o_tile=o_tile+40;
		enemy *s = ((enemy*)guys.spr(index+i+1));
		s->parent_script_UID = this->script_UID;
			}
			else
			{
				((enemy*)guys.spr(index+i+1))->o_tile=o_tile+160;
		enemy *s = ((enemy*)guys.spr(index+i+1));
		s->parent_script_UID = this->script_UID;
			}
		}
		
		if(((enemy*)guys.spr(index+i+1))->dying)
		{
			for(int32_t j=i; j<armcnt-1; j++)
			{
				zc_swap(arm[j],arm[j+1]);
				guys.swap(index+j+1,index+j+2);
				
			}
			if((editorflags & ENEMY_FLAG6)) //They only did this in 2.10
		{
				leave_item();
		}
			--armcnt;
		}
	}
	
	adjusted=true;
	
	// move or die
	if(armcnt==0)
		hp=0;
	else
	{
		// Speed starts at 0.5, and increases by 0.5 for each head lost. Max speed is 4.5.
		step=zc_min(4.5,(((!dmisc2)?4:8)-int64_t(armcnt))*0.5+zslongToFix(dstep*100));
		int32_t dx1=0, dy1=-8, dx2=15, dy2=15;
		
		if(!dmisc2)
		{
			for(int32_t i=0; i<armcnt; i++)
			{
				switch(arm[i])
				{
				case 0:
					dy1=-24;
					break;
					
				case 1:
					dy2=31;
					break;
					
				case 2:
					dx1=-16;
					break;
					
				case 3:
					dx2=31;
					break;
				}
			}
		}
		else
		{
			dx1=-8, dy1=-16, dx2=23, dy2=23;
			
			for(int32_t i=0; i<armcnt; i++)
			{
				switch(arm[i]&3)
				{
				case 0:
					dy1=-32;
					break;
					
				case 1:
					dy2=39;
					break;
					
				case 2:
					dx1=-24;
					break;
					
				case 3:
					dx2=39;
					break;
				}
			}
		}
		
		variable_walk_8(rate,homing,hrate,spw_floater,dx1,dy1,dx2,dy2);
		
		for(int32_t i=0; i<armcnt; i++)
		{
			zfix dx=(zfix)0,dy=(zfix)0;
			
			if(!dmisc2)
			{
				switch(arm[i])
				{
				case 0:
					dy=-16;
					break;
					
				case 1:
					dy=16;
					break;
					
				case 2:
					dx=-16;
					break;
					
				case 3:
					dx=16;
					break;
				}
			}
			else
			{
				switch(arm[i])
				{
				case 0:
					dy=-24;
					dx=-8;
					break;
					
				case 1:
					dy=24;
					dx=8;
					break;
					
				case 2:
					dx=-24;
					dy=8;
					break;
					
				case 3:
					dx=24;
					dy=-8;
					break;
					
				case 4:
					dy=-24;
					dx=8;
					break;
					
				case 5:
					dy=24;
					dx=-8;
					break;
					
				case 6:
					dx=-24;
					dy=-8;
					break;
					
				case 7:
					dx=24;
					dy=8;
					break;
				}
			}
			
			guys.spr(index+i+1)->x = x+dx;
			guys.spr(index+i+1)->y = y+dy;
		}
	}
	
	return enemy::animate(index);
}


int32_t eManhandla::takehit(weapon *w)
{
	int32_t wpnId = w->id;
	
	if(dying)
		return 0;
		
	switch(wpnId)
	{
	case wLitBomb:
	case wLitSBomb:
	case wBait:
	case wWhistle:
	case wFire:
	case wWind:
	case wSSparkle:
	case wFSparkle:
	case wPhantom:
		return 0;
		
	case wHookshot:
	case wBrang:
		sfx(WAV_CHINK,pan(int32_t(x)));
		break;
		
	default:
		sfx(WAV_EHIT,pan(int32_t(x)));
		
	}
	
	return 1;
}

void eManhandla::draw(BITMAP *dest)
{
	tile=o_tile;
	int32_t fdiv = frate/4;
	int32_t efrate = fdiv == 0 ? 0 : clk/fdiv;
	
	int32_t f2=get_bit(quest_rules,qr_NEWENEMYTILES)?
		   efrate:((clk>=(frate>>1))?1:0);
		   
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
		if(!dmisc2)
		{
			switch(dir-8)                                         //directions get screwed up after 8.  *shrug*
			{
			case up:                                            //u
				flip=0;
				break;
				
			case l_up:                                          //d
				flip=0;
				tile+=4;
				break;
				
			case l_down:                                        //l
				flip=0;
				tile+=8;
				break;
				
			case left:                                          //r
				flip=0;
				tile+=12;
				break;
				
			case r_down:                                        //ul
				flip=0;
				tile+=20;
				break;
				
			case down:                                          //ur
				flip=0;
				tile+=24;
				break;
				
			case r_up:                                          //dl
				flip=0;
				tile+=28;
				break;
				
			case right:                                         //dr
				flip=0;
				tile+=32;
				break;
			}
			
			tile+=f2;
			enemy::draw(dest);
		}                                                       //manhandla 2, big body
		else
		{
		
			switch(dir-8)                                         //directions get screwed up after 8.  *shrug*
			{
			case up:                                            //u
				flip=0;
				break;
				
			case l_up:                                          //d
				flip=0;
				tile+=8;
				break;
				
			case l_down:                                        //l
				flip=0;
				tile+=40;
				break;
				
			case left:                                          //r
				flip=0;
				tile+=48;
				break;
				
			case r_down:                                        //ul
				flip=0;
				tile+=80;
				break;
				
			case down:                                          //ur
				flip=0;
				tile+=88;
				break;
				
			case r_up:                                          //dl
				flip=0;
				tile+=120;
				break;
				
			case right:                                         //dr
				flip=0;
				tile+=128;
				break;
			}
			
			tile+=(f2*2);
			xofs-=8;
			yofs-=8;
			drawblock(dest,15);
			xofs+=8;
			yofs+=8;
		}
	}
	else
	{
		if(!dmisc2)
		{
			enemy::draw(dest);
		}
		else
		{
			xofs-=8;
			yofs-=8;
			enemy::draw(dest);
			xofs+=16;
			enemy::draw(dest);
			yofs+=16;
			enemy::draw(dest);
			xofs-=16;
			enemy::draw(dest);
			xofs+=8;
			yofs-=8;
		}
	}
}

esManhandla::esManhandla(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)
{
	id=misc=clk;
	dir = clk & 3;
	clk=0;
	mainguy=count_enemy=false;
	dummy_bool[0]=false;
	item_set=0;
	bgsfx=-1;
	deadsfx = WAV_EDEAD;
	flags &= (~guy_neverret);
	isCore = false;
	//Probably will be buggy. -Z 12 AUG 2020
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = d->yofs+playing_field_offset; 
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = d->zofs;
}

bool esManhandla::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(dying)
		return Dead(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	if(--clk2<=0)
	{
		clk2=unsigned(zc_oldrand())%5+5;
		clk3^=1;
	}
	
	if(!(zc_oldrand()&127))
	{
		addEwpn(x,y,z,wpn,3,wdp,dir,getUID());
		sfx(wpnsfx(wpn),pan(int32_t(x)));
	}
	
	return enemy::animate(index);
}

void esManhandla::draw(BITMAP *dest)
{
	tile=o_tile;
	int32_t fdiv = frate/4;
	int32_t efrate = fdiv == 0 ? 0 : clk/fdiv;
	int32_t f2=get_bit(quest_rules,qr_NEWENEMYTILES)?
		   efrate:((clk>=(frate>>1))?1:0);
		   
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
		switch(misc&3)
		{
		case up:
			break;
			
		case down:
			tile+=4;
			break;
			
		case left:
			tile+=8;
			break;
			
		case right:
			tile+=12;
			break;
		}
		
		tile+=f2;
	}
	else
	{
		switch(misc&3)
		{
		case down:
			flip=2;

			[[fallthrough]];
		case up:
			tile=(clk3)?188:189;
			break;
			
		case right:
			flip=1;
			[[fallthrough]];
			
		case left:
			tile=(clk3)?186:187;
			break;
		}
	}
	
	enemy::draw(dest);
}

eGleeok::eGleeok(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk) //enemy((zfix)120,(zfix)48,Id,Clk)
{
	if ( !(editorflags & ENEMY_FLAG5) )
	{
		x = 120;
		y = 48;
	}
	else 
	{ 
		if ( !(editorflags & ENEMY_FLAG6) )
		{
			x = X; y = Y; 
		}
		else 
		{
			x = X+8; y = Y; 
		}
	}
	hzsz = 32; // can't be jumped.
	flameclk=0;
	misc=clk;                                                 // total head count
	clk3=clk;                                                 // live head count
	clk=0;
	clk2=60;                                                  // fire ball clock
	//    hp=(guysbuf[eGLEEOK2+(misc-2)].misc2)*(misc-1)*game->get_hero_dmgmult()+guysbuf[eGLEEOK2+(misc-2)].hp;
	hp=(guysbuf[id&0xFFF].misc2)*(misc-1)*game->get_hero_dmgmult()+guysbuf[id&0xFFF].hp;
	dir = down;
	hxofs=4;
	hxsz=8;
	//    frate=17*4;
	fading=fade_blue_poof;
	//nets+5420;
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
		/*
			necktile=o_tile+8;
			if (dmisc3)
			{
			  necktile+=8;
			}
		*/
		necktile=o_tile+dmisc6;
	}
	else
	{
		necktile=s_tile;
	}
}

bool eGleeok::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(dying)
		return Dead(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	// Check if a head was killed somehow...
	if(index+1+clk3>=guys.Count())
		clk3=guys.Count()-index-1;
	if(index+1+misc>=guys.Count())
		misc=guys.Count()-index-1;
	
	//fix for the "kill all enemies" item
	if(hp==-1000)
	{
		for(int32_t i=0; i<clk3; ++i)
		{
			// I haven't seen this fail, but it seems like it ought to be
			// possible, so I'm checking for it. - Saf
			if((((enemy*)guys.spr(index+i+1))->id&0xFFF)!=(id&0xFFF))
				break;
			((enemy*)guys.spr(index+i+1))->hp=1;                   // re-animate each head,
			((enemy*)guys.spr(index+i+1))->misc = -1;              // disconnect it,
			((enemy*)guys.spr(index+i+1))->animate(index+i+1);     // let it animate one frame,
			((enemy*)guys.spr(index+i+1))->hp=-1000;               // and kill it for good
		}
		
		clk3=0;
		
		for(int32_t i=0; i<misc; i++)
		{
			if((((enemy*)guys.spr(index+i+1))->id&0xFFF)!=(id&0xFFF))
				break;
			((enemy*)guys.spr(index+i+1))->misc = -2;             // give the signal to disappear
		}
	}
	
	for(int32_t i=0; i<clk3; i++)
	{
		enemy *head = ((enemy*)guys.spr(index+i+1));
		head->dummy_int[1]=necktile;
		head->parent_script_UID = this->script_UID;
		
		if(get_bit(quest_rules,qr_NEWENEMYTILES))
		{
			head->dummy_int[2]=o_tile+dmisc8; //connected head tile
			head->dummy_int[3]=o_tile+dmisc9; //flying head tile
		}
		else
		{
			head->dummy_int[2]=necktile+1; //connected head tile
			head->dummy_int[3]=necktile+2; //flying head tile
		}
		
		head->dmisc5=dmisc5; //neck segments
		
		/*
			if (dmisc3)
			{
			  head->dummy_bool[0]=true;
			}
		*/
		if(head->hclk)
		{
			if(hclk==0)
			{
				hp -= 1000 - head->hp;
				hclk = 33;
				
				if(hitsfx>0) sfx(hitsfx,pan(int32_t(head->x)));
				
				sfx(WAV_EHIT,pan(int32_t(head->x)));
			}
			
			head->hclk = 0;
		}
		
		// Must be set in case of naughty ZScripts
		head->hp = 1000;
	}
	
	if(hp<=(guysbuf[id&0xFFF].misc2)*(clk3-1)*game->get_hero_dmgmult())
	{
		((enemy*)guys.spr(index+clk3))->misc = -1;              // give signal to fly off
		hp=(guysbuf[id&0xFFF].misc2)*(--clk3)*game->get_hero_dmgmult();
	}
	
	if(!dmisc3)
	{
		if(++clk2>72 && !(zc_oldrand()&3))
		{
			int32_t i=zc_oldrand()%misc;
			enemy *head = ((enemy*)guys.spr(index+i+1));
			addEwpn(head->x,head->y,head->z,wpn,3,wdp,dir,getUID());
			sfx(wpnsfx(wpn),pan(int32_t(x)));
			clk2=0;
		}
	}
	else
	{
		if(++clk2>100 && !(zc_oldrand()&3))
		{
			enemy *head = ((enemy*)guys.spr(zc_oldrand()%misc+index+1));
			head->timer=zc_oldrand()%50+50;
			clk2=0;
		}
	}
	
	if((hp<=0 && !immortal))
	{
		for(int32_t i=0; i<misc; i++)
			((enemy*)guys.spr(index+i+1))->misc = -2;             // give the signal to disappear
			
		if(flags&guy_neverret) never_return(index);
	}
	
	return enemy::animate(index);
}

int32_t eGleeok::takehit(weapon*)
{
	return 0;
}

void eGleeok::draw(BITMAP *dest)
{
	tile=o_tile;
	
	if(dying)
	{
		enemy::draw(dest);
		return;
	}
	
	int32_t f=clk/17;
	
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
		// body
		xofs=-8;
		yofs=32;
		
		switch(f)
		
		{
		case 0:
			tile+=0;
			break;
			
		case 1:
			tile+=2;
			break;
			
		case 2:
			tile+=4;
			break;
			
		default:
			tile+=6;
			break;
		}
	}
	else
	{
		// body
		xofs=-8;
		yofs=32;
		
		switch(f)
		{
		case 0:
			tile+=0;
			break;
			
		case 2:
			tile+=4;
			break;
			
		default:
			tile+=2;
			break;
		}
	}
	
	enemy::drawblock(dest,15);
}

void eGleeok::draw2(BITMAP *dest)
{
	// the neck stub
	tile=necktile;
	xofs=0;
	yofs=playing_field_offset;
	
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
		tile+=((clk&24)>>3);
	}
	
	if(hp > 0 && !dont_draw())
	{
		if((tmpscr->flags3&fINVISROOM)&& !(current_item(itype_amulet)))
			sprite::drawcloaked(dest);
		else
			sprite::draw(dest);
	}
}

esGleeok::esGleeok(zfix X,zfix Y,int32_t Id,int32_t Clk, sprite * prnt) : enemy(X,Y,Id,Clk), parent(prnt)
{
	xoffset=0;
	yoffset=(zfix)((dmisc5*4+2));
//  dummy_bool[0]=false;
	timer=0;
	/*  fixing */
	hp=1000;
	step=1;
	item_set=0;
	//x=120; y=70;
	x = xoffset+parent->x;
	y = yoffset+parent->y;
	hxofs=4;
	hxsz=8;
	yofs=playing_field_offset;
	clk2=clk;                                                 // how int32_t to wait before moving first time
	clk=0;
	mainguy=count_enemy=false;
	dir=zc_oldrand();
	clk3=((dir&2)>>1)+2;                                      // left or right
	dir&=1;                                                   // up or down
	dmisc5=vbound(dmisc5,1,255);
	isCore = false;
	parentCore = parent->getUID();
	for(int32_t i=0; i<dmisc5; i++)
	{
		nxoffset[i] = 0;
		nyoffset[i] = 0;
		nx[i] = ((((i*(int32_t)x) + (dmisc5-i)*((int32_t)parent->x))) /dmisc5);
		ny[i] = ((((i*(int32_t)y) + (dmisc5-i)*((int32_t)parent->y))) /dmisc5);
	}
	
	necktile=0;
	//TODO compatibility? -DD
	/*
	for(int32_t i=0; i<4; i++)
	{
	  nx[i]=124;
	  ny[i]=i*6+48;
	}*/
	bgsfx=-1;
	//no need for deadsfx
}

bool esGleeok::animate(int32_t index)
{
	if(switch_hooked) return false;
	// don't call removearmos() - it's a segment.
	
	dmisc5=vbound(dmisc5,1,255);
	
	if(misc == 0)
	{
		x = (xoffset+parent->x);
		y = (yoffset+parent->y);
		
		for(int32_t i=0; i<dmisc5; i++)
		{
			nx[i] = ((((i*(int32_t)x) + (dmisc5-i)*((int32_t)parent->x))) /dmisc5) + 3 + nxoffset[i];
			ny[i] = ((((i*(int32_t)y) + (dmisc5-i)*((int32_t)parent->y))) /dmisc5) + nyoffset[i];
		}
	}
	
	//  set up the head tiles
//  headtile=nets+5588;                                       //5580, actually.  must adjust for direction later on
	/*
	  if (dummy_bool[0])                                        //if this is a flame gleeok
	  {
		headtile+=180;
	  }
	*/
	headtile=dummy_int[2];                                       //5580, actually.  must adjust for direction later on
	flyingheadtile=dummy_int[3];
	
	//  set up the neck tiles
	necktile=dummy_int[1];
	
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
		necktile+=((clk&24)>>3);
	}
	
	/*
	  else
	  {
		necktile=145;
	  }
	*/
	//    ?((dummy_bool[0])?(nets+4052+(16+((clk&24)>>3))):(nets+4040+(8+((clk&24)>>3)))):145)
	
	switch(misc)
	{
	case 0:                                                 // live head
		//  set up the attached head tiles
		tile=headtile;
		
		if(get_bit(quest_rules,qr_NEWENEMYTILES))
		{
			tile+=((clk&24)>>3);
			/*
			  if (dummy_bool[0]) {
			  tile+=1561;
			  }
			  */
		}
		
		/*
			else
			{
			  tile=146;
			}
		*/
		if(++clk2>=0 && !(clk2&3))
		{
			if(y<= (int32_t)parent->y + 8) dir=down;
			
			if(y>= (int32_t)parent->y + dmisc5*8) dir = up;
			
			if(y<= (int32_t)parent->y + 10 && !(zc_oldrand()&31))
			{
				dir^=1;
			}
			
			zfix tempx = x;
			zfix tempy = y;
			
			sprite::move(step);
			xoffset += (x-tempx);
			yoffset += (y-tempy);
			
			if(clk2>=4)
			{
				clk3^=1;
				clk2=-4;
			}
			else
			{
				if(x <= (int32_t)parent->x-(dmisc5*6))
				{
					clk3=right;
				}
				
				if(x >= (int32_t)parent->x+(dmisc5*6))
				{
					clk3=left;
				}
				
				if(y <= (int32_t)parent->y+(dmisc5*6) && !(zc_oldrand()&15))
				{
					clk3^=1;                                        // x jig
				}
				else
				{
					if(y<=(int32_t)parent->y+(dmisc5*4) && !(zc_oldrand()&31))
					{
						clk3^=1;                                      // x switch back
					}
					
					clk2=-4;
				}
			}
			
			zc_swap(dir,clk3);
			tempx = x;
			tempy = y;
			sprite::move(step);
			xoffset += (x-tempx);
			yoffset += (y-tempy);
			zc_swap(dir,clk3);
			
			for(int32_t i=1; i<dmisc5; i++)
			{
				nxoffset[i] = (zc_oldrand()%3);
				nyoffset[i] = (zc_oldrand()%3);
			}
		}
		
		break;
		
	case 1:                                                 // flying head
		if(clk>=0)
		
		{
			variable_walk_8(rate,homing,hrate,spw_floater);
		}
		
		break;
		
		// the following are messages sent from the main guy...
	case -1:                                                // got chopped off
	{
		misc=1;
		superman=1;
		hxofs=xofs=0;
		hxsz=16;
		cs=8;
		clk=-24;
		clk2=40;
		dir=(zc_oldrand()&7)+8;
		step=8.0/9.0;
	}
	break;
	
	case -2:                                                // the big guy is dead
		return true;
	}
	
	if(timer)
	{
		if(!(timer%8))
		{
			FireBreath(true);
		}
		
		--timer;
	}
	
	return enemy::animate(index);
}

int32_t esGleeok::takehit(weapon *w)
{
	int32_t ret = enemy::takehit(w);
	
	if(ret==-1)
		return 2; // force it to wait a frame before checking sword attacks again
		
	return ret;
}

void esGleeok::draw(BITMAP *dest)
{
	dmisc5=vbound(dmisc5,1,255);
	
	switch(misc)
	{
	case 0:                                                 //neck
		if(!dont_draw())
		{
			for(int32_t i=1; i<dmisc5; i++)                              //draw the neck
			{
				if(get_bit(quest_rules,qr_NEWENEMYTILES))
				{
					if((tmpscr->flags3&fINVISROOM)&& !(current_item(itype_amulet)))
						overtilecloaked16(dest,necktile+(i*dmisc7),nx[i]-4,ny[i]+playing_field_offset,0);
					else
						overtile16(dest,necktile+(i*dmisc7),nx[i]-4,ny[i]+playing_field_offset,cs,0);
				}
				else
				{
					if((tmpscr->flags3&fINVISROOM)&& !(current_item(itype_amulet)))
						overtilecloaked16(dest,necktile,nx[i]-4,ny[i]+playing_field_offset,0);
					else
						overtile16(dest,necktile,nx[i]-4,ny[i]+playing_field_offset,cs,0);
				}
			}
		}
		
		break;
		
	case 1:                                                 //flying head
		tile=flyingheadtile;
		
		if(get_bit(quest_rules,qr_NEWENEMYTILES))
		{
			tile+=((clk&24)>>3);
			break;
		}
		
		/*
			else
			{
			  tile=(clk&1)?147:148;
			  break;
			}
		*/
	}
}

void esGleeok::draw2(BITMAP *dest)
{
	enemy::draw(dest);
}

ePatra::ePatra(zfix X,zfix Y,int32_t Id,int32_t Clk) : enemy(X,Y,Id,Clk)// enemy((zfix)128,(zfix)48,Id,Clk)
{
	if ( !(editorflags & ENEMY_FLAG5) )
	{
		x = 128;
		y = 48;
	}
	else { x = X; y = Y; }
	adjusted=false;
	dir=(zc_oldrand()&7)+8;
	//step=0.25;
	flycnt=dmisc1;
	flycnt2=dmisc2;
	loopcnt=0;
	clk4 = 0;
	clk5 = 0;
	clk6 = 0;
	clk7 = 0;
	if(dmisc6<int16_t(1))dmisc6=1; // ratio cannot be 0!
	SIZEflags = d->SIZEflags;
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	else if (dmisc10 == 1) { txsz = 2; extend = 3; }
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = tysz; if ( tysz > 1 ) extend = 3; }
	else if (dmisc10 == 1) { tysz = 2; extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = hxsz;
	else if (dmisc10 == 1) hxsz = 32;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = hxofs;
	else if (dmisc10 == 1) hxofs = -8;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)xofs;
	else if (dmisc10 == 1) xofs = -8;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
	else if (dmisc10 == 1) yofs = playing_field_offset-8;
	if (editorflags & ENEMY_FLAG8) misc = 1;
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)zofs;
	
	if (dmisc29 == 0)
	{
		if(!dmisc4)
		{
			if (dmisc10) dmisc29 = (90 / 3);
			else dmisc29 = (84 / 3);
		}
		else
		{
			if (dmisc10) dmisc29 = (90 / 2);
			else dmisc29 = (84 / 2);
		}
	}
	if (dmisc30 == 0)
	{
		if(!dmisc4)
		{
			if (dmisc10) dmisc30 = (90 / 3)*0.5;
			else dmisc30 = (84 / 3)*0.5;
		}
		else
		{
			if (dmisc10) dmisc30 = (90 / 2)*0.5;
			else dmisc30 = (84 / 2)*0.5;
		}
	}
	if (dmisc31 == 0)
	{
		if(!dmisc4)
		{
			if (dmisc10) dmisc31 = (90 / 3)*2;
			else dmisc31 = (84 / 3)*2;
		}
		else
		{
			if (dmisc10) dmisc31 = (90 / 2)*0.5;
			else dmisc31 = (84 / 2)*0.5;
		}
	}
	if (dmisc32 == 0)
	{
		if(!dmisc4)
		{
			if (dmisc10) dmisc32 = (90 / 3);
			else dmisc32 = (84 / 3);
		}
		else
		{
			if (dmisc10) dmisc32 = (90 / 2)*0.25;
			else dmisc32 = (84 / 2)*0.25;
		}
	}
}

bool ePatra::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(dying)
	{
		for(int32_t i=index+1; i<index+flycnt+flycnt2+1; i++)
		{
			((enemy*)guys.spr(i))->hp = -1000;
		}
		
		return Dead(index);
	}
	
	double basesize = 84;
	if (dmisc10) basesize = 90;
	double halfsize = basesize / 2;
	double quartersize = halfsize / 2;
	double twothirdsize = (basesize / 3)*2;
	double onethirdsize = (basesize / 3);

	
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	if ((clk4 <=0 || clk4%2) && (clk7 <= 0 || clk6 <= -16))
	{
		if (!dmisc22 || loopcnt == 0 || (dmisc22 == 1 && loopcnt < 0)) variable_walk_8(rate,homing,hrate,spw_floater);
		if (loopcnt < 0) ++clk2;
		if(++clk2>basesize)
		{
			clk2=0;
			if ((!dmisc26 || (dmisc26 == 1 && flycnt) || (dmisc26 == 2 && !flycnt)) && (!(editorflags & ENEMY_FLAG10) || flycnt || flycnt2)) 
			{
				if(loopcnt > 0)
					--loopcnt;
				else if (loopcnt == 0)
				{
					if((misc%dmisc6)==0)
					{
						if (dmisc21 > 0) loopcnt=-dmisc21;
						else loopcnt=dmisc7;
					}
				}
				else if (loopcnt == -1) loopcnt=dmisc7;
				else ++loopcnt;
				
				if (!(editorflags & ENEMY_FLAG9) || loopcnt == 0) ++misc;
			}
			else 
			{
				loopcnt = 0;
				misc = 1;
			}
		}
	}
	if (clk4 > 0) --clk4;
	
	double size=1;
	
	if (clk6 < 0)
	{
		if (dmisc5 == 1 || dmisc5 == 3)
		{
			if (get_bit(quest_rules,qr_NEWENEMYTILES))
			{
				if (clk7 <= 0 || clk6 != -16) ++clk6;
				if (clk6 == 0) o_tile=d->e_tile;
				else
				{
					if (clk6 >= -16) o_tile=d->e_tile + (IsBigAnim() ? 320 : 80);
					else o_tile=d->e_tile + (IsBigAnim() ? 160 : 40);
				}
			}
			else clk6 = 0;
		}
	}
	else if (dmisc19) ++clk6;
	if (clk5 < 0) ++clk5;
	else if (dmisc19) ++clk5;
	
	if (clk7 > 0 && clk6 >= -16) --clk7;
	if (clk6 > 0) clk7 = 0;
	
	for(int32_t i=index+1; i<index+flycnt+1; i++)
	{
		//outside ring
		if(!adjusted)
		{
			if(get_bit(quest_rules,qr_NEWENEMYTILES))
			{
				((enemy*)guys.spr(i))->o_tile=d->e_tile+dmisc8;
				enemy *s = ((enemy*)guys.spr(i));
				s->parent_script_UID = this->script_UID;
			}
			else
			{
				((enemy*)guys.spr(i))->o_tile=o_tile+1;
				enemy *s = ((enemy*)guys.spr(i));
				s->parent_script_UID = this->script_UID;
			}
			
			((enemy*)guys.spr(i))->cs=dmisc9;
			((enemy*)guys.spr(i))->hp=dmisc3;
		}
		
		if(((enemy*)guys.spr(i))->hp <= 0)
		{
			for(int32_t j=i; j<index+flycnt+flycnt2; j++)
			{
				guys.swap(j,j+1);
			}
			
			if (--flycnt == 0 && dmisc23 != 0) step += zslongToFix(dmisc23*100);
		}
		else
		{
			int32_t pos2 = ((enemy*)guys.spr(i))->misc;
			double a2 = (clk2-pos2*(double)basesize/(dmisc1 == 0 ? 1 : dmisc1))*PI/halfsize;
			
			if(!dmisc4) //Big Ring
			{
				//maybe playing_field_offset here?
				if(loopcnt>0)
				{
					guys.spr(i)->x =  cos(a2+PI/2)*abs(dmisc31) - sin(pos2*PI*2/(dmisc1 == 0 ? 1 : dmisc1))*((int64_t)abs(dmisc31)-abs(dmisc29));
					guys.spr(i)->y = -sin(a2+PI/2)*abs(dmisc31) + cos(pos2*PI*2/(dmisc1 == 0 ? 1 : dmisc1))*((int64_t)abs(dmisc31)-abs(dmisc29));
				}
				else
				{
					guys.spr(i)->x =  cos(a2+PI/2)*abs(dmisc29);
					guys.spr(i)->y = -sin(a2+PI/2)*abs(dmisc29);
				}
				
				temp_x=guys.spr(i)->x;
				temp_y=guys.spr(i)->y;
			}
			else //Oval
			{
				circle_x =  cos(a2+PI/2)*abs(dmisc29);
				circle_y = -sin(a2+PI/2)*abs(dmisc29);
				
				if(loopcnt>0)
				{
					guys.spr(i)->x =  cos(a2+PI/2)*abs(dmisc29);
					guys.spr(i)->y = (-sin(a2+PI/2)-cos(pos2*PI*2/(dmisc1 == 0 ? 1 : dmisc1)))*abs(dmisc31);
				}
				else
				{
					guys.spr(i)->x = circle_x;
					guys.spr(i)->y = circle_y;
				}
				
				temp_x=circle_x;
				temp_y=circle_y;
			}
			
			double ddir=atan2(double(temp_y),double(temp_x));
			
			if((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
			{
				guys.spr(i)->dir=l_down;
			}
			else if((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
			{
				guys.spr(i)->dir=left;
			}
			else if((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
			{
				guys.spr(i)->dir=l_up;
			}
			else if((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
			{
				guys.spr(i)->dir=up;
			}
			else if((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
			{
				guys.spr(i)->dir=r_up;
			}
			else if((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
			{
				guys.spr(i)->dir=right;
			}
			else if((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
			{
				guys.spr(i)->dir=r_down;
			}
			else
			{
				guys.spr(i)->dir=down;
			}
			
			guys.spr(i)->x += x;
			guys.spr(i)->y += y;
		}
	}
	
	if((wpn>wEnemyWeapons || (wpn >= wScript1 && wpn <= wScript10)) && (dmisc5==1 || dmisc5== 3) && (!dmisc25 || (dmisc25 == 1 && !flycnt && !flycnt2) || (dmisc25 == 2 && (flycnt || flycnt2)) || (dmisc25 == 3 && flycnt2 && !flycnt)))
	{
		int timeneeded = 48;
		int patbreath = (zc_oldrand()%50+50);
		if ((patbreath % 4) == 0) ++patbreath;
		if (dmisc28 == patratBREATH)
		{
			timeneeded = 48 + patbreath;
		}
		if (dmisc28 == patratSTREAM)
		{
			timeneeded = 48 + 96;
		}
		if (((((dmisc18 > 0 || ((editorflags & ENEMY_FLAG10) && !flycnt && !flycnt2)) && !(zc_oldrand() % zc_max(dmisc18, 1))) || //New 1/N chance
		(dmisc18 == 0 && !(zc_oldrand()&127)) //Old hardcoded firing chance
		|| (dmisc18 == -1 && loopcnt > 0 && (clk2 == round(halfsize) && (!(editorflags & ENEMY_FLAG3) || !get_bit(quest_rules,qr_NEWENEMYTILES))
		|| (clk4 == 10 && (editorflags & ENEMY_FLAG3) && get_bit(quest_rules,qr_NEWENEMYTILES)))))
		&& (clk6 >= 0) //if not in the middle of firing...
		&& clk6 >= dmisc19) //if over the set cooldown between shots...
		&& ((!(editorflags & ENEMY_FLAG7) || (loopcnt == 0 && (basesize*((int64_t)dmisc6 - (misc%dmisc6))) > timeneeded)) || dmisc18 == -1)) //And lastly, if not in danger of starting a loop during the attack.
		{
			switch(dmisc28)
			{
				case patratSTREAM:
				{
					clk7 = 97;
					if ((editorflags & ENEMY_FLAG3) && get_bit(quest_rules,qr_NEWENEMYTILES))  clk6 = -48;
					else clk6 = 0;
					break;
				}
				case patratBREATH:
				{
					clk7 = patbreath;
					if ((editorflags & ENEMY_FLAG3) && get_bit(quest_rules,qr_NEWENEMYTILES))  clk6 = -48;
					else clk6 = 0;
					break;
				}
				default:
				{
					if ((editorflags & ENEMY_FLAG3) && get_bit(quest_rules,qr_NEWENEMYTILES)) 
					{
						clk6 = -48;
						if (editorflags & ENEMY_FLAG6) clk4 = abs(clk6) + 16;
					}
					else
					{
						clk6 = 0;
						if (editorflags & ENEMY_FLAG6) clk4 = 16;
						FirePatraWeapon();
					}
					break;
				}
			} //ew->setAngle(atan2(double(LinkY()-y),double(LinkX()-x)));
		}
		if (clk6 < 0)
		{
			switch(dmisc28)
			{
				case patratSTREAM:
				{
					if (clk7 > 0 && (clk7 % 12) == 0) FirePatraWeapon();
					if (editorflags & ENEMY_FLAG6) clk4 = abs(clk6) + 16;
					break;
				}
				case patratBREATH:
				{
					if (clk7 > 0 && (clk7 % 4) == 0) FirePatraWeapon();
					if (editorflags & ENEMY_FLAG6) clk4 = abs(clk6) + 16;
					break;
				}
				default:
				{
					if ((editorflags & ENEMY_FLAG3) && get_bit(quest_rules,qr_NEWENEMYTILES) && clk6 == -16)
					{
						FirePatraWeapon();
						if (editorflags & ENEMY_FLAG6) clk4 = abs(clk6) + 16;
					}
					break;
				}
			}
		}
	}
	
	size=.5;
	int randattempts = 0;
	int randeye = 0;
	if (flycnt2 > 0)
	{
		do
		{
			randeye = ((flycnt2 > 0) ? (zc_oldrand() % zc_max(1, flycnt2)) : 0);
			randeye += (index + flycnt + 1);
			++randattempts;
		} while (((esPatra*)guys.spr(randeye))->clk5 < 0 && randattempts < 10);
	}
	bool dofire = false;
	if (dmisc20)
	{
		if ((dmisc18 > 0 && !(zc_oldrand() % zc_max(dmisc18, 1))) || 
		(dmisc18 == 0 && !(zc_oldrand()&127)) || 
		(dmisc18 == -1 && (loopcnt > 0 || dmisc20 == 4) && ((clk2 == round(halfsize) && (!(editorflags & ENEMY_FLAG3) || !get_bit(quest_rules,qr_NEWENEMYTILES)) && dmisc20 != 2 && dmisc20 != 4)
		|| (clk2 == 10 && dmisc20 != 4 && ((editorflags & ENEMY_FLAG3) && get_bit(quest_rules,qr_NEWENEMYTILES) || dmisc20 == 2))
		|| ((((((misc%dmisc6) == 0 && (loopcnt == 0 && !dmisc21)) || loopcnt > 1 || loopcnt == -1) && clk2 <= 53 && clk2 >= 51 && (editorflags & ENEMY_FLAG3)) || (!(editorflags & ENEMY_FLAG3) && loopcnt > 0 && clk2 == 1)) && dmisc20 == 4))))
		{
			if (clk5 >= 0 || !(editorflags & ENEMY_FLAG3) || !get_bit(quest_rules,qr_NEWENEMYTILES)) 
			{
				if (clk5 >= dmisc19)
				{
					if ((!(editorflags & ENEMY_FLAG7) || (loopcnt == 0 &&
					(dmisc20 == 2 && (basesize*((int64_t)dmisc6 - (misc%dmisc6))) > ((int64_t)48 + (int64_t(12)*flycnt2))) ||
					(dmisc20 == 4 && (basesize*((int64_t)dmisc6 - (misc%dmisc6))) > ((int64_t)48 + 96)) ||
					(dmisc20 != 2 && dmisc20 != 4 && (basesize*((int64_t)dmisc6 - (misc%dmisc6))) > 48)))
					|| dmisc18 == -1)  
						dofire = true;
				}
			}
		}
	}
	if(flycnt2)
	{
		for(int32_t i=index+flycnt+1; i<index+flycnt+flycnt2+1; i++)//inner ring
		{
			if(!adjusted)
			{
				((enemy*)guys.spr(i))->hp=12*game->get_hero_dmgmult();
				
				if(get_bit(quest_rules,qr_NEWENEMYTILES))
				{
					if (get_bit(quest_rules,qr_PATRAS_USE_HARDCODED_OFFSETS))
					{
						switch(dmisc5)
						{
							// Center eye shoots projectiles; make room for its firing tiles
						case 1:
						case 3:
							((enemy*)guys.spr(i))->o_tile=d->e_tile+120;
							break;
							
							// Center eyes does not shoot; use tiles two rows below for inner eyes.
						default:
						case 2:
							((enemy*)guys.spr(i))->o_tile=d->e_tile+40;
							break;
						}
					}
					else ((enemy*)guys.spr(i))->o_tile = d->s_tile;
				}
				else
				{
					((enemy*)guys.spr(i))->o_tile=o_tile+1;
				}
				
				((enemy*)guys.spr(i))->cs=dmisc9;
				if (dmisc27) ((enemy*)guys.spr(i))->hp=dmisc27;
			}
			
			if(flycnt>0)
			{
				((enemy*)guys.spr(i))->superman=true;
			}
			else
			{
				((enemy*)guys.spr(i))->superman=false;
			}
			
			if(((enemy*)guys.spr(i))->hp <= 0)
			{
				for(int32_t j=i; j<index+flycnt+flycnt2; j++)
				{
					guys.swap(j,j+1);
				}
				
				if (--flycnt2 == 0 && dmisc24 != 0) step += zslongToFix(dmisc24*100);
			}
			else
			{
				int32_t pos2 = ((enemy*)guys.spr(i))->misc;
				double a2 = ((clk2-pos2*basesize/(dmisc2==0 ? 1 : dmisc2))*PI/(halfsize));
				
				if(dmisc4==0)
				{
					if(loopcnt>0)
					{
						guys.spr(i)->x =  cos(a2+PI/2)*abs(dmisc32) - sin(pos2*PI*2/(dmisc2==0?1:dmisc2))*((int64_t)abs(dmisc32)-abs(dmisc30));
						guys.spr(i)->y = -sin(a2+PI/2)*abs(dmisc32) + cos(pos2*PI*2/(dmisc2==0?1:dmisc2))*((int64_t)abs(dmisc32)-abs(dmisc30));
					}
					else
					{
						guys.spr(i)->x =  cos(a2+PI/2)*abs(dmisc30);
						guys.spr(i)->y = -sin(a2+PI/2)*abs(dmisc30);
					}
					
					temp_x=guys.spr(i)->x;
					temp_y=guys.spr(i)->y;
				}
				else
				{
					circle_x =  cos(a2+PI/2)*abs(dmisc30);
					circle_y = -sin(a2+PI/2)*abs(dmisc30);
					
					if(loopcnt>0)
					{
						guys.spr(i)->x =  cos(a2+PI/2)*abs(dmisc30);
						guys.spr(i)->y = (-sin(a2+PI/2)-cos(pos2*PI*2/(dmisc2 == 0 ? 1 : dmisc2)))*abs(dmisc32);
					}
					else
					{
						guys.spr(i)->x = circle_x;
						guys.spr(i)->y = circle_y;
					}
					
					temp_x=circle_x;
					temp_y=circle_y;
				}
				
				double ddir=atan2(double(temp_y),double(temp_x));
				
				if((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
				{
					guys.spr(i)->dir=l_down;
				}
				else if((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
				{
					guys.spr(i)->dir=left;
				}
				else if((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
				{
					guys.spr(i)->dir=l_up;
				}
				else if((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
				{
					guys.spr(i)->dir=up;
				}
				else if((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
				{
					guys.spr(i)->dir=r_up;
				}
				else if((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
				{
					guys.spr(i)->dir=right;
				}
				else if((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
				{
					guys.spr(i)->dir=r_down;
				}
				else
				{
					guys.spr(i)->dir=down;
				}
				
				guys.spr(i)->x += x;
				guys.spr(i)->y = y-guys.spr(i)->y;
				
				if((wpn>wEnemyWeapons || (wpn >= wScript1 && wpn <= wScript10)) && (dmisc5==2 || dmisc5== 3))
				{
					/*
					if(!(zc_oldrand()&127))
					{
						addEwpn(guys.spr(i)->x,guys.spr(i)->y,guys.spr(i)->z,wpn,3,wdp,dir,getUID());
						sfx(wpnsfx(wpn),pan(int32_t(x)));
					}
					*/
					if (((esPatra*)guys.spr(i))->clk5 < 0 && (editorflags & ENEMY_FLAG3))
					{
						if (((esPatra*)guys.spr(i))->clk4 <= 0 || ((esPatra*)guys.spr(i))->clk5 != -16) ++((esPatra*)guys.spr(i))->clk5;
						if (get_bit(quest_rules,qr_PATRAS_USE_HARDCODED_OFFSETS))
						{
							if (dmisc5 == 3)
							{
								if (((esPatra*)guys.spr(i))->clk5 >= 0) ((esPatra*)guys.spr(i))->o_tile=d->e_tile+120;
								else if (((esPatra*)guys.spr(i))->clk5 >= -16) ((esPatra*)guys.spr(i))->o_tile=d->e_tile+200;
								else if (((esPatra*)guys.spr(i))->clk5 >= -48) ((esPatra*)guys.spr(i))->o_tile=d->e_tile+160;
								else ((esPatra*)guys.spr(i))->o_tile=d->e_tile+120;
							}
							else
							{
								if (((esPatra*)guys.spr(i))->clk5 >= 0) ((esPatra*)guys.spr(i))->o_tile=d->e_tile+40;
								else if (((esPatra*)guys.spr(i))->clk5 >= -16) ((esPatra*)guys.spr(i))->o_tile=d->e_tile+120;
								else if (((esPatra*)guys.spr(i))->clk5 >= -48) ((esPatra*)guys.spr(i))->o_tile=d->e_tile+80;
								else ((esPatra*)guys.spr(i))->o_tile=d->e_tile+40;
							}
						}
						else
						{
							if (((esPatra*)guys.spr(i))->clk5 >= 0) ((esPatra*)guys.spr(i))->o_tile=d->s_tile;
							else if (((esPatra*)guys.spr(i))->clk5 >= -16) ((esPatra*)guys.spr(i))->o_tile=d->s_tile+80;
							else if (((esPatra*)guys.spr(i))->clk5 >= -48) ((esPatra*)guys.spr(i))->o_tile=d->s_tile+40;
							else ((esPatra*)guys.spr(i))->o_tile=d->s_tile;
						}
					}
					else if ((dmisc19 || ((esPatra*)guys.spr(i))->clk5) && (((esPatra*)guys.spr(i))->clk4 <= 0 || ((esPatra*)guys.spr(i))->clk5 != -16)) ++((esPatra*)guys.spr(i))->clk5;
					if (((esPatra*)guys.spr(i))->clk4 > 0) --((esPatra*)guys.spr(i))->clk4;
					if (!dmisc25 || (dmisc25 == 1 && !((enemy*)guys.spr(i))->superman) || ((dmisc25 == 2 || dmisc25 == 3) && ((enemy*)guys.spr(i))->superman))
					{
						switch(dmisc20) //Patra Attack Patterns
						{
							case 4: //Single one rapidfires
							{
								if (dofire && i == randeye)
								{
									((esPatra*)guys.spr(i))->clk5 = -16;
									if ((editorflags & ENEMY_FLAG3) && get_bit(quest_rules,qr_NEWENEMYTILES)) ((esPatra*)guys.spr(i))->clk5 = -48;
									((esPatra*)guys.spr(i))->clk4 = 96;
									clk5 = -3;
									if (editorflags & ENEMY_FLAG6) clk4 = abs(clk5) + 16;
								}
								if (((esPatra*)guys.spr(i))->clk5 == -16 && (((esPatra*)guys.spr(i))->clk4 % 12) == 0)
								{
									addEwpn(guys.spr(i)->x,guys.spr(i)->y,guys.spr(i)->z,wpn,3,wdp,dir,getUID());
									sfx(wpnsfx(wpn),pan(int32_t(x)));
								}
								break;
							}
							case 3: //Ring
							{
								if (dofire)
								{
									if ((editorflags & ENEMY_FLAG3) && get_bit(quest_rules,qr_NEWENEMYTILES)) 
									{
										((esPatra*)guys.spr(i))->clk5 = -48;
										clk5 = -48;
										if (editorflags & ENEMY_FLAG6) clk4 = 64;
									}
									else
									{
										addEwpn(guys.spr(i)->x,guys.spr(i)->y,guys.spr(i)->z,wpn,3,wdp,dir,getUID());
										sfx(wpnsfx(wpn),pan(int32_t(x)));
										int32_t m=Ewpns.Count()-1;
										weapon *ew = (weapon*)(Ewpns.spr(m));
										
										ew->setAngle(atan2(double(LinkY()-y),double(LinkX()-x)));
										((esPatra*)guys.spr(i))->clk5 = 0;
										clk5 = 0;
										if (editorflags & ENEMY_FLAG6) clk4 = 16;
									}
								}
								if (((esPatra*)guys.spr(i))->clk5 == -16)
								{
									addEwpn(guys.spr(i)->x,guys.spr(i)->y,guys.spr(i)->z,wpn,3,wdp,dir,getUID());
									sfx(wpnsfx(wpn),pan(int32_t(x)));
									int32_t m=Ewpns.Count()-1;
									weapon *ew = (weapon*)(Ewpns.spr(m));
									
									ew->setAngle(atan2(double(LinkY()-y),double(LinkX()-x)));
								}
								break;
							}
							case 2: //one after another
							{
								if (dofire)
								{
									((esPatra*)guys.spr(i))->clk5 = -48 - (12*(i-(index+flycnt+1)));
									clk5 = -48 - (12*flycnt2);
									if (editorflags & ENEMY_FLAG6) clk4 = abs(clk5) + 16;
								}
								if (((esPatra*)guys.spr(i))->clk5 == -16)
								{
									addEwpn(guys.spr(i)->x,guys.spr(i)->y,guys.spr(i)->z,wpn,3,wdp,dir,getUID());
									sfx(wpnsfx(wpn),pan(int32_t(x)));
								}
								break;
							}
							case 1: //random one eye
							{
								if (dofire && i == randeye)
								{
									if ((editorflags & ENEMY_FLAG3) && get_bit(quest_rules,qr_NEWENEMYTILES)) 
									{
										((esPatra*)guys.spr(i))->clk5 = -48;
										clk5 = -48;
										if (editorflags & ENEMY_FLAG6) clk4 = 64;
									}
									else
									{
										addEwpn(guys.spr(i)->x,guys.spr(i)->y,guys.spr(i)->z,wpn,3,wdp,dir,getUID());
										sfx(wpnsfx(wpn),pan(int32_t(x)));
										((esPatra*)guys.spr(i))->clk5 = 0;
										clk5 = 0;
										if (editorflags & ENEMY_FLAG6) clk4 = 16;
									}
								}
								if ((editorflags & ENEMY_FLAG3) && get_bit(quest_rules,qr_NEWENEMYTILES) && ((esPatra*)guys.spr(i))->clk5 == -16)
								{
									addEwpn(guys.spr(i)->x,guys.spr(i)->y,guys.spr(i)->z,wpn,3,wdp,dir,getUID());
									sfx(wpnsfx(wpn),pan(int32_t(x)));
								}
								break;
							}
							default: //old behavior, all eyes can fire any time
							{
								if ((((dmisc18 && !(zc_oldrand() % zc_max(dmisc18, 1))) || 
								(!dmisc18 && !(zc_oldrand()&127))) && (((esPatra*)guys.spr(i))->clk5 >= 0 || !(editorflags & ENEMY_FLAG3) || !get_bit(quest_rules,qr_NEWENEMYTILES))
								&& ((esPatra*)guys.spr(i))->clk5 >= dmisc19) && (!(editorflags & ENEMY_FLAG7) || (loopcnt == 0 &&
								(dmisc20 != 2 && (basesize*((int64_t)dmisc6 - (misc%dmisc6))) > 48))))
								{
									if ((editorflags & ENEMY_FLAG3) && get_bit(quest_rules,qr_NEWENEMYTILES)) 
									{
										((esPatra*)guys.spr(i))->clk5 = -48;
										if (editorflags & ENEMY_FLAG6) clk4 = 64;
									}
									else
									{
										addEwpn(guys.spr(i)->x,guys.spr(i)->y,guys.spr(i)->z,wpn,3,wdp,dir,getUID());
										sfx(wpnsfx(wpn),pan(int32_t(x)));
										((esPatra*)guys.spr(i))->clk5 = 0;
										if (editorflags & ENEMY_FLAG6) clk4 = 16;
									}
								}
								if ((editorflags & ENEMY_FLAG3) && get_bit(quest_rules,qr_NEWENEMYTILES) && ((esPatra*)guys.spr(i))->clk5 == -16)
								{
									addEwpn(guys.spr(i)->x,guys.spr(i)->y,guys.spr(i)->z,wpn,3,wdp,dir,getUID());
									sfx(wpnsfx(wpn),pan(int32_t(x)));
								}
								break;
							}
						}
					}
				}
				
			}
		}
	}
	
	adjusted=true;
	return enemy::animate(index);
}

void ePatra::FirePatraWeapon()
{ //.707
	int32_t xoff = 0;
	int32_t yoff = 0;
	if ( SIZEflags&guyflagOVERRIDE_HIT_WIDTH )
	{
		xoff += (hxsz/2)-8;   
		//Z_scripterrlog("width flag enabled. xoff = %d\n", xoff);
	}
	if ( SIZEflags&guyflagOVERRIDE_HIT_HEIGHT )
	{
		yoff += (hysz/2)-8;   
		//Z_scripterrlog("width flag enabled. yoff = %d\n", yoff);
	}
	sfx(wpnsfx(wpn),pan(int32_t(x)));
	switch (dmisc28)
	{
		case patrat8SHOT: //Fire Wizzrobe
		case patrat4SHOTDIAG:
		case patrat4SHOTRAND:
			if (dmisc28 != patrat4SHOTRAND || (zc_oldrand()%2)) //if it's the 4 shot rand type, only let it through half the time. Break is within so it doesn't do both, but if it skips this one it'll always do the other one.
			{
				Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,1,wdp,l_up,-1, getUID(),false));
				((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
				if (wpn != ewFlame && wpn != ewFlame2)  ((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->step *= .707; //Fire already does this internall for asome bizarre reason.
				
				Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,1,wdp,l_down,-1, getUID(),false));
				((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
				if (wpn != ewFlame && wpn != ewFlame2)  ((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->step *= .707; //Fire already does this internall for asome bizarre reason.
				
				Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,1,wdp,r_up,-1, getUID(),false));
				((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
				if (wpn != ewFlame && wpn != ewFlame2)  ((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->step *= .707; //Fire already does this internall for asome bizarre reason.
				
				Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,1,wdp,r_down,-1, getUID(),false));
				((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
				if (wpn != ewFlame && wpn != ewFlame2)  ((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->step *= .707; //Fire already does this internall for asome bizarre reason.
				
				if (dmisc28 == patrat4SHOTDIAG || dmisc28 == patrat4SHOTRAND) break;
			}	
			
			[[fallthrough]];
		case patrat4SHOTCARD: //Stalfos 3
			Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,1,wdp,up,-1, getUID(),false));
			((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
			Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,1,wdp,down,-1, getUID(),false));
			((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
			Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,1,wdp,left,-1, getUID(),false));
			((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
			Ewpns.add(new weapon(x+xoff,y+yoff,z,wpn,1,wdp,right,-1, getUID(),false));
			((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->moveflags &= ~FLAG_CAN_PITFALL; //No falling in pits
			break;
		
		default:
			addEwpn(x,y,z,wpn,3,wdp,dir,getUID());
			if (dmisc28 == patratBREATH) ((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->angle += (zc_rand(20,-20)/100.0)*PI;
			double anglestore = ((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->angle;
			if (dmisc28 == patrat1SHOTFAST || dmisc28 == patrat3SHOTFAST || dmisc28 == patrat5SHOTFAST) ((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->step *= 2;
			if (dmisc28 == patrat3SHOT || dmisc28 == patrat3SHOTFAST || dmisc28 == patrat5SHOT || dmisc28 == patrat5SHOTFAST)
			{
				addEwpn(x,y,z,wpn,3,wdp,dir,getUID());
				((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->angle = anglestore + (double)0.46364761;
				((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->step += 0.1180;
				if (dmisc28 == patrat3SHOTFAST || dmisc28 == patrat5SHOTFAST) ((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->step *= 2;
				addEwpn(x,y,z,wpn,3,wdp,dir,getUID());
				((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->angle = anglestore - (double)0.46364761;
				((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->step += 0.1180;
				if (dmisc28 == patrat3SHOTFAST || dmisc28 == patrat5SHOTFAST) ((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->step *= 2;
				if (dmisc28 == patrat5SHOT || dmisc28 == patrat5SHOTFAST)
				{
					addEwpn(x,y,z,wpn,3,wdp,dir,getUID());
					((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->angle = anglestore + (double)0.78539816;
					((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->step += 0.4142;
					if (dmisc28 == patrat5SHOTFAST) ((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->step *= 2;
					addEwpn(x,y,z,wpn,3,wdp,dir,getUID());
					((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->angle = anglestore - (double)0.78539816;
					((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->step += 0.4142;
					if (dmisc28 == patrat5SHOTFAST) ((weapon*)(Ewpns.spr(Ewpns.Count()-1)))->step *= 2;
				}
			}
			break;
			
	}
	sfx(wpnsfx(wpn),pan(int32_t(x)));
	//+0.46364761
	//11.80
}

void ePatra::draw(BITMAP *dest)
{
	tile=o_tile;
	update_enemy_frame();
	enemy::draw(dest);
}

int32_t ePatra::defend(int32_t wpnId, int32_t *power, int32_t edef)
{
	int32_t ret = enemy::defend(wpnId, power, edef);
	
	if(ret < 0 && (flycnt||flycnt2))
		return 0;
		
	return ret;
}

int32_t ePatra::defendNew(int32_t wpnId, int32_t *power, int32_t edef, byte unblockable)
{
	int32_t ret = enemy::defendNew(wpnId, power, edef, unblockable);
	
	if(ret < 0 && (flycnt||flycnt2))
		return 0;
		
	return ret;
}

esPatra::esPatra(zfix X,zfix Y,int32_t Id,int32_t Clk, sprite * prnt) : enemy(X,Y,Id,Clk), parent(prnt)
{
	//cs=8;
	item_set=0;
	misc=clk;
	clk4 = 0;
	clk5 = 0;
	clk = -((misc*21)>>1)-1;
	yofs=playing_field_offset;
	hxsz=12;
	hysz=12;
	hxofs=2;
	hyofs=2;
	extend = 0;
	txsz = 1;
	tysz = 1;
	/* //These need to be separate enemy editor fields. This enemy class also it's draw altered to correctly support big stuff.
		enemy *prntenemy = (enemy *) guys.getByUID(parent->getUID());
		int32_t prntSIZEflags = prntenemy->SIZEflags;
		if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = prntenemy->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
		//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
	   // al_trace("Enemy txsz:%i\n", txsz);
		if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = prntenemy->tysz; if ( tysz > 1 ) extend = 3; }
		if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = prntenemy->hxsz;
		else
			hxsz=12;
		if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = prntenemy->hysz;
		else 
			hysz=12;
		if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = prntenemy->hzsz;
		if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = prntenemy->hxofs;
		else 
			hxofs=2;
		if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = prntenemy->hyofs;
		else hyofs=2;
	//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
		if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)prntenemy->xofs;
		if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
		{
			yofs = (int32_t)prntenemy->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		}
	  
		if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)prntenemy->zofs;
	*/
	mainguy=count_enemy=false;
	bgsfx=-1;
	//o_tile=0;
	flags &= (~guy_neverret);
	deadsfx = WAV_EDEAD;
	hitsfx = WAV_EHIT;
	isCore = false;
}

bool esPatra::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(dying)
		return Dead(index);
		
	return enemy::animate(index);
}

void esPatra::draw(BITMAP *dest)
{
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
		tile = o_tile+(clk&3);
		
		switch(dir)                                             //directions get screwed up after 8.  *shrug*
		{
		case up:                                              //u
			flip=0;
			break;
			
		case down:                                            //d
			flip=0;
			tile+=4;
			break;
			
		case left:                                            //l
			flip=0;
			tile+=8;
			break;
			
		case right:                                           //r
			flip=0;
			tile+=12;
			break;
			
		case l_up:                                            //ul
			flip=0;
			tile+=20;
			break;
			
		case r_up:                                            //ur
			flip=0;
			tile+=24;
			break;
			
		case l_down:                                          //dl
			flip=0;
			tile+=28;
			break;
			
		case r_down:                                          //dr
			flip=0;
			tile+=32;
			break;
		}
	}
	else
	{
		tile = o_tile+((clk&2)>>1);
	}
	
	if(clk>=0)
		enemy::draw(dest);
}


ePatraBS::ePatraBS(zfix ,zfix ,int32_t Id,int32_t Clk) : enemy((zfix)128,(zfix)48,Id,Clk)
{
	adjusted=false;
	dir=(zc_oldrand()&7)+8;
	step=0.25;
	clk4 = 0;
	clk5 = 0;
	//flycnt=6; flycnt2=0;
	flycnt=dmisc1;
	flycnt2=0; // PatraBS doesn't have inner rings!
		loopcnt=0;
		
		SIZEflags = d->SIZEflags;
		if ( ((SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = d->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
		//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = d->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = d->hxsz;
	else hxsz = 32;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = d->hysz;
	if ( ((SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = d->hzsz;
	if ( (SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = d->hxofs;
	else hxofs=-8;
	if (  (SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = d->hyofs;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)d->xofs;
	if ( (SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += playing_field_offset ; //this offset fixes yofs not plaing properly. -Z
	}
  
	if (  (SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) zofs = (int32_t)d->zofs;
	
	if(dmisc6<int16_t(1))dmisc6=1; // ratio cannot be 0!
	
	//nets+4480;
}

bool ePatraBS::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(dying)
		return Dead(index);
		
	if(clk==0)
	{
		removearmos(x,y);
	}
	
	variable_walk_8(rate,homing,hrate,spw_floater);
	
	if(++clk2>90)
	{
		clk2=0;
		
		if(loopcnt)
			--loopcnt;
		else
		{
			if((misc%dmisc6)==0)
				loopcnt=dmisc7;
		}
		
		++misc;
	}
	
	//    double size=1;;
	for(int32_t i=index+1; i<index+flycnt+1; i++)
	{
		if(!adjusted)
		{
			((enemy*)guys.spr(i))->hp=dmisc3;
			
			if(get_bit(quest_rules,qr_NEWENEMYTILES))
			{
				((enemy*)guys.spr(i))->o_tile=o_tile+dmisc8;
			}
			else
			{
				((enemy*)guys.spr(i))->o_tile=o_tile+1;
			}
			
			((enemy*)guys.spr(i))->cs = dmisc9;
		}
		
		if(((enemy*)guys.spr(i))->hp <= 0)
		{
			for(int32_t j=i; j<index+flycnt+flycnt2; j++)
			{
				guys.swap(j,j+1);
			}
			
			--flycnt;
		}
		else
		{
			int32_t pos2 = ((enemy*)guys.spr(i))->misc;
			double a2 = ((int64_t)clk2-pos2*90/(dmisc1==0?1:dmisc1))*PI/45;
			temp_x =  cos(a2+PI/2)*45;
			temp_y = -sin(a2+PI/2)*45;
			
			if(loopcnt>0)
			{
				guys.spr(i)->x =  cos(a2+PI/2)*45;
				guys.spr(i)->y = (-sin(a2+PI/2)-cos(pos2*PI*2/(dmisc1==0?1:dmisc1)))*22.5;
			}
			else
			{
				guys.spr(i)->x = temp_x;
				guys.spr(i)->y = temp_y;
			}
			
			double ddir=atan2(double(temp_y),double(temp_x));
			
			if((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
			{
				guys.spr(i)->dir=l_down;
			}
			else if((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
			{
				guys.spr(i)->dir=left;
			}
			else if((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
			{
				guys.spr(i)->dir=l_up;
			}
			else if((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
			{
				guys.spr(i)->dir=up;
			}
			else if((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
			{
				guys.spr(i)->dir=r_up;
			}
			else if((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
			{
				guys.spr(i)->dir=right;
			}
			else if((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
			{
				guys.spr(i)->dir=r_down;
			}
			else
			{
				guys.spr(i)->dir=down;
			}
			
			guys.spr(i)->x += x;
			guys.spr(i)->y += y;
		}
	}
	
	adjusted=true;
	return enemy::animate(index);
}

void ePatraBS::draw(BITMAP *dest)
{
	tile=o_tile;
	
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
		double ddir=atan2(double(y-(Link.y)),double(Link.x-x));
		
		if((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
		{
			lookat=l_down;
		}
		else if((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
		{
			lookat=down;
		}
		else if((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
		{
			lookat=r_down;
		}
		else if((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
		{
			lookat=right;
		}
		else if((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
		{
			lookat=r_up;
		}
		else if((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
		{
			lookat=up;
		}
		else if((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
		{
			lookat=l_up;
		}
		else
		{
			lookat=left;
		}
		
		switch(lookat)                                          //directions get screwed up after 8.  *shrug*
		{
		case up:                                              //u
			flip=0;
			break;
			
		case down:                                            //d
			flip=0;
			tile+=8;
			break;
			
		case left:                                            //l
			flip=0;
			tile+=40;
			break;
			
		case right:                                           //r
			flip=0;
			tile+=48;
			break;
			
		case l_up:                                            //ul
			flip=0;
			tile+=80;
			break;
			
		case r_up:                                            //ur
			flip=0;
			tile+=88;
			break;
			
		case l_down:                                          //dl
			flip=0;
			tile+=120;
			break;
			
		case r_down:                                          //dr
			flip=0;
			tile+=128;
			break;
		}
		
		tile+=(2*(clk&3));
		xofs-=8;
		yofs-=8;
		drawblock(dest,15);
		xofs+=8;
		yofs+=8;
	}
	else
	{
		flip=(clk&1);
		xofs-=8;
		yofs-=8;
		enemy::draw(dest);
		xofs+=16;
		enemy::draw(dest);
		yofs+=16;
		enemy::draw(dest);
		xofs-=16;
		enemy::draw(dest);
		xofs+=8;
		yofs-=8;
	}
}

int32_t ePatraBS::defend(int32_t wpnId, int32_t *power, int32_t edef)
{
	int32_t ret = enemy::defend(wpnId, power, edef);
	
	if(ret < 0 && (flycnt||flycnt2))
		return 0;
		
	return ret;
}

int32_t ePatraBS::defendNew(int32_t wpnId, int32_t *power, int32_t edef, byte unblockable)
{
	int32_t ret = enemy::defendNew(wpnId, power, edef, unblockable);
	
	if(ret < 0 && (flycnt||flycnt2))
		return 0;
		
	return ret;
}

esPatraBS::esPatraBS(zfix X,zfix Y,int32_t Id,int32_t Clk, sprite * prnt) : enemy(X,Y,Id,Clk), parent(prnt)
{
	//cs=csBOSS;
	item_set=0;
	misc=clk;
	clk = -((misc*21)>>1)-1;
	clk4 = 0;
	clk5 = 0;
	enemy *prntenemy = (enemy *) guys.getByUID(parent->getUID());
	int32_t prntSIZEflags = prntenemy->SIZEflags;
	if ( ((prntSIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) && txsz > 0 ) { txsz = prntenemy->txsz; if ( txsz > 1 ) extend = 3; } //! Don;t forget to set extend if the tilesize is > 1. 
	//al_trace("->txsz:%i\n", txsz); Verified that this is setting the value. -Z
   // al_trace("Enemy txsz:%i\n", txsz);
	if ( ((prntSIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) && tysz > 0 ) { tysz = prntenemy->tysz; if ( tysz > 1 ) extend = 3; }
	if ( ((prntSIZEflags&guyflagOVERRIDE_HIT_WIDTH) != 0) && hxsz >= 0 ) hxsz = prntenemy->hxsz;
	else hxsz=16;
	if ( ((prntSIZEflags&guyflagOVERRIDE_HIT_HEIGHT) != 0) && hysz >= 0 ) hysz = prntenemy->hysz;
	else hysz=16;
	if ( ((prntSIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) != 0) && hzsz >= 0  ) hzsz = prntenemy->hzsz;
	if ( (prntSIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) != 0 ) hxofs = prntenemy->hxofs;
	if (  (prntSIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) != 0 ) hyofs = prntenemy->hyofs;
	else hyofs=2;
//    if ( (SIZEflags&guyflagOVERRIDEHITZOFFSET) != 0 ) hzofs = hzofs;
	if (  (prntSIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) != 0 ) xofs = (int32_t)prntenemy->xofs;
	if ( (prntSIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) != 0 ) 
	{
		yofs = (int32_t)prntenemy->yofs; 
	}
	else yofs=playing_field_offset;
	if (  (prntSIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) != 0 ) prntenemy->zofs = (int32_t)zofs;
	
	bgsfx=-1;
	mainguy=count_enemy=false;
	deadsfx = WAV_EDEAD;
	hitsfx = WAV_EHIT;
	flags &= ~guy_neverret;
	isCore = false;
}

bool esPatraBS::animate(int32_t index)
{
	if(switch_hooked) return false;
	if(dying)
		return Dead(index);
	
	return enemy::animate(index);
}

void esPatraBS::draw(BITMAP *dest)
{
	tile=o_tile;
	
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
		switch(dir)                                             //directions get screwed up after 8.  *shrug*
		{
		case up:                                              //u
			flip=0;
			break;
			
		case down:                                            //d
			flip=0;
			tile+=4;
			break;
			
		case left:                                            //l
			flip=0;
			tile+=8;
			break;
			
		case right:                                           //r
			flip=0;
			tile+=12;
			break;
			
		case l_up:                                            //ul
			flip=0;
			tile+=20;
			break;
			
		case r_up:                                            //ur
			flip=0;
			tile+=24;
			break;
			
		case l_down:                                          //dl
			flip=0;
			tile+=28;
			break;
			
		case r_down:                                          //dr
			flip=0;
			tile+=32;
			break;
		}
		
		tile += ((clk&6)>>1);
	}
	else
	{
		tile += (clk&4)?1:0;
	}
	
	if(clk>=0)
		enemy::draw(dest);
}


/**********************************/
/**********  Misc Code  ***********/
/**********************************/

void addEwpn(int32_t x,int32_t y,int32_t z,int32_t id,int32_t type,int32_t power,int32_t dir, int32_t parentid, byte script_generated)
{
	if(id>wEnemyWeapons || (id >= wScript1 && id <= wScript10))
		Ewpns.add(new weapon((zfix)x,(zfix)y,(zfix)z,id,type,power,dir, -1, parentid, script_generated));
}

int32_t hit_enemy(int32_t index, int32_t wpnId,int32_t power,int32_t wpnx,int32_t wpny,int32_t dir, int32_t enemyHitWeapon)
{
	// Kludge
	weapon *w = new weapon((zfix)wpnx,(zfix)wpny,(zfix)0,wpnId,0,power,dir,enemyHitWeapon,-1,false);
	int32_t ret= ((enemy*)guys.spr(index))->takehit(w);
	delete w;
	return ret;
}

void enemy_scored(int32_t index)
{
	((enemy*)guys.spr(index))->scored=true;
}

void addguy(int32_t x,int32_t y,int32_t id,int32_t clk,bool mainguy)
{
	guy *g = new guy((zfix)x,(zfix)(y+(isdungeon()?1:0)),id,get_bit(quest_rules,qr_NOGUYPOOF)?0:clk,mainguy);
	guys.add(g);
}

void additem(int32_t x,int32_t y,int32_t id,int32_t pickup)
{
	item *i = new item(zfix(x), zfix(y - get_bit(quest_rules, qr_NOITEMOFFSET)), zfix(0), id, pickup, 0);
	items.add(i);
}

void additem(int32_t x,int32_t y,int32_t id,int32_t pickup,int32_t clk)
{
	item *i = new item((zfix)x,(zfix)y-(get_bit(quest_rules, qr_NOITEMOFFSET)),(zfix)0,id,pickup,clk);
	items.add(i);
}

void adddummyitem(int32_t x,int32_t y,int32_t id,int32_t pickup)
{
	item *i = new item((zfix)x,(zfix)y-(get_bit(quest_rules, qr_NOITEMOFFSET)),(zfix)0,id,pickup,0,true);
	items.add(i);
}

void kill_em_all()
{
	for(int32_t i=0; i<guys.Count(); i++)
	{
		enemy *e = ((enemy*)guys.spr(i));
		
		if(e->flags&(1<<3) && !(e->family == eeGHINI && e->dmisc1 == 1)) continue;
		
		e->kickbucket();
	}
}

//This needs a quest rule, or enemy flag, Dying Enemy Doesn't  Hurt Link
// For Link's hit detection. Don't count them if they are stunned or are guys.
int32_t GuyHit(int32_t tx,int32_t ty,int32_t tz,int32_t txsz,int32_t tysz,int32_t tzsz)
{
	for(int32_t i=0; i<guys.Count(); i++)
	{
		if(guys.spr(i)->hit(tx,ty,tz,txsz,tysz,tzsz))
		{
			if(((enemy*)guys.spr(i))->stunclk==0 &&  ((enemy*)guys.spr(i))->frozenclock==0 && (!get_bit(quest_rules, qr_SAFEENEMYFADE) || ((enemy*)guys.spr(i))->fading != fade_flicker)
					&&(((enemy*)guys.spr(i))->d->family != eeGUY || ((enemy*)guys.spr(i))->dmisc1))
			{
				return i;
			}
		}
	}
   
	return -1;
}

// For Link's hit detection. Count them if they are dying.
int32_t GuyHit(int32_t index,int32_t tx,int32_t ty,int32_t tz,int32_t txsz,int32_t tysz,int32_t tzsz)
{
	enemy *e = (enemy*)guys.spr(index);
	
	if(e->hp > 0)
		return -1;
		
	bool d = e->dying;
	int32_t hc = e->hclk;
	e->dying = false;
	e->hclk = 0;
	bool hit = e->hit(tx,ty,tz,txsz,tysz,tzsz);
	e->dying = d;
	e->hclk = hc;
	
	return hit ? index : -1;
}

bool hasMainGuy()
{
	for(int32_t i=0; i<guys.Count(); i++)
	{
		if(((enemy*)guys.spr(i))->mainguy)
		{
			return true;
		}
	}
	
	return false;
}

void EatLink(int32_t index)
{
	((eStalfos*)guys.spr(index))->eatlink();
}

void GrabLink(int32_t index)
{
	((eWallM*)guys.spr(index))->grablink();
}

bool CarryLink()
{
	for(int32_t i=0; i<guys.Count(); i++)
	{
		if(((guy*)(guys.spr(i)))->family==eeWALLM)
		{
			if(((eWallM*)guys.spr(i))->haslink)
			{
				Link.x=guys.spr(i)->x;
				Link.y=guys.spr(i)->y;
				return ((eWallM*)guys.spr(i))->misc > 0;
			}
		}
		
		// Like Likes currently can't carry Link.
		/*
		if(((guy*)(guys.spr(i)))->family==eeLIKE)
		{
		  if(((eLikeLike*)guys.spr(i))->haslink)
		  {
			Link.x=guys.spr(i)->x;
			Link.y=guys.spr(i)->y;
			return (true);
		  }
		}*/
	}
	
	return false;
}

// Move item with guy
void movefairy(zfix &x,zfix &y,int32_t misc)
{
	int32_t i = guys.idFirst(eITEMFAIRY+0x1000*misc);
	
	if(i!=-1)
	{
		x = guys.spr(i)->x;
		y = guys.spr(i)->y;
	}
}

// Move guy with item (used by FFC scripts and hookshot-dragged fairies)
void movefairy2(zfix x,zfix y,int32_t misc)
{
	int32_t i = guys.idFirst(eITEMFAIRY+0x1000*misc);
	
	if(i!=-1)
	{
		guys.spr(i)->x = x;
		guys.spr(i)->y = y;
	}
}// Move item with guy

void movefairynew(zfix &x,zfix &y, item const &itemfairy)
{
	enemy *fairy = (enemy *) guys.getByUID(itemfairy.fairyUID);
	
	if(fairy)
	{
		x = fairy->x;
		y = fairy->y;
	}
}

// Move guy with item (used by FFC scripts and hookshot-dragged fairies)
void movefairynew2(zfix x,zfix y, item const &itemfairy)
{
	enemy *fairy = (enemy *) guys.getByUID(itemfairy.fairyUID);
	
	if(fairy)
	{
		fairy->x = x;
		fairy->y = y;
	}
}

void killfairy(int32_t misc)
{
	int32_t i = guys.idFirst(eITEMFAIRY+0x1000*misc);
	guys.del(i);
}

void killfairynew(item const &itemfairy)
{
	enemy *fairy = (enemy *) guys.getByUID(itemfairy.fairyUID);
	guys.del(fairy->id);
}

int32_t addenemy(int32_t x,int32_t y,int32_t id,int32_t clk)
{
	return addenemy(x,y,0,id,clk);
}

int32_t addchild(int32_t x,int32_t y,int32_t id,int32_t clk, int32_t parent_scriptUID)
{
	return addchild(x,y,0,id,clk, parent_scriptUID);
}

int32_t addchild(int32_t x,int32_t y,int32_t z,int32_t id,int32_t clk, int32_t parent_scriptUID)
{
	if(id <= 0) return 0;
	
	int32_t ret = 0;
	sprite *e=NULL;
	al_trace("Adding child\n");
	
	switch(guysbuf[id&0xFFF].family)
	{
		//Fixme: possible enemy memory leak. (minor)
	case eeWALK:
		e = new eStalfos((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeLEV:
		e = new eLeever((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeTEK:
		e = new eTektite((zfix)x,(zfix)y,id,clk);
		break;
		
	case eePEAHAT:
		e = new ePeahat((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeZORA:
		e = new eZora((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeGHINI:
		e = new eGhini((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeKEESE:
		e = new eKeese((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeWIZZ:
		e = new eWizzrobe((zfix)x,(zfix)y,id,clk);
		break;
		
	case eePROJECTILE:
		e = new eProjectile((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeWALLM:
		e = new eWallM((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeAQUA:
		e = new eAquamentus((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeMOLD:
		e = new eMoldorm((zfix)x,(zfix)y,id,zc_max(1,zc_min(254,guysbuf[id&0xFFF].misc1)));
		break;
		
	case eeMANHAN:
		e = new eManhandla((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeGLEEOK:
		e = new eGleeok((zfix)x,(zfix)y,id,zc_max(1,zc_min(254,guysbuf[id&0xFFF].misc1)));
		break;
		
	case eeGHOMA:
		e = new eGohma((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeLANM:
		e = new eLanmola((zfix)x,(zfix)y,id,zc_max(1,zc_min(253,guysbuf[id&0xFFF].misc1)));
		break;
		
	case eeGANON:
		e = new eGanon((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeFAIRY:
		e = new eItemFairy((zfix)x,(zfix)y,id+0x1000*clk,clk);
		break;
		
	case eeFIRE:
		e = new eFire((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeOTHER: 
		e = new eOther((zfix)x,(zfix)y,id,clk);
		break;
	
	
		case eeSCRIPT01: 
	case eeSCRIPT02: 
	case eeSCRIPT03: 
	case eeSCRIPT04: 
	case eeSCRIPT05: 
	case eeSCRIPT06: 
	case eeSCRIPT07: 
	case eeSCRIPT08: 
	case eeSCRIPT09: 
	case eeSCRIPT10: 
	case eeSCRIPT11: 
	case eeSCRIPT12: 
	case eeSCRIPT13: 
	case eeSCRIPT14: 
	case eeSCRIPT15: 
	case eeSCRIPT16: 
	case eeSCRIPT17: 
	case eeSCRIPT18: 
	case eeSCRIPT19: 
	case eeSCRIPT20: 
	{
		if ( !get_bit(quest_rules, qr_SCRIPT_FRIENDLY_ENEMY_TYPES) )
		{
			e = new eScript((zfix)x,(zfix)y,id,clk);
			break;
		}
		else return 0;
	}
				
	case eeFFRIENDLY01:
	case eeFFRIENDLY02:
	case eeFFRIENDLY03:
	case eeFFRIENDLY04:
	case eeFFRIENDLY05:
	case eeFFRIENDLY06:
	case eeFFRIENDLY07:
	case eeFFRIENDLY08:
	case eeFFRIENDLY09:
	case eeFFRIENDLY10:
	{
		if ( !get_bit(quest_rules, qr_SCRIPT_FRIENDLY_ENEMY_TYPES) )
		{
			e = new eFriendly((zfix)x,(zfix)y,id,clk); break;
		}
		else return 0;
				
	}	
		
	case eeSPINTILE:
		e = new eSpinTile((zfix)x,(zfix)y,id,clk);
		break;
		
		// and these enemies use the misc10/misc2 value
	case eeROCK:
	{
		switch(guysbuf[id&0xFFF].misc10)
		{
		case 1:
			e = new eBoulder((zfix)x,(zfix)y,id,clk);
			break;
			
		case 0:
		default:
			e = new eRock((zfix)x,(zfix)y,id,clk);
			break;
		}
		
		break;
	}
	
	case eeTRAP:
	{
		switch(guysbuf[id&0xFFF].misc2)
		{
		case 1:
			e = new eTrap2((zfix)x,(zfix)y,id,clk);
			break;
			
		case 0:
		default:
			e = new eTrap((zfix)x,(zfix)y,id,clk);
			break;
		}
		
		break;
	}
	
	case eeDONGO:
	{
		switch(guysbuf[id&0xFFF].misc10)
		{
		case 1:
			e = new eDodongo2((zfix)x,(zfix)y,id,clk);
			break;
			
		case 0:
		default:
			e = new eDodongo((zfix)x,(zfix)y,id,clk);
			break;
		}
		
		break;
	}
	
	case eeDIG:
	{
		switch(guysbuf[id&0xFFF].misc10)
		{
		case 1:
			e = new eLilDig((zfix)x,(zfix)y,id,clk);
			break;
			
		case 0:
		default:
			e = new eBigDig((zfix)x,(zfix)y,id,clk);
			break;
		}
		
		break;
	}
	
	case eePATRA:
	{
		switch(guysbuf[id&0xFFF].misc10)
		{
		case 1:
			if (get_bit(quest_rules,qr_HARDCODED_BS_PATRA))
			{
				e = new ePatraBS((zfix)x,(zfix)y,id,clk);
				break;
			}
			[[fallthrough]];
		case 0:
		default:
			e = new ePatra((zfix)x,(zfix)y,id,clk);
			break;
		}
		
		break;
	}
	
	case eeGUY:
	{
		switch(guysbuf[id&0xFFF].misc10)
		{
		case 1:
			e = new eTrigger((zfix)x,(zfix)y,id,clk);
			break;
			
		case 0:
		default:
			e = new eNPC((zfix)x,(zfix)y,id,clk);
			break;
		}
		
		break;
	}
	
	case eeNONE:
		if(guysbuf[id&0xFFF].misc10 ==1)
		{
			e = new eTrigger((zfix)x,(zfix)y,id,clk);
			break;
			break;
		}
		[[fallthrough]];
	default:
	
		return 0;
	}
	
	ret++; // Made one enemy.
	
	if(z && canfall(id))
	{
		e->z = (zfix)z;
	}
	
	((enemy*)e)->ceiling = (z && canfall(id));
	((enemy*)e)->parent_script_UID = parent_scriptUID;
	//al_trace("Child Script UID: %d\n",((enemy*)e)->script_UID);
	//zprint2("Child Script UID: %d\n",((enemy*)e)->script_UID);
	//al_trace("Child's Parent UID: %d\n",((enemy*)e)->parent_script_UID);
	//zprint2("Child's Parent UID: %d\n",((enemy*)e)->parent_script_UID);
			
	
	if(!guys.add(e))
	{
		return 0;
	}
	
	// add segments of segmented enemies
	int32_t c=0;
	
	switch(guysbuf[id&0xFFF].family)
	{
	case eeMOLD:
	{
		byte is=((enemy*)guys.spr(guys.Count()-1))->item_set;
		id &= 0xFFF;
		
		for(int32_t i=0; i<zc_max(1,zc_min(254,guysbuf[id].misc1)); i++)
		{
			//christ this is messy -DD
			int32_t segclk = -i*((int32_t)(8.0/(zslongToFix(guysbuf[id&0xFFF].step*100))));
			
			if(!guys.add(new esMoldorm((zfix)x,(zfix)y,id+0x1000,segclk)))
			{
				al_trace("Moldorm segment %d could not be created!\n",i+1);
				
				for(int32_t j=0; j<i+1; j++)
					guys.del(guys.Count()-1);
					
				return 0;
			}
			
			if(i>0)
				((enemy*)guys.spr(guys.Count()-1))->item_set=is;
				
			ret++;
		}
		
		break;
	}
	
	case eeLANM:
	{
		id &= 0xFFF;
		int32_t shft = guysbuf[id].misc2;
		byte is=((enemy*)guys.spr(guys.Count()-1))->item_set;
		
		if(!guys.add(new esLanmola((zfix)x,(zfix)y,id+0x1000,0)))
		{
			al_trace("Lanmola segment 1 could not be created!\n");
			guys.del(guys.Count()-1);
			return 0;
		}
		
		ret++;
		
		for(int32_t i=1; i<zc_max(1,zc_min(253,guysbuf[id&0xFFF].misc1)); i++)
		{
			if(!guys.add(new esLanmola((zfix)x,(zfix)y,id+0x2000,-(i<<shft))))
			{
				al_trace("Lanmola segment %d could not be created!\n",i+1);
				
				for(int32_t j=0; j<i+1; j++)
					guys.del(guys.Count()-1);
					
				return 0;
			}
			
			((enemy*)guys.spr(guys.Count()-1))->item_set=is;
			ret++;
		}
	}
	break;
	
	case eeMANHAN:
		id &= 0xFFF;
		
		for(int32_t i=0; i<((!(guysbuf[id].misc2))?4:8); i++)
		{
			if(!guys.add(new esManhandla((zfix)x,(zfix)y,id+0x1000,i)))
			{
				al_trace("Manhandla head %d could not be created!\n",i+1);
				
				for(int32_t j=0; j<i+1; j++)
				{
					guys.del(guys.Count()-1);
				}
				
				return 0;
			}
			
			ret++;
			((enemy*)guys.spr(guys.Count()-1))->frate=guysbuf[id].misc1;
		}
		
		break;
		
	case eeGLEEOK:
	{
		id &= 0xFFF;
		
		for(int32_t i=0; i<zc_max(1,zc_min(254,guysbuf[id&0xFFF].misc1)); i++)
		{
			if(!guys.add(new esGleeok((zfix)x,(zfix)y,id+0x1000,c, e)))
			{
				al_trace("Gleeok head %d could not be created!\n",i+1);
				
				for(int32_t j=0; j<i+1; j++)
				{
					guys.del(guys.Count()-1);
				}
				
				return false;
			}
			
			c-=guysbuf[id].misc4;
			ret++;
		}
	}
	break;
	
	
	case eePATRA:
	{
		id &= 0xFFF;
		int32_t outeyes = 0;
		
		for(int32_t i=0; i<zc_min(254,guysbuf[id&0xFFF].misc1); i++)
		{
			if(!((guysbuf[id].misc10&&get_bit(quest_rules,qr_HARDCODED_BS_PATRA))?guys.add(new esPatraBS((zfix)x,(zfix)y,id+0x1000,i,e)):guys.add(new esPatra((zfix)x,(zfix)y,id+0x1000,i,e))))
			{
				al_trace("Patra outer eye %d could not be created!\n",i+1);
				
				for(int32_t j=0; j<i+1; j++)
					guys.del(guys.Count()-1);
					
				return 0;
			}
			else
				outeyes++;
				
			ret++;
		}
		
		for(int32_t i=0; i<zc_min(254,guysbuf[id&0xFFF].misc2); i++)
		{
			if(!guys.add(new esPatra((zfix)x,(zfix)y,id+0x1000,i,e)))
			{
				al_trace("Patra inner eye %d could not be created!\n",i+1);
				
				for(int32_t j=0; j<i+1+zc_min(254,outeyes); j++)
					guys.del(guys.Count()-1);
					
				return 0;
			}
			
			ret++;
		}
		
		break;
	}
	}
	
	return ret;
}

// Returns number of enemies/segments created
int32_t addenemy(int32_t x,int32_t y,int32_t z,int32_t id,int32_t clk)
{
	//zprint2("addenemy id is: %d\n", (id&0xFFF));
	int32_t realid = id&0xFFF;
	if( realid > MAXGUYS ) 
	{
		//zprint2("Invalid enemy ID (%d) passed to %s\n", id, "addenemy()"); 
		return 0;
	}
	if(id <= 0) return 0;
	
	int32_t ret = 0;
	sprite *e=NULL;
	
	switch(guysbuf[id&0xFFF].family)
	{
		//Fixme: possible enemy memory leak. (minor)
	case eeWALK:
		e = new eStalfos((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeLEV:
		e = new eLeever((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeTEK:
		e = new eTektite((zfix)x,(zfix)y,id,clk);
		break;
		
	case eePEAHAT:
		e = new ePeahat((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeZORA:
		e = new eZora((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeGHINI:
		e = new eGhini((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeKEESE:
		e = new eKeese((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeWIZZ:
		e = new eWizzrobe((zfix)x,(zfix)y,id,clk);
		break;
		
	case eePROJECTILE:
		e = new eProjectile((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeWALLM:
		e = new eWallM((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeAQUA:
		e = new eAquamentus((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeMOLD:
		e = new eMoldorm((zfix)x,(zfix)y,id,zc_max(1,zc_min(254,guysbuf[id&0xFFF].misc1)));
		break;
		
	case eeMANHAN:
		e = new eManhandla((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeGLEEOK:
		e = new eGleeok((zfix)x,(zfix)y,id,zc_max(1,zc_min(254,guysbuf[id&0xFFF].misc1)));
		break;
		
	case eeGHOMA:
		e = new eGohma((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeLANM:
		e = new eLanmola((zfix)x,(zfix)y,id,zc_max(1,zc_min(253,guysbuf[id&0xFFF].misc1)));
		break;
		
	case eeGANON:
		e = new eGanon((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeFAIRY:
		e = new eItemFairy((zfix)x,(zfix)y,id+0x1000*clk,clk);
		break;
		
	case eeFIRE:
		e = new eFire((zfix)x,(zfix)y,id,clk);
		break;
		
	case eeOTHER: 
		e = new eOther((zfix)x,(zfix)y,id,clk);
		break;
	
	
		case eeSCRIPT01: 
	case eeSCRIPT02: 
	case eeSCRIPT03: 
	case eeSCRIPT04: 
	case eeSCRIPT05: 
	case eeSCRIPT06: 
	case eeSCRIPT07: 
	case eeSCRIPT08: 
	case eeSCRIPT09: 
	case eeSCRIPT10: 
	case eeSCRIPT11: 
	case eeSCRIPT12: 
	case eeSCRIPT13: 
	case eeSCRIPT14: 
	case eeSCRIPT15: 
	case eeSCRIPT16: 
	case eeSCRIPT17: 
	case eeSCRIPT18: 
	case eeSCRIPT19: 
	case eeSCRIPT20: 
	{
		if ( !get_bit(quest_rules, qr_SCRIPT_FRIENDLY_ENEMY_TYPES) )
		{
			e = new eScript((zfix)x,(zfix)y,id,clk);
			break;
		}
		else return 0;
	}
				
	case eeFFRIENDLY01:
	case eeFFRIENDLY02:
	case eeFFRIENDLY03:
	case eeFFRIENDLY04:
	case eeFFRIENDLY05:
	case eeFFRIENDLY06:
	case eeFFRIENDLY07:
	case eeFFRIENDLY08:
	case eeFFRIENDLY09:
	case eeFFRIENDLY10:
	{
		if ( !get_bit(quest_rules, qr_SCRIPT_FRIENDLY_ENEMY_TYPES) )
		{
			e = new eFriendly((zfix)x,(zfix)y,id,clk); break;
		}
		else return 0;
				
	}	
		
	case eeSPINTILE:
		e = new eSpinTile((zfix)x,(zfix)y,id,clk);
		break;
		
		// and these enemies use the misc10/misc2 value
	case eeROCK:
	{
		switch(guysbuf[id&0xFFF].misc10)
		{
		case 1:
			e = new eBoulder((zfix)x,(zfix)y,id,clk);
			break;
			
		case 0:
		default:
			e = new eRock((zfix)x,(zfix)y,id,clk);
			break;
		}
		
		break;
	}
	
	case eeTRAP:
	{
		switch(guysbuf[id&0xFFF].misc2)
		{
		case 1:
			e = new eTrap2((zfix)x,(zfix)y,id,clk);
			break;
			
		case 0:
		default:
			e = new eTrap((zfix)x,(zfix)y,id,clk);
			break;
		}
		
		break;
	}
	
	case eeDONGO:
	{
		switch(guysbuf[id&0xFFF].misc10)
		{
		case 1:
			e = new eDodongo2((zfix)x,(zfix)y,id,clk);
			break;
			
		case 0:
		default:
			e = new eDodongo((zfix)x,(zfix)y,id,clk);
			break;
		}
		
		break;
	}
	
	case eeDIG:
	{
		switch(guysbuf[id&0xFFF].misc10)
		{
		case 1:
			e = new eLilDig((zfix)x,(zfix)y,id,clk);
			break;
			
		case 0:
		default:
			e = new eBigDig((zfix)x,(zfix)y,id,clk);
			break;
		}
		
		break;
	}
	
	case eePATRA:
	{
		switch(guysbuf[id&0xFFF].misc10)
		{
		case 1:
			if (get_bit(quest_rules,qr_HARDCODED_BS_PATRA))
			{
				e = new ePatraBS((zfix)x,(zfix)y,id,clk);
				break;
			}
			[[fallthrough]];
		case 0:
		default:
			e = new ePatra((zfix)x,(zfix)y,id,clk);
			break;
		}
		
		break;
	}
	
	case eeGUY:
	{
		switch(guysbuf[id&0xFFF].misc10)
		{
		case 1:
			e = new eTrigger((zfix)x,(zfix)y,id,clk);
			break;
			
		case 0:
		default:
			e = new eNPC((zfix)x,(zfix)y,id,clk);
			break;
		}
		
		break;
	}
	
	case eeNONE:
		if(guysbuf[id&0xFFF].misc10 ==1)
		{
			e = new eTrigger((zfix)x,(zfix)y,id,clk);
			break;
			break;
		}
		[[fallthrough]];
	default:
	
		return 0;
	}
	
	ret++; // Made one enemy.
	
	if(z && canfall(id))
	{
		e->z = (zfix)z;
	}
	
	((enemy*)e)->ceiling = (z && canfall(id));
	
	if(!guys.add(e))
	{
		return 0;
	}
	
	// add segments of segmented enemies
	int32_t c=0;
	
	switch(guysbuf[id&0xFFF].family)
	{
	case eeMOLD:
	{
		byte is=((enemy*)guys.spr(guys.Count()-1))->item_set;
		id &= 0xFFF;
		
		for(int32_t i=0; i<zc_max(1,zc_min(254,guysbuf[id].misc1)); i++)
		{
			//christ this is messy -DD
			int32_t segclk = -i*((int32_t)(8.0/(zslongToFix(guysbuf[id&0xFFF].step*100))));
			
			if(!guys.add(new esMoldorm((zfix)x,(zfix)y,id+0x1000,segclk)))
			{
				al_trace("Moldorm segment %d could not be created!\n",i+1);
				
				for(int32_t j=0; j<i+1; j++)
					guys.del(guys.Count()-1);
					
				return 0;
			}
			
			if(i>0)
				((enemy*)guys.spr(guys.Count()-1))->item_set=is;
				
			ret++;
		}
		
		break;
	}
	
	case eeLANM:
	{
		id &= 0xFFF;
		int32_t shft = guysbuf[id].misc2;
		byte is=((enemy*)guys.spr(guys.Count()-1))->item_set;
		
		if(!guys.add(new esLanmola((zfix)x,(zfix)y,id+0x1000,0)))
		{
			al_trace("Lanmola segment 1 could not be created!\n");
			guys.del(guys.Count()-1);
			return 0;
		}
		
		ret++;
		
		for(int32_t i=1; i<zc_max(1,zc_min(253,guysbuf[id&0xFFF].misc1)); i++)
		{
			if(!guys.add(new esLanmola((zfix)x,(zfix)y,id+0x2000,-(i<<shft))))
			{
				al_trace("Lanmola segment %d could not be created!\n",i+1);
				
				for(int32_t j=0; j<i+1; j++)
					guys.del(guys.Count()-1);
					
				return 0;
			}
			
			((enemy*)guys.spr(guys.Count()-1))->item_set=is;
			ret++;
		}
	}
	break;
	
	case eeMANHAN:
		id &= 0xFFF;
		
		for(int32_t i=0; i<((!(guysbuf[id].misc2))?4:8); i++)
		{
			if(!guys.add(new esManhandla((zfix)x,(zfix)y,id+0x1000,i)))
			{
				al_trace("Manhandla head %d could not be created!\n",i+1);
				
				for(int32_t j=0; j<i+1; j++)
				{
					guys.del(guys.Count()-1);
				}
				
				return 0;
			}
			
			ret++;
			((enemy*)guys.spr(guys.Count()-1))->frate=guysbuf[id].misc1;
		}
		
		break;
		
	case eeGLEEOK:
	{
		id &= 0xFFF;
		
		for(int32_t i=0; i<zc_max(1,zc_min(254,guysbuf[id&0xFFF].misc1)); i++)
		{
			if(!guys.add(new esGleeok((zfix)x,(zfix)y,id+0x1000,c, e)))
			{
				al_trace("Gleeok head %d could not be created!\n",i+1);
				
				for(int32_t j=0; j<i+1; j++)
				{
					guys.del(guys.Count()-1);
				}
				
				return false;
			}
			
			c-=guysbuf[id].misc4;
			ret++;
		}
	}
	break;
	
	
	case eePATRA:
	{
		id &= 0xFFF;
		int32_t outeyes = 0;
		
		for(int32_t i=0; i<zc_min(254,guysbuf[id&0xFFF].misc1); i++)
		{
			if(!((guysbuf[id].misc10&&get_bit(quest_rules,qr_HARDCODED_BS_PATRA))?guys.add(new esPatraBS((zfix)x,(zfix)y,id+0x1000,i,e)):guys.add(new esPatra((zfix)x,(zfix)y,id+0x1000,i,e))))
			{
				al_trace("Patra outer eye %d could not be created!\n",i+1);
				
				for(int32_t j=0; j<i+1; j++)
					guys.del(guys.Count()-1);
					
				return 0;
			}
			else
				outeyes++;
				
			ret++;
		}
		
		for(int32_t i=0; i<zc_min(254,guysbuf[id&0xFFF].misc2); i++)
		{
			if(!guys.add(new esPatra((zfix)x,(zfix)y,id+0x1000,i,e)))
			{
				al_trace("Patra inner eye %d could not be created!\n",i+1);
				
				for(int32_t j=0; j<i+1+zc_min(254,outeyes); j++)
					guys.del(guys.Count()-1);
					
				return 0;
			}
			
			ret++;
		}
		
		break;
	}
	}
	
	return ret;
}

bool isjumper(int32_t id)
{
	if( ((unsigned)(id&0xFFF)) > MAXGUYS ) 
	{
		//zprint2("Invalid enemy ID (%d) passed to %s\n", id, "isjumper()"); 
		return false;
	}
	switch(guysbuf[id&0xFFF].family)
	{
	case eeROCK:
	case eeTEK:
		return true;
		
	case eeWALK:
		if(guysbuf[id&0xFFF].misc9==e9tVIRE || guysbuf[id&0xFFF].misc9==e9tPOLSVOICE) return true;
	}
	
	return false;
}


bool isfixedtogrid(int32_t id)
{
	if( ((unsigned)(id&0xFFF)) > MAXGUYS ) 
	{
		//zprint2("Invalid enemy ID (%d) passed to %s\n", id, "isfixedtogrid()"); 
		return false;
	}
	switch(guysbuf[id&0xFFF].family)
	{
	case eeWALK:
	case eeLEV:
	case eeZORA:
	case eeDONGO:
	case eeGANON:
	case eeROCK:
	case eeGLEEOK:
	case eeAQUA:
	case eeLANM:
		return true;
	}
	
	return false;
}

// Can't fall, can have Z value.
bool isflier(int32_t id)
{
	if( ((unsigned)(id&0xFFF)) > MAXGUYS ) 
	{
		//zprint2("Invalid enemy ID (%d) passed to %s\n", id, "isflier()"); 
		return false;
	}
	switch(guysbuf[id&0xFFF].family) //id&0x0FFF)
	{
	case eePEAHAT:
	case eeKEESE:
	case eePATRA:
	case eeFAIRY:
	case eeGHINI:
	
		// Could theoretically have their Z set by a script
	case eeFIRE:
		return true;
		break;
	}
	
	return false;
}

// Can't have Z position
bool never_in_air(int32_t id)
{
	if( ((unsigned)(id&0xFFF)) > MAXGUYS ) 
	{
		//zprint2("Invalid enemy ID (%d) passed to %s\n", id, "never_in_air()"); 
		return false;
	}
	switch(guysbuf[id&0xFFF].family)
	{
	case eeMANHAN:
	case eeMOLD:
	case eeLANM:
	case eeGLEEOK:
	case eeZORA:
	case eeLEV:
	case eeAQUA:
	case eeROCK:
	case eeGANON:
	case eeTRAP:
	case eePROJECTILE:
	case eeSPINTILE:
		return true;
	}
	
	return false;
}

bool canfall(int32_t id)
{
	if( ((unsigned)(id&0xFFF)) > MAXGUYS ) 
	{
		//zprint2("Invalid enemy ID (%d) passed to %s\n", id, "canfall()"); 
		return false;
	}
	switch(guysbuf[id&0xFFF].family)
	{
	case eeGUY:
	{
		if(id < eOCTO1S)
			return false;
			
		switch(guysbuf[id&0xFFF].misc10)
		{
		case 1:
		case 2:
			return true;
			
		case 0:
		case 3:
		default:
			return false;
		}
		
		case eeGHOMA:
		case eeDIG:
			return false;
		}
	}
   
	
	return !never_in_air(id) && !isflier(id) && !isjumper(id);
}

bool enemy::enemycanfall(int32_t id)
{
	if( ((unsigned)(id&0xFFF)) > MAXGUYS ) 
	{
		//zprint2("Invalid enemy ID (%d) passed to %s\n", id, "enemycanfall()"); 
		return false;
	}
	//Z_scripterrlog("canfall family is %d:\n", family);
	//Z_scripterrlog("canfall gravity is %s:\n", moveflags & FLAG_OBEYS_GRAV ? "true" : "false");
	//if ( family == eeFIRE && id >= eSTART ) 
	//{
	//	Z_scripterrlog("eeFire\n");
	//	return moveflags & FLAG_OBEYS_GRAV; //'Other' enemy class, used by scripts. -Z
	//}
	
	//In ZQ, eeFIRE is Other(floating) and eeOTHER is 'other'.
	
	switch(guysbuf[id&0xFFF].family)
	{
	case eeGUY:
	{
		if(id < eOCTO1S) //screen guys and fires that aren't real enemies, and never fall
			return false;
			
		switch(guysbuf[id&0xFFF].misc10) //I'm unsure what these specify off-hand. Needs better comments. -Z
		{
		case 1:
		case 2:
			return true;
			
		case 0:
		case 3:
		default:
			return false;
		}
		
		case eeGHOMA:
		case eeDIG:
			return false;
		}
	}
	
	if ( isflier(id) || isjumper(id) || never_in_air(id) )
	{
		if ( moveflags & FLAG_OBEYS_GRAV ) return true;
		else return false;
	}
	else
	{
		return (moveflags & FLAG_OBEYS_GRAV);    
	}
	//return !never_in_air(id) && !isflier(id) && !isjumper(id);
}

void addfires()
{
	if(!get_bit(quest_rules,qr_NOGUYFIRES))
	{
		int32_t bs = get_bit(quest_rules,qr_BSZELDA);
		addguy(bs? 64: 72,64,gFIRE,-17,false);
		addguy(bs?176:168,64,gFIRE,-18,false);
	}
}

void loadguys()
{
	if(loaded_guys)
		return;
		
	loaded_guys=true;
	
	byte Guy=0;
	// When in caves/item rooms, use mBELOW and ipONETIME2
	// Else use mITEM and ipONETIME
	int32_t mf = (currscr>=128) ? mBELOW : mITEM;
	int32_t onetime = (currscr>=128) ? ipONETIME2 : ipONETIME;
	
	repaircharge=0;
	adjustmagic=false;
	learnslash=false;
	
	for(int32_t i=0; i<3; i++)
	{
		prices[i]=0;
	}
	
	hasitem=0;
	
	if(currscr>=128 && DMaps[currdmap].flags&dmfGUYCAVES)
	{
		if(DMaps[currdmap].flags&dmfCAVES)
		{
			Guy=tmpscr[1].guy;
		}
	}
	else
	{
		Guy=tmpscr->guy;
		
		if(DMaps[currdmap].flags&dmfVIEWMAP)
			game->maps[(currmap*MAPSCRSNORMAL)+currscr] |= mVISITED;          // mark as visited
	}
	
	// The Guy appears if 'Link is in cave' equals 'Guy is in cave'.
	if(Guy && ((currscr>=128) == !!(DMaps[currdmap].flags&dmfGUYCAVES)))
	{
		if(tmpscr->room==rZELDA)
		{
			addguy(120,72,Guy,-15,true);
			guys.spr(0)->hxofs=1000;
			addenemy(128,96,eFIRE,-15);
			addenemy(112,96,eFIRE,-15);
			addenemy(96,120,eFIRE,-15);
			addenemy(144,120,eFIRE,-15);
			return;
		}
		
		if(Guy!=gFAIRY || !get_bit(quest_rules,qr_NOFAIRYGUYFIRES))
			addfires();
			
		if(currscr>=128)
			if(getmapflag() && !(tmpscr->flags9&fBELOWRETURN))
				Guy=0;
				
		switch(tmpscr->room)
		{
		case rSP_ITEM:
		case rGRUMBLE:
		case rBOMBS:
		case rARROWS:
		case rSWINDLE:
		case rMUPGRADE:
		case rLEARNSLASH:
		case rTAKEONE:
			if((get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW) && getmapflag((currscr < 128) ? mITEM : mBELOW)) || (!get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW) && getmapflag() && !(tmpscr->flags9&fBELOWRETURN))) //get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)
				Guy=0;
				
			break;
			
		case rREPAIR:
			if (get_bit(quest_rules, qr_OLD_DOORREPAIR)) break;
			if((get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW) && getmapflag((currscr < 128) ? mITEM : mBELOW)) || (!get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW) && getmapflag() && !(tmpscr->flags9&fBELOWRETURN))) //get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)
				Guy=0;
				
			break;
		case rRP_HC:
			if (get_bit(quest_rules, qr_OLD_POTION_OR_HC)) break;
			if((get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW) && getmapflag((currscr < 128) ? mITEM : mBELOW)) || (!get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW) && getmapflag() && !(tmpscr->flags9&fBELOWRETURN))) //get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)
				Guy=0;
				
			break;
		case rMONEY:
			if (get_bit(quest_rules, qr_OLD_SECRETMONEY)) break;
			if((get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW) && getmapflag((currscr < 128) ? mITEM : mBELOW)) || (!get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW) && getmapflag() && !(tmpscr->flags9&fBELOWRETURN))) //get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)
				Guy=0;
				
			break;
			
		case rTRIFORCE:
		{
			int32_t tc = TriforceCount();
			
			if(get_bit(quest_rules,qr_4TRI))
			{
				if((get_bit(quest_rules,qr_3TRI) && tc>=3) || tc>=4)
					Guy=0;
			}
			else
			{
				if((get_bit(quest_rules,qr_3TRI) && tc>=6) || tc>=8)
					Guy=0;
			}
		}
		break;
		}
		
		if(Guy)
		{
			if(Guy!=gFAIRY || !get_bit(quest_rules,qr_NOFAIRYGUYFIRES))
				blockpath=true;
				
			if(currscr<128)
				sfx(WAV_SCALE);
				
			addguy(120,64,Guy, (dlevel||BSZ)?-15:startguy[zc_oldrand()&7], true);
			Link.Freeze();
		}
	}
	else if(Guy==gFAIRY)  // The only Guy that somewhat ignores the "Guys In Caves Only" DMap flag
	{
		sfx(WAV_SCALE);
		addguy(120,62,gFAIRY,-14,false);
	}
	
	loaditem();
	
	// Collecting a rupee in a '10 Rupees' screen sets the mITEM screen state if
	// it doesn't appear in a Cave/Item Cellar, and the mBELOW screen state if it does.
	if(tmpscr->room==r10RUPIES && !getmapflag(mf))
	{
		//setmapflag();
		for(int32_t i=0; i<10; i++)
			additem(ten_rupies_x[i],ten_rupies_y[i],0,ipBIGRANGE+onetime,-14);
	}
}

void loaditem()
{
	byte Item = 0;
	
	if(currscr<128)
	{
		Item=tmpscr->item;
		
		if((!getmapflag(mITEM) || (tmpscr->flags9&fITEMRETURN)) && (tmpscr->hasitem != 0))
		{
			if(tmpscr->flags8&fSECRETITEM)
				hasitem=8;
			else if(tmpscr->flags&fITEM)
				hasitem=1;
			else if(tmpscr->enemyflags&efCARRYITEM)
				hasitem=4; // Will be set to 2 by roaming_item
			else
				items.add(new item((zfix)tmpscr->itemx,
								   (tmpscr->flags7&fITEMFALLS && isSideViewGravity()) ? (zfix)-170 : (zfix)tmpscr->itemy+(get_bit(quest_rules, qr_NOITEMOFFSET)?0:1),
								   (tmpscr->flags7&fITEMFALLS && !(isSideViewGravity())) ? (zfix)170 : (zfix)0,
								   Item,ipONETIME|ipBIGRANGE|((itemsbuf[Item].family==itype_triforcepiece ||
										   (tmpscr->flags3&fHOLDITEM)) ? ipHOLDUP : 0) | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0),0));
		}
	}
	else if(!(DMaps[currdmap].flags&dmfCAVES))
	{
		if((!getmapflag((currscr < 128 && get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)) ? mITEM : mBELOW) || (tmpscr[1].flags9&fBELOWRETURN)) && tmpscr[1].room==rSP_ITEM
				&& (currscr==128 || !get_bit(quest_rules,qr_ITEMSINPASSAGEWAYS)))
		{
			Item=tmpscr[1].catchall;
			
			if(Item)
				items.add(new item((zfix)tmpscr->itemx,
								   (tmpscr->flags7&fITEMFALLS && isSideViewGravity()) ? (zfix)-170 : (zfix)tmpscr->itemy+(get_bit(quest_rules, qr_NOITEMOFFSET)?0:1),
								   (tmpscr->flags7&fITEMFALLS && !(isSideViewGravity())) ? (zfix)170 : (zfix)0,
								   Item,ipONETIME2|ipBIGRANGE|ipHOLDUP | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0),0));
		}
	}
}

void never_return(int32_t index)
{
	if(!get_bit(quest_rules,qr_KILLALL))
		goto doit;
		
	for(int32_t i=0; i<guys.Count(); i++)
		if(((((enemy*)guys.spr(i))->d->flags)&guy_neverret) && i!=index)
		{
			goto dontdoit;
		}
		
doit:
	setmapflag(mNEVERRET);
dontdoit:
	return;
}

bool slowguy(int32_t id)
{
	if( ((unsigned)(id&0xFFF)) > MAXGUYS ) 
	{
		//zprint2("Invalid enemy ID (%d) passed to %s\n", id, "slowguy()"); 
		return false;
	}
//return (guysbuf[id].step<100);
	switch(id)
	{
	case eOCTO1S:
	case eOCTO2S:
	case eOCTO1F:
	case eOCTO2F:
	case eLEV1:
	case eLEV2:
	case eROCK:
	case eBOULDER:
		return true;
	}
	
	return false;
}

bool countguy(int32_t id)
{
	id=id; //This is here to prevent a compiler warning.
	//The only argument this ever receives is tmpscr->enemy[i], which means
	// an entry in the screen's enemy list. Thus, it should always be counted. Right? - L.
	return true;
}

bool ok2add(int32_t id)
{
	if( ((unsigned)(id&0xFFF)) > MAXGUYS ) 
	{
		//zprint2("Invalid enemy ID (%d) passed to %s\n", id, "oktoadd()"); 
		return false;
	}
	if(getmapflag(mNEVERRET) && (guysbuf[id].flags & guy_neverret))
		return false;
		
	switch(guysbuf[id].family)
	{
		// I added a special case for shooters because having traps on the same screen
		// was preventing them from spawning due to TMPNORET. This means they will
		// never stay dead, though, so it may not be the best solution. - Saf
	case eePROJECTILE:
		return true;
		
		
	case eeDIG:
	{
		switch(guysbuf[id].misc10)
		{
		case 1:
			if(!get_bit(quest_rules,qr_NOTMPNORET))
				return !getmapflag(mTMPNORET);
				
			return true;
			
		case 0:
		default:
			return true;
		}
	}
	case eeGANON:
	case eeTRAP:
		if ((guysbuf[id].family == eeGANON && !get_bit(quest_rules, qr_CAN_PLACE_GANON))
		|| (guysbuf[id].family == eeTRAP && !get_bit(quest_rules, qr_CAN_PLACE_TRAPS))) return false;
		[[fallthrough]];
	default:
		if (guysbuf[id].flags2&guy_ignoretmpnr) return true;
		break;
	}
	
	if(!get_bit(quest_rules,qr_NOTMPNORET))
		return !getmapflag(mTMPNORET);
		
	return true;
}

void activate_fireball_statue(int32_t pos)
{
	if(!(tmpscr->enemyflags&efFIREBALLS) || statueID<0)
	{
		return;
	}
	
	int32_t cx=-1000, cy=-1000;
	int32_t x = (pos&15)<<4;
	int32_t y = pos&0xF0;
	
	int32_t ctype = combobuf[MAPCOMBO(x,y)].type;
	
	if(!isfixedtogrid(statueID))
	{
		if(ctype==cL_STATUE)
		{
			cx=x+4;
			cy=y+7;
		}
		else if(ctype==cR_STATUE)
		{
			cx=x-8;
			cy=y-1;
		}
		else if(ctype==cC_STATUE)
		{
			cx=x;
			cy=y;
		}
	}
	else if(ctype==cL_STATUE || ctype==cR_STATUE || ctype==cC_STATUE)
	{
		cx=x;
		cy=y;
	}
	
	if(cx!=-1000)  // No point creating it if this is false
	{
		for(int32_t j=0; j<guys.Count(); j++)
		{
			if((int32_t(guys.spr(j)->x)==cx)&&(int32_t(guys.spr(j)->y)==cy))
			{
				if((guys.spr(j)->id&0xFFF) == statueID)  // There's already a matching enemy here!
					return; // No point deleting it. A script might be toying with it in some way.
				else
					guys.del(j);
			}
		}
		
		addenemy(cx, cy, statueID, !isfixedtogrid(statueID) ? 24 : 0);
	}
}

void activate_fireball_statues()
{
	if(!(tmpscr->enemyflags&efFIREBALLS))
	{
		return;
	}
	
	for(int32_t i=0; i<176; i++)
	{
		activate_fireball_statue(i);
	}
}

void load_default_enemies()
{
	wallm_load_clk=frame-80;
	int32_t Id=0;
	
	if(tmpscr->enemyflags&efZORA)
	{
		if(zoraID>=0)
			addenemy(-16, -16, zoraID, 0);
	}
	
	if(tmpscr->enemyflags&efTRAP4)
	{
		if(cornerTrapID>=0)
		{
			addenemy(32, 32, cornerTrapID, -14);
			addenemy(208, 32, cornerTrapID, -14);
			addenemy(32, 128, cornerTrapID, -14);
			addenemy(208, 128, cornerTrapID, -14);
		}
	}
	
	for(int32_t y=0; y<176; y+=16)
	{
		for(int32_t x=0; x<256; x+=16)
		{
			int32_t ctype = combobuf[MAPCOMBO(x,y)].type;
			int32_t cflag = MAPFLAG(x, y);
			int32_t cflag_i = MAPCOMBOFLAG(x, y);
			
			if(ctype==cTRAP_H || cflag==mfTRAP_H || cflag_i==mfTRAP_H)
			{
				if(trapLOSHorizontalID>=0)
					addenemy(x, y, trapLOSHorizontalID, -14);
			}
			else if(ctype==cTRAP_V || cflag==mfTRAP_V || cflag_i==mfTRAP_V)
			{
				if(trapLOSVerticalID>=0)
					addenemy(x, y, trapLOSVerticalID, -14);
			}
			else if(ctype==cTRAP_4 || cflag==mfTRAP_4 || cflag_i==mfTRAP_4)
			{
				if(trapLOS4WayID>=0)
				{
					if(addenemy(x, y, trapLOS4WayID, -14))
						guys.spr(guys.Count()-1)->dummy_int[1]=2;
				}
			}
			
			else if(ctype==cTRAP_LR || cflag==mfTRAP_LR || cflag_i==mfTRAP_LR)
			{
				if(trapConstantHorizontalID>=0)
					addenemy(x, y, trapConstantHorizontalID, -14);
			}
			else if(ctype==cTRAP_UD || cflag==mfTRAP_UD || cflag_i==mfTRAP_UD)
			{
				if(trapConstantVerticalID>=0)
					addenemy(x, y, trapConstantVerticalID, -14);
			}
			
			if(ctype==cSPINTILE1)
			{
				// Awaken spinning tile
				awaken_spinning_tile(tmpscr,COMBOPOS(x,y));
			}
		}
	}
	
	if(tmpscr->enemyflags&efTRAP2)
	{
		if(centerTrapID>=-1)
		{
			if(addenemy(64, 80, centerTrapID, -14))
				guys.spr(guys.Count()-1)->dummy_int[1]=1;
			
			if(addenemy(176, 80, centerTrapID, -14))
				guys.spr(guys.Count()-1)->dummy_int[1]=1;
		}
	}
	
	if(tmpscr->enemyflags&efROCKS)
	{
		if(rockID>=0)
		{
			addenemy(zc_oldrand()&0xF0, 0, rockID, 0);
			addenemy(zc_oldrand()&0xF0, 0, rockID, 0);
			addenemy(zc_oldrand()&0xF0, 0, rockID, 0);
		}
	}
	
	activate_fireball_statues();
}


// Everything that must be done before we change a screen's combo to another combo, or a combo's type to another type.
// There's 2 routines because it's unclear if combobuf or tmpscr->data gets modified. -L
void screen_combo_modify_preroutine(mapscr *s, int32_t pos)
{
	delete_fireball_shooter(s, pos);
}

// Everything that must be done after we change a screen's combo to another combo. -L
void screen_combo_modify_postroutine(mapscr *s, int32_t pos)
{
	s->valid |= mVALID;
	activate_fireball_statue(pos);
	
	if(combobuf[s->data[pos]].type==cSPINTILE1)
	{
		// Awaken spinning tile
		awaken_spinning_tile(s,pos);
	}
	int32_t lyr = -1;
	if(s == tmpscr) lyr = 0;
	else for(size_t q = 0; q < 6; ++q)
	{
		if(s == tmpscr2+q)
		{
			lyr = q+1;
			break;
		}
	}
	if(lyr > -1)
	{
		FFCore.reset_combo_script(lyr, pos);
	}
}

void awaken_spinning_tile(mapscr *s, int32_t pos)
{
	addenemy((pos&15)<<4,pos&0xF0,(s->cset[pos]<<12)+eSPINTILE1,combobuf[s->data[pos]].o_tile+zc_max(1,combobuf[s->data[pos]].frames));
}


// It stands for next_side_pos
void nsp(bool random)
// moves sle_x and sle_y to the next position
{
	if(random)
	{
		if(zc_oldrand()%2)
		{
			sle_x = (zc_oldrand()%2) ? 0 : 240;
			sle_y = (zc_oldrand()%10)*16;
		}
		else
		{
			sle_y = (zc_oldrand()%2) ? 0 : 160;
			sle_x = (zc_oldrand()%15)*16;
		}
		
		return;
	}
	
	if(sle_x==0)
	{
		if(sle_y<160)
			sle_y+=16;
		else
			sle_x+=16;
	}
	else if(sle_y==160)
	{
		if(sle_x<240)
			sle_x+=16;
		else
			sle_y-=16;
	}
	else if(sle_x==240)
	{
		if(sle_y>0)
			sle_y-=16;
		else
			sle_x-=16;
	}
	else if(sle_y==0)
	{
		if(sle_x>0)
			sle_x-=16;
		else
			sle_y+=16;
	}
}

int32_t next_side_pos(bool random)
// moves sle_x and sle_y to the next available position
// returns the direction the enemy should face
{
	bool blocked;
	int32_t c=0;
	
	do
	{
		nsp(c>35 ? false : random);
		blocked = _walkflag(sle_x,sle_y,2) || _walkflag(sle_x,sle_y+8,2) ||
				  (combo_class_buf[COMBOTYPE(sle_x,sle_y)].block_enemies ||
				   MAPFLAG(sle_x,sle_y) == mfNOENEMY || MAPCOMBOFLAG(sle_x,sle_y)==mfNOENEMY ||
				   MAPFLAG(sle_x,sle_y) == mfNOGROUNDENEMY || MAPCOMBOFLAG(sle_x,sle_y)==mfNOGROUNDENEMY ||
				   iswaterex(MAPCOMBO(sle_x,sle_y), currmap, currscr, -1, sle_x, sle_y, true));
				   
		if(++c>50)
			return -1;
	}
	while(blocked);
	
	int32_t dir=0;
	
	if(sle_x==0)    dir=right;
	
	if(sle_y==0)    dir=down;
	
	if(sle_x==240)  dir=left;
	
	if(sle_y==168)  dir=up;
	
	return dir;
}

bool can_side_load(int32_t id)
{
	if( ((unsigned)(id&0xFFF)) > MAXGUYS ) 
	{
		//zprint2("Invalid enemy ID (%d) passed to %s\n", id, "can_side_load()"); 
		return false;
	}
	switch(guysbuf[id].family) //id&0x0FFF)
	{
		//case eTEK1:
		//case eTEK2:
		//case eTEK3:
		//case eLEV1:
		//case eLEV2:
		//case eLEV3:
		//case eRAQUAM:
		//case eLAQUAM:
		//case eDODONGO:
		//case eMANHAN:
		//case eGLEEOK1:
		//case eGLEEOK2:
		//case eGLEEOK3:
		//case eGLEEOK4:
		//case eDIG1:
		//case eDIG3:
		//case eGOHMA1:
		//case eGOHMA2:
		//case eCENT1:
		//case eCENT2:
		//case ePATRA1:
		//case ePATRA2:
		//case eGANON:
		//case eMANHAN2:
		//case eCEILINGM: later
		//case eFLOORM: later
		//case ePATRABS:
		//case ePATRAL2:
		//case ePATRAL3:
		//case eGLEEOK1F:
		//case eGLEEOK2F:
		//case eGLEEOK3F:
		//case eGLEEOK4F:
		//case eDODONGOBS:
		//case eDODONGOF:
		//case eGOHMA3:
		//case eGOHMA4:
		//case eSHOOTMAGIC:
		//case eSHOOTROCK:
		//case eSHOOTSPEAR:
		//case eSHOOTSWORD:
		//case eSHOOTFLAME:
		//case eSHOOTFLAME2:
		//case eSHOOTFBALL:
	case eeTEK:
	case eeLEV:
	case eeAQUA:
	case eeDONGO:
	case eeMANHAN:
	case eeGLEEOK:
	case eeDIG:
	case eeGHOMA:
	case eeLANM:
	case eePATRA:
	case eeGANON:
	case eePROJECTILE:
		return false;
		break;
	}
	
	return true;
}


void side_load_enemies()
{
	if(sle_clk==0)
	{
		sle_cnt = 0;
		int32_t guycnt = 0;
		int16_t s = (currmap<<7)+currscr;
		bool beenhere=false;
		bool reload=true;
		bool reloadspecial = false;
		
		load_default_enemies();
		
		for(int32_t i=0; i<6; i++)
			if(visited[i]==s)
				beenhere=true;
				
		if(!beenhere)
		{
			visited[vhead]=s;
			vhead = (vhead+1)%6;
		}
		else if(game->guys[s]==0)
		{
			sle_cnt=0;
			reload=false;
			reloadspecial = true;
		}
		
		if(reload)
		{
			sle_cnt = game->guys[s];
			
			if(sle_cnt==0)
			{
				while(sle_cnt<10 && tmpscr->enemy[sle_cnt]!=0)
					++sle_cnt;
			}
			else
				reloadspecial = true;
		}
		
		if((get_bit(quest_rules,qr_ALWAYSRET)) || (tmpscr->flags3&fENEMIESRETURN))
		{
			sle_cnt = 0;
			reloadspecial = false;
			
			while(sle_cnt<10 && tmpscr->enemy[sle_cnt]!=0)
				++sle_cnt;
		}
		
		if(reloadspecial)
		{
			for(int32_t i=0; !countguy(tmpscr->enemy[i]) && sle_cnt<10; i++)
				++sle_cnt;
		}
		
		for(int32_t i=0; i<sle_cnt; i++)
			if(countguy(tmpscr->enemy[i]))
				++guycnt;
				
		game->guys[s] = guycnt;
	}
	
	if((++sle_clk+8)%24 == 0)
	{
		int32_t dir = next_side_pos(tmpscr->pattern==pSIDESR);
		
		if(dir==-1 || tooclose(sle_x,sle_y,32))
		{
			return;
		}
		
		int32_t enemy_slot=guys.Count();
		
		while(sle_cnt > 0 && !ok2add(tmpscr->enemy[sle_cnt-1]))
			sle_cnt--;
			
		if(sle_cnt > 0)
		{
			if(addenemy(sle_x,sle_y,tmpscr->enemy[--sle_cnt],0))
				guys.spr(enemy_slot)->dir = dir;
		}
	}
	
	if(sle_cnt<=0)
		loaded_enemies=true;
}

bool is_starting_pos(int32_t i, int32_t x, int32_t y, int32_t t)
{ 
	
	if(tmpscr->enemy[i]<1||tmpscr->enemy[i]>=MAXGUYS) //Hackish fix for crash in Waterford.st on screen 0x65 of dmap 0 (map 1).
	{
		//zprint2("is_starting_pos(), tmpscr->enemy[i] is: %d\n", tmpscr->enemy[i]);
		return false; //never 0, never OoB.
	}
	// No corner enemies
	if((x==0 || x==240) && (y==0 || y==160))

		return false;
	
	//Is a no spawn combo...
	if(MAPFLAG(x+8,y+8)==mfNOENEMYSPAWN || MAPCOMBOFLAG(x+8,y+8)==mfNOENEMYSPAWN)
		return false;
		
	// No enemies in dungeon walls
	if(isdungeon() && (x<32 || x>=224 || y<32 || y>=144))
		return false;
		
	// Too close
	if(tooclose(x,y,40) && t<11)
		return false;
		
	// Can't fly onto it?
	if(isflier(tmpscr->enemy[i])&&
			(flyerblocked(x+8,y+8,spw_floater,guysbuf[tmpscr->enemy[i]])||
			 (_walkflag(x,y+8,2)&&!get_bit(quest_rules,qr_WALLFLIERS))))
		return false;
		
	// Can't jump onto it?
	if
	(
		guysbuf[tmpscr->enemy[i]].family==eeTEK 
		
		&&
		(
			COMBOTYPE(x+8,y+8)==cNOJUMPZONE||
			COMBOTYPE(x+8,y+8)==cNOENEMY||
			ispitfall(x+8,y+8)||
			MAPFLAG(x+8,y+8)==mfNOENEMY||
			MAPCOMBOFLAG(x+8,y+8)==mfNOENEMY
		)
	)
	{
		return false;
	}
		
	// Other off-limit combos
	if((!isflier(tmpscr->enemy[i])&& guysbuf[tmpscr->enemy[i]].family!=eeTEK &&
			(_walkflag(x,y+8,2) || groundblocked(x+8,y+8,guysbuf[tmpscr->enemy[i]]))) &&
			guysbuf[tmpscr->enemy[i]].family!=eeZORA)
		return false;
		
	// Don't ever generate enemies on these combos!
	if(COMBOTYPE(x+8,y+8)==cARMOS||COMBOTYPE(x+8,y+8)==cBSGRAVE)
		return false;
		
	//BS Dodongos need at least 2 spaces.
	if((guysbuf[tmpscr->enemy[i]].family==eeDONGO)&&(guysbuf[tmpscr->enemy[i]].misc10==1))
	{
		if(((x<16) ||_walkflag(x-16,y+8, 2))&&
				((x>224)||_walkflag(x+16,y+8, 2))&&
				((y<16) ||_walkflag(x,   y-8, 2))&&
				((y>144)||_walkflag(x,   y+24,2)))
		{
			return false;
		}
	}
	
	return true;
}

bool is_ceiling_pattern(int32_t i)
{
	return (i==pCEILING || i==pCEILINGR);
}

int32_t placeenemy(int32_t i)
{
	std::map<int32_t, int32_t> freeposcache;
	int32_t frees = 0;
	
	for(int32_t y=0; y<176; y+=16)
	{
		for(int32_t x=0; x<256; x+=16)
		{
			if(is_starting_pos(i,x,y,0))
			{
				freeposcache[frees++] = (y&0xF0)+(x>>4);
			}
		}
	}
	
	if(frees > 0)
		return freeposcache[zc_oldrand()%frees];
		
	return -1;
}

void loadenemies()
{
	if(tmpscr->pattern==pNOSPAWN) return;
	if(loaded_enemies)
		return;
		
	// check if it's the dungeon boss and it has been beaten before
	if(tmpscr->enemyflags&efBOSS && game->lvlitems[dlevel]&liBOSS)
	{
		loaded_enemies = true;
		return;
	}
	
	if(tmpscr->pattern==pSIDES || tmpscr->pattern==pSIDESR)
	{
		side_load_enemies();
		return;
	}
	
	loaded_enemies=true;
	
	// do enemies that are always loaded
	load_default_enemies();
	
	// dungeon basements
	
	static byte dngn_enemy_x[4] = {32,96,144,208};
	
	if(currscr>=128)
	{
		if(DMaps[currdmap].flags&dmfCAVES) return;
		if ( DMaps[currdmap].flags&dmfNEWCELLARENEMIES )
		{
			for(int32_t i=0; i<10; i++)
			{
				if ( tmpscr->enemy[i] )
				{
					addenemy(dngn_enemy_x[i],96,tmpscr->enemy[i],-14-i);
				}
			}
		}
		else
		{
			for(int32_t i=0; i<4; i++)
				addenemy(dngn_enemy_x[i],96,tmpscr->enemy[i]?tmpscr->enemy[i]:(int32_t)eKEESE1,-14-i);
		}
		return;
	}
	
	// check if it's been int32_t enough to reload all enemies
	
	int32_t loadcnt = 10;
	int16_t s = (currmap<<7)+currscr;
	bool beenhere = false;
	bool reload = true;
	
	for(int32_t i=0; i<6; i++)
		if(visited[i]==s)
			beenhere = true;
			
	if(!beenhere)
	{
		visited[vhead]=s;
		vhead = (vhead+1)%6;
	}
	else if(game->guys[s]==0)
	{
		loadcnt = 0;
		reload  = false;
	}
	
	if(reload)
	{
		loadcnt = game->guys[s];
		
		if(loadcnt==0)
			loadcnt = 10;
	}
	
	if((get_bit(quest_rules,qr_ALWAYSRET)) || (tmpscr->flags3&fENEMIESRETURN))
		loadcnt = 10;
		
	for(int32_t i=0; !countguy(tmpscr->enemy[i]) && loadcnt<10; i++)
		++loadcnt;
		
	// load enemies
	
	//if(true)                    // enemies appear at random places
	//{
	//int32_t set=loadside*9;
	int32_t pos=zc_oldrand()%9;
	int32_t clk=-15,x=0,y=0,fastguys=0;
	int32_t i=0,guycnt=0;
	
	for(; i<loadcnt && tmpscr->enemy[i]>0; i++)             /* i=0 */
	{
		bool placed=false;
		int32_t t=-1;
		
		// First: enemy combo flags
		for(int32_t sy=0; sy<176; sy+=16)
		{
			for(int32_t sx=0; sx<256; sx+=16)
			{
				int32_t cflag = MAPFLAG(sx, sy);
				int32_t cflag_i = MAPCOMBOFLAG(sx, sy);
				
				if(((cflag==mfENEMYALL)||(cflag_i==mfENEMYALL)) && (!placed))
				{
					if(!ok2add(tmpscr->enemy[i]))
						++loadcnt;
					else
					{
						addenemy(sx,
								 (is_ceiling_pattern(tmpscr->pattern) && isSideViewGravity()) ? -(150+50*guycnt) : sy,
								 (is_ceiling_pattern(tmpscr->pattern) && !(isSideViewGravity())) ? 150+50*guycnt : 0,tmpscr->enemy[i],-15);
								 
						if(countguy(tmpscr->enemy[i]))
							++guycnt;
							
						placed=true;
						goto placed_enemy;
					}
				}
				
				else if(((cflag==mfENEMY0+i)||(cflag_i==mfENEMY0+i)) && (!placed))
				{
					if(!ok2add(tmpscr->enemy[i]))
						++loadcnt;
					else
					{
						addenemy(sx,
								 (is_ceiling_pattern(tmpscr->pattern) && isSideViewGravity()) ? -(150+50*guycnt) : sy,
								 (is_ceiling_pattern(tmpscr->pattern) && !(isSideViewGravity())) ? 150+50*guycnt : 0,tmpscr->enemy[i],-15);
								 
						if(countguy(tmpscr->enemy[i]))
							++guycnt;
							
						placed=true;
						goto placed_enemy;
					}
				}
			}
		}
		
		// Next: enemy pattern
		if((tmpscr->pattern==pRANDOM || tmpscr->pattern==pCEILING) && !(isSideViewGravity()) && ((tmpscr->enemy[i]>0&&tmpscr->enemy[i]<MAXGUYS)))
		{
			do
			{

				// NES positions
				pos%=9;
				x=stx[loadside][pos];
				y=sty[loadside][pos];
				++pos;
				++t;
			}
			while((t< 20) && !is_starting_pos(i,x,y,t));
		}
		
		if(t<0 || t >= 20) // above enemy pattern failed
		{
			// Final chance: find a random position anywhere onscreen
			int32_t randpos = placeenemy(i);
			
			if(randpos>-1)
			{
				x=(randpos&15)<<4;
				y= randpos&0xF0;
			}
			else // All opportunities failed - abort
			{
				--clk;
				continue;
			}
		}
		
		{
			int32_t c=0;
			c=clk;
			
			if(!slowguy(tmpscr->enemy[i]))
				++fastguys;
			else if(fastguys>0)
				c=-15*(i-fastguys+2);
			else
				c=-15*(i+1);
				
			if(BSZ&&((tmpscr->enemy[i]>0&&tmpscr->enemy[i]<MAXGUYS))) // Hackish fix for crash in Waterford.qst on screen 0x65 of dmap 0 (map 1).
			{
				// Special case for blue leevers
				if(guysbuf[tmpscr->enemy[i]].family==eeLEV && guysbuf[tmpscr->enemy[i]].misc1==1)
					c=-15*(i+1);
				else
					c=-15;
			}
			
			if(!ok2add(tmpscr->enemy[i]))
				++loadcnt;
			else
			{
				if(((tmpscr->enemy[i]>0||tmpscr->enemy[i]<MAXGUYS))) // Hackish fix for crash in Waterford.qst on screen 0x65 of dmap 0 (map 1).
					{
						addenemy(x,(is_ceiling_pattern(tmpscr->pattern) && isSideViewGravity()) ? -(150+50*guycnt) : y,
								 (is_ceiling_pattern(tmpscr->pattern) && !(isSideViewGravity())) ? 150+50*guycnt : 0,tmpscr->enemy[i],c);
								 
						if(countguy(tmpscr->enemy[i]))
							++guycnt;
					}
				}
				
			placed=true;
		}                                                     // if(t < 20)
		
placed_enemy:
		
		// I don't like this, but it seems to work...
		static bool foundCarrier;
		
		if(i==0)
			foundCarrier=false;
		
		if(placed)
		{
			if(i==0 && tmpscr->enemyflags&efLEADER)
			{
				int32_t index = guys.idFirst(tmpscr->enemy[i],0xFFF);
				
				if(index!=-1)
				{
					//grab the first segment. Not accurate to how older versions did it, but the way they did it might be incompatible with enemy editor.
					if ((((enemy*)guys.spr(index))->family == eeLANM) && !get_bit(quest_rules, qr_NO_LANMOLA_RINGLEADER)) index = guys.idNth(tmpscr->enemy[i], 2, 0xFFF); 
					if(index!=-1)                                                                                                                                      
					{
						((enemy*)guys.spr(index))->leader = true;
					}
				}
			}
			
			if(!foundCarrier && hasitem&(4|2))
			{
				int32_t index = guys.idFirst(tmpscr->enemy[i],0xFFF);
				
				if(index!=-1 && (((enemy*)guys.spr(index))->flags&guy_doesntcount)==0)
				{
					((enemy*)guys.spr(index))->itemguy = true;
					foundCarrier=true;
				}
			}
		}
		
		--clk;
	}                                                       // for
	
	game->guys[s] = guycnt;
	//} //if(true)
}

void moneysign()
{
	additem(48,108,iRupy,ipDUMMY);
	//  textout(scrollbuf,zfont,"X",64,112,CSET(0)+1);
	set_clip_state(pricesdisplaybuf, 0);
	textout_ex(pricesdisplaybuf,zfont,"X",64,112,CSET(0)+1,-1);
}

void putprices(bool sign)
{
	if(fadeclk > 0) return;
	// refresh what's under the prices
	// for(int32_t i=5; i<12; i++)
	//   putcombo(scrollbuf,i<<4,112,tmpscr->data[112+i],tmpscr->cpage);
	
	rectfill(pricesdisplaybuf, 72, 112, pricesdisplaybuf->w-1, pricesdisplaybuf->h-1, 0);
	int32_t step=32;
	int32_t x=80;
	
	if(prices[2]==0)
	{
		step<<=1;
		
		if(prices[1]==0)
		{
			x=112;
		}
	}
	
	for(int32_t i=0; i<3; i++)
	{
		// Kind of stupid, but it works: 100000 is used to indicate that an item
		// has a price of zero rather than there being no item.
		// 100000 isn't a valid price, so this doesn't cause problems.
		if(prices[i]!=0 && prices[i]<100000)
		{
			char buf[8];
			sprintf(buf,sign?"%+3d":"%3d",prices[i]);
			
			int32_t l=(int32_t)strlen(buf);
			set_clip_state(pricesdisplaybuf, 0);
			textout_ex(pricesdisplaybuf,zfont,buf,x-(l>3?(l-3)<<3:0),112,CSET(0)+1,-1);
		}
		
		x+=step;
	}
}

// Setting up special rooms
// Also called when the Letter is used successfully.
void setupscreen()
{
	boughtsomething=false;
	int32_t t=currscr<128?0:1;
	word str=tmpscr[t].str;
	
	// Prices are already set to 0 in dowarp()
	switch(tmpscr[t].room)
	{
	case rSP_ITEM:                                          // special item
		additem(120,89,tmpscr[t].catchall,ipONETIME2+ipHOLDUP+ipCHECK | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0));
		break;
		
	case rINFO:                                             // pay for info
	{
		int32_t count = 0;
		int32_t base  = 88;
		int32_t step  = 5;
		
		moneysign();
		
		for(int32_t i=0; i<3; i++)
		{
			if(QMisc.info[tmpscr[t].catchall].str[i])
			{
				++count;
			}
			else
				break;
		}
		
		if(count)
		{
			if(count==1)
			{
				base = 88+32;
			}
			
			if(count==2)
			{
				step = 6;
			}
			
			for(int32_t i=0; i < count; i++)
			{
				additem((i << step)+base, 89, iRupy, ipMONEY + ipDUMMY);
				((item*)items.spr(items.Count()-1))->PriceIndex = i;
				prices[i] = -(QMisc.info[tmpscr[t].catchall].price[i]);
				if(prices[i]==0)
					prices[i]=100000; // So putprices() knows there's an item here and positions the price correctly
				int32_t itemid = current_item_id(itype_wealthmedal);
				
				if(itemid>=0 && prices[i]!=100000)
				{
					if(itemsbuf[itemid].flags & ITEM_FLAG1)
						prices[i]=((prices[i]*itemsbuf[itemid].misc1)/100);
					else
						prices[i]-=itemsbuf[itemid].misc1;
					prices[i]=vbound(prices[i], -99999, 0);
					if(prices[i]==0)
						prices[i]=100000;
				}
				
				if((QMisc.info[tmpscr[t].catchall].price[i])>1 && prices[i]>-1 && prices[i]!=100000)
					prices[i]=-1;
			}
		}
		
		break;
	}
	
	case rMONEY:                                            // secret money
		additem(120,89,iRupy,ipONETIME+ipDUMMY+ipMONEY);
		((item*)items.spr(items.Count()-1))->PriceIndex = 0;
		break;
		
	case rGAMBLE:                                           // gambling
		prices[0]=prices[1]=prices[2]=-10;
		moneysign();
		additem(88,89,iRupy,ipMONEY+ipDUMMY);
		((item*)items.spr(items.Count()-1))->PriceIndex = 0;
		additem(120,89,iRupy,ipMONEY+ipDUMMY);
		((item*)items.spr(items.Count()-1))->PriceIndex = 1;
		additem(152,89,iRupy,ipMONEY+ipDUMMY);
		((item*)items.spr(items.Count()-1))->PriceIndex = 2;
		break;
		
	case rREPAIR:                                           // door repair
		setmapflag((currscr < 128 && get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)) ? mITEM : mBELOW);
		//  }
		repaircharge=tmpscr[t].catchall;
		break;
		
	case rMUPGRADE:                                         // upgrade magic
		adjustmagic=true;
		break;
		
	case rLEARNSLASH:                                       // learn slash attack
		learnslash=true;
		break;
		
	case rRP_HC:                                            // heart container or red potion
		additem(88,89,iRPotion,ipONETIME2+ipHOLDUP+ipFADE);
		((item*)items.spr(items.Count()-1))->PriceIndex = 0;
		additem(152,89,iHeartC,ipONETIME2+ipHOLDUP+ipFADE);
		((item*)items.spr(items.Count()-1))->PriceIndex = 1;
		break;
		
	case rP_SHOP:                                           // potion shop
		if(current_item(itype_letter)<i_letter_used)
		{
			str=0;
			break;
		}

		[[fallthrough]];
	case rTAKEONE:                                          // take one
	case rSHOP:                                             // shop
	{
		int32_t count = 0;
		int32_t base  = 88;
		int32_t step  = 5;
		
		if(tmpscr[t].room != rTAKEONE)
			moneysign();
			
		//count and align the stuff
		for(int32_t i=0; i<3; ++i)
		{
			if(QMisc.shop[tmpscr[t].catchall].hasitem[count] != 0)
			{
				++count;
			}
			else
			{
				break;
			}
		}
		
		if(count==1)
		{
			base = 88+32;
		}
		
		if(count==2)
		{
			step = 6;
		}
		
		for(int32_t i=0; i<count; i++)
		{
			additem((i<<step)+base, 89, QMisc.shop[tmpscr[t].catchall].item[i], ipHOLDUP+ipFADE+(tmpscr[t].room == rTAKEONE ? ipONETIME2 : ipCHECK));
			((item*)items.spr(items.Count()-1))->PriceIndex = i;
			
			if(tmpscr[t].room != rTAKEONE)
			{
				prices[i] = QMisc.shop[tmpscr[t].catchall].price[i];
				if(prices[i]==0)
					prices[i]=100000; // So putprices() knows there's an item here and positions the price correctly
				int32_t itemid = current_item_id(itype_wealthmedal);
				
				if(itemid>=0 && prices[i]!=100000)
				{
					if(itemsbuf[itemid].flags & ITEM_FLAG1)
						prices[i]=((prices[i]*itemsbuf[itemid].misc1)/100);
					else
						prices[i]+=itemsbuf[itemid].misc1;
					prices[i]=vbound(prices[i], 0, 99999);
					if(prices[i]==0)
						prices[i]=100000;
				}
				
				if((QMisc.shop[tmpscr[t].catchall].price[i])>1 && prices[i]<1)
					prices[i]=1;
			}
		}
		
		break;
	}
	case rBOTTLESHOP:                                       // bottle shop
	{
		int32_t count = 0;
		int32_t base  = 88;
		int32_t step  = 5;
		
		moneysign();
		bottleshoptype const& bst = QMisc.bottle_shop_types[tmpscr[t].catchall];
		//count and align the stuff
		for(int32_t i=0; i<3; ++i)
		{
			if(bst.fill[count] != 0)
			{
				++count;
			}
			else
			{
				break;
			}
		}
		
		if(count==1)
		{
			base = 88+32;
		}
		
		if(count==2)
		{
			step = 6;
		}
		
		for(int32_t i=0; i<count; i++)
		{
			adddummyitem((i<<step)+base, 89, /*Use item 0 as a dummy...*/0, ipHOLDUP+ipFADE+ipCHECK);
			//{ Setup dummy item
			item* curItem = ((item*)items.spr(items.Count()-1));
			curItem->PriceIndex = i;
			newcombo const& cmb = combobuf[bst.comb[i]];
			curItem->o_tile = cmb.o_tile;
			curItem->o_cset = bst.cset[i];
			curItem->cs = curItem->o_cset;
			curItem->tile = cmb.o_tile;
			curItem->o_speed = cmb.speed;
			curItem->o_delay = 0;
			curItem->frames = cmb.frames;
			curItem->flip = cmb.flip;
			curItem->family = itype_bottlefill; //no pickup w/o empty bottle
			curItem->pstring = 0;
			curItem->pickup = ipHOLDUP+ipFADE+ipCHECK;
			curItem->flash = false;
			curItem->twohand = false;
			curItem->anim = true;
			curItem->hxsz=1;
			curItem->hyofs=4;
			curItem->hysz=12;
			curItem->script=0;
			curItem->txsz=1;
			curItem->tysz=1;
			//}
			
			prices[i] = bst.price[i];
			if(prices[i]==0)
				prices[i]=100000; // So putprices() knows there's an item here and positions the price correctly
			int32_t itemid = current_item_id(itype_wealthmedal);
			
			if(itemid>=0 && prices[i]!=100000)
			{
				if(itemsbuf[itemid].flags & ITEM_FLAG1)
					prices[i]=((prices[i]*itemsbuf[itemid].misc1)/100);
				else
					prices[i]+=itemsbuf[itemid].misc1;
				prices[i]=vbound(prices[i], 0, 99999);
				if(prices[i]==0)
					prices[i]=100000;
			}
			
			if((bst.price[i])>1 && prices[i]<1)
				prices[i]=1;
		}
		
		break;
	}
	
	case rBOMBS:                                            // more bombs
		additem(120,89,iRupy,ipDUMMY+ipMONEY);
		((item*)items.spr(items.Count()-1))->PriceIndex = 0;
		prices[0]=-tmpscr[t].catchall;
		break;
		
	case rARROWS:                                            // more arrows
		additem(120,89,iRupy,ipDUMMY+ipMONEY);
		((item*)items.spr(items.Count()-1))->PriceIndex = 0;
		prices[0]=-tmpscr[t].catchall;
		break;
		
	case rSWINDLE:                                          // leave heart container or money
		additem(88,89,iHeartC,ipDUMMY+ipMONEY);
		((item*)items.spr(items.Count()-1))->PriceIndex = 0;
		prices[0]=-1;
		additem(152,89,iRupy,ipDUMMY+ipMONEY);
		((item*)items.spr(items.Count()-1))->PriceIndex = 1;
		prices[1]=-tmpscr[t].catchall;
		break;
		
	}
	
	if(tmpscr[t].room == rBOMBS || tmpscr[t].room == rARROWS)
	{
		int32_t i = (tmpscr[t].room == rSWINDLE ? 1 : 0);
		int32_t itemid = current_item_id(itype_wealthmedal);
		
		if(itemid >= 0)
		{
			if(itemsbuf[itemid].flags & ITEM_FLAG1)
				prices[i]*=(itemsbuf[itemid].misc1/100.0);
			else
				prices[i]+=itemsbuf[itemid].misc1;
		}
		
		if(tmpscr[t].catchall>1 && prices[i]>-1)
			prices[i]=-1;
	}
	
	putprices(false);
	
	if(str)
	{
		donewmsg(str);
	}
	else
	{
		Link.unfreeze();
	}
}

// Increments msgptr and returns the control code argument pointed at.
word grab_next_argument()
{
	byte val=MsgStrings[msgstr].s[++msgptr]-1;
	word ret=val;
	
	// If an argument is succeeded by 255, then it's a three-byte argument -
	// between 254 and 65535 (or whatever the maximum actually is)
	if((uint8_t)(MsgStrings[msgstr].s[msgptr+1]) == 255)
	{
		val=MsgStrings[msgstr].s[msgptr+2];
		word next=val;
		ret += 254*next;
		msgptr+=2;
	}
	
	return ret;
}

static bool doing_name_insert = false;
static char namebuf[9] = {0};
static char* nameptr = NULL;
static int32_t ssc_tile_hei = -1, ssc_tile_hei_buf = -1;
bool parsemsgcode()
{
	if(msgptr>=MSGSIZE-2) return false;
	switch(MsgStrings[msgstr].s[msgptr]-1)
	{
		case MSGC_NEWLINE:
		{
			int32_t thei = zc_max(ssc_tile_hei, text_height(msgfont));
			ssc_tile_hei = ssc_tile_hei_buf;
			ssc_tile_hei_buf = -1;
			cursor_y += thei + MsgStrings[msgstr].vspace;
			cursor_x=(get_bit(quest_rules,qr_OLD_STRING_EDITOR_MARGINS)!=0 ? 0 : msg_margins[left]);
			return true;
		}	
		
		case MSGC_COLOUR:
		{
			int32_t cset = (grab_next_argument());
			msgcolour = CSET(cset)+(grab_next_argument());
			return true;
		}
		
		case MSGC_SHDCOLOR:
		{
			int32_t cset = (grab_next_argument());
			msg_shdcol = CSET(cset)+(grab_next_argument());
			return true;
		}
		case MSGC_SHDTYPE:
		{
			msg_shdtype = grab_next_argument();
			return true;
		}
		
		case MSGC_SPEED:
		{
			msgspeed=grab_next_argument();
			return true;
		}
		
		case MSGC_CTRUP:
		{
			int32_t a1 = grab_next_argument();
			int32_t a2 = grab_next_argument();
			game->change_counter(a2, a1);
			return true;
		}
		
		case MSGC_CTRDN:
		{
			int32_t a1 = grab_next_argument();
			int32_t a2 = grab_next_argument();
			game->change_counter(-a2, a1);
			return true;
		}
		
		case MSGC_CTRSET:
		{
			int32_t a1 = grab_next_argument();
			int32_t a2 = grab_next_argument();
			game->set_counter(vbound(a2, 0, game->get_maxcounter(a1)), a1);
			return true;
		}
		
		case MSGC_CTRUPPC:
		case MSGC_CTRDNPC:
		case MSGC_CTRSETPC:
		{
			int32_t code = MsgStrings[msgstr].s[msgptr]-1;
			int32_t counter = grab_next_argument();
			int32_t amount = grab_next_argument();
			amount = int32_t(vbound(amount*0.01, 0.0, 1.0)*game->get_maxcounter(counter));
			
			if(code==MSGC_CTRDNPC)
				amount*=-1;
				
			if(code==MSGC_CTRSETPC)
				game->set_counter(amount, counter);
			else
				game->change_counter(amount, counter);
				
			return true;
		}
		
		case MSGC_GIVEITEM:
		{
			int32_t itemID = grab_next_argument();
			
			getitem(itemID, true);
			if ( !item_doscript[itemID] && (((unsigned)itemID) < 256) )
			{
				itemScriptData[itemID].Clear();
				memset(item_stack[itemID], 0xFFFF, MAX_SCRIPT_REGISTERS * sizeof(int32_t));
				if ( (itemsbuf[itemID].flags&ITEM_PASSIVESCRIPT) ) item_doscript[itemID] = 1;
			}
			return true;
		}
			
		
		case MSGC_WARP:
		{
			int32_t    dmap =  grab_next_argument();
			int32_t    scrn =  grab_next_argument();
			int32_t    dx =  grab_next_argument();
			int32_t    dy =  grab_next_argument();
			int32_t    wfx =  grab_next_argument();
			int32_t    sfx =  grab_next_argument();
			FFCore.warp_link(wtIWARP, dmap, scrn, dx, dy, wfx, sfx, warpFlagDONTKILLMUSIC, 0);
			return true;
		}
		
		case MSGC_SETSCREEND:
		{
			int32_t dmap =     (grab_next_argument()<<7); //dmap and screen may be transposed here.
			int32_t screen =     grab_next_argument();
			int32_t reg =     grab_next_argument();
			int32_t val =     grab_next_argument();
			FFCore.set_screen_d(screen + dmap, reg, val);
			return true;
		}
		case MSGC_TAKEITEM:
		{
			int32_t itemID = grab_next_argument();
			if ( item_doscript[itemID] )
			{
				item_doscript[itemID] = 4; //Val of 4 means 'clear stack and quit'
			}
			takeitem(itemID);
			if ( game->forced_bwpn == itemID ) 
			{
				game->forced_bwpn = -1;
			} //not else if! -Z
			if ( game->forced_awpn == itemID ) 
			{
				game->forced_awpn = -1;
			}
			if ( game->forced_xwpn == itemID ) 
			{
				game->forced_xwpn = -1;
			} //not else if! -Z
			if ( game->forced_ywpn == itemID ) 
			{
				game->forced_ywpn = -1;
			}
			verifyBothWeapons();
			return true;
		}
			
		case MSGC_SFX:
		{
			sfx((int32_t)grab_next_argument(),128);
			return true;
		}
		
		case MSGC_MIDI:
		{
			int32_t music = (int32_t)(grab_next_argument());
			
			if(music==0)
				music_stop();
			else
				jukebox(music+(ZC_MIDI_COUNT-1));
				
			return true;
		}
		
		case MSGC_NAME:
		{
			doing_name_insert = true;
			sprintf(namebuf, "%s", game->get_name());
			nameptr = namebuf;
			return true;
		}
			
		case MSGC_DRAWTILE:
		{
			int32_t tl = grab_next_argument();
			int32_t cs = grab_next_argument();
			int32_t t_wid = grab_next_argument();
			int32_t t_hei = grab_next_argument();
			int32_t fl = grab_next_argument();
			
			if(cursor_x+MsgStrings[msgstr].hspace + t_wid > msg_w-msg_margins[right])
			{
				int32_t thei = zc_max(ssc_tile_hei, text_height(msgfont));
				ssc_tile_hei = ssc_tile_hei_buf;
				ssc_tile_hei_buf = -1;
				cursor_y += thei + MsgStrings[msgstr].vspace;
				if(cursor_y >= (msg_h - msg_margins[down])) break;
				cursor_x=msg_margins[left];
			}
			
			overtileblock16(msg_txt_bmp_buf, tl, cursor_x, cursor_y, (int32_t)ceil(t_wid/16.0), (int32_t)ceil(t_hei/16.0), cs, fl);
			ssc_tile_hei_buf = zc_max(ssc_tile_hei_buf, t_hei);
			cursor_x += MsgStrings[msgstr].hspace + t_wid;
			return true;
		}
		
		case MSGC_GOTOIFRAND:
		{
			int32_t odds = (int32_t)(grab_next_argument());
			
			if(!((zc_oldrand()%(2*odds))/odds))
				goto switched;
				
			(void)grab_next_argument();
			return true;
		}
		
		case MSGC_GOTOIFGLOBAL:
		{
			int32_t arg = (int32_t)grab_next_argument();
			int32_t d = zc_min(7,arg);
			int32_t s = ((get_currdmap())<<7) + get_currscr()-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
			arg = (int32_t)grab_next_argument();
			
			if(game->screen_d[s][d] >= arg)
				goto switched;
				
			(void)grab_next_argument();
			return true;
		}
		
		case MSGC_CHANGEPORTRAIT:
		{
			return true; //not implemented
		}
		
		case MSGC_GOTOIFCREEND:
		{
			int32_t dmap =     (grab_next_argument()<<7); //dmap and screen may be transposed here.
			int32_t screen =     grab_next_argument();
			int32_t reg =     grab_next_argument();
			int32_t val =     grab_next_argument();
			int32_t nxtstr = grab_next_argument();
			if ( FFCore.get_screen_d(screen + dmap, reg) >= val )
			{
				goto switched;
			}
			(void)grab_next_argument();
			return true;
		}
		
		case MSGC_GOTOIF:
		{
			int32_t it = (int32_t)grab_next_argument();
			
			if(it<MAXITEMS && game->item[it])
				goto switched;
				
			(void)grab_next_argument();
			return true;
		}
		
		case MSGC_GOTOIFCTR:
		{
			if(game->get_counter(grab_next_argument())>=grab_next_argument())
				goto switched;
				
			(void)grab_next_argument();
			return true;
		}
		
		case MSGC_GOTOIFCTRPC:
		{
			int32_t counter = grab_next_argument();
			int32_t amount = (int32_t)(((grab_next_argument())/100)*game->get_maxcounter(counter));
			
			if(game->get_counter(counter)>=amount)
				goto switched;
				
			(void)grab_next_argument();
			return true;
		}
		
		case MSGC_GOTOIFTRICOUNT:
		{
			if(TriforceCount() >= (int32_t)(grab_next_argument()))
				goto switched;
				
			(void)grab_next_argument();
			return true;
		}
		
		case MSGC_GOTOIFTRI:
		{
			int32_t lev = (int32_t)(grab_next_argument());
			
			if(lev<MAXLEVELS && game->lvlitems[lev]&liTRIFORCE)
				goto switched;
				
			(void)grab_next_argument();
			return true;
		}
		
		/*case MSGC_GOTOIFYN:
		{
			bool done=false;
			int32_t pos = 0;
			set_clip_state(msg_txt_display_buf, 0);
			
			do // Copied from title.cpp...
			{
				int32_t f=-1;
				bool done2=false;
				// TODO: Lower Y value limit
				textout_ex(msg_txt_display_buf, msgfont,"YES",112,MsgStrings[msgstr].y+36,msgcolour,-1);
				textout_ex(msg_txt_display_buf, msgfont,"NO",112,MsgStrings[msgstr].y+48,msgcolour,-1);
				
				do
				{
					load_control_state();
					
					if(f==-1)
					{
						if(rUp())
						{
							sfx(WAV_CHINK);
							pos=0;
						}
						
						if(rDown())
						{
							sfx(WAV_CHINK);
							pos=1;
						}
						
						if(rSbtn()) ++f;
					}
					
					if(f>=0)
					{
						if(++f == 65)
							done2=true;
							
						if(!(f&3))
						{
							int32_t c = (f&4) ? msgcolour : QMisc.colors.caption;
							
							switch(pos)
							{
							case 0:
								textout_ex(msg_txt_display_buf, msgfont,"YES",112,MsgStrings[msgstr].y+36,c,-1);
								break;
								
							case 1:
								textout_ex(msg_txt_display_buf, msgfont,"NO",112,MsgStrings[msgstr].y+48,c,-1);
								break;
							}
						}
					}
					
					rectfill(msg_txt_display_buf,96,MsgStrings[msgstr].y+36,136,MsgStrings[msgstr].y+60,0);
					overtile8(msg_txt_display_buf,2,96,(pos*16)+MsgStrings[msgstr].y+36,1,0);
					advanceframe(true);
				}
				while(!Quit && !done2);
				
				clear_bitmap(msg_txt_display_buf);
				done=true;
			}
			while(!Quit && !done);
			
			if(pos==0)
				goto switched;
				
			++msgptr;
			return true;
		}
		*/
	
switched:
		int32_t lev = (int32_t)(grab_next_argument());
		donewmsg(lev);
		msgptr--; // To counteract it being incremented after this routine is called.
		putprices(false);
		return true;
	}
	
	return false;
}

// Wraps the message string... probably.
void wrapmsgstr(char *s3)
{
	int32_t j=0;
	
	if(MsgStrings[msgstr].stringflags & STRINGFLAG_WRAP)
	{
		if(msgspace)
		{
			if(MsgStrings[msgstr].s[msgptr] >= 32 && MsgStrings[msgstr].s[msgptr] <= 126)
			{
				for(int32_t k=0; MsgStrings[msgstr].s[msgptr+k] && MsgStrings[msgstr].s[msgptr+k] != ' '; k++)
				{
					if(MsgStrings[msgstr].s[msgptr+k] >= 32 && MsgStrings[msgstr].s[msgptr+k] <= 126) s3[j++] = MsgStrings[msgstr].s[msgptr+k];
				}
				
				s3[j] = 0;
				msgspace = false;
			}
			else
			{
				s3[0] = MsgStrings[msgstr].s[msgptr];
				s3[1] = 0;
			}
		}
		else
		{
			s3[0] = MsgStrings[msgstr].s[msgptr];
			s3[1] = 0;
			
			if(s3[0] == ' ') msgspace=true;
		}
	}
	else
	{
		s3[0] = MsgStrings[msgstr].s[msgptr];
		s3[1] = 0;
	}
}

// Returns true if the pointer is at a string's
// null terminator or a trailing space
bool atend(char *str)
{
	int32_t i=0;
	
	while(str[i]==' ')
		i++;
		
	return str[i]=='\0';
}

void putmsg()
{
	bool oldmargin = get_bit(quest_rules,qr_OLD_STRING_EDITOR_MARGINS)!=0;
	if(!msgorig) msgorig=msgstr;
	
	if(linkedmsgclk>0)
	{
		if(linkedmsgclk==1)
		{
			if(cAbtn()||cBbtn())
			{
				msgstr=MsgStrings[msgstr].nextstring;
				if(!msgstr && enqueued_str)
				{
					msgstr = enqueued_str;
					enqueued_str = 0;
				}
				if(!msgstr)
				{
					msgfont=zfont;
					
					if(tmpscr->room!=rGRUMBLE)
						blockpath=false;
						
					dismissmsg();
					goto disappear;
				}
				
				donewmsg(msgstr);
				putprices(false);
			}
		}
		else
		{
			--linkedmsgclk;
		}
	}
	
	if(!msgstr || msgpos>=10000 || msgptr>=MSGSIZE || cursor_y >= msg_h-(oldmargin?0:msg_margins[down]))
	{
		if(!msgstr)
			msgorig=0;
			
		msg_active = false;
		return;
	}
	
	msg_onscreen = true; // Now the message is onscreen (see donewmsg()).
	
	char s3[145];
	int32_t tlength;
	
	// Bypass the string with the B button!
	if(((cBbtn())&&(get_bit(quest_rules,qr_ALLOWMSGBYPASS))) || msgspeed==0)
	{
		//finish writing out the string
		while(msgptr<MSGSIZE && !atend(MsgStrings[msgstr].s+msgptr))
		{
			if(msgspeed && !(cBbtn() && get_bit(quest_rules,qr_ALLOWMSGBYPASS)))
				goto breakout; // break out if message speed was changed to non-zero
			else if(!doing_name_insert && !parsemsgcode())
			{
				if(cursor_y >= msg_h-(oldmargin?0:msg_margins[down]))
					break;
					
				wrapmsgstr(s3);
				
				if(MsgStrings[msgstr].s[msgptr]==' ')
				{
					tlength = msgfont->vtable->char_length(msgfont, MsgStrings[msgstr].s[msgptr]) + MsgStrings[msgstr].hspace;
					
					if(cursor_x+tlength > (msg_w-(oldmargin ? 0 : msg_margins[right]))
					   && ((cursor_x > (msg_w-(oldmargin ? 0 : msg_margins[right])) || !(MsgStrings[msgstr].stringflags & STRINGFLAG_WRAP))
							? true : strcmp(s3," ")!=0))
					{
						int32_t thei = zc_max(ssc_tile_hei, text_height(msgfont));
						ssc_tile_hei = ssc_tile_hei_buf;
						ssc_tile_hei_buf = -1;
						cursor_y += thei + MsgStrings[msgstr].vspace;
						cursor_x=oldmargin ? 0 : msg_margins[left];
					}
					
					/*textprintf_ex(msg_txt_bmp_buf,msgfont,cursor_x+(oldmargin?8:0),cursor_y+(oldmargin?8:0),msgcolour,-1,
								  "%c",MsgStrings[msgstr].s[msgptr]);*/
					
					char buf[2] = {0};
					sprintf(buf,"%c",MsgStrings[msgstr].s[msgptr]);
					
					textout_styled_aligned_ex(msg_txt_bmp_buf,msgfont,buf,cursor_x+(oldmargin?8:0),cursor_y+(oldmargin?8:0),msg_shdtype,sstaLEFT,msgcolour,msg_shdcol,-1);
					
					cursor_x+=tlength;
				}
				else
				{
					tlength = text_length(msgfont, s3) + ((int32_t)strlen(s3)*MsgStrings[msgstr].hspace);
					
					if(cursor_x+tlength > (msg_w-(oldmargin ? 0 : msg_margins[right]))
					   && ((cursor_x > (msg_w-(oldmargin ? 0 : msg_margins[right])) || !(MsgStrings[msgstr].stringflags & STRINGFLAG_WRAP))
							? true : strcmp(s3," ")!=0))
					{
						int32_t thei = zc_max(ssc_tile_hei, text_height(msgfont));
						ssc_tile_hei = ssc_tile_hei_buf;
						ssc_tile_hei_buf = -1;
						cursor_y += thei + MsgStrings[msgstr].vspace;
						cursor_x=oldmargin ? 0 : msg_margins[left];
					}
					
					sfx(MsgStrings[msgstr].sfx);
					/*textprintf_ex(msg_txt_bmp_buf,msgfont,cursor_x+(oldmargin?8:0),cursor_y+(oldmargin?8:0),msgcolour,-1,
								  "%c",MsgStrings[msgstr].s[msgptr]);*/
					
					char buf[2] = {0};
					sprintf(buf,"%c",MsgStrings[msgstr].s[msgptr]);
					
					textout_styled_aligned_ex(msg_txt_bmp_buf,msgfont,buf,cursor_x+(oldmargin?8:0),cursor_y+(oldmargin?8:0),msg_shdtype,sstaLEFT,msgcolour,msg_shdcol,-1);
					
					cursor_x += msgfont->vtable->char_length(msgfont, MsgStrings[msgstr].s[msgptr]);
					cursor_x += MsgStrings[msgstr].hspace;
				}
				
				msgpos++;
			}
			if(doing_name_insert)
			{
				if(!*nameptr)
				{
					doing_name_insert = false;
					++msgptr;
					continue; //back to next normal character
				}
				if(cursor_y >= msg_h-(oldmargin?0:msg_margins[down]))
					break;
				
				char s3[9] = {0};
				
				if(MsgStrings[msgstr].stringflags & STRINGFLAG_WRAP)
				{
					strcpy(s3, nameptr);
				}
				else
				{
					s3[0] = *nameptr;
					s3[1] = 0;
				}
				
				tlength = text_length(msgfont, s3) + ((int32_t)strlen(s3)*MsgStrings[msgstr].hspace);
				
				if(cursor_x+tlength > (msg_w-(oldmargin ? 0 : msg_margins[right]))
				   && ((cursor_x > (msg_w-(oldmargin ? 0 : msg_margins[right])) || !(MsgStrings[msgstr].stringflags & STRINGFLAG_WRAP))
						? true : strcmp(s3," ")!=0))
				{
					int32_t thei = zc_max(ssc_tile_hei, text_height(msgfont));
					ssc_tile_hei = ssc_tile_hei_buf;
					ssc_tile_hei_buf = -1;
					cursor_y += thei + MsgStrings[msgstr].vspace;
					cursor_x=oldmargin ? 0 : msg_margins[left];
				}
				
				sfx(MsgStrings[msgstr].sfx);
				
				char buf[2] = {0};
				sprintf(buf,"%c",*nameptr);
				
				textout_styled_aligned_ex(msg_txt_bmp_buf,msgfont,buf,cursor_x+(oldmargin?8:0),cursor_y+(oldmargin?8:0),msg_shdtype,sstaLEFT,msgcolour,msg_shdcol,-1);
				
				cursor_x += msgfont->vtable->char_length(msgfont, *nameptr);
				cursor_x += MsgStrings[msgstr].hspace;
				++nameptr;
				continue; //don't advance the msgptr, as the next char in it was not processed!
			}
			++msgptr;
			
			if(atend(MsgStrings[msgstr].s+msgptr))
			{
				if(MsgStrings[msgstr].nextstring)
				{
					if(MsgStrings[MsgStrings[msgstr].nextstring].stringflags & STRINGFLAG_CONT)
					{
						msgstr=MsgStrings[msgstr].nextstring;
						msgpos=msgptr=0;
						msgfont=setmsgfont();
					}
				}
			}
		}
		
		msgclk=72;
		msgpos=10000;
	}
	else
	{
breakout:

		if(((msgclk++)%(msgspeed+1)<msgspeed)&&((!cAbtn())||(!get_bit(quest_rules,qr_ALLOWFASTMSG))))
			return;
	}
	
	// Start writing the string
	if(msgptr == 0)
	{
		while(MsgStrings[msgstr].s[msgptr]==' ')
		{
			tlength = msgfont->vtable->char_length(msgfont, MsgStrings[msgstr].s[msgptr]) + MsgStrings[msgstr].hspace;
			
			if(cursor_x+tlength > (msg_w-(oldmargin ? 0 : msg_margins[right]))
			   && ((cursor_x > (msg_w-(oldmargin ? 0 : msg_margins[right])) || !(MsgStrings[msgstr].stringflags & STRINGFLAG_WRAP))
					? 1 : strcmp(s3," ")!=0))
			{
				int32_t thei = zc_max(ssc_tile_hei, text_height(msgfont));
				ssc_tile_hei = ssc_tile_hei_buf;
				ssc_tile_hei_buf = -1;
				cursor_y += thei + MsgStrings[msgstr].vspace;
				cursor_x=oldmargin ? 0 : msg_margins[left];
			}
			
			cursor_x+=tlength;
			++msgptr;
			++msgpos;
			
			// The "Continue From Previous" feature
			if(atend(MsgStrings[msgstr].s+msgptr))
			{
				if(MsgStrings[msgstr].nextstring)
				{
					if(MsgStrings[MsgStrings[msgstr].nextstring].stringflags & STRINGFLAG_CONT)
					{
						msgstr=MsgStrings[msgstr].nextstring;
						msgpos=msgptr=0;
						msgfont=setmsgfont();
					}
				}
			}
		}
	}
	
reparsesinglechar:
	// Continue printing the string!
	if(!atend(MsgStrings[msgstr].s+msgptr) && cursor_y < msg_h-(oldmargin?0:msg_margins[down]))
	{
		if(!doing_name_insert && !parsemsgcode())
		{
			wrapmsgstr(s3);
			
			tlength = text_length(msgfont, s3) + ((int32_t)strlen(s3)*MsgStrings[msgstr].hspace);
			
			if(cursor_x+tlength > (msg_w-(oldmargin ? 0 : msg_margins[right]))
			   && ((cursor_x > (msg_w-(oldmargin ? 0 : msg_margins[right])) || !(MsgStrings[msgstr].stringflags & STRINGFLAG_WRAP))
					? true : strcmp(s3," ")!=0))
			{
				int32_t thei = zc_max(ssc_tile_hei, text_height(msgfont));
				ssc_tile_hei = ssc_tile_hei_buf;
				ssc_tile_hei_buf = -1;
				cursor_y += thei + MsgStrings[msgstr].vspace;
				cursor_x=oldmargin ? 0 : msg_margins[left];
				//if(space) s3[0]=0;
			}
			
			sfx(MsgStrings[msgstr].sfx);
			/*textprintf_ex(msg_txt_bmp_buf,msgfont,cursor_x+(oldmargin?8:0),cursor_y+(oldmargin?8:0),msgcolour,-1,
						  "%c",MsgStrings[msgstr].s[msgptr]);*/
			
			char buf[2] = {0};
			sprintf(buf,"%c",MsgStrings[msgstr].s[msgptr]);
			
			textout_styled_aligned_ex(msg_txt_bmp_buf,msgfont,buf,cursor_x+(oldmargin?8:0),cursor_y+(oldmargin?8:0),msg_shdtype,sstaLEFT,msgcolour,msg_shdcol,-1);
			
			cursor_x += msgfont->vtable->char_length(msgfont, MsgStrings[msgstr].s[msgptr]);
			cursor_x += MsgStrings[msgstr].hspace;
			msgpos++;
		}
		if(doing_name_insert)
		{
			if(!*nameptr)
			{
				doing_name_insert = false;
				++msgptr;
				goto reparsesinglechar; //
			}
			
			char s3[9] = {0};
			
			if(MsgStrings[msgstr].stringflags & STRINGFLAG_WRAP)
			{
				strcpy(s3, nameptr);
			}
			else
			{
				s3[0] = *nameptr;
				s3[1] = 0;
			}
			
			tlength = text_length(msgfont, s3) + ((int32_t)strlen(s3)*MsgStrings[msgstr].hspace);
			
			if(cursor_x+tlength > (msg_w-(oldmargin ? 0 : msg_margins[right]))
			   && ((cursor_x > (msg_w-(oldmargin ? 0 : msg_margins[right])) || !(MsgStrings[msgstr].stringflags & STRINGFLAG_WRAP))
					? true : strcmp(s3," ")!=0))
			{
				int32_t thei = zc_max(ssc_tile_hei, text_height(msgfont));
				ssc_tile_hei = ssc_tile_hei_buf;
				ssc_tile_hei_buf = -1;
				cursor_y += thei + MsgStrings[msgstr].vspace;
				cursor_x=oldmargin ? 0 : msg_margins[left];
			}
			
			sfx(MsgStrings[msgstr].sfx);
			
			char buf[2] = {0};
			sprintf(buf,"%c",*nameptr);
			
			textout_styled_aligned_ex(msg_txt_bmp_buf,msgfont,buf,cursor_x+(oldmargin?8:0),cursor_y+(oldmargin?8:0),msg_shdtype,sstaLEFT,msgcolour,msg_shdcol,-1);
			
			cursor_x += msgfont->vtable->char_length(msgfont, *nameptr);
			cursor_x += MsgStrings[msgstr].hspace;
			++nameptr;
		}
		else
		{
			msgptr++;
			
			if(atend(MsgStrings[msgstr].s+msgptr))
			{
				if(MsgStrings[msgstr].nextstring)
				{
					if(MsgStrings[MsgStrings[msgstr].nextstring].stringflags & STRINGFLAG_CONT)
					{
						msgstr=MsgStrings[msgstr].nextstring;
						msgpos=msgptr=0;
						msgfont=setmsgfont();
					}
				}
			}
			
			if((MsgStrings[msgstr].s[msgptr]==' ') && (MsgStrings[msgstr].s[msgptr+1]==' '))
				while(MsgStrings[msgstr].s[msgptr]==' ')
				{
					tlength = msgfont->vtable->char_length(msgfont, MsgStrings[msgstr].s[msgptr]) + MsgStrings[msgstr].hspace;
					
					if(cursor_x+tlength > (msg_w-(oldmargin ? 0 : msg_margins[right]))
					   && ((cursor_x > (msg_w-(oldmargin ? 0 : msg_margins[right])) || !(MsgStrings[msgstr].stringflags & STRINGFLAG_WRAP))
							? true : strcmp(s3," ")!=0))
					{
						int32_t thei = zc_max(ssc_tile_hei, text_height(msgfont));
						ssc_tile_hei = ssc_tile_hei_buf;
						ssc_tile_hei_buf = -1;
						cursor_y += thei + MsgStrings[msgstr].vspace;
						cursor_x=oldmargin ? 0 : msg_margins[left];
					}
					
					cursor_x+=tlength;
					++msgpos;
					++msgptr;
					
					if(atend(MsgStrings[msgstr].s+msgptr))
					{
						if(MsgStrings[msgstr].nextstring)
						{
							if(MsgStrings[MsgStrings[msgstr].nextstring].stringflags & STRINGFLAG_CONT)
							{
								msgstr=MsgStrings[msgstr].nextstring;
								msgpos=msgptr=0;
								msgfont=setmsgfont();
							}
						}
					}
				}
		}
	}
	
	// Done printing the string
	if(!doing_name_insert && (msgpos>=10000 || msgptr>=MSGSIZE || cursor_y >= msg_h-(oldmargin?0:msg_margins[down]) || atend(MsgStrings[msgstr].s+msgptr)) && !linkedmsgclk)
	{
		while(parsemsgcode()) // Finish remaining control codes
			;
			
		// Go to next string, or make it disappear by going to string 0.
		if(MsgStrings[msgstr].nextstring!=0 || get_bit(quest_rules,qr_MSGDISAPPEAR) || enqueued_str)
		{
			linkedmsgclk=51;
		}
		
		if(MsgStrings[msgstr].nextstring==0)
		{
			if(!get_bit(quest_rules,qr_MSGDISAPPEAR))
			{
disappear:
				msg_active = false;
				Link.finishedmsg();
			}
			
			if(repaircharge)
			{
				//       if (get_bit(quest_rules,qr_REPAIRFIX)) {
				//         fixed_door=true;
				//       }
				game->change_drupy(-tmpscr[currscr<128?0:1].catchall);
				repaircharge = 0;
			}
			
			if(adjustmagic)
			{
				if(game->get_magicdrainrate())
					game->set_magicdrainrate(1);
					
				adjustmagic = false;
				sfx(WAV_SCALE);
				setmapflag((currscr < 128 && get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)) ? mITEM : mBELOW);
			}
			
			if(learnslash)
			{
				game->set_canslash(1);
				learnslash = false;
				sfx(WAV_SCALE);
				setmapflag((currscr < 128 && get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)) ? mITEM : mBELOW);
			}
		}
	}
}

int32_t message_more_y()
{
	//Is the flag ticked, do we really want a message more y larger than 160?
	int32_t msgy=zc_min((zinit.msg_more_is_offset==0)?zinit.msg_more_y:zinit.msg_more_y+MsgStrings[msgstr].y ,160);
	msgy+=playing_field_offset;
	return msgy;
}

/***  Collision detection & handling  ***/

void clear_script_one_frame_conditions()
{
	for(int32_t j=0; j<guys.Count(); j++)
	{
				enemy *e = (enemy*)guys.spr(j);
		for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) e->hitby[q] = 0;
	}
}
		
void check_collisions()
{
	bool temp_hit = false;
	for(int32_t i=0; i<Lwpns.Count(); i++)
	{
		weapon *w = (weapon*)Lwpns.spr(i);
		
		if(!(w->Dead()) && w->id!=wSword && w->id!=wHammer && w->id!=wWand)
		{
			for(int32_t j=0; j<guys.Count(); j++)
			{
				enemy *e = (enemy*)guys.spr(j);
		if ( !temp_hit ) e->hitby[HIT_BY_LWEAPON] = 0;
				
				if(e->hit(w)) //boomerangs and such that last for more than a frame can write hitby[] for more than one frame, 
				//because this only checks `if(dying || clk<0 || hclk>0 || superman)`
				{//!(e->stunclk)
					int32_t h = e->takehit(w);
					 if (h == -1) 
			{ 
				e->hitby[HIT_BY_LWEAPON] = i+1; temp_hit = true; 
				e->hitby[HIT_BY_LWEAPON_UID] = w->script_UID;
				//e->hitby[HIT_BY_LWEAPON_FAMILY] = itemsbuf[w->parentid].family; //that would be the itemclass, not the weapon type!
				e->hitby[HIT_BY_LWEAPON_FAMILY] = w->id;
				//al_trace("npc->HitBy[] Parent Item is: %d /n", w->parentitem);
				//al_trace("npc->HitBy[] Parent ID is: %d /n", w->parentid);
				e->hitby[HIT_BY_LWEAPON_LITERAL_ID] = w->parentitem;
				
			}
								  //we may need to handle this in special cases. -Z
				   
			//if h == stun or ignore
			
			//if e->stun > DEFAULT_STUN -1 || !e->stun 
			//if the enemy wasn't stunned this round -- what a bitch, as the stun value is set before we check this
			///! how about: if w->dead != bounce !
			
			// NOT FOR PUBLIC RELEASE
					/*if(h==3) //Mirror shield
					{
					if (w->id==ewFireball || w->id==wRefFireball)
					{
					  w->id=wRefFireball;
					  switch(e->dir)
					  {
						case up:    e->angle += (PI - e->angle) * 2.0;      break;
						case down:  e->angle = -e->angle;                   break;
						case left:  e->angle += ((-PI/2) - e->angle) * 2.0; break;
						case right: e->angle += (( PI/2) - e->angle) * 2.0; break;
						// TODO: the following. -L.
						case l_up:  break;
						case r_up:  break;
						case l_down: break;
						case r_down: break;
					  }
					}
					else
					{
					  w->id = ((w->id==ewMagic || w->id==wRefMagic || w->id==wMagic) ? wRefMagic : wRefBeam);
					  w->dir ^= 1;
					  if(w->dir&2)
						w->flip ^= 1;
					  else
						w->flip ^= 2;
					}
					w->ignoreLink=false;
					}
					else*/
					if(h)
					{
						if(e->switch_hooked && w->family_class == itype_switchhook)
							w->onhit(false, e, -1);
						else w->onhit(false, e, h);
					}
					
					if(h==2)
					{
						break;
					}
				}
				
				if(w->Dead())
				{
					break;
				}
			}
	
		// Item flags added in 2.55:
		// BRang/HShot/Arrows ITEM_FLAG4 is "Pick up anything" (port of qr_BRANGPICKUP)
		// BRang/HShot ITEM_FLAG5 is "Drags Items" (port of qr_Z3BRANG_HSHOT)
		// Arrows ITEM_FLAG2 is "Picks up items" (inverse port of qr_Z3BRANG_HSHOT)
		// -V
		if(w->id == wBrang || w->id == wHookshot || w->id == wArrow)
		{
			if((w->id==wArrow&&itemsbuf[w->parentitem].flags & ITEM_FLAG2)||(w->id!=wArrow&&!(itemsbuf[w->parentitem].flags & ITEM_FLAG5)))//An arrow with "Picks up items" or a BRang/HShot without "Drags items"
			{
				for(int32_t j=0; j<items.Count(); j++)
				{
					if(items.spr(j)->hit(w))
					{
						item *theItem = ((item*)items.spr(j));
						bool priced = theItem->PriceIndex >-1;
						bool isKey = itemsbuf[theItem->id].family==itype_key||itemsbuf[theItem->id].family==itype_lkey;
						if(!theItem->fallclk && !theItem->drownclk && ((theItem->pickup & ipTIMER && theItem->clk2 >= 32)
							|| (((itemsbuf[w->parentitem].flags & ITEM_FLAG4)||(theItem->pickup & ipCANGRAB)||((itemsbuf[w->parentitem].flags & ITEM_FLAG7)&&isKey))&& !priced)))
						{
							if(itemsbuf[theItem->id].collect_script)
							{
								ZScriptVersion::RunScript(SCRIPT_ITEM, itemsbuf[theItem->id].collect_script, theItem->id & 0xFFF);
							}
							
							Link.checkitems(j);
						}
					}
				}
			} else if(w->id!=wArrow){//A BRang/HShot with "Drags Items"
				for(int32_t j=0; j<items.Count(); j++)
				{
					if(items.spr(j)->hit(w))
					{
						item *theItem = ((item*)items.spr(j));
						bool priced = theItem->PriceIndex >-1;
						bool isKey = itemsbuf[theItem->id].family==itype_key||itemsbuf[theItem->id].family==itype_lkey;
						if(!theItem->fallclk && !theItem->drownclk && ((theItem->pickup & ipTIMER && theItem->clk2 >= 32)
							|| (((itemsbuf[w->parentitem].flags & ITEM_FLAG4)||(theItem->pickup & ipCANGRAB)||((itemsbuf[w->parentitem].flags & ITEM_FLAG7)&&isKey)) && !priced && !(theItem->pickup & ipDUMMY))))
						{
							if(w->id == wBrang)
							{
								w->onhit(false);
							}

							if(w->dragging==-1)
							{
								w->dead=1;
								theItem->clk2=256;
								w->dragging=j;
							}
						}
					}
				}
			}
		}
		}
	}
}

void dragging_item()
{
	for(int32_t i=0; i<Lwpns.Count(); i++)
	{
		weapon *w = (weapon*)Lwpns.spr(i);
		
		if((w->id == wBrang || w->id==wHookshot)&&itemsbuf[w->parentitem].flags & ITEM_FLAG5)//ITEM_FLAG5 is a port for qr_Z3BRANG_HSHOT
		{
			if(w->dragging>=0 && w->dragging<items.Count())
			{
				items.spr(w->dragging)->x=w->x;
				items.spr(w->dragging)->y=w->y;
				
				// Drag the Fairy enemy as well as the Fairy item
				int32_t id = items.spr(w->dragging)->id;
				
				if(itemsbuf[id].family ==itype_fairy && itemsbuf[id].misc3)
				{
					movefairynew2(w->x,w->y,*((item*)items.spr(w->dragging)));
				}
			}
		}
	}
}

int32_t more_carried_items()
{
	int32_t hasmorecarries = 0;
	
	for(int32_t i=0; i<items.Count(); i++)
	{
		if(((item*)items.spr(i))->pickup & ipENEMY)
		{
			hasmorecarries++;
		}
	}
	
	return hasmorecarries;
}

// messy code to do the enemy-carrying-the-item thing
void roaming_item()
{
	if(!(hasitem&(4|2)) || !loaded_enemies)
		return;
	
	// All enemies already dead upon entering a room?
	if(guys.Count()==0)
	{
		return;
	}
	
	// Lost track of the carrier?
	if(guycarryingitem<0 || guycarryingitem>=guys.Count() ||
	   !((enemy*)guys.spr(guycarryingitem))->itemguy)
	{
		guycarryingitem=-1;
		for(int32_t j=0; j<guys.Count(); j++)
		{
			if(((enemy*)guys.spr(j))->itemguy)
			{
				guycarryingitem=j;
				break;
			}
		}
	}
	
	if(hasitem&4)
	{
		guycarryingitem = -1;
		
		for(int32_t i=0; i<guys.Count(); i++)
		{
			if(((enemy*)guys.spr(i))->itemguy)
			{
				guycarryingitem = i;
			}
		}
		
		if(guycarryingitem == -1)                                      //This happens when "default enemies" such as
		{
			return;                                               //eSHOOTFBALL are alive but enemies from the list
		}                                                       //are not. Defer to LinkClass::checkspecial().
		
		int32_t Item=tmpscr->item;
		
		hasitem &= ~4;
		
		if((!getmapflag(mITEM) || (tmpscr->flags9&fITEMRETURN)) && (tmpscr->hasitem != 0))
		{
			additem(0,0,Item,ipENEMY+ipONETIME+ipBIGRANGE
					+ (((tmpscr->flags3&fHOLDITEM) || (itemsbuf[Item].family==itype_triforcepiece)) ? ipHOLDUP : 0)
				   );
			hasitem |= 2;
		}
		else
		{
			return;
		}
	}
	
	for(int32_t i=0; i<items.Count(); i++)
	{
		if(((item*)items.spr(i))->pickup&ipENEMY)
		{
			if(get_bit(quest_rules,qr_HIDECARRIEDITEMS))
			{
				items.spr(i)->x = -128; // Awfully inelegant, innit?
				items.spr(i)->y = -128;
			}
			else if(guycarryingitem>=0 && guycarryingitem<guys.Count())
			{
				items.spr(i)->x = guys.spr(guycarryingitem)->x;
				items.spr(i)->y = guys.spr(guycarryingitem)->y - 2;
			}
		}
	}
}

bool enemy::IsBigAnim()
{
	return (anim == a2FRMB || anim == a4FRM8EYEB || anim == a4FRM4EYEB
	|| anim == a4FRM8DIRFB || anim == a4FRM4DIRB || anim == a4FRM4DIRFB
	|| anim == a4FRM8DIRB);
}

const char *old_guy_string[OLDMAXGUYS] =
{
	"(None)","Abei","Ama","Merchant","Moblin","Fire","Fairy","Goriya","Zelda","Abei 2","Empty","","","","","","","","","",
	// 020
	"Octorok (L1, Slow)","Octorok (L2, Slow)","Octorok (L1, Fast)","Octorok (L2, Fast)","Tektite (L1)",
	// 025
	"Tektite (L2)","Leever (L1)","Leever (L2)","Moblin (L1)","Moblin (L2)",
	// 030
	"Lynel (L1)","Lynel (L2)","Peahat (L1)","Zora","Rock",
	// 035
	"Ghini (L1, Normal)","Ghini (L1, Phantom)","Armos","Keese (CSet 7)","Keese (CSet 8)",
	// 040
	"Keese (CSet 9)","Stalfos (L1)","Gel (L1, Normal)","Zol (L1, Normal)","Rope (L1)",
	// 045
	"Goriya (L1)","Goriya (L2)","Trap (4-Way)","Wall Master","Darknut (L1)",
	// 050
	"Darknut (L2)","Bubble (Sword, Temporary Disabling)","Vire (Normal)","Like Like","Gibdo",
	// 055
	"Pols Voice (Arrow)","Wizzrobe (Teleporting)","Wizzrobe (Floating)","Aquamentus (Facing Left)","Moldorm",
	// 060
	"Dodongo","Manhandla (L1)","Gleeok (1 Head)","Gleeok (2 Heads)","Gleeok (3 Heads)",
	// 065
	"Gleeok (4 Heads)","Digdogger (1 Kid)","Digdogger (3 Kids)","Digdogger Kid (1)","Digdogger Kid (2)",
	// 070
	"Digdogger Kid (3)","Digdogger Kid (4)","Gohma (L1)","Gohma (L2)","Lanmola (L1)",
	// 075
	"Lanmola (L2)","Patra (L1, Big Circle)","Patra (L1, Oval)","Ganon","Stalfos (L2)",
	// 080
	"Rope (L2)","Bubble (Sword, Permanent Disabling)","Bubble (Sword, Re-enabling)","Shooter (Fireball)","Item Fairy ",
	// 085
	"Fire","Octorok (Magic)", "Darknut (Death Knight)", "Gel (L1, Tribble)", "Zol (L1, Tribble)",
	// 090
	"Keese (Tribble)", "Vire (Tribble)", "Darknut (Splitting)", "Aquamentus (Facing Right)", "Manhandla (L2)",
	// 095
	"Trap (Horizontal, Line of Sight)", "Trap (Vertical, Line of Sight)", "Trap (Horizontal, Constant)", "Trap (Vertical, Constant)", "Wizzrobe (Fire)",
	// 100
	"Wizzrobe (Wind)", "Ceiling Master ", "Floor Master ", "Patra (BS Zelda)", "Patra (L2)",
	// 105
	"Patra (L3)", "Bat", "Wizzrobe (Bat)", "Wizzrobe (Bat 2) ", "Gleeok (Fire, 1 Head)",
	// 110
	"Gleeok (Fire, 2 Heads)",  "Gleeok (Fire, 3 Heads)","Gleeok (Fire, 4 Heads)", "Wizzrobe (Mirror)", "Dodongo (BS Zelda)",
	// 115
	"Dodongo (Fire) ","Trigger", "Bubble (Item, Temporary Disabling)", "Bubble (Item, Permanent Disabling)", "Bubble (Item, Re-enabling)",
	// 120
	"Stalfos (L3)", "Gohma (L3)", "Gohma (L4)", "NPC 1 (Standing) ", "NPC 2 (Standing) ",
	// 125
	"NPC 3 (Standing) ", "NPC 4 (Standing) ", "NPC 5 (Standing) ", "NPC 6 (Standing) ", "NPC 1 (Walking) ",
	// 130
	"NPC 2 (Walking) ", "NPC 3 (Walking) ", "NPC 4 (Walking) ", "NPC 5 (Walking) ", "NPC 6 (Walking) ",
	// 135
	"Boulder", "Goriya (L3)", "Leever (L3)", "Octorok (L3, Slow)", "Octorok (L3, Fast)",
	// 140
	"Octorok (L4, Slow)", "Octorok (L4, Fast)", "Trap (8-Way) ", "Trap (Diagonal) ", "Trap (/, Constant) ",
	// 145
	"Trap (/, Line of Sight) ", "Trap (\\, Constant) ", "Trap (\\, Line of Sight) ", "Trap (CW, Constant) ", "Trap (CW, Line of Sight) ",
	// 150
	"Trap (CCW, Constant) ", "Trap (CCW, Line of Sight) ", "Wizzrobe (Summoner)", "Wizzrobe (Ice) ", "Shooter (Magic)",
	// 155
	"Shooter (Rock)", "Shooter (Spear)", "Shooter (Sword)", "Shooter (Fire)", "Shooter (Fire 2)",
	// 160
	"Bombchu", "Gel (L2, Normal)", "Zol (L2, Normal)", "Gel (L2, Tribble)", "Zol (L2, Tribble)",
	// 165
	"Tektite (L3) ", "Spinning Tile (Combo)", "Spinning Tile (Enemy Sprite)", "Lynel (L3) ", "Peahat (L2) ",
	// 170
	"Pols Voice (Magic) ", "Pols Voice (Whistle) ", "Darknut (Mirror) ", "Ghini (L2, Fire) ", "Ghini (L2, Magic) ",
	// 175
	"Grappler Bug (HP) ", "Grappler Bug (MP) "
};

/*** end of guys.cc ***/

