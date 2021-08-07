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

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include <string.h>

#include "weapons.h"
#include "guys.h"
#include "zelda.h"
#include "zsys.h"
#include "maps.h"
#include "tiles.h"
#include "pal.h"
#include "link.h"
#include "mem_debug.h"
#include "ffscript.h"
#include "decorations.h"

extern LinkClass Link;
extern zinitdata zinit;
extern int directWpn;
extern FFScript FFCore;
extern ZModule zcm;
extern enemy Enemy;
extern byte epilepsyFlashReduction;

static double WrapAngle( double radians ) 
{
	while (radians <= -PI) radians += (PI*2);
	while (radians > PI) radians -= (PI*2);
	return radians;
}

static double DegreesToRadians(double d)
{
	double dvs = PI/180.0;
	return d*dvs;
	
}

static void weapon_triggersecret(int pos, int flag)
{
	mapscr *s = tmpscr;
	int ft=0, checkflag; //Flag trigger, checked flag temp. 
	bool putit = true;  //Is set false with a mismatch (illegal value input).
	//Convert a flag type to a secret type. -Z
	switch(flag)
	{
		case mfBCANDLE:
			ft=sBCANDLE;
			break;
			
		case mfRCANDLE:
			ft=sRCANDLE;
			break;
			
		case mfWANDFIRE:
			ft=sWANDFIRE;
			break;
			
		case mfDINSFIRE:
			ft=sDINSFIRE;
			break;
			
		case mfARROW:
			ft=sARROW;
			break;
			
		case mfSARROW:
			ft=sSARROW;
			break;
			
		case mfGARROW:
			ft=sGARROW;
			break;
			
		case mfSBOMB:
			ft=sSBOMB;
			break;
			
		case mfBOMB:
			ft=sBOMB;
			break;
			
		case mfBRANG:
			ft=sBRANG;
			break;
			
		case mfMBRANG:
			ft=sMBRANG;
			break;
			
		case mfFBRANG:
			ft=sFBRANG;
			break;
			
		case mfWANDMAGIC:
			ft=sWANDMAGIC;
			break;
			
		case mfREFMAGIC:
			ft=sREFMAGIC;
			break;
			
		case mfREFFIREBALL:
			ft=sREFFIREBALL;
			break;
			
		case mfSWORD:
			ft=sSWORD;
			break;
			
		case mfWSWORD:
			ft=sWSWORD;
			break;
			
		case mfMSWORD:
			ft=sMSWORD;
			break;
			
		case mfXSWORD:
			ft=sXSWORD;
			break;
			
		case mfSWORDBEAM:
			ft=sSWORDBEAM;
			break;
			
		case mfWSWORDBEAM:
			ft=sWSWORDBEAM;
			break;
			
		case mfMSWORDBEAM:
			ft=sMSWORDBEAM;
			break;
			
		case mfXSWORDBEAM:
			ft=sXSWORDBEAM;
			break;
			
		case mfHOOKSHOT:
			ft=sHOOKSHOT;
			break;
			
		case mfWAND:
			ft=sWAND;
			break;
			
		case mfHAMMER:
			ft=sHAMMER;
			break;
			
		case mfSTRIKE:
			ft=sSTRIKE;
			break;
			
		default:
			putit = false;
			break;
	}
	if ( putit )
	{		
		for(int iter=0; iter<2; ++iter)
		{
			//for ( int pos = 0; pos < 176; pos++ ) 
			//{		
				if(iter==1) checkflag=s->sflag[pos]; //Placed
				else checkflag=combobuf[s->data[pos]].flag; //Inherent
				Z_message("checkflag is: %d\n", checkflag);
				al_trace("checkflag is: %d\n", checkflag);
				
				Z_message("flag is: %d\n", flag);
				al_trace("flag is: %d\n", flag);
				//cmbx = COMBOX(pos);
				////cmby = COMBOY(pos);
				
				//Placed flags
				if ( iter == 1 )
				{
					if ( s->sflag[pos] == flag ) {
						screen_combo_modify_preroutine(s,pos);
						s->data[pos] = s->secretcombo[ft];
						s->cset[pos] = s->secretcset[ft];
						s->sflag[pos] = s->secretflag[ft];
						// newflag = s->secretflag[ft];
						screen_combo_modify_postroutine(s,pos);
					}
				}
				//Inherent flags
				else
				{
					if ( combobuf[s->data[pos]].flag == flag ) {
						screen_combo_modify_preroutine(s,pos);
						s->data[pos] = s->secretcombo[ft];
						s->cset[pos] = s->secretcset[ft];
						//s->sflag[pos] = s->secretflag[ft];
						screen_combo_modify_postroutine(s,pos);
					}
					
				}
			//}
		}
	}
	
}

static void MatchComboTrigger(weapon *w, int bx, int by, newcombo *c/*, int comboid, int flag*/)
{
	if(screenIsScrolling()) return;
	int wid = (w->useweapon > 0) ? w->useweapon : w->id;
	int cid = MAPCOMBO(bx,by);
	int flag = MAPFLAG(bx,by);
	int flag2 = MAPCOMBOFLAG(bx,by);
	int ft = 0;
	int scombo=COMBOPOS(bx,by);
	bool single16 = false;
	switch(flag)
	{
		
		case mfSWORDBEAM:
		case mfWSWORDBEAM:
		case mfMSWORDBEAM:
		case mfXSWORDBEAM:
		case mfBCANDLE:
		case mfRCANDLE:
		case mfDINSFIRE:
		case mfSTRIKE:
		case mfARROW:
		case mfSARROW:
		case mfGARROW:
		case mfBRANG:
		case mfMBRANG:
		case mfFBRANG:
		case mfWANDMAGIC:
		case mfREFMAGIC:
		case mfREFFIREBALL:
		case mfHAMMER:
		case mfWAND:
		{
			ft = flag;
			
			if ( wid == wSword && ( c[cid].triggerflags[0]&combotriggerSWORD ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wBeam && ( c[cid].triggerflags[0]&combotriggerSWORDBEAM ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wBrang && ( c[cid].triggerflags[0]&combotriggerBRANG ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wBomb && ( c[cid].triggerflags[0]&combotriggerBOMB ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wSBomb && ( c[cid].triggerflags[0]&combotriggerSBOMB ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wLitBomb && ( c[cid].triggerflags[0]&combotriggerLITBOMB ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wLitSBomb && ( c[cid].triggerflags[0]&combotriggerLITSBOMB ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wArrow && ( c[cid].triggerflags[0]&combotriggerARROW ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wFire && ( c[cid].triggerflags[0]&combotriggerFIRE ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wWhistle && ( c[cid].triggerflags[0]&combotriggerWHISTLE ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wBait && ( c[cid].triggerflags[0]&combotriggerBAIT ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wWand && ( c[cid].triggerflags[0]&combotriggerWAND ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wMagic && ( c[cid].triggerflags[0]&combotriggerMAGIC ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wWind && ( c[cid].triggerflags[0]&combotriggerWIND ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wRefMagic && ( c[cid].triggerflags[0]&combotriggerREFMAGIC ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wRefFireball && ( c[cid].triggerflags[0]&combotriggerREFFIREBALL ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wRefRock && ( c[cid].triggerflags[0]&combotriggerREFROCK ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wHammer && ( c[cid].triggerflags[0]&combotriggerHAMMER ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			    //ZScript liter support ends here. 
			
			else if ( wid == wHookshot && ( c[cid].triggerflags[1]&combotriggerHOOKSHOT ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			
			else if ( wid == wFSparkle && ( c[cid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			else if ( wid == wSSparkle && ( c[cid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wCByrna && ( c[cid].triggerflags[1]&combotriggerBYRNA ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wRefBeam && ( c[cid].triggerflags[1]&combotriggerREFBEAM ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wStomp && ( c[cid].triggerflags[1]&combotriggerSTOMP ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			    
			    //item trigger flags page 2
			
			else if ( wid == wScript1 && ( c[cid].triggerflags[1]&combotriggerSCRIPT01 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wScript2 && ( c[cid].triggerflags[1]&combotriggerSCRIPT02 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wScript3 && ( c[cid].triggerflags[1]&combotriggerSCRIPT03 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wScript4 && ( c[cid].triggerflags[1]&combotriggerSCRIPT04 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wScript5 && ( c[cid].triggerflags[1]&combotriggerSCRIPT05 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wScript6 && ( c[cid].triggerflags[1]&combotriggerSCRIPT06 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wScript7 && ( c[cid].triggerflags[1]&combotriggerSCRIPT07 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wScript8 && ( c[cid].triggerflags[1]&combotriggerSCRIPT08 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wScript9 && ( c[cid].triggerflags[1]&combotriggerSCRIPT09 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
		
			else if ( wid == wScript10 && ( c[cid].triggerflags[1]&combotriggerSCRIPT10 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			break;
		
		}
		
		
		
		
		//secrets 1 to 16
		case mfSINGLE:
		case mfSINGLE16:
		{
			single16 = true;
		}
		case mfSECRETS01:
		case mfSECRETS02:
		case mfSECRETS03:
		case mfSECRETS04:
		case mfSECRETS05:
		case mfSECRETS06:
		case mfSECRETS07:
		case mfSECRETS08:
		case mfSECRETS09:
		case mfSECRETS10:
		case mfSECRETS11:
		case mfSECRETS12:
		case mfSECRETS13:
		case mfSECRETS14:
		case mfSECRETS15:
		case mfSECRETS16:
		{
			ft = flag;
			
			if ( wid == wSword && ( c[cid].triggerflags[0]&combotriggerSWORD ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wBeam && ( c[cid].triggerflags[0]&combotriggerSWORDBEAM ) && ( w->type >= c[cid].triggerlevel ) )   
				{ (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wBrang && ( c[cid].triggerflags[0]&combotriggerBRANG ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wBomb && ( c[cid].triggerflags[0]&combotriggerBOMB ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wSBomb && ( c[cid].triggerflags[0]&combotriggerSBOMB ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wLitBomb && ( c[cid].triggerflags[0]&combotriggerLITBOMB ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wLitSBomb && ( c[cid].triggerflags[0]&combotriggerLITSBOMB ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wArrow && ( c[cid].triggerflags[0]&combotriggerARROW ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wFire && ( c[cid].triggerflags[0]&combotriggerFIRE ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wWhistle && ( c[cid].triggerflags[0]&combotriggerWHISTLE ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wBait && ( c[cid].triggerflags[0]&combotriggerBAIT ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wWand && ( c[cid].triggerflags[0]&combotriggerWAND ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wMagic && ( c[cid].triggerflags[0]&combotriggerMAGIC ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wWind && ( c[cid].triggerflags[0]&combotriggerWIND ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wRefMagic && ( c[cid].triggerflags[0]&combotriggerREFMAGIC ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wRefFireball && ( c[cid].triggerflags[0]&combotriggerREFFIREBALL ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wRefRock && ( c[cid].triggerflags[0]&combotriggerREFROCK ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wHammer && ( c[cid].triggerflags[0]&combotriggerHAMMER ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			    //ZScript liter support ends here. 
			
			else if ( wid == wHookshot && ( c[cid].triggerflags[1]&combotriggerHOOKSHOT ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			
			else if ( wid == wFSparkle && ( c[cid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			else if ( wid == wSSparkle && ( c[cid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wCByrna && ( c[cid].triggerflags[1]&combotriggerBYRNA ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wRefBeam && ( c[cid].triggerflags[1]&combotriggerREFBEAM ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wStomp && ( c[cid].triggerflags[1]&combotriggerSTOMP ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			    
			    //item trigger flags page 2
			
			else if ( wid == wScript1 && ( c[cid].triggerflags[1]&combotriggerSCRIPT01 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript2 && ( c[cid].triggerflags[1]&combotriggerSCRIPT02 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript3 && ( c[cid].triggerflags[1]&combotriggerSCRIPT03 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript4 && ( c[cid].triggerflags[1]&combotriggerSCRIPT04 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript5 && ( c[cid].triggerflags[1]&combotriggerSCRIPT05 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript6 && ( c[cid].triggerflags[1]&combotriggerSCRIPT06 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript7 && ( c[cid].triggerflags[1]&combotriggerSCRIPT07 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript8 && ( c[cid].triggerflags[1]&combotriggerSCRIPT08 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript9 && ( c[cid].triggerflags[1]&combotriggerSCRIPT09 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
		
			else if ( wid == wScript10 && ( c[cid].triggerflags[1]&combotriggerSCRIPT10 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			if ( wid == wSword && ( c[cid].triggerflags[0]&combotriggerSWORD ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wBeam && ( c[cid].triggerflags[0]&combotriggerSWORDBEAM ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wBrang && ( c[cid].triggerflags[0]&combotriggerBRANG ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wBomb && ( c[cid].triggerflags[0]&combotriggerBOMB ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wSBomb && ( c[cid].triggerflags[0]&combotriggerSBOMB ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wLitBomb && ( c[cid].triggerflags[0]&combotriggerLITBOMB ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wLitSBomb && ( c[cid].triggerflags[0]&combotriggerLITSBOMB ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wArrow && ( c[cid].triggerflags[0]&combotriggerARROW ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wFire && ( c[cid].triggerflags[0]&combotriggerFIRE ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wWhistle && ( c[cid].triggerflags[0]&combotriggerWHISTLE ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wBait && ( c[cid].triggerflags[0]&combotriggerBAIT ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wWand && ( c[cid].triggerflags[0]&combotriggerWAND ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wMagic && ( c[cid].triggerflags[0]&combotriggerMAGIC ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wWind && ( c[cid].triggerflags[0]&combotriggerWIND ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wRefMagic && ( c[cid].triggerflags[0]&combotriggerREFMAGIC ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wRefFireball && ( c[cid].triggerflags[0]&combotriggerREFFIREBALL ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wRefRock && ( c[cid].triggerflags[0]&combotriggerREFROCK ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wHammer && ( c[cid].triggerflags[0]&combotriggerHAMMER ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			    //ZScript liter support ends here. 
			
			else if ( wid == wHookshot && ( c[cid].triggerflags[1]&combotriggerHOOKSHOT ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			
			else if ( wid == wFSparkle && ( c[cid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			else if ( wid == wSSparkle && ( c[cid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wCByrna && ( c[cid].triggerflags[1]&combotriggerBYRNA ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wRefBeam && ( c[cid].triggerflags[1]&combotriggerREFBEAM ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wStomp && ( c[cid].triggerflags[1]&combotriggerSTOMP ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			    
			    //item trigger flags page 2
			
			else if ( wid == wScript1 && ( c[cid].triggerflags[1]&combotriggerSCRIPT01 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript2 && ( c[cid].triggerflags[1]&combotriggerSCRIPT02 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript3 && ( c[cid].triggerflags[1]&combotriggerSCRIPT03 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript4 && ( c[cid].triggerflags[1]&combotriggerSCRIPT04 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript5 && ( c[cid].triggerflags[1]&combotriggerSCRIPT05 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript6 && ( c[cid].triggerflags[1]&combotriggerSCRIPT06 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript7 && ( c[cid].triggerflags[1]&combotriggerSCRIPT07 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript8 && ( c[cid].triggerflags[1]&combotriggerSCRIPT08 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript9 && ( c[cid].triggerflags[1]&combotriggerSCRIPT09 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
		
			else if ( wid == wScript10 && ( c[cid].triggerflags[1]&combotriggerSCRIPT10 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			break;
		
		}
		
		case mfBOMB:
		case mfSBOMB:
		{
			ft = flag2;
			
			if ( wid == wSword && ( c[cid].triggerflags[0]&combotriggerSWORD ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wBeam && ( c[cid].triggerflags[0]&combotriggerSWORDBEAM ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wBrang && ( c[cid].triggerflags[0]&combotriggerBRANG ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wBomb && ( c[cid].triggerflags[0]&combotriggerBOMB ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wSBomb && ( c[cid].triggerflags[0]&combotriggerSBOMB ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wLitBomb && ( c[cid].triggerflags[0]&combotriggerLITBOMB ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wLitSBomb && ( c[cid].triggerflags[0]&combotriggerLITSBOMB ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wArrow && ( c[cid].triggerflags[0]&combotriggerARROW ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wFire && ( c[cid].triggerflags[0]&combotriggerFIRE ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wWhistle && ( c[cid].triggerflags[0]&combotriggerWHISTLE ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wBait && ( c[cid].triggerflags[0]&combotriggerBAIT ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wWand && ( c[cid].triggerflags[0]&combotriggerWAND ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wMagic && ( c[cid].triggerflags[0]&combotriggerMAGIC ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wWind && ( c[cid].triggerflags[0]&combotriggerWIND ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wRefMagic && ( c[cid].triggerflags[0]&combotriggerREFMAGIC ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wRefFireball && ( c[cid].triggerflags[0]&combotriggerREFFIREBALL ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wRefRock && ( c[cid].triggerflags[0]&combotriggerREFROCK ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wHammer && ( c[cid].triggerflags[0]&combotriggerHAMMER ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			    //ZScript liter support ends here. 
			
			else if ( wid == wHookshot && ( c[cid].triggerflags[1]&combotriggerHOOKSHOT ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			
			else if ( wid == wFSparkle && ( c[cid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			else if ( wid == wSSparkle && ( c[cid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wCByrna && ( c[cid].triggerflags[1]&combotriggerBYRNA ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wRefBeam && ( c[cid].triggerflags[1]&combotriggerREFBEAM ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wStomp && ( c[cid].triggerflags[1]&combotriggerSTOMP ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			    
			    //item trigger flags page 2
			
			else if ( wid == wScript1 && ( c[cid].triggerflags[1]&combotriggerSCRIPT01 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wScript2 && ( c[cid].triggerflags[1]&combotriggerSCRIPT02 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wScript3 && ( c[cid].triggerflags[1]&combotriggerSCRIPT03 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wScript4 && ( c[cid].triggerflags[1]&combotriggerSCRIPT04 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wScript5 && ( c[cid].triggerflags[1]&combotriggerSCRIPT05 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wScript6 && ( c[cid].triggerflags[1]&combotriggerSCRIPT06 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wScript7 && ( c[cid].triggerflags[1]&combotriggerSCRIPT07 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wScript8 && ( c[cid].triggerflags[1]&combotriggerSCRIPT08 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wScript9 && ( c[cid].triggerflags[1]&combotriggerSCRIPT09 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
		
			else if ( wid == wScript10 && ( c[cid].triggerflags[1]&combotriggerSCRIPT10 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			break;
			
		}
	}
	switch(flag2)
	{
		
		case mfSWORDBEAM:
		case mfWSWORDBEAM:
		case mfMSWORDBEAM:
		case mfXSWORDBEAM:
		case mfBCANDLE:
		case mfRCANDLE:
		case mfDINSFIRE:
		case mfSTRIKE:
		case mfARROW:
		case mfSARROW:
		case mfGARROW:
		case mfBRANG:
		case mfMBRANG:
		case mfFBRANG:
		case mfWANDMAGIC:
		case mfREFMAGIC:
		case mfREFFIREBALL:
		case mfHAMMER:
		case mfWAND:
		{
			ft = flag;
			
			if ( wid == wSword && ( c[cid].triggerflags[0]&combotriggerSWORD ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wBeam && ( c[cid].triggerflags[0]&combotriggerSWORDBEAM ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wBrang && ( c[cid].triggerflags[0]&combotriggerBRANG ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wBomb && ( c[cid].triggerflags[0]&combotriggerBOMB ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wSBomb && ( c[cid].triggerflags[0]&combotriggerSBOMB ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wLitBomb && ( c[cid].triggerflags[0]&combotriggerLITBOMB ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wLitSBomb && ( c[cid].triggerflags[0]&combotriggerLITSBOMB ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wArrow && ( c[cid].triggerflags[0]&combotriggerARROW ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wFire && ( c[cid].triggerflags[0]&combotriggerFIRE ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wWhistle && ( c[cid].triggerflags[0]&combotriggerWHISTLE ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wBait && ( c[cid].triggerflags[0]&combotriggerBAIT ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wWand && ( c[cid].triggerflags[0]&combotriggerWAND ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wMagic && ( c[cid].triggerflags[0]&combotriggerMAGIC ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wWind && ( c[cid].triggerflags[0]&combotriggerWIND ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wRefMagic && ( c[cid].triggerflags[0]&combotriggerREFMAGIC ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wRefFireball && ( c[cid].triggerflags[0]&combotriggerREFFIREBALL ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wRefRock && ( c[cid].triggerflags[0]&combotriggerREFROCK ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wHammer && ( c[cid].triggerflags[0]&combotriggerHAMMER ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			    //ZScript liter support ends here. 
			
			else if ( wid == wHookshot && ( c[cid].triggerflags[1]&combotriggerHOOKSHOT ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			
			else if ( wid == wFSparkle && ( c[cid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			else if ( wid == wSSparkle && ( c[cid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wCByrna && ( c[cid].triggerflags[1]&combotriggerBYRNA ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wRefBeam && ( c[cid].triggerflags[1]&combotriggerREFBEAM ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wStomp && ( c[cid].triggerflags[1]&combotriggerSTOMP ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			    
			    //item trigger flags page 2
			
			else if ( wid == wScript1 && ( c[cid].triggerflags[1]&combotriggerSCRIPT01 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wScript2 && ( c[cid].triggerflags[1]&combotriggerSCRIPT02 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wScript3 && ( c[cid].triggerflags[1]&combotriggerSCRIPT03 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wScript4 && ( c[cid].triggerflags[1]&combotriggerSCRIPT04 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wScript5 && ( c[cid].triggerflags[1]&combotriggerSCRIPT05 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wScript6 && ( c[cid].triggerflags[1]&combotriggerSCRIPT06 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wScript7 && ( c[cid].triggerflags[1]&combotriggerSCRIPT07 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wScript8 && ( c[cid].triggerflags[1]&combotriggerSCRIPT08 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			
			else if ( wid == wScript9 && ( c[cid].triggerflags[1]&combotriggerSCRIPT09 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
		
			else if ( wid == wScript10 && ( c[cid].triggerflags[1]&combotriggerSCRIPT10 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by,ft,true));
			break;
		
		}
		
		
		
		
		//secrets 1 to 16
		case mfSINGLE:
		case mfSINGLE16:
		{
			single16 = true;
		}
		case mfSECRETS01:
		case mfSECRETS02:
		case mfSECRETS03:
		case mfSECRETS04:
		case mfSECRETS05:
		case mfSECRETS06:
		case mfSECRETS07:
		case mfSECRETS08:
		case mfSECRETS09:
		case mfSECRETS10:
		case mfSECRETS11:
		case mfSECRETS12:
		case mfSECRETS13:
		case mfSECRETS14:
		case mfSECRETS15:
		case mfSECRETS16:
		{
			ft = flag;
			
			if ( wid == wSword && ( c[cid].triggerflags[0]&combotriggerSWORD ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wBeam && ( c[cid].triggerflags[0]&combotriggerSWORDBEAM ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wBrang && ( c[cid].triggerflags[0]&combotriggerBRANG ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wBomb && ( c[cid].triggerflags[0]&combotriggerBOMB ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wSBomb && ( c[cid].triggerflags[0]&combotriggerSBOMB ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wLitBomb && ( c[cid].triggerflags[0]&combotriggerLITBOMB ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wLitSBomb && ( c[cid].triggerflags[0]&combotriggerLITSBOMB ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wArrow && ( c[cid].triggerflags[0]&combotriggerARROW ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wFire && ( c[cid].triggerflags[0]&combotriggerFIRE ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wWhistle && ( c[cid].triggerflags[0]&combotriggerWHISTLE ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wBait && ( c[cid].triggerflags[0]&combotriggerBAIT ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wWand && ( c[cid].triggerflags[0]&combotriggerWAND ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wMagic && ( c[cid].triggerflags[0]&combotriggerMAGIC ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wWind && ( c[cid].triggerflags[0]&combotriggerWIND ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wRefMagic && ( c[cid].triggerflags[0]&combotriggerREFMAGIC ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wRefFireball && ( c[cid].triggerflags[0]&combotriggerREFFIREBALL ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wRefRock && ( c[cid].triggerflags[0]&combotriggerREFROCK ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wHammer && ( c[cid].triggerflags[0]&combotriggerHAMMER ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			    //ZScript liter support ends here. 
			
			else if ( wid == wHookshot && ( c[cid].triggerflags[1]&combotriggerHOOKSHOT ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			
			else if ( wid == wFSparkle && ( c[cid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			else if ( wid == wSSparkle && ( c[cid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wCByrna && ( c[cid].triggerflags[1]&combotriggerBYRNA ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wRefBeam && ( c[cid].triggerflags[1]&combotriggerREFBEAM ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wStomp && ( c[cid].triggerflags[1]&combotriggerSTOMP ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			    
			    //item trigger flags page 2
			
			else if ( wid == wScript1 && ( c[cid].triggerflags[1]&combotriggerSCRIPT01 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript2 && ( c[cid].triggerflags[1]&combotriggerSCRIPT02 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript3 && ( c[cid].triggerflags[1]&combotriggerSCRIPT03 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript4 && ( c[cid].triggerflags[1]&combotriggerSCRIPT04 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript5 && ( c[cid].triggerflags[1]&combotriggerSCRIPT05 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript6 && ( c[cid].triggerflags[1]&combotriggerSCRIPT06 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript7 && ( c[cid].triggerflags[1]&combotriggerSCRIPT07 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript8 && ( c[cid].triggerflags[1]&combotriggerSCRIPT08 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript9 && ( c[cid].triggerflags[1]&combotriggerSCRIPT09 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
		
			else if ( wid == wScript10 && ( c[cid].triggerflags[1]&combotriggerSCRIPT10 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			if ( wid == wSword && ( c[cid].triggerflags[0]&combotriggerSWORD ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wBeam && ( c[cid].triggerflags[0]&combotriggerSWORDBEAM ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wBrang && ( c[cid].triggerflags[0]&combotriggerBRANG ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wBomb && ( c[cid].triggerflags[0]&combotriggerBOMB ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wSBomb && ( c[cid].triggerflags[0]&combotriggerSBOMB ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wLitBomb && ( c[cid].triggerflags[0]&combotriggerLITBOMB ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wLitSBomb && ( c[cid].triggerflags[0]&combotriggerLITSBOMB ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wArrow && ( c[cid].triggerflags[0]&combotriggerARROW ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wFire && ( c[cid].triggerflags[0]&combotriggerFIRE ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wWhistle && ( c[cid].triggerflags[0]&combotriggerWHISTLE ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wBait && ( c[cid].triggerflags[0]&combotriggerBAIT ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wWand && ( c[cid].triggerflags[0]&combotriggerWAND ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wMagic && ( c[cid].triggerflags[0]&combotriggerMAGIC ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wWind && ( c[cid].triggerflags[0]&combotriggerWIND ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wRefMagic && ( c[cid].triggerflags[0]&combotriggerREFMAGIC ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wRefFireball && ( c[cid].triggerflags[0]&combotriggerREFFIREBALL ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wRefRock && ( c[cid].triggerflags[0]&combotriggerREFROCK ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wHammer && ( c[cid].triggerflags[0]&combotriggerHAMMER ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			    //ZScript liter support ends here. 
			
			else if ( wid == wHookshot && ( c[cid].triggerflags[1]&combotriggerHOOKSHOT ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			
			else if ( wid == wFSparkle && ( c[cid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			else if ( wid == wSSparkle && ( c[cid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wCByrna && ( c[cid].triggerflags[1]&combotriggerBYRNA ) && ( w->type >= c[cid].triggerlevel ) )   
				{ (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wRefBeam && ( c[cid].triggerflags[1]&combotriggerREFBEAM ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wStomp && ( c[cid].triggerflags[1]&combotriggerSTOMP ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			    
			    //item trigger flags page 2
			
			else if ( wid == wScript1 && ( c[cid].triggerflags[1]&combotriggerSCRIPT01 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript2 && ( c[cid].triggerflags[1]&combotriggerSCRIPT02 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript3 && ( c[cid].triggerflags[1]&combotriggerSCRIPT03 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript4 && ( c[cid].triggerflags[1]&combotriggerSCRIPT04 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript5 && ( c[cid].triggerflags[1]&combotriggerSCRIPT05 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript6 && ( c[cid].triggerflags[1]&combotriggerSCRIPT06 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript7 && ( c[cid].triggerflags[1]&combotriggerSCRIPT07 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript8 && ( c[cid].triggerflags[1]&combotriggerSCRIPT08 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			
			else if ( wid == wScript9 && ( c[cid].triggerflags[1]&combotriggerSCRIPT09 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
		
			else if ( wid == wScript10 && ( c[cid].triggerflags[1]&combotriggerSCRIPT10 ) && ( w->type >= c[cid].triggerlevel ) )   { (hidden_entrance(0,true,single16,scombo)); weapon_triggersecret(scombo, ft); }
			break;
		
		}
		
		case mfBOMB:
		case mfSBOMB:
		{
			ft = flag2;
			
			if ( wid == wSword && ( c[cid].triggerflags[0]&combotriggerSWORD ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wBeam && ( c[cid].triggerflags[0]&combotriggerSWORDBEAM ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wBrang && ( c[cid].triggerflags[0]&combotriggerBRANG ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wBomb && ( c[cid].triggerflags[0]&combotriggerBOMB ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wSBomb && ( c[cid].triggerflags[0]&combotriggerSBOMB ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wLitBomb && ( c[cid].triggerflags[0]&combotriggerLITBOMB ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wLitSBomb && ( c[cid].triggerflags[0]&combotriggerLITSBOMB ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wArrow && ( c[cid].triggerflags[0]&combotriggerARROW ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wFire && ( c[cid].triggerflags[0]&combotriggerFIRE ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wWhistle && ( c[cid].triggerflags[0]&combotriggerWHISTLE ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wBait && ( c[cid].triggerflags[0]&combotriggerBAIT ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wWand && ( c[cid].triggerflags[0]&combotriggerWAND ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wMagic && ( c[cid].triggerflags[0]&combotriggerMAGIC ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wWind && ( c[cid].triggerflags[0]&combotriggerWIND ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wRefMagic && ( c[cid].triggerflags[0]&combotriggerREFMAGIC ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wRefFireball && ( c[cid].triggerflags[0]&combotriggerREFFIREBALL ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wRefRock && ( c[cid].triggerflags[0]&combotriggerREFROCK ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wHammer && ( c[cid].triggerflags[0]&combotriggerHAMMER ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			    //ZScript liter support ends here. 
			
			else if ( wid == wHookshot && ( c[cid].triggerflags[1]&combotriggerHOOKSHOT ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			
			else if ( wid == wFSparkle && ( c[cid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			else if ( wid == wSSparkle && ( c[cid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wCByrna && ( c[cid].triggerflags[1]&combotriggerBYRNA ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wRefBeam && ( c[cid].triggerflags[1]&combotriggerREFBEAM ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wStomp && ( c[cid].triggerflags[1]&combotriggerSTOMP ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			    
			    //item trigger flags page 2
			
			else if ( wid == wScript1 && ( c[cid].triggerflags[1]&combotriggerSCRIPT01 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wScript2 && ( c[cid].triggerflags[1]&combotriggerSCRIPT02 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wScript3 && ( c[cid].triggerflags[1]&combotriggerSCRIPT03 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wScript4 && ( c[cid].triggerflags[1]&combotriggerSCRIPT04 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wScript5 && ( c[cid].triggerflags[1]&combotriggerSCRIPT05 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wScript6 && ( c[cid].triggerflags[1]&combotriggerSCRIPT06 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wScript7 && ( c[cid].triggerflags[1]&combotriggerSCRIPT07 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wScript8 && ( c[cid].triggerflags[1]&combotriggerSCRIPT08 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			
			else if ( wid == wScript9 && ( c[cid].triggerflags[1]&combotriggerSCRIPT09 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
		
			else if ( wid == wScript10 && ( c[cid].triggerflags[1]&combotriggerSCRIPT10 ) && ( w->type >= c[cid].triggerlevel ) )   (findentrance(bx,by+(isSideViewGravity()?2:-3),ft,true));
			break;
			
		}
	}
	
}

static int MatchComboTrigger(weapon *w, newcombo *c, int comboid)
{
	if(screenIsScrolling()) return 0;
	int wid = (w->useweapon > 0) ? w->useweapon : w->id;
	
		if ( ( wid == wSword && c[comboid].triggerflags[0]&combotriggerSWORD ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wBeam && c[comboid].triggerflags[0]&combotriggerSWORDBEAM ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wBrang && c[comboid].triggerflags[0]&combotriggerBRANG ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wBomb && c[comboid].triggerflags[0]&combotriggerBOMB ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wSBomb && c[comboid].triggerflags[0]&combotriggerSBOMB ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wLitBomb && c[comboid].triggerflags[0]&combotriggerLITBOMB ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wLitSBomb && c[comboid].triggerflags[0]&combotriggerLITSBOMB ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wArrow && c[comboid].triggerflags[0]&combotriggerARROW ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wFire && c[comboid].triggerflags[0]&combotriggerFIRE ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wWhistle && c[comboid].triggerflags[0]&combotriggerWHISTLE ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wBait && c[comboid].triggerflags[0]&combotriggerBAIT ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wWand && c[comboid].triggerflags[0]&combotriggerWAND ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wMagic && c[comboid].triggerflags[0]&combotriggerMAGIC ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wWind && c[comboid].triggerflags[0]&combotriggerWIND ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wRefMagic && c[comboid].triggerflags[0]&combotriggerREFMAGIC ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wRefFireball && c[comboid].triggerflags[0]&combotriggerREFFIREBALL ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wRefRock && c[comboid].triggerflags[0]&combotriggerREFROCK ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wHammer && c[comboid].triggerflags[0]&combotriggerHAMMER ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		    //ZScript liter support ends here. 
		
		else if ( ( wid == wHookshot && c[comboid].triggerflags[1]&combotriggerHOOKSHOT ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		
		else if ( ( wid == wFSparkle && c[comboid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		else if ( ( wid == wSSparkle && c[comboid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wCByrna && c[comboid].triggerflags[1]&combotriggerBYRNA ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wRefBeam && c[comboid].triggerflags[1]&combotriggerREFBEAM ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wStomp && c[comboid].triggerflags[1]&combotriggerSTOMP ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		    
		    //item trigger flags page 2
		
		else if ( ( wid == wScript1 && c[comboid].triggerflags[1]&combotriggerSCRIPT01 ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wScript2 && c[comboid].triggerflags[1]&combotriggerSCRIPT02 ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wScript3 && c[comboid].triggerflags[1]&combotriggerSCRIPT03 ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wScript4 && c[comboid].triggerflags[1]&combotriggerSCRIPT04 ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wScript5 && c[comboid].triggerflags[1]&combotriggerSCRIPT05 ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wScript6 && c[comboid].triggerflags[1]&combotriggerSCRIPT06 ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wScript7 && c[comboid].triggerflags[1]&combotriggerSCRIPT07 ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wScript8 && c[comboid].triggerflags[1]&combotriggerSCRIPT08 ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wScript9 && c[comboid].triggerflags[1]&combotriggerSCRIPT09 ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else if ( ( wid == wScript10 && c[comboid].triggerflags[1]&combotriggerSCRIPT10 ) && ( w->type >= c[comboid].triggerlevel ) )  return 1;
		
		else return 0;
}

static int COMBOAT(int x, int y) 
{
	x = vbound(x,255,0);
	y = vbound(y,175,0);
	return (y & 240)+(x>>4);
}


#define ComboX(pos) ((pos)%16*16)
#define ComboY(pos) ((pos)&0xF0)
#define minSECRET_TYPE 0
#define maxSECRET_TYPE 43
static void do_generic_combo(weapon *w, int bx, int by, newcombo *c, int wid, 
	int cid, int flag, int flag2, int ft, int scombo, bool single16, int layer)

/*
int wid = (w->useweapon > 0) ? w->useweapon : w->id;
	int cid = MAPCOMBO(bx,by);
	int flag = MAPFLAG(bx,by);
	int flag2 = MAPCOMBOFLAG(bx,by);
	int ft = c[cid].attributes[3] / 10000L;
	//if (!ft) return;
	//zprint("ft: %d\n", ft);
	int scombo=COMBOPOS(bx,by);
	bool single16 = false;

*/
{
	if ( c[cid].type < cTRIGGERGENERIC && !(c[cid].usrflags&cflag9 )  )  //Script combos need an 'Engine' flag
	{ 
		//zprint("cGeneric abort on c[cid].type %d\n", c[cid].type); 
		return;
	} 
	//zprint("Generic combo\n ");
	ft = vbound(ft, minSECRET_TYPE, maxSECRET_TYPE); //sanity guard to legal secret types. 44 to 127 are unused
	//zprint("swordbeam\n");
	//zprint("sfx is: %d\n", c[cid].attributes[2] / 10000L);
	//zprint("scombo is: %d\n", scombo);
	if ( !(get_bit(w->wscreengrid,(((bx>>4) + by)))) || (c[cid].usrflags&cflag5) ) 
	{
		if ((c[cid].usrflags&cflag1)) 
		{
			//zprint("Adding decoration, sprite: %d\n", c[cid].attributes[0] / 10000L);
			if ((c[cid].usrflags&cflag10) && ( c[cid].attribytes[0] == 1 ) )
				decorations.add(new dBushLeaves((zfix)ComboX(scombo), (zfix)ComboY(scombo),dBUSHLEAVES, 0, 0));
			if ((c[cid].usrflags&cflag10) && ( c[cid].attribytes[0] == 2 ) )
				decorations.add(new dFlowerClippings((zfix)ComboX(scombo), (zfix)ComboY(scombo),dFLOWERCLIPPINGS, 0, 0));
			if ((c[cid].usrflags&cflag10) && ( c[cid].attribytes[0] == 3 ) )
				decorations.add(new dGrassClippings((zfix)ComboX(scombo), (zfix)ComboY(scombo), dGRASSCLIPPINGS, 0, 0));
			else if ( c[cid].attribytes[0] > 0 )
				decorations.add(new comboSprite((zfix)ComboX(scombo), (zfix)ComboY(scombo), 0, 0, c[cid].attribytes[0]));
		}
		int it = -1; 
		if ( (c[cid].usrflags&cflag2) )
		{
			
			if ( c[cid].usrflags&cflag11 ) //specific item
			{
				it = c[cid].attribytes[1];
			}
			else it = select_dropitem(c[cid].attribytes[1]); 
			
		}
		//zprint("dropset: %d\n", c[cid].attributes[1] / 10000L);
		//zprint("drops enabled: %s\n", (c[cid].usrflags&cflag2) ? "true":"false");
		//zprint("it: %d\n", it);
		if( it != -1 )
		{
			items.add(new item((zfix)ComboX(scombo), (zfix)ComboY(scombo),(zfix)0, it, ipBIGRANGE + ipTIMER, 0));
		}
		//drop special room item

		if ( (c[cid].usrflags&cflag6) && tmpscr->hasitem && !getmapflag(mITEM))
		{
			if(tmpscr->hasitem==1)
				sfx(WAV_CLEARED);
			items.add(new item((zfix)ComboX(scombo),
				//(tmpscr->flags7&fITEMFALLS && isSideViewLink()) ? (zfix)-170 : (zfix)tmpscr->itemy+1,
				(zfix)ComboY(scombo),
				//(tmpscr->flags7&fITEMFALLS && !isSideViewLink()) ? (zfix)170 : (zfix)0,
				(zfix)0,
				tmpscr->item,ipONETIME+ipBIGRANGE+((itemsbuf[tmpscr->item].family==itype_triforcepiece ||
				(tmpscr->flags3&fHOLDITEM)) ? ipHOLDUP : 0),0));
		}
		//screen secrets
		if ( c[cid].usrflags&cflag7 )
		{
			screen_combo_modify_preroutine(tmpscr,scombo);
			tmpscr->data[scombo] = tmpscr->secretcombo[ft];
			tmpscr->cset[scombo] = tmpscr->secretcset[ft];
			tmpscr->sflag[scombo] = tmpscr->secretflag[ft];
			// newflag = s->secretflag[ft];
			screen_combo_modify_postroutine(tmpscr,scombo);
			if ( c[cid].attribytes[2] > 0 )
				sfx(c[cid].attribytes[2],int(bx));
		}
		
		//loop next combo
		if((c[cid].usrflags&cflag4))
		{
			do
			{
				
				
				if (layer) 
				{
					
					screen_combo_modify_preroutine(tmpscr,scombo);
					screen_combo_modify_preroutine(FFCore.tempScreens[layer],scombo);
					
					//undercombo or next?
					if((c[cid].usrflags&cflag12))
					{
						FFCore.tempScreens[layer]->data[scombo] = tmpscr->undercombo;
						FFCore.tempScreens[layer]->cset[scombo] = tmpscr->undercset;
						FFCore.tempScreens[layer]->sflag[scombo] = 0;	
					}
					else
						++FFCore.tempScreens[layer]->data[scombo];
					
					screen_combo_modify_postroutine(FFCore.tempScreens[layer],scombo);
					//screen_combo_modify_postroutine(FFCore.tempScreens[layer],cid);
					screen_combo_modify_postroutine(tmpscr,scombo);
				}
				else
				{
					screen_combo_modify_preroutine(tmpscr,scombo);
					//undercombo or next?
					if((c[cid].usrflags&cflag12))
					{
						tmpscr->data[scombo] = tmpscr->undercombo;
						tmpscr->cset[scombo] = tmpscr->undercset;
						tmpscr->sflag[scombo] = 0;	
					}
					else
					{
						tmpscr->data[scombo]=vbound(tmpscr->data[scombo]+1,0,MAXCOMBOS);
						//++tmpscr->data[scombo];
					}
					screen_combo_modify_postroutine(tmpscr,scombo);
				}
				
				if ( (c[cid].usrflags&cflag5) ) cid = ( layer ) ? MAPCOMBO2(layer,bx,by) : MAPCOMBO(bx,by);
				if ( c[cid].usrflags&cflag8 ) w->dead = 1;
				//tmpscr->sflag[scombo] = c[cid].sflag;
				//c[tmpscr->data[cid]].cset;
				//c[tmpscr->data[cid]].cset;
				
				//tmpscr->cset[scombo] = c[cid].cset;
				//tmpscr->sflag[scombo] = c[cid].sflag;
				//zprint("++comboD\n");
			} while((c[cid].usrflags&cflag5) && (c[cid].type == cTRIGGERGENERIC) && (cid < (MAXCOMBOS-1)));
			if ( (c[cid].attribytes[2]) > 0 )
				sfx(c[cid].attribytes[2],int(bx));
			
			
		}
		if((c[cid].usrflags&cflag14)) //drop enemy
		{
			addenemy(ComboX(scombo),ComboY(scombo),(c[cid].attribytes[1]),((c[cid].usrflags&cflag6) ? 0 : -15));
		}
		//zprint("continuous\n");
		
	}
	set_bit(w->wscreengrid,(((bx>>4) + by)),1);
	
	if ( c[cid].usrflags&cflag8 ) w->dead = 1;
}

static void MatchComboTrigger2(weapon *w, int bx, int by, newcombo *c, int layer = 0/*, int comboid, int flag*/)
{
	if(screenIsScrolling()) return;
	//zprint("Layer: %d\n", layer);
	int wid = (w->useweapon > 0) ? w->useweapon : w->id;
	int cid = ( layer ) ? MAPCOMBOL(layer,bx,by) : MAPCOMBO(bx,by);
	//zprint("cid: %d\n", cid);
	int flag = ( layer ) ? MAPFLAGL(layer, bx,by) : MAPFLAG(bx,by);
	//zprint("flag: %d\n", flag);
	int flag2 = ( layer ) ? MAPCOMBOFLAGL(layer,bx,by): MAPCOMBOFLAG(bx,by);
	//zprint("flag2: %d\n", layer);
	int ft = c[cid].attribytes[3];
	//if (!ft) return;
	//zprint("ft: %d\n", ft);
	int scombo=COMBOPOS(bx,by);
	bool single16 = false;
	
	///MAPCOMBO2(layer,x,y)
	if ( c[cid].type >= cSCRIPT1 && c[cid].type <= cTRIGGERGENERIC )
	{
		//zprint("Weapon touched generic combo trigger. \n");
		//zprint("wid is: %d\n", wid);
		//zprint("cid is: %d\n", cid);
		//zprint("ft is: %d\n", ft);
		//zprint("tmpscr->secretcombo[ft]: %d\n", tmpscr->secretcombo[ft]);
		//zprint("tmpscr->secretcset[ft]: %d\n", tmpscr->secretcset[ft]);
		//zprint("tmpscr->secretflag[ft]: %d\n", tmpscr->secretflag[ft]);
		//zprint("matched is: %s\n", ( (c[cid].triggerflags[0]&combotriggerSWORD) ) ? "true" : "false");
		if ( wid == wSword && ( c[cid].triggerflags[0]&combotriggerSWORD ) && ( w->type >= c[cid].triggerlevel ) )   
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}		
		else if ( wid == wBeam && ( c[cid].triggerflags[0]&combotriggerSWORDBEAM ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wBrang && ( c[cid].triggerflags[0]&combotriggerBRANG ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wBomb && ( c[cid].triggerflags[0]&combotriggerBOMB ) && ( w->type >= c[cid].triggerlevel ) ) 
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wSBomb && ( c[cid].triggerflags[0]&combotriggerSBOMB ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wLitBomb && ( c[cid].triggerflags[0]&combotriggerLITBOMB ) && ( w->type >= c[cid].triggerlevel ) )   
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wLitSBomb && ( c[cid].triggerflags[0]&combotriggerLITSBOMB ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wArrow && ( c[cid].triggerflags[0]&combotriggerARROW ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wFire && ( c[cid].triggerflags[0]&combotriggerFIRE ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wWhistle && ( c[cid].triggerflags[0]&combotriggerWHISTLE ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wBait && ( c[cid].triggerflags[0]&combotriggerBAIT ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wWand && ( c[cid].triggerflags[0]&combotriggerWAND ) && ( w->type >= c[cid].triggerlevel ) ) 
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wMagic && ( c[cid].triggerflags[0]&combotriggerMAGIC ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wWind && ( c[cid].triggerflags[0]&combotriggerWIND ) && ( w->type >= c[cid].triggerlevel ) )   
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wRefMagic && ( c[cid].triggerflags[0]&combotriggerREFMAGIC ) && ( w->type >= c[cid].triggerlevel ) )   
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wRefFireball && ( c[cid].triggerflags[0]&combotriggerREFFIREBALL ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wRefRock && ( c[cid].triggerflags[0]&combotriggerREFROCK ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wHammer && ( c[cid].triggerflags[0]&combotriggerHAMMER ) && ( w->type >= c[cid].triggerlevel ) )   
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		
		    //ZScript liter support ends here. 
		
		else if ( wid == wHookshot && ( c[cid].triggerflags[1]&combotriggerHOOKSHOT ) && ( w->type >= c[cid].triggerlevel ) ) 
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wFSparkle && ( c[cid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[cid].triggerlevel ) )   
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wSSparkle && ( c[cid].triggerflags[1]&combotriggerSPARKLE ) && ( w->type >= c[cid].triggerlevel ) )   
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wCByrna && ( c[cid].triggerflags[1]&combotriggerBYRNA ) && ( w->type >= c[cid].triggerlevel ) )   
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wRefBeam && ( c[cid].triggerflags[1]&combotriggerREFBEAM ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wStomp && ( c[cid].triggerflags[1]&combotriggerSTOMP ) && ( w->type >= c[cid].triggerlevel ) ) 
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		    
		    //item trigger flags page 2
		
		else if ( wid == wScript1 && ( c[cid].triggerflags[1]&combotriggerSCRIPT01 ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wScript2 && ( c[cid].triggerflags[1]&combotriggerSCRIPT02 ) && ( w->type >= c[cid].triggerlevel ) ) 
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wScript3 && ( c[cid].triggerflags[1]&combotriggerSCRIPT03 ) && ( w->type >= c[cid].triggerlevel ) )   
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wScript4 && ( c[cid].triggerflags[1]&combotriggerSCRIPT04 ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wScript5 && ( c[cid].triggerflags[1]&combotriggerSCRIPT05 ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wScript6 && ( c[cid].triggerflags[1]&combotriggerSCRIPT06 ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wScript7 && ( c[cid].triggerflags[1]&combotriggerSCRIPT07 ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wScript8 && ( c[cid].triggerflags[1]&combotriggerSCRIPT08 ) && ( w->type >= c[cid].triggerlevel ) )   
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wScript9 && ( c[cid].triggerflags[1]&combotriggerSCRIPT09 ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		else if ( wid == wScript10 && ( c[cid].triggerflags[1]&combotriggerSCRIPT10 ) && ( w->type >= c[cid].triggerlevel ) )  
		{
			do_generic_combo(w, bx, by, c, wid, cid, flag, flag2, ft, scombo, single16, layer);
		}
		
	}
}


/**************************************/
/***********  Weapon Class  ***********/
/**************************************/

byte boomframe[16] = {0,0,1,0,2,0,1,1,0,1,1,3,2,2,1,2};
byte bszboomflip[4] = {0,2,3,1};

//light up the screen if there is at least one "lit" weapon
//otherwise darken the screen
void checkLightSources(bool perm = false)
{
	int isPerm = 0;
	if(perm)isPerm=3;
    for(int i=0; i<Lwpns.Count(); i++)
    {
        if(((weapon *)Lwpns.spr(i))->isLit)
        {
            lighting(true, false, isPerm);
            return;
        }
    }
    
    for(int i=0; i<Ewpns.Count(); i++)
    {
        if(((weapon *)Ewpns.spr(i))->isLit)
        {
            lighting(true, false, isPerm);
            return;
        }
    }
    
    lighting(false,false,isPerm);
}

void getdraggeditem(int j)
{
    item *it=(item*)items.spr(j);
    
    if(it==NULL)
        return;
        
    it->x = LinkX();
    it->y = LinkY();
    it->z = LinkZ();
    LinkCheckItems();
}

void weapon::seekLink()
{
    angular = true;
    angle = atan2(double(LinkY()-y),double(LinkX()-x));
    
    if(angle==-PI || angle==PI) dir=left;
    else if(angle==-PI/2) dir=up;
    else if(angle==PI/2)  dir=down;
    else if(angle==0)     dir=right;
    else if(angle<-PI/2)  dir=l_up;
    else if(angle<0)      dir=r_up;
    else if(angle>(PI/2))   dir=l_down;
    else                  dir=r_down;
    
    if(z>LinkZ()) z--;
    else if(z<LinkZ()) z++;
}

void weapon::seekEnemy(int j)
{
    angular = true;
    zfix mindistance=(zfix)1000000;
    zfix tempdistance;
    
    if((j==-1)||(j>=GuyCount()))
    {
        j=-1;
        
        for(int i=0; i<GuyCount(); i++)
        {
            //        tempdistance=sqrt(pow(abs(x-GuyX(i)),2)+pow(abs(y-GuyY(i)),2));
            tempdistance=distance(x,y,GuyX(i),GuyY(i));
            
            if((tempdistance<mindistance)&&(GuyID(i)>=10) && !GuySuperman(i))
            {
                mindistance=tempdistance;
                j=i;
            }
        }
    }
    
    if(j==-1)
    {
        return;
    }
    
    angle = atan2(double(GuyY(j)-y),double(GuyX(j)-x));
    
    if(angle==-PI || angle==PI) dir=left;
    else if(angle==-PI/2) dir=up;
    else if(angle==PI/2)  dir=down;
    else if(angle==0)     dir=right;
    else if(angle<-PI/2)  dir=l_up;
    else if(angle<0)      dir=r_up;
    else if(angle>PI/2)   dir=l_down;
    else                  dir=r_down;
}

int weapon::seekEnemy2(int j)
{
    angular = true;
    zfix mindistance=(zfix)1000000;
    zfix tempdistance;
    
    if((j==-1)||(j>=GuyCount()))
    {
        j=-1;
        
        for(int i=0; i<GuyCount(); i++)
        {
            //        tempdistance=sqrt(pow(abs(x-GuyX(i)),2)+pow(abs(y-GuyY(i)),2));
            tempdistance=distance(dummy_fix[0],dummy_fix[1],GuyX(i),GuyY(i));
            
            if((tempdistance<mindistance)&&(GuyID(i)>=10) && !GuySuperman(i))
            {
                mindistance=tempdistance;
                j=i;
            }
        }
    }
    
    if(j==-1)
    {
        return j;
    }
    
    //al_trace("Guy: %d, gx: %f, gy: %f, x: %f, y: %f\n", j, float(GuyX(j)), float(GuyY(j)), float(dummy_fix[0]), float(dummy_fix[1]));
    angle = atan2(double(GuyY(j)-dummy_fix[1]),double(GuyX(j)-dummy_fix[0]));
    
    if(angle==-PI || angle==PI) dir=left;
    else if(angle==-PI/2) dir=up;
    else if(angle==PI/2)  dir=down;
    else if(angle==0)     dir=right;
    else if(angle<-PI/2)  dir=l_up;
    else if(angle<0)      dir=r_up;
    else if(angle>PI/2)   dir=l_down;
    else                  dir=r_down;
    
    return j;
}

void weapon::convertType(bool toLW)
{
	if((isLWeapon && toLW) || (!isLWeapon && !toLW)) return; //Already the right type
	//== here is unsafe!
	weaponscript = 0;
	doscript = 0;
	initialised = 0;
	for(int q = 0; q < 8; ++q)
	{
		weap_initd[q] = 0;
	}
}

weapon::weapon(weapon const & other):
     //Struct Element			Type		Purpose
    sprite(other),
    power(other.power), 		//int
    type(other.type), 			//int
    dead(other.dead),			//int
    clk2(other.clk2),			//int
    misc2(other.misc2),			//int
    ignorecombo(other.ignorecombo),	//int
    isLit(other.isLit),			//bool		Does it light the screen?
    parentid(other.parentid),		//int		Enemy that created it. -1 for none. This is the Enemy POINTER, not the Enemy ID. 
    parentitem(other.parentitem),	//int		Item that created it. -1 for none. 
    dragging(other.dragging),		//int draggong		?
    step(other.step),			//zfix		Speed of movement
    bounce(other.bounce),		//bool		Boomerang, or hookshot bounce. 
    ignoreLink(other.ignoreLink),	//bool		?
    flash(other.flash),			//word		Is it flashing?
    wid(other.wid),			//word		ID
    aframe(other.aframe),		//word		Anim frame
    csclk(other.csclk),			//word		CSet flash clk (?)
    o_tile(other.o_tile),		//int		The base item tile
    o_cset(other.o_cset),		//int		The CSet		
    o_speed(other.o_speed),		//int		Original anim (?) speed.
    o_type(other.o_type),		//int		The weapon ID (type)
    frames(other.frames),		//int		Frames of the anim cycle
    o_flip(other.o_flip),		//int		The original flip/orientationn
    temp1(other.temp1),			//int		Misc var.
    behind(other.behind),		//bool		Should it be drawn behind Link, NPC, and other sprites?
    minX(other.minX),			//int		How close can the weapon get tot he edge of the screen
    maxX(other.maxX),			//int		...before being deleted or bouncing
    minY(other.minY),			//int		...
    maxY(other.maxY),			//int		...
	
    //! Dimentio Wand
    /*
    //!Dimentio: These 5 exist both here and in the header file. If you remove these, don't forget to
    remove them over there as well.
    */
    count1(other.count1), 		//int		Dimentio Wand 
    count2(other.count2), 		//int		Dimentio Wand 
    count3(other.count3), 		//int		Dimentio Wand
    count4(other.count4), 		//int		Dimentio Wand
    count5(other.count5), 		//int		Dimentio Wand
	
    //Weapon Editor -Z
    useweapon(other.useweapon),		//byte		The weapon editor weapon type.
    usedefence(other.usedefence),	//byte		The defence type to evaluate in do_enemy_hit()
    weaprange(other.weaprange),		//int		The range or distance of the weapon before removing it. 
    weapduration(other.weapduration),	//int		The number of frames that must elapse before removing it
   	//word		The weapon action script. 
    tilemod(other.tilemod),		//long		The LTM to use when the weapon is active. 
    drawlayer(other.drawlayer),		//byte		The layer onto which we draw the weapon.
    family_class(other.family_class),	//byte		Item Class
    family_level(other.family_level),	//byte		Item Level
    flags(other.flags),			//word		A misc flagset. 
    collectflags(other.collectflags),	//long		A flagset that determines of the weapon can collect an item.
    duplicates(other.duplicates),	//long		A flagset that determines of the weapon can collect an item.
    linked_parent(other.linked_parent),	//long		A flagset that determines of the weapon can collect an item.
    quantity_iterator(other.quantity_iterator),	//long		A flagset that determines of the weapon can collect an item.
    script_UID(FFCore.GetScriptObjectUID(UID_TYPE_WEAPON)),
//Enemy Editor Weapon Sprite
    wpnsprite(other.wpnsprite),
    specialinfo(other.specialinfo),
    magiccosttimer(other.magiccosttimer),
    ScriptGenerated(other.ScriptGenerated),
    isLWeapon(other.isLWeapon),
	linkedItem(other.linkedItem),
	//weaponscript(other.weaponscript),
	parent_script_UID(other.parent_script_UID), //Theoretical: Should the parent remain the same, or change to the weapon that spawned the copy?
	//script_UID(other.script_UID), //Should never be identical. Should get a new script_UID if needed.
	//If the cloned weapon is not getting an incremented UID for ZASM, then it needs one below.
	script_wrote_otile(other.script_wrote_otile),
	weapon_dying_frame(other.weapon_dying_frame)
    
	
	//End Weapon editor non-arrays. 

{
	weaponscript = other.weaponscript;
	doscript = other.doscript;
	//script_wrote_otile = 0;
	//if ( isLWeapon ) goto skip_eweapon_script_init;
	//eweapons
	//if ( parentid > -1 && parentid != Link.getUID() 
	//	&& !ScriptGenerated //Don't try to read the parent script for a script-generated eweapon!
	//) 
	//{
	//	enemy *s = (enemy *)guys.getByUID(parentid);
	//
	//	weaponscript = guysbuf[s->id & 0xFFF].weaponscript;
	//	parent_script_UID = s->script_UID;
	//	for ( int q = 0; q < INITIAL_D; q++ ) 
	//	{
	//		//Z_scripterrlog("(weapon::weapon(weapon const & other)): Loading Initd[%d] for this eweapon script with a value of (%d).\n", q, guysbuf[parentid].weap_initiald[q]); 
	//	
	//		weap_initd[q] = guysbuf[s->id & 0xFFF].weap_initiald[q];
	//		
	//	}
		
	//}
	//skip_eweapon_script_init:
	//if ( parentitem > -1 ) //lweapons
	//{
	//	
	//	weaponscript = itemsbuf[parentitem].weaponscript; //Set the weapon script based on the item editor data.
	//	for ( int q = 0; q < INITIAL_D; q++ ) 
	//	{
	//		weap_initd[q] = itemsbuf[parentitem].weap_initiald[q];
	//		
	//	}
		
	//}
	for ( int q = 0; q < 22; q++ ) wscreengrid[q] = 0;
	memset(wscreengrid_layer, 0, sizeof(wscreengrid_layer));
	for( int q = 0; q < 8; q++ ) 
	{
		weap_initd[q] = other.weap_initd[q];
	}
	for(int i=0; i<10; ++i)
	{
		dummy_int[i]=other.dummy_int[i];
		dummy_fix[i]=other.dummy_fix[i];
		dummy_float[i]=other.dummy_float[i];
		dummy_bool[i]=other.dummy_bool[i];
	}
    
	script_UID = FFCore.GetScriptObjectUID(UID_TYPE_WEAPON); 
    //memset(stack,0,sizeof(stack));
    //memset(stack, 0xFFFF, MAX_SCRIPT_REGISTERS * sizeof(long));
    
    //Weapon Editor Arrays
    for ( int q = 0; q < ITEM_MOVEMENT_PATTERNS; q++ ) 
    {
	weap_pattern[q] = other.weap_pattern[q];	//int	The movement pattern and args.
    }
    for ( int q = 0; q < WEAPON_CLOCKS; q++ ) 
    {
	clocks[q] = other.clocks[q];		//long	An array of misc clocks. 
    }
   // for ( int q = 0; q < INITIAL_A; q++ )
    //{
	//initiala[q] = other.initiala[q];		//byte	InitA[]
    //}
    //for ( int q = 0; q < INITIAL_D; q++ ) 
    //{
//	initiald[q] = other.initiald[q];		//long	InitD[]
    //}
    for ( int q = 0; q < FFSCRIPT_MISC; q++ ) 
    {
	ffmisc[q] = other.ffmisc[q];		//long -The base wpn->Misc[32] set from the editor
    }
    for ( int q = 0; q < FFSCRIPT_MISC; q++ ) 
    {
	wpn_misc_d[q] = other.wpn_misc_d[q];		//long -The base wpn->Misc[32] set from the editor
    }
    for ( int q = 0; q < 128; q++ ) 
    {
	weapname[q] = 0;		//long -The base wpn->Misc[32] set from the editor
    }
    script_wrote_otile = 0;
    
    //if ( parentitem > -1 )
    //{
	//weaponscript = itemsbuf[parentitem].weaponscript;
    //}
    
	//! END Weapon Editor
    
    /*for (int i=0; i<8; ++i)
    {
      d[i]=other.d[i];
    }
    for (int i=0; i<2; ++i)
    {
      a[i]=other.a[i];
    }*/
    
    
    //if ( parentid > 0 ) wpnsprite = guysbuf[parentid].wpnsprite;
    //else wpnsprite  = -1;
}

// Let's dispose of some sound effects!
void weapon::cleanup_sfx()
{
	//Check weapon id
    switch(id)
    {
		case wWind:
		case ewBrang:
		case wBrang:
		case wCByrna:
			break;
		case wSSparkle:
		case wFSparkle:
			if(parentitem>=0 && itemsbuf[parentitem].family==itype_cbyrna)
				break;
			return;
		default: return; //No repeating sfx
    }
    // First, check for the existence of weapons that don't have parentitems
    // but make looping sounds anyway.
    if(parentitem<0 && get_bit(quest_rules, qr_MORESOUNDS))
    {
        //I am reasonably confident that I fixed these expressions. ~pkmnfrk
			//No, you didn't. Now I have. -V
        if(id==ewBrang && Ewpns.idCount(ewBrang) > 1)
            return;
            
        if(id==wWind && Lwpns.idCount(wWind) > 1)
            return;
    }
    
    // Check each Lwpn to see if this weapon's sound is also allocated by it.
	int use_sfx = 0;
	if(parentitem >= 0) use_sfx = itemsbuf[parentitem].usesound;
	else switch(id)
	{
		case ewBrang:
		case wBrang:
			use_sfx = WAV_BRANG;
			break;
		case wWind:
			use_sfx = WAV_ZN1WHIRLWIND;
			break;
		case wSSparkle:
		case wFSparkle:
		case wCByrna:
			use_sfx = WAV_ZN2CANE;
			break;
	}
	
    if(use_sfx)
    {
        for(int i=0; i<Lwpns.Count(); i++)
        {
            weapon *w = ((weapon *)Lwpns.spr(i));
            
            if(w->getUID() == getUID())  // Itself!
            {
                continue;
            }
            
            int wparent = w->parentitem;
            
            if(wparent>=0 && (itemsbuf[wparent].family == itype_brang || itemsbuf[wparent].family == itype_nayruslove
                              || itemsbuf[wparent].family == itype_hookshot || itemsbuf[wparent].family == itype_cbyrna))
            {
                if(itemsbuf[wparent].usesound == use_sfx)
                    return;
            }
        }
    }
    
	stop_sfx(use_sfx);
	
    /*switch(id)
    {
    case wWind:
        stop_sfx(WAV_ZN1WHIRLWIND);
        break;
        
    case ewBrang:
        //stop_sfx(WAV_BRANG); //causes a bug -L
        break;
        
    case wBrang:
    case wCByrna:
        if(parentitem>=0)
        {
            stop_sfx(itemsbuf[parentitem].usesound);
        }
        
        break;
        
    case wSSparkle:
    case wFSparkle:
        if(parentitem>=0 && itemsbuf[parentitem].family==itype_cbyrna)
        {
            stop_sfx(itemsbuf[parentitem].usesound);
        }
        
        break;
    }*/
}
weapon::~weapon()
{
	FFCore.deallocateAllArrays(isLWeapon ? SCRIPT_LWPN : SCRIPT_EWPN, getUID());
	cleanup_sfx();
}

weapon::weapon(zfix X,zfix Y,zfix Z,int Id,int Type,int pow,int Dir, int Parentitem, int prntid, bool isDummy, byte script_gen, byte isLW, byte special, int Linked_Parent) : sprite(), parentid(prntid)
{
    x=X;
    y=Y;
    z=Z;
    id=Id;
    type=Type;
    power=pow;
    parentitem=Parentitem;
    dir=zc_max(Dir,0);
    clk=clk2=flip=misc=misc2=0;
    frames=flash=wid=aframe=csclk=0;
    ignorecombo=-1;
    step=0;
    dead=-1;
    specialinfo = special;
    bounce=ignoreLink=false;
    yofs=playing_field_offset - 2;
    dragging=-1;
    hxsz=15;
    hysz=15;
    hzsz=8;
    useweapon = usedefence = 0;
    weaprange = weapduration = 0;
    script_wrote_otile = 0;
    linked_parent = Linked_Parent;
    quantity_iterator = 0;
	weapon_dying_frame = false;
	parent_script_UID = 0;
    if ( Parentitem > -1 )
    {
	weaponscript = itemsbuf[Parentitem].weaponscript;
	useweapon = itemsbuf[Parentitem].useweapon;
	usedefence = itemsbuf[Parentitem].usedefence;
	quantity_iterator = type; //wCByrna uses this for positioning.
	if ( id != wPhantom /*&& (id != wWind && !specialinfo)*/ && /*id != wFSparkle && id != wSSparkle &&*/ ( id < wEnemyWeapons || ( id >= wScript1 && id <= wScript10) ) ) type = itemsbuf[Parentitem].fam_type; //the weapon level for real lweapons.
	    //Note: eweapons use this for boss weapon block flags
	    // Note: wInd uses type for special properties.
	    //Note: wFire is bonkers. If it writes this, then red candle and above use the wrong sprites. 
	//load initd
	for ( int q = 0; q < 8; q++ )
	{
		//load InitD
		weap_initd[q] = itemsbuf[Parentitem].weap_initiald[q];
		    
	}
    }
    
    if ( isLW ) goto skip_eweapon_script;
    if ( prntid > -1 && prntid != Link.getUID()  ) //eweapon scripts
    {
	    
	//Z_scripterrlog("Eweapon created with a prntid of: %d\n",prntid);
	enemy *s = (enemy *)guys.getByUID(prntid);
	//int parent_enemy_id = 0;
	//parent_enemy_id = s->id & 0xFFF;
	//Z_scripterrlog("The enemy ID that created it was: %d\n",s->id & 0xFFF);
	//weaponscript = guysbuf[prntid].weaponscript;
	weaponscript = guysbuf[s->id & 0xFFF].weaponscript;
	parent_script_UID = s->script_UID;
	    //Z_scripterrlog("parentUID %d\n", parent_script_UID);
	for ( int q = 0; q < 8; q++ )
	{
		//load InitD
		//Z_scripterrlog("(weapon::weapon(zfix)): Loading Initd[%d] for this eweapon script with a value of (%d).\n", q, guysbuf[parentid].weap_initiald[q]); 
		weap_initd[q] = guysbuf[s->id & 0xFFF].weap_initiald[q];
		    
	}
    }
    skip_eweapon_script:
    tilemod = 0;
    drawlayer = 0;
    family_class = family_level = 0;
    flags = 0;
    collectflags = 0;
    duplicates = 0;
    count1 = count2 = count3 = count4 = count5 = 0;
    temp1 = 0;
    scriptrange = blastsfx = wpnsprite = magiccosttimer = 0;
    for ( int q = 0; q < FFSCRIPT_MISC; q++ ) ffmisc[q] = 0;
    for ( int q = 0; q < 128; q++ ) weapname[q] = 0;
    
    for ( int q = 0; q < FFSCRIPT_MISC; q++ ) wpn_misc_d[q] = 0;
    //for ( int q = 0; q < 2; q++ ) initiala[q] = 0;
    for ( int q = 0; q < WEAPON_CLOCKS; q++ ) clocks[q] = 0;
    for ( int q = 0; q < ITEM_MOVEMENT_PATTERNS; q++ ) 
    {
	weap_pattern[q] = 0; //int	The movement pattern and args.
    }
    isLit = false;
	linkedItem = 0;
	for ( int q = 0; q < 22; q++ ) wscreengrid[q] = 0;
        memset(wscreengrid_layer, 0, sizeof(wscreengrid_layer));
	script_UID = FFCore.GetScriptObjectUID(UID_TYPE_WEAPON); 
        
	ScriptGenerated = script_gen; //t/b/a for script generated swords and other LinkCLass items. 
		//This will need an input in the params! -Z
		
	isLWeapon = isLW;
    minX = minY = maxX = maxY = 0;
    //memset(stack,0,sizeof(stack));
    //memset(stack, 0xFFFF, MAX_SCRIPT_REGISTERS * sizeof(long));
    
    int defaultw, itemid = parentitem;
    
    if(id>wEnemyWeapons)
    {
        canfreeze=true;
        
        if(id!=ewBrang)
        {
            if(Type&2)
            {
                misc=(Type>>3)-1;
                type &= ~2;
            }
            else
                misc=-1;
        }
    }
    
	//Default Gravity
    switch(id)
    {
	    case wFire:
	    
		// Din's Fire shouldn't fall
		if(parentitem>=0 && itemsbuf[parentitem].family==itype_dinsfire && !(itemsbuf[parentitem].flags & ITEM_FLAG3))
		{
		    break;
		}
		
	    case wLitBomb:
	    case wLitSBomb:
	    case wBait:
	    case ewFlame:
	    case ewFireTrail:
			moveflags |= FLAG_OBEYS_GRAV | FLAG_CAN_PITFALL;
    }
	
    switch(id)
    {
	    
	    
	case wScript1:
	case wScript2:
	case wScript3:
	case wScript4:
	case wScript5:
	case wScript6:
	case wScript7:
	case wScript8:
	case wScript9:
	case wScript10:
	case wIce:
    {
	if(parentitem >-1)
	{
		if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].weap_tilew;}
		if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].weap_tileh;}
		if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].weap_hxsz;}
		if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].weap_hysz;}
		if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].weap_hzsz;}
		if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].weap_hxofs;}
		if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].weap_hyofs;}
		if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].weap_xofs;}
		if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].weap_yofs+playing_field_offset;}
		
		defaultw = itemsbuf[parentitem].wpn;
		LOADGFX(defaultw);
		
		if (!( itemsbuf[parentitem].flags & ITEM_FLAG1 ) )
		{
			switch(dir)
			{
				case up:
				{
					flip = 0; break;
				}
				case down:
				{
					flip = 2; break;
				}
				case left:
				{
					flip = 7; break;
				}
				case right:
				{
					flip = 4; break;
				}
				default: flip = 0; break;
			}
		}
	}
	else 
	{
		if ( !o_tile )
		{
			LOADGFX(0);
			o_tile = tile;
		}
	}
	break;
    } 
    case wSword: // Link's sword
    {
        if(isDummy || itemid<0)
        {
            itemid = getCanonicalItemID(itemsbuf, itype_sword);
        }
	hxsz=hysz=15;
	if ( parentitem > -1 )
	{
		//Port Item Editor Weapon Size Values
		if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
			extend = 3; 
			
			switch(dir)
			{
				case up:
				{
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].weap_tilew;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].weap_tileh;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].weap_hxsz;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].weap_hysz;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].weap_hzsz;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].weap_hxofs;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].weap_hyofs;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].weap_xofs;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].weap_yofs+playing_field_offset;}
					/* yofs+playing_field_offset == yofs+56.
					It is needed for the passive subscreen offset.
					*/
					yofs-=16;
					break;
				}
				case down:
				{
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].weap_tilew;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].weap_tileh;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].weap_hxsz;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].weap_hysz;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].weap_hzsz;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].weap_hxofs;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].weap_hyofs;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].weap_xofs;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].weap_yofs+playing_field_offset;}
					/* yofs+playing_field_offset == yofs+56.
					It is needed for the passive subscreen offset.
					*/
					break;
				}
				case left:
				{
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].weap_tileh;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].weap_tilew;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].weap_hysz;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].weap_hxsz;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].weap_hzsz;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].weap_hyofs;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].weap_hxofs;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].weap_yofs;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].weap_xofs+playing_field_offset;}
					/* yofs+playing_field_offset == yofs+56.
					It is needed for the passive subscreen offset.
					*/
					xofs-=16;
					break;
				}
				case right:
				{
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].weap_tileh;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].weap_tilew;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].weap_hysz;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].weap_hxsz;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].weap_hzsz;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].weap_hyofs;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].weap_hxofs;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].weap_yofs;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].weap_xofs+playing_field_offset;}
					/* yofs+playing_field_offset == yofs+56.
					It is needed for the passive subscreen offset.
					*/
					break;
				}
				
			}
			
			
		}
	}
        if(itemid >-1)
	{
            defaultw = itemsbuf[itemid].wpn;
	}
        else
            defaultw = wSWORD;
            
      
	
	
	
        LOADGFX(defaultw);
        break;
    }
    
    case wWand: // Link's wand, as well as the Cane itself
    {
        if(isDummy || itemid<0)
        {
            itemid = getCanonicalItemID(itemsbuf, itype_wand);
        }
        hxsz=15;
        hysz=15; //hysz=24;
	/* The wand class items need a special set of sizes, as the size is for their projectiles. 
	if ( parentitem > -1 )
	{
		//Port Item Editor Weapon Size Values
		if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
			extend = 3; 
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].tilew;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].tileh;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].hxsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].hysz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].hzsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].hxofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].hyofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].xofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].yofs+playing_field_offset;}
			// yofs+playing_field_offset == yofs+56.
			//It is needed for the passive subscreen offset.
			
		}
	}
	*/
        if(itemid >-1)
	{
            defaultw = itemsbuf[itemid].wpn;
	}
        else
            defaultw = wWAND;
            
        
        LOADGFX(defaultw);
        break;
    }
    
    case wHammer:
    {
        if(isDummy || itemid<0)
        {
            itemid = getCanonicalItemID(itemsbuf, itype_hammer);
        }
	
        hxsz=15;
        hysz=24;
	if ( parentitem > -1 )
	{
		//Port Item Editor Weapon Size Values
		if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
			extend = 3; 
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].weap_tilew;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].weap_tileh;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].weap_hxsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].weap_hysz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].weap_hzsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].weap_hxofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].weap_hyofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].weap_xofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].weap_yofs+playing_field_offset;}
			/* yofs+playing_field_offset == yofs+56.
			It is needed for the passive subscreen offset.
			*/
		}
	}
        if(itemid >-1)
	{
            defaultw = itemsbuf[itemid].wpn;
	}
        else
            defaultw = wHAMMER;
            
        
        LOADGFX(defaultw);
        break;
    }
    
    case wCByrna: // The Cane's beam
    {
        if(isDummy || itemid<0)
        {
            itemid = getCanonicalItemID(itemsbuf, itype_cbyrna);
        }
        if ( parentitem > -1 )
	{
		//Port Item Editor Weapon Size Values
		if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
			extend = 3; 
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].weap_tilew;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].weap_tileh;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].weap_hxsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].weap_hysz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].weap_hzsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].weap_hxofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].weap_hyofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].weap_xofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].weap_yofs+playing_field_offset;}
			/* yofs+playing_field_offset == yofs+56.
			It is needed for the passive subscreen offset.
			*/
		}
	}
        if(itemid >-1)
	{
            defaultw = itemsbuf[itemid].wpn3;
	}
        else
            defaultw = wCBYRNA;
            
        LOADGFX(defaultw);
        int speed = parentitem>-1 ? zc_max(itemsbuf[parentitem].misc1,1) : 1;
        int qty = parentitem>-1 ? zc_max(itemsbuf[parentitem].misc3,1) : 1;
	//zprint("byrna quantity_iterator: %d\n", quantity_iterator);
        clk = (int)((((2*quantity_iterator*PI)/qty)
                     // Appear on top of the cane's hook
                     + (dir==right? 3*PI/2 : dir==left? PI/2 : dir==down ? 0 : PI))*speed);
        quantity_iterator = 0;
        
        if(parentitem>-1)
        {
            cont_sfx(itemsbuf[parentitem].usesound);
        }
        
        break;
    }
    
    case wWhistle:
        xofs=1000;                                            // don't show
        x=y=hxofs=hyofs=0;
        hxsz=hysz=255;                                        // hit the whole screen
    	//Port Item Editor Weapon Size Values
    
        if(isDummy || itemid<0)
        {
            itemid = getCanonicalItemID(itemsbuf, itype_whistle);
        }
        
        if ( parentitem > -1 )
	{
		//Whistle damage
		if ((itemsbuf[parentitem].flags & ITEM_FLAG2)!=0 ) //Flags[1]
		{
			power = itemsbuf[parentitem].misc5; //Attributews[5]
		}
		
		//Port Item Editor Weapon Size Values
		if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
			extend = 3; 
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].weap_tilew;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].weap_tileh;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].weap_hxsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].weap_hysz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].weap_hzsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].weap_hxofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].weap_hyofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].weap_xofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].weap_yofs+playing_field_offset;}
			/* yofs+playing_field_offset == yofs+56.
			It is needed for the passive subscreen offset.
			*/
		}
	}
        break;
        
    case wWind:
    {
        if(isDummy || itemid<0)
        {
            itemid = getCanonicalItemID(itemsbuf, itype_whistle);
        }
        clk=-14;
        step=2;
	if ( parentitem > -1 )
	{
		//Port Item Editor Weapon Size Values
		if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
			extend = 3; 
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].weap_tilew;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].weap_tileh;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].weap_hxsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].weap_hysz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].weap_hzsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].weap_hxofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].weap_hyofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].weap_xofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].weap_yofs+playing_field_offset;}
			/* yofs+playing_field_offset == yofs+56.
			It is needed for the passive subscreen offset.
			*/
		}
	}
        if(itemid >-1)
	{
            defaultw = itemsbuf[itemid].wpn;
	}
        else
            defaultw = wWIND;
            
        LOADGFX(defaultw);
        
        if(get_bit(quest_rules,qr_MORESOUNDS))
            cont_sfx(WAV_ZN1WHIRLWIND);
            
        
        break;
    }
    
    case wBeam:
    case wRefBeam:
        step = 3;
        
        if(isDummy || itemid<0)
        {
            itemid = getCanonicalItemID(itemsbuf, itype_sword);
        }
        
        if(itemid>-1 && id!=wRefBeam)
            defaultw = itemsbuf[itemid].wpn3;
        else
            defaultw = ewSWORD;
            
        LOADGFX(defaultw);
        flash = 1;
        cs = 6;
        if ( do_animation ) 
	{
        switch(dir)
        {
        case down:
            flip=get_bit(quest_rules,qr_SWORDWANDFLIPFIX)?3:2;
            
	//if ( itemid > -1 ) 
	//{
	//	This requires special cases. 	The sword beams need a special size field!!!
	//}
        case up:
            hyofs=2;
            hysz=12;
            break;
            
        case left:
	//case l_down:
            flip=1;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            hxofs=2;
            hxsz=12;
            yofs = playing_field_offset+(BSZ ? 3 : 1);
            break;
        }
        
	}
	if(id==wRefBeam)
        {
            ignorecombo=(((int)y&0xF0)+((int)x>>4));
        }
        
        break;
        
    case wArrow:
        if(isDummy || itemid<0)
        {
            itemid = getCanonicalItemID(itemsbuf, itype_arrow);
        }
        
        if(itemid >-1)
	{
            defaultw = itemsbuf[itemid].wpn;
	}
        else
	{
            defaultw = wARROW;
	}
	LOADGFX(defaultw);
        step=3;
        if ( parentitem > -1 )
	{
		//Port Item Editor Weapon Size Values
		if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
			extend = 3; 
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].weap_tilew;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].weap_tileh;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].weap_hxsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].weap_hysz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].weap_hzsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].weap_hxofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].weap_hyofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].weap_xofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].weap_yofs+playing_field_offset;}
			/* yofs+playing_field_offset == yofs+56.
			It is needed for the passive subscreen offset.
			*/
		}
	}    

	if ( do_animation ) 
	{
	switch(dir)
        {
        case down:
            flip=2;
            
        case up:
            //hyofs=2;
            //hysz=12;
	    hyofs= ( (parentitem > -1) && itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) ? itemsbuf[parentitem].weap_hyofs : 2;
			//2;
	    hysz= ( (parentitem > -1) && itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) ? itemsbuf[parentitem].weap_hysz : 12;
			//12;
            break;
            
        case left:
	case l_down:
            flip=1;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            yofs=playing_field_offset + 1;
	
	
	    hyofs= ( (parentitem > -1) && itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) ? itemsbuf[parentitem].weap_hyofs : 2;
		//2;
	    hysz= ( (parentitem > -1) && itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) ? itemsbuf[parentitem].weap_hysz : 14;
		//14;
	    hxofs=( (parentitem > -1) && itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) ? itemsbuf[parentitem].weap_hxofs : 2;
		//2;
	    hxsz=( (parentitem > -1) && itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ) ? itemsbuf[parentitem].weap_hxsz : 12;
		//12;
            //hyofs=2;
            //hysz=14;
            //hxofs=2;
            //hxsz=12;
            break;
        }
        
	}       
	if(itemid >-1)
            misc = itemsbuf[itemid].misc1;
            
        break;
        
    case wSSparkle:
	/* Sparkles would be broken if we allowed them to be sized from here. 
	if ( parentitem > -1 )
	{
		//Port Item Editor Weapon Size Values
		if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
			extend = 3; 
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].tilew;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].tileh;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].hxsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].hysz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].hzsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].hxofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].hyofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].xofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].yofs+playing_field_offset;}
			// yofs+playing_field_offset == yofs+56.
			//It is needed for the passive subscreen offset.
			
		}
	}
	*/
        LOADGFX(linked_parent ? linked_parent : wSSPARKLE);
        step=0;
        break;
        
    case wFSparkle:
	    /* Sparkes would break if we sized them from the parent item.
	if ( parentitem > -1 )
	{
		//Port Item Editor Weapon Size Values
		if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
			extend = 3; 
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].tilew;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].tileh;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].hxsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].hysz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].hzsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].hxofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].hyofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].xofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].yofs+playing_field_offset;}
			// yofs+playing_field_offset == yofs+56.
			It is needed for the passive subscreen offset.
			
		}
	}
	*/
        LOADGFX(linked_parent ? linked_parent : wFSPARKLE);
        step=0;
        break;
        
    case wFire:
        if(isDummy || itemid<0)
        {
            itemid = getCanonicalItemID(itemsbuf, itype_candle);
        }
        if ( parentitem > -1 )
	{
		//Port Item Editor Weapon Size Values
		if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
			extend = 3; 
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].weap_tilew;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].weap_tileh;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].weap_hxsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].weap_hysz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].weap_hzsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].weap_hxofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].weap_hyofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].weap_xofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].weap_yofs+playing_field_offset;}
			/* yofs+playing_field_offset == yofs+56.
			It is needed for the passive subscreen offset.
			*/
		}
		
		switch(itemsbuf[parentitem].family)
		{
		case itype_dinsfire: // Din's Fire. This uses magicitem rather than itemid
		    if(magicitem >-1 && !isDummy)
			defaultw = itemsbuf[magicitem].wpn5;
		    else defaultw = wFIRE;
		    step = 0; 
		    break;
		    
		case itype_wand: // Wand
		    if(itemid>-1 && !isDummy)
			defaultw = itemsbuf[itemid].wpn2;
		    else defaultw = wFIRE; //this is why setting ->type as weapon Level, for fire weapons has issues. 
		    step = 0;
		    break;
		    
		case itype_candle: // Candles
			hxofs = hyofs=1;
			hxsz = hysz = 14;    
			step = 0.5;
			if ( parentitem > -1 )
			{
				//Port Item Editor Weapon Size Values
				if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
					extend = 3; 
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].weap_tilew;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].weap_tileh;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].weap_hxsz;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].weap_hysz;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].weap_hzsz;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].weap_hxofs;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].weap_hyofs;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].weap_xofs;}
					if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].weap_yofs+playing_field_offset;}
					/* yofs+playing_field_offset == yofs+56.
					It is needed for the passive subscreen offset.
					*/
				}
			}
		    if(itemid>-1 && !isDummy)
		    {
			defaultw = itemsbuf[itemid].wpn3;
			break;
		    }
		
		default:
		    defaultw = wFIRE;
		}
	}
        else { defaultw = wFIRE; step = 0; }
        
        LOADGFX(defaultw);
        //step = (type<2)?.5:0;
        
        
        if(BSZ)
            yofs+=2;
            
        break;
        
    case wLitBomb:
    case wBomb:
    {
        if(isDummy || itemid<0)
        {
            itemid = getCanonicalItemID(itemsbuf, itype_bomb);
        }
        hxofs=hyofs=4;
        hxsz=hysz=8;
	
	
        if(itemid >-1)
        {
            defaultw = itemsbuf[itemid].wpn;
            misc = (id==wBomb ? 1 : itemsbuf[itemid].misc1);
        }
        else
        {
            defaultw = wBOMB;
            misc = (id==wBomb ? 1 : 50);
        }
        //This may not work for bombs, as they need special size data. We need a 'Special Size' tab.
	if ( parentitem > -1 )
	{
		//Port Item Editor Weapon Size Values
		if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
			extend = 3; 
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].weap_tilew;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].weap_tileh;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].weap_hxsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].weap_hysz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].weap_hzsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].weap_hxofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].weap_hyofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].weap_xofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].weap_yofs+playing_field_offset;}
			/* yofs+playing_field_offset == yofs+56.
			It is needed for the passive subscreen offset.
			*/
		}
	}
	
        LOADGFX(defaultw);
        
        break;
    }
    
    case wLitSBomb:
    case wSBomb:
    {
        if(isDummy || itemid<0)
        {
            itemid = getCanonicalItemID(itemsbuf, itype_sbomb);
        }
        hxofs=hyofs=4;
        hxsz=hysz=8;
	
	if ( parentitem > -1 )
	{
		defaultw = itemsbuf[itemid].wpn;
            misc = (id==wSBomb ? 1 : itemsbuf[itemid].misc1);
		//Port Item Editor Weapon Size Values
		if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
			extend = 3; 
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].weap_tilew;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].weap_tileh;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].weap_hxsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].weap_hysz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].weap_hzsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].weap_hxofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].weap_hyofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].weap_xofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].weap_yofs+playing_field_offset;}
			/* yofs+playing_field_offset == yofs+56.
			It is needed for the passive subscreen offset.
			*/
		}
	}
        else
        {
            defaultw = wSBOMB;
            misc = (id==wSBomb ? 1 : 50);
        }
        
        LOADGFX(defaultw);
        
        break;
    }
    
    case wBait:
        if(isDummy || itemid<0)
        {
            itemid = getCanonicalItemID(itemsbuf, itype_bait);
        }
        
        if(itemid >-1)
            defaultw = itemsbuf[itemid].wpn;
        else
            defaultw = wBAIT;
	
	if ( parentitem > -1 )
	{
		//Port Item Editor Weapon Size Values
		if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
			extend = 3; 
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].weap_tilew;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].weap_tileh;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].weap_hxsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].weap_hysz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].weap_hzsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].weap_hxofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].weap_hyofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].weap_xofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].weap_yofs+playing_field_offset;}
			/* yofs+playing_field_offset == yofs+56.
			It is needed for the passive subscreen offset.
			*/
		}
	}
	
        LOADGFX(defaultw);
        break;
        
    case wMagic:
    {
        itemid = current_item_id(itype_book);
        bool book = true;
        
        if(itemid<0)
        {
            itemid = directWpn>-1 ? directWpn : current_item_id(itype_wand);
            book = false;
        }
        
        if(isDummy || itemid<0)
        {
            itemid = getCanonicalItemID(itemsbuf, itype_wand);
            book = false;
        }
        
        if(itemid >-1)
            // Book Magic sprite is wpn, Wand Magic sprite is wpn3.
            defaultw = book ? itemsbuf[itemid].wpn : itemsbuf[itemid].wpn3;
        else
            defaultw = wMAGIC;
	//Z_message("itemid: %d\n",itemid);
	//Z_message("parentitem: %d\n",parentitem);
	
	
        LOADGFX(defaultw);
	if ( itemid > -1 )
	{
		Z_message("Flags: %d\n",itemsbuf[itemid].weapoverrideFLAGS);
		//Port Item Editor Weapon Size Values
		if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
			extend = 3; 
			if ( itemsbuf[itemid].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[itemid].weap_tilew;}
			if ( itemsbuf[itemid].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[itemid].weap_tileh;}
			if ( itemsbuf[itemid].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[itemid].weap_hxsz;}
			if ( itemsbuf[itemid].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[itemid].weap_hysz;}
			if ( itemsbuf[itemid].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[itemid].weap_hzsz;}
			if ( itemsbuf[itemid].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[itemid].weap_hxofs;}
			if ( itemsbuf[itemid].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[itemid].weap_hyofs;}
			if ( itemsbuf[itemid].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[itemid].weap_xofs;}
			if ( itemsbuf[itemid].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[itemid].weap_yofs+playing_field_offset;}
			/* yofs+playing_field_offset == yofs+56.
			It is needed for the passive subscreen offset.
			*/
		}
	}
	//Z_message("Flags: %d\n",itemsbuf[itemid].weapoverrideFLAGS);
	//Z_message("Extend: %d\n",extend);
	//Z_message("TileWidth: %d\n",txsz);
	//Z_message("TileHeight: %d\n",tysz);
        step = (BSZ ? 3 : 2.5);
        if ( do_animation ) 
	{
        switch(dir)
        {
        case down:
            flip=2;
            
        case up:
            hyofs = ( (parentitem > -1) && itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) ? itemsbuf[parentitem].weap_hyofs : 2;
	//2;
            hysz=( (parentitem > -1) && itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) ? itemsbuf[parentitem].weap_hysz : 12;
	//12;
            break;
            
        case left:
            flip=1;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            hxofs=( (parentitem > -1) && itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) ? itemsbuf[parentitem].weap_hxofs : 2;
	//2;
            hxsz=( (parentitem > -1) && itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ) ? itemsbuf[parentitem].weap_hxsz : 12;
	//12;
            break;
        }
	}
        break;
	
    }
    
    case wBrang:
        if(isDummy || itemid<0)
        {
            itemid = getCanonicalItemID(itemsbuf, itype_brang);
        }
        hxofs=4;
        hxsz=7;
        hyofs=2;
        hysz=11;
	if ( parentitem > -1 )
	{
		//Port Item Editor Weapon Size Values
		if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
			extend = 3; 
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itemsbuf[parentitem].weap_tilew;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itemsbuf[parentitem].weap_tileh;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hxsz = itemsbuf[parentitem].weap_hxsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hysz = itemsbuf[parentitem].weap_hysz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[parentitem].weap_hzsz;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[parentitem].weap_hxofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[parentitem].weap_hyofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[parentitem].weap_xofs;}
			if ( itemsbuf[parentitem].weapoverrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[parentitem].weap_yofs+playing_field_offset;}
			/* yofs+playing_field_offset == yofs+56.
			It is needed for the passive subscreen offset.
			*/
		}
	}
	
	Z_message("itemid: %d\n",itemid);
	Z_message("parentitem: %d\n",parentitem);
	Z_message("extend: %d\n",extend);
	Z_message("size flags: %d\n",itemsbuf[parentitem].weapoverrideFLAGS);
	Z_message("TileWidth: %d\n",txsz);
	Z_message("TileHeight: %d\n",tysz);
	Z_message("HitWidth: %d\n",hxsz);
	Z_message("HitHeight: %d\n",hysz);
	Z_message("HitZHeight: %d\n",hzsz);
	Z_message("HitXOffset: %d\n",hxofs);
	Z_message("HitYOffset: %d\n",(int)xofs);
	Z_message("DrawYOffset: %d\n",(int)yofs);
	
	
        if(itemid >-1)
            defaultw = itemsbuf[itemid].wpn;
        else
            defaultw = wBRANG;
            
        LOADGFX(defaultw);
        
        dummy_bool[0]=false;                                  //grenade armed?
        break;
        
    case wHookshot:
    {
        hookshot_used=true;
        
        if(isDummy || itemid<0)
        {
            itemid = getCanonicalItemID(itemsbuf, itype_hookshot);
        }
        
        if(itemid >-1)
            defaultw = itemsbuf[itemid].wpn;
        else
            defaultw = wHSHEAD;
            
        LOADGFX(defaultw);
        step = 4;
        clk2=256;
        
	//Size Tab Settings here would be broken, IMO. -Z
	if ( do_animation ) 
	{
        switch(dir)
        {
        case down:
            flip=2;
            xofs+=4;
            yofs+=1;
            hyofs=2;
            hysz=12;
            break;
            
        case up:
            yofs+=3;
            xofs-=5;
            hyofs=2;
            hysz=12;
            break;
            
        case left:
            flip=1; /*tile=o_tile+((frames>1)?frames:1)*/update_weapon_frame(((frames>1)?frames:1),o_tile);
            xofs+=2;
            yofs=playing_field_offset+4;
            hxofs=2;
            hxsz=12;
            break;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            xofs-=2;
            yofs=playing_field_offset+4;
            hxofs=2;
            hxsz=12;
            break;
	//Diagonal Hookshot (1)
	case l_up:
		LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn5);
		yofs+=3;
		xofs-=3;
		hysz=12;
		hxsz=12;
		update_weapon_frame(((frames>1)?frames:0),o_tile);
		flip=0;
		break;
	case r_down:
		LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn5);
		yofs+=3; //check numbers ater
		xofs-=3;
		hysz=12;
		hxsz=12;
		//update gfx here
		update_weapon_frame(((frames>1)?frames:0),o_tile);
		flip=3;
		break;
	case l_down:
		LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn5);
		yofs+=3;
		xofs-=3;
		hysz=12;
		hxsz=12;
		//update gfx here
		update_weapon_frame(((frames>1)?frames:0),o_tile);
		flip=2;
		break;
	case r_up:
		LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn5);
		yofs+=3;
		xofs-=3;
		hysz=12;
		hxsz=12;
		//update gfx here
		update_weapon_frame(((frames>1)?frames:0),o_tile);
		flip=1;
		break;
	
        }
        
	}
	break;
    }
        
    case wHSHandle:
        step = 0;
        
        if(isDummy || itemid<0)
        {
            itemid = getCanonicalItemID(itemsbuf, itype_hookshot);
        }
        
        if(itemid >-1)
            defaultw = itemsbuf[itemid].wpn4;
        else
            defaultw = wHSHANDLE;
            
        LOADGFX(defaultw);
        if ( do_animation ) 
	{
        switch(dir)
        {
        case down:
            flip=2;
            xofs+=4;
            yofs+=1;
            hyofs=2;
            hysz=12;
            break;
            
        case up:
            yofs+=3;
            xofs-=5;
            hyofs=2;
            hysz=12;
            break;
            
        case left:
            flip=1; /*tile=o_tile+((frames>1)?frames:1)*/update_weapon_frame(((frames>1)?frames:1),o_tile);
            xofs+=2;
            yofs=playing_field_offset+4;
            hxofs=2;
            hxsz=12;
            break;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            xofs-=2;
            yofs=playing_field_offset+4;
            hxofs=2;
            hxsz=12;
            break;
	
	//Diagonal Hookshot (5)
	case r_down:
		LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn6);
		yofs+=3; //check numbers ater
		xofs-=3;
		hysz=12;
		hxsz=12;
		//update gfx here
		update_weapon_frame(((frames>1)?frames:0),o_tile);
		flip=3;
		break;
	case l_down:
		LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn6);
		yofs+=3;
		xofs-=3;
		hysz=12;
		hxsz=12;
		//update gfx here
		update_weapon_frame(((frames>1)?frames:0),o_tile);
		flip=2;
		break;
	case r_up:
		LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn6);
		yofs+=3;
		xofs-=3;
		hysz=12;
		hxsz=12;
		//update gfx here
		update_weapon_frame(((frames>1)?frames:0),o_tile);
		flip=1;
		break;
	case l_up:
		LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn6);
		yofs+=3;
		xofs-=3;
		hysz=12;
		hxsz=12;
		//update gfx here
		update_weapon_frame(((frames>1)?frames:0),o_tile);
		flip=0;
		break;
        }
        
	}
	break;
        
    case wHSChain:
    {
        if(isDummy || itemid<0)
        {
            itemid = getCanonicalItemID(itemsbuf, itype_hookshot);
        }
        
        if(itemid >-1)
            defaultw = (dir<left) ? itemsbuf[itemid].wpn3 : itemsbuf[itemid].wpn2;
        else
            defaultw = (dir<left) ? wHSCHAIN_V : wHSCHAIN_H;
            
        step = 0;
        
        switch(dir)
        {
        case down:
            LOADGFX(defaultw);
            xofs+=4;
            yofs-=7;
            break;
            
        case up:
            LOADGFX(defaultw);
            xofs-=5;
            yofs+=11;
            break;
            
        case left:
            LOADGFX(defaultw);
            xofs+=10;
            yofs=playing_field_offset+4;
            break;
            
        case right:
            LOADGFX(defaultw);
            xofs-=10;
            yofs=playing_field_offset+4;
            break;
	
	//Diagonal Hookshot (4)
	//Try drawing diagonal. -Z
	//Sprites wpn5: Head, diagonal
	//	  wpn6: handle, diagonal
	//	  wpn7: chainlink, diagonal
	//
	case r_up:
		LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn7);
		xofs-=10;
		yofs+=7;
		update_weapon_frame(((frames>1)?frames:0),o_tile);
		flip=1;
		break;
	case r_down:
		LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn7);
		xofs-=10;
		yofs-=7;
		update_weapon_frame(((frames>1)?frames:0),o_tile);
		flip=3;
		break;
	case l_up:
		LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn7);
		xofs+=10;
		yofs+=7;
		update_weapon_frame(((frames>1)?frames:0),o_tile);
		flip=0;
		break;
	case l_down:
		LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn7);
		xofs+=10;
		yofs-=7;
		update_weapon_frame(((frames>1)?frames:0),o_tile);
		flip=2;
		break;
        }
    }
    break;
    
    //EWeapons 
    //The ENemy Editor Needs a Weapon Tab for Sizing and other values. The Weapon Sprite field can move to /that/. -Z
    case ewLitBomb:
    case ewBomb:
        if ( parentid > -1 )
	{
		enemy *e = (enemy*)guys.getByUID(parentid);
		int enemy_wpnsprite = e->wpnsprite;
		if ( enemy_wpnsprite > 0 ) LOADGFX(enemy_wpnsprite);
		
		else LOADGFX(ewBOMB);
	}
	
	else LOADGFX(ewBOMB);
        hxofs=0;
        hxsz=16;
        
		if(get_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX))
		{
			hyofs=0;
			hysz=16;
		}
        
        if(id==ewBomb)
            misc=2;
        else
            step=3;
	if ( do_animation ) 
	{
        switch(dir)
        {
        case down:
	case l_down:
	case r_down:
            flip=2;
            
	case l_up:
	case r_up:
        case up:
            break;
            
        case left:
            flip=1;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            break;
        }
	}
        break;
        
    case ewLitSBomb:
    case ewSBomb:
        if ( parentid > -1 )
	{
		enemy *e = (enemy*)guys.getByUID(parentid);
		int enemy_wpnsprite = e->wpnsprite; if ( enemy_wpnsprite > 0 ) LOADGFX(enemy_wpnsprite);
		else LOADGFX(ewSBOMB);
	}
	else LOADGFX(ewSBOMB);
        hxofs=0;
        hxsz=16;
		if(get_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX))
		{
			hyofs=0;
			hysz=16;
		}
        
        if(id==ewSBomb)
            misc=2;
        else
            step=3;
	if ( do_animation ) 
	{
        switch(dir)
        {
        case down:
	case l_down:
	case r_down:
            flip=2;
            
        case up:
	case r_up:
	case l_up:
            break;
            
        case left:
            flip=1;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            break;
        }
        
	}
	break;
        
    case ewBrang:
        if(get_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX))
        {
            hxofs=0;
            hxsz=16;
			hyofs=0;
			hysz=16;
        }
        else
        {
            hxofs=4;
            hxsz=8;
        }
        
        wid = zc_min(zc_max(current_item(itype_brang),1),3)-1+wBRANG;
        if ( parentid > -1 )
	{
		enemy *e = (enemy*)guys.getByUID(parentid);
		int enemy_wpnsprite = e->wpnsprite; 
		if ( enemy_wpnsprite > 0 ) LOADGFX(enemy_wpnsprite);
		else LOADGFX(wid);
	}
	else LOADGFX(wid);
        break;
        
    case ewFireball2:
        if(dir < 8)
            misc=dir;
        else
        {
            misc = -1;
        }
        
        //fallthrough
    case ewFireball:
    {
        if ( parentid > -1 && !isLWeapon )
	{
		enemy *e = (enemy*)guys.getByUID(parentid);
		int enemy_wpnsprite = e->wpnsprite; 
		if ( enemy_wpnsprite > 0 ) LOADGFX(enemy_wpnsprite);
		else LOADGFX(ewFIREBALL);
	}
	else LOADGFX(ewFIREBALL);
        step=1.75;
        
        if(Type&2)
        {
            seekLink();
        }
        else misc=-1;
        
        break;
    }
    case wRefFireball:
    {
        LOADGFX(ewFIREBALL);
        step=1.75;
        
        if(Type&2)
        {
            seekLink();
        }
        else misc=-1;
        
        break;
    }
        
    case ewRock:
        if ( parentid > -1 && !isLWeapon )
	{
		enemy *e = (enemy*)guys.getByUID(parentid);
		int enemy_wpnsprite = e->wpnsprite; 
		if ( enemy_wpnsprite > 0 ) LOADGFX(enemy_wpnsprite);
		else LOADGFX(ewROCK);
	}
	else LOADGFX(ewROCK);
        
        if(get_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX))
        {
            hxofs=0;
            hxsz=16;
			hyofs=0;
			hysz=16;
        }
        else
        {
            hxofs=4;
            hxsz=8;
        }
        
        step=3;
        break;
        
    case ewArrow:
        if ( parentid > -1 && !isLWeapon )
	{
		enemy *e = (enemy*)guys.getByUID(parentid);
		int enemy_wpnsprite = e->wpnsprite; 
		if ( enemy_wpnsprite > 0 ) LOADGFX(enemy_wpnsprite);
		else LOADGFX(ewARROW);
	}
	else LOADGFX(ewARROW);
        step=2;
        if ( do_animation ) 
	{
        switch(dir)
        {
        case down:
	case l_down:
	case r_down:
            flip=2;
            
	case r_up:
	case l_up:
        case up:
            xofs=-4;
            hxsz=8;
            break;
            
        case left:
            flip=1;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            yofs=playing_field_offset+1;
            break;
        }
        
	}
	break;
        
    case ewSword:
        if ( parentid > -1 && !isLWeapon )
	{
		enemy *e = (enemy*)guys.getByUID(parentid);
		int enemy_wpnsprite = e->wpnsprite; 
		if ( enemy_wpnsprite > 0 ) LOADGFX(enemy_wpnsprite);
		else LOADGFX(ewSWORD);
	}
	else LOADGFX(ewSWORD);
        
        if(get_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX))
        {
            hxofs=0;
            hxsz=16;
			hyofs=0;
			hysz=16;
        }
        else
        {
            hxofs=4;
            hxsz=8;
        }
        
        step=3;
        if ( do_animation ) 
	{
        switch(dir)
        {
        case down:
	case l_down:
	case r_down:
            flip=2;
            
        case up:
	case r_up:
	case l_up:
            break;
            
        case left:
            flip=1;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            yofs=playing_field_offset+1;
            break;
        }
        
	}
	break;
        
    case wRefMagic:
    case ewMagic:
    {
        //reached case wRefMagic in weapons.cpp
        //al_trace("Reached case wRefMagic in weapons.cpp, line %d\n",1734);
        if ( parentid > -1 && !script_gen && (!(id == ewMagic && isLWeapon)) )
	{
		enemy *e = (enemy*)guys.getByUID(parentid);
		int enemy_wpnsprite = e->wpnsprite; 
		if ( enemy_wpnsprite > 0 ) LOADGFX(enemy_wpnsprite);
		else LOADGFX(ewMAGIC);
	}
	else LOADGFX(ewMAGIC);
        
        if(get_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX))
        {
            hxofs=0;
            hxsz=16;
			hyofs=0;
			hysz=16;
        }
        else
        {
            hxofs=4;
            hxsz=8;
        }
        
        step=3;
        if ( do_animation ) 
	{
        switch(dir)
        {
        case down:
	case l_down:
	case r_down:
            flip=2;
            
        case up:
	case r_up:
	case l_up:
            break;
            
        case left:
            flip=1;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            yofs=playing_field_offset+1;
            break;
        }
        
	}
	if(id==wRefMagic)
        {
            //al_trace("Reached case wRefMagic in weapons.cpp, line %d\n",1779);
            ignorecombo=(((int)y&0xF0)+((int)x>>4));
        }
        
        break;
    }
    case ewFlame:
    case ewFlame2:
	if(id==ewFlame)
	{
		if ( parentid > -1  && !script_gen && !isLWeapon)
		{
			enemy *e = (enemy*)guys.getByUID(parentid);
			int enemy_wpnsprite = e->wpnsprite; 
			if ( enemy_wpnsprite > 0 ) LOADGFX(enemy_wpnsprite);
			else LOADGFX(ewFLAME);
		}
		else LOADGFX(ewFLAME);
	}
        else
	{
		if ( parentid > -1 && !script_gen &&!isLWeapon )
		{
			enemy *e = (enemy*)guys.getByUID(parentid);
			int enemy_wpnsprite = e->wpnsprite; 
			if ( enemy_wpnsprite > 0 ) LOADGFX(enemy_wpnsprite);
			else LOADGFX(ewFLAME2);
		}
		else LOADGFX(ewFLAME2);
	}
        if(dir==255)
        {
            step=2;
            seekLink();
        }
        else
        {
            /*if(Dir<0)
            {
            	step = 0;
            }*/
            if(dir>right)
            {
                step = .707;
            }
            else
            {
                step = 1;
            }
        }
        
		if(get_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX))
		{
			hxofs=hyofs=0;
			hxsz=hysz=16;
		}
		else
		{
			hxofs = hyofs=1; // hof of 1 means that link can use the 'half-tile trick'.
			hxsz = hysz = 14;
		}
        
        if(BSZ)
            yofs+=2;
            
        break;
        
    case ewFireTrail:
        if ( parentid > -1 && !script_gen &&!isLWeapon)
	{
		enemy *e = (enemy*)guys.getByUID(parentid);
		int enemy_wpnsprite = e->wpnsprite; 
		if ( enemy_wpnsprite > 0 ) LOADGFX(enemy_wpnsprite);
		else LOADGFX(ewFIRETRAIL);
	}
	else LOADGFX(ewFIRETRAIL);
        step=0;
        dir=-1;
        
        if(get_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX))
		{
			hxofs=hyofs=0;
			hxsz=hysz=16;
		}
		else
		{
			hxofs = hyofs=1; // hof of 1 means that link can use the 'half-tile trick'.
			hxsz = hysz = 15;
		}
        
        if(BSZ)
            yofs+=2;
            
        break;
        
    case ewWind:
        if(get_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX))
		{
			hxofs=hyofs=0;
			hxsz=hysz=16;
		}
		
        if ( parentid > -1 && !script_gen &&!isLWeapon)
	{
		enemy *e = (enemy*)guys.getByUID(parentid);
		int enemy_wpnsprite = e->wpnsprite;
		if ( enemy_wpnsprite > 0 ) LOADGFX(enemy_wpnsprite);
		else LOADGFX(ewWIND);
	}
	else LOADGFX(ewWIND);
        clk=0;
        step=3;
        break;
        
    case wPhantom:
        switch(type)
        {
        case pDINSFIREROCKET:
            if(get_bit(quest_rules,qr_MORESOUNDS))
                sfx(WAV_ZN1ROCKETUP,(int)x);
                
            LOADGFX(itemsbuf[parentitem].wpn);
            step = 4;
            break;
            
        case pDINSFIREROCKETRETURN:
            if(get_bit(quest_rules,qr_MORESOUNDS))
                sfx(WAV_ZN1ROCKETDOWN,(int)x);
                
            LOADGFX(itemsbuf[parentitem].wpn2);
            step = 4;
            break;
            
        case pDINSFIREROCKETTRAIL:
            LOADGFX(itemsbuf[parentitem].wpn3);
            break;
            
        case pDINSFIREROCKETTRAILRETURN:
            LOADGFX(itemsbuf[parentitem].wpn4);
            break;
            
        case pMESSAGEMORE:
            LOADGFX(iwMore);
            break;
            
        case pNAYRUSLOVEROCKET1:
            LOADGFX(itemsbuf[parentitem].wpn);
            
            if(get_bit(quest_rules,qr_MORESOUNDS))
                sfx(WAV_ZN1ROCKETUP,(int)x);
                
            step = 4;
            drawstyle=itemsbuf[parentitem].flags & ITEM_FLAG2 ? 1 : 0;
            break;
            
        case pNAYRUSLOVEROCKETRETURN1:
            LOADGFX(itemsbuf[parentitem].wpn2);
            
            if(get_bit(quest_rules,qr_MORESOUNDS))
                sfx(WAV_ZN1ROCKETDOWN,(int)x);
                
            step = 4;
            drawstyle=itemsbuf[parentitem].flags & ITEM_FLAG2 ? 1 : 0;
            break;
            
        case pNAYRUSLOVEROCKETTRAIL1:
            LOADGFX(itemsbuf[parentitem].wpn3);
            drawstyle=itemsbuf[parentitem].flags & ITEM_FLAG2 ? 1 : 0;
            break;
            
        case pNAYRUSLOVEROCKETTRAILRETURN1:
            LOADGFX(itemsbuf[parentitem].wpn4);
            drawstyle=itemsbuf[parentitem].flags & ITEM_FLAG2 ? 1 : 0;
            break;
            
        case pNAYRUSLOVEROCKET2:
            LOADGFX(itemsbuf[parentitem].wpn6);
            step = 4;
            drawstyle=itemsbuf[parentitem].flags & ITEM_FLAG2 ? 1 : 0;
            break;
            
        case pNAYRUSLOVEROCKETRETURN2:
            LOADGFX(itemsbuf[parentitem].wpn7);
            step = 4;
            drawstyle=itemsbuf[parentitem].flags & ITEM_FLAG2 ? 1 : 0;
            break;
            
        case pNAYRUSLOVEROCKETTRAIL2:
            LOADGFX(itemsbuf[parentitem].wpn8);
            drawstyle=itemsbuf[parentitem].flags & ITEM_FLAG2 ? 1 : 0;
            break;
            
        case pNAYRUSLOVEROCKETTRAILRETURN2:
            LOADGFX(itemsbuf[parentitem].wpn9);
            drawstyle=itemsbuf[parentitem].flags & ITEM_FLAG2 ? 1 : 0;
            break;
            
        default:
            tile=0;
            break;
        }
        
        break;
        
//	case wScript1:
//	case wScript2:
//	case wScript3:
//	case wScript4:
//	case wScript5:
//	case wScript6:
//	case wScript7:
//	case wScript8:
//	case wScript9:
//	case wScript10:
//	{
		//LOADGFX(0);
		//if ( do_animation ) update_weapon_frame(((frames>1)?frames:1),o_tile);
		//break;
//	}
	
    default:
        LOADGFX(0);
        break;
    }
    
    if(id>wEnemyWeapons && id!=ewBrang && (Type&4)!=0)  // Increase speed of Aquamentus 2 fireballs
    {
        step *=2;
    }
}

int weapon::getScriptUID() { return script_UID; }
void weapon::setScriptUID(int new_id) { script_UID = new_id; }

int weapon::getParentScriptUID() { return parent_script_UID; }
void weapon::setParentScriptUID(int new_id) { parent_script_UID = new_id; }


bool weapon::isLinkWeapon()
{
	if ( isLWeapon > 0 ) return true;
	if ( id < lwMax ) return true;
	if ( id < wEnemyWeapons && isLWeapon > 0 ) return true;
	return false;
}
bool weapon::isLinkMelee()
{
	int family = itemsbuf[parentitem].family;
	if ( family == itype_sword && id != wBeam ) return true;
	//if ( id == wBeam )  return true;
	return false;
}


void weapon::LOADGFX(int wpn)
{
    if(wpn<0)
        return;
        
    wid = wpn;
    flash = wpnsbuf[wid].misc&3;
    tile  = wpnsbuf[wid].newtile;
    cs = wpnsbuf[wid].csets&15;
    o_tile = wpnsbuf[wid].newtile;
    o_cset = wpnsbuf[wid].csets;
    o_flip=(wpnsbuf[wid].misc>>2)&3;
    o_speed = wpnsbuf[wid].speed;
    o_type = wpnsbuf[wid].type;
    frames = wpnsbuf[wid].frames;
    temp1 = wpnsbuf[wFIRE].newtile;
    behind = (wpnsbuf[wid].misc&WF_BEHIND)!=0;
}

bool weapon::Dead()
{
    return dead!=-1;
}

bool weapon::isScriptGenerated()
{
	return (ScriptGenerated != 0);
}

bool weapon::clip()
{
    int c[4];
    int d2=isdungeon();
    int nb1 = get_bit(quest_rules,qr_NOBORDER) ? 16 : 0;
    int nb2 = get_bit(quest_rules,qr_NOBORDER) ? 8 : 0;
    
    if(id>wEnemyWeapons && id!=ewBrang)
    {
        c[0] = d2?32:(16-nb1);
        c[1] = d2?128:(144+nb1);
        c[2] = d2?32:(16-nb1);
        c[3] = d2?208:(224+nb1);
    }
    else if(id==wHookshot||id==wHSChain)
    {
        c[0] = d2?8:0;
        c[1] = d2?152:160;
        c[2] = d2?8:0;
        c[3] = d2?248:256;
    }
    else
    {
        c[0] = d2?18:2;
        c[1] = d2?144:160;
        c[2] = d2?20:4;
        c[3] = d2?220:236;
    }
    
    if(id==wSSparkle || id==wFSparkle)
    {
        c[0] = 0;
        c[1] = 176;
        c[2] = 0;
        c[3] = 256;
    }
    
    if(id==ewFlame)
    {
        c[0] = d2?32:(16-nb1);
        c[1] = d2?128:(144+nb1);
        c[2] = d2?32:(16-nb1);
        c[3] = d2?208:(224+nb1);
    }
    
    if(id==ewFireTrail)
    {
        c[0] = d2?32:(16-nb1);
        c[1] = d2?128:(144+nb1);
        c[2] = d2?32:(16-nb1);
        c[3] = d2?208:(224+nb1);
    }
    
    if(id==ewWind)
    {
        c[0] = d2?32:(16-nb1);
        c[1] = d2?128:(144+nb1);
        c[2] = d2?32:(16-nb1);
        c[3] = d2?208:(224+nb1);
    }
    
    if(x < c[2])
        if(dir==left || dir==l_up || dir==l_down)
            return true;
            
    if(x > c[3])
        if(dir==right || dir==r_up || dir==r_down)
            return true;
            
    if(y < c[0])
        if(dir==up || dir==l_up || dir==r_up)
            return true;
            
    if(y > c[1])
        if(dir==down || dir==l_down || dir==r_down)
            return true;
            
    if(id>wEnemyWeapons)
    {
        if((x<(8-nb2) && dir==left)
                || (y<(8-nb2) && dir==up)
                || (x>(232+nb2) && dir==right)
                || (y>(168+nb2) && dir==down))
            return true;
    }
    
    if(x<0||y<0||x>240||y>176)
        return true;
        
    return false;
}

bool weapon::blocked()
{
    return blocked(8, 8);
}

bool weapon::blocked(int xOffset, int yOffset)
{
    int wx = x+xOffset;
    int wy = y+yOffset;
    
    if(id == wPhantom || id == wHSHandle || id == wHSChain)  // Sanity check
    {
        return false;
    }
    
    if(get_bit(combo_class_buf[COMBOTYPE(wx,wy)].block_weapon,id)
            || get_bit(combo_class_buf[FFCOMBOTYPE(wx,wy)].block_weapon, id))
    {
	    //Add lw->Level check here. -Z
        if(parentitem<0 || (combo_class_buf[COMBOTYPE(wx,wy)].block_weapon_lvl >=
                            itemsbuf[parentitem].fam_type))
        {
            return true;
        }
    }
    
    switch(dir)
    {
    case up:
        wy-=8;
        break;
        
    case down:
        wy+=8;
        break;
        
    case left:
        wx-=8;
        break;
        
    case right:
        wx+=8;
        break;
    }
    
    if(get_bit(combo_class_buf[COMBOTYPE(wx,wy)].block_weapon,id)
            || get_bit(combo_class_buf[FFCOMBOTYPE(wx,wy)].block_weapon, id))
    {
        if(parentitem<0 || (combo_class_buf[COMBOTYPE(wx,wy)].block_weapon_lvl >=
                            itemsbuf[parentitem].fam_type))
        {
            return true;
        }
    }
    
    return false;
}

void weapon::runscript(int index)
{
    return;
	if(dead != 0) weapon_dying_frame = false; //reset dying frame if weapon revived
	switch(id)
	{
	    case wScript1:
	    case wScript2:
	    case wScript3:
	    case wScript4:
	    case wScript5:
	    case wScript6:
	    case wScript7:
	    case wScript8:
	    case wScript9:
	    case wScript10:
	    {
		    if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());	
		/*
                if ( doscript && weaponscript > 0 ) 
                {
			if ( Dead() )
			{
				doscript = 0;
				weaponscript = 0;
			}
			else
			{
				//al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
				//FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, index);		
				FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());		
			}
		}
		    */
		break;
	    }
	    case wBeam:
	    case wRefBeam:
	    {
		if ( doscript && weaponscript > 0 ) 
		{
			/*if ( Dead() )
			{
				doscript = 0;
				weaponscript = 0;
			}
			else*/
			{
				//al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
				//FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, index);		
				if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS]) ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());		
			}
		}
		break;
	    }
		
	    case wWhistle:
	    {
		if ( doscript && weaponscript > 0 ) 
		{
			/*if ( Dead() )
			{
				doscript = 0;
				weaponscript = 0;
			}
			else*/
			{
				//al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
				//FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, index);		
				if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());		
			}
		}
		break;
	    }
		
	    case wWind:
	    {
		break;
	    }
	    
	    case wFire:
	    {
		if ( doscript && weaponscript > 0 ) 
		{
			/*if ( Dead() )
			{
				doscript = 0;
				weaponscript = 0;
			}
			else*/
			{
				//al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
				//FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, index);		
				if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS]) ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());		
			}
		}
		break;
	    }
	    
	    case wLitBomb:
	    case wBomb:
	    case ewLitBomb:
	    case ewBomb:
	    case ewLitSBomb:
	    case ewSBomb:
	    case wLitSBomb:
	    case wSBomb:
	    {
		break;
	    }
	    
	    case wArrow:
	    {
		if ( doscript && weaponscript > 0 ) 
		{
			/*if ( Dead() )
			{
				doscript = 0;
				weaponscript = 0;
			}
			else*/
			{
				//al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
				//FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, index);		
				if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS]) ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());		
			}
		}
		
		break;
	    }
	    
	    case wSSparkle:
	    case wFSparkle:
	    {
		if ( doscript && weaponscript > 0 ) 
		{
			ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
		}
		break;
	    }
	    case wBait:
	    {
		if ( doscript && weaponscript > 0 ) 
		{
			/*if ( dead != -1 )
			{
				doscript = 0;
				weaponscript = 0;
			}
			else*/
			{
                            //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                            //FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, index);		
                            if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS]) ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());		
			}
		}
		break;
	    }
	    case wBrang:
	    {
                
		if ( doscript && weaponscript > 0 ) 
		{
			/*if ( Dead() )
			{
				doscript = 0;
				weaponscript = 0;
			}
			else*/
			{
				//al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
				//FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, index);		
				if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255 && !(FFCore.system_suspend[susptLWEAPONSCRIPTS]) ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());		
			}
		}
	
		
		break;
	    }
	    
	    case wHookshot:
	    {
		break;
	    }
	    case wHSHandle:
	    {
		break;
	    }
	    case wPhantom:
	    {
		break;
	    }
	    case wRefMagic:
	    case wMagic:
	    {
		//:Weapon Only
		if ( doscript && weaponscript > 0 ) 
		{
			/*if ( Dead() )
			{
				doscript = 0;
				weaponscript = 0;
			}
			else*/
			{
				//al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
				//FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, index);		
				if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS]) ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());		
			}
		}
	    }
	    break;
	    
	    case wRefFireball:
	    {
		if ( doscript && weaponscript > 0 && ScriptGenerated ) 
		{
			/*if ( Dead() )
			{
				doscript = 0;
				weaponscript = 0;
			}
			else*/
			{
				//al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
				//FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, index);		
				if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS]) ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());		
			}
		}
		break;
	    }
	    
	    default: break;
	}
}

bool weapon::animate(int index)
{
	if(dead != 0) weapon_dying_frame = false; //reset dying frame if weapon revived
	if(fallclk > 0)
	{
		if(fallclk == PITFALL_FALL_FRAMES && fallCombo) sfx(combobuf[fallCombo].attribytes[0], pan(x.getInt()));
		if(!--fallclk)
		{
			if(!weapon_dying_frame && get_bit(quest_rules,qr_WEAPONS_EXTRA_FRAME))
			{
				if(id==wSword || id==wBrang)
				{
					return true;
				}
				dead = 0;
				weapon_dying_frame = true;
				++fallclk;
				
				run_script(MODE_NORMAL);
				
				return false;
			}
			return true;
		}
		
		wpndata& spr = wpnsbuf[QMisc.sprites[sprFALL]];
		cs = spr.csets & 0xF;
		int fr = spr.frames ? spr.frames : 1;
		int spd = spr.speed ? spr.speed : 1;
		int animclk = (PITFALL_FALL_FRAMES-fallclk);
		tile = spr.newtile + zc_min(animclk / spd, fr-1);
		
		run_script(MODE_NORMAL);
		
		return false;
	}
	if(drownclk > 0)
	{
		if(drownclk == WATER_DROWN_FRAMES && drownCombo); //sfx(combobuf[drownCombo].attribytes[0], pan(x.getInt()));
		//!TODO: Drown SFX
		if(!--drownclk)
		{
			if(!weapon_dying_frame && get_bit(quest_rules,qr_WEAPONS_EXTRA_FRAME))
			{
				if(id==wSword || id==wBrang)
				{
					return true;
				}
				dead = 0;
				weapon_dying_frame = true;
				++drownclk;
				
				run_script(MODE_NORMAL);
				
				return false;
			}
			return true;
		}
		
		if (combobuf[drownCombo].usrflags&cflag1) 
		{
			wpndata &spr = wpnsbuf[QMisc.sprites[sprLAVADROWN]];
			cs = spr.csets & 0xF;
			int fr = spr.frames ? spr.frames : 1;
			int spd = spr.speed ? spr.speed : 1;
			int animclk = (WATER_DROWN_FRAMES-drownclk);
			tile = spr.newtile + zc_min(animclk / spd, fr-1);
		}
		else 
		{
			wpndata &spr = wpnsbuf[QMisc.sprites[sprDROWN]];
			cs = spr.csets & 0xF;
			int fr = spr.frames ? spr.frames : 1;
			int spd = spr.speed ? spr.speed : 1;
			int animclk = (WATER_DROWN_FRAMES-drownclk);
			tile = spr.newtile + zc_min(animclk / spd, fr-1);
		}
		
		run_script(MODE_NORMAL);
		
		return false;
	}
    // do special timing stuff
    bool hooked=false;
//	Z_scripterrlog("Weapon script is: %d\n",weaponscript);
	
    //Only lweapons, or wScript if the weapon is not script generated, or if it IS script-generated and is not an eweapon.
    if ( id < wEnemyWeapons || ( id >= wScript1 && id <= wScript10 && ( (ScriptGenerated && isLWeapon) || !ScriptGenerated) ) ) 
    {
	    /*
	    if ( useweapon == wSword )
	    {
		if(dir==up && ((int(x)&15)==0))
		{
		    Link.check_slash_block2((int)x,(int)y);
		    Link.check_slash_block2((int)x,(int)y+8);
		}
		else if(dir==up && ((int(x)&15)==8||Link.diagonalMovement))
		{
		    Link.check_slash_block2((int)x,(int)y);
		    Link.check_slash_block2((int)x,(int)y+8);
		    Link.check_slash_block2((int)x+8,(int)y);
		    Link.check_slash_block2((int)x+8,(int)y+8);
		}
		
		if(dir==down && ((int(x)&15)==0))
		{
		    Link.check_slash_block2((int)x,(int)y+(int)hysz-8);
		    Link.check_slash_block2((int)x,(int)y+(int)hysz);
		}
		else if(dir==down && ((int(x)&15)==8||Link.diagonalMovement))
		{
		    Link.check_slash_block2((int)x,(int)y+hysz-8);
		    Link.check_slash_block2((int)x,(int)y+hysz);
		    Link.check_slash_block2((int)x+8,(int)y+hysz-8);
		    Link.check_slash_block2((int)x+8,(int)y+hysz);
		}
		
		if(dir==left)
		{
		    Link.check_slash_block2((int)x,(int)y+8);
		    Link.check_slash_block2((int)x+8,(int)y+8);
		}
		
		if(dir==right)
		{
		    Link.check_slash_block2((int)x+hxsz,(int)y+8);
		    Link.check_slash_block2((int)x+hxsz-8,(int)y+8);
		}
		
	    }*/
	    
		for(int dx = 0; dx < hxsz; dx += 16)
		{
			for(int dy = 0; dy < hysz; dy += 16)
			{
				Link.check_slash_block2((int)x+dx+hxofs, (int)y+dy+hyofs, this);
				//Layers
				//1
				Link.check_slash_block_layer2((int)x+dx+hxofs, (int)y+dy+hyofs, this,1);
				//2
				Link.check_slash_block_layer2((int)x+dx+hxofs, (int)y+dy+hyofs, this,2);
				
				Link.check_wand_block2((int)x+dx+hxofs, (int)y+dy+hyofs, this);
				Link.check_pound_block2((int)x+dx+hxofs, (int)y+dy+hyofs, this);
				Link.check_wpn_triggers((int)x+dx+hxofs, (int)y+dy+hyofs, this);
			}
			Link.check_slash_block2((int)x+dx+hxofs, (int)y+hyofs+(hysz-1), this);
			Link.check_slash_block_layer2((int)x+dx+hxofs, (int)y+hyofs+(hysz-1), this,1);
			Link.check_slash_block_layer2((int)x+dx+hxofs, (int)y+hyofs+(hysz-1), this,2);
			Link.check_wand_block2((int)x+dx+hxofs, (int)y+hyofs+(hysz-1), this);
			Link.check_pound_block2((int)x+dx+hxofs, (int)y+hyofs+(hysz-1), this);
			Link.check_wpn_triggers((int)x+dx+hxofs, (int)y+hyofs+(hysz-1), this);
		}
		for(int dy = 0; dy < hysz; dy += 16)
		{
			Link.check_slash_block2((int)x+hxofs+(hxsz-1), (int)y+dy+hyofs, this);
			Link.check_slash_block_layer2((int)x+hxofs+(hxsz-1), (int)y+dy+hyofs, this,1);
			Link.check_slash_block_layer2((int)x+hxofs+(hxsz-1), (int)y+dy+hyofs, this,2);
			Link.check_wand_block2((int)x+hxofs+(hxsz-1), (int)y+dy+hyofs, this);
			Link.check_pound_block2((int)x+hxofs+(hxsz-1), (int)y+dy+hyofs, this);
			Link.check_wpn_triggers((int)x+hxofs+(hxsz-1), (int)y+dy+hyofs, this);
		}
		Link.check_slash_block2((int)x+hxofs+(hxsz-1), (int)y+hyofs+(hysz-1), this);
		Link.check_slash_block_layer2((int)x+hxofs+(hxsz-1), (int)y+hyofs+(hysz-1), this,1);
		Link.check_slash_block_layer2((int)x+hxofs+(hxsz-1), (int)y+hyofs+(hysz-1), this,2);
		Link.check_wand_block2((int)x+hxofs+(hxsz-1), (int)y+hyofs+(hysz-1), this);
		Link.check_pound_block2((int)x+hxofs+(hxsz-1), (int)y+hyofs+(hysz-1), this);
		Link.check_wpn_triggers((int)x+hxofs+(hxsz-1), (int)y+hyofs+(hysz-1), this);
	        findcombotriggers();
		/* Don't check every single pixel.
		for ( int w = 0; q < hysz; q++ )
		{
			for ( int q = 0; w < hxsz; w++ )
			{
				Link.check_slash_block2((int)x+(int)hxofs+q,(int)y+(int)hyofs+w, this);
				Link.check_wand_block2((int)x+(int)hxofs+q,(int)y+(int)hyofs+w, this);
				Link.check_pound_block2((int)x+(int)hxofs+q,(int)y+(int)hyofs+w, this);
				Link.check_wpn_triggers((int)x+(int)hxofs+q,(int)y+(int)hyofs+w, this);
			}
		}
		*/
	    //Link.check_slash_block(this); //Activates triggers for slash combos if the weapon is the correct type, or is
					  //acting as the correct type with 'useweapon'.
					  //Non-script-generated eweapons should be safe.
	    
	    //Link.check_wand_block(this);
	    //Link.check_pound_block(this);
    }
    // fall down
	if ( moveflags & FLAG_OBEYS_GRAV ) // from above, or if scripted
	{
		if(isSideViewGravity())
		{
		    if(!_walkflag(x,y+16,0))
		    {
			y+=fall/100;
			
			if(fall <= (int)zinit.terminalv)
			{
			    fall += zinit.gravity;
			}
		    }
		    else
		    {
			if(fall!=0 && !(step>0 && dir==up))  // Don't fix pos if still moving through solidness
			    y-=(int)y%8; // Fix position
			    
			fall = 0;
		    }
		    
		    if(y>192) dead=0;  // Out of bounds
		}
		else
		{
		    z-=fall/100;
		    
		    if(z<=0)
		    {
				z = fall = 0;
		    }
		    else if(fall <= (int)zinit.terminalv)
		    {
				fall += zinit.gravity;
		    }
		}
	}
	if(moveflags & FLAG_CAN_PITFALL)
	{
		switch(id)
		{
			case wSword:
			case wWand:
			case wCByrna:
			case wHammer:
			case wHookshot:
			case wWhistle:
			case wFSparkle:
			case wHSChain:
			case wHSHandle:
			case wSSparkle:
			case wStomp:
			case wSmack:
				break;
			default:
				if(z <= 0)
				{
					fallCombo = check_pits();
				}
		}
	}
	if(moveflags & FLAG_CAN_WATERDROWN)
	{
		switch(id)
		{
			case wSword:
			case wWand:
			case wCByrna:
			case wHammer:
			case wHookshot:
			case wWhistle:
			case wFSparkle:
			case wHSChain:
			case wHSHandle:
			case wSSparkle:
			case wStomp:
			case wSmack:
				break;
			default:
				if(z <= 0)
				{
					drownCombo = check_water();
				}
		}
	}
    
    if(id>wEnemyWeapons && id!=ewBrang && id != ewFireball2 && id != ewBomb && id != ewSBomb)
        switch(misc)
        {
        case up:
            y-=.5;
            break;
            
        case down:
            y+=.5;
            break;
            
        case left:
            x-=.5;
            break;
            
        case right:
            x+=.5;
            break;
            
        case 4:
            y-=1;
            break;
            
        case 5:
            y+=1;
            break;
            
        case 6:
            x-=1;
            break;
            
        case 7:
            x+=1;
            break;
            //case l_up:  y-=.354; x-=.354; break;
            //case r_up:  y-=.354; x+=.354; break;
            //case l_down:y+=.354; x-=.354; break;
            //case r_down:y+=.354; x+=.354; break;
        }
        
    switch(id)
    {
	case wScript1:
	    case wScript2:
	    case wScript3:
	    case wScript4:
	    case wScript5:
	    case wScript6:
	    case wScript7:
	    case wScript8:
	    case wScript9:
	    case wScript10:
	    {
		if ( ScriptGenerated && !isLWeapon ) break; //Return early for eweapons. We handle those elsewhere. 
		//Z_scripterrlog("Script LWeapon Type (%d) has a weapon script of: %d\n", id, weaponscript);
		if ( parentitem > -1 || (isLWeapon && ScriptGenerated) )
		{
		    //Z_scripterrlog("Script LWeapon Type (%d) has a weapon script of: %d\n", id, weaponscript);
		    if ( doscript && FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());	
		}
		
		//Can EW_SCRIPT run a weapon script ?? -Z
		
		//Disabled for now because FFCore.eweaponScriptEngine() runs the enemy weapon scripts. -Z (25-Dec-2018)
		//else if ( parentid > -1 && parentid != Link.getUID() && !isLWeapon )
		//{
			
		//	if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_EWPN, weaponscript, getUID());	
		//}
		/*
                if ( doscript && weaponscript > 0 ) 
                {
			if ( Dead() )
			{
				doscript = 0;
				weaponscript = 0;
			}
			else
			{
				//al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
				//FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, index);		
				FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());		
			}
		}
		    */
		break;
	    }
        // Link's weapons
    case wSword:
	if ( doscript && itemsbuf[parentitem].misc10 == 50 )
	{
		if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS]) ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	}
    case wWand:
    case wHammer:
        if(LinkAction()!=attacking && LinkAction()!=ischarging && !LinkCharged())
        {
            dead=0;
        }
        
        break;
        
    case wCByrna:
    {
        if(blocked())
        {
            dead=0;
        }
        
        int speed = parentitem>-1 ? zc_max(itemsbuf[parentitem].misc1,1) : 1;
        int radius = parentitem>-1 ? zc_max(itemsbuf[parentitem].misc2,8) : 8;
        double xdiff = -(sin((double)clk/speed) * radius);
        double ydiff = (cos((double)clk/speed) * radius);
        
        double ddir=atan2(double(ydiff),double(xdiff));
        
        if((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
            dir=r_up;
        else if((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
            dir=right;
        else if((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
            dir=r_down;
        else if((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
            dir=down;
        else if((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
            dir=l_down;
        else if((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
            dir=left;
        else if((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
            dir=l_up;
        else
            dir=up;
            
        x = (zfix)((double)LinkX() + xdiff);
        y = (zfix)((double)LinkY() + ydiff);
        z = LinkZ();
        
	if(parentitem>-1 && dead != 1) //Perhaps don't play the sound if the weapon is dead?
	{
            sfx(itemsbuf[parentitem].usesound,pan(int(x)),true,false);
	}
	if ( doscript )
	{
		if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS]) ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	}
    }
    
    break;
    
    case wBeam:
    case wRefBeam:
    {
        for(int i2=0; i2<=zc_min(type-1,3) && dead!=23; i2++)
        {
            if(findentrance(x,y,mfSWORDBEAM+i2,true)) dead=23;
        }
        
        if(blocked())
        {
            dead=23;
        }		 
        
    }
    case ewSword:
        if(blocked())
        {
            dead=0;
        }
        
        if(id==ewSword && get_bit(quest_rules,qr_SWORDMIRROR) || id!=ewSword && (parentitem > -1 ? itemsbuf[parentitem].flags & ITEM_FLAG9 : get_bit(quest_rules,qr_SWORDMIRROR))) //TODO: First qr_SWORDMIRROR port to enemy weapon flag, second qr_SWORDMIRROR port to script default flag -V
        {
            int checkx=0, checky=0;
            
            switch(dir)
            {
            case up:
                checkx=x+7;
                checky=y+8;
                break;
                
            case down:
                checkx=x+7;
                checky=y;
                break;
                
            case left:
                checkx=x+8;
                checky=y+7;
                break;
                
            case right:
                checkx=x;
                checky=y+7;
                break;
            }
            
            if(ignorecombo==(((int)checky&0xF0)+((int)checkx>>4)))
                break;
                
		int posx, posy;
		if(get_bit(quest_rules,qr_OLDMIRRORCOMBOS))//Replace this conditional with an ER; true if the ER is checked. This will use the old (glitchy) behavior for sword beams.
		{
			posx=x;
			posy=y;
		}
		else
		{
			posx=checkx;
			posy=checky;
		}
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMIRROR))
            {
                id = wRefBeam;
                dir ^= 1;
                
                if(dir&2)
                    flip ^= 1;
                else
                    flip ^= 2;
                    
                ignoreLink=false;
                ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                y=(int)posy&0xF0;
                x=(int)posx&0xF0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMIRRORSLASH))
            {
                id = wRefBeam;
		if ( do_animation ) 
		{
			dir = 3-dir;
			{
			    if(dir==right)
				flip &= ~1; // not horiz
			    else if(dir==left)
				flip |= 1;  // horiz
			    else if(dir==up)
				flip &= ~2; // not vert
			    else if(dir==down)
				flip |= 2;  // vert
			}
			tile=o_tile;
			
			if(dir&2)
			{
			    if(frames>1)
			    {
				tile+=frames;
			    }
			    else
			    {
				++tile;
			    }
			}
                }
                ignoreLink=false;
                ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                y=(int)posy&0xF0;
                x=(int)posx&0xF0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMIRRORBACKSLASH))
            {
                id = wRefBeam;
                dir ^= 2;
                {
			if ( do_animation ) 
			{
			    if(dir==right)
				flip &= ~1; // not horiz
			    else if(dir==left)
				flip |= 1;  // horiz
			    else if(dir==up)
				flip &= ~2; // not vert
			    else if(dir==down)
				flip |= 2;  // vert
			}
                }
		if ( do_animation ) 
		{
			tile=o_tile;
			
			if(dir&2)
			{
			    if(frames>1)
			    {
				tile+=frames;
			    }
			    else
			    {
				++tile;
			    }
			}
                }
                ignoreLink=false;
                ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                y=(int)posy&0xF0;
                x=(int)posx&0xF0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMAGICPRISM))
            {
                int newx, newy;
                newy=(int)posy&0xF0;
                newx=(int)posx&0xF0;
                
                for(int tdir=0; tdir<4; tdir++)
                {
                    if(dir!=(tdir^1))
                    {
                        weapon *w=new weapon(*this);
                        w->dir=tdir;
			if ( this->angular && get_bit(quest_rules, qr_ANGULAR_REFLECTED_WEAPONS) )
			{
				double newangle = this->angle + DegreesToRadians(90*tdir);
				w->angle = WrapAngle(newangle);
			}
                        w->x=newx;
                        w->y=newy;
                        w->z=z;
                        w->id=wRefBeam;
                        w->parentid=parentid;
                        w->parentitem=parentitem;
                        w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                        w->hyofs = w->hxofs = 0;
                        //also set up the magic's correct animation -DD
                        w->flip=0;
                        if ( do_animation ) 
			{
				switch(w->dir)
				{
				case down:
				    w->flip=2;
				    
				case up:
				    w->tile = w->o_tile;
				    w->hyofs=2;
				    w->hysz=12;
				    break;
				    
				case left:
				    w->flip=1;
				    
				case right:
				    w->tile=w->o_tile+((w->frames>1)?w->frames:1);
				    w->hxofs=2;
				    w->hxsz=12;
				    break;
				
				default: break;
				}
                        }
                        Lwpns.add(w);
                    }
                }
                
                dead=0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMAGICPRISM4))
            {
                int newx, newy;
                newy=(int)posy&0xF0;
                newx=(int)posx&0xF0;
                
                for(int tdir=0; tdir<4; tdir++)
                {
                    weapon *w=new weapon(*this);
                    w->dir=tdir;
		    if ( this->angular && get_bit(quest_rules, qr_ANGULAR_REFLECTED_WEAPONS) )
		    {
			double newangle = this->angle + DegreesToRadians(90*tdir);
			w->angle = WrapAngle(newangle);
		    }
                    w->x=newx;
                    w->y=newy;
                    w->z=z;
                    w->id=wRefBeam;
                    w->parentid=parentid;
                    w->parentitem=parentitem;
                    w->hyofs = w->hxofs = 0;
                    w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                    //also set up the magic's correct animation -DD
                    w->flip=0;
                    if ( do_animation ) 
		    {
			    switch(w->dir)
			    {
			    case down:
				w->flip=2;
				
			    case up:
				w->tile = w->o_tile;
				w->hyofs=2;
				w->hysz=12;
				break;
				
			    case left:
				w->flip=1;
				
			    case right:
				w->tile=w->o_tile+((w->frames>1)?w->frames:1);
				w->hxofs=2;
				w->hxsz=12;
				break;
			    
			    default: break;
			    }
                    }
                    Lwpns.add(w);
                }
                
                dead=0;
            }
        }
        
	
	if ( ( id == wRefBeam && ScriptGenerated )  || id == wBeam )
	{
		if ( doscript )
		{
			if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS]) ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
		}
	}
	if ( id == ewSword )
	{
		//eweqapon script here, later
		
	}
	
        break;
        
    case wWhistle:
    {
        if(clk)
        {
            dead=1;
        }
        
	if ( doscript )
	{
		if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	}
        break;
    }
        
    case wWind:
    {
        if(blocked())
        {
            dead=1;
        }
        
        int wrx;
        
        if(get_bit(quest_rules,qr_NOARRIVALPOINT))
            wrx=tmpscr->warpreturnx[0];
        else wrx=tmpscr->warparrivalx;
        
        int wry;
        
        if(get_bit(quest_rules,qr_NOARRIVALPOINT))
            wry=tmpscr->warpreturny[0];
        else wry=tmpscr->warparrivaly;
        
        if(specialinfo==1 && dead==-1 && x==(int)wrx && y==(int)wry)
        {
	    stop_sfx(WAV_ZN1WHIRLWIND);
            dead=2;
        }
        else if(LinkAction() !=inwind && ((dir==right && x>=240) || (dir==down && y>=160) || (dir==left && x<=0) || (dir==up && y<=0)))
        {
            stop_sfx(WAV_ZN1WHIRLWIND);
            dead=1;
        }
        else if(get_bit(quest_rules,qr_MORESOUNDS) && dead < 1)
            sfx(WAV_ZN1WHIRLWIND,pan(int(x)),true,false);
            
        if((parentitem==-1 && get_bit(quest_rules,qr_WHIRLWINDMIRROR)) || (parentitem > -1 && itemsbuf[parentitem].flags & ITEM_FLAG3))
            goto mirrors;
            
	if ( doscript )
	{
		if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	}
        break;
    }
    
    case wFire:
    {
        if(blocked())
        {
            dead=1;
        }
        
        if(parentitem<0 || (parentitem>-1 && itemsbuf[parentitem].family!=itype_book))
        {
            if(clk==32)
            {
                step=0;
                
                if(parentitem<0 || !(itemsbuf[parentitem].flags & ITEM_FLAG2))
                {
                    isLit = true;
                    if((parentitem==-1&&get_bit(quest_rules,qr_TEMPCANDLELIGHT))||itemsbuf[parentitem].flags & ITEM_FLAG5){
				checkLightSources();
			} else {
				checkLightSources(true);
			}
                }
            }
            
            if(clk==94)
            {
                dead=1;
                
                if(((parentitem==-1 && get_bit(quest_rules,qr_TEMPCANDLELIGHT)) ||
		   (parentitem>-1&&!(itemsbuf[parentitem].flags & ITEM_FLAG2)&&(itemsbuf[parentitem].flags & ITEM_FLAG5))) &&
                   (Lwpns.idCount(wFire) + Ewpns.idCount(ewFlame))==1)
                {
                    isLit = false;
                    checkLightSources();
                }
            }
            
            if(clk==94 || get_bit(quest_rules,qr_INSTABURNFLAGS))
            {
                findentrance(x,y,mfBCANDLE,true);
                
                if(type>1) //red candle 
                {
                    findentrance(x,y,mfRCANDLE,true);
                }
                
                if(linked_parent == itype_dinsfire)
                {
                    findentrance(x,y,mfDINSFIRE,true);
                }
            }
        }                                                     //wand fire
        else
        {
            if(clk==1)
            {
                isLit = true;
                if(itemsbuf[parentitem].flags & ITEM_FLAG5){
			checkLightSources();
		} else {
			checkLightSources(true);
		}
            }
            
            if(clk==80)
            {
                dead=1;
                findentrance(x,y,mfBCANDLE,true);
                findentrance(x,y,mfRCANDLE,true);
                findentrance(x,y,mfWANDFIRE,true);
                
                if(((parentitem==-1&&get_bit(quest_rules,qr_TEMPCANDLELIGHT))||(parentitem>-1&&(itemsbuf[parentitem].flags & ITEM_FLAG5))) && (Lwpns.idCount(wFire) + Ewpns.idCount(ewFlame))==1)
                {
                    isLit=false;
                    checkLightSources();
                }
            }
        }
        
        // Killed by script?
        if(dead==0 && ((parentitem==-1&&get_bit(quest_rules,qr_TEMPCANDLELIGHT))||(parentitem>0&&(itemsbuf[parentitem].flags & ITEM_FLAG5))) && (Lwpns.idCount(wFire) + Ewpns.idCount(ewFlame))==1)
        {
            isLit=false;
            checkLightSources();
        }
        if ( doscript )
	{
		if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	}
        break;
    }
    
    case wLitBomb:
    case wBomb:
    case wLitSBomb:
    case wSBomb:
    {
	if ( doscript )
	{
		if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	}
        if(!misc)
        {
            break;
        }
        
        // Naaah.
        /*if (blocked())
        {
          dead=1;
        }*/
        if(clk==(misc-2) && step==0)
        {
            id = (id>wEnemyWeapons ? (id==ewLitSBomb||id==ewSBomb ? ewSBomb : ewBomb)
                      : parentitem>-1 ? ((itemsbuf[parentitem].family==itype_sbomb) ? wSBomb:wBomb)
                      : (id==wLitSBomb||id==wSBomb ? wSBomb : wBomb));
            hxofs=2000;
        }
        
        if(clk==(misc-1) && step==0)
    {
            sfx((id>=wEnemyWeapons || parentitem<0) ? WAV_BOMB :
                itemsbuf[parentitem].usesound,pan(int(x)));
                
            if(id==wSBomb || id==wLitSBomb || id==ewSBomb || id==ewLitSBomb)
            {
                hxofs=hyofs=-16;
                hxsz=hysz=48;
            }
            else
            {
                hxofs=hyofs=-8;
                hxsz=hysz=32;
            }
            
            hzsz=16;
        }
        
        int boomend = (misc+(((id == wBomb || id == wSBomb || id == wLitBomb || id == wLitSBomb) &&
                              (parentitem>-1 && itemsbuf[parentitem].flags & ITEM_FLAG1)) ? 35 : 31));
                              
        if(clk==boomend && step==0)
        {
            hxofs=2000;
        }
        
        if(id<wEnemyWeapons)
        {
            if(clk==(misc-1))
            {
                int f1 = (id==wSBomb || id==wLitSBomb) ? 16 : 0; // Large SBomb triggerbox
                
                for(int tx=-f1; tx<=f1; tx+=8)  // -16,-8,0,8,16
                {
                    int f2 = 0;
                    
                    if(tx==-8 || tx==8)
                        f2 = f1;
                        
                    for(int ty=-f2; ty<=f2; ty+=32)
                    {
                        findentrance(x+tx,y+ty+(isSideViewGravity()?2:-3),mfBOMB,true);
                        
                        if(id==wSBomb || id==wLitSBomb)
                        {
                            findentrance(x+tx,y+ty+(isSideViewGravity()?2:-3),mfSBOMB,true);
                        }
                        
                        findentrance(x+tx,y+ty+(isSideViewGravity()?2:-3),mfSTRIKE,true);
                    }
                }
            }
            
            if(!get_bit(quest_rules,qr_NOBOMBPALFLASH) && !epilepsyFlashReduction)
            {
                if(!usebombpal)
                {
                    if(clk==misc || clk==misc+5)
                    {
                    
                        usebombpal=true;
                        memcpy(tempbombpal, RAMpal, PAL_SIZE*sizeof(RGB));
                        
                        //grayscale entire screen
                        if(get_bit(quest_rules,qr_FADE))
                        {
                            for(int i=CSET(0); i < CSET(15); i++)
                            {
                                int g = zc_min((RAMpal[i].r*42 + RAMpal[i].g*75 + RAMpal[i].b*14) >> 7, 63);
                                g = (g >> 1) + 32;
                                RAMpal[i] = _RGB(g,g,g);
                            }
                            
                        }
                        else
                        {
                            // this is awkward. NES Z1 converts colors based on the global
                            // NES palette. Something like RAMpal[i] = NESpal( reverse_NESpal(RAMpal[i]) & 0x30 );
                            for(int i=CSET(0); i < CSET(15); i++)
                            {
                                RAMpal[i] = NESpal(reverse_NESpal(RAMpal[i]) & 0x30);
                            }
                        }
                        
                        refreshpal = true;
                    }
                }
                
                if((clk==misc+4 || clk==misc+9) && usebombpal)
                {
                    // undo grayscale
                    usebombpal=false;
                    memcpy(RAMpal, tempbombpal, PAL_SIZE*sizeof(RGB));
                    refreshpal = true;
                }
            }
            
            if(clk==misc+30)
            {
                bombdoor(x,y);
            }
        }
        
        if(clk==misc+34)
        {
            if(step==0)
            {
                dead=1;
            }
        }
        //if ( doscript )
	//{
	//	FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	//}
        break;
    }
        case ewLitBomb:
    case ewBomb:
    case ewLitSBomb:
    case ewSBomb:
    {
        if(!misc)
        {
            break;
        }
        
        // Naaah.
        /*if (blocked())
        {
          dead=1;
        }*/
        if(clk==(misc-2) && step==0)
        {
            id = (id>wEnemyWeapons ? (id==ewLitSBomb||id==ewSBomb ? ewSBomb : ewBomb)
                      : parentitem>-1 ? ((itemsbuf[parentitem].family==itype_sbomb) ? wSBomb:wBomb)
                      : (id==wLitSBomb||id==wSBomb ? wSBomb : wBomb));
            hxofs=2000;
        }
        
        if(clk==(misc-1) && step==0)
    {
            sfx((id>=wEnemyWeapons || parentitem<0) ? WAV_BOMB :
                itemsbuf[parentitem].usesound,pan(int(x)));
                
            if(id==wSBomb || id==wLitSBomb || id==ewSBomb || id==ewLitSBomb)
            {
                hxofs=hyofs=-16;
                hxsz=hysz=48;
            }
            else
            {
                hxofs=hyofs=-8;
                hxsz=hysz=32;
            }
            
            hzsz=16;
        }
        
        int boomend = (misc+(((id == wBomb || id == wSBomb || id == wLitBomb || id == wLitSBomb) &&
                              (parentitem>-1 && itemsbuf[parentitem].flags & ITEM_FLAG1)) ? 35 : 31));
                              
        if(clk==boomend && step==0)
        {
            hxofs=2000;
        }
        
        if(id<wEnemyWeapons)
        {
            if(clk==(misc-1))
            {
                int f1 = (id==wSBomb || id==wLitSBomb) ? 16 : 0; // Large SBomb triggerbox
                
                for(int tx=-f1; tx<=f1; tx+=8)  // -16,-8,0,8,16
                {
                    int f2 = 0;
                    
                    if(tx==-8 || tx==8)
                        f2 = f1;
                        
                    for(int ty=-f2; ty<=f2; ty+=32)
                    {
                        findentrance(x+tx,y+ty+(isSideViewGravity()?2:-3),mfBOMB,true);
                        
                        if(id==wSBomb || id==wLitSBomb)
                        {
                            findentrance(x+tx,y+ty+(isSideViewGravity()?2:-3),mfSBOMB,true);
                        }
                        
                        findentrance(x+tx,y+ty+(isSideViewGravity()?2:-3),mfSTRIKE,true);
                    }
                }
            }
            
            if(!get_bit(quest_rules,qr_NOBOMBPALFLASH))
            {
                if(!usebombpal)
                {
                    if(clk==misc || clk==misc+5)
                    {
                    
                        usebombpal=true;
                        memcpy(tempbombpal, RAMpal, PAL_SIZE*sizeof(RGB));
                        
                        //grayscale entire screen
                        if(get_bit(quest_rules,qr_FADE))
                        {
                            for(int i=CSET(0); i < CSET(15); i++)
                            {
                                int g = zc_min((RAMpal[i].r*42 + RAMpal[i].g*75 + RAMpal[i].b*14) >> 7, 63);
                                g = (g >> 1) + 32;
                                RAMpal[i] = _RGB(g,g,g);
                            }
                            
                        }
                        else
                        {
                            // this is awkward. NES Z1 converts colors based on the global
                            // NES palette. Something like RAMpal[i] = NESpal( reverse_NESpal(RAMpal[i]) & 0x30 );
                            for(int i=CSET(0); i < CSET(15); i++)
                            {
                                RAMpal[i] = NESpal(reverse_NESpal(RAMpal[i]) & 0x30);
                            }
                        }
                        
                        refreshpal = true;
                    }
                }
                
                if((clk==misc+4 || clk==misc+9) && usebombpal)
                {
                    // undo grayscale
                    usebombpal=false;
                    memcpy(RAMpal, tempbombpal, PAL_SIZE*sizeof(RGB));
                    refreshpal = true;
                }
            }
            
            if(clk==misc+30)
            {
                bombdoor(x,y);
            }
        }
        
        if(clk==misc+34)
        {
            if(step==0)
            {
                dead=1;
            }
        }
	//eweapon script, again, later
       //if ( doscript )
	// {
	//	FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	///}
        break;
    }
    
    case wArrow:
    {
	//Z_scripterrlog("Arrow weaponscript is: %d\n", weaponscript);
	if ( doscript )
	{
		if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS]) ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	}
        if(dead>0)
        {
	    
            break;
        }
        
        if(misc>0 && clk > misc)
        {
            dead=4;
        }
        
        if(findentrance(x,y,mfSTRIKE,true))
        {
            dead=4;
        }
        
        if(findentrance(x,y,mfARROW,true))
        {
            dead=4;
        }
        
        if(current_item(itype_arrow)>1)
        {
            if(findentrance(x,y,mfSARROW,true))
            {
                dead=4;
            }
        }
        
        if(current_item(itype_arrow)>=3)
        {
            if(findentrance(x,y,mfGARROW,true))
            {
                dead=4;
            }
        }
        
        if(blocked())
        {
            dead=4;
        }
        
        break;
    }
    
    case wSSparkle:
    {
        if(blocked())  //no spakle area?
        {
            dead=1;
        }
        
        if(clk>=frames*o_speed-1) //(((wpnsbuf[wSSPARKLE].frames) * (wpnsbuf[wSSPARKLE].speed))-1))
        {
            dead=1;
        }
	
	if ( doscript && isLWeapon )
	{
		if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	}
        
        break;
    }
        
    case wFSparkle:
    {
        if(blocked())  //no sparkle area?
        {
            dead=1;
        }
        
        if(clk>=frames*o_speed-1) //(((wpnsbuf[wFSPARKLE].frames) * (wpnsbuf[wFSPARKLE].speed))-1))
        {
            dead=1;
        }
	
	if ( doscript && isLWeapon )
	{
		if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	}
        
        break;
    }
    case wBait:
    {
        if(blocked())  //no bait area?
        {
	    if ( doscript )
	    {
		   if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	    }
            dead=23;
	    goto skip_second_bait_script;
        }
        
        if(parentitem>-1 && clk>=itemsbuf[parentitem].misc1)
        {
            dead=1;
        }
        if ( doscript )
	{
		   if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS]) ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	}
	skip_second_bait_script:
        break;
    }
    case wBrang:
    {
	    //run first? brang scripts were being killed on WDS_BOUNCE, so this may fix that.
	
        if(dead==0)  // Set by ZScript
        {
            stop_sfx(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].usesound);
	    if ( doscript )
	    {
		   if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	    }
            break;
        }
        
        else if(dead==1) // Set by ZScript
        {
	    if ( doscript )
	    {
		   if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	    }
            onhit(false);
        }
	else
	{
	    if ( doscript )
	    {
		   if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS]) ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	    }
	}
	
        
        int deadval=(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].flags & ITEM_FLAG3)?-2:1;
        
	/* This causes brang weapons with a level > 3 to hit OTHER flags. 
	// e.g., a brang with a level of 5 would trigger mfBRANG through mfGARROW! -Z
        for(int i=0; i<current_item(itype_brang); i++)
        {
            if(findentrance(x,y,mfBRANG+i,true)) dead=deadval;
        }
	*/
	
	int branglevel = itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].fam_type;
	
	switch ( branglevel )
	{
		case 0:
		case 1:
		{
			if(findentrance(x,y,mfBRANG,true)) dead=deadval; break;
		}
		case 2: 
		{
			if(findentrance(x,y,mfBRANG,true)) dead=deadval;
			if(findentrance(x,y,mfMBRANG,true)) dead=deadval;
			break;
		}
		case 3:
		{
			goto brang_level_3_or_higher;
		}
		default: //level higher than 3
		{
			goto brang_level_3_or_higher;
		}
		brang_level_3_or_higher: 
		{
			if(findentrance(x,y,mfBRANG,true)) dead=deadval;
			if(findentrance(x,y,mfMBRANG,true)) dead=deadval;
			if(findentrance(x,y,mfFBRANG,true)) dead=deadval;
			break;
		}
	}
		
        
        if(findentrance(x,y,mfSTRIKE,true)) dead=deadval;
		if((itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].flags & ITEM_FLAG8) && findentrance(x,y,mfBCANDLE,true)) dead=deadval;
        
        if(blocked())
        {
            dead=deadval;
            onhit(false);
        }
        
        ++clk2;
        int range = itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].misc1;
        
        if(range && clk2>=range)
        {
            if(deadval==1)
            {
                misc=1;
            }
            else
            {
                dead=deadval;
            }
        }
        
        if(range && clk2>range-18 && clk2<range+16)
        {
            step=1;
        }
        else if(misc)
        {
            step=2;
        }
        else
        {
            step=3;
        }
        
        if(clk==0)                                            // delay a frame
        {
            ++clk;
            sfx(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].usesound,pan(int(x)),true);
	    //if ( doscript )
	    //{
		//   FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	    //}
            return false;
        }
        
        if(clk==1)                                            // then check directional input
        {
            if(Up())
            {
                dir=up;
                
                if(Left())  dir=l_up;
                
                if(Right()) dir=r_up;
            }
            else if(Down())
            {
                dir=down;
                
                if(Left())  dir=l_down;
                
                if(Right()) dir=r_down;
            }
            else if(Left())
            {
                dir=left;
            }
            else if(Right())
            {
                dir=right;
            }
        }
        
        if(dead==1)
        {
            dead=-1;
            misc=1;
        }
        
        if(dead==-2)
        {
            misc=1;
        }
        
        if(misc==1)                                           // returning
        {
            if((abs(LinkY()-y)<7 && abs(LinkX()-x)<7)||dead==-2)
            {
                if(dead!=-2)
                {
                    CatchBrang();
                }
                
                if(Lwpns.idCount(wBrang)<=1 && (!get_bit(quest_rules, qr_MORESOUNDS) || !Ewpns.idCount(ewBrang)))
                    stop_sfx(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].usesound);
                    
                /*if (dummy_bool[0])
                {
                    add_grenade(x,y,z,0,-1); //TODO: Super bombs as well?
                    dummy_bool[0]=false;
                }*/
                if(dragging!=-1)
                {
                    getdraggeditem(dragging);
                }
                
                return true;
            }
            
            seekLink();
        }
        //call before the sfx
        
        //if ( doscript )
	//{
		//Z_scripterrlog("Engine Brang DeadState is: %d\n",dead);
	//	FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	//}
        sfx(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].usesound,pan(int(x)),true,false);
        
        break;
    }
    
    case wHookshot:
    {
        if(dead==0)  // Set by ZScript
        {
            hookshot_used = false;
            
            for(int i=0; i<chainlinks.Count(); i++)
            {
                chainlinks.del(chainlinks.idFirst(wHSChain));
            }
        }
	//Diagonal Hookshot (8)
        byte allow_diagonal = (itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].flags & ITEM_FLAG2) ? 1 : 0; 
	//zprint2("allow_diagonal: %s\n", allow_diagonal ? "true" : "false");
	//if ( allow_diagonal && misc2 == 0 ) 
	if(clk==0 && allow_diagonal)                                            // delay a frame ere setting a dir
        {
            ++clk;
            return false;
        }
        //Diagonal Hookshot (10)
	//Sprites wpn5: Head, diagonal
	//	  wpn6: handle, diagonal
	//	  wpn7: chainlink, diagonal
	//This sets the direction for digaonals based on controller input. 
        if(clk==1 && allow_diagonal)    
	{
		//zprint2("(int)(Link.dir): %d\n", (int)(Link.dir));
	    //zprint2("clk is 1\n");
	    if(Up())
	    {
		//dir=up; //Up would already have been set if facing up.
		//zprint2("UP\n");
		if(Left() )  
		{
			LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn5);
			dir=l_up;
			update_weapon_frame(((frames>1)?frames:0),o_tile);
			flip=0;
			switch((int)(Link.dir))
			{
				case up:
					yofs += 7;
					xofs -= 2;
					break;
				case left:
					yofs -= 5;
					xofs += 5;
					break;
			}
			
			//zprint2("LEFT\n");
		}
		
		else if(Right() ) 
		{
			LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn5);
			dir=r_up;
			update_weapon_frame(((frames>1)?frames:0),o_tile);
			flip=1;
			
			switch((int)(Link.dir))
			{
				case up:
					yofs += 7;
					xofs -= 0;
					break;
				case right:
					yofs -= 8;
					xofs -= 3;
					break;
			}
			
			
			//zprint2("RIGHT\n");
		}
		    misc2 = 1; //to prevent wagging it all over the screen, we set it once. 
	    }
	    else if(Down())
	    {
		//zprint2("DOWN\n");
		//dir=down; //Up would already have been set if facing down.
		
		if(Left() )  
		{
			LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn5);
			dir=l_down;
			update_weapon_frame(((frames>1)?frames:0),o_tile);
			flip=2;
			switch((int)(Link.dir))
			{
				case down:
					yofs -= 5;
					xofs -= 2;
					break;
				case left:
					yofs -= 2;
					xofs += 5;
					break;
			}
			
			//zprint2("LEFT\n");
		}
		
		else if(Right() ) 
		{
			LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn5);
			dir=r_down;
			update_weapon_frame(((frames>1)?frames:0),o_tile);
			flip=3;
			switch((int)(Link.dir))
			{
				case down:
					yofs -= 8;
					xofs += 1;
					break;
				case right:
					yofs += 2;
					xofs -= 3;
					break;
			}
			
			//zprint2("RIGHT\n");
		}
		     misc2 = 1; //to prevent wagging it all over the screen, we set it once. 
	    }
	}
        
        // Hookshot grab and retract code 
	//Diagonal Hookshot (2)
	
        if(misc==0)
        {
            int maxlength=parentitem>-1 ? 16*itemsbuf[parentitem].misc1 : 0;
            //If the hookshot has extended to maxlength, retract it.
	    //Needa an option to measure in pixels, instead of tiles. -Z
            if((abs(LinkX()-x)>maxlength)||(abs(LinkY()-y)>maxlength))
            {
                dead=1;
            }
            //If it hits a block object, retract it.
            if(findentrance(x,y,mfSTRIKE,true)) dead=1;
            
            if(findentrance(x,y,mfHOOKSHOT,true)) dead=1;
	    
            //Look for grab combos based on direction.
            if(dir==up)
            {
                if(isHSGrabbable(combobuf[MAPCOMBO(x+2,y+7)]))
                {
                    hooked=true;
                }
                
                if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))
                    hooked = hooked || isHSGrabbable(combobuf[MAPCOMBO2(0,x+2,y+7)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+2,y+7)]);
                    
                if(!hooked && _walkflag(x+2,y+7,1) && !ishookshottable((int)x+2,(int)y+7))
                {
                    dead=1;
                }
            }
            
            if(dir==down)
            {
                if(isHSGrabbable(combobuf[MAPCOMBO(x+12,y+12)]))
                {
                    hooked=true;
                }
                
                if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))
                    hooked = hooked || isHSGrabbable(combobuf[MAPCOMBO2(0,x+12,y+12)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+12,y+12)]);
                    
                if(!hooked && _walkflag(x+12,y+12,1) && !ishookshottable((int)x+12,(int)y+12))
                {
                    dead=1;
                }
            }
            
            if(dir==left)
            {
                if(get_bit(quest_rules, qr_OLDHOOKSHOTGRAB))
                {
                    if(isHSGrabbable(combobuf[MAPCOMBO(x+6,y+7)]))
                    {
                        hooked=true;
                    }
                }
                else if(isHSGrabbable(combobuf[MAPCOMBO(x+6,y+13)]))
                {
                    hooked=true;
                }
                
                if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))
                    hooked = hooked || isHSGrabbable(combobuf[MAPCOMBO2(0,x+6,y+13)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+6,y+13)]);
                    
                if(!hooked && _walkflag(x+6,y+13,1) && !ishookshottable((int)x+6,(int)y+13))
                {
                    dead=1;
                }
            }
            
            if(dir==right)
            {
                if(get_bit(quest_rules, qr_OLDHOOKSHOTGRAB))
                {
                    if(isHSGrabbable(combobuf[MAPCOMBO(x+9,y+7)]))
                    {
                        hooked=true;
                    }
                }
                else if(isHSGrabbable(combobuf[MAPCOMBO(x+9,y+13)]))
                {
                    hooked=true;
                }
                
                if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))
                    hooked = hooked || isHSGrabbable(combobuf[MAPCOMBO2(0,x+9,y+13)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+9,y+13)]);
                    
                if(!hooked && _walkflag(x+9,y+13,1) && !ishookshottable((int)x+9,(int)y+13))
                {
                    dead=1;
                }
            }
	    //Diagonal Hookshot (3)
	    //Diagonal Hookshot Grab Points
	    //! -Z Hookshot diagonals. Will need bugtesting galore. 
		if ( dir == r_down ) 
		{
			if(get_bit(quest_rules, qr_OLDHOOKSHOTGRAB))
			{	//right						//down
				if( isHSGrabbable(combobuf[MAPCOMBO(x+9,y+7)]) || isHSGrabbable(combobuf[MAPCOMBO(x+12,y+12)]) )
				{
					hooked=true;
				}
			}

					//right						//down
			else if( ( isHSGrabbable(combobuf[MAPCOMBO(x+9,y+13)])) || isHSGrabbable(combobuf[MAPCOMBO(x+12,y+12)]) )
			{
				hooked=true;
			}

			if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))			//right
			{
				hooked = hooked || isHSGrabbable(combobuf[MAPCOMBO2(0,x+9,y+13)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+9,y+13)]) || 
				//down
				isHSGrabbable(combobuf[MAPCOMBO2(0,x+12,y+12)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+12,y+12)]);
			}
                    
			//right
			if(!hooked &&  ( ( ( _walkflag(x+9,y+13,1) && !ishookshottable((int)x+9,(int)y+13)) ) ||
				//down
				(_walkflag(x+12,y+12,1) && !ishookshottable((int)x+12,(int)y+12)) ) )
			{
			    dead=1;
			}
		    
		    
		}
		if ( dir == l_down ) 
		{
			if(get_bit(quest_rules, qr_OLDHOOKSHOTGRAB))
			{	//left						//down
				if( isHSGrabbable(combobuf[MAPCOMBO(x+6,y+7)]) || isHSGrabbable(combobuf[MAPCOMBO(x+12,y+12)]) )
				{
					hooked=true;
				}
			}
			

					//left						//down
			else if( ( isHSGrabbable(combobuf[MAPCOMBO(x+6,y+13)])) || isHSGrabbable(combobuf[MAPCOMBO(x+12,y+12)]) )
			{
				hooked=true;
			}

			if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))
			{
							//left
				hooked = hooked || isHSGrabbable(combobuf[MAPCOMBO2(0,x+6,y+13)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+6,y+13)]) || 
					//down
				isHSGrabbable(combobuf[MAPCOMBO2(0,x+12,y+12)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+12,y+12)]);
			}
			//left
			if(!hooked && ( ( ( _walkflag(x+6,y+13,1) && !ishookshottable((int)x+6,(int)y+13)) ) ||
				//down
				(_walkflag(x+12,y+12,1) && !ishookshottable((int)x+12,(int)y+12)) ) )
			{
				dead=1;
			}
		    
		    
		}
		if ( dir == r_up ) 
		{
		        if(get_bit(quest_rules, qr_OLDHOOKSHOTGRAB))
			{	//right						//up
			    if( isHSGrabbable(combobuf[MAPCOMBO(x+9,y+7)]) || isHSGrabbable(combobuf[MAPCOMBO(x+2,y+7)]) )
				{
					hooked=true;
				}
			}

					//right						//up
			else if( ( isHSGrabbable(combobuf[MAPCOMBO(x+9,y+13)])) || isHSGrabbable(combobuf[MAPCOMBO(x+2,y+7)]) )
			{
				hooked=true;
			}

			if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))			//right
			{
				hooked = hooked || isHSGrabbable(combobuf[MAPCOMBO2(0,x+9,y+13)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+9,y+13)]) || 
				//up
				isHSGrabbable(combobuf[MAPCOMBO2(0,x+2,y+7)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+2,y+7)]);
			}
			//right
			if(!hooked &&  ( ( ( _walkflag(x+9,y+13,1) && !ishookshottable((int)x+9,(int)y+13)) ) ||
				//up
				(_walkflag(x+2,y+7,1) && !ishookshottable((int)x+2,(int)y+7)) ) )
			{
				dead=1;
			}
		}
		if ( dir == l_up ) 
		{
		        if(get_bit(quest_rules, qr_OLDHOOKSHOTGRAB))
			{	//left						//up
				if( isHSGrabbable(combobuf[MAPCOMBO(x+6,y+7)]) || isHSGrabbable(combobuf[MAPCOMBO(x+2,y+7)]) )
				{
					hooked=true;
				}
			}

					//left						//up
			else if( ( isHSGrabbable(combobuf[MAPCOMBO(x+6,y+13)])) || isHSGrabbable(combobuf[MAPCOMBO(x+2,y+7)]) )
			{
				hooked=true;
			}

			if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))			//left
			{
				hooked = hooked || isHSGrabbable(combobuf[MAPCOMBO2(0,x+6,y+13)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+6,y+13)]) || 
				//up
				isHSGrabbable(combobuf[MAPCOMBO2(0,x+2,y+7)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+2,y+7)]);
			}
							//left
			if(!hooked && ( ( ( _walkflag(x+6,y+13,1) && !ishookshottable((int)x+6,(int)y+13)) ) ||
				//up
				(_walkflag(x+2,y+7,1) && !ishookshottable((int)x+2,(int)y+7)) ) )
			{
				dead=1;
			}
		}
        }
        
        if(hooked==true)
        {
            misc=1;
            pull_link=true;
            step=0;
        }
        
        ++clk2;
        
        if(clk==0)                                            // delay a frame
        {
            ++clk;
            
            if(parentitem>-1)
            {
                sfx(itemsbuf[parentitem].usesound,pan(int(x)),true);
            }
            if ( doscript )
	    {
		   if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	    }
            return false;
        }
        
        if(dead==1)
        {
            dead=-1;
            
            if(step > 0)
                step = -step;
                
            misc=1;
        }
        
        if(misc==1)                                           // returning
        {
            if((dir<left && abs(LinkY()-y)<9) || (dir >= left && abs(LinkX()-x)<9))
            {
                hookshot_used=false;
                
                if(pull_link)
                {
                    hs_fix=true;
                }
                
                pull_link=false;
                chainlinks.clear();
                CatchBrang();
                
                if(parentitem>-1)
                {
                    stop_sfx(itemsbuf[parentitem].usesound);
                }
                
                if(dragging!=-1)
                {
                    getdraggeditem(dragging);
                }
                if ( doscript )
	        {
		   if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	        }
                return true;
            }
        }
        
        if(parentitem>-1)
        {
            sfx(itemsbuf[parentitem].usesound,pan(int(x)),true,false);
        }
        
        if(blocked())
        {
            //not on the return!! -DD
            if(dead != -1)
                dead=1;
        }
    }
    if ( doscript )
    {
		   if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
    }
    break;
    
    case wHSHandle:
    {
        if(hookshot_used==false)
        {
            dead=0;
        }
        
        if(blocked())  //no hookshot handle area?
        {
            hookshot_used=false;
            dead=0;
        }
	//Diagonal Hookshot Handle
        byte allow_diagonal = (itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].flags & ITEM_FLAG2) ? 1 : 0; 
	//zprint2("allow_diagonal: %s\n", allow_diagonal ? "true" : "false");
	//if ( allow_diagonal && misc2 == 0 ) 
	if(clk==0 && allow_diagonal)                                            // delay a frame ere setting a dir
        {
            ++clk;
            return false;
        }
        //Diagonal Hookshot (10)
	//This sets the direction for digaonals based on controller input. 
        if(clk==1 && allow_diagonal)    
	{
		if(Up())
		{
			if(Left() )  
			{
				LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn6);
				dir=l_up;
				update_weapon_frame(((frames>1)?frames:0),o_tile);
				flip=0;
				switch((int)(Link.dir))
				{
					case up:
						yofs += 7;
						xofs += 2;
						break;
					case left:
						yofs -= 1;
						xofs += 6;
						break;
				}
				
				//zprint2("LEFT\n");
			}
			
			else if(Right() ) 
			{
				LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn6);
				dir=r_up;
				update_weapon_frame(((frames>1)?frames:0),o_tile);
				flip=1;
				
				switch((int)(Link.dir))
				{
					case up:
						yofs += 5;
						xofs -= 3;
						break;
					case right:
						yofs -= 0;
						xofs -= 8;
						break;
				}
				
				
				//zprint2("RIGHT\n");
			}
			misc2 = 1; //to prevent wagging it all over the screen, we set it once. 
		}
	
		else if(Down())
		{
			//zprint2("DOWN\n");
			//dir=down; //Up would already have been set if facing down.
			
			if(Left() )  
			{
				LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn6);
				dir=l_down;
				update_weapon_frame(((frames>1)?frames:0),o_tile);
				flip=2;
				switch((int)(Link.dir))
				{
					case down:
						yofs -= 8;
						xofs -= 0;
						break;
					case left:
						yofs -= 6;
						xofs += 5;
						break;
				}
				
				//zprint2("LEFT\n");
			}
			
			else if(Right() ) 
			{
				LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn6);
				dir=r_down;
				update_weapon_frame(((frames>1)?frames:0),o_tile);
				flip=3;
				switch((int)(Link.dir))
				{
					case down:
						yofs -= 8;
						xofs -= 0;
						break;
					case right:
						yofs -= 3;
						xofs -= 5;
						break;
				}
				
				//zprint2("RIGHT\n");
			}
			misc2 = 1; //to prevent wagging it all over the screen, we set it once. 
		}
	}
        break;
    }
    
    
    case wHSChain:
    {
        
	//Diagonal Hookshot Handle
        byte allow_diagonal = (itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].flags & ITEM_FLAG2) ? 1 : 0; 
	//zprint2("allow_diagonal: %s\n", allow_diagonal ? "true" : "false");
	//if ( allow_diagonal && misc2 == 0 ) 
	if(clk==0 && allow_diagonal)                                            // delay a frame ere setting a dir
        {
            ++clk;
            return false;
        }
        //Diagonal Hookshot (10)
	//This sets the direction for digaonals based on controller input. 
        if(clk==1 && allow_diagonal)    
	{
		if(Up())
		{
			if(Left() )  
			{
				LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn7);
				dir=l_up;
				update_weapon_frame(((frames>1)?frames:0),o_tile);
				//flip=0;
				//switch((int)(Link.dir))
				//{
				//	case up:
				//		yofs += 7;
				//		xofs += 2;
				//		break;
				//	case left:
				//		yofs -= 1;
				//		xofs += 6;
				//		break;
				//}
				
				//zprint2("LEFT\n");
			}
			
			else if(Right() ) 
			{
				LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn7);
				dir=r_up;
				update_weapon_frame(((frames>1)?frames:0),o_tile);
				//flip=1;
				
				//switch((int)(Link.dir))
				//{
				//	case up:
				//		yofs += 5;
				//		xofs -= 3;
				//		break;
				//	case right:
				//		yofs -= 0;
				//		xofs -= 8;
				//		break;
				//}
				
				
				//zprint2("RIGHT\n");
			}
			misc2 = 1; //to prevent wagging it all over the screen, we set it once. 
		}
	
		else if(Down())
		{
			//zprint2("DOWN\n");
			//dir=down; //Up would already have been set if facing down.
			
			if(Left() )  
			{
				LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn7);
				dir=l_down;
				update_weapon_frame(((frames>1)?frames:0),o_tile);
				//flip=2;
				//switch((int)(Link.dir))
				//{
				//	case down:
				//		yofs -= 8;
				//		xofs -= 0;
				//		break;
				//	case left:
				//		yofs -= 6;
				//		xofs += 5;
				//		break;
				//}
				
				//zprint2("LEFT\n");
			}
			
			else if(Right() ) 
			{
				LOADGFX(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)].wpn7);
				dir=r_down;
				update_weapon_frame(((frames>1)?frames:0),o_tile);
				//flip=3;
				//switch((int)(Link.dir))
				//{
				///	case down:
				//		yofs -= 8;
				//		xofs -= 0;
				//		break;
				//	case right:
				//		yofs -= 3;
				//		xofs -= 5;
				//		break;
				//}
				
				//zprint2("RIGHT\n");
			}
			misc2 = 1; //to prevent wagging it all over the screen, we set it once. 
		}
	}
        break;
    }
    
    case wPhantom:
    {
        switch(type)
        {
        case pDINSFIREROCKET:
            if(y <= -200)
                dead = 1;
                
            break;
            
        case pDINSFIREROCKETRETURN:                                             //Din's Fire Rocket return
            if(y>=casty)
            {
                dead=1;
                castnext=true;
            }
            
            break;
            
        case pDINSFIREROCKETTRAIL:                                             //Din's Fire Rocket trail
            if(clk>=(((wpnsbuf[wDINSFIRES1A].frames) * (wpnsbuf[wDINSFIRES1A].speed))-1))
            {
                dead=0;
            }
            
            break;
            
        case pDINSFIREROCKETTRAILRETURN:                                             //Din's Fire Rocket return trail
            if(clk>=(((wpnsbuf[wDINSFIRES1B].frames) * (wpnsbuf[wDINSFIRES1B].speed))-1))
            {
                dead=0;
            }
            
            break;
            
        case pNAYRUSLOVEROCKETRETURN1:                                             //Nayru's Love Rocket return
            if(x>=castx)
            {
                dead=1;
                castnext=true;
            }
            
            break;
            
        case pNAYRUSLOVEROCKETTRAIL1:                                             //Nayru's Love Rocket trail
            if(clk>=(((wpnsbuf[wNAYRUSLOVES1A].frames) * (wpnsbuf[wNAYRUSLOVES1A].speed))-1))
            {
                dead=0;
            }
            
            break;
            
        case pNAYRUSLOVEROCKETTRAILRETURN1:                                             //Nayru's Love Rocket return trail
            if(clk>=(((wpnsbuf[wNAYRUSLOVES1B].frames) * (wpnsbuf[wNAYRUSLOVES1B].speed))-1))
            {
                dead=0;
            }
            
            break;
            
        case pNAYRUSLOVEROCKETRETURN2:                                             //Nayru's Love Rocket return
            if(x<=castx)
            {
                dead=0;
                castnext=true;
            }
            
            break;
            
        case pNAYRUSLOVEROCKETTRAIL2:                                             //Nayru's Love Rocket trail
            if(clk>=(((wpnsbuf[wNAYRUSLOVES2A].frames) * (wpnsbuf[wNAYRUSLOVES2A].speed))-1))
            {
                dead=0;
            }
            
            break;
            
        case pNAYRUSLOVEROCKETTRAILRETURN2:                                             //Nayru's Love Rocket return trail
            if(clk>=(((wpnsbuf[wNAYRUSLOVES2B].frames) * (wpnsbuf[wNAYRUSLOVES2B].speed))-1))
            {
                dead=0;
            }
            
            break;
            
        }
        
        if(blocked()) //not really sure this is needed
        {
            dead=1;
        }
        
        break;
    }
        case wRefMagic:
    case wMagic:
    {
	    
	if (this->isLWeapon && (unsigned)linkedItem > 0 )
	{
		//using a book with magic
		if ( ((unsigned)itemsbuf[linkedItem].flags&ITEM_FLAG6) > 0 && ((unsigned)itemsbuf[linkedItem].useweapon) < 128 )
		{
			//change id
			this->id = itemsbuf[linkedItem].useweapon;
			//Step Speed
			int tmpstep = (itemsbuf[linkedItem].misc3);
			//zprint2("initial step: %d\n", tmpstep);
			this->step =  zslongToFix(tmpstep*100);
			//zprint2("true step: %d\n", this->step);
			this->LOADGFX(itemsbuf[linkedItem].wpn3);
			if ( itemsbuf[linkedItem].wpn > 0 )
				this->power = itemsbuf[linkedItem].wpn;
			
		}
	}
			    
        if((id==wMagic)&&(findentrance(x,y,mfWANDMAGIC,true))) dead=0;
        
        if((id==wRefMagic)&&(findentrance(x,y,mfREFMAGIC,true))) dead=0;
        
        if((id!=ewMagic)&&(findentrance(x,y,mfSTRIKE,true))) dead=0;
       
	//Create an ER to use this in older quests -V
	if ( get_bit(quest_rules,qr_BROKENBOOKCOST) )
	{
		
                //al_trace("Reached case wRefMagic in weapons.cpp, line %d\n",3407);
		if((id==wMagic && current_item(itype_book) &&
			itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_book)].flags&ITEM_FLAG1) && get_bit(quest_rules,qr_INSTABURNFLAGS))
		{
		    findentrance(x,y,mfBCANDLE,true);
		    findentrance(x,y,mfRCANDLE,true);
		    findentrance(x,y,mfWANDFIRE,true);
		}
	}
	else
	{
                //al_trace("Reached case wRefMagic in weapons.cpp, line %d\n",3418);
		 if((id==wMagic && linkedItem && itemsbuf[linkedItem].family == itype_book &&
                itemsbuf[linkedItem].flags&ITEM_FLAG1) && get_bit(quest_rules,qr_INSTABURNFLAGS))
		{
		    findentrance(x,y,mfBCANDLE,true);
		    findentrance(x,y,mfRCANDLE,true);
		    findentrance(x,y,mfWANDFIRE,true);
		}
	}
		
        
//mirrors: //the latter instance should suffice
        int checkx=0, checky=0;
        
        switch(dir)
        {
        case up:
            checkx=x+7;
            checky=y+8;
            break;
            
        case down:
            checkx=x+7;
            checky=y;
            break;
            
        case left:
            checkx=x+8;
            checky=y+7;
            break;
            
        case right:
            checkx=x;
            checky=y+7;
            break;
        }
        
        if(ignorecombo!=(((int)checky&0xF0)+((int)checkx>>4)))
        {
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMIRROR))
            {
                weapon *w=NULL;
                
                if(id==ewMagic)
                {
		    w=new weapon(*this);
                    Lwpns.add(w);
                    dead=0;
                }
                else
                {
                    w=this;
                }
                
                
                w->dir ^= 1;
                
                if(w->id != wWind)
                {
                    w->id = wRefMagic; w->convertType(true);
                    
                    if(w->dir&2)
                        w->flip ^= 1;
                    else
                        w->flip ^= 2;
                }
                
                w->ignoreLink=false;
                w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                w->y=checky&0xF0;
                w->x=checkx&0xF0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMIRRORSLASH))
            {
                weapon *w=NULL;
                
                if(id==ewMagic)
                {
		    w=new weapon(*this);
                    Lwpns.add(w);
                    dead=0;
                }
                else
                {
                    w=this; //Oh, look, a memory leak. The new instruction is making something on the heap, but this circumvents removing it. 
                }
                
                w->dir = 3-w->dir;
                
                if(w->id != wWind)
                {
                    w->id = wRefMagic; w->convertType(true);
			if ( do_animation ) 
			{
				if((w->dir==1)||(w->dir==2))
				w->flip ^= 3;
			}
                }
                if ( do_animation ) 
		{
			w->tile=w->o_tile;
			
			if(w->dir&2)
			{
			    if(w->frames>1)
			    {
				w->tile+=w->frames;
			    }
			    else
			    {
				++w->tile;
			    }
			}
                }
                w->ignoreLink=false;
                w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                w->y=checky&0xF0;
                w->x=checkx&0xF0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMIRRORBACKSLASH))
            {
                weapon *w = NULL;
                
                if(id==ewMagic)
                {
		    w=new weapon(*this);
                    Lwpns.add(w);
                    dead=0;
                }
                else
                {
                    w=this;
                }
                
                w->dir ^= 2;
                
                if(w->id != wWind)
                {
                    w->id = wRefMagic; w->convertType(true);
			if ( do_animation ) 
			{
				if(w->dir&1)
					w->flip ^= 2;
				else
					w->flip ^= 1;
			}
                }
                if ( do_animation ) 
		{
			w->tile=w->o_tile;
			
			if(w->dir&2)
			{
			    if(w->frames>1)
			    {
				w->tile+=w->frames;
			    }
			    else
			    {
				++w->tile;
			    }
			}
		}
                
                w->ignoreLink=false;
                w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                w->y=checky&0xF0;
                w->x=checkx&0xF0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMAGICPRISM) && (id != wWind))
            {
                int newx, newy;
                newy=checky&0xF0;
                newx=checkx&0xF0;
                
                for(int tdir=0; tdir<4; tdir++)
                {
                    if(dir!=(tdir^1))
                    {
                        weapon *w=new weapon(*this);
			if ( this->angular && get_bit(quest_rules, qr_ANGULAR_REFLECTED_WEAPONS) )
			{
				double newangle = this->angle + DegreesToRadians(90*tdir);
				w->angle = WrapAngle(newangle);
			}
                        w->dir=tdir;
                        w->x=newx;
                        w->y=newy;
                        w->z=z;
                        w->id=wRefMagic; w->convertType(true);
                        w->parentid=parentid;
                        w->parentitem=parentitem;
                        w->flip = 0;
                        w->ignoreLink = false;
                        w->hyofs = w->hxofs = 0;
                        w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                        if ( do_animation ) 
			{
				//also set up the magic's correct animation -DD
				switch(w->dir)
				{
				case down:
				    w->flip=2;
				    
				case up:
				    w->tile = w->o_tile;
				    w->hyofs=2;
				    w->hysz=12;
				    break;
				    
				case left:
				    w->flip=1;
				    
				case right:
				    w->tile=w->o_tile+((w->frames>1)?w->frames:1);
				    w->hxofs=2;
				    w->hxsz=12;
				    break;
				
				default: break;
				}
                        }
                        Lwpns.add(w);
                    }
                }
                
                dead=0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMAGICPRISM4) && (id != wWind))
            {
                int newx, newy;
                newy=checky&0xF0;
                newx=checkx&0xF0;
                
                for(int tdir=0; tdir<4; tdir++)
                {
                    weapon *w=new weapon(*this);
		    if ( this->angular && get_bit(quest_rules, qr_ANGULAR_REFLECTED_WEAPONS) )
		    {
			double newangle = this->angle - DegreesToRadians(90*tdir);
			w->angle = WrapAngle(newangle);
		    }
                    w->dir=tdir;
                    w->x=newx;
                    w->y=newy;
                    w->z=z;
                    w->id=wRefMagic; w->convertType(true);
                    w->parentid=parentid;
                    w->parentitem=parentitem;
                    w->flip = 0;
                    w->ignoreLink = false;
                    w->hyofs = w->hxofs = 0;
                    w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                    
		    if ( do_animation ) 
		    {
			    //also set up the magic's correct animation -DD
			    switch(w->dir)
			    {
			    case down:
				w->flip=2;
				
			    case up:
				w->tile = w->o_tile;
				w->hyofs=2;
				w->hysz=12;
				break;
				
			    case left:
				w->flip=1;
				
			    case right:
				w->tile=w->o_tile+((w->frames>1)?w->frames:1);
				w->hxofs=2;
				w->hxsz=12;
				break;
			    
			    default: break;
			    }
                    }
                    Lwpns.add(w);
                }
                
                dead=0;
            }
            
            if(blocked(0, 0))
            {
                dead=0;
            }
        }
        //:Weapon Only
        if ( doscript )
	{
		   if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	}
    }
    break;
    case ewMagic:
    {
        if((id==wMagic)&&(findentrance(x,y,mfWANDMAGIC,true))) dead=0;
        
        if((id==wRefMagic)&&(findentrance(x,y,mfREFMAGIC,true))) dead=0;
        
        if((id!=ewMagic)&&(findentrance(x,y,mfSTRIKE,true))) dead=0;
	
	    
	
       
	//Create an ER to use this in older quests -V
	if ( get_bit(quest_rules,qr_BROKENBOOKCOST) )
	{
		
                //al_trace("Reached case wRefMagic in weapons.cpp, line %d\n",3407);
		if((id==wMagic && current_item(itype_book) &&
			itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_book)].flags&ITEM_FLAG1) && get_bit(quest_rules,qr_INSTABURNFLAGS))
		{
		    findentrance(x,y,mfBCANDLE,true);
		    findentrance(x,y,mfRCANDLE,true);
		    findentrance(x,y,mfWANDFIRE,true);
		}
	}
	else
	{
                //al_trace("Reached case wRefMagic in weapons.cpp, line %d\n",3418);
		 if((id==wMagic && linkedItem && itemsbuf[linkedItem].family == itype_book &&
                itemsbuf[linkedItem].flags&ITEM_FLAG1) && get_bit(quest_rules,qr_INSTABURNFLAGS))
		{
		    findentrance(x,y,mfBCANDLE,true);
		    findentrance(x,y,mfRCANDLE,true);
		    findentrance(x,y,mfWANDFIRE,true);
		}
	}
		
        
mirrors:
        int checkx=0, checky=0;
        
        switch(dir)
        {
        case up:
            checkx=x+7;
            checky=y+8;
            break;
            
        case down:
            checkx=x+7;
            checky=y;
            break;
            
        case left:
            checkx=x+8;
            checky=y+7;
            break;
            
        case right:
            checkx=x;
            checky=y+7;
            break;
        }
        
        if(ignorecombo!=(((int)checky&0xF0)+((int)checkx>>4)))
        {
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMIRROR))
            {
                weapon *w=NULL;
                
                if(id==ewMagic)
                {
		    w=new weapon(*this);
                    Lwpns.add(w);
                    dead=0;
                }
                else
                {
                    w=this;
                }
                
                
                w->dir ^= 1;
                
                if(w->id != wWind)
                {
                    w->id = wRefMagic; w->convertType(true);
                    
                    if(w->dir&2)
                        w->flip ^= 1;
                    else
                        w->flip ^= 2;
                }
                
                w->ignoreLink=false;
                w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                w->y=checky&0xF0;
                w->x=checkx&0xF0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMIRRORSLASH))
            {
                weapon *w=NULL;
                
                if(id==ewMagic)
                {
		    w=new weapon(*this);
                    Lwpns.add(w);
                    dead=0;
                }
                else
                {
                    w=this;
                }
                
                w->dir = 3-w->dir;
                
                if(w->id != wWind)
                {
                    w->id = wRefMagic; w->convertType(true);
			if ( do_animation ) 
			{
				if((w->dir==1)||(w->dir==2))
				w->flip ^= 3;
			}
                }
                if ( do_animation ) 
		{
			w->tile=w->o_tile;
			
			if(w->dir&2)
			{
			    if(w->frames>1)
			    {
				w->tile+=w->frames;
			    }
			    else
			    {
				++w->tile;
			    }
			}
                }
                w->ignoreLink=false;
                w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                w->y=checky&0xF0;
                w->x=checkx&0xF0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMIRRORBACKSLASH))
            {
                weapon *w=NULL;
                
                if(id==ewMagic)
                {
		    w=new weapon(*this);
                    Lwpns.add(w);
                    dead=0;
                }
                else
                {
                    w=this;
                }
                
                w->dir ^= 2;
                
                if(w->id != wWind)
                {
                    w->id = wRefMagic; w->convertType(true);
			if ( do_animation ) 
			{
				if(w->dir&1)
					w->flip ^= 2;
				else
					w->flip ^= 1;
			}
                }
                if ( do_animation ) 
		{
			w->tile=w->o_tile;
			
			if(w->dir&2)
			{
			    if(w->frames>1)
			    {
				w->tile+=w->frames;
			    }
			    else
			    {
				++w->tile;
			    }
			}
		}
                
                w->ignoreLink=false;
                w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                w->y=checky&0xF0;
                w->x=checkx&0xF0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMAGICPRISM) && (id != wWind))
            {
                int newx, newy;
                newy=checky&0xF0;
                newx=checkx&0xF0;
                
                for(int tdir=0; tdir<4; tdir++)
                {
                    if(dir!=(tdir^1))
                    {
                        weapon *w=new weapon(*this);
			if ( this->angular && get_bit(quest_rules, qr_ANGULAR_REFLECTED_WEAPONS) )
			{
				double newangle = this->angle - DegreesToRadians(90*tdir);
				w->angle = WrapAngle(newangle);
			}
                        w->dir=tdir;
                        w->x=newx;
                        w->y=newy;
                        w->z=z;
                        w->id=wRefMagic; w->convertType(true);
                        w->parentid=parentid;
                        w->parentitem=parentitem;
                        w->flip = 0;
                        w->ignoreLink = false;
                        w->hyofs = w->hxofs = 0;
                        w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                        if ( do_animation ) 
			{
				//also set up the magic's correct animation -DD
				switch(w->dir)
				{
				case down:
				    w->flip=2;
				    
				case up:
				    w->tile = w->o_tile;
				    w->hyofs=2;
				    w->hysz=12;
				    break;
				    
				case left:
				    w->flip=1;
				    
				case right:
				    w->tile=w->o_tile+((w->frames>1)?w->frames:1);
				    w->hxofs=2;
				    w->hxsz=12;
				    break;
				
				default: break;
				}
                        }
                        Lwpns.add(w);
                    }
                }
                
                dead=0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMAGICPRISM4) && (id != wWind))
            {
                int newx, newy;
                newy=checky&0xF0;
                newx=checkx&0xF0;
                
                for(int tdir=0; tdir<4; tdir++)
                {
                    weapon *w=new weapon(*this);
                    w->dir=tdir;
		    if ( this->angular && get_bit(quest_rules, qr_ANGULAR_REFLECTED_WEAPONS) )
		    {
			double newangle = this->angle - DegreesToRadians(90*tdir);
			w->angle = WrapAngle(newangle);
		    }
                    w->x=newx;
                    w->y=newy;
                    w->z=z;
                    w->id=wRefMagic; w->convertType(true);
                    w->parentid=parentid;
                    w->parentitem=parentitem;
                    w->flip = 0;
                    w->ignoreLink = false;
                    w->hyofs = w->hxofs = 0;
                    w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                    
		    if ( do_animation ) 
		    {
			    //also set up the magic's correct animation -DD
			    switch(w->dir)
			    {
			    case down:
				w->flip=2;
				
			    case up:
				w->tile = w->o_tile;
				w->hyofs=2;
				w->hysz=12;
				break;
				
			    case left:
				w->flip=1;
				
			    case right:
				w->tile=w->o_tile+((w->frames>1)?w->frames:1);
				w->hxofs=2;
				w->hxsz=12;
				break;
			    
			    default: break;
			    }
                    }
                    Lwpns.add(w);
                }
                
                dead=0;
            }
            
            if(blocked(0, 0))
            {
                dead=0;
            }
        }
        //:Weapon Only
        //eweapon, later
	//if ( doscript )
	//{
	//	   FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	//}
    }
    break;
    
    // enemy weapons
    case ewFireball2:
        switch(misc)
        {
        case up:
            y-=.5;
            break;
            
        case down:
            y+=.5;
            break;
            
        case left:
            x-=.5;
            break;
            
        case right:
            x+=.5;
            break;
            
        case l_up:
            y-=.354;
            x-=.354;
            break;
            
        case r_up:
            y-=.354;
            x+=.354;
            break;
            
        case l_down:
            y+=.354;
            x-=.354;
            break;
            
        case r_down:
            y+=.354;
            x+=.354;
            break;
        }
        
        //fallthrough
    case wRefFireball:
    case ewFireball:
    {
        if((id==wRefFireball)&&(findentrance(x,y,mfREFFIREBALL,true))) dead=0;
        
        if((id==wRefFireball)&&(findentrance(x,y,mfSTRIKE,true))) dead=0;
        
        if(blocked())
        {
            dead=0;
        }
        
        if(clk<16)
        {
            ++clk;
            
            if(dead>0)
                --dead;
            
			if(dead == 0 && !weapon_dying_frame && get_bit(quest_rules,qr_WEAPONS_EXTRA_FRAME))
			{
				if(id==wSword) return true;
				else if ( id==wBrang ) return dead==0;
				weapon_dying_frame = true;
				return false;
			}
			
            return dead==0;
        }
        if ( id == wRefFireball && ScriptGenerated && doscript )
	{
		if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
	}
        break;
    }
    
    case ewFlame:
    {
        if(clk==32)
        {
            step=0;
            misc = -1; // Don't drift diagonally anymore
            isLit=true;
            if(get_bit(quest_rules,qr_TEMPCANDLELIGHT)){
			checkLightSources();
		} else {
			checkLightSources(true);
		}
        }
        
        if(clk==126)
        {
            dead=1;
            
            if(get_bit(quest_rules,qr_TEMPCANDLELIGHT))
            {
                isLit=false;
                checkLightSources();
            }
        }
        
        if(blocked())
        {
            dead=1;
        }
        
        // Killed by script?
        if(dead==0 && get_bit(quest_rules,qr_TEMPCANDLELIGHT) && (Lwpns.idCount(wFire) + Ewpns.idCount(ewFlame))==1)
        {
            isLit=false;
            checkLightSources();
        }
        
        break;
    }
    
    case ewFireTrail:
    {
        if(clk==32)
        {
            step=0;  //should already be 0, but still...
            isLit=true;
            if(get_bit(quest_rules,qr_TEMPCANDLELIGHT)){
			checkLightSources();
		} else {
			checkLightSources(true);
		}
        }
        
        if(clk==640)
        {
            dead=1;
            
            if(get_bit(quest_rules,qr_TEMPCANDLELIGHT))
            {
                isLit=false;
                checkLightSources();
            }
        }
        
        if(blocked())
        {
            dead=1;
        }
        
        
        break;
    }
    
    case ewBrang:
    {
        if(clk==0)
        {
            misc2=(dir<left)?y:x;                               // save home position
            
            if(dummy_bool[0]==true)
            {
                seekLink();
            }
            
            if(get_bit(quest_rules,qr_MORESOUNDS))
            {
                //if (step!=0)
                sfx(WAV_BRANG, pan(int(x)), true);
                //else
                ;//stop_sfx(WAV_BRANG);
            }
        }
        
        ++clk2;
        
        if(clk2==45&&!dummy_bool[0])
        {
            misc=1;
            dir^=1;
        }
        
        if(dummy_bool[0])
        {
            step=5;
        }
        else
        {
            if(clk2>27 && clk2<61)
            {
                step=1;
            }
            else if(misc)
            {
                step=2;
            }
            else
            {
                step=3;
            }
        }
        
        bool killrang = false;
        
        if(misc==1)                                           // returning
        {
            if(dummy_bool[0]==true)
            {
                //no parent
                if(parentid < 0)
                    return true;
                    
                //find parent
                int index = -1;
                
                for(int i=0; i<guys.Count(); i++)
                    if(guys.spr(i)->getUID()==parentid)
                    {
                        index = i;
                        break;
                    }
                    
                //parent is not a guy
                if(index < 0)
                    return true;
                    
                seekEnemy(index);
                
                if((abs(x-GuyX(index))<7)&&(abs(y-GuyY(index))<7))
                {
                    if(get_bit(quest_rules,qr_MORESOUNDS) && !Lwpns.idCount(wBrang) && Ewpns.idCount(ewBrang)<=1)
                    {
                        stop_sfx(WAV_BRANG);
                    }
                    
                    return true;
                }
            }
            else
            {
                switch(dir)
                {
                case up:
                    if(y<misc2) killrang=true;
                    
                    break;
                    
                case down:
                    if(y>misc2) killrang=true;
                    
                    break;
                    
                case left:
                    if(x<misc2) killrang=true;
                    
                    break;
                    
                case right:
                    if(x>misc2) killrang=true;
                    
                    break;
                }
            }
        }
        else if(blocked())
        {
            //only obey block flags before the bounce
            dead=1;
        }
        
        //if the boomerang is not on its way home, it bounces
        if(dead==1 && misc != 1)
        {
            dead=-1;
            misc=1;
            dir^=1;
            clk2=zc_max(46,90-clk2);
        }
        
        //otherwise it disappears
        if(killrang || dead==1)
        {
            if(get_bit(quest_rules,qr_MORESOUNDS) && !Lwpns.idCount(wBrang) && Ewpns.idCount(ewBrang)<=1)
            {
                stop_sfx(WAV_BRANG);
            }
            
            dead = 1;
        }
    }
    /*
    //script weapon types:
    case wScript1:
	case wScript2:
	case wScript3:
	case wScript4:
	case wScript5:
	case wScript6:
	case wScript7:
	case wScript8:
	case wScript9:
	case wScript10:
	{
		if ( doscript && getUID() == Link.getUID() || isLWeapon )
		{
			   FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, getUID());
		}
		break;
	}
		
	case wIce: break;
	*/
    
    }
    
    // move sprite, check clipping
    if(dead==-1 && clk>=0)
    {
        move(step);
        
        if(clip())
        {
            onhit(true);
        }
        else if(id==ewRock)
        {
            if(_walkflag(x,y,2) || _walkflag(x,y+8,2))
            {
                onhit(true);
            }
        }
    }
    else if(dead==-10) // Scripting hack thing related to weapon->DeadState
    {
        if(clip())
        {
            onhit(true);
        }
    }
    
    if(bounce)
    {
        switch(dir)
        {
        case up:
            x-=1;
            y+=2;
            break;
            
        case down:
            x+=1;
            y-=2;
            break;
            
        case left:
            x+=2;
            y-=1;
            break;
            
        case right:
            x-=2;
            y-=1;
            break;
        }
    }
    
    // update clocks
    ++clk;
    //if ( id >= wScript1 && id <= wScript10 ) Z_scripterrlog("Updated Script Weapon Clock\n");
    
    if(dead>0)
    {
	    
        --dead;
    }
    
    //if ( linked_parent != wBrang || linked_parent != wArrow && id != wPhantom )
    //{
	if(dead == 0 && !weapon_dying_frame && get_bit(quest_rules,qr_WEAPONS_EXTRA_FRAME))
	{
		if(id==wSword) return true;
		weapon_dying_frame = true;
		return false;
	}
    //}
    return dead==0;
}


bool weapon::animateandrunscript(int ii)
{
    // do special timing stuff
    bool hooked=false;
//	Z_scripterrlog("Weapon script is: %d\n",weaponscript);
	
    
    
    // fall down
    switch(id)
    {
    case wFire:
    
        // Din's Fire shouldn't fall
        if(parentitem>=0 && itemsbuf[parentitem].family==itype_dinsfire && !(itemsbuf[parentitem].flags & ITEM_FLAG3))
        {
            break;
        }
        
    case wLitBomb:
    case wLitSBomb:
    case wBait:
    case ewFlame:
    case ewFireTrail:
        if(isSideViewGravity())
        {
            if(!_walkflag(x,y+16,0))
            {
                y+=fall/100;
                
                if(fall <= (int)zinit.terminalv)
                {
                    fall += zinit.gravity;
                }
            }
            else
            {
                if(fall!=0 && !(step>0 && dir==up))  // Don't fix pos if still moving through solidness
                    y-=(int)y%8; // Fix position
                    
                fall = 0;
            }
            
            if(y>192) dead=0;  // Out of bounds
        }
        else
        {
            z-=fall/100;
            
            if(z<=0)
            {
                z = fall = 0;
            }
            else if(fall <= (int)zinit.terminalv)
            {
                fall += zinit.gravity;
            }
        }
    }
    
    if(id>wEnemyWeapons && id!=ewBrang && id != ewFireball2 && id != ewBomb && id != ewSBomb)
        switch(misc)
        {
        case up:
            y-=.5;
            break;
            
        case down:
            y+=.5;
            break;
            
        case left:
            x-=.5;
            break;
            
        case right:
            x+=.5;
            break;
            
        case 4:
            y-=1;
            break;
            
        case 5:
            y+=1;
            break;
            
        case 6:
            x-=1;
            break;
            
        case 7:
            x+=1;
            break;
            //case l_up:  y-=.354; x-=.354; break;
            //case r_up:  y-=.354; x+=.354; break;
            //case l_down:y+=.354; x-=.354; break;
            //case r_down:y+=.354; x+=.354; break;
        }
        
    switch(id)
    {
        // Link's weapons
    case wSword:
    case wWand:
    case wHammer:
        if(LinkAction()!=attacking && LinkAction()!=ischarging && !LinkCharged())
        {
            dead=0;
        }
        
        break;
        
    case wCByrna:
    {
        if(blocked())
        {
            dead=0;
        }
        
        int speed = parentitem>-1 ? zc_max(itemsbuf[parentitem].misc1,1) : 1;
        int radius = parentitem>-1 ? zc_max(itemsbuf[parentitem].misc2,8) : 8;
        double xdiff = -(sin((double)clk/speed) * radius);
        double ydiff = (cos((double)clk/speed) * radius);
        
        double ddir=atan2(double(ydiff),double(xdiff));
        
        if((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
            dir=r_up;
        else if((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
            dir=right;
        else if((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
            dir=r_down;
        else if((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
            dir=down;
        else if((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
            dir=l_down;
        else if((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
            dir=left;
        else if((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
            dir=l_up;
        else
            dir=up;
            
        x = (zfix)((double)LinkX() + xdiff);
        y = (zfix)((double)LinkY() + ydiff);
        z = LinkZ();
        
        if(parentitem>-1)
            sfx(itemsbuf[parentitem].usesound,pan(int(x)),true,false);
    }
    break;
    
    case wBeam:
    case wRefBeam:
    {
        for(int i2=0; i2<=zc_min(type-1,3) && dead!=23; i2++)
        {
            if(findentrance(x,y,mfSWORDBEAM+i2,true)) dead=23;
        }
        
        if(blocked())
        {
            dead=23;
        }
        if ( doscript && weaponscript > 0 ) 
        {
                /*if ( Dead() )
                {
                        doscript = 0;
                        weaponscript = 0;
                }
                else*/
                {
                        int w_index = -1; //Give the script the correct index! -Z
                        for(word i = 0; i < Lwpns.Count(); i++)
                        {
                                if(Lwpns.spr(i)->getUID() == getUID())
                                w_index = i;
                                //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                        }
                                
                        //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                        if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, ii);		
                }
        }
        
    }
    case ewSword:
        if(blocked())
        {
            dead=0;
        }
        
        if(id==ewSword && get_bit(quest_rules,qr_SWORDMIRROR) || id!=ewSword && (parentitem > -1 ? itemsbuf[parentitem].flags & ITEM_FLAG9 : get_bit(quest_rules,qr_SWORDMIRROR))) //TODO: First qr_SWORDMIRROR port to enemy weapon flag, second qr_SWORDMIRROR port to script default flag -V
        {
            int checkx=0, checky=0;
            
            switch(dir)
            {
            case up:
                checkx=x+7;
                checky=y+8;
                break;
                
            case down:
                checkx=x+7;
                checky=y;
                break;
                
            case left:
                checkx=x+8;
                checky=y+7;
                break;
                
            case right:
                checkx=x;
                checky=y+7;
                break;
            }
            
            if(ignorecombo==(((int)checky&0xF0)+((int)checkx>>4)))
                break;
                
		int posx, posy;
		if(get_bit(quest_rules,qr_OLDMIRRORCOMBOS))//Replace this conditional with an ER; true if the ER is checked. This will use the old (glitchy) behavior for sword beams.
		{
			posx=x;
			posy=y;
		}
		else
		{
			posx=checkx;
			posy=checky;
		}
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMIRROR))
            {
                id = wRefBeam;
                dir ^= 1;
                
                if(dir&2)
                    flip ^= 1;
                else
                    flip ^= 2;
                    
                ignoreLink=false;
                ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                y=(int)posy&0xF0;
                x=(int)posx&0xF0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMIRRORSLASH))
            {
                id = wRefBeam;
		if ( do_animation ) 
		{
			dir = 3-dir;
			{
			    if(dir==right)
				flip &= ~1; // not horiz
			    else if(dir==left)
				flip |= 1;  // horiz
			    else if(dir==up)
				flip &= ~2; // not vert
			    else if(dir==down)
				flip |= 2;  // vert
			}
			tile=o_tile;
			
			if(dir&2)
			{
			    if(frames>1)
			    {
				tile+=frames;
			    }
			    else
			    {
				++tile;
			    }
			}
                }
                ignoreLink=false;
                ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                y=(int)posy&0xF0;
                x=(int)posx&0xF0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMIRRORBACKSLASH))
            {
                id = wRefBeam;
                dir ^= 2;
                {
			if ( do_animation ) 
			{
			    if(dir==right)
				flip &= ~1; // not horiz
			    else if(dir==left)
				flip |= 1;  // horiz
			    else if(dir==up)
				flip &= ~2; // not vert
			    else if(dir==down)
				flip |= 2;  // vert
			}
                }
		if ( do_animation ) 
		{
			tile=o_tile;
			
			if(dir&2)
			{
			    if(frames>1)
			    {
				tile+=frames;
			    }
			    else
			    {
				++tile;
			    }
			}
                }
                ignoreLink=false;
                ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                y=(int)posy&0xF0;
                x=(int)posx&0xF0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMAGICPRISM))
            {
                int newx, newy;
                newy=(int)posy&0xF0;
                newx=(int)posx&0xF0;
                
                for(int tdir=0; tdir<4; tdir++)
                {
                    if(dir!=(tdir^1))
                    {
                        weapon *w=new weapon(*this);
                        w->dir=tdir;
			if ( this->angular && get_bit(quest_rules, qr_ANGULAR_REFLECTED_WEAPONS) )
			{
				double newangle = this->angle - DegreesToRadians(90*tdir);
				w->angle = WrapAngle(newangle);
			}
                        w->x=newx;
                        w->y=newy;
                        w->z=z;
                        w->id=wRefBeam;
                        w->parentid=parentid;
                        w->parentitem=parentitem;
                        w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                        w->hyofs = w->hxofs = 0;
                        //also set up the magic's correct animation -DD
                        w->flip=0;
                        if ( do_animation ) 
			{
				switch(w->dir)
				{
				case down:
				    w->flip=2;
				    
				case up:
				    w->tile = w->o_tile;
				    w->hyofs=2;
				    w->hysz=12;
				    break;
				    
				case left:
				    w->flip=1;
				    
				case right:
				    w->tile=w->o_tile+((w->frames>1)?w->frames:1);
				    w->hxofs=2;
				    w->hxsz=12;
				    break;
				
				default: break;
				}
                        }
                        Lwpns.add(w);
                    }
                }
                
                dead=0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMAGICPRISM4))
            {
                int newx, newy;
                newy=(int)posy&0xF0;
                newx=(int)posx&0xF0;
                
                for(int tdir=0; tdir<4; tdir++)
                {
                    weapon *w=new weapon(*this);
                    w->dir=tdir;
		    if ( this->angular && get_bit(quest_rules, qr_ANGULAR_REFLECTED_WEAPONS) )
		    {
			double newangle = this->angle - DegreesToRadians(90*tdir);
			w->angle = WrapAngle(newangle);
		    }
                    w->x=newx;
                    w->y=newy;
                    w->z=z;
                    w->id=wRefBeam;
                    w->parentid=parentid;
                    w->parentitem=parentitem;
                    w->hyofs = w->hxofs = 0;
                    w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                    //also set up the magic's correct animation -DD
                    w->flip=0;
                    if ( do_animation ) 
		    {
			    switch(w->dir)
			    {
			    case down:
				w->flip=2;
				
			    case up:
				w->tile = w->o_tile;
				w->hyofs=2;
				w->hysz=12;
				break;
				
			    case left:
				w->flip=1;
				
			    case right:
				w->tile=w->o_tile+((w->frames>1)?w->frames:1);
				w->hxofs=2;
				w->hxsz=12;
				break;
			    
			    default: break;
			    }
                    }
                    Lwpns.add(w);
                }
                
                dead=0;
            }
        }
        
        break;
        
    case wWhistle:
    {
        if(clk)
        {
            dead=1;
        }
        if ( doscript && weaponscript > 0 ) 
        {
                /*if ( Dead() )
                {
                        doscript = 0;
                        weaponscript = 0;
                }
                else*/
                {
                        int w_index = -1; //Give the script the correct index! -Z
                        for(word i = 0; i < Lwpns.Count(); i++)
                        {
                                if(Lwpns.spr(i)->getUID() == getUID())
                                w_index = i;
                                //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                        }
                                
                        //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                        if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, ii);		
                }
        }
        break;
    }
        
    case wWind:
    {
        if(blocked())
        {
            dead=1;
        }
        
        int wrx;
        
        if(get_bit(quest_rules,qr_NOARRIVALPOINT))
            wrx=tmpscr->warpreturnx[0];
        else wrx=tmpscr->warparrivalx;
        
        int wry;
        
        if(get_bit(quest_rules,qr_NOARRIVALPOINT))
            wry=tmpscr->warpreturny[0];
        else wry=tmpscr->warparrivaly;
        
        if(specialinfo==1 && dead==-1 && x==(int)wrx && y==(int)wry)
        {
	    stop_sfx(WAV_ZN1WHIRLWIND);
            dead=2;
        }
        else if(LinkAction() !=inwind && ((dir==right && x>=240) || (dir==down && y>=160) || (dir==left && x<=0) || (dir==up && y<=0)))
        {
            stop_sfx(WAV_ZN1WHIRLWIND);
            dead=1;
        }
        else if(get_bit(quest_rules,qr_MORESOUNDS) && dead < 1)
            sfx(WAV_ZN1WHIRLWIND,pan(int(x)),true,false);
            
        if((parentitem==-1 && get_bit(quest_rules,qr_WHIRLWINDMIRROR)) || (parentitem > -1 && itemsbuf[parentitem].flags & ITEM_FLAG3))
            goto mirrors;
            
        break;
    }
    
    case wFire:
    {
        if(blocked())
        {
            dead=1;
        }
        
        if(parentitem<0 || (parentitem>-1 && itemsbuf[parentitem].family!=itype_book))
        {
            if(clk==32)
            {
                step=0;
                
                if(parentitem<0 || !(itemsbuf[parentitem].flags & ITEM_FLAG2))
                {
                    isLit = true;
                    if((parentitem==-1&&get_bit(quest_rules,qr_TEMPCANDLELIGHT))||itemsbuf[parentitem].flags & ITEM_FLAG5){
				checkLightSources();
			} else {
				checkLightSources(true);
			}
                }
            }
            
            if(clk==94)
            {
                dead=1;
                
                if(((parentitem==-1 && get_bit(quest_rules,qr_TEMPCANDLELIGHT)) ||
		   (parentitem>-1&&!(itemsbuf[parentitem].flags & ITEM_FLAG2)&&(itemsbuf[parentitem].flags & ITEM_FLAG5))) &&
                   (Lwpns.idCount(wFire) + Ewpns.idCount(ewFlame))==1)
                {
                    isLit = false;
                    checkLightSources();
                }
            }
            
            if(clk==94 || get_bit(quest_rules,qr_INSTABURNFLAGS))
            {
                findentrance(x,y,mfBCANDLE,true);
                
                if(type>1) //red candle
                {
                    findentrance(x,y,mfRCANDLE,true);
                }
                
                if(linked_parent == itype_dinsfire)
                {
                    findentrance(x,y,mfDINSFIRE,true);
                }
            }
        }                                                     //wand fire
        else
        {
            if(clk==1)
            {
                isLit = true;
                if(itemsbuf[parentitem].flags & ITEM_FLAG5){
			checkLightSources();
		} else {
			checkLightSources(true);
		}
            }
            
            if(clk==80)
            {
                dead=1;
                findentrance(x,y,mfBCANDLE,true);
                findentrance(x,y,mfRCANDLE,true);
                findentrance(x,y,mfWANDFIRE,true);
                
                if(((parentitem==-1&&get_bit(quest_rules,qr_TEMPCANDLELIGHT))||(parentitem>-1&&(itemsbuf[parentitem].flags & ITEM_FLAG5))) && (Lwpns.idCount(wFire) + Ewpns.idCount(ewFlame))==1)
                {
                    isLit=false;
                    checkLightSources();
                }
            }
        }
        
        // Killed by script?
        if(dead==0 && ((parentitem==-1&&get_bit(quest_rules,qr_TEMPCANDLELIGHT))||(parentitem>0&&(itemsbuf[parentitem].flags & ITEM_FLAG5))) && (Lwpns.idCount(wFire) + Ewpns.idCount(ewFlame))==1)
        {
            isLit=false;
            checkLightSources();
        }
        if ( doscript && weaponscript > 0 ) 
        {
                /*if ( Dead() )
                {
                        doscript = 0;
                        weaponscript = 0;
                }
                else*/
                {
                        int w_index = -1; //Give the script the correct index! -Z
                        for(word i = 0; i < Lwpns.Count(); i++)
                        {
                                if(Lwpns.spr(i)->getUID() == getUID())
                                w_index = i;
                                //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                        }
                                
                        //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                        if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, ii);		
                }
        }
        break;
    }
    
    case wLitBomb:
    case wBomb:
    case ewLitBomb:
    case ewBomb:
    case ewLitSBomb:
    case ewSBomb:
    case wLitSBomb:
    case wSBomb:
    {
        if(!misc)
        {
            break;
        }
        
        // Naaah.
        /*if (blocked())
        {
          dead=1;
        }*/
        if(clk==(misc-2) && step==0)
        {
            id = (id>wEnemyWeapons ? (id==ewLitSBomb||id==ewSBomb ? ewSBomb : ewBomb)
                      : parentitem>-1 ? ((itemsbuf[parentitem].family==itype_sbomb) ? wSBomb:wBomb)
                      : (id==wLitSBomb||id==wSBomb ? wSBomb : wBomb));
            hxofs=2000;
        }
        
        if(clk==(misc-1) && step==0)
    {
            sfx((id>=wEnemyWeapons || parentitem<0) ? WAV_BOMB :
                itemsbuf[parentitem].usesound,pan(int(x)));
                
            if(id==wSBomb || id==wLitSBomb || id==ewSBomb || id==ewLitSBomb)
            {
                hxofs=hyofs=-16;
                hxsz=hysz=48;
            }
            else
            {
                hxofs=hyofs=-8;
                hxsz=hysz=32;
            }
            
            hzsz=16;
        }
        
        int boomend = (misc+(((id == wBomb || id == wSBomb || id == wLitBomb || id == wLitSBomb) &&
                              (parentitem>-1 && itemsbuf[parentitem].flags & ITEM_FLAG1)) ? 35 : 31));
                              
        if(clk==boomend && step==0)
        {
            hxofs=2000;
        }
        
        if(id<wEnemyWeapons)
        {
            if(clk==(misc-1))
            {
                int f1 = (id==wSBomb || id==wLitSBomb) ? 16 : 0; // Large SBomb triggerbox
                
                for(int tx=-f1; tx<=f1; tx+=8)  // -16,-8,0,8,16
                {
                    int f2 = 0;
                    
                    if(tx==-8 || tx==8)
                        f2 = f1;
                        
                    for(int ty=-f2; ty<=f2; ty+=32)
                    {
                        findentrance(x+tx,y+ty+(isSideViewGravity()?2:-3),mfBOMB,true);
                        
                        if(id==wSBomb || id==wLitSBomb)
                        {
                            findentrance(x+tx,y+ty+(isSideViewGravity()?2:-3),mfSBOMB,true);
                        }
                        
                        findentrance(x+tx,y+ty+(isSideViewGravity()?2:-3),mfSTRIKE,true);
                    }
                }
            }
            
            if(!get_bit(quest_rules,qr_NOBOMBPALFLASH))
            {
                if(!usebombpal)
                {
                    if(clk==misc || clk==misc+5)
                    {
                    
                        usebombpal=true;
                        memcpy(tempbombpal, RAMpal, PAL_SIZE*sizeof(RGB));
                        
                        //grayscale entire screen
                        if(get_bit(quest_rules,qr_FADE))
                        {
                            for(int i=CSET(0); i < CSET(15); i++)
                            {
                                int g = zc_min((RAMpal[i].r*42 + RAMpal[i].g*75 + RAMpal[i].b*14) >> 7, 63);
                                g = (g >> 1) + 32;
                                RAMpal[i] = _RGB(g,g,g);
                            }
                            
                        }
                        else
                        {
                            // this is awkward. NES Z1 converts colors based on the global
                            // NES palette. Something like RAMpal[i] = NESpal( reverse_NESpal(RAMpal[i]) & 0x30 );
                            for(int i=CSET(0); i < CSET(15); i++)
                            {
                                RAMpal[i] = NESpal(reverse_NESpal(RAMpal[i]) & 0x30);
                            }
                        }
                        
                        refreshpal = true;
                    }
                }
                
                if((clk==misc+4 || clk==misc+9) && usebombpal)
                {
                    // undo grayscale
                    usebombpal=false;
                    memcpy(RAMpal, tempbombpal, PAL_SIZE*sizeof(RGB));
                    refreshpal = true;
                }
            }
            
            if(clk==misc+30)
            {
                bombdoor(x,y);
            }
        }
        
        if(clk==misc+34)
        {
            if(step==0)
            {
                dead=1;
            }
        }
        
        break;
    }
    
    case wArrow:
    {
        if(dead>0)
        {
            break;
        }
        
        if(misc>0 && clk > misc)
        {
            dead=4;
        }
        
        if(findentrance(x,y,mfSTRIKE,true))
        {
            dead=4;
        }
        
        if(findentrance(x,y,mfARROW,true))
        {
            dead=4;
        }
        
        if(current_item(itype_arrow)>1)
        {
            if(findentrance(x,y,mfSARROW,true))
            {
                dead=4;
            }
        }
        
        if(current_item(itype_arrow)>=3)
        {
            if(findentrance(x,y,mfGARROW,true))
            {
                dead=4;
            }
        }
        
        if(blocked())
        {
            dead=4;
        }
        
        if ( doscript && weaponscript > 0 ) 
        {
                /*if ( Dead() )
                {
                        doscript = 0;
                        weaponscript = 0;
                }
                else*/
                {
                        int w_index = -1; //Give the script the correct index! -Z
                        for(word i = 0; i < Lwpns.Count(); i++)
                        {
                                if(Lwpns.spr(i)->getUID() == getUID())
                                w_index = i;
                                //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                        }
                                
                        //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                        if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, ii);		
                }
        }
        
        break;
    }
    
    case wSSparkle:
    {
        if(blocked())  //no spakle area?
        {
            dead=1;
        }
        
        if(clk>=frames*o_speed-1) //(((wpnsbuf[wSSPARKLE].frames) * (wpnsbuf[wSSPARKLE].speed))-1))
        {
            dead=1;
        }
        
        break;
    }
        
    case wFSparkle:
    {
        if(blocked())  //no sparkle area?
        {
            dead=1;
        }
        
        if(clk>=frames*o_speed-1) //(((wpnsbuf[wFSPARKLE].frames) * (wpnsbuf[wFSPARKLE].speed))-1))
        {
            dead=1;
        }
        
        break;
    }
    case wBait:
    {
        if(blocked())  //no bait area?
        {
            dead=23;
        }
        
        if(parentitem>-1 && clk>=itemsbuf[parentitem].misc1)
        {
            int w_index = -1; //Give the script the correct index! -Z
                        for(word i = 0; i < Lwpns.Count(); i++)
                        {
                                if(Lwpns.spr(i)->getUID() == getUID())
                                w_index = i;
                                //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                        }
                                
                        //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                        if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS]) ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, ii);
            dead=1;
        }
        if ( doscript && weaponscript > 0 ) 
        {
                /*if ( dead != -1 )
                {
                        doscript = 0;
                        weaponscript = 0;
                }
                else*/
                {
                        int w_index = -1; //Give the script the correct index! -Z
                        for(word i = 0; i < Lwpns.Count(); i++)
                        {
                                if(Lwpns.spr(i)->getUID() == getUID())
                                w_index = i;
                                //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                        }
                                
                        //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                        if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, ii);		
                }
        }
        break;
    }
    case wBrang:
    {
        if(dead==0)  // Set by ZScript
        {
            stop_sfx(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].usesound);
            break;
        }
        
        else if(dead==1) // Set by ZScript
        {
            onhit(false);
        }
        
        int deadval=(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].flags & ITEM_FLAG3)?-2:1;
        
	/* This causes brang weapons with a level > 3 to hit OTHER flags. 
	// e.g., a brang with a level of 5 would trigger mfBRANG through mfGARROW! -Z
        for(int i=0; i<current_item(itype_brang); i++)
        {
            if(findentrance(x,y,mfBRANG+i,true)) dead=deadval;
        }
	*/
	
	int branglevel = itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].fam_type;
	
	switch ( branglevel )
	{
		case 0:
		case 1:
		{
			if(findentrance(x,y,mfBRANG,true)) dead=deadval; break;
		}
		case 2: 
		{
			if(findentrance(x,y,mfBRANG,true)) dead=deadval;
			if(findentrance(x,y,mfMBRANG,true)) dead=deadval;
			break;
		}
		case 3:
		{
			goto brang_level_3_or_higher;
		}
		default: //level higher than 3
		{
			goto brang_level_3_or_higher;
		}
		brang_level_3_or_higher: 
		{
			if(findentrance(x,y,mfBRANG,true)) dead=deadval;
			if(findentrance(x,y,mfMBRANG,true)) dead=deadval;
			if(findentrance(x,y,mfFBRANG,true)) dead=deadval;
			break;
		}
	}
		
        
        if(findentrance(x,y,mfSTRIKE,true)) dead=deadval;
        
        if(blocked())
        {
            dead=deadval;
            onhit(false);
        }
        
        ++clk2;
        int range = itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].misc1;
        
        if(range && clk2>=range)
        {
            if(deadval==1)
            {
                misc=1;
            }
            else
            {
                dead=deadval;
            }
        }
        
        if(range && clk2>range-18 && clk2<range+16)
        {
            step=1;
        }
        else if(misc)
        {
            step=2;
        }
        else
        {
            step=3;
        }
        
        if(clk==0)                                            // delay a frame
        {
            ++clk;
            sfx(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].usesound,pan(int(x)),true);
            return false;
        }
        
        if(clk==1)                                            // then check directional input
        {
            if(Up())
            {
                dir=up;
                
                if(Left())  dir=l_up;
                
                if(Right()) dir=r_up;
            }
            else if(Down())
            {
                dir=down;
                
                if(Left())  dir=l_down;
                
                if(Right()) dir=r_down;
            }
            else if(Left())
            {
                dir=left;
            }
            else if(Right())
            {
                dir=right;
            }
        }
        
        if(dead==1)
        {
            dead=-1;
            misc=1;
        }
        
        if(dead==-2)
        {
            misc=1;
        }
        
        if(misc==1)                                           // returning
        {
            if((abs(LinkY()-y)<7 && abs(LinkX()-x)<7)||dead==-2)
            {
                if(dead!=-2)
                {
                    CatchBrang();
                }
                
                if(Lwpns.idCount(wBrang)<=1 && (!get_bit(quest_rules, qr_MORESOUNDS) || !Ewpns.idCount(ewBrang)))
                    stop_sfx(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].usesound);
                    
                /*if (dummy_bool[0])
                {
                    add_grenade(x,y,z,0,-1); //TODO: Super bombs as well?
                    dummy_bool[0]=false;
                }*/
                if(dragging!=-1)
                {
                    getdraggeditem(dragging);
                }
                
                return true;
            }
            
            seekLink();
        }
        //call before the sfx
        if ( doscript && weaponscript > 0 ) 
        {
                /*if ( Dead() )
                {
                        doscript = 0;
                        weaponscript = 0;
                }
                else*/
                {
                        int w_index = -1; //Give the script the correct index! -Z
                        for(word i = 0; i < Lwpns.Count(); i++)
                        {
                                if(Lwpns.spr(i)->getUID() == getUID())
                                w_index = i;
                                //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                        }
                                
                        //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                        if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS]) ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, ii);		
                }
        }
        
        sfx(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].usesound,pan(int(x)),true,false);
        
        break;
    }
    
    case wHookshot:
    {
        if(dead==0)  // Set by ZScript
        {
            hookshot_used = false;
            
            for(int i=0; i<chainlinks.Count(); i++)
            {
                chainlinks.del(chainlinks.idFirst(wHSChain));
            }
        }
	
        // Hookshot grab and retract code
        if(misc==0)
        {
            int maxlength=parentitem>-1 ? 16*itemsbuf[parentitem].misc1 : 0;
            
            if((abs(LinkX()-x)>maxlength)||(abs(LinkY()-y)>maxlength))
            {
                dead=1;
            }
            
            if(findentrance(x,y,mfSTRIKE,true)) dead=1;
            
            if(findentrance(x,y,mfHOOKSHOT,true)) dead=1;
            
            if(dir==up)
            {
                if(isHSGrabbable(combobuf[MAPCOMBO(x+2,y+7)]))
                {
                    hooked=true;
                }
                
                if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))
                    hooked = hooked || isHSGrabbable(combobuf[MAPCOMBO2(0,x+2,y+7)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+2,y+7)]);
                    
                if(!hooked && _walkflag(x+2,y+7,1) && !ishookshottable((int)x+2,(int)y+7))
                {
                    dead=1;
                }
            }
            
            if(dir==down)
            {
                if(isHSGrabbable(combobuf[MAPCOMBO(x+12,y+12)]))
                {
                    hooked=true;
                }
                
                if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))
                    hooked = hooked || isHSGrabbable(combobuf[MAPCOMBO2(0,x+12,y+12)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+12,y+12)]);
                    
                if(!hooked && _walkflag(x+12,y+12,1) && !ishookshottable((int)x+12,(int)y+12))
                {
                    dead=1;
                }
            }
            
            if(dir==left)
            {
                if(get_bit(quest_rules, qr_OLDHOOKSHOTGRAB))
                {
                    if(isHSGrabbable(combobuf[MAPCOMBO(x+6,y+7)]))
                    {
                        hooked=true;
                    }
                }
                else if(isHSGrabbable(combobuf[MAPCOMBO(x+6,y+13)]))
                {
                    hooked=true;
                }
                
                if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))
                    hooked = hooked || isHSGrabbable(combobuf[MAPCOMBO2(0,x+6,y+13)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+6,y+13)]);
                    
                if(!hooked && _walkflag(x+6,y+13,1) && !ishookshottable((int)x+6,(int)y+13))
                {
                    dead=1;
                }
            }
            
            if(dir==right)
            {
                if(get_bit(quest_rules, qr_OLDHOOKSHOTGRAB))
                {
                    if(isHSGrabbable(combobuf[MAPCOMBO(x+9,y+7)]))
                    {
                        hooked=true;
                    }
                }
                else if(isHSGrabbable(combobuf[MAPCOMBO(x+9,y+13)]))
                {
                    hooked=true;
                }
                
                if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))
                    hooked = hooked || isHSGrabbable(combobuf[MAPCOMBO2(0,x+9,y+13)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+9,y+13)]);
                    
                if(!hooked && _walkflag(x+9,y+13,1) && !ishookshottable((int)x+9,(int)y+13))
                {
                    dead=1;
                }
            }
	    //Diagonal Hookshot (9) (This function never runs at this time.)
	    //Diagonal Hookshot Grab Points (This function never runs at this time.)
	    if ( dir == r_down ) 
		{
			if(get_bit(quest_rules, qr_OLDHOOKSHOTGRAB))
			{	//right						//down
				if( isHSGrabbable(combobuf[MAPCOMBO(x+9,y+7)]) || isHSGrabbable(combobuf[MAPCOMBO(x+12,y+12)]) )
				{
					hooked=true;
				}
			}

					//right						//down
			else if( ( isHSGrabbable(combobuf[MAPCOMBO(x+9,y+13)])) || isHSGrabbable(combobuf[MAPCOMBO(x+12,y+12)]) )
			{
				hooked=true;
			}

			if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))			//right
			{
				hooked = hooked || isHSGrabbable(combobuf[MAPCOMBO2(0,x+9,y+13)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+9,y+13)]) || 
				//down
				isHSGrabbable(combobuf[MAPCOMBO2(0,x+12,y+12)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+12,y+12)]);
			}
                    
			//right
			if(!hooked &&  ( ( ( _walkflag(x+9,y+13,1) && !ishookshottable((int)x+9,(int)y+13)) ) ||
				//down
				(_walkflag(x+12,y+12,1) && !ishookshottable((int)x+12,(int)y+12)) ) )
			{
			    dead=1;
			}
		    
		    
		}
		if ( dir == l_down ) 
		{
			if(get_bit(quest_rules, qr_OLDHOOKSHOTGRAB))
			{	//left						//down
				if( isHSGrabbable(combobuf[MAPCOMBO(x+6,y+7)]) || isHSGrabbable(combobuf[MAPCOMBO(x+12,y+12)]) )
				{
					hooked=true;
				}
			}
			

					//left						//down
			else if( ( isHSGrabbable(combobuf[MAPCOMBO(x+6,y+13)])) || isHSGrabbable(combobuf[MAPCOMBO(x+12,y+12)]) )
			{
				hooked=true;
			}

			if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))
			{
							//left
				hooked = hooked || isHSGrabbable(combobuf[MAPCOMBO2(0,x+6,y+13)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+6,y+13)]) || 
					//down
				isHSGrabbable(combobuf[MAPCOMBO2(0,x+12,y+12)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+12,y+12)]);
			}
			//left
			if(!hooked && ( ( ( _walkflag(x+6,y+13,1) && !ishookshottable((int)x+6,(int)y+13)) ) ||
				//down
				(_walkflag(x+12,y+12,1) && !ishookshottable((int)x+12,(int)y+12)) ) )
			{
				dead=1;
			}
		    
		    
		}
		if ( dir == r_up ) 
		{
		        if(get_bit(quest_rules, qr_OLDHOOKSHOTGRAB))
			{	//right						//up
			    if( isHSGrabbable(combobuf[MAPCOMBO(x+9,y+7)]) || isHSGrabbable(combobuf[MAPCOMBO(x+2,y+7)]) )
				{
					hooked=true;
				}
			}

					//right						//up
			else if( ( isHSGrabbable(combobuf[MAPCOMBO(x+9,y+13)])) || isHSGrabbable(combobuf[MAPCOMBO(x+2,y+7)]) )
			{
				hooked=true;
			}

			if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))			//right
			{
				hooked = hooked || isHSGrabbable(combobuf[MAPCOMBO2(0,x+9,y+13)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+9,y+13)]) || 
				//up
				isHSGrabbable(combobuf[MAPCOMBO2(0,x+2,y+7)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+2,y+7)]);
			}
			//right
			if(!hooked &&  ( ( ( _walkflag(x+9,y+13,1) && !ishookshottable((int)x+9,(int)y+13)) ) ||
				//up
				(_walkflag(x+2,y+7,1) && !ishookshottable((int)x+2,(int)y+7)) ) )
			{
				dead=1;
			}
		}
		if ( dir == l_up ) 
		{
		        if(get_bit(quest_rules, qr_OLDHOOKSHOTGRAB))
			{	//left						//up
				if( isHSGrabbable(combobuf[MAPCOMBO(x+6,y+7)]) || isHSGrabbable(combobuf[MAPCOMBO(x+2,y+7)]) )
				{
					hooked=true;
				}
			}

					//left						//up
			else if( ( isHSGrabbable(combobuf[MAPCOMBO(x+6,y+13)])) || isHSGrabbable(combobuf[MAPCOMBO(x+2,y+7)]) )
			{
				hooked=true;
			}

			if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))			//left
			{
				hooked = hooked || isHSGrabbable(combobuf[MAPCOMBO2(0,x+6,y+13)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+6,y+13)]) || 
				//up
				isHSGrabbable(combobuf[MAPCOMBO2(0,x+2,y+7)]) || isHSGrabbable(combobuf[MAPCOMBO2(1,x+2,y+7)]);
			}
							//left
			if(!hooked && ( ( ( _walkflag(x+6,y+13,1) && !ishookshottable((int)x+6,(int)y+13)) ) ||
				//up
				(_walkflag(x+2,y+7,1) && !ishookshottable((int)x+2,(int)y+7)) ) )
			{
				dead=1;
			}
		}
        }
        
        if(hooked==true)
        {
            misc=1;
            pull_link=true;
            step=0;
        }
        
        ++clk2;
        
        if(clk==0)                                            // delay a frame
        {
            ++clk;
            
            if(parentitem>-1)
            {
                sfx(itemsbuf[parentitem].usesound,pan(int(x)),true);
            }
            
            return false;
        }
        
        if(dead==1)
        {
            dead=-1;
            
            if(step > 0)
                step = -step;
                
            misc=1;
        }
        
        if(misc==1)                                           // returning
        {
            if((dir<left && abs(LinkY()-y)<9) || (dir >= left && abs(LinkX()-x)<9))
            {
                hookshot_used=false;
                
                if(pull_link)
                {
                    hs_fix=true;
                }
                
                pull_link=false;
                chainlinks.clear();
                CatchBrang();
                
                if(parentitem>-1)
                {
                    stop_sfx(itemsbuf[parentitem].usesound);
                }
                
                if(dragging!=-1)
                {
                    getdraggeditem(dragging);
                }
                
                return true;
            }
        }
        
        if(parentitem>-1)
        {
            sfx(itemsbuf[parentitem].usesound,pan(int(x)),true,false);
        }
        
        if(blocked())
        {
            //not on the return!! -DD
            if(dead != -1)
                dead=1;
        }
    }
    break;
    
    case wHSHandle:
    {
        if(hookshot_used==false)
        {
            dead=0;
        }
        
        if(blocked())  //no hookshot handle area?
        {
            hookshot_used=false;
            dead=0;
        }
        
        break;
    }
    case wPhantom:
    {
        switch(type)
        {
        case pDINSFIREROCKET:
            if(y <= -200)
                dead = 1;
                
            break;
            
        case pDINSFIREROCKETRETURN:                                             //Din's Fire Rocket return
            if(y>=casty)
            {
                dead=1;
                castnext=true;
            }
            
            break;
            
        case pDINSFIREROCKETTRAIL:                                             //Din's Fire Rocket trail
            if(clk>=(((wpnsbuf[wDINSFIRES1A].frames) * (wpnsbuf[wDINSFIRES1A].speed))-1))
            {
                dead=0;
            }
            
            break;
            
        case pDINSFIREROCKETTRAILRETURN:                                             //Din's Fire Rocket return trail
            if(clk>=(((wpnsbuf[wDINSFIRES1B].frames) * (wpnsbuf[wDINSFIRES1B].speed))-1))
            {
                dead=0;
            }
            
            break;
            
        case pNAYRUSLOVEROCKETRETURN1:                                             //Nayru's Love Rocket return
            if(x>=castx)
            {
                dead=1;
                castnext=true;
            }
            
            break;
            
        case pNAYRUSLOVEROCKETTRAIL1:                                             //Nayru's Love Rocket trail
            if(clk>=(((wpnsbuf[wNAYRUSLOVES1A].frames) * (wpnsbuf[wNAYRUSLOVES1A].speed))-1))
            {
                dead=0;
            }
            
            break;
            
        case pNAYRUSLOVEROCKETTRAILRETURN1:                                             //Nayru's Love Rocket return trail
            if(clk>=(((wpnsbuf[wNAYRUSLOVES1B].frames) * (wpnsbuf[wNAYRUSLOVES1B].speed))-1))
            {
                dead=0;
            }
            
            break;
            
        case pNAYRUSLOVEROCKETRETURN2:                                             //Nayru's Love Rocket return
            if(x<=castx)
            {
                dead=0;
                castnext=true;
            }
            
            break;
            
        case pNAYRUSLOVEROCKETTRAIL2:                                             //Nayru's Love Rocket trail
            if(clk>=(((wpnsbuf[wNAYRUSLOVES2A].frames) * (wpnsbuf[wNAYRUSLOVES2A].speed))-1))
            {
                dead=0;
            }
            
            break;
            
        case pNAYRUSLOVEROCKETTRAILRETURN2:                                             //Nayru's Love Rocket return trail
            if(clk>=(((wpnsbuf[wNAYRUSLOVES2B].frames) * (wpnsbuf[wNAYRUSLOVES2B].speed))-1))
            {
                dead=0;
            }
            
            break;
            
        }
        
        if(blocked()) //not really sure this is needed
        {
            dead=1;
        }
        
        break;
    }
    case wRefMagic:
    case wMagic:
    case ewMagic:
    {
        if((id==wMagic)&&(findentrance(x,y,mfWANDMAGIC,true))) dead=0;
        
        if((id==wRefMagic)&&(findentrance(x,y,mfREFMAGIC,true))) dead=0;
        
        if((id!=ewMagic)&&(findentrance(x,y,mfSTRIKE,true))) dead=0;
	
	    
	
       
	//Create an ER to use this in older quests -V
	if ( get_bit(quest_rules,qr_BROKENBOOKCOST) )
	{
		
                //al_trace("Reached case wRefMagic in weapons.cpp, line %d\n",3407);
		if((id==wMagic && current_item(itype_book) &&
			itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_book)].flags&ITEM_FLAG1) && get_bit(quest_rules,qr_INSTABURNFLAGS))
		{
		    findentrance(x,y,mfBCANDLE,true);
		    findentrance(x,y,mfRCANDLE,true);
		    findentrance(x,y,mfWANDFIRE,true);
		}
	}
	else
	{
                //al_trace("Reached case wRefMagic in weapons.cpp, line %d\n",3418);
		 if((id==wMagic && linkedItem && itemsbuf[linkedItem].family == itype_book &&
                itemsbuf[linkedItem].flags&ITEM_FLAG1) && get_bit(quest_rules,qr_INSTABURNFLAGS))
		{
		    findentrance(x,y,mfBCANDLE,true);
		    findentrance(x,y,mfRCANDLE,true);
		    findentrance(x,y,mfWANDFIRE,true);
		}
	}
		
        
mirrors:
        int checkx=0, checky=0;
        
        switch(dir)
        {
        case up:
            checkx=x+7;
            checky=y+8;
            break;
            
        case down:
            checkx=x+7;
            checky=y;
            break;
            
        case left:
            checkx=x+8;
            checky=y+7;
            break;
            
        case right:
            checkx=x;
            checky=y+7;
            break;
        }
        
        if(ignorecombo!=(((int)checky&0xF0)+((int)checkx>>4)))
        {
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMIRROR))
            {
                weapon *w=NULL;
                
                if(id==ewMagic)
                {
		    w=new weapon(*this);
                    Lwpns.add(w);
                    dead=0;
                }
                else
                {
                    w=this;
                }
                
                
                w->dir ^= 1;
                
                if(w->id != wWind)
                {
                    w->id = wRefMagic; w->convertType(true);
                    
                    if(w->dir&2)
                        w->flip ^= 1;
                    else
                        w->flip ^= 2;
                }
                
                w->ignoreLink=false;
                w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                w->y=checky&0xF0;
                w->x=checkx&0xF0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMIRRORSLASH))
            {
                weapon *w=NULL;
                
                if(id==ewMagic)
                {
		    w=new weapon(*this);
                    Lwpns.add(w);
                    dead=0;
                }
                else
                {
                    w=this;
                }
                
                w->dir = 3-w->dir;
                
                if(w->id != wWind)
                {
                    w->id = wRefMagic; w->convertType(true);
			if ( do_animation ) 
			{
				if((w->dir==1)||(w->dir==2))
				w->flip ^= 3;
			}
                }
                if ( do_animation ) 
		{
			w->tile=w->o_tile;
			
			if(w->dir&2)
			{
			    if(w->frames>1)
			    {
				w->tile+=w->frames;
			    }
			    else
			    {
				++w->tile;
			    }
			}
                }
                w->ignoreLink=false;
                w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                w->y=checky&0xF0;
                w->x=checkx&0xF0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMIRRORBACKSLASH))
            {
                weapon *w=NULL;
                
                if(id==ewMagic)
                {
		    w=new weapon(*this);
                    Lwpns.add(w);
                    dead=0;
                }
                else
                {
                    w=this;
                }
                
                w->dir ^= 2;
                
                if(w->id != wWind)
                {
                    w->id = wRefMagic; w->convertType(true);
			if ( do_animation ) 
			{
				if(w->dir&1)
					w->flip ^= 2;
				else
					w->flip ^= 1;
			}
                }
                if ( do_animation ) 
		{
			w->tile=w->o_tile;
			
			if(w->dir&2)
			{
			    if(w->frames>1)
			    {
				w->tile+=w->frames;
			    }
			    else
			    {
				++w->tile;
			    }
			}
		}
                
                w->ignoreLink=false;
                w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                w->y=checky&0xF0;
                w->x=checkx&0xF0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMAGICPRISM) && (id != wWind))
            {
                int newx, newy;
                newy=checky&0xF0;
                newx=checkx&0xF0;
                
                for(int tdir=0; tdir<4; tdir++)
                {
                    if(dir!=(tdir^1))
                    {
                        weapon *w=new weapon(*this);
                        w->dir=tdir;
			if ( this->angular && get_bit(quest_rules, qr_ANGULAR_REFLECTED_WEAPONS) )
			{
				double newangle = this->angle - DegreesToRadians(90*tdir);
				w->angle = WrapAngle(newangle);
			}
                        w->x=newx;
                        w->y=newy;
                        w->z=z;
                        w->id=wRefMagic; w->convertType(true);
                        w->parentid=parentid;
                        w->parentitem=parentitem;
                        w->flip = 0;
                        w->ignoreLink = false;
                        w->hyofs = w->hxofs = 0;
                        w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                        if ( do_animation ) 
			{
				//also set up the magic's correct animation -DD
				switch(w->dir)
				{
				case down:
				    w->flip=2;
				    
				case up:
				    w->tile = w->o_tile;
				    w->hyofs=2;
				    w->hysz=12;
				    break;
				    
				case left:
				    w->flip=1;
				    
				case right:
				    w->tile=w->o_tile+((w->frames>1)?w->frames:1);
				    w->hxofs=2;
				    w->hxsz=12;
				    break;
				
				default: break;
				}
                        }
                        Lwpns.add(w);
                    }
                }
                
                dead=0;
            }
            
            if(hitcombo((get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (x+hxofs+(hxsz*0.5)) : checkx),(get_bit(quest_rules,qr_MIRRORS_USE_WEAPON_CENTRE) ? (y+hyofs+(hysz*0.5)) : checky),cMAGICPRISM4) && (id != wWind))
            {
                int newx, newy;
                newy=checky&0xF0;
                newx=checkx&0xF0;
                
                for(int tdir=0; tdir<4; tdir++)
                {
                    weapon *w=new weapon(*this);
                    w->dir=tdir;
		    if ( this->angular && get_bit(quest_rules, qr_ANGULAR_REFLECTED_WEAPONS) )
		    {
			double newangle = this->angle - DegreesToRadians(90*tdir);
			w->angle = WrapAngle(newangle);
		    }
                    w->x=newx;
                    w->y=newy;
                    w->z=z;
                    w->id=wRefMagic; w->convertType(true);
                    w->parentid=parentid;
                    w->parentitem=parentitem;
                    w->flip = 0;
                    w->ignoreLink = false;
                    w->hyofs = w->hxofs = 0;
                    w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                    
		    if ( do_animation ) 
		    {
			    //also set up the magic's correct animation -DD
			    switch(w->dir)
			    {
			    case down:
				w->flip=2;
				
			    case up:
				w->tile = w->o_tile;
				w->hyofs=2;
				w->hysz=12;
				break;
				
			    case left:
				w->flip=1;
				
			    case right:
				w->tile=w->o_tile+((w->frames>1)?w->frames:1);
				w->hxofs=2;
				w->hxsz=12;
				break;
			    
			    default: break;
			    }
                    }
                    Lwpns.add(w);
                }
                
                dead=0;
            }
            
            if(blocked(0, 0))
            {
                dead=0;
            }
        }
        //:Weapon Only
        if ( doscript && weaponscript > 0 && (id == wMagic || id == wRefMagic) ) 
        {
                /*if ( Dead() )
                {
                        doscript = 0;
                        weaponscript = 0;
                }
                else*/
                {
                        int w_index = -1; //Give the script the correct index! -Z
                        for(word i = 0; i < Lwpns.Count(); i++)
                        {
                                if(Lwpns.spr(i)->getUID() == getUID())
                                w_index = i;
                                //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                        }
                                
                        //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                        if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS]) ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, ii);		
                }
        }
    }
    break;
    
    // enemy weapons
    case ewFireball2:
        switch(misc)
        {
        case up:
            y-=.5;
            break;
            
        case down:
            y+=.5;
            break;
            
        case left:
            x-=.5;
            break;
            
        case right:
            x+=.5;
            break;
            
        case l_up:
            y-=.354;
            x-=.354;
            break;
            
        case r_up:
            y-=.354;
            x+=.354;
            break;
            
        case l_down:
            y+=.354;
            x-=.354;
            break;
            
        case r_down:
            y+=.354;
            x+=.354;
            break;
        }
        
        //fallthrough
    case wRefFireball:
    case ewFireball:
    {
        if((id==wRefFireball)&&(findentrance(x,y,mfREFFIREBALL,true))) dead=0;
        
        if((id==wRefFireball)&&(findentrance(x,y,mfSTRIKE,true))) dead=0;
        
        if(blocked())
        {
            dead=0;
        }
        
        if(clk<16)
        {
            ++clk;
            
            if(dead>0)
                --dead;
                
            return dead==0;
        }
        
        break;
    }
    
    case ewFlame:
    {
        if(clk==32)
        {
            step=0;
            misc = -1; // Don't drift diagonally anymore
            isLit=true;
            if(get_bit(quest_rules,qr_TEMPCANDLELIGHT)){
			checkLightSources();
		} else {
			checkLightSources(true);
		}
        }
        
        if(clk==126)
        {
            dead=1;
            
            if(get_bit(quest_rules,qr_TEMPCANDLELIGHT))
            {
                isLit=false;
                checkLightSources();
            }
        }
        
        if(blocked())
        {
            dead=1;
        }
        
        // Killed by script?
        if(dead==0 && get_bit(quest_rules,qr_TEMPCANDLELIGHT) && (Lwpns.idCount(wFire) + Ewpns.idCount(ewFlame))==1)
        {
            isLit=false;
            checkLightSources();
        }
        
        break;
    }
    
    case ewFireTrail:
    {
        if(clk==32)
        {
            step=0;  //should already be 0, but still...
            isLit=true;
            if(get_bit(quest_rules,qr_TEMPCANDLELIGHT)){
			checkLightSources();
		} else {
			checkLightSources(true);
		}
        }
        
        if(clk==640)
        {
            dead=1;
            
            if(get_bit(quest_rules,qr_TEMPCANDLELIGHT))
            {
                isLit=false;
                checkLightSources();
            }
        }
        
        if(blocked())
        {
            dead=1;
        }
        
        if ( doscript && weaponscript > 0 && id == wRefFireball) 
        {
                /*if ( Dead() )
                {
                        doscript = 0;
                        weaponscript = 0;
                }
                else*/
                {
                        int w_index = -1; //Give the script the correct index! -Z
                        for(word i = 0; i < Lwpns.Count(); i++)
                        {
                                if(Lwpns.spr(i)->getUID() == getUID())
                                w_index = i;
                                //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                        }
                                
                        //al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
                        if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255  && !(FFCore.system_suspend[susptLWEAPONSCRIPTS])) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, ii);		
                }
        }
        break;
    }
    
    case ewBrang:
    {
        if(clk==0)
        {
            misc2=(dir<left)?y:x;                               // save home position
            
            if(dummy_bool[0]==true)
            {
                seekLink();
            }
            
            if(get_bit(quest_rules,qr_MORESOUNDS))
            {
                //if (step!=0)
                sfx(WAV_BRANG, pan(int(x)), true);
                //else
                ;//stop_sfx(WAV_BRANG);
            }
        }
        
        ++clk2;
        
        if(clk2==45&&!dummy_bool[0])
        {
            misc=1;
            dir^=1;
        }
        
        if(dummy_bool[0])
        {
            step=5;
        }
        else
        {
            if(clk2>27 && clk2<61)
            {
                step=1;
            }
            else if(misc)
            {
                step=2;
            }
            else
            {
                step=3;
            }
        }
        
        bool killrang = false;
        
        if(misc==1)                                           // returning
        {
            if(dummy_bool[0]==true)
            {
                //no parent
                if(parentid < 0)
                    return true;
                    
                //find parent
                int index = -1;
                
                for(int i=0; i<guys.Count(); i++)
                    if(guys.spr(i)->getUID()==parentid)
                    {
                        index = i;
                        break;
                    }
                    
                //parent is not a guy
                if(index < 0)
                    return true;
                    
                seekEnemy(index);
                
                if((abs(x-GuyX(index))<7)&&(abs(y-GuyY(index))<7))
                {
                    if(get_bit(quest_rules,qr_MORESOUNDS) && !Lwpns.idCount(wBrang) && Ewpns.idCount(ewBrang)<=1)
                    {
                        stop_sfx(WAV_BRANG);
                    }
                    
                    return true;
                }
            }
            else
            {
                switch(dir)
                {
                case up:
                    if(y<misc2) killrang=true;
                    
                    break;
                    
                case down:
                    if(y>misc2) killrang=true;
                    
                    break;
                    
                case left:
                    if(x<misc2) killrang=true;
                    
                    break;
                    
                case right:
                    if(x>misc2) killrang=true;
                    
                    break;
                }
            }
        }
        else if(blocked())
        {
            //only obey block flags before the bounce
            dead=1;
        }
        
        //if the boomerang is not on its way home, it bounces
        if(dead==1 && misc != 1)
        {
            dead=-1;
            misc=1;
            dir^=1;
            clk2=zc_max(46,90-clk2);
        }
        
        //otherwise it disappears
        if(killrang || dead==1)
        {
            if(get_bit(quest_rules,qr_MORESOUNDS) && !Lwpns.idCount(wBrang) && Ewpns.idCount(ewBrang)<=1)
            {
                stop_sfx(WAV_BRANG);
            }
            
            dead = 1;
        }
    }
    }
    
    // move sprite, check clipping
    if(dead==-1 && clk>=0)
    {
        move(step);
        
        if(clip())
        {
            onhit(true);
        }
        else if(id==ewRock)
        {
            if(_walkflag(x,y,2) || _walkflag(x,y+8,2))
            {
                onhit(true);
            }
        }
    }
    else if(dead==-10) // Scripting hack thing related to weapon->DeadState
    {
        if(clip())
        {
            onhit(true);
        }
    }
    
    if(bounce)
    {
        switch(dir)
        {
        case up:
            x-=1;
            y+=2;
            break;
            
        case down:
            x+=1;
            y-=2;
            break;
            
        case left:
            x+=2;
            y-=1;
            break;
            
        case right:
            x-=2;
            y-=1;
            break;
        }
    }
    
    // update clocks
    ++clk;
    
    if(dead>0)
    {
        --dead;
    }
    /*
    if ( weaponscript > 0 ) 
    {
	if ( isLinkWeapon() )
	{
		int w_index = -1; //Give the script the correct index! -Z
		for(word i = 0; i < Lwpns.Count(); i++)
		{
			if(Lwpns.spr(i)->getUID() == getUID())
			w_index = i;
			//al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
		}
		if ( !isLinkMelee() ) 
		{
			al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
			FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, ii);
		}
		//else if ( canrunscript > 0 ) 
		//{
		//	al_trace("Found an lweapon index of: %d, when trying to run an lweapon script.\n",w_index);
		//	FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_LWPN, weaponscript, ii);
		//}
		
			
	}
	else //eweapons
	{
		int w_index = -1; //Give the script the correct index! -Z
		for(word i = 0; i < Lwpns.Count(); i++)
		{
			if(Lwpns.spr(i)->getUID() == getUID())
			w_index = i;
		}
		//Z_scripterrlog("Running an EWeapon script (script ID: %d) for item index: %d\n", weaponscript, index);
		if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) ZScriptVersion::RunScript(SCRIPT_EWPN, weaponscript, w_index);
	}
        
    }*/
    
    return dead==0;
}


void weapon::onhit(bool clipped)
{
    onhit(clipped, 0, -1);
}

void weapon::onhit(bool clipped, int special, int linkdir)
{
    if((scriptcoldet&1) == 0 || fallclk || drownclk)
    {
        // These won't hit anything, but they can still go too far offscreen...
        // Unless the compatibility rule is set.
        if(get_bit(quest_rules, qr_OFFSCREENWEAPONS) || !clipped)
            return;
        goto offscreenCheck;
    }
    
    if(special==2)                                            // hit Link's mirror shield
    {
        switch(id)
        {
        case ewFireball2:
        case ewFireball:
            id = wRefFireball;
            ignoreLink=true;
            goto reflect;
            
        case ewRock:
        case ewSword:
        case wRefBeam:
        case ewMagic:
        case wRefMagic:
            //otherwise he can get hit by the newly-created projectile if he's walking into it fast enough -DD
            ignoreLink=true;
            id = ((id==ewMagic || id==wRefMagic) ? wRefMagic : id==ewRock ? wRefRock : wRefBeam);
            goto reflect;
            
        case wScript1:
        case wScript2:
        case wScript3:
        case wScript4:
        case wScript5:
        case wScript6:
        case wScript7:
        case wScript8:
        case wScript9:
        case wScript10:
            // If this isn't set, the weapon may reflect repeatedly
            ignoreLink=true;
reflect:

            if(angular) switch(linkdir)
                {
                case up:
                    angle += (PI - angle) * 2.0;
                    break;
                    
                case down:
                    angle = -angle;
                    break;
                    
                case left:
                    angle += ((-PI/2) - angle) * 2.0;
                    break;
                    
                case right:
                    angle += ((PI/2) - angle) * 2.0;
                    break;
                    
                default:
                    angle += PI;
                    break;
                }
            else
            {
                dir ^= 1;
                
                if(dir&2)
                    flip ^= 1;
                else
                    flip ^= 2;
            }
            
            return;
        }
    }
    
    if(special>=1)                                            // hit Link's shield
    {
        switch(id)
        {
        case ewRock:
        case ewMagic:
        case ewArrow:
        case ewSword:
            bounce=true;
            dead=16;
            return;
            
        case ewBrang:
            if(misc==0)
            {
                clk2=256;
                misc=1;
                dir^=1;
            }
            
            return;
        }
    }
    
offscreenCheck:
    
    if ( get_bit(quest_rules,qr_WEAPONSMOVEOFFSCREEN) || (screenedge&SPRITE_MOVESOFFSCREEN) ) goto skip_offscreenCheck;
    switch(id)
    {
    case wSword:
    case wWand:
    case wHammer:
        break;
        
    case ewBomb:
        step=0;
        break;
        
    case ewLitBomb:
        step=0;
        misc=50;
        clk=misc-3;
        hxofs=hyofs=-7;
        hxsz=hysz=30;
        break;
        
    case ewSBomb:
        step=0;
        break;
        
    case ewLitSBomb:
        step=0;
        misc=50;
        clk=misc-3;
        hxofs=hyofs=-16;
        hxsz=hysz=48;
        break;
        
    case wLitBomb:
        if(!clipped) dead=1;
        
    case wLitSBomb:
        if(!clipped) dead=1;
        
    case wWhistle:
    case wBomb:
    case wSBomb:
    case wBait:
    case wFire:
    case wHSHandle:
    case wPhantom:
        break;                                   // don't worry about clipping or hits with these
        
    case ewFireTrail:
        if(!clipped) dead=1;
        
        break;
    
    case ewFlame:
        if(!clipped) dead=1;
        
        break;
        
    case wRefBeam:
    case wBeam:
        dead=23;
        break;
        
    case wArrow:
        dead=4;
        break;                           //findentrance(x,y,mfARROW,true); break;
        
    case ewArrow:
        dead=clipped?4:1;
        break;
        
    case wCByrna:
    
        // byrna keeps going
        if(parentitem<0 || !(itemsbuf[parentitem].flags&ITEM_FLAG1))
            dead=0;
            
        break;
        
    case wWind:
        if(x>=240)
            dead=2;
            
        break;
        
    case wBrang:
        if(misc==0)
        {
            clk2=256;
            int deadval=(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].flags & ITEM_FLAG3)?-2:4;
	    if(clipped)
            {
                dead=deadval;
            }
            else
            {
                if(deadval==-2)
                {
                    dead=deadval;
                }
                
                misc=1;
                /*
                  if (current_item(itype_brang,true)>1) {
                  if (dummy_bool[0]) {
                  add_grenade(x,y,z,current_item(itype_brang,true)>2);
                  dummy_bool[0]=false;
                  }
                  }
                  */
            }
        }
        
        break;
        
    case wHookshot:
        if(misc==0)
        {
            clk2=256;
            
            if(clipped)
                dead=4;
            else
                dead=1;
        }
        
        break;
        
    case ewBrang:
        if(misc==0)
        {
            clk2=256;
            dead=4;
        }
        
        break;
        
    case wRefMagic:
    case wMagic:
        dead=1; //remove the dead part to make the wand only die when clipped
        
	if ( get_bit(quest_rules, qr_BROKENBOOKCOST) )
	{
	//Create an ER to sue this in older quests -V
		//used a QR. -Z
		if(((id==wMagic && current_item(itype_book) &&
			(itemsbuf[current_item_id(itype_book)].flags&ITEM_FLAG1))) && Lwpns.idCount(wFire)<2)
		{
		    Lwpns.add(new weapon(x,y,z,wFire,2,1*DAMAGE_MULTIPLIER,0,current_item_id(itype_book),-1));
		    if ( FFCore.getQuestHeaderInfo(vZelda) < 0x255 ) 
		    {
			sfx(WAV_FIRE,pan(x));
		    }
			
		    else sfx(itemsbuf[linkedItem].usesound > 0 ? itemsbuf[linkedItem].usesound : WAV_FIRE,pan(x));
		}
	}
	else
	{
		if(((id==wMagic && linkedItem && itemsbuf[linkedItem].family==itype_book &&
			(itemsbuf[linkedItem].flags&ITEM_FLAG1))) && Lwpns.idCount(wFire)<2)
		{
		    Lwpns.add(new weapon(x,y,z,wFire,2,1*DAMAGE_MULTIPLIER,0,linkedItem,-1));
		    if ( FFCore.getQuestHeaderInfo(vZelda) < 0x255 ) 
		    {
			sfx(WAV_FIRE,pan(x));
		    }
		    else sfx(itemsbuf[linkedItem].usesound > 0 ? itemsbuf[linkedItem].usesound : WAV_FIRE,pan(x));
		}
        }
        break;
        
    case ewWind:
        if(clipped)
        {
            if(misc==999)                                       // in enemy wind
            {
                ewind_restart=true;
            }
            
            dead=1;
        }
        
        break;
        
	case wScript1:
	case wScript2:
	case wScript3:
	case wScript4:
	case wScript5:
	case wScript6:
	case wScript7:
	case wScript8:
	case wScript9:
	case wScript10:
	{
		
		if(!clipped || !get_bit(quest_rules,qr_CHECKSCRIPTWEAPONOFFSCREENCLIP) ) dead=1;
		break;
	}
    default:
        dead=1;
    }
    skip_offscreenCheck:
    
    {
	//do not remove these braces!
    }
}

// override hit detection to check for invicibility, etc
bool weapon::hit(sprite *s)
{
    if(!(scriptcoldet&1) || fallclk || drownclk) return false;
    
    if(id==ewBrang && misc)
        return false;
        
    return (Dead()&&dead!=-10) ? false : sprite::hit(s);
}

bool weapon::hit(int tx,int ty,int tz,int txsz2,int tysz2,int tzsz2)
{
    if(!(scriptcoldet&1) || fallclk || drownclk) return false;
    
    if(id==ewBrang && misc)
        return false;
        
    return (Dead()&&dead!=-10) ? false : sprite::hit(tx,ty,tz,txsz2,tysz2,tzsz2);
}

void weapon::update_weapon_frame(int change, int orig)
{
    if(extend > 2)
    {
        byte extra_rows=(tysz-1)*(((orig%TILES_PER_ROW)+(txsz*change))/TILES_PER_ROW);
        tile=orig+(txsz*change)+(extra_rows*TILES_PER_ROW);
    }
    else
        tile=orig+change;
}

void weapon::draw(BITMAP *dest)
{
    if(weapon_dying_frame) return;
	if(fallclk || drownclk)
	{
		sprite::draw(dest);
		return;
	}
    if(flash==1)
    {
        if(!BSZ)
        {
            cs = (id==wBeam || id==wRefBeam) ? 6 : o_cset&15;
            cs += frame&3;
        }
        else
        {
            if(id==wBeam || id==wRefBeam)
                cs = ((frame>>2)&1)+7;
            else
            {
                cs = o_cset&15;
                
                if(++csclk >= 12)
                    csclk = 0;
                    
                cs += csclk>>2;
            }
        }
    }
    
    if(flash>1)
    {
        if(++csclk >= (o_speed<<1))
            csclk = 0;
            
        cs = o_cset&15;
        
        if(csclk >= o_speed)
            cs = o_cset>>4;
    }
    
    if(frames)
    {
        if(id!=wBrang && id!=ewBrang)  // Brangs use clk2 for adjusting boomerang's flight range.
        {
            if(++clk2 >= o_speed)
            {
                clk2 = 0;
                
                if(frames>1 && ++aframe >= frames)
                {
                    aframe = 0;
                }
            }
        }
        else
        {
            if((clk2 % zc_max(1, o_speed))==0)
            {
                if(frames>1 && ++aframe >= frames)
                {
                    aframe = 0;
                }
            }
        }
        
        //tile = o_tile+aframe;
	if ( do_animation ) 
	{
		update_weapon_frame(aframe,o_tile);
	}
    }
    
    // do special case stuff
    switch(id)
    {
    case wSword:
    case wHammer:
        if(get_bit(quest_rules,qr_LINKFLICKER)&&((getClock()||LinkHClk())&&(frame&1)) ||
                Link.getDontDraw() || tmpscr->flags3&fINVISLINK)
            return;
            
    case wBeam:
    case wRefBeam:
    {
        if(dead==-1) break;
        
        // draw the beam shards
        int ofs=23-dead;
        int f = frame&3;
        int type2 = o_type;
        //tile = o_tile+((frames?frames:1)*2);
        update_weapon_frame(((frames?frames:1)*2),o_tile);
        
        if(type2)
            cs = o_cset>>4;
	if ( do_animation ) 
	{
		if(type2==3 && (f&2))
		++tile;
	}
        if(!type2 || f==0 || (type2>1 && f==3)) overtile16(dest,tile,x-2-ofs,y+playing_field_offset-2-ofs-(z+zofs),cs,0);
        
        if(!type2 || f==2 || (type2>1 && f==1)) overtile16(dest,tile,x+2+ofs,y+playing_field_offset-2-ofs-(z+zofs),cs,1);
        
        if(!type2 || f==1 || (type2>1 && f==2)) overtile16(dest,tile,x-2-ofs,y+playing_field_offset+2+ofs-(z+zofs),cs,2);
        
        if(!type2 || f==3 || (type2>1 && f==0)) overtile16(dest,tile,x+2+ofs,y+playing_field_offset+2+ofs-(z+zofs),cs,3);
    }
    
    return;                                               // don't draw sword
    
    case wBomb:
    case wSBomb:
    case ewBomb:
    case ewSBomb:
    {
        if(!misc || clk<misc-2) break;
        
        // draw the explosion
        int id2=0;
        int boomframes = 0;
        
        switch(id)
        {
        case wBomb:
        {
            id2=wBOOM;
            
            if(parentitem>-1)
            {
                id2=itemsbuf[parentitem].wpn2;
            }
            
            break;
        }
        
        case wSBomb:
        {
            id2=wSBOOM;
            
            if(parentitem>-1)
            {
                id2=itemsbuf[parentitem].wpn2;
            }
            
            break;
        }
        
        case ewBomb:
            id2=ewBOOM;
            break;
            
        case ewSBomb:
            id2=ewSBOOM;
            break;
        }
        
        tile = wpnsbuf[id2].newtile;
        cs = wpnsbuf[id2].csets&15;
        boomframes = wpnsbuf[id2].frames;
        
        if(boomframes != 0)
        {
            //equally divide up the explosion time among the frames
            int perframe = (boomframes >= 34 ? 1 : (boomframes + 34)/boomframes);
            if ( do_animation ) 
	    {
		if(clk > misc)
			tile += (clk-misc)/perframe;
	    }
            //update_weapon_frame((clk-misc)/perframe,tile);
        }
        else if(clk>misc+22)
        {
            if ( do_animation ) ++tile;
            //update_weapon_frame(1,tile);
        }
        
        overtile16(dest,tile,x+((clk&1)?7:-7),y+yofs-13-(z+zofs),cs,0);
        overtile16(dest,tile,x,y+yofs-(z+zofs),cs,0);
        overtile16(dest,tile,x+((clk&1)?-14:14),y+yofs-(z+zofs),cs,0);
        overtile16(dest,tile,x+((clk&1)?-7:7),y+yofs+14-(z+zofs),cs,0);
        
        if(id==wSBomb||id==ewSBomb)
        {
            overtile16(dest,tile,x+((clk&1)?7:-7),y+yofs-27-(z+zofs),cs,0);
            overtile16(dest,tile,x+((clk&1)?-21:21),y+yofs-13-(z+zofs),cs,0);
            overtile16(dest,tile,x+((clk&1)?-28:28),y+yofs-(z+zofs),cs,0);
            overtile16(dest,tile,x+((clk&1)?21:-21),y+yofs+14-(z+zofs),cs,0);
            overtile16(dest,tile,x+((clk&1)?-7:7),y+yofs+28-(z+zofs),cs,0);
        }
        else
        {
            ;
        }
        
        if(get_debug() && zc_getkey(KEY_O))
            rectfill(dest,x+hxofs,y+hyofs+yofs-(z+zofs),
                     x+hxofs+hxsz-1,y+hyofs+hysz-1+yofs,vc(id));
                     
        return;                                               // don't draw bomb
    }
    
    case wArrow:
    case ewArrow:
        if(dead>0 && !bounce)
        {
		if ( do_animation ) 
		{
			cs=7;
			tile=54;
			flip=0;
		}
        }
        
        break;
        
    case ewFireTrail:
    case ewFlame:
    case wFire:
    
        //if no animation, flip tile
        if(frames==0 && do_animation ) //do_animation is a ZScript setting. -Z
            flip = o_flip & (clk>>2);
            
        break;
        
    case ewBrang:
    case wBrang:
		cs = o_cset&15;
		
		if((id == wBrang && (parentitem<0 || !(itemsbuf[parentitem].flags & ITEM_FLAG1)))
			|| (id == ewBrang && !get_bit(quest_rules,qr_CORRECTED_EW_BRANG_ANIM)))
		{
			if ( do_animation ) 
			{
				tile = o_tile;
				
				if(BSZ)
				flip = bszboomflip[(clk>>2)&3];
				else
				{
				//tile = boomframe[clk&0xE] + o_tile;
				update_weapon_frame(boomframe[clk&0xE],o_tile);
				flip = boomframe[(clk&0xE)+1];
				}
				
				if(parentitem>=0 && itemsbuf[parentitem].flags & ITEM_FLAG2)
				{
				update_weapon_frame((BSZ?1:4)*dir,tile);
				}
			}
		}
		else
		{
			if(parentitem>=0 && itemsbuf[parentitem].flags & ITEM_FLAG2)
			{
				if ( do_animation )update_weapon_frame(zc_max(frames,1)*dir,tile);
			}
		}
        
        if(dead>0)
        {
            if(parentitem>=0 && itemsbuf[parentitem].flags & ITEM_FLAG1)
            {
		if ( do_animation ) 
		{
			tile=o_tile+(frames*(itemsbuf[parentitem].flags & ITEM_FLAG2)?8:1);
		}
            }
            else
            {
		if ( do_animation ) 
		{
			cs=7;
			tile=54;
			flip=0;
		}
            }
        }
        
        break;
        
    case wHookshot:
        break;
        
    case wWind:
        if(frames==0 && do_animation)
            flip ^= o_flip;
            
        if(Dead() && !BSZ && do_animation)
            tile = temp1;//wpnsbuf[wFIRE].tile;
            
        break;
        
    case ewWind:
        /*
          if(wpnsbuf[wid].frames==0)
          flip ^= (wpnsbuf[wid].misc>>2)&3;
          */
        break;
        
    case wPhantom:
        switch(type)
        {
        case pNAYRUSLOVEROCKET1:
        case pNAYRUSLOVEROCKETRETURN1:
        case pNAYRUSLOVEROCKETTRAIL1:
        case pNAYRUSLOVEROCKETTRAILRETURN1:
        case pNAYRUSLOVEROCKET2:
        case pNAYRUSLOVEROCKETRETURN2:
        case pNAYRUSLOVEROCKETTRAIL2:
        case pNAYRUSLOVEROCKETTRAILRETURN2:
            if(parentitem>=0 && (itemsbuf[parentitem].flags & ITEM_FLAG1 ? 1 : 0)&&!(frame&1))
            {
                return;
            }
            
            break;
        }
        
	case wScript1:
	case wScript2:
	case wScript3:
	case wScript4:
	case wScript5:
	case wScript6:
	case wScript7:
	case wScript8:
	case wScript9:
	case wScript10:
	{
		if ( do_animation )
		{
			//Bugfix script weapons not animating:
			//Let's see if this works, and failstobreakanything. -Z
			//This also will need a QR, if it works!
			/* Bugged, disabling.
			if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255 && get_bit(quest_rules, qr_ANIMATECUSTOMWEAPONS) )
			{
				if(frames>1 && ++aframe >= frames)
				{
				    aframe = 0;
				}
				//update_weapon_frame(aframe,o_tile);
				update_weapon_frame(aframe,o_tile);
			}
			*/
			//al_trace("script_wrote_otile = %d\n",script_wrote_otile);
			//if ( ScriptGenerated && script_wrote_otile && aframe > 0 ) 
			//{ 
			//	script_wrote_otile = 0; // NOTES and ISSUES
							// I honestly do not recall when or why I added this. I think that it was
							// in an attempt to fix Tile not being reset when writing to OTile. 
							// 
							// PROBLEM
							// Doing any of this on the first frame of a weapon will overwrite a script-set
							// tile with the original tile if the scripter does this in a script:
							//
							// this->OriginalTile = 6;
							// this->Tile = 12345; 	// will be overwritten by o_tile because of the 
							//			// script_wrote_otile FLAG being checked by the engine
							//			// after the script writes to tile! -Z 26th October, 2019
				
				//tile = o_tile; //This will overwrite the tile on the calls above, so we can't do it. Fuck it. 
			//}
		}
		//Z_scripterrlog("weapon::draw() o_tile is: %d\n", o_tile);
	}
	
        break;
	
    }
    
    // draw it
    
    if ( z > 0 && get_bit(quest_rules, qr_WEAPONSHADOWS) )
    {
	shadowtile = wpnsbuf[iwShadow].newtile+aframe;
	sprite::drawshadow(dest,get_bit(quest_rules, qr_TRANSSHADOWS) != 0);
    }
    sprite::draw(dest);
}

void putweapon(BITMAP *dest,int x,int y,int weapon_id, int type, int dir, int &aclk, int &aframe, int parentid)
{
    weapon temp((zfix)x,(zfix)y,(zfix)0,weapon_id,type,0,dir,-1,parentid,true);
    temp.ignorecombo=((dir==up?y+8:y)&0xF0)+((dir==left?x+8:x)>>4); // Lens hints can sometimes create real weapons without this
    temp.ignoreLink=true;
    temp.yofs=0;
    temp.clk2=aclk;
    temp.aframe=aframe;
    temp.doscript = 0; //Running q script would cause a crash, as this weapon has no parent data to use to determine if it should run an lweapon, or eweapon script.
    temp.script = 0; //Can not have script data.
    temp.animate(0); //Scripts run in this function. Call after forcing script data to 0.
    temp.draw(dest);
    aclk=temp.clk2;
    aframe=temp.aframe;
}


void weapon::findcombotriggers()
{
	for(int dx = 0; dx < hxsz; dx += 16)
	{
		for(int dy = 0; dy < hysz; dy += 16)
		{
			for (int ly = 0; ly < ((get_bit(quest_rules,qr_CUSTOMCOMBOSLAYERS1AND2)) ? 3 : 1); ++ly )
			{
				MatchComboTrigger2(this, (int)x+dx+hxofs, (int)y+dy+hyofs, combobuf, ly);
				MatchComboTrigger2(this, (int)x+dx+hxofs, (int)y+dy+hyofs, combobuf, ly);
				MatchComboTrigger2(this, (int)x+dx+hxofs, (int)y+dy+hyofs, combobuf, ly);
				MatchComboTrigger2(this, (int)x+dx+hxofs, (int)y+dy+hyofs, combobuf, ly);
			}
			
			MatchComboTrigger(this, (int)x+dx+hxofs, (int)y+dy+hyofs, combobuf);
			MatchComboTrigger(this, (int)x+dx+hxofs, (int)y+dy+hyofs, combobuf);
			MatchComboTrigger(this, (int)x+dx+hxofs, (int)y+dy+hyofs, combobuf);
			MatchComboTrigger(this, (int)x+dx+hxofs, (int)y+dy+hyofs, combobuf);
		}
		for (int ly = 0; ly < ((get_bit(quest_rules,qr_CUSTOMCOMBOSLAYERS1AND2)) ? 3 : 1); ++ly )
		{
			MatchComboTrigger2(this, (int)x+dx+hxofs, (int)y+hyofs+(hysz-1), combobuf, ly);
			MatchComboTrigger2(this, (int)x+dx+hxofs, (int)y+hyofs+(hysz-1), combobuf, ly);
			MatchComboTrigger2(this, (int)x+dx+hxofs, (int)y+hyofs+(hysz-1), combobuf, ly);
			MatchComboTrigger2(this, (int)x+dx+hxofs, (int)y+hyofs+(hysz-1), combobuf, ly);
		}
		MatchComboTrigger(this, (int)x+dx+hxofs, (int)y+hyofs+(hysz-1), combobuf);
		MatchComboTrigger(this, (int)x+dx+hxofs, (int)y+hyofs+(hysz-1), combobuf);
		MatchComboTrigger(this, (int)x+dx+hxofs, (int)y+hyofs+(hysz-1), combobuf);
		MatchComboTrigger(this, (int)x+dx+hxofs, (int)y+hyofs+(hysz-1), combobuf);
		
	}
	for(int dy = 0; dy < hysz; dy += 16)
	{
		for (int ly = 0; ly < ((get_bit(quest_rules,qr_CUSTOMCOMBOSLAYERS1AND2)) ? 3 : 1); ++ly )
		{
			MatchComboTrigger2(this, (int)x+hxofs+(hxsz-1), (int)y+dy+hyofs, combobuf, ly);
			MatchComboTrigger2(this, (int)x+hxofs+(hxsz-1), (int)y+dy+hyofs, combobuf, ly);
			MatchComboTrigger2(this, (int)x+hxofs+(hxsz-1), (int)y+dy+hyofs, combobuf, ly);
			MatchComboTrigger2(this, (int)x+hxofs+(hxsz-1), (int)y+dy+hyofs, combobuf, ly);
		}
		MatchComboTrigger(this, (int)x+hxofs+(hxsz-1), (int)y+dy+hyofs, combobuf);
		MatchComboTrigger(this, (int)x+hxofs+(hxsz-1), (int)y+dy+hyofs, combobuf);
		MatchComboTrigger(this, (int)x+hxofs+(hxsz-1), (int)y+dy+hyofs, combobuf);
		MatchComboTrigger(this, (int)x+hxofs+(hxsz-1), (int)y+dy+hyofs, combobuf);
		
	}
	for (int ly = 0; ly < ((get_bit(quest_rules,qr_CUSTOMCOMBOSLAYERS1AND2)) ? 3 : 1); ++ly )
	{
		MatchComboTrigger2(this, (int)x+hxofs+(hxsz-1), (int)y+hyofs+(hysz-1), combobuf, ly);
		MatchComboTrigger2(this, (int)x+hxofs+(hxsz-1), (int)y+hyofs+(hysz-1), combobuf, ly);
		MatchComboTrigger2(this, (int)x+hxofs+(hxsz-1), (int)y+hyofs+(hysz-1), combobuf, ly);
		MatchComboTrigger2(this, (int)x+hxofs+(hxsz-1), (int)y+hyofs+(hysz-1), combobuf, ly);
	}
	
	MatchComboTrigger(this, (int)x+hxofs+(hxsz-1), (int)y+hyofs+(hysz-1), combobuf);
	MatchComboTrigger(this, (int)x+hxofs+(hxsz-1), (int)y+hyofs+(hysz-1), combobuf);
	MatchComboTrigger(this, (int)x+hxofs+(hxsz-1), (int)y+hyofs+(hysz-1), combobuf);
	MatchComboTrigger(this, (int)x+hxofs+(hxsz-1), (int)y+hyofs+(hysz-1), combobuf);
}

int weapon::run_script(int mode)
{
	if (script <= 0 || !doscript || FFCore.getQuestHeaderInfo(vZelda) < 0x255 || FFCore.system_suspend[isLWeapon ? susptLWEAPONSCRIPTS : susptEWEAPONSCRIPTS])
		return RUNSCRIPT_OK;
	int ret = RUNSCRIPT_OK;
	switch(mode)
	{
		case MODE_NORMAL:
			return ZScriptVersion::RunScript(isLWeapon ? SCRIPT_LWPN : SCRIPT_EWPN, script, getUID());
		case MODE_WAITDRAW:
			if(waitdraw)
			{
				ret = ZScriptVersion::RunScript(isLWeapon ? SCRIPT_LWPN : SCRIPT_EWPN, script, getUID());
				waitdraw = 0;
			}
			break;
	}
    return ret;
}

//Dummy weapon for visual effects.
weapon::weapon(zfix X,zfix Y,zfix Z,int Id,int usesprite, int Dir, int step, int prntid, int height, int width, int a, int b, int c, int d, int e, int f, int g) : sprite(), parentid(prntid)
{
	//Z_scripterrlog("Dummy weapon param(%s) is: %d\n", "X", (int)X);
	//Z_scripterrlog("Dummy weapon param(%s) is: %d\n", "Y", (int)Y);
	//Z_scripterrlog("Dummy weapon param(%s) is: %d\n", "Z", (int)Z);
	//Z_scripterrlog("Dummy weapon param(%s) is: %d\n", "Id", Id);
	//Z_scripterrlog("Dummy weapon param(%s) is: %d\n", "usesprite", usesprite);
	//Z_scripterrlog("Dummy weapon param(%s) is: %d\n", "Dir", Dir);
	//Z_scripterrlog("Dummy weapon param(%s) is: %d\n", "step", step);
	//Z_scripterrlog("Dummy weapon param(%s) is: %d\n", "prntid", prntid);
	//Z_scripterrlog("Dummy weapon param(%s) is: %d\n", "height", height);
	//Z_scripterrlog("Dummy weapon param(%s) is: %d\n", "width", width);
    x=X;
    y=Y;
    z=Z;
	txsz = width > 0 ? width : 1;
	tysz = height > 0 ? height : 1;
    id=Id;
    type=0;
    power=0;
    specialinfo = 0;
    parentitem=-1;
    dir=zc_max(Dir,0);
    clk=clk2=flip=misc=misc2=0;
    frames=flash=wid=aframe=csclk=0;
    ignorecombo=-1;
    step=0;
    dead=-1;
    bounce= false;
	ignoreLink=true;
    yofs=playing_field_offset - 2;
    dragging=-1;
    hxsz=1;
    hysz=1;
    hzsz=1;
	hyofs = -32768;
    useweapon = usedefence = 0;
    weaprange = weapduration = 0;
    script_wrote_otile = 0;
    isLWeapon = 0;
    ScriptGenerated = 0;
    LOADGFX(usesprite);
    //Z_scripterrlog("After calling LOADGFX(), the dummy weapon o_tile is: %d\n", o_tile);
    step=0;
    doscript = 0;
    weaponscript = 0;
	weapon_dying_frame = false;
}

/*** end of weapons.cpp ***/

