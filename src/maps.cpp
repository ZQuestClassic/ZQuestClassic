//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  maps.cc
//
//  Map and screen scrolling stuff for zelda.cc
//
//--------------------------------------------------------

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include <string.h>
#include <assert.h>
#include <math.h>
#include <vector>
#include <deque>
#include <string>

//#include "zc_math.h"
#include "maps.h"
#include "zelda.h"
#include "tiles.h"
#include "sprite.h"
#include "jwin.h"
#include "zsys.h"
#include "subscr.h"
#include "zc_subscr.h"
#include "link.h"
#include "guys.h"
#include "ffscript.h"
extern word combo_doscript[176];
extern refInfo screenScriptData;
extern FFScript FFCore;
#include "particles.h"
#include "mem_debug.h"


#define EPSILON 0.01 // Define your own tolerance
#define FLOAT_EQ(x,v) (((v - EPSILON) < x) && (x <( v + EPSILON)))
#define DegtoFix(d)     ((d)*0.71111111)


//MSVC does not provide a log2 funcion in <cmath>
// adjust version number as needed. MSVC 2015 (1900) does *not* need these functions
#if _MSC_VER < 1900
double log2(double n)
{
    return log(n) / log(2.0);
}
float log2(float n)
{
    return log(n) / log(2.f);
}
#endif


FONT *get_zc_font(int index);

extern sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations, particles;
extern movingblock mblock2;                                 //mblock[4]?
extern zinitdata zinit;
extern LinkClass Link;
int current_ffcombo=-1;

short ffposx[32]= {-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
                   -1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000
                  };
short ffposy[32]= {-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
                   -1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000
                  };
long ffprvx[32]= {-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
                  -10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000
                 };
long ffprvy[32]= {-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
                  -10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000
                 };




void Z_message_d(const char *format,...)
{
#ifdef _DEBUG
    char buf[512];
    va_list ap;
    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);
    
    al_trace("%s",buf);
#else
    format=format;
#endif
}



//bool draw_screen_clip_rect_show_link=true;
//bool draw_screen_clip_rect_show_guys=false;
bool checktrigger=false;

void debugging_box(int x1, int y1, int x2, int y2)
{
    //reference/optimization: the start of the unused drawing command index can now be queried. -Gleeok
    int index = script_drawing_commands.GetNext();
    
    if(index < 0)
        return;
        
    int *sdci = &script_drawing_commands[index][0];
    
    sdci[0] = RECTR;
    sdci[1] = 30000;
    sdci[2] = x1*10000;
    sdci[3] = y1*10000;
    sdci[4] = x2*10000;
    sdci[5] = y2*10000;
    sdci[6] = 10000;
    sdci[7] = 10000;
    sdci[8] = 0;
    sdci[9] = 0;
    sdci[10] = 0;
    sdci[11] = 10000;
    sdci[12] = 1280000;
}

void clear_dmap(word i)
{
    memset(&DMaps[i],0,sizeof(dmap));
}

void clear_dmaps()
{
    for(int i=0; i<MAXDMAPS; i++)
    {
        clear_dmap(i);
    }
}

int isdungeon(int dmap, int scr) // The arg is only used by loadscr2 and loadscr
{
    if(scr < 0) scr=currscr;
    
    if(dmap < 0) dmap = currdmap;
    
    // dungeons can have any dlevel above 0
    if((DMaps[dmap].type&dmfTYPE) == dmDNGN)
    {
        if(TheMaps[(currmap*MAPSCRS)+scr].flags6&fCAVEROOM)
            return 0;
            
        return 1;
    }
    
    // dlevels that aren't dungeons are caves
    if(TheMaps[(currmap*MAPSCRS)+scr].flags6&fDUNGEONROOM)
        return 1;
        
    return 0;
}

bool canPermSecret(int dmap, int scr)
{
	return (!isdungeon(dmap, scr) || get_bit(quest_rules,qr_DUNGEON_DMAPS_PERM_SECRETS));
}

int MAPCOMBO(int x,int y)
{
    //extend combos outwards if out of bounds -DD
    x = vbound(x, 0, (16*16)-1);
    y = vbound(y, 0, (11*16)-1);
    int combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return tmpscr->data[combo];                               // entire combo code
}

int MAPCOMBOzq(int x,int y)
{
	return MAPCOMBO(x,y);
}

//specific layers 1 to 6
int MAPCOMBOL(int layer,int x,int y)
{
    
    if(tmpscr2[layer-1].data.empty()) return 0;
    
    if(tmpscr2[layer-1].valid==0) return 0;
    
    int combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return tmpscr2[layer-1].data[combo];                        // entire combo code
}

int MAPCSETL(int layer,int x,int y)
{
    
    if(tmpscr2[layer-1].cset.empty()) return 0;
    
    if(tmpscr2[layer-1].valid==0) return 0;
    
    int combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return tmpscr2[layer-1].cset[combo];                        // entire combo code
}

int MAPFLAGL(int layer,int x,int y)
{
    
    if(tmpscr2[layer-1].sflag.empty()) return 0;
    
    if(tmpscr2[layer-1].valid==0) return 0;
    
    int combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return tmpscr2[layer-1].sflag[combo];                       // flag
}

int COMBOTYPEL(int layer,int x,int y)
{
    
    if(tmpscr2[layer-1].valid==0)
    {
        return 0;
    }
    
    return combobuf[MAPCOMBO2(layer,x,y)].type;
}

int MAPCOMBOFLAGL(int layer,int x,int y)
{
    if(layer==-1) return MAPCOMBOFLAG(x,y);
    
    if(tmpscr2[layer-1].data.empty()) return 0;
    
    if(tmpscr2[layer-1].valid==0) return 0;
    
    int combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return combobuf[tmpscr2[layer-1].data[combo]].flag;                        // entire combo code
}


// True if the FFC covers x, y and is not ethereal or a changer.
// Used by MAPFFCOMBO(), MAPFFCOMBOFLAG, and getFFCAt().
inline bool ffcIsAt(int index, int x, int y)
{
    int fx=tmpscr->ffx[index]/10000;
    if(x<fx || x>fx+(tmpscr->ffwidth[index]&63)) // FFC sizes are weird.
        return false;
    
    int fy=tmpscr->ffy[index]/10000;
    if(y<fy || y>fy+(tmpscr->ffheight[index]&63))
        return false;
    
    if((tmpscr->ffflags[index]&(ffCHANGER|ffETHEREAL))!=0)
        return false;
	
    if(tmpscr->ffdata[index]<=0)
        return false;
    
    return true;
}

int MAPFFCOMBO(int x,int y)
{
    for(int i=0; i<32; i++)
    {
        if(ffcIsAt(i, x, y))
            return tmpscr->ffdata[i];
    }
    
    return 0;
}

int MAPCSET(int x,int y)
{
    if(x<0 || x>255 || y<0 || y>175)
        return 0;
        
    int combo = COMBOPOS(x,y);
    return tmpscr->cset[combo];                               // entire combo code
}

int MAPFLAG(int x,int y)
{
    if(x<0 || x>255 || y<0 || y>175)
        return 0;
        
    int combo = COMBOPOS(x,y);
    return tmpscr->sflag[combo];                              // flag
}

int COMBOTYPE(int x,int y)
{
	for (int i = 0; i <= 1; ++i)
	{
		if(tmpscr2[i].valid!=0)
		{
			if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, &(tmpscr2[i]))) return cNONE;
		}
	}
	int b=1;
    
	if(x&8) b<<=2;
    
	if(y&8) b<<=1;
	newcombo const& cmb = combobuf[MAPCOMBO(x,y)];
	if (cmb.type == cWATER && (cmb.usrflags&cflag4) && (cmb.walk&b)) return cSHALLOWWATER;
	if (cmb.type == cWATER && (cmb.usrflags&cflag3) && (cmb.walk&b)) return cNONE;
	return cmb.type;
}

int FFCOMBOTYPE(int x,int y)
{
    return combobuf[MAPFFCOMBO(x,y)].type;
}

int FFORCOMBO(int x, int y)
{
	for(int i=0; i<32; i++)
    {
        if(ffcIsAt(i, x, y))
            return tmpscr->ffdata[i];
    }
	
	return MAPCOMBO(x,y);
}

int FFORCOMBOTYPE(int x, int y)
{
	for (int i = 0; i <= 1; ++i)
	{
		if(tmpscr2[i].valid!=0)
		{
			if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, &(tmpscr2[i]))) return cNONE;
		}
	}
	int b=1;
    
	if(x&8) b<<=2;
    
	if(y&8) b<<=1;
	newcombo const& cmb = combobuf[FFORCOMBO(x,y)];
	if (cmb.type == cWATER && (cmb.usrflags&cflag4) && (cmb.walk&b)) return cSHALLOWWATER;
	if (cmb.type == cWATER && (cmb.usrflags&cflag3) && (cmb.walk&b)) return cNONE;
	return cmb.type;
}

int FFORCOMBO_L(int layer, int x, int y)
{
	for(int i=0; i<32; i++)
    {
        if(ffcIsAt(i, x, y))
            return tmpscr->ffdata[i];
    }
	
	return layer ? MAPCOMBOL(layer, x, y) : MAPCOMBO(x,y);
}

int FFORCOMBOTYPE_L(int layer, int x, int y)
{
	return combobuf[FFORCOMBO_L(layer,x,y)].type;
}

int MAPCOMBOFLAG(int x,int y)
{
    if(x<0 || x>255 || y<0 || y>175)
        return 0;
        
    int combo = COMBOPOS(x,y);
    return combobuf[tmpscr->data[combo]].flag;                               // entire combo code
}

int MAPFFCOMBOFLAG(int x,int y)
{
    for(int i=0; i<32; i++)
    {
        if(ffcIsAt(i, x, y))
        {
            current_ffcombo = i;
            return combobuf[tmpscr->ffdata[i]].flag;
        }
    }
    
    current_ffcombo=-1;
    return 0;
}

int getFFCAt(int x, int y)
{
    for(int i=0; i<32; i++)
    {
        if(ffcIsAt(i, x, y))
            return i;
    }
    
    return -1;
}

int MAPCOMBO2(int layer,int x,int y)
{
    if(layer<=-1) return MAPCOMBO(x,y);
    
    if(tmpscr2[layer].data.empty()) return 0;
    
    if(tmpscr2[layer].valid==0) return 0;
    
    int combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return tmpscr2[layer].data[combo];                        // entire combo code
}

int MAPCOMBO3(int map, int screen, int layer, int x,int y, bool secrets)
{
	return MAPCOMBO3(map, screen, layer, COMBOPOS(x,y), secrets);
}

int MAPCOMBOX(int pos) { return ((pos)%16*16); }
int MAPCOMBOY(int pos) { return ((pos)&0xF0); }

int MAPCOMBO3(int map, int screen, int layer, int pos, bool secrets)
{ 
	if (map < 0 || screen < 0) return 0;
	
	if (map == currmap && screen == currscr) return MAPCOMBO2(layer,MAPCOMBOX(pos),MAPCOMBOY(pos));
	
	if(pos>175 || pos < 0)
		return 0;
		
	mapscr *m = &TheMaps[(map*MAPSCRS)+screen];
	
	if(m->data.empty()) return 0;
    
	if(m->valid==0) return 0;
	
	int flags = 0;
	
	if(secrets && game->maps[(map*MAPSCRSNORMAL)+screen])
	{
		flags = game->maps[(map*MAPSCRSNORMAL)+screen];
		//secrets = false;
	}
	
	int mapid = (layer < 0 ? -1 : ((m->layermap[layer] - 1) * MAPSCRS + m->layerscreen[layer]));
	
	if (layer >= 0 && (mapid < 0 || mapid > MAXMAPS2*MAPSCRS)) return 0;
	
	mapscr scr = ((mapid < 0 || mapid > MAXMAPS2*MAPSCRS) ? *m : TheMaps[mapid]);
	
	if(scr.data.empty()) return 0;
    
	if(scr.valid==0) return 0;
	
	if ((flags & mSECRET) && canPermSecret(currdmap, screen))
	{
		hiddenstair2(&scr, false);
		hidden_entrance2(&scr, (mapscr*)NULL, false, -3);
	}
	if(flags&mLOCKBLOCK)              // if special stuff done before
	{
	    remove_screenstatecombos2(&scr, (mapscr*)NULL, cLOCKBLOCK, cLOCKBLOCK2);
	}

	if(flags&mBOSSLOCKBLOCK)          // if special stuff done before
	{
	    remove_screenstatecombos2(&scr, (mapscr*)NULL, cBOSSLOCKBLOCK, cBOSSLOCKBLOCK2);
	}

	if(flags&mCHEST)              // if special stuff done before
	{
	    remove_screenstatecombos2(&scr, (mapscr*)NULL, cCHEST, cCHEST2);
	}

	if(flags&mCHEST)              // if special stuff done before
	{
	    remove_screenstatecombos2(&scr, (mapscr*)NULL, cLOCKEDCHEST, cLOCKEDCHEST2);
	}

	if(flags&mBOSSCHEST)              // if special stuff done before
	{
	    remove_screenstatecombos2(&scr, (mapscr*)NULL, cBOSSCHEST, cBOSSCHEST2);
	}
	
	return scr.data[pos];						// entire combo code
}

int MAPCSET2(int layer,int x,int y)
{
	if(layer==-1) return MAPCSET(x,y);
	
	if(tmpscr2[layer].cset.empty()) return 0;
	
	if(tmpscr2[layer].valid==0) return 0;
	
	int combo = COMBOPOS(x,y);
	
	if(combo>175 || combo < 0)
		return 0;
		
	return tmpscr2[layer].cset[combo];						// entire combo code
}

int MAPFLAG2(int layer,int x,int y)
{
    if(layer==-1) return MAPFLAG(x,y);
    
    if(tmpscr2[layer].sflag.empty()) return 0;
    
    if(tmpscr2[layer].valid==0) return 0;
    
    int combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return tmpscr2[layer].sflag[combo];                       // flag
}

int COMBOTYPE2(int layer,int x,int y)
{
    if(layer < 1)
    {
	for (int i = layer+1; i <= 1; ++i)
	{
		if(tmpscr2[i].valid!=0)
		{
			if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, &(tmpscr2[i]))) return cNONE;
		}
	}
    }
    if(layer==-1) return COMBOTYPE(x,y);
    if(tmpscr2[layer].valid==0)
    {
        return 0;
    }
    
    return combobuf[MAPCOMBO2(layer,x,y)].type;
}

int MAPCOMBOFLAG2(int layer,int x,int y)
{
    if(layer==-1) return MAPCOMBOFLAG(x,y);
    
    if(tmpscr2[layer].data.empty()) return 0;
    
    if(tmpscr2[layer].valid==0) return 0;
    
    int combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return combobuf[tmpscr2[layer].data[combo]].flag;                        // entire combo code
}

void setmapflag(int flag)
{
    setmapflag((currmap*MAPSCRSNORMAL)+homescr,flag);
}

const char *screenstate_string[15] =
{
    "Door Up", "Door Down", "Door Left", "Door Right", "Item", "Special Item", "No Return",
    "Temporary No Return", "Lock Blocks", "Boss Lock Blocks", "Treasure Chests", "Locked Chests",
    "Boss Locked Chests", "Secrets", "Visited"
};

void eventlog_mapflag_line(word* g, word flag, int ss_s_index)
{
    if((*g)&flag)
    {
        (*g)&=~flag;
        Z_eventlog("%s%s", screenstate_string[ss_s_index], (*g)!=0 ? ", " : "");
    }
}

void eventlog_mapflags()
{
    word g = game->maps[(currmap*MAPSCRSNORMAL)+homescr] &0x3FFF;
    word g2 = g;
    Z_eventlog("Screen (%d, %02x) %s", currmap+1, homescr, (g2) != 0 ? "[":"");
    // Print them in order of importance.
    eventlog_mapflag_line(&g, mSECRET,13);
    eventlog_mapflag_line(&g, mITEM,4);
    eventlog_mapflag_line(&g, mBELOW,5);
    eventlog_mapflag_line(&g, mLOCKBLOCK,8);
    eventlog_mapflag_line(&g, mBOSSLOCKBLOCK,9);
    eventlog_mapflag_line(&g, mCHEST,10);
    eventlog_mapflag_line(&g, mLOCKEDCHEST,11);
    eventlog_mapflag_line(&g, mBOSSCHEST,12);
    eventlog_mapflag_line(&g, mDOOR_UP,0);
    eventlog_mapflag_line(&g, mDOOR_DOWN,1);
    eventlog_mapflag_line(&g, mDOOR_LEFT,2);
    eventlog_mapflag_line(&g, mDOOR_RIGHT,3);
    eventlog_mapflag_line(&g, mNEVERRET,6);
    eventlog_mapflag_line(&g, mTMPNORET,7);
    Z_eventlog("%s\n",g2 != 0 ? "]":"");
}

// set specific flag
void setmapflag(int mi2, int flag)
{
    byte cscr = mi2&((1<<7)-1);
    byte cmap = (mi2>>7);
    char buf[20];
    sprintf(buf,"Screen (%d, %02X)",cmap+1,cscr);
    
    game->maps[mi2] |= flag;
    Z_eventlog("%s's State was set: %s\n",
               mi2 != (currmap*MAPSCRSNORMAL)+homescr ? buf : "Current screen",
               flag>0 ? screenstate_string[(int)log2((float)flag)] : "<Unknown>");
               
    if(flag==mSECRET||flag==mITEM||flag==mBELOW||flag==mLOCKBLOCK||
            flag==mBOSSLOCKBLOCK||flag==mCHEST||flag==mBOSSCHEST||flag==mLOCKEDCHEST)
    {
        byte nmap=TheMaps[((cmap)*MAPSCRS)+cscr].nextmap;
        byte nscr=TheMaps[((cmap)*MAPSCRS)+cscr].nextscr;
        
        std::vector<int> done;
        bool looped = (nmap==cmap+1 && nscr==cscr);
        
        while((nmap!=0) && !looped && !(nscr>=128))
        {
            if((tmpscr->nocarry&flag)!=flag)
            {
                Z_eventlog("State change carried over to (%d, %02X)\n",nmap,nscr);
                game->maps[((nmap-1)<<7)+nscr] |= flag;
            }
            
            cmap=nmap;
            cscr=nscr;
            nmap=TheMaps[((cmap-1)*MAPSCRS)+cscr].nextmap;
            nscr=TheMaps[((cmap-1)*MAPSCRS)+cscr].nextscr;
            
            for(std::vector<int>::iterator it = done.begin(); it != done.end(); it++)
            {
                if(*it == ((nmap-1)<<7)+nscr)
                    looped = true;
            }
            
            done.push_back(((nmap-1)<<7)+nscr);
        }
    }
}

void unsetmapflag(int flag)
{
    unsetmapflag((currmap*MAPSCRSNORMAL)+homescr,flag);
}

void unsetmapflag(int mi2, int flag, bool anyflag)
{
    if(anyflag)
        game->maps[mi2] &= ~flag;
    else if(flag==mITEM || flag==mBELOW)
    {
        if(!(tmpscr->flags4&fNOITEMRESET))
            game->maps[mi2] &= ~flag;
    }
    else game->maps[(currmap*MAPSCRSNORMAL)+homescr] &= ~flag;
    
    byte cscr = mi2&((1<<7)-1);
    byte cmap = (mi2>>7);
    char buf[20];
    sprintf(buf,"Screen (%d, %02X)",cmap+1,cscr);
    
    Z_eventlog("%s's State was unset: %s\n",
               mi2 != (currmap*MAPSCRSNORMAL)+homescr ? buf : "Current screen",
               flag>0 ? screenstate_string[(int)log2((float)flag)] : "<Unknown>");
               
    if(flag==mSECRET||flag==mITEM||flag==mBELOW||flag==mLOCKBLOCK||
            flag==mBOSSLOCKBLOCK||flag==mCHEST||flag==mBOSSCHEST||flag==mLOCKEDCHEST)
    {
        byte nmap=TheMaps[((cmap)*MAPSCRS)+cscr].nextmap;
        byte nscr=TheMaps[((cmap)*MAPSCRS)+cscr].nextscr;
        
        std::vector<int> done;
        bool looped = (nmap==cmap+1 && nscr==cscr);
        
        while((nmap!=0) && !looped && !(nscr>=128))
        {
            if((tmpscr->nocarry&flag)!=flag)
            {
                Z_eventlog("State change carried over to (%d, %02X)\n",nmap,nscr);
                game->maps[((nmap-1)<<7)+nscr] &= ~flag;
            }
            
            cmap=nmap;
            cscr=nscr;
            nmap=TheMaps[((cmap-1)*MAPSCRS)+cscr].nextmap;
            nscr=TheMaps[((cmap-1)*MAPSCRS)+cscr].nextscr;
            
            for(std::vector<int>::iterator it = done.begin(); it != done.end(); it++)
            {
                if(*it == ((nmap-1)<<7)+nscr)
                    looped = true;
            }
            
            done.push_back(((nmap-1)<<7)+nscr);
        }
    }
}

bool getmapflag(int flag)
{
    return (game->maps[(currmap*MAPSCRSNORMAL)+homescr] & flag) != 0;
}

int WARPCODE(int dmap,int scr,int dw)
// returns: -1 = not a warp screen
//          0+ = warp screen code ( high byte=dmap, low byte=scr )
{
    mapscr *s = &TheMaps[DMaps[dmap].map*MAPSCRS+scr];
    
    if(s->room!=rWARP)
        return -1;
        
    int ring=s->catchall;
    int size=QMisc.warp[ring].size;
    
    if(size==0)
        return -2;
        
    int index=-1;
    
    for(int i=0; i<size; i++)
        if(dmap==QMisc.warp[ring].dmap[i] && scr==
                (QMisc.warp[ring].scr[i] + DMaps[dmap].xoff))
            index=i;
            
    if(index==-1)
        return -3;
        
    index = (index+dw)%size;
    return (QMisc.warp[ring].dmap[index] << 8) + QMisc.warp[ring].scr[index];
}

void update_combo_cycling()
{
    int x,y;
    y = 0;
    static int newdata[176];
    static int newcset[176];
    static int newdata2[176];
    static int newcset2[176];
    static bool restartanim[MAXCOMBOS];
    static bool restartanim2[MAXCOMBOS];
    static bool initialized=false;
    
    // Just a simple bit of optimization
    if(!initialized)
    {
        for(int i=0; i<176; i++)
        {
            newdata[i]=-1;
            newcset[i]=-1;
            newdata2[i]=-1;
            newcset2[i]=-1;
        }
        
        memset(restartanim, 0, MAXCOMBOS);
        memset(restartanim2, 0, MAXCOMBOS);
        initialized=true;
    }
    
    for(int i=0; i<176; i++)
    {
        x=tmpscr->data[i];
        //y=animated_combo_table[x][0];
        
        if(combobuf[x].animflags & AF_FRESH) continue;
        
        //time to restart
        if((combobuf[x].aclk>=combobuf[x].speed) &&
                (combobuf[x].tile-combobuf[x].frames>=combobuf[x].o_tile-1) &&
                (combobuf[x].nextcombo!=0))
        {
            newdata[i]=combobuf[x].nextcombo;
            newcset[i]=combobuf[x].nextcset;
            int c=newdata[i];
            
            if(combobuf[c].animflags & AF_CYCLE)
            {
                restartanim[c]=true;
            }
        }
    }
    
    for(int i=0; i<176; i++)
    {
        x=tmpscr->data[i];
        //y=animated_combo_table2[x][0];
        
        if(!(combobuf[x].animflags & AF_FRESH)) continue;
        
        //time to restart
        if((combobuf[x].aclk>=combobuf[x].speed) &&
                (combobuf[x].tile-combobuf[x].frames>=combobuf[x].o_tile-1) &&
                (combobuf[x].nextcombo!=0))
        {
            newdata[i]=combobuf[x].nextcombo;
            newcset[i]=combobuf[x].nextcset;
            int c=newdata[i];
            
            if(combobuf[c].animflags & AF_CYCLE)
            {
                restartanim2[c]=true;
            }
        }
    }
    
    for(int i=0; i<176; i++)
    {
        if(newdata[i]==-1)
            continue;
            
        screen_combo_modify_preroutine(tmpscr,i);
        tmpscr->data[i]=newdata[i];
        tmpscr->cset[i]=newcset[i];
        screen_combo_modify_postroutine(tmpscr,i);
        
        newdata[i]=-1;
        newcset[i]=-1;
    }
    
    for(int i=0; i<32; i++)
    {
        x=tmpscr->ffdata[i];
        //y=animated_combo_table[x][0];
        
        if(combobuf[x].animflags & AF_FRESH) continue;
        
        //time to restart
        if((combobuf[x].aclk>=combobuf[x].speed) &&
                (combobuf[x].tile-combobuf[x].frames>=combobuf[x].o_tile-1) &&
                (combobuf[x].nextcombo!=0))
        {
            newdata[i]=combobuf[x].nextcombo;
            newcset[i]=combobuf[x].nextcset;
            int c=newdata[i];
            
            if(combobuf[c].animflags & AF_CYCLE)
            {
                restartanim[c]=true;
            }
        }
    }
    
    for(int i=0; i<32; i++)
    {
        x=tmpscr->ffdata[i];
        //y=animated_combo_table2[x][0];
        
        if(!(combobuf[x].animflags & AF_FRESH)) continue;
        
        //time to restart
        if((combobuf[x].aclk>=combobuf[x].speed) &&
                (combobuf[x].tile-combobuf[x].frames>=combobuf[x].o_tile-1) &&
                (combobuf[x].nextcombo!=0))
        {
            newdata[i]=combobuf[x].nextcombo;
            newcset[i]=combobuf[x].nextcset;
            int c=newdata[i];
            
            if(combobuf[c].animflags & AF_CYCLE)
            {
                restartanim2[c]=true;
            }
        }
    }
    
    for(int i=0; i<32; i++)
    {
        if(newdata[i]==-1)
            continue;
            
        tmpscr->ffdata[i]=newdata[i];
        tmpscr->ffcset[i]=newcset[i];
        
        newdata[i]=-1;
        newcset[i]=-1;
    }
    
    if(get_bit(quest_rules,qr_CMBCYCLELAYERS))
    {
        for(int j=0; j<6; j++)
        {
            if(tmpscr2[j].data.empty()) continue;
            
            for(int i=0; i<176; i++)
            {
                x=(tmpscr2+j)->data[i];
               // y=animated_combo_table[x][0];
                
                if(combobuf[x].animflags & AF_FRESH) continue;
                
                //time to restart
                if((combobuf[x].aclk>=combobuf[x].speed) &&
                        (combobuf[x].tile-combobuf[x].frames>=combobuf[x].o_tile-1) &&
                        (combobuf[x].nextcombo!=0))
                {
                    newdata[i]=combobuf[x].nextcombo;
                    newcset[i]=combobuf[x].nextcset;
                    int c=newdata[i];
                    
                    if(combobuf[c].animflags & AF_CYCLE)
                    {
                        restartanim[c]=true;
                    }
                }
            }
            
            for(int i=0; i<176; i++)
            {
                x=(tmpscr2+j)->data[i];
                //y=animated_combo_table2[x][0];
                
                if(!(combobuf[x].animflags & AF_FRESH)) continue;
                
                //time to restart
                if((combobuf[x].aclk>=combobuf[x].speed) &&
                        (combobuf[x].tile-combobuf[x].frames>=combobuf[x].o_tile-1) &&
                        (combobuf[x].nextcombo!=0))
                {
                    newdata2[i]=combobuf[x].nextcombo;
                    newcset2[i]=combobuf[x].nextcset;
                    int c=newdata2[i];
                    int cs=newcset2[i];
                    
                    if(combobuf[c].animflags & AF_CYCLE)
                    {
                        restartanim2[c]=true;
                    }
                    
                    if(combobuf[c].type==cSPINTILE1)
                    {
                        // Uses animated_combo_table2
                        addenemy((i&15)<<4,i&0xF0,(cs<<12)+eSPINTILE1,combobuf[c].o_tile+zc_max(1,combobuf[c].frames));
                    }
                }
            }
            
            for(int i=0; i<176; i++)
            {
                if(newdata[i]!=-1)
                {
                    screen_combo_modify_preroutine(tmpscr2+j,i);
                    (tmpscr2+j)->data[i]=newdata[i];
                    (tmpscr2+j)->cset[i]=newcset[i];
                    screen_combo_modify_postroutine(tmpscr2+j,i);
                    
                    newdata[i]=-1;
                    newcset[i]=-1;
                }
                
                if(newdata2[i]!=-1)
                {
                    (tmpscr2+j)->data[i]=newdata2[i];
                    (tmpscr2+j)->cset[i]=newcset2[i];
                    newdata2[i]=-1;
                    newcset2[i]=-1;
                }
            }
        }
    }
    
    for(int i=0; i<MAXCOMBOS; i++)
    {
        if(restartanim[i])
        {
            combobuf[i].tile = combobuf[i].o_tile;
			combobuf[i].aclk = 0;
            restartanim[i]=false;
        }
        
        if(restartanim2[i])
        {
            combobuf[i].tile = combobuf[i].o_tile;
			combobuf[i].aclk = 0;
            restartanim2[i]=false;
        }
    }
}

bool iswater_type(int type)
{
//  return type==cOLD_WATER || type==cSWIMWARP || type==cDIVEWARP || type==cDIVEWARPB || type==cDIVEWARPC || type==cDIVEWARPD || type==cSWIMWARPB || type==cSWIMWARPC || type==cSWIMWARPD;
    return (combo_class_buf[type].water!=0);
}

bool iswater(int combo)
{
    return iswater_type(combobuf[combo].type) && !DRIEDLAKE;
}
int iswaterexzq(int combo, int map, int screen, int layer, int x, int y, bool secrets, bool fullcheck, bool LayerCheck)
{
	return iswaterex(combo, map, screen, layer, x, y, secrets, fullcheck, LayerCheck);
}
int iswaterex(int combo, int map, int screen, int layer, int x, int y, bool secrets, bool fullcheck, bool LayerCheck, bool ShallowCheck)
{
	//Honestly, fullcheck is kinda useless... I made this function back when I thought it was checking the entire combo and not just a glorified x/y value.
	//Fullcheck makes no sense to ever be on, but hey I guess it's here in case you ever need it... 
	
	//Oh hey, Zoras might actually need it. Nevermind, this had a use!
	if (get_bit(quest_rules, qr_SMARTER_WATER))
	{
		if (LayerCheck && (get_bit(quest_rules,  qr_WATER_ON_LAYER_1) || get_bit(quest_rules,  qr_WATER_ON_LAYER_2))) //LayerCheck is a bit dumber, but it lets me add this QR without having to replace all calls, again.
		{
			for (int m = layer; m <= 1; m++)
			{
				if (m < 0 || m == 0 && get_bit(quest_rules,  qr_WATER_ON_LAYER_1)
				|| m == 1 && get_bit(quest_rules,  qr_WATER_ON_LAYER_2))
				{
					int checkwater = iswaterex(combo, map, screen, m, x, y, secrets, fullcheck, false, ShallowCheck);
					if (checkwater > 0) 
					{
						return checkwater;
					}
				}
			}
			return 0;
		}
		else
		{
			for(int i=(fullcheck?3:0); i>=0; i--)
			{
				int tx2=((i&2)<<2)+x;
				int ty2=((i&1)<<3)+y;
				int b = i; //Originally b was not needed and I read off i, but then I added the boolean for fullcheck.
				//In which case it's just easier to change b if fullcheck is false instead of changing i and potentially screwing up the for loop.
				if (!fullcheck)
				{
					tx2 = x;
					ty2 = y;
					if(tx2&8) b+=2;
					if(ty2&8) b+=1;
				}
				bool bridgedetected = false;
				for (int m = layer; m <= 1; m++)
				{
					newcombo const& cmb = combobuf[MAPCOMBO3(map, screen, m,tx2,ty2, true)];
					if (cmb.type == cBRIDGE && !(cmb.walk&(1<<b))) 
					{
						bridgedetected = true;
					}
					if (get_bit(quest_rules, qr_NO_SOLID_SWIM))
					{
						if (cmb.type != cBRIDGE && (cmb.walk&(1<<b)))
						{
							bridgedetected = true;
						}						
					}
					if (iswater_type(cmb.type) && (cmb.walk&(1<<b)) && ((cmb.usrflags&cflag3) || (cmb.usrflags&cflag4) || current_item(itype_flippers) < cmb.attribytes[0] || ((cmb.usrflags&cflag1) && !(itemsbuf[current_item_id(itype_flippers)].flags & ITEM_FLAG3))))
					{
						if (!(ShallowCheck && (cmb.walk&(1<<b)) && (cmb.usrflags&cflag4))) bridgedetected = true;
					}
				}
				for(int k=0; k<32; k++)
				{
					if(ffcIsAt(k, tx2, ty2) && !combo_class_buf[FFCOMBOTYPE(tx2,ty2)].water && !(ShallowCheck && FFCOMBOTYPE(tx2,ty2) == cSHALLOWWATER))
						bridgedetected = true;
				}
				if (bridgedetected)
				{
					return 0;
				}
				if (!DRIEDLAKE)
				{
					for(int k=0; k<32; k++)
					{
						if(combo_class_buf[FFCOMBOTYPE(tx2,ty2)].water || (ShallowCheck && FFCOMBOTYPE(tx2,ty2) == cSHALLOWWATER))
						{
							if (i == 0) return MAPFFCOMBO(tx2,ty2);
							else continue;
						}
					}
					int checkcombo = MAPCOMBO3(map, screen, layer, tx2, ty2, secrets);
					if (iswater_type(combobuf[checkcombo].type)||(ShallowCheck && (combobuf[checkcombo].type == cSHALLOWWATER || (iswater_type(combobuf[checkcombo].type) && (combobuf[checkcombo].walk&(1<<b)) && (combobuf[checkcombo].usrflags&cflag4))))) 
					{
						if (i == 0) return checkcombo;
						else continue;
					}
				}
				return 0;
			}
			return 0;
		}
	}
	else
	{
		if (get_bit(quest_rules, qr_NO_SOLID_SWIM))
		{
			int b = 0;
			if(x&8) b+=2;
			if(y&8) b+=1;
			if (combobuf[combo].walk&(1<<b))
			{
				return 0;
			}						
		}
		return (((iswater_type(combobuf[combo].type) || (ShallowCheck && combobuf[combo].type == cSHALLOWWATER)) && !DRIEDLAKE)?combo:0); //These used to return booleans; returning the combo id of the water combo it caught is essential for Rob's proposed water changes.
	}
}

bool ispitfall_type(int type)
{
	return combo_class_buf[type].pit != 0;
}

bool ispitfall(int combo)
{
    return ispitfall_type(combobuf[combo].type);
}

bool ispitfall(int x, int y)
{
	/*
	if(int c = MAPFFCOMBO(x,y))
		return ispitfall(c);
	return ispitfall(MAPCOMBO(x,y)) || ispitfall(MAPCOMBOL(1,x,y)) || ispitfall(MAPCOMBOL(2,x,y));
	*/
	if(int c = MAPFFCOMBO(x,y))
	{
		return ispitfall(c) ? true : false;
	}
	int c = MAPCOMBOL(2,x,y);
	if(ispitfall(c)) return true;
	if(tmpscr2[1].valid!=0)
	{
		if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, &(tmpscr2[1]))) return false;
	}
	c = MAPCOMBOL(1,x,y);
	if(ispitfall(c)) return true;
	if(tmpscr2[0].valid!=0)
	{
		if (combobuf[MAPCOMBO2(0,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, &(tmpscr2[0]))) return false;
	}
	c = MAPCOMBO(x,y);
	if(ispitfall(c)) return true;
	return false;
}

int getpitfall(int x, int y) //Return the highest-layer active pit combo at the given position
{
	if(int c = MAPFFCOMBO(x,y))
	{
		return ispitfall(c) ? c : 0;
	}
	int c = MAPCOMBOL(2,x,y);
	if(ispitfall(c)) return c;
	if(tmpscr2[1].valid!=0)
	{
		if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, &(tmpscr2[1]))) return 0;
	}
	c = MAPCOMBOL(1,x,y);
	if(ispitfall(c)) return c;
	if(tmpscr2[0].valid!=0)
	{
		if (combobuf[MAPCOMBO2(0,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, &(tmpscr2[0]))) return 0;
	}
	c = MAPCOMBO(x,y);
	if(ispitfall(c)) return c;
	return 0;
}

bool isSVLadder(int x, int y)
{
	if(x<0 || x>255 || y<0 || y>175)
        return false;
	
    mapscr *s1, *s2;
    s1=(((*tmpscr).layermap[0]-1)>=0)?tmpscr2:tmpscr;
    s2=(((*tmpscr).layermap[1]-1)>=0)?tmpscr2+1:tmpscr;
	
    int combo = COMBOPOS(x,y);
    return (tmpscr->sflag[combo] == mfSIDEVIEWLADDER) || (combobuf[tmpscr->data[combo]].flag == mfSIDEVIEWLADDER) ||
		(s1->sflag[combo] == mfSIDEVIEWLADDER) || (combobuf[s1->data[combo]].flag == mfSIDEVIEWLADDER) ||
		(s2->sflag[combo] == mfSIDEVIEWLADDER) || (combobuf[s2->data[combo]].flag == mfSIDEVIEWLADDER);
}

bool isSVPlatform(int x, int y)
{
	if(x<0 || x>255 || y<0 || y>175)
        return false;
	
    mapscr *s1, *s2;
    s1=(((*tmpscr).layermap[0]-1)>=0)?tmpscr2:tmpscr;
    s2=(((*tmpscr).layermap[1]-1)>=0)?tmpscr2+1:tmpscr;
	
    int combo = COMBOPOS(x,y);
    return (tmpscr->sflag[combo] == mfSIDEVIEWPLATFORM) || (combobuf[tmpscr->data[combo]].flag == mfSIDEVIEWPLATFORM) ||
		(s1->sflag[combo] == mfSIDEVIEWPLATFORM) || (combobuf[s1->data[combo]].flag == mfSIDEVIEWPLATFORM) ||
		(s2->sflag[combo] == mfSIDEVIEWPLATFORM) || (combobuf[s2->data[combo]].flag == mfSIDEVIEWPLATFORM);
}

bool checkSVLadderPlatform(int x, int y)
{
	return isSVPlatform(x,y) || (isSVLadder(x,y) && !isSVLadder(x,y-16));
}

bool isstepable(int combo)                                  //can use ladder on it
{
    if(combo_class_buf[combobuf[combo].type].ladder_pass) return true;
	if(combo_class_buf[combobuf[combo].type].pit)
	{
		if(combobuf[combo].usrflags&cflag4)
		{
			int ldrid = current_item_id(itype_ladder);
			return (ldrid > -1 && itemsbuf[ldrid].flags & ITEM_FLAG1);
		}
	}
	return false;
}

bool ishookshottable(int bx, int by)
{
    if(!_walkflag(bx,by,1))
        return true;
        
    bool ret = true;
    
    for(int i=2; i>=0; i--)
    {
        int c = MAPCOMBO2(i-1,bx,by);
        int t = combobuf[c].type;
        
        if(i == 0 && (t == cHOOKSHOTONLY || t == cLADDERHOOKSHOT)) return true;
        
        bool dried = (iswater_type(t) && DRIEDLAKE);
        
        int b=1;
        
        if(bx&8) b<<=2;
        
        if(by&8) b<<=1;
        
        if(combobuf[c].walk&b && !dried && !(combo_class_buf[t].ladder_pass && t!=cLADDERONLY) && t!=cHOOKSHOTONLY)
            ret = false;
    }
    
    return ret;
}

bool ishookshottable(int map, int screen, int bx, int by)
{
	if (map < 0 || screen < 0) return false;
		
	mapscr *m = &TheMaps[(map*MAPSCRS)+screen];
	
	if(m->data.empty()) return false;
	
	if(m->valid==0) return false;
	
	if(!_walkflag(bx,by,1, m))
		return true;
		
	bool ret = true;
	
	for(int i=2; i>=0; i--)
	{
		int c = MAPCOMBO3(map, screen, i-1,bx,by);
		int t = combobuf[c].type;
		
		if(i == 0 && (t == cHOOKSHOTONLY || t == cLADDERHOOKSHOT)) return true;
		
		//bool dried = (iswater_type(t) && DRIEDLAKE);
		
		int b=1;
		
		if(bx&8) b<<=2;
		
		if(by&8) b<<=1;
		
		if(combobuf[c].walk&b && !(combo_class_buf[t].ladder_pass && t!=cLADDERONLY) && t!=cHOOKSHOTONLY)
			ret = false;
	}
	
	return ret;
}

bool hiddenstair(int tmp,bool redraw)                       // tmp = index of tmpscr[]
{
    return hiddenstair2(tmpscr + tmp,redraw);
}

bool hiddenstair2(mapscr *s,bool redraw)                       // tmp = index of tmpscr[]
{
    
    if((s->stairx || s->stairy) && s->secretcombo[sSTAIRS])
    {
        int di = COMBOPOS(s->stairx,s->stairy);
        s->data[di] = s->secretcombo[sSTAIRS];
        s->cset[di] = s->secretcset[sSTAIRS];
        s->sflag[di] = s->secretflag[sSTAIRS];
        
        if(redraw)
            putcombo(scrollbuf,s->stairx,s->stairy,s->data[di],s->cset[di]);
            
        return true;
    }
    
    return false;
}

bool remove_screenstatecombos(int tmp, int what1, int what2)
{
	mapscr *s = tmpscr + tmp;
	mapscr *t = tmpscr2;
	return remove_screenstatecombos2(s, t, what1, what2);
}

bool remove_screenstatecombos2(mapscr *s, mapscr *t, int what1, int what2)
{
    bool didit=false;
    
    for(int i=0; i<176; i++)
    {
        if((combobuf[s->data[i]].type== what1) ||
                (combobuf[s->data[i]].type== what2))
        {
            s->data[i]++;
            didit=true;
        }
    }
    
    if(t)
    {
	    for(int j=0; j<6; j++)
	    {
		if(t[j].data.empty()) continue;
		
		for(int i=0; i<176; i++)
		{
		    if((combobuf[t[j].data[i]].type== what1) ||
			    (combobuf[t[j].data[i]].type== what2))
		    {
			t[j].data[i]++;
			didit=true;
		    }
		}
	    }
    }
    
    return didit;
}

bool remove_lockblocks(int tmp)
{
    return remove_screenstatecombos(tmp, cLOCKBLOCK, cLOCKBLOCK2);
}

bool remove_bosslockblocks(int tmp)
{
    return remove_screenstatecombos(tmp, cBOSSLOCKBLOCK, cBOSSLOCKBLOCK2);
}

bool remove_chests(int tmp)                 // tmp = index of tmpscr[]
{
    return remove_screenstatecombos(tmp, cCHEST, cCHEST2);
}

bool remove_lockedchests(int tmp)                 // tmp = index of tmpscr[]
{
    return remove_screenstatecombos(tmp, cLOCKEDCHEST, cLOCKEDCHEST2);
}

bool remove_bosschests(int tmp)                 // tmp = index of tmpscr[]
{
    return remove_screenstatecombos(tmp, cBOSSCHEST, cBOSSCHEST2);
}


bool overheadcombos(mapscr *s)
{
    for(int i=0; i<176; i++)
    {
//    if (combobuf[s->data[i]].type==cOLD_OVERHEAD)
        if(combo_class_buf[combobuf[s->data[i]].type].overhead)
        {
            return true;
        }
    }
    
    return false;
}

void delete_fireball_shooter(mapscr *s, int i)
{
    int cx=0, cy=0;
    int ct=combobuf[s->data[i]].type;
    
    if(ct!=cL_STATUE && ct!=cR_STATUE && ct!=cC_STATUE)
        return;
        
    switch(ct)
    {
    case cL_STATUE:
        cx=((i&15)<<4)+4;
        cy=(i&0xF0)+7;
        break;
        
    case cR_STATUE:
        cx=((i&15)<<4)-8;
        cy=(i&0xF0)-1;
        break;
        
    case cC_STATUE:
        cx=((i&15)<<4);
        cy=(i&0xF0);
        break;
    }
    
    for(int j=0; j<guys.Count(); j++)
    {
        // Finds the smallest enemy ID
        if((int(guys.spr(j)->x)==cx)&&(int(guys.spr(j)->y)==cy)&&(guysbuf[(guys.spr(j)->id)&0xFFF].flags2 & eneflag_fire))
        {
            guys.del(j);
        }
    }
}

int findtrigger(int scombo, bool ff)
{
    int checkflag=0;
    int iter;
    int ret = 0;
    
    for(int j=0; j<(ff?32:176); j++)
    {
        if(ff)
        {
            checkflag=combobuf[tmpscr->ffdata[j]].flag;
            iter=1;
        }
        else iter=2;
        
        for(int layer=-1; !ff && layer<6; layer++)
        {
            if(layer>-1 && tmpscr2[layer].valid==0) continue;
            
            for(int i=0; i<iter; i++)
            {
                if(i==0&&!ff)
                    checkflag = (layer>-1 ? combobuf[tmpscr2[layer].data[j]].flag : combobuf[tmpscr->data[j]].flag);
                else if(i==1&&!ff)
                    checkflag = (layer>-1 ? tmpscr2[layer].sflag[j] : tmpscr->sflag[j]);
                    
                switch(checkflag)
                {
                case mfBCANDLE:
                case mfRCANDLE:
                case mfWANDFIRE:
                case mfDINSFIRE:
                case mfARROW:
                case mfSARROW:
                case mfGARROW:
                case mfSBOMB:
                case mfBOMB:
                case mfBRANG:
                case mfMBRANG:
                case mfFBRANG:
                case mfWANDMAGIC:
                case mfREFMAGIC:
                case mfREFFIREBALL:
                case mfSWORD:
                case mfWSWORD:
                case mfMSWORD:
                case mfXSWORD:
                case mfSWORDBEAM:
                case mfWSWORDBEAM:
                case mfMSWORDBEAM:
                case mfXSWORDBEAM:
                case mfHOOKSHOT:
                case mfWAND:
                case mfHAMMER:
                case mfSTRIKE:
                    if(scombo!=j)
                        ret += 1;
                        
                default:
                    break;
                }
            }
        }
    }
    
    return ret;
}

// single:
// >-1 : the singular triggering combo
// -1: triggered by some other cause
// -2: triggered by Enemies->Secret
// -3: triggered by Secrets screen state
// -4: Screen->TriggerSecrets()
void hidden_entrance(int tmp,bool refresh, bool high16only,int single) //Perhaps better known as 'Trigger Secrets'
{
	//There are no calls to 'hidden_entrance' in the code where tmp != 0
	Z_eventlog("%sScreen Secrets triggered%s.\n",
			   single>-1? "Restricted ":"",
			   single==-2? " by the 'Enemies->Secret' screen flag":
			   single==-3? " by the 'Secrets' screen state" :
			   single==-4? " by a script":"");
	hidden_entrance2(tmpscr + tmp, tmpscr2, high16only, single);
}
void hidden_entrance2(mapscr *s, mapscr *t, bool high16only,int single) //Perhaps better known as 'Trigger Secrets'
{
	/*
	mapscr *s = tmpscr + tmp;
	mapscr *t = tmpscr2;
	*/
	int ft=0; //Flag trigger?
	int msflag=0; // Misc. secret flag
	
	for(int i=0; i<176; i++) //Do the 'trigger flags' (non 16-31)
	{
		if(single>=0 && i!=single) continue; //If it's got a singular flag and i isn't where the flag is
		
		bool putit;
		
		// Remember the misc. secret flag; if triggered, use this instead
		if(s->sflag[i]>=mfSECRETS01 && s->sflag[i]<=mfSECRETS16)
			msflag=sSECRET01+(s->sflag[i]-mfSECRETS01);
		else if(combobuf[s->data[i]].flag>=mfSECRETS01 && combobuf[s->data[i]].flag<=mfSECRETS16)
			msflag=sSECRET01+(combobuf[s->data[i]].flag-mfSECRETS01);
		else
			msflag=0;
			
		if(!high16only || single>=0)
		{
			int newflag = -1;
			
			
			for(int iter=0; iter<2; ++iter)
			{
				putit=true;
				int checkflag=combobuf[s->data[i]].flag; //Inherent
				
				if(iter==1) checkflag=s->sflag[i]; //Placed
				
				switch(checkflag)
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
				
				if(putit)  //Change the combos for the secret
				{
					// Use misc. secret flag instead if one is present
					if(msflag!=0)
						ft=msflag;
						
					screen_combo_modify_preroutine(s,i);
					s->data[i] = s->secretcombo[ft];
					s->cset[i] = s->secretcset[ft];
					newflag = s->secretflag[ft];
					screen_combo_modify_postroutine(s,i);
				}
			}
			
			if(newflag >-1) s->sflag[i] = newflag; //Tiered secret
			
			if (t)
			{
				for(int j=0; j<6; j++)  //Layers
				{
					if(t[j].data.empty()||t[j].cset.empty()) continue; //If layer isn't used
					
					if(single>=0 && i!=single) continue; //If it's got a singular flag and i isn't where the flag is
					
					int newflag2 = -1;
					
					// Remember the misc. secret flag; if triggered, use this instead
					if(t[j].sflag[i]>=mfSECRETS01 && t[j].sflag[i]<=mfSECRETS16)
						msflag=sSECRET01+(t[j].sflag[i]-mfSECRETS01);
					else if(combobuf[t[j].data[i]].flag>=mfSECRETS01 && combobuf[t[j].data[i]].flag<=mfSECRETS16)
						msflag=sSECRET01+(combobuf[t[j].data[i]].flag-mfSECRETS01);
					else
						msflag=0;
						
					for(int iter=0; iter<2; ++iter)
					{
						putit=true;
						int checkflag=combobuf[t[j].data[i]].flag; //Inherent
						
						if(iter==1) checkflag=t[j].sflag[i];  //Placed
						
						switch(checkflag)
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
						
						if(putit)  //Change the combos for the secret
						{
						// Use misc. secret flag instead if one is present
						if(msflag!=0)
							ft=msflag;
							
						t[j].data[i] = t[j].secretcombo[ft];
						t[j].cset[i] = t[j].secretcset[ft];
						newflag2 = t[j].secretflag[ft];
						int c=t[j].data[i];
						int cs=t[j].cset[i];
						
						if(combobuf[c].type==cSPINTILE1)  //Surely this means we can have spin tiles on layers 3+? Isn't that bad? ~Joe123
							addenemy((i&15)<<4,i&0xF0,(cs<<12)+eSPINTILE1,combobuf[c].o_tile+zc_max(1,combobuf[c].frames));
						}
					}
					
					if(newflag2 >-1) t[j].sflag[i] = newflag2;  //Tiered secret
				}
			}
		}
	}
	
	for(int i=0; i<32; i++) //FFC 'trigger flags'
	{
		if(single>=0) if(i+176!=single) continue;
		
		bool putit;
		
		if((!high16only)||(single>=0))
		{
			//for (int iter=0; iter<1; ++iter) // Only one kind of FFC flag now.
			{
				putit=true;
				int checkflag=combobuf[s->ffdata[i]].flag; //Inherent
				
				//No placed flags yet
				switch(checkflag)
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
				
				if(putit)  //Change the ffc's combo
				{
					s->ffdata[i] = s->secretcombo[ft];
					s->ffcset[i] = s->secretcset[ft];
				}
			}
		}
	}
	
	if(checktrigger) //Hit all triggers->16-31
	{
		checktrigger=false;
		
		if(tmpscr->flags6&fTRIGGERF1631)
		{
			int tr = findtrigger(-1,false);  //Normal flags
			
			if(tr)
			{
				Z_eventlog("Hit All Triggers->16-31 not fulfilled (%d trigger flag%s remain).\n", tr, tr>1?"s":"");
				goto endhe;
			}
			
			int ftr = findtrigger(-1,true); //FFCs
			
			if(ftr)
			{
				Z_eventlog("Hit All Triggers->16-31 not fulfilled (%d trigger FFC%s remain).\n", ftr, ftr>1?"s":"");
				goto endhe;
			}
		}
	}
	
	for(int i=0; i<176; i++) // Do the 16-31 secrets
	{
		//If it's an enemies->secret screen, only do the high 16 if told to
		//That way you can have secret and burn/bomb entrance separately
		if((!(s->flags2&fCLEARSECRET) /*Enemies->Secret*/ && single < 0) || high16only || s->flags4&fENEMYSCRTPERM)
		{
			int newflag = -1;
			
			for(int iter=0; iter<2; ++iter)
			{
				int checkflag=combobuf[s->data[i]].flag; //Inherent
				
				if(iter==1) checkflag=s->sflag[i];  //Placed
				
				if((checkflag > 15)&&(checkflag < 32)) //If we've got a 16->32 flag change the combo
				{
					screen_combo_modify_preroutine(s,i);
					s->data[i] = s->secretcombo[checkflag-16+4];
					s->cset[i] = s->secretcset[checkflag-16+4];
					newflag = s->secretflag[checkflag-16+4];
					screen_combo_modify_postroutine(s,i);
				}
			}
			
			if(newflag >-1) s->sflag[i] = newflag;  //Tiered flag
			if (t)
			{
				for(int j=0; j<6; j++)  //Layers
				{
					if(t[j].data.empty()||t[j].cset.empty()) continue; //If layer is not valid (surely checking for 'valid' would be better?)
					
					int newflag2 = -1;
					
					for(int iter=0; iter<2; ++iter)
					{
						int checkflag=combobuf[t[j].data[i]].flag; //Inherent
						
						if(iter==1) checkflag=t[j].sflag[i];  //Placed
						
						if((checkflag > 15)&&(checkflag < 32)) //If we've got a 16->32 flag change the combo
						{
							t[j].data[i] = t[j].secretcombo[checkflag-16+4];
							t[j].cset[i] = t[j].secretcset[checkflag-16+4];
							newflag2 = t[j].secretflag[checkflag-16+4];
						}
					}
					
					if(newflag2 >-1) t[j].sflag[i] = newflag2;  //Tiered flag
				}
			}
		}
		
		/*
		  if(putit && refresh)
		  putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cset[i]);
		  */
	}
	
	for(int i=0; i<32; i++) // FFCs
	{
		if((!(s->flags2&fCLEARSECRET) /*Enemies->Secret*/ && single < 0) || high16only || s->flags4&fENEMYSCRTPERM)
		{
			for(int iter=0; iter<1; ++iter)  // Only one kind of FFC flag now.
			{
				int checkflag=combobuf[s->ffdata[i]].flag; //Inherent
				
				//No placed flags yet
				if((checkflag > 15)&&(checkflag < 32)) //If we find a flag, change the combo
				{
					s->ffdata[i] = s->secretcombo[checkflag-16+4];
					s->ffcset[i] = s->secretcset[checkflag-16+4];
				}
			}
		}
	}
	
endhe:

	if(tmpscr->flags4&fDISABLETIME) //Finish timed warp if 'Secrets Disable Timed Warp'
	{
		activated_timed_warp=true;
		tmpscr->timedwarptics = 0;
	}
}


bool findentrance(int x, int y, int flag, bool setflag)
{
    bool foundflag=false;
    bool foundcflag=false;
    bool foundnflag=false;
    bool foundfflag=false;
    //bool ffcombosingle = false;
    int ffcombos[4] = {-1, -1, -1, -1};
    bool single16=false;
    int scombo=-1;
    
    for(int i=-1; i<6; i++)  // Layers. -1 = screen.
    {
        if(MAPFLAG2(i,x,y)==flag || MAPFLAG2(i,x+15,y)==flag ||
                MAPFLAG2(i,x,y+15)==flag || MAPFLAG2(i,x+15,y+15)==flag)
        {
            foundflag=true;
            foundnflag=true;
        }
    }
    
    for(int i=-1; i<6; i++)  // Layers. -1 = screen.
    {
        if(MAPCOMBOFLAG2(i,x,y)==flag || MAPCOMBOFLAG2(i,x+15,y)==flag ||
                MAPCOMBOFLAG2(i,x,y+15)==flag || MAPCOMBOFLAG2(i,x+15,y+15)==flag)
        {
            foundflag=true;
            foundcflag=true;
        }
    }
    
    if(MAPFFCOMBOFLAG(x,y)==flag)
    {
        foundflag=true;
        foundfflag=true;
    }
    
    ffcombos[0] = current_ffcombo;
    
    if(MAPFFCOMBOFLAG(x+15,y)==flag)
    {
        foundflag=true;
        foundfflag=true;
    }
    
    ffcombos[1] = current_ffcombo;
    
    if(MAPFFCOMBOFLAG(x,y+15)==flag)
    {
        foundflag=true;
        foundfflag=true;
    }
    
    ffcombos[2] = current_ffcombo;
    
    if(MAPFFCOMBOFLAG(x+15,y+15)==flag)
    {
        foundflag=true;
        foundfflag=true;
    }
    
    ffcombos[3] = current_ffcombo;
    
    if(!foundflag)
    {
        return false;
    }
    
    for(int i=-1; i<6; i++)  // Look for Trigger->Self on all layers
    {
        if(foundnflag) // Trigger->Self (a.k.a Singular) is inherent
        {
            if((MAPCOMBOFLAG2(i,x,y)==mfSINGLE)&&(MAPFLAG2(i,x,y)==flag))
            {
                scombo=COMBOPOS(x,y);
            }
            else if((MAPCOMBOFLAG2(i,x,y)==mfSINGLE16)&&(MAPFLAG2(i,x,y)==flag))
            {
                scombo=COMBOPOS(x,y);
                single16=true;
            }
            else if((MAPCOMBOFLAG2(i,x+15,y)==mfSINGLE)&&(MAPFLAG2(i,x+15,y)==flag))
            {
                scombo=COMBOPOS(x+15,y);
            }
            else if((MAPCOMBOFLAG2(i,x+15,y)==mfSINGLE16)&&(MAPFLAG2(i,x+15,y)==flag))
            {
                scombo=COMBOPOS(x+15,y);
                single16=true;
            }
            else if((MAPCOMBOFLAG2(i,x,y+15)==mfSINGLE)&&(MAPFLAG2(i,x,y+15)==flag))
            {
                scombo=COMBOPOS(x,y+15);
            }
            else if((MAPCOMBOFLAG2(i,x,y+15)==mfSINGLE16)&&(MAPFLAG2(i,x,y+15)==flag))
            {
                scombo=COMBOPOS(x,y+15);
                single16=true;
            }
            else if((MAPCOMBOFLAG2(i,x+15,y+15)==mfSINGLE)&&(MAPFLAG2(i,x+15,y+15)==flag))
            {
                scombo=COMBOPOS(x+15,y+15);
            }
            else if((MAPCOMBOFLAG2(i,x+15,y+15)==mfSINGLE16)&&(MAPFLAG2(i,x+15,y+15)==flag))
            {
                scombo=COMBOPOS(x+15,y+15);
                single16=true;
            }
        }
        
        if(foundcflag) // Trigger->Self (a.k.a Singular) is non-inherent
        {
            if((MAPFLAG2(i,x,y)==mfSINGLE)&&(MAPCOMBOFLAG2(i,x,y)==flag))
            {
                scombo=COMBOPOS(x,y);
            }
            else if((MAPFLAG2(i,x,y)==mfSINGLE16)&&(MAPCOMBOFLAG2(i,x,y)==flag))
            {
                scombo=COMBOPOS(x,y);
                single16=true;
            }
            else if((MAPFLAG2(i,x+15,y)==mfSINGLE)&&(MAPCOMBOFLAG2(i,x+15,y)==flag))
            {
                scombo=COMBOPOS(x+15,y);
            }
            else if((MAPFLAG2(i,x+15,y)==mfSINGLE16)&&(MAPCOMBOFLAG2(i,x+15,y)==flag))
            {
                scombo=COMBOPOS(x+15,y);
                single16=true;
            }
            else if((MAPFLAG2(i,x,y+15)==mfSINGLE)&&(MAPCOMBOFLAG2(i,x,y+15)==flag))
            {
                scombo=COMBOPOS(x,y+15);
            }
            else if((MAPFLAG2(i,x,y+15)==mfSINGLE16)&&(MAPCOMBOFLAG2(i,x,y+15)==flag))
            {
                scombo=COMBOPOS(x,y+15);
                single16=true;
            }
            else if((MAPFLAG2(i,x+15,y+15)==mfSINGLE)&&(MAPCOMBOFLAG2(i,x+15,y+15)==flag))
            {
                scombo=COMBOPOS(x+15,y+15);
            }
            else if((MAPFLAG2(i,x+15,y+15)==mfSINGLE16)&&(MAPCOMBOFLAG2(i,x+15,y+15)==flag))
            {
                scombo=COMBOPOS(x+15,y+15);
                single16=true;
            }
        }
    }
    
    if(scombo<0)
    {
        checktrigger=true;
        hidden_entrance(0,true);
    }
    else
    {
        checktrigger=true;
        hidden_entrance(0,true,single16,scombo);
    }
    
    sfx(tmpscr->secretsfx);
    
    if(tmpscr->flags6&fTRIGGERFPERM)
    {
        int tr = findtrigger(-1,false);  //Normal flags
        
        if(tr)
        {
            Z_eventlog("Hit All Triggers->Perm Secret not fulfilled (%d trigger flag%s remain).\n", tr, tr>1?"s":"");
            setflag=false;
        }
        
        int ftr = findtrigger(-1,true); //FFCs
        
        if(ftr)
        {
            Z_eventlog("Hit All Triggers->Perm Secret not fulfilled (%d trigger FFC%s remain).\n", ftr, ftr>1?"s":"");
            setflag=false;
        }
    }
    
    if(setflag && canPermSecret())
        if(!(tmpscr->flags5&fTEMPSECRETS))
            setmapflag(mSECRET);
            
    return true;
}

void update_freeform_combos()
{
    ffscript_engine(false);
    
    for(int i=0; i<32; i++)
    {
        // Combo 0?
        if(tmpscr->ffdata[i]==0)
            continue;
            
        // Changer?
        if(tmpscr->ffflags[i]&ffCHANGER)
            continue;
            
        // Stationary?
        if(tmpscr->ffflags[i]&ffSTATIONARY)
            continue;
            
        // Frozen because Link's holding up an item?
        if(Link.getHoldClk()>0 && (tmpscr->ffflags[i]&ffIGNOREHOLDUP)==0)
            continue;
            
        // Check for changers
        if(tmpscr->fflink[i]==0)
        {
            for(int j=0; j<32; j++)
            {
                // Combo 0?
                if(tmpscr->ffdata[j]==0)
                    continue;
                    
                // Not a changer?
                if(!(tmpscr->ffflags[j]&ffCHANGER))
                    continue;
                    
                // Ignore this changer? (ffposx and ffposy are last changer position)
                if((tmpscr->ffx[j]/10000==ffposx[i]&&tmpscr->ffy[j]/10000==ffposy[i]) || tmpscr->ffflags[i]&ffIGNORECHANGER)
                    continue;
                    
                if((isonline(tmpscr->ffx[i], tmpscr->ffy[i], ffprvx[i],ffprvy[i], tmpscr->ffx[j], tmpscr->ffy[j]) || // Along the line, or...
                        //(tmpscr->ffx[i]==tmpscr->ffx[j] && tmpscr->ffy[i]==tmpscr->ffy[j])) && // At exactly the same position, and...
			( // At exactly the same position, 
				(tmpscr->ffx[i]==tmpscr->ffx[j] && tmpscr->ffy[i]==tmpscr->ffy[j])) 
				||
				//or imprecision and close enough
				( (tmpscr->ffflags[i]&ffIMPRECISIONCHANGER) && ((abs(tmpscr->ffx[i] - tmpscr->ffx[j]) < 10000) && abs(tmpscr->ffy[i] - tmpscr->ffy[j]) < 10000) )
			)
		&& //and...
                        (ffprvx[i]>-10000000 && ffprvy[i]>-10000000)) // This isn't the first frame on this screen
                {
                    if(tmpscr->ffflags[j]&ffCHANGETHIS)
                    {
                        tmpscr->ffdata[i] = tmpscr->ffdata[j];
                        tmpscr->ffcset[i] = tmpscr->ffcset[j];
                    }
                    
                    if(tmpscr->ffflags[j]&ffCHANGENEXT)
                        tmpscr->ffdata[i]++;
                        
                    if(tmpscr->ffflags[j]&ffCHANGEPREV)
                        tmpscr->ffdata[i]--;
                        
                    tmpscr->ffdelay[i]=tmpscr->ffdelay[j];
                    tmpscr->ffx[i]=tmpscr->ffx[j];
                    tmpscr->ffy[i]=tmpscr->ffy[j];
                    tmpscr->ffxdelta[i]=tmpscr->ffxdelta[j];
                    tmpscr->ffydelta[i]=tmpscr->ffydelta[j];
                    tmpscr->ffxdelta2[i]=tmpscr->ffxdelta2[j];
                    tmpscr->ffydelta2[i]=tmpscr->ffydelta2[j];
                    tmpscr->fflink[i]=tmpscr->fflink[j];
                    tmpscr->ffwidth[i]=tmpscr->ffwidth[j];
                    tmpscr->ffheight[i]=tmpscr->ffheight[j];
                    
                    if(tmpscr->ffflags[i]&ffCARRYOVER)
                        tmpscr->ffflags[i]=tmpscr->ffflags[j]|ffCARRYOVER;
                    else
                        tmpscr->ffflags[i]=tmpscr->ffflags[j];
                        
                    tmpscr->ffflags[i]&=~ffCHANGER;
                    ffposx[i]=(short)(tmpscr->ffx[j]/10000);
                    ffposy[i]=(short)(tmpscr->ffy[j]/10000);
                    
                    if(combobuf[tmpscr->ffdata[j]].flag>15 && combobuf[tmpscr->ffdata[j]].flag<32)
                        tmpscr->ffdata[j]=tmpscr->secretcombo[combobuf[tmpscr->ffdata[j]].flag-16+4];
                        
                    if((tmpscr->ffflags[j]&ffSWAPNEXT)||(tmpscr->ffflags[j]&ffSWAPPREV))
                    {
                        int k=0;
                        
                        if(tmpscr->ffflags[j]&ffSWAPNEXT)
                            k=j<31?j+1:0;
                            
                        if(tmpscr->ffflags[j]&ffSWAPPREV)
                            k=j>0?j-1:31;
                            
                        zc_swap(tmpscr->ffdata[j],tmpscr->ffdata[k]);
                        zc_swap(tmpscr->ffcset[j],tmpscr->ffcset[k]);
                        zc_swap(tmpscr->ffdelay[j],tmpscr->ffdelay[k]);
                        zc_swap(tmpscr->ffxdelta[j],tmpscr->ffxdelta[k]);
                        zc_swap(tmpscr->ffydelta[j],tmpscr->ffydelta[k]);
                        zc_swap(tmpscr->ffxdelta2[j],tmpscr->ffxdelta2[k]);
                        zc_swap(tmpscr->ffydelta2[j],tmpscr->ffydelta2[k]);
                        zc_swap(tmpscr->fflink[j],tmpscr->fflink[k]);
                        zc_swap(tmpscr->ffwidth[j],tmpscr->ffwidth[k]);
                        zc_swap(tmpscr->ffheight[j],tmpscr->ffheight[k]);
                        zc_swap(tmpscr->ffflags[j],tmpscr->ffflags[k]);
                    }
                    
                    break;
                }
            }
        }
        
        if(tmpscr->fflink[i] ? !tmpscr->ffdelay[tmpscr->fflink[i]] : !tmpscr->ffdelay[i])
        {
            if(tmpscr->fflink[i]&&(tmpscr->fflink[i]-1)!=i)
            {
                ffprvx[i] = tmpscr->ffx[i];
                ffprvy[i] = tmpscr->ffy[i];
                tmpscr->ffx[i]+=tmpscr->ffxdelta[tmpscr->fflink[i]-1];
                tmpscr->ffy[i]+=tmpscr->ffydelta[tmpscr->fflink[i]-1];
            }
            else
            {
                ffprvx[i] = tmpscr->ffx[i];
                ffprvy[i] = tmpscr->ffy[i];
                tmpscr->ffx[i]+=tmpscr->ffxdelta[i];
                tmpscr->ffy[i]+=tmpscr->ffydelta[i];
                tmpscr->ffxdelta[i]+=tmpscr->ffxdelta2[i];
                tmpscr->ffydelta[i]+=tmpscr->ffydelta2[i];
                
                if(tmpscr->ffxdelta[i]>1280000) tmpscr->ffxdelta[i]=1280000;
                
                if(tmpscr->ffxdelta[i]<-1280000) tmpscr->ffxdelta[i]=-1280000;
                
                if(tmpscr->ffydelta[i]>1280000) tmpscr->ffydelta[i]=1280000;
                
                if(tmpscr->ffydelta[i]<-1280000) tmpscr->ffydelta[i]=-1280000;
            }
        }
        else
        {
            if(!tmpscr->fflink[i] || (tmpscr->fflink[i]-1)==i)
                tmpscr->ffdelay[i]--;
        }
        
        // Check if the FFC's off the side of the screen
        
        // Left
        if(tmpscr->ffx[i]<-320000)
        {
            if(tmpscr->flags6&fWRAPAROUNDFF)
            {
                tmpscr->ffx[i] = 2880000+(tmpscr->ffx[i]+320000);
                ffprvy[i] = tmpscr->ffy[i];
                ffposx[i]=-1000; // Re-enable previous changer
                ffposy[i]=-1000;
            }
            else if(tmpscr->ffx[i]<-640000)
            {
                tmpscr->ffdata[i]=0;
                tmpscr->ffflags[i]&=~ffCARRYOVER;
            }
        }
        
        // Right
        else if(tmpscr->ffx[i]>=2880000)
        {
            if(tmpscr->flags6&fWRAPAROUNDFF)
            {
                tmpscr->ffx[i] = tmpscr->ffx[i]-2880000-320000;
                ffprvy[i] = tmpscr->ffy[i];
                ffposx[i]=-1000;
                ffposy[i]=-1000;
            }
            else
            {
                tmpscr->ffdata[i]=0;
                tmpscr->ffflags[i]&=~ffCARRYOVER;
            }
        }
        
        // Top
        if(tmpscr->ffy[i]<-320000)
        {
            if(tmpscr->flags6&fWRAPAROUNDFF)
            {
                tmpscr->ffy[i] = 2080000+(tmpscr->ffy[i]+320000);
                ffprvx[i] = tmpscr->ffx[i];
                ffposx[i]=-1000;
                ffposy[i]=-1000;
            }
            else if(tmpscr->ffy[i]<-640000)
            {
                tmpscr->ffdata[i]=0;
                tmpscr->ffflags[i]&=~ffCARRYOVER;
            }
        }
        
        // Bottom
        else if(tmpscr->ffy[i]>=2080000)
        {
            if(tmpscr->flags6&fWRAPAROUNDFF)
            {
                tmpscr->ffy[i] = tmpscr->ffy[i]-2080000-320000;
                ffprvy[i] = tmpscr->ffy[i];
                ffposx[i]=-1000;
                ffposy[i]=-1000;
            }
            else
            {
                tmpscr->ffdata[i]=0;
                tmpscr->ffflags[i]&=~ffCARRYOVER;
            }
        }
    }
}

bool hitcombo(int x, int y, int combotype)
{
    return (COMBOTYPE(x,y)==combotype);
}

bool hitflag(int x, int y, int flagtype)
{
    return (MAPFLAG(x,y)==flagtype||MAPCOMBOFLAG(x,y)==flagtype);
}

int nextscr(int dir)
{
    int m = currmap;
    int s = currscr;
    
    switch(dir)
    {
    case up:
        s-=16;
        break;
        
    case down:
        s+=16;
        break;
        
    case left:
        s-=1;
        break;
        
    case right:
        s+=1;
        break;
    }
    
    // need to check for screens on other maps, 's' not valid, etc.
    
    int index = (tmpscr->sidewarpindex >> (dir*2))&3;
    
    // Fun fact: when a scrolling warp is triggered, this function
    // is never even called! - Saf
    if(tmpscr->sidewarptype[index] == 3)                                // scrolling warp
    {
        switch(dir)
        {
        case up:
            if(!(tmpscr->flags2&wfUP))    goto nowarp;
            
            break;
            
        case down:
            if(!(tmpscr->flags2&wfDOWN))  goto nowarp;
            
            break;
            
        case left:
            if(!(tmpscr->flags2&wfLEFT))  goto nowarp;
            
            break;
            
        case right:
            if(!(tmpscr->flags2&wfRIGHT)) goto nowarp;
            
            break;
        }
        
        m = DMaps[tmpscr->sidewarpdmap[index]].map;
        s = tmpscr->sidewarpscr[index] + DMaps[tmpscr->sidewarpdmap[index]].xoff;
    }
    
nowarp:
    if(s<0||s>=128)
        return 0xFFFF;

    return (m<<7) + s;
}

void bombdoor(int x,int y)
{
    if(tmpscr->door[0]==dBOMB && isinRect(x,y,100,0,139,48))
    {
        tmpscr->door[0]=dBOMBED;
        putdoor(scrollbuf,0,0,dBOMBED);
        setmapflag(mDOOR_UP);
        markBmap(-1);
        
        if(nextscr(up)!=0xFFFF)
        {
            setmapflag(nextscr(up), mDOOR_DOWN);
            markBmap(-1,nextscr(up)-(get_currdmap()<<7));
        }
    }
    
    if(tmpscr->door[1]==dBOMB && isinRect(x,y,100,112,139,176))
    {
        tmpscr->door[1]=dBOMBED;
        putdoor(scrollbuf,0,1,dBOMBED);
        setmapflag(mDOOR_DOWN);
        markBmap(-1);
        
        if(nextscr(down)!=0xFFFF)
        {
            setmapflag(nextscr(down), mDOOR_UP);
            markBmap(-1,nextscr(down)-(get_currdmap()<<7));
        }
    }
    
    if(tmpscr->door[2]==dBOMB && isinRect(x,y,0,60,48,98))
    {
        tmpscr->door[2]=dBOMBED;
        putdoor(scrollbuf,0,2,dBOMBED);
        setmapflag(mDOOR_LEFT);
        markBmap(-1);
        
        if(nextscr(left)!=0xFFFF)
        {
            setmapflag(nextscr(left), mDOOR_RIGHT);
            markBmap(-1,nextscr(left)-(get_currdmap()<<7));
        }
    }
    
    if(tmpscr->door[3]==dBOMB && isinRect(x,y,192,60,240,98))
    {
        tmpscr->door[3]=dBOMBED;
        putdoor(scrollbuf,0,3,dBOMBED);
        setmapflag(mDOOR_RIGHT);
        markBmap(-1);
        
        if(nextscr(right)!=0xFFFF)
        {
            setmapflag(nextscr(right), mDOOR_LEFT);
            markBmap(-1,nextscr(right)-(get_currdmap()<<7));
        }
    }
}

void do_scrolling_layer(BITMAP *bmp, int type, mapscr* layer, int x, int y, bool scrolling, int tempscreen)
{
    static int mf;
    
    switch(type)
    {
    case -4: //overhead FFCs
    case -3:                                                //freeform combos
        for(int i = 31; i >= 0; i--)
        {
            if(layer->ffdata[i])
            {
                if(!(layer->ffflags[i]&ffCHANGER) //If FFC is a changer, don't draw
					&& !((layer->ffflags[i]&ffLENSINVIS) && lensclk) //If lens is active and ffc is invis to lens, don't draw
					&& (!(layer->ffflags[i]&ffLENSVIS) || lensclk)) //If FFC does not require lens, or lens is active, draw
                {
                    if(scrolling && (layer->ffflags[i] & ffCARRYOVER) != 0 && tempscreen == 3)
                        continue; //If scrolling, only draw carryover ffcs from newscr and not oldscr,
                        
                    //otherwise we'll draw the same one twice
                    
                    if(!!(layer->ffflags[i]&ffOVERLAY) == (type==-4)) //what exactly is this supposed to mean?
                    {
                        int tx=((layer->ffx[i]/10000));
                        int ty=((layer->ffy[i]/10000))+playing_field_offset;
                        
                        if(layer->ffflags[i]&ffTRANS)
                        {
                            overcomboblocktranslucent(bmp, tx-x, ty-y, layer->ffdata[i], layer->ffcset[i], 1+(layer->ffwidth[i]>>6), 1+(layer->ffheight[i]>>6),128);
                        }
                        else
                        {
                            overcomboblock(bmp, tx-x, ty-y, layer->ffdata[i], layer->ffcset[i], 1+(layer->ffwidth[i]>>6), 1+(layer->ffheight[i]>>6));
                        }
                    }
                }
            }
        }
        
        break;
        
    case -2:                                                //push blocks
        for(int i=0; i<176; i++)
        {
            mf=layer->sflag[i];
            
            if(mf==mfPUSHUD || mf==mfPUSH4 || mf==mfPUSHED || ((mf>=mfPUSHLR)&&(mf<=mfPUSHRINS)))
            {
                overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,layer->data[i],layer->cset[i]);
            }
            else
            {
                mf=combobuf[layer->data[i]].flag;
                
                if(mf==mfPUSHUD || mf==mfPUSH4 || mf==mfPUSHED || ((mf>=mfPUSHLR)&&(mf<=mfPUSHRINS)))
                {
                    overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,layer->data[i],layer->cset[i]);
                }
            }
        }
        
        break;
        
    case -1:                                                //over combo
        for(int i=0; i<176; i++)
        {
//        if (combobuf[layer->data[i]].type==cOLD_OVERHEAD)
            if(combo_class_buf[combobuf[layer->data[i]].type].overhead)
            {
                overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,layer->data[i],layer->cset[i]);
            }
        }
        
        break;
        
    case 0:
    
        //case 1:
        //case 2:
    case 3:
    case 4:
    case 5:
        if(TransLayers || layer->layeropacity[type]==255)
        {
            if(layer->layermap[type]>0)
            {
                if(scrolling)
                {
                    if(layer->layeropacity[type]==255)
                    {
                        if(tempscreen==2)
                        {
                            for(int i=0; i<176; i++)
                            {
                                overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                            }
                        }
                        else
                        {
                            for(int i=0; i<176; i++)
                            {
                                overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                            }
                        }
                    }
                    else
                    {
                        if(tempscreen==2)
                        {
                            for(int i=0; i<176; i++)
                            {
                                overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i],layer->layeropacity[type]);
                            }
                        }
                        else
                        {
                            for(int i=0; i<176; i++)
                            {
                                overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i],layer->layeropacity[type]);
                            }
                        }
                    }
                }
                else
                {
                    if(layer->layeropacity[type]==255)
                    {
                        if(tempscreen==2)
                        {
                            for(int i=0; i<176; i++)
                            {
                                overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                            }
                        }
                        else
                        {
                            for(int i=0; i<176; i++)
                            {
                                overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                            }
                        }
                    }
                    else
                    {
                        if(tempscreen==2)
                        {
                            for(int i=0; i<176; i++)
                            {
                                overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i],layer->layeropacity[type]);
                            }
                        }
                        else
                        {
                            for(int i=0; i<176; i++)
                            {
                                overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i],layer->layeropacity[type]);
                            }
                        }
                    }
                }
            }
        }
        
        break;
        
    case 1:
        if(TransLayers || layer->layeropacity[type]==255)
        {
            if(layer->layermap[type]>0)
            {
                if(scrolling)
                {
                    if(layer->layeropacity[type]==255)
                    {
                        if(layer->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG)
                        {
                            if(tempscreen==2)
                            {
                                for(int i=0; i<176; i++)
                                {
                                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                                }
                            }
                            else
                            {
                                for(int i=0; i<176; i++)
                                {
                                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                                }
                            }
                        }
                        else
                        {
                            if(tempscreen==2)
                            {
                                for(int i=0; i<176; i++)
                                {
                                    overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                                }
                            }
                            else
                            {
                                for(int i=0; i<176; i++)
                                {
                                    overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                                }
                            }
                        }
                    }
                    else
                    {
                        if(layer->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG)
                        {
                            if(tempscreen==2)
                            {
                                for(int i=0; i<176; i++)
                                {
                                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                                }
                            }
                            else
                            {
                                for(int i=0; i<176; i++)
                                {
                                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                                }
                            }
                        }
                        else
                        {
                            if(tempscreen==2)
                            {
                                for(int i=0; i<176; i++)
                                {
                                    overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i],layer->layeropacity[type]);
                                }
                            }
                            else
                            {
                                for(int i=0; i<176; i++)
                                {
                                    overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i],layer->layeropacity[type]);
                                }
                            }
                        }
                    }
                }
                else
                {
                    if(layer->layeropacity[type]==255)
                    {
                        if(layer->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG)
                        {
                            if(tempscreen==2)
                            {
                                for(int i=0; i<176; i++)
                                {
                                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                                }
                            }
                            else
                            {
                                for(int i=0; i<176; i++)
                                {
                                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                                }
                            }
                        }
                        else
                        {
                            if(tempscreen==2)
                            {
                                for(int i=0; i<176; i++)
                                {
                                    overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                                }
                            }
                            else
                            {
                                for(int i=0; i<176; i++)
                                {
                                    overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                                }
                            }
                        }
                    }
                    else
                    {
                        if(layer->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG)
                        {
                            if(tempscreen==2)
                            {
                                for(int i=0; i<176; i++)
                                {
                                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                                }
                            }
                            else
                            {
                                for(int i=0; i<176; i++)
                                {
                                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                                }
                            }
                        }
                        else
                        {
                            if(tempscreen==2)
                            {
                                for(int i=0; i<176; i++)
                                {
                                    overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i],layer->layeropacity[type]);
                                }
                            }
                            else
                            {
                                for(int i=0; i<176; i++)
                                {
                                    overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i],layer->layeropacity[type]);
                                }
                            }
                        }
                    }
                }
            }
        }
        
        break;
        
    case 2:
        if(TransLayers || layer->layeropacity[type]==255)
        {
            if(layer->layermap[type]>0)
            {
                if(scrolling)
                {
                    if(layer->layeropacity[type]==255)
                    {
                        if( (layer->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG ) && !(layer->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG))
                        {
                            if(tempscreen==2)
                            {
                                for(int i=0; i<176; i++)
                                {
                                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                                }
                            }
                            else
                            {
                                for(int i=0; i<176; i++)
                                {
                                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                                }
                            }
                        }
                        else
                        {
                            if(tempscreen==2)
                            {
                                for(int i=0; i<176; i++)
                                {
                                    overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                                }
                            }
                            else
                            {
                                for(int i=0; i<176; i++)
                                {
                                    overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                                }
                            }
                        }
                    }
                    else
                    {
                        if( (layer->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG ) && !(layer->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG))
                        {
                            if(tempscreen==2)
                            {
                                for(int i=0; i<176; i++)
                                {
                                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                                }
                            }
                            else
                            {
                                for(int i=0; i<176; i++)
                                {
                                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                                }
                            }
                        }
                        else
                        {
                            if(tempscreen==2)
                            {
                                for(int i=0; i<176; i++)
                                {
                                    overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i],layer->layeropacity[type]);
                                }
                            }
                            else
                            {
                                for(int i=0; i<176; i++)
                                {
                                    overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i],layer->layeropacity[type]);
                                }
                            }
                        }
                    }
                }
                else
                {
                    if(layer->layeropacity[type]==255)
                    {
                        if((layer->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG) &&!(layer->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG))
                        {
                            if(tempscreen==2)
                            {
                                for(int i=0; i<176; i++)
                                {
                                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                                }
                            }
                            else
                            {
                                for(int i=0; i<176; i++)
                                {
                                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                                }
                            }
                        }
                        else
                        {
                            if(tempscreen==2)
                            {
                                for(int i=0; i<176; i++)
                                {
                                    overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                                }
                            }
                            else
                            {
                                for(int i=0; i<176; i++)
                                {
                                    overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                                }
                            }
                        }
                    }
                    else
                    {
                        if((layer->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG) &&!(layer->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG))
                        {
                            if(tempscreen==2)
                            {
                                for(int i=0; i<176; i++)
                                {
                                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                                }
                            }
                            else
                            {
                                for(int i=0; i<176; i++)
                                {
                                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                                }
                            }
                        }
                        else
                        {
                            if(tempscreen==2)
                            {
                                for(int i=0; i<176; i++)
                                {
                                    overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i],layer->layeropacity[type]);
                                }
                            }
                            else
                            {
                                for(int i=0; i<176; i++)
                                {
                                    overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i],layer->layeropacity[type]);
                                }
                            }
                        }
                    }
                }
            }
        }
        
        break;
    }
}


void do_layer(BITMAP *bmp, int type, mapscr* layer, int x, int y, int tempscreen, bool scrolling, bool drawprimitives)
{
    bool showlayer = true;
    
    switch(type)
    {
    case -4:
    case -3:
        if(!show_ffcs)
        {
            showlayer = false;
        }
        
        break;
        
    case -2:
        if(!show_layer_push)
        {
            showlayer = false;
        }
        
        break;
        
    case -1:
        if(!show_layer_over)
        {
            showlayer = false;
        }
        
        break;
        
    case 0:
        if(!show_layer_1)
        {
            showlayer = false;
        }
        
        break;
        
    case 1:
        if(!show_layer_2)
        {
            showlayer = false;
        }
        
        break;
        
    case 2:
        if(!show_layer_3)
        {
            showlayer = false;
        }
        
        break;
        
    case 3:
        if(!show_layer_4)
        {
            showlayer = false;
        }
        
        break;
        
    case 4:
        if(!show_layer_5)
        {
            showlayer = false;
        }
        
        break;
        
    case 5:
        if(!show_layer_6)
        {
            showlayer = false;
        }
        
        break;
    }
    
    if(type==(int)(layer->lens_layer&7) && ((layer->lens_layer&llLENSSHOWS && !lensclk) || (layer->lens_layer&llLENSHIDES && lensclk)))
    {
        showlayer = false;
    }
    
	
    if(showlayer)
    {
		if(!(type >= 0 && (layer->hidelayers & (1 << (type+1)))))
			do_scrolling_layer(bmp, type, layer, x, y, scrolling, tempscreen);
        
        if(drawprimitives && type >= 0 && type <= 5)
        {
            do_primitives(bmp, type+1, layer, 0,  playing_field_offset);
        }
    }
}


// Called by do_walkflags
void put_walkflags(BITMAP *dest,int x,int y,int xofs,int yofs, word cmbdat,int lyr)
{
    newcombo c = combobuf[cmbdat];
    
    if (c.type == cBRIDGE) return;
    
    int xx = x-xofs;
    int yy = y+playing_field_offset-yofs;
    
    int bridgedetected = 0;
    
    for(int i=0; i<4; i++)
    {
        int tx=((i&2)<<2)+xx;
        int ty=((i&1)<<3)+yy;
	int tx2=((i&2)<<2)+x;
        int ty2=((i&1)<<3)+y;
	for (int m = lyr-1; m <= 1; m++)
	{
		if (combobuf[MAPCOMBO2(m,tx2,ty2)].type == cBRIDGE && !_walkflag_layer(tx2,ty2,1, &(tmpscr2[m]))) 
		{
			bridgedetected |= (1<<i);
		}
        }
	if ((bridgedetected & (1<<i))) 
	{
		if (i >= 3) break;
		else continue;
	}
	bool doladdercheck = true;
        //if ( iswaterex(cmbdat, currmap, currscr, lyr, tx2, ty2, true, false, false)!=0 )
	if (iswater_type(c.type) && !DRIEDLAKE) //Yes, I realize this is horribly inaccurate; the alternative is the game chugging every time you turn on walk cheats.
	{
		if (get_bit(quest_rules,  qr_NO_SOLID_SWIM)) doladdercheck = false;
		if((lyr==0 || (get_bit(quest_rules,  qr_WATER_ON_LAYER_1) && lyr == 1) || (get_bit(quest_rules,  qr_WATER_ON_LAYER_2) && lyr == 2)) && get_bit(quest_rules, qr_DROWN))
			rectfill(dest,tx,ty,tx+7,ty+7,makecol(85,85,255));
		else rectfill(dest,tx,ty,tx+7,ty+7,makecol(0,0,255));
	}
	
        if(c.walk&(1<<i) && !(iswater_type(c.type) && DRIEDLAKE))  // Check for dried lake (watertype && not water)
        {
            if(c.type==cLADDERHOOKSHOT && isstepable(cmbdat) && ishookshottable(xx,yy))
            {
                for(int k=0; k<8; k+=2)
                    for(int j=0; j<8; j+=2)
                        rectfill(dest,tx+k,ty+j,tx+k+1,ty+j+1,((k+j)/2)%2 ? makecol(165,105,8) : makecol(170,170,170));
            }
            else
            {
                int color = makecol(255,85,85);
                
                if(isstepable(cmbdat)&& (!doladdercheck))
                    color=makecol(165,105,8);
                else if((c.type==cHOOKSHOTONLY || c.type==cLADDERHOOKSHOT) && ishookshottable(xx,yy))
                    color=makecol(170,170,170);
                    
                rectfill(dest,tx,ty,tx+7,ty+7,color);
            }
        }
    }
    
    // Draw damage combos
    bool dmg = combo_class_buf[combobuf[MAPCOMBO2(-1,xx,yy)].type].modify_hp_amount
               || combo_class_buf[combobuf[MAPCOMBO2(0,xx,yy)].type].modify_hp_amount
               || combo_class_buf[combobuf[MAPCOMBO2(1,xx,yy)].type].modify_hp_amount;
               
    if(dmg)
    {
	int color = makecol(255,255,0);
	if (bridgedetected <= 0)
	{
		for(int k=0; k<16; k+=2)
		    for(int j=0; j<16; j+=2)
			if(((k+j)/2)%2)
			    rectfill(dest,x+k,y+j,x+k+1,y+j+1,color);
	}
	else
	{
		for(int i=0; i<4; i++)
		{
			if (!(bridgedetected & (1<<i)))
			{
				int tx=((i&2)<<2)+x;
				int ty=((i&1)<<3)+y;
				for(int k=0; k<8; k+=2)
				    for(int j=0; j<8; j+=2)
					if((k+j)%4 < 2)
					    rectfill(dest,tx+k,ty+j,tx+k+1,ty+j+1,color);
			}
		}
	}
    }
}

// Walkflags L4 cheat
void do_walkflags(BITMAP *dest,mapscr* layer,int x, int y, int tempscreen)
{
    if(show_walkflags)
    {
        for(int i=0; i<176; i++)
        {
            put_walkflags(dest,((i&15)<<4),(i&0xF0),x,y,layer->data[i], 0);
        }
        
        int layermap;
        
        for(int k=0; k<2; k++)
        {
            layermap=layer->layermap[k%2];
            
            if(layermap>0)
            {
                if(tempscreen==2)
                {
                    for(int i=0; i<176; i++)
                    {
                        put_walkflags(temp_buf,((i&15)<<4),(i&0xF0),x,y,tmpscr2[k].data[i], k%2+1);
                        put_walkflags(scrollbuf,((i&15)<<4),(i&0xF0),x,y,tmpscr2[k].data[i], k%2+1);
                    }
                }
                else
                {
                    for(int i=0; i<176; i++)
                    {
                        put_walkflags(temp_buf,((i&15)<<4),(i&0xF0),x,y,tmpscr3[k].data[i], k%2+1);
                        put_walkflags(scrollbuf,((i&15)<<4),(i&0xF0),x,y,tmpscr3[k].data[i], k%2+1);
                    }
                }
            }
        }
    }
}

void draw_screen(mapscr* this_screen, bool showlink)
{
	if((GameFlags & (GAMEFLAG_SCRIPTMENU_ACTIVE|GAMEFLAG_F6SCRIPT_ACTIVE))!=0)
	{
		FFCore.doScriptMenuDraws();
		return;
	}
    //The Plan:
	//0: Set sideview gravity from dmaps. -Z
    //1. Draw some layers onto scrollbuf with clipping
    //2. Blit scrollbuf onto framebuf
    //3. Draw some sprites onto framebuf
    //4. Blit framebuf onto temp_buf
    //5. Draw some layers onto temp_buf and scrollbuf
    //6. Blit temp_buf onto framebuf with clipping
    //6b. Draw the subscreen onto temp_buf, without clipping
    //7. Draw some flying sprites onto framebuf
    //8. Blit frame_buf onto temp_buf
    //9. Draw some layers onto temp_buf
    //10. Blit temp_buf onto framebuf with clipping
    //11. Draw some text on framebuf and scrollbuf
    //12. Draw the subscreen onto framebuf, without clipping
    clear_bitmap(framebuf);
    set_clip_rect(framebuf,0,0,256,224);
    
    clear_bitmap(temp_buf);
    set_clip_state(temp_buf,1);
    set_clip_rect(temp_buf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
    
    int cmby2=0;
    int pcounter;
    
    //0: Sideview Grvity from DMaps.
    
    /* DON'T MODIFY THE SCREEN DIRECTLY!
	if ( DMaps[currdmap].sideview != 0 ) 
    {
		this_screen->flags7 |= fSIDEVIEW;
    }*/
    //1. Draw some layers onto temp_buf
    clear_bitmap(scrollbuf);
    
    if(this_screen->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG)
    {
        do_layer(scrollbuf,1, this_screen, 0, 0, 2, false, true);
        
        for(pcounter=0; pcounter<particles.Count(); pcounter++)
        {
            if(((particle*)particles.spr(pcounter))->layer==1)
            {
                particles.spr(pcounter)->draw(scrollbuf);
            }
        }
    }
    
    if(this_screen->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG)
    {
        do_layer(scrollbuf,2, this_screen, 0, 0, 2, false, true);
        
        for(pcounter=0; pcounter<particles.Count(); pcounter++)
        {
            if(((particle*)particles.spr(pcounter))->layer==2)
            {
                particles.spr(pcounter)->draw(scrollbuf);
            }
        }
    }
    
    putscr(scrollbuf,0,playing_field_offset,this_screen);
    
    // Lens hints, then primitives, then particles.
    if((lensclk || (get_debug() && zc_getkey(KEY_L))) && !get_bit(quest_rules, qr_OLDLENSORDER))
    {
        draw_lens_under(scrollbuf, false);
    }
    
    if(show_layer_0)
        do_primitives(scrollbuf, 0, this_screen, 0, playing_field_offset);
        
    for(pcounter=0; pcounter<particles.Count(); pcounter++)
    {
        if(((particle*)particles.spr(pcounter))->layer==-3)
        {
            particles.spr(pcounter)->draw(scrollbuf);
        }
    }
    
    set_clip_rect(scrollbuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
    
    if(!(get_bit(quest_rules,qr_LAYER12UNDERCAVE)))
    {
        if(showlink &&
                ((Link.getAction()==climbcovertop)||(Link.getAction()==climbcoverbottom)))
        {
            if(Link.getAction()==climbcovertop)
            {
                cmby2=16;
            }
            else if(Link.getAction()==climbcoverbottom)
            {
                cmby2=-16;
            }
            
            decorations.draw2(scrollbuf,true);
            Link.draw(scrollbuf);
            decorations.draw(scrollbuf,true);
            int ccx = (int)(Link.getClimbCoverX());
            int ccy = (int)(Link.getClimbCoverY());
            
            overcombo(scrollbuf,ccx,ccy+cmby2+playing_field_offset,MAPCOMBO(ccx,ccy+cmby2),MAPCSET(ccx,ccy+cmby2));
            putcombo(scrollbuf,ccx,ccy+playing_field_offset,MAPCOMBO(ccx,ccy),MAPCSET(ccx,ccy));
            
            if(int(Link.getX())&15)
            {
                overcombo(scrollbuf,ccx+16,ccy+cmby2+playing_field_offset,MAPCOMBO(ccx+16,ccy+cmby2),MAPCSET(ccx+16,ccy+cmby2));
                putcombo(scrollbuf,ccx+16,ccy+playing_field_offset,MAPCOMBO(ccx+16,ccy),MAPCSET(ccx+16,ccy));
            }
        }
    }
    
    do_layer(scrollbuf,0, this_screen, 0, 0, 2, false, true); // LAYER 1
    
    for(pcounter=0; pcounter<particles.Count(); pcounter++)
    {
        if(((particle*)particles.spr(pcounter))->layer==0)
        {
            particles.spr(pcounter)->draw(scrollbuf);
        }
    }
    
    do_layer(scrollbuf,-3, this_screen, 0, 0, 2); // freeform combos!
    
    if(!(this_screen->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG))
    {
        do_layer(scrollbuf,1, this_screen, 0, 0, 2, false, true); // LAYER 2
        
        for(pcounter=0; pcounter<particles.Count(); pcounter++)
        {
            if(((particle*)particles.spr(pcounter))->layer==1)
            {
                particles.spr(pcounter)->draw(scrollbuf);
            }
        }
    }
    
    if(get_bit(quest_rules,qr_LAYER12UNDERCAVE))
    {
        if(showlink &&
                ((Link.getAction()==climbcovertop)||(Link.getAction()==climbcoverbottom)))
        {
            if(Link.getAction()==climbcovertop)
            {
                cmby2=16;
            }
            else if(Link.getAction()==climbcoverbottom)
            {
                cmby2=-16;
            }
            
            decorations.draw2(scrollbuf,true);
            Link.draw(scrollbuf);
            decorations.draw(scrollbuf,true);
            int ccx = (int)(Link.getClimbCoverX());
            int ccy = (int)(Link.getClimbCoverY());
            
            overcombo(scrollbuf,ccx,ccy+cmby2+playing_field_offset,MAPCOMBO(ccx,ccy+cmby2),MAPCSET(ccx,ccy+cmby2));
            putcombo(scrollbuf,ccx,ccy+playing_field_offset,MAPCOMBO(ccx,ccy),MAPCSET(ccx,ccy));
            
            if(int(Link.getX())&15)
            {
                overcombo(scrollbuf,ccx+16,ccy+cmby2+playing_field_offset,MAPCOMBO(ccx+16,ccy+cmby2),MAPCSET(ccx+16,ccy+cmby2));
                putcombo(scrollbuf,ccx+16,ccy+playing_field_offset,MAPCOMBO(ccx+16,ccy),MAPCSET(ccx+16,ccy));
            }
        }
    }
    
    do_layer(scrollbuf,-2, this_screen, 0, 0, 2); // push blocks!
    
    //Show walkflags cheat
    do_walkflags(temp_buf,this_screen,0,0,2);
    do_walkflags(scrollbuf,this_screen,0,0,2);
    
    putscrdoors(scrollbuf,0,playing_field_offset,this_screen);
    
    // Lens hints, doors etc.
    if(lensclk || (get_debug() && zc_getkey(KEY_L)))
    {
        if(get_bit(quest_rules, qr_OLDLENSORDER))
        {
            draw_lens_under(scrollbuf, false);
        }
        
        draw_lens_under(scrollbuf, true);
    }
    
    //2. Blit those layers onto framebuf
    
    
    set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
    masked_blit(scrollbuf, framebuf, 0, 0, 0, 0, 256, 224);
    
    
    //3. Draw some sprites onto framebuf
    set_clip_rect(framebuf,0,0,256,224);
    
    if(!(pricesdisplaybuf->clip))
    {
        masked_blit(pricesdisplaybuf,framebuf,0,0,0,playing_field_offset,256,168);
    }
    
    if(showlink && ((Link.getAction()!=climbcovertop)&&(Link.getAction()!=climbcoverbottom)))
    {
        Link.draw_under(framebuf);
        
        if(Link.isSwimming())
        {
            decorations.draw2(framebuf,true);
            Link.draw(framebuf);
            decorations.draw(framebuf,true);
        }
    }
    
    if(drawguys)
    {
        if(get_bit(quest_rules,qr_NOFLICKER) || (frame&1))
        {
            for(int i=0; i<Ewpns.Count(); i++)
            {
                if(((weapon *)Ewpns.spr(i))->behind)
                    Ewpns.spr(i)->draw(framebuf);
            }
	    
	    for(int i=0; i<Lwpns.Count(); i++)
            {
                if(((weapon *)Lwpns.spr(i))->behind)
                    Lwpns.spr(i)->draw(framebuf);
            }
            
            if(get_bit(quest_rules,qr_SHADOWS)&&(!get_bit(quest_rules,qr_SHADOWSFLICKER)||frame&1))
            {
                guys.drawshadow(framebuf,get_bit(quest_rules,qr_TRANSSHADOWS)!=0,true);
            }
            
            guys.draw(framebuf,true);
            chainlinks.draw(framebuf,true);
            //Lwpns.draw(framebuf,true);
            
            for(int i=0; i<Ewpns.Count(); i++)
            {
                if(!((weapon *)Ewpns.spr(i))->behind)
                    Ewpns.spr(i)->draw(framebuf);
            }
	    
	    for(int i=0; i<Lwpns.Count(); i++)
            {
                if(!((weapon *)Lwpns.spr(i))->behind)
                    Lwpns.spr(i)->draw(framebuf);
            }
	    
            
            items.draw(framebuf,true);
        }
        else
        {
            for(int i=0; i<Ewpns.Count(); i++)
            {
                if(((weapon *)Ewpns.spr(i))->behind)
                    Ewpns.spr(i)->draw(framebuf);
            }
	    
	    for(int i=0; i<Lwpns.Count(); i++)
            {
                if(((weapon *)Lwpns.spr(i))->behind)
                    Lwpns.spr(i)->draw(framebuf);
            }
            
            if(get_bit(quest_rules,qr_SHADOWS)&&(!get_bit(quest_rules,qr_SHADOWSFLICKER)||frame&1))
            {
                guys.drawshadow(framebuf,get_bit(quest_rules,qr_TRANSSHADOWS)!=0,true);
            }
            
            items.draw(framebuf,false);
            chainlinks.draw(framebuf,false);
            //Lwpns.draw(framebuf,false);
            guys.draw(framebuf,false);
            
            for(int i=0; i<Ewpns.Count(); i++)
            {
                if(!((weapon *)Ewpns.spr(i))->behind)
                {
                    Ewpns.spr(i)->draw(framebuf);
                }
            }
	    
	    for(int i=0; i<Lwpns.Count(); i++)
            {
                if(!((weapon *)Lwpns.spr(i))->behind)
                {
                    Lwpns.spr(i)->draw(framebuf);
                }
            }
        }
        
        guys.draw2(framebuf,true);
    }
    
    if(showlink && ((Link.getAction()!=climbcovertop)&& (Link.getAction()!=climbcoverbottom)))
    {
        mblock2.draw(framebuf);
        
        if(!Link.isSwimming())
        {
            if(Link.getZ()>0 &&(!get_bit(quest_rules,qr_SHADOWSFLICKER)||frame&1))
            {
                Link.drawshadow(framebuf,get_bit(quest_rules,qr_TRANSSHADOWS)!=0);
            }
            
            decorations.draw2(framebuf,true);
            Link.draw(framebuf);
            decorations.draw(framebuf,true);
        }
    }
    
    for(int i=0; i<guys.Count(); i++)
    {
        if(((enemy*)guys.spr(i))->family == eeWALK)
        {
            if(((eStalfos*)guys.spr(i))->haslink)
            {
                guys.spr(i)->draw(framebuf);
            }
        }
        
        if(((enemy*)guys.spr(i))->family == eeWALLM)
        {
            if(((eWallM*)guys.spr(i))->haslink)
            {
                guys.spr(i)->draw(framebuf);
            }
        }
        
        if(guys.spr(i)->z > Link.getZ())
        {
            //Jumping enemies in front of Link.
            guys.spr(i)->draw(framebuf);
        }
    }
    
    //4. Blit framebuf onto temp_buf
    
    //you have to do this, because do_layer calls overcombo, which doesn't respect the clipping rectangle, which messes up the triforce curtain. -DD
    blit(framebuf, temp_buf, 0, 0, 0, 0, 256, 224);
    
    //5. Draw some layers onto temp_buf and scrollbuf
    
    if(!(this_screen->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG))
    {
        do_layer(temp_buf,2, this_screen, 0, 0, 2, false, true);
        do_layer(scrollbuf, 2, this_screen, 0, 0, 2);
        
        for(pcounter=0; pcounter<particles.Count(); pcounter++)
        {
            if(((particle*)particles.spr(pcounter))->layer==2)
            {
                particles.spr(pcounter)->draw(temp_buf);
            }
        }
    }
    
    do_layer(temp_buf,3, this_screen, 0, 0, 2, false, true);
    do_layer(scrollbuf, 3, this_screen, 0, 0, 2);
    //do_primitives(temp_buf, 3, this_screen, 0,playing_field_offset);//don't uncomment me
    
    for(pcounter=0; pcounter<particles.Count(); pcounter++)
    {
        if(((particle*)particles.spr(pcounter))->layer==3)
        {
            particles.spr(pcounter)->draw(temp_buf);
        }
    }
    
    do_layer(temp_buf,-1, this_screen, 0, 0, 2);
    do_layer(scrollbuf,-1, this_screen, 0, 0, 2);
    
    for(pcounter=0; pcounter<particles.Count(); pcounter++)
    {
        if(((particle*)particles.spr(pcounter))->layer==-1)
        {
            particles.spr(pcounter)->draw(temp_buf);
        }
    }
    
    //6. Blit temp_buf onto framebuf with clipping
    
    set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
    blit(temp_buf, framebuf, 0, 0, 0, 0, 256, 224);
    
    //6b. Draw the subscreen, without clipping
    if(!get_bit(quest_rules,qr_SUBSCREENOVERSPRITES))
    {
        set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
        put_passive_subscr(framebuf, &QMisc, 0, passive_subscreen_offset, false, sspUP);
    }
    
    
    //7. Draw some flying sprites onto framebuf
    set_clip_rect(framebuf,0,0,256,224);
    
    //Jumping Link and jumping enemies are drawn on this layer.
    if(Link.getZ() > (zfix)zinit.jump_link_layer_threshold)
    {
        decorations.draw2(framebuf,false);
        Link.draw(framebuf);
        chainlinks.draw(framebuf,true);
        
        for(int i=0; i<Lwpns.Count(); i++)
        {
            if(Lwpns.spr(i)->z > (zfix)zinit.jump_link_layer_threshold)
            {
                Lwpns.spr(i)->draw(framebuf);
            }
        }
        
        decorations.draw(framebuf,false);
    }
    
    if(!get_bit(quest_rules,qr_ENEMIESZAXIS)) for(int i=0; i<guys.Count(); i++)
        {
            if((isflier(guys.spr(i)->id)) || guys.spr(i)->z > (zfix)zinit.jump_link_layer_threshold)
            {
                guys.spr(i)->draw(framebuf);
            }
        }
    else
    {
        for(int i=0; i<guys.Count(); i++)
        {
            if((isflier(guys.spr(i)->id)) || guys.spr(i)->z > 0)
            {
                guys.spr(i)->draw(framebuf);
            }
        }
    }
    
    // Draw the Moving Fairy above layer 3
    for(int i=0; i<items.Count(); i++)
        if(itemsbuf[items.spr(i)->id].family == itype_fairy && itemsbuf[items.spr(i)->id].misc3)
            items.spr(i)->draw(framebuf);
            
    //8. Blit framebuf onto temp_buf
    
    masked_blit(framebuf, temp_buf, 0, 0, 0, 0, 256, 224);
    
    //9. Draw some layers onto temp_buf and scrollbuf
    
    set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
    
    do_layer(temp_buf,4, this_screen, 0, 0, 2, false, true);
    do_layer(scrollbuf, 4, this_screen, 0, 0, 2);
    
    for(pcounter=0; pcounter<particles.Count(); pcounter++)
    {
        if(((particle*)particles.spr(pcounter))->layer==4)
        {
            particles.spr(pcounter)->draw(temp_buf);
        }
    }
    
    do_layer(temp_buf,-4, this_screen, 0, 0, 2); // overhead freeform combos!
    do_layer(scrollbuf, -4, this_screen, 0, 0, 2);
    
    do_layer(temp_buf,5, this_screen, 0, 0, 2, false, true);
    do_layer(scrollbuf, 5, this_screen, 0, 0, 2);
    
    for(pcounter=0; pcounter<particles.Count(); pcounter++)
    {
        if(((particle*)particles.spr(pcounter))->layer==5)
        {
            particles.spr(pcounter)->draw(temp_buf);
        }
    }
    
    //10. Blit temp_buf onto framebuf with clipping
    
    set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
    blit(temp_buf, framebuf, 0, 0, 0, 0, 256, 224);
    
    
    //11. Draw some text on framebuf
    
    set_clip_rect(framebuf,0,0,256,224);
    set_clip_rect(scrollbuf,0,0,256,224);
    
    if(!(msg_bg_display_buf->clip))
    {
		blit_msgstr_bg(framebuf,0,0,0,playing_field_offset,256,168);
		blit_msgstr_bg(scrollbuf,0,0,0,playing_field_offset,256,168);
    }
    
    if(!(msg_portrait_display_buf->clip))
    {
		blit_msgstr_prt(framebuf,0,0,0,playing_field_offset,256,168);
		blit_msgstr_prt(scrollbuf,0,0,0,playing_field_offset,256,168);
    }
    
    if(!(msg_txt_display_buf->clip))
    {
		blit_msgstr_fg(framebuf,0,0,0,playing_field_offset,256,168);
		blit_msgstr_fg(scrollbuf,0,0,0,playing_field_offset,256,168);
    }
    
    //12. Draw the subscreen, without clipping
    
    if(get_bit(quest_rules,qr_SUBSCREENOVERSPRITES))
    {
        put_passive_subscr(framebuf, &QMisc, 0, passive_subscreen_offset, false, sspUP);
        
        // Draw primitives over subscren
        do_primitives(framebuf, 7, this_screen, 0, playing_field_offset); //Layer '7' appears above subscreen if quest rule is set
    }
    
    set_clip_rect(scrollbuf, 0, 0, scrollbuf->w, scrollbuf->h);
}

void put_door(BITMAP *dest,int t,int pos,int side,int type,bool redraw,bool even_walls)
{
    int d=tmpscr[t].door_combo_set;
    
    switch(type)
    {
    case dt_wall:
    case dt_walk:
        if(!even_walls)
            break;
            
    case dt_pass:
        if(!get_bit(quest_rules, qr_REPLACEOPENDOORS) && !even_walls)
            break;
            
    case dt_lock:
    case dt_shut:
    case dt_boss:
    case dt_olck:
    case dt_osht:
    case dt_obos:
    case dt_bomb:
        switch(side)
        {
        case up:
            tmpscr[t].data[pos]   = DoorComboSets[d].doorcombo_u[type][0];
            tmpscr[t].cset[pos]   = DoorComboSets[d].doorcset_u[type][0];
            tmpscr[t].sflag[pos]  = 0;
            tmpscr[t].data[pos+1]   = DoorComboSets[d].doorcombo_u[type][1];
            tmpscr[t].cset[pos+1]   = DoorComboSets[d].doorcset_u[type][1];
            tmpscr[t].sflag[pos+1]  = 0;
            tmpscr[t].data[pos+16]   = DoorComboSets[d].doorcombo_u[type][2];
            tmpscr[t].cset[pos+16]   = DoorComboSets[d].doorcset_u[type][2];
            tmpscr[t].sflag[pos+16]  = 0;
            tmpscr[t].data[pos+16+1]   = DoorComboSets[d].doorcombo_u[type][3];
            tmpscr[t].cset[pos+16+1]   = DoorComboSets[d].doorcset_u[type][3];
            tmpscr[t].sflag[pos+16+1]  = 0;
            
            if(redraw)
            {
                putcombo(dest,(pos&15)<<4,pos&0xF0,
                         DoorComboSets[d].doorcombo_u[type][0],
                         DoorComboSets[d].doorcset_u[type][0]);
                putcombo(dest,((pos&15)<<4)+16,pos&0xF0,
                         DoorComboSets[d].doorcombo_u[type][1],
                         DoorComboSets[d].doorcset_u[type][1]);
            }
            
            break;
            
        case down:
            tmpscr[t].data[pos]   = DoorComboSets[d].doorcombo_d[type][0];
            tmpscr[t].cset[pos]   = DoorComboSets[d].doorcset_d[type][0];
            tmpscr[t].sflag[pos]  = 0;
            tmpscr[t].data[pos+1]   = DoorComboSets[d].doorcombo_d[type][1];
            tmpscr[t].cset[pos+1]   = DoorComboSets[d].doorcset_d[type][1];
            tmpscr[t].sflag[pos+1]  = 0;
            tmpscr[t].data[pos+16]   = DoorComboSets[d].doorcombo_d[type][2];
            tmpscr[t].cset[pos+16]   = DoorComboSets[d].doorcset_d[type][2];
            tmpscr[t].sflag[pos+16]  = 0;
            tmpscr[t].data[pos+16+1]   = DoorComboSets[d].doorcombo_d[type][3];
            tmpscr[t].cset[pos+16+1]   = DoorComboSets[d].doorcset_d[type][3];
            tmpscr[t].sflag[pos+16+1]  = 0;
            
            if(redraw)
            {
                putcombo(dest,(pos&15)<<4,(pos&0xF0)+16,
                         DoorComboSets[d].doorcombo_d[type][2],
                         DoorComboSets[d].doorcset_d[type][2]);
                putcombo(dest,((pos&15)<<4)+16,(pos&0xF0)+16,
                         DoorComboSets[d].doorcombo_d[type][3],
                         DoorComboSets[d].doorcset_d[type][3]);
            }
            
            break;
            
        case left:
            tmpscr[t].data[pos]   = DoorComboSets[d].doorcombo_l[type][0];
            tmpscr[t].cset[pos]   = DoorComboSets[d].doorcset_l[type][0];
            tmpscr[t].sflag[pos]  = 0;
            tmpscr[t].data[pos+1]   = DoorComboSets[d].doorcombo_l[type][1];
            tmpscr[t].cset[pos+1]   = DoorComboSets[d].doorcset_l[type][1];
            tmpscr[t].sflag[pos+1]  = 0;
            tmpscr[t].data[pos+16]   = DoorComboSets[d].doorcombo_l[type][2];
            tmpscr[t].cset[pos+16]   = DoorComboSets[d].doorcset_l[type][2];
            tmpscr[t].sflag[pos+16]  = 0;
            tmpscr[t].data[pos+16+1]   = DoorComboSets[d].doorcombo_l[type][3];
            tmpscr[t].cset[pos+16+1]   = DoorComboSets[d].doorcset_l[type][3];
            tmpscr[t].sflag[pos+16+1]  = 0;
            tmpscr[t].data[pos+32]   = DoorComboSets[d].doorcombo_l[type][4];
            tmpscr[t].cset[pos+32]   = DoorComboSets[d].doorcset_l[type][4];
            tmpscr[t].sflag[pos+32]  = 0;
            tmpscr[t].data[pos+32+1]   = DoorComboSets[d].doorcombo_l[type][5];
            tmpscr[t].cset[pos+32+1]   = DoorComboSets[d].doorcset_l[type][5];
            tmpscr[t].sflag[pos+32+1]  = 0;
            
            if(redraw)
            {
                putcombo(dest,(pos&15)<<4,pos&0xF0,
                         DoorComboSets[d].doorcombo_l[type][0],
                         DoorComboSets[d].doorcset_l[type][0]);
                putcombo(dest,(pos&15)<<4,(pos&0xF0)+16,
                         DoorComboSets[d].doorcombo_l[type][2],
                         DoorComboSets[d].doorcset_l[type][2]);
                putcombo(dest,(pos&15)<<4,(pos&0xF0)+32,
                         DoorComboSets[d].doorcombo_l[type][4],
                         DoorComboSets[d].doorcset_l[type][4]);
            }
            
            break;
            
        case right:
            tmpscr[t].data[pos]   = DoorComboSets[d].doorcombo_r[type][0];
            tmpscr[t].cset[pos]   = DoorComboSets[d].doorcset_r[type][0];
            tmpscr[t].sflag[pos]  = 0;
            tmpscr[t].data[pos+1]   = DoorComboSets[d].doorcombo_r[type][1];
            tmpscr[t].cset[pos+1]   = DoorComboSets[d].doorcset_r[type][1];
            tmpscr[t].sflag[pos+1]  = 0;
            tmpscr[t].data[pos+16]   = DoorComboSets[d].doorcombo_r[type][2];
            tmpscr[t].cset[pos+16]   = DoorComboSets[d].doorcset_r[type][2];
            tmpscr[t].sflag[pos+16]  = 0;
            tmpscr[t].data[pos+16+1]   = DoorComboSets[d].doorcombo_r[type][3];
            tmpscr[t].cset[pos+16+1]   = DoorComboSets[d].doorcset_r[type][3];
            tmpscr[t].sflag[pos+16+1]  = 0;
            tmpscr[t].data[pos+32]   = DoorComboSets[d].doorcombo_r[type][4];
            tmpscr[t].cset[pos+32]   = DoorComboSets[d].doorcset_r[type][4];
            tmpscr[t].sflag[pos+32]  = 0;
            tmpscr[t].data[pos+32+1]   = DoorComboSets[d].doorcombo_r[type][5];
            tmpscr[t].cset[pos+32+1]   = DoorComboSets[d].doorcset_r[type][5];
            tmpscr[t].sflag[pos+32+1]  = 0;
            
            if(redraw)
            {
                putcombo(dest,(pos&15)<<4,pos&0xF0,
                         DoorComboSets[d].doorcombo_r[type][0],
                         DoorComboSets[d].doorcset_r[type][0]);
                putcombo(dest,(pos&15)<<4,(pos&0xF0)+16,
                         DoorComboSets[d].doorcombo_r[type][2],
                         DoorComboSets[d].doorcset_r[type][2]);
                putcombo(dest,(pos&15)<<4,(pos&0xF0)+32,
                         DoorComboSets[d].doorcombo_r[type][4],
                         DoorComboSets[d].doorcset_r[type][4]);
            }
            
            break;
        }
        
        break;
        
    default:
        break;
    }
}

void over_door(BITMAP *dest,int t, int pos,int side, int xoff, int yoff)
{
    int d=tmpscr[t].door_combo_set;
    int x=(pos&15)<<4;
    int y=(pos&0xF0);
    
    switch(side)
    {
    case up:
        overcombo2(dest,x+xoff,y+yoff,
                   DoorComboSets[d].bombdoorcombo_u[0],
                   DoorComboSets[d].bombdoorcset_u[0]);
        overcombo2(dest,x+16+xoff,y+yoff,
                   DoorComboSets[d].bombdoorcombo_u[1],
                   DoorComboSets[d].bombdoorcset_u[1]);
        break;
        
    case down:
        overcombo2(dest,x+xoff,y+yoff,
                   DoorComboSets[d].bombdoorcombo_d[0],
                   DoorComboSets[d].bombdoorcset_d[0]);
        overcombo2(dest,x+16+xoff,y+yoff,
                   DoorComboSets[d].bombdoorcombo_d[1],
                   DoorComboSets[d].bombdoorcset_d[1]);
        break;
        
    case left:
        overcombo2(dest,x+xoff,y+yoff,
                   DoorComboSets[d].bombdoorcombo_l[0],
                   DoorComboSets[d].bombdoorcset_l[0]);
        overcombo2(dest,x+xoff,y+yoff+16,
                   DoorComboSets[d].bombdoorcombo_l[1],
                   DoorComboSets[d].bombdoorcset_l[1]);
        overcombo2(dest,x+xoff,y+yoff+16,
                   DoorComboSets[d].bombdoorcombo_l[2],
                   DoorComboSets[d].bombdoorcset_l[2]);
        break;
        
    case right:
        overcombo2(dest,x+xoff,y+yoff,
                   DoorComboSets[d].bombdoorcombo_r[0],
                   DoorComboSets[d].bombdoorcset_r[0]);
        overcombo2(dest,x+xoff,y+yoff+16,
                   DoorComboSets[d].bombdoorcombo_r[1],
                   DoorComboSets[d].bombdoorcset_r[1]);
        overcombo2(dest,x+xoff,y+yoff+16,
                   DoorComboSets[d].bombdoorcombo_r[2],
                   DoorComboSets[d].bombdoorcset_r[2]);
        break;
    }
}

void putdoor(BITMAP *dest,int t,int side,int door,bool redraw,bool even_walls)
{
    /*
      #define dWALL           0  //  000    0
      #define dBOMB           6  //  011    0
      #define              8  //  100    0
      enum {dt_pass=0, dt_lock, dt_shut, dt_boss, dt_olck, dt_osht, dt_obos, dt_wall, dt_bomb, dt_walk, dt_max};
      */
    
    if(!even_walls&&(door==dWALL||door==dWALK))
    {
        return;
    }
    
    int doortype;
    
    switch(door)
    {
    case dWALL:
        doortype=dt_wall;
        break;
        
    case dWALK:
        doortype=dt_walk;
        break;
        
    case dOPEN:
        doortype=dt_pass;
        break;
        
    case dLOCKED:
        doortype=dt_lock;
        break;
        
    case dUNLOCKED:
        doortype=dt_olck;
        break;
        
    case dSHUTTER:
        if(screenscrolling && ((LinkDir()^1)==side))
        {
            doortype=dt_osht;
            opendoors=-4;
            break;
        }
        
        //fallthrough
    case d1WAYSHUTTER:
        doortype=dt_shut;
        break;
        
    case dOPENSHUTTER:
        doortype=dt_osht;
        break;
        
    case dBOSS:
        doortype=dt_boss;
        break;
        
    case dOPENBOSS:
        doortype=dt_obos;
        break;
        
    case dBOMBED:
        doortype=dt_bomb;
        break;
        
    default:
        return;
    }
    
    switch(side)
    {
    case up:
        switch(door)
        {
        case dBOMBED:
            if(redraw)
            {
                over_door(dest,t,39,side,0,0);
            }
            
        default:
            put_door(dest,t,7,side,doortype,redraw, even_walls);
            break;
        }
        
        break;
        
    case down:
        switch(door)
        {
        case dBOMBED:
            if(redraw)
            {
                over_door(dest,t,135,side,0,0);
            }
            
        default:
            put_door(dest,t,151,side,doortype,redraw, even_walls);
            break;
        }
        
        break;
        
    case left:
        switch(door)
        {
        case dBOMBED:
            if(redraw)
            {
                over_door(dest,t,66,side,0,0);
            }
            
        default:
            put_door(dest,t,64,side,doortype,redraw, even_walls);
            break;
        }
        
        break;
        
    case right:
        switch(door)
        {
        case dBOMBED:
            if(redraw)
            {
                over_door(dest,t,77,side,0,0);
            }
            
        default:
            put_door(dest,t,78,side,doortype,redraw, even_walls);
            break;
        }
        
        break;
    }
}

void putcombo_not_zero(BITMAP *dest, int x, int y, int combo, int cset)
{
    if(combo!=0)
    {
        putcombo(dest,x, y, combo, cset);
    }
}

void overcombo_not_zero(BITMAP *dest, int x, int y, int combo, int cset)
{
    if(combo!=0)
    {
        overcombo(dest,x, y, combo, cset);
    }
}

void showbombeddoor(BITMAP *dest, int side)
{
    int d=tmpscr->door_combo_set;
    
    switch(side)
    {
    case up:
        putcombo_not_zero(dest,(7&15)<<4,(7&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_u[dt_bomb][0],
                          DoorComboSets[d].doorcset_u[dt_bomb][0]);
        putcombo_not_zero(dest,(8&15)<<4,(8&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_u[dt_bomb][1],
                          DoorComboSets[d].doorcset_u[dt_bomb][1]);
        putcombo_not_zero(dest,(23&15)<<4,(23&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_u[dt_bomb][2],
                          DoorComboSets[d].doorcset_u[dt_bomb][2]);
        putcombo_not_zero(dest,(24&15)<<4,(24&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_u[dt_bomb][3],
                          DoorComboSets[d].doorcset_u[dt_bomb][3]);
        overcombo_not_zero(dest,(39&15)<<4,(39&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_u[0],
                           DoorComboSets[d].bombdoorcset_u[0]);
        overcombo_not_zero(dest,(40&15)<<4,(40&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_u[1],
                           DoorComboSets[d].bombdoorcset_u[1]);
        break;
        
    case down:
        putcombo_not_zero(dest,(151&15)<<4,(151&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_d[dt_bomb][0],
                          DoorComboSets[d].doorcset_d[dt_bomb][0]);
        putcombo_not_zero(dest,(152&15)<<4,(152&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_d[dt_bomb][1],
                          DoorComboSets[d].doorcset_d[dt_bomb][1]);
        putcombo_not_zero(dest,(167&15)<<4,(167&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_d[dt_bomb][2],
                          DoorComboSets[d].doorcset_d[dt_bomb][2]);
        putcombo_not_zero(dest,(168&15)<<4,(168&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_d[dt_bomb][3],
                          DoorComboSets[d].doorcset_d[dt_bomb][3]);
        overcombo_not_zero(dest,(135&15)<<4,(135&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_d[0],
                           DoorComboSets[d].bombdoorcset_d[0]);
        overcombo_not_zero(dest,(136&15)<<4,(136&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_d[1],
                           DoorComboSets[d].bombdoorcset_d[1]);
        break;
        
    case left:
        putcombo_not_zero(dest,(64&15)<<4,(64&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_l[dt_bomb][0],
                          DoorComboSets[d].doorcset_l[dt_bomb][0]);
        putcombo_not_zero(dest,(65&15)<<4,(65&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_l[dt_bomb][1],
                          DoorComboSets[d].doorcset_l[dt_bomb][1]);
        putcombo_not_zero(dest,(80&15)<<4,(80&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_l[dt_bomb][2],
                          DoorComboSets[d].doorcset_l[dt_bomb][2]);
        putcombo_not_zero(dest,(81&15)<<4,(81&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_l[dt_bomb][3],
                          DoorComboSets[d].doorcset_l[dt_bomb][3]);
        putcombo_not_zero(dest,(96&15)<<4,(96&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_l[dt_bomb][4],
                          DoorComboSets[d].doorcset_l[dt_bomb][4]);
        putcombo_not_zero(dest,(97&15)<<4,(97&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_l[dt_bomb][5],
                          DoorComboSets[d].doorcset_l[dt_bomb][5]);
        overcombo_not_zero(dest,(66&15)<<4,(66&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_l[0],
                           DoorComboSets[d].bombdoorcset_l[0]);
        overcombo_not_zero(dest,(82&15)<<4,(82&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_l[1],
                           DoorComboSets[d].bombdoorcset_l[1]);
        overcombo_not_zero(dest,(98&15)<<4,(98&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_l[2],
                           DoorComboSets[d].bombdoorcset_l[2]);
        break;
        
    case right:
        putcombo_not_zero(dest,(78&15)<<4,(78&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_r[dt_bomb][0],
                          DoorComboSets[d].doorcset_r[dt_bomb][0]);
        putcombo_not_zero(dest,(79&15)<<4,(79&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_r[dt_bomb][1],
                          DoorComboSets[d].doorcset_r[dt_bomb][1]);
        putcombo_not_zero(dest,(94&15)<<4,(94&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_r[dt_bomb][2],
                          DoorComboSets[d].doorcset_r[dt_bomb][2]);
        putcombo_not_zero(dest,(95&15)<<4,(95&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_r[dt_bomb][3],
                          DoorComboSets[d].doorcset_r[dt_bomb][3]);
        putcombo_not_zero(dest,(110&15)<<4,(110&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_r[dt_bomb][4],
                          DoorComboSets[d].doorcset_r[dt_bomb][4]);
        putcombo_not_zero(dest,(111&15)<<4,(111&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_r[dt_bomb][5],
                          DoorComboSets[d].doorcset_r[dt_bomb][5]);
        overcombo_not_zero(dest,(77&15)<<4,(77&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_r[0],
                           DoorComboSets[d].bombdoorcset_r[0]);
        overcombo_not_zero(dest,(93&15)<<4,(93&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_r[1],
                           DoorComboSets[d].bombdoorcset_r[1]);
        overcombo_not_zero(dest,(109&15)<<4,(109&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_r[2],
                           DoorComboSets[d].bombdoorcset_r[2]);
        break;
    }
}

void openshutters()
{
    for(int i=0; i<4; i++)
        if(tmpscr->door[i]==dSHUTTER)
        {
            putdoor(scrollbuf,0,i,dOPENSHUTTER);
            tmpscr->door[i]=dOPENSHUTTER;
        }
        
    sfx(WAV_DOOR,128);
}

void loadscr(int tmp,int destdmap, int scr,int ldir,bool overlay=false)
{
	
    
    //  introclk=intropos=msgclk=msgpos=dmapmsgclk=0;
    for(word x=0; x<animated_combos; x++)
    {
        if(combobuf[animated_combo_table4[x][0]].nextcombo!=0)
        {
			combobuf[animated_combo_table4[x][0]].aclk = 0;
        }
    }
    
    for(word x=0; x<animated_combos2; x++)
    {
        if(combobuf[animated_combo_table24[x][0]].nextcombo!=0)
        {
			combobuf[animated_combo_table24[x][0]].aclk = 0;
        }
    }
    
    reset_combo_animations2();
    
    
    mapscr ffscr = tmpscr[tmp];
    tmpscr[tmp] = TheMaps[currmap*MAPSCRS+scr];
    
    
    const int _mapsSize = ZCMaps[currmap].tileHeight*ZCMaps[currmap].tileWidth;
    tmpscr[tmp].data = TheMaps[currmap*MAPSCRS+scr].data;
    tmpscr[tmp].sflag = TheMaps[currmap*MAPSCRS+scr].sflag;
    tmpscr[tmp].cset = TheMaps[currmap*MAPSCRS+scr].cset;
    
    //screen / screendata script
    FFCore.clear_screen_stack();
    screenScriptData.Clear();
    FFCore.deallocateAllArrays(SCRIPT_SCREEN, 0);
    FFCore.deallocateAllArrays(SCRIPT_COMBO, 0);
    //reset combo script doscripts
    //Init combo scripts
    FFCore.init_combo_doscript();
    if ( TheMaps[currmap*MAPSCRS+scr].script > 0 )
    {
	    tmpscr[tmp].script = TheMaps[currmap*MAPSCRS+scr].script;
	    al_trace("The screen script id is: %d \n", TheMaps[currmap*MAPSCRS+scr].script);
	    //if ( !tmpscr[tmp].screendatascriptInitialised )
	    //{
		    for ( int q = 0; q < 8; q++ )
		    {
			tmpscr[tmp].screeninitd[q] = TheMaps[currmap*MAPSCRS+scr].screeninitd[q];
		    }
	    //}
	tmpscr[tmp].screendatascriptInitialised = 0;
	tmpscr[tmp].doscript = 1;
    }
    else
    {
	tmpscr[tmp].script = 0;
	tmpscr[tmp].screendatascriptInitialised = 0;
	tmpscr[tmp].doscript = 0;
    }
    
    
    tmpscr[tmp].data.resize(_mapsSize, 0);
    tmpscr[tmp].sflag.resize(_mapsSize, 0);
    tmpscr[tmp].cset.resize(_mapsSize, 0);
    
    if(overlay)
    {
        for(int c=0; c< ZCMaps[currmap].tileHeight*ZCMaps[currmap].tileWidth; ++c)
        {
            if(tmpscr[tmp].data[c]==0)
            {
                tmpscr[tmp].data[c]=ffscr.data[c];
                tmpscr[tmp].sflag[c]=ffscr.sflag[c];
                tmpscr[tmp].cset[c]=ffscr.cset[c];
            }
        }
        
        for(int i=0; i<6; i++)
        {
            if(ffscr.layermap[i]>0 && tmpscr[tmp].layermap[i]>0)
            {
                int lm = (tmpscr[tmp].layermap[i]-1)*MAPSCRS+tmpscr[tmp].layerscreen[i];
                int fm = (ffscr.layermap[i]-1)*MAPSCRS+ffscr.layerscreen[i];
                
                if(!TheMaps[lm].data.empty() && !TheMaps[fm].data.empty())
                {
                    for(int c=0; c< ZCMaps[currmap].tileHeight*ZCMaps[currmap].tileWidth; ++c)
                    {
                        if(TheMaps[lm].data[c]==0)
                        {
                            TheMaps[lm].data[c] = TheMaps[fm].data[c];
                            TheMaps[lm].sflag[c] = TheMaps[fm].sflag[c];
                            TheMaps[lm].cset[c] = TheMaps[fm].cset[c];
                        }
                    }
                }
            }
        }
    }
    
    if(tmp==0)
    {
        // Before loading new FFCs, deallocate the arrays used by those that aren't carrying over
	
	for(int ffid = 0; ffid < 32; ++ffid)
	{
		if(!(ffscr.flags5&fNOFFCARRYOVER) && (ffscr.ffflags[ffid]&ffCARRYOVER)) continue;
		FFCore.deallocateAllArrays(SCRIPT_FFC, ffid, false); //false means this does not require 'qr_ALWAYS_DEALLOCATE_ARRAYS' to be checked. -V
	}
	FFCore.deallocateAllArrays(SCRIPT_SCREEN, 0);
        
        for(int i = 0; i < 32; i++)
        {
            // If these aren't reset, changers may not work right
            ffposx[i]=-1000;
            ffposy[i]=-1000;
            ffprvx[i]=-10000000;
            ffprvy[i]=-10000000;
            
            if((ffscr.ffflags[i]&ffCARRYOVER) && !(ffscr.flags5&fNOFFCARRYOVER))
            {
                tmpscr[tmp].ffdata[i] = ffscr.ffdata[i];
                tmpscr[tmp].ffx[i] = ffscr.ffx[i];
                tmpscr[tmp].ffy[i] = ffscr.ffy[i];
                tmpscr[tmp].ffxdelta[i] = ffscr.ffxdelta[i];
                tmpscr[tmp].ffydelta[i] = ffscr.ffydelta[i];
                tmpscr[tmp].ffxdelta2[i] = ffscr.ffxdelta2[i];
                tmpscr[tmp].ffydelta2[i] = ffscr.ffydelta2[i];
                tmpscr[tmp].fflink[i] = ffscr.fflink[i];
                tmpscr[tmp].ffdelay[i] = ffscr.ffdelay[i];
                tmpscr[tmp].ffcset[i] = ffscr.ffcset[i];
                tmpscr[tmp].ffwidth[i] = ffscr.ffwidth[i];
                tmpscr[tmp].ffheight[i] = ffscr.ffheight[i];
                tmpscr[tmp].ffflags[i] = ffscr.ffflags[i];
                tmpscr[tmp].ffscript[i] = ffscr.ffscript[i];
                
                for(int j=0; j<2; ++j)
                {
                    tmpscr[tmp].inita[i][j] = ffscr.inita[i][j];
                }
                
                for(int j=0; j<8; ++j)
                {
                    tmpscr[tmp].initd[i][j] = ffscr.initd[i][j];
                }
                
                if(!(ffscr.ffflags[i]&ffSCRIPTRESET))
                {
                    tmpscr[tmp].ffscript[i] = ffscr.ffscript[i]; // Restart script if it has halted.
                    tmpscr[tmp].initialized[i] = ffscr.initialized[i];
                }
                else
                {
                    tmpscr[tmp].initialized[i] = false;
                    
                    ffcScriptData[i].pc = 0;
                    ffcScriptData[i].sp = 0;
                    ffcScriptData[i].ffcref = 0;
                }
            }
            else
            {
                memset(ffmisc[i], 0, 16 * sizeof(long));
                ffcScriptData[i].Clear();
                clear_ffc_stack(i);
            }
        }
    }
    
    
    if(tmp==0)
    {
        for(int i=0; i<6; i++)
        {
            mapscr layerscr = tmpscr2[i];
            
            // Don't delete the old tmpscr2's data yet!
            if(tmpscr[tmp].layermap[i]>0 && (ZCMaps[tmpscr[tmp].layermap[i]-1].tileWidth==ZCMaps[currmap].tileWidth)
                    && (ZCMaps[tmpscr[tmp].layermap[i]-1].tileHeight==ZCMaps[currmap].tileHeight))
            {
                // const int _mapsSize = (ZCMaps[currmap].tileWidth)*(ZCMaps[currmap].tileHeight);
                
                tmpscr2[i]=TheMaps[(tmpscr[tmp].layermap[i]-1)*MAPSCRS+tmpscr[tmp].layerscreen[i]];
                
                tmpscr2[i].data.resize(_mapsSize, 0);
                tmpscr2[i].sflag.resize(_mapsSize, 0);
                tmpscr2[i].cset.resize(_mapsSize, 0);
                
                if(overlay)
                {
                    for(int y=0; y<ZCMaps[currmap].tileHeight; ++y)
                    {
                        for(int x=0; x<ZCMaps[currmap].tileWidth; ++x)
                        {
                            int c=y*ZCMaps[currmap].tileWidth+x;
                            
                            if(tmpscr2[i].data[c]==0)
                            {
                                tmpscr2[i].data[c]=layerscr.data[c];
                                tmpscr2[i].sflag[c]=layerscr.sflag[c];
                                tmpscr2[i].cset[c]=layerscr.cset[c];
                            }
                        }
                    }
                }
            }
            else
            {
                (tmpscr2+i)->zero_memory();
            }
        }
    }
    
    if(canPermSecret(destdmap,scr)/*||TheMaps[(currmap*MAPSCRS)+currscr].flags6&fTRIGGERFPERM*/)
    {
        if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mSECRET)               // if special stuff done before
        {
            hiddenstair(tmp,false);
            hidden_entrance(tmp,false,false,-3);
        }
    }
    
    
    if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mLOCKBLOCK)              // if special stuff done before
    {
        remove_lockblocks(tmp);
    }
    
    if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mBOSSLOCKBLOCK)          // if special stuff done before
    {
        remove_bosslockblocks(tmp);
    }
    
    if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mCHEST)              // if special stuff done before
    {
        remove_chests(tmp);
    }
    
    if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mLOCKEDCHEST)              // if special stuff done before
    {
        remove_lockedchests(tmp);
    }
    
    if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mBOSSCHEST)              // if special stuff done before
    {
        remove_bosschests(tmp);
    }
    
    // check doors
    if(isdungeon(destdmap,scr))
    {
        for(int i=0; i<4; i++)
        {
            int door=tmpscr[tmp].door[i];
            bool putit=true;
            
            switch(door)
            {
            case d1WAYSHUTTER:
            case dSHUTTER:
                if((ldir^1)==i)
                {
                    tmpscr[tmp].door[i]=dOPENSHUTTER;
                    //          putit=false;
                }
                
                opendoors = -4;
                break;
                
            case dLOCKED:
                if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&(1<<i))
                {
                    tmpscr[tmp].door[i]=dUNLOCKED;
                    //          putit=false;
                }
                
                break;
                
            case dBOSS:
                if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&(1<<i))
                {
                    tmpscr[tmp].door[i]=dOPENBOSS;
                    //          putit=false;
                }
                
                break;
                
            case dBOMB:
                if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&(1<<i))
                {
                    tmpscr[tmp].door[i]=dBOMBED;
                }
                
                break;
            }
            
            if(putit)
            {
                putdoor(scrollbuf,tmp,i,tmpscr[tmp].door[i],false);
            }
            
            if(door==dSHUTTER||door==d1WAYSHUTTER)
            {
                tmpscr[tmp].door[i]=door;
            }
        }
    }
    
    
    for(int j=-1; j<6; ++j)  // j == -1 denotes the current screen
    {
        if(j<0 || ((tmpscr[tmp].layermap[j]>0)&&(ZCMaps[tmpscr[tmp].layermap[j]-1].tileWidth==ZCMaps[currmap].tileWidth) && (ZCMaps[tmpscr[tmp].layermap[j]-1].tileHeight==ZCMaps[currmap].tileHeight)))
        {
            mapscr *layerscreen= (j<0 ? &tmpscr[tmp] : !tmpscr2[j].data.empty() ? &tmpscr2[j] :
                                  &TheMaps[(tmpscr[tmp].layermap[j]-1)*MAPSCRS]+tmpscr[tmp].layerscreen[j]);
                                  
            for(int i=0; i<(ZCMaps[currmap].tileWidth)*(ZCMaps[currmap].tileHeight); ++i)
            {
                int c=layerscreen->data[i];
                int cs=layerscreen->cset[i];
                
                // New screen flag: Cycle Combos At Screen Init
                if(combobuf[c].nextcombo != 0 && (tmpscr[tmp].flags3 & fCYCLEONINIT) && (j<0 || get_bit(quest_rules,qr_CMBCYCLELAYERS)))
                {
                    int r = 0;
                    
                    while(combobuf[c].nextcombo != 0 && r++ < 10)
                    {
                        layerscreen->data[i] = combobuf[c].nextcombo;
                        layerscreen->cset[i] = combobuf[c].nextcset;
                        c=layerscreen->data[i];
                        cs=layerscreen->cset[i];
                    }
                }
            }
        }
    }
}

// Screen is being viewed by the Overworld Map viewer.
void loadscr2(int tmp,int scr,int)
{
    for(word x=0; x<animated_combos; x++)
    {
        if(combobuf[animated_combo_table4[x][0]].nextcombo!=0)
        {
			combobuf[animated_combo_table4[x][0]].aclk=0;
        }
    }
    
    const int _mapsSize = (ZCMaps[currmap].tileWidth)*(ZCMaps[currmap].tileHeight);
    
    tmpscr[tmp] = TheMaps[currmap*MAPSCRS+scr];
    
    tmpscr[tmp].data.resize(_mapsSize, 0);
    tmpscr[tmp].sflag.resize(_mapsSize, 0);
    tmpscr[tmp].cset.resize(_mapsSize, 0);
    
    if(tmp==0)
    {
        for(int i=0; i<6; i++)
        {
            if(tmpscr[tmp].layermap[i]>0)
            {
            
                if((ZCMaps[tmpscr[tmp].layermap[i]-1].tileWidth==ZCMaps[currmap].tileWidth) && (ZCMaps[tmpscr[tmp].layermap[i]-1].tileHeight==ZCMaps[currmap].tileHeight))
                {
                    tmpscr2[i]=TheMaps[(tmpscr[tmp].layermap[i]-1)*MAPSCRS+tmpscr[tmp].layerscreen[i]];
                    
                    tmpscr2[i].data.resize(_mapsSize, 0);
                    tmpscr2[i].sflag.resize(_mapsSize, 0);
                    tmpscr2[i].cset.resize(_mapsSize, 0);
                }
                else
                {
                    (tmpscr2+i)->zero_memory();
                }
            }
            else
            {
                (tmpscr2+i)->zero_memory();
            }
        }
    }
    
    if(canPermSecret(-1,scr))
    {
        if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mSECRET)               // if special stuff done before
        {
            hiddenstair(tmp,false);
            hidden_entrance(tmp,false,false,-3);
        }
    }
    
    if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mLOCKBLOCK)              // if special stuff done before
    {
        remove_lockblocks(tmp);
    }
    
    if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mBOSSLOCKBLOCK)          // if special stuff done before
    {
        remove_bosslockblocks(tmp);
    }
    
    if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mCHEST)              // if special stuff done before
    {
        remove_chests(tmp);
    }
    
    if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mLOCKEDCHEST)              // if special stuff done before
    {
        remove_lockedchests(tmp);
    }
    
    if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mBOSSCHEST)              // if special stuff done before
    {
        remove_bosschests(tmp);
    }
    
    // check doors
    if(isdungeon(scr))
    {
        for(int i=0; i<4; i++)
        {
            int door=tmpscr[tmp].door[i];
            bool putit=true;
            
            switch(door)
            {
            case d1WAYSHUTTER:
            case dSHUTTER:
                /*
                        if((ldir^1)==i)
                        {
                          tmpscr[tmp].door[i]=dOPENSHUTTER;
                          //          putit=false;
                        }
                */
                break;
                
            case dLOCKED:
                if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&(1<<i))
                {
                    tmpscr[tmp].door[i]=dUNLOCKED;
                    //          putit=false;
                }
                
                break;
                
            case dBOSS:
                if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&(1<<i))
                {
                    tmpscr[tmp].door[i]=dOPENBOSS;
                    //          putit=false;
                }
                
                break;
                
            case dBOMB:
                if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&(1<<i))
                {
                    tmpscr[tmp].door[i]=dBOMBED;
                }
                
                break;
            }
            
            if(putit)
            {
                putdoor(scrollbuf,tmp,i,tmpscr[tmp].door[i],false);
            }
            
            if(door==dSHUTTER||door==d1WAYSHUTTER)
            {
                tmpscr[tmp].door[i]=door;
            }
        }
    }
    
    for(int j=-1; j<6; ++j)  // j == -1 denotes the current screen
    {
        if(j<0 || ((tmpscr[tmp].layermap[j]>0)&&(ZCMaps[tmpscr[tmp].layermap[j]-1].tileWidth==ZCMaps[currmap].tileWidth) && (ZCMaps[tmpscr[tmp].layermap[j]-1].tileHeight==ZCMaps[currmap].tileHeight)))
        {
            mapscr *layerscreen= (j<0 ? &tmpscr[tmp]
                                  : &(TheMaps[(tmpscr[tmp].layermap[j]-1)*MAPSCRS+tmpscr[tmp].layerscreen[j]]));
                                  
            for(int i=0; i<(ZCMaps[currmap].tileWidth)*(ZCMaps[currmap].tileHeight); ++i)
            {
                int c=layerscreen->data[i];
                int cs=layerscreen->cset[i];
                
                // New screen flag: Cycle Combos At Screen Init
                if((tmpscr[tmp].flags3 & fCYCLEONINIT) && (j<0 || get_bit(quest_rules,qr_CMBCYCLELAYERS)))
                {
                    int r = 0;
                    
                    while(combobuf[c].nextcombo != 0 && r++ < 10)
                    {
                        layerscreen->data[i] = combobuf[c].nextcombo;
                        layerscreen->cset[i] = combobuf[c].nextcset;
                        c=layerscreen->data[i];
                        cs=layerscreen->cset[i];
                    }
                }
            }
        }
    }
    
}

void putscr(BITMAP* dest,int x,int y, mapscr* scrn)
{
    if(scrn->valid==0||!show_layer_0||scrn->hidelayers & 1)
    {
        rectfill(dest,x,y,x+255,y+175,0);
        return;
    }
	
	if(scrn->flags7&fLAYER2BG||scrn->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER2BG || DMaps[currdmap].flags&dmfLAYER3BG)
	{
		for(int i=0; i<176; ++i)
		{
            overcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,scrn->data[i],scrn->cset[i]);
		}
	}
	else
	{
		for(int i=0; i<176; ++i)
		{
            putcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,scrn->data[i],scrn->cset[i]);
        }
    }
}

void putscrdoors(BITMAP *dest,int x,int y, mapscr* scrn)
{
    if(scrn->valid==0||!show_layer_0)
    {
        return;
    }
    
    if(scrn->door[0]==dBOMBED)
    {
        over_door(dest,0,39,up,x,y);
    }
    
    if(scrn->door[1]==dBOMBED)
    {
        over_door(dest,0,135,down,x,y);
    }
    
    if(scrn->door[2]==dBOMBED)
    {
        over_door(dest,0,66,left,x,y);
    }
    
    if(scrn->door[3]==dBOMBED)
    {
        over_door(dest,0,77,right,x,y);
    }
}

bool _walkflag(int x,int y,int cnt)
{
    //  walkflagx=x; walkflagy=y;
    if(get_bit(quest_rules,qr_LTTPWALK))
    {
        if(x<0||y<0) return false;
        
        if(x>255) return false;
        
        if(x>247&&cnt==2) return false;
        
        if(y>175) return false;
    }
    else
    {
        if(x<0||y<0) return false;
        
        if(x>248) return false;
        
        if(x>240&&cnt==2) return false;
        
        if(y>168) return false;
    }
    
    mapscr *s1, *s2;
    s1=(((*tmpscr).layermap[0]-1)>=0)?tmpscr2:tmpscr;
    s2=(((*tmpscr).layermap[1]-1)>=0)?tmpscr2+1:tmpscr;
    //  s2=TheMaps+((*tmpscr).layermap[1]-1)MAPSCRS+((*tmpscr).layerscreen[1]);
    
    int bx=(x>>4)+(y&0xF0);
    newcombo c = combobuf[tmpscr->data[bx]];
    newcombo c1 = combobuf[s1->data[bx]];
    newcombo c2 = combobuf[s2->data[bx]];
    bool dried = (((iswater_type(c.type)) || (iswater_type(c1.type)) ||
                   (iswater_type(c2.type))) && DRIEDLAKE);
    int b=1;
    
    if(x&8) b<<=2;
    
    if(y&8) b<<=1;
    
    int cwalkflag = c.walk;
    if (c.type == cBRIDGE || (iswater_type(c.type) && ((c.usrflags&cflag3) || (c.usrflags&cflag4)))) cwalkflag = 0;
    if (((*tmpscr).layermap[0]-1)>=0)
    {
	    if (c1.type == cBRIDGE || (iswater_type(c1.type) && get_bit(quest_rules,  qr_WATER_ON_LAYER_1) && !((c1.usrflags&cflag3) || (c1.usrflags&cflag4)))) cwalkflag &= c1.walk;
	    else if ((iswater_type(c1.type) && get_bit(quest_rules,  qr_WATER_ON_LAYER_1) && ((c1.usrflags&cflag3) || (c1.usrflags&cflag4)))) cwalkflag = 0;
	    else cwalkflag |= c1.walk;
    }
    if (((*tmpscr).layermap[1]-1)>=0)
    {
	    if (c2.type == cBRIDGE || (iswater_type(c2.type) && get_bit(quest_rules,  qr_WATER_ON_LAYER_2) && !((c2.usrflags&cflag3) || (c2.usrflags&cflag4)))) cwalkflag &= c2.walk;
	    else if ((iswater_type(c2.type) && get_bit(quest_rules,  qr_WATER_ON_LAYER_2) && ((c2.usrflags&cflag3) || (c2.usrflags&cflag4)))) cwalkflag = 0;
	    else cwalkflag |= c2.walk;
    }
    
    if((cwalkflag&b) && !dried)
        return true;
        
    if(cnt==1) return false;
    
    ++bx;
    
    if(!(x&8))
        b<<=2;
    else
    {
        c  = combobuf[tmpscr->data[bx]];
        c1 = combobuf[s1->data[bx]];
        c2 = combobuf[s2->data[bx]];
        dried = (((iswater_type(c.type)) || (iswater_type(c1.type)) ||
                  (iswater_type(c2.type))) && DRIEDLAKE);
        b=1;
        
        if(y&8) b<<=1;
    }
    cwalkflag = c.walk;
    if (c.type == cBRIDGE || (iswater_type(c.type) && ((c.usrflags&cflag3) || (c.usrflags&cflag4)))) cwalkflag = 0;
    if (((*tmpscr).layermap[0]-1)>=0)
    {
	    if (c1.type == cBRIDGE || (iswater_type(c1.type) && get_bit(quest_rules,  qr_WATER_ON_LAYER_1) && !((c1.usrflags&cflag3) || (c1.usrflags&cflag4)))) cwalkflag &= c1.walk;
	    else if ((iswater_type(c1.type) && get_bit(quest_rules,  qr_WATER_ON_LAYER_1) && ((c1.usrflags&cflag3) || (c1.usrflags&cflag4)))) cwalkflag = 0;
	    else cwalkflag |= c1.walk;
    }
    if (((*tmpscr).layermap[1]-1)>=0)
    {
	    if (c2.type == cBRIDGE || (iswater_type(c2.type) && get_bit(quest_rules,  qr_WATER_ON_LAYER_2) && !((c2.usrflags&cflag3) || (c2.usrflags&cflag4)))) cwalkflag &= c2.walk;
	    else if ((iswater_type(c2.type) && get_bit(quest_rules,  qr_WATER_ON_LAYER_2) && ((c2.usrflags&cflag3) || (c2.usrflags&cflag4)))) cwalkflag = 0;
	    else cwalkflag |= c2.walk;
    }
    return (cwalkflag&b) ? !dried : false;
}

//used by mapdata->isSolid(x,y) in ZScript:
bool _walkflag(int x,int y,int cnt, mapscr* m)
{
    //  walkflagx=x; walkflagy=y;
    if(get_bit(quest_rules,qr_LTTPWALK))
    {
        if(x<0||y<0) return false;
        
        if(x>255) return false;
        
        if(x>247&&cnt==2) return false;
        
        if(y>175) return false;
    }
    else
    {
        if(x<0||y<0) return false;
        
        if(x>248) return false;
        
        if(x>240&&cnt==2) return false;
        
        if(y>168) return false;
    }
    
    mapscr *s1, *s2;
    
    if ( m->layermap[0] > 0 )
    {
	    s1 = &TheMaps[(m->layermap[0]*MAPSCRS + m->layerscreen[0])];
    }
    else s1 = m;
    
    if ( m->layermap[1] > 0 )
    {
	    s2 = &TheMaps[(m->layermap[1]*MAPSCRS + m->layerscreen[1])];
    }
    else s2 = m;
    
    int bx=(x>>4)+(y&0xF0);
    newcombo c = combobuf[m->data[bx]];
    newcombo c1 = combobuf[s1->data[bx]];
    newcombo c2 = combobuf[s2->data[bx]];
    bool dried = (((iswater_type(c.type)) || (iswater_type(c1.type)) ||
                   (iswater_type(c2.type))) && DRIEDLAKE);
    int b=1;
    
    if(x&8) b<<=2;
    
    if(y&8) b<<=1;
    
    int cwalkflag = c.walk;
    if (c1.type == cBRIDGE) cwalkflag &= c1.walk;
    else if (((*tmpscr).layermap[0]-1)>=0) cwalkflag |= c1.walk;
    if (c2.type == cBRIDGE) cwalkflag &= c2.walk;
    else if (((*tmpscr).layermap[1]-1)>=0) cwalkflag |= c2.walk;
    
    if((cwalkflag&b) && !dried)
        return true;
        
    if(cnt==1) return false;
    
    ++bx;
    
    if(!(x&8))
        b<<=2;
    else
    {
        c  = combobuf[m->data[bx]];
        c1 = combobuf[s1->data[bx]];
        c2 = combobuf[s2->data[bx]];
        dried = (((iswater_type(c.type)) || (iswater_type(c1.type)) ||
                  (iswater_type(c2.type))) && DRIEDLAKE);
        b=1;
        
        if(y&8) b<<=1;
    }
    
    cwalkflag = c.walk;
    if (c1.type == cBRIDGE) cwalkflag &= c1.walk;
    else if (((*tmpscr).layermap[0]-1)>=0) cwalkflag |= c1.walk;
    if (c2.type == cBRIDGE) cwalkflag &= c2.walk;
    else if (((*tmpscr).layermap[1]-1)>=0) cwalkflag |= c2.walk;
    return (cwalkflag&b) ? !dried : false;
}

bool _walkflag(int x,int y,int cnt, mapscr* m, mapscr* s1, mapscr* s2)
{
    //  walkflagx=x; walkflagy=y;
    if(get_bit(quest_rules,qr_LTTPWALK))
    {
        if(x<0||y<0) return false;
        
        if(x>255) return false;
        
        if(x>247&&cnt==2) return false;
        
        if(y>175) return false;
    }
    else
    {
        if(x<0||y<0) return false;
        
        if(x>248) return false;
        
        if(x>240&&cnt==2) return false;
        
        if(y>168) return false;
    }
    
    if(!s1) s1 = m;
	if(!s2) s2 = m;
    
    int bx=(x>>4)+(y&0xF0);
    newcombo c = combobuf[m->data[bx]];
    newcombo c1 = combobuf[s1->data[bx]];
    newcombo c2 = combobuf[s2->data[bx]];
    bool dried = (((iswater_type(c.type)) || (iswater_type(c1.type)) ||
                   (iswater_type(c2.type))) && DRIEDLAKE);
    int b=1;
    
    if(x&8) b<<=2;
    
    if(y&8) b<<=1;
    
    int cwalkflag = c.walk;
    if (c1.type == cBRIDGE) cwalkflag &= c1.walk;
    else if (((*tmpscr).layermap[0]-1)>=0) cwalkflag |= c1.walk;
    if (c2.type == cBRIDGE) cwalkflag &= c2.walk;
    else if (((*tmpscr).layermap[1]-1)>=0) cwalkflag |= c2.walk;
    
    if((cwalkflag&b) && !dried)
        return true;
        
    if(cnt==1) return false;
    
    ++bx;
    
    if(!(x&8))
        b<<=2;
    else
    {
        c  = combobuf[m->data[bx]];
        c1 = combobuf[s1->data[bx]];
        c2 = combobuf[s2->data[bx]];
        dried = (((iswater_type(c.type)) || (iswater_type(c1.type)) ||
                  (iswater_type(c2.type))) && DRIEDLAKE);
        b=1;
        
        if(y&8) b<<=1;
    }
    
    cwalkflag = c.walk;
    if (c1.type == cBRIDGE) cwalkflag &= c1.walk;
    else if (((*tmpscr).layermap[0]-1)>=0) cwalkflag |= c1.walk;
    if (c2.type == cBRIDGE) cwalkflag &= c2.walk;
    else if (((*tmpscr).layermap[1]-1)>=0) cwalkflag |= c2.walk;
    return (cwalkflag&b) ? !dried : false;
}

//Only check the given mapscr*, not it's layer 1&2
bool _walkflag_layer(int x,int y,int cnt, mapscr* m)
{
    //  walkflagx=x; walkflagy=y;
    if(get_bit(quest_rules,qr_LTTPWALK))
    {
        if(x<0||y<0) return false;
        
        if(x>255) return false;
        
        if(x>247&&cnt==2) return false;
        
        if(y>175) return false;
    }
    else
    {
        if(x<0||y<0) return false;
        
        if(x>248) return false;
        
        if(x>240&&cnt==2) return false;
        
        if(y>168) return false;
    }
	if(!m) return true;
    
    int bx=(x>>4)+(y&0xF0);
    newcombo c = combobuf[m->data[bx]];
    bool dried = ((iswater_type(c.type)) && DRIEDLAKE);
    int b=1;
    
    if(x&8) b<<=2;
    
    if(y&8) b<<=1;
    
    if((c.walk&b) && !dried)
        return true;
        
    if(cnt==1) return false;
    
    ++bx;
    
    if(!(x&8))
        b<<=2;
    else
    {
        c  = combobuf[m->data[bx]];
        dried = ((iswater_type(c.type)) && DRIEDLAKE);
        b=1;
        
        if(y&8) b<<=1;
    }
    
    return (c.walk&b) ? !dried : false;
}

bool water_walkflag(int x,int y,int cnt)
{
    if(get_bit(quest_rules,qr_LTTPWALK))
    {
        if(x<0||y<0) return false;
        
        if(x>255) return false;
        
        if(x>247&&cnt==2) return false;
        
        if(y>175) return false;
    }
    else
    {
        if(x<0||y<0) return false;
        
        if(x>248) return false;
        
        if(x>240&&cnt==2) return false;
        
        if(y>168) return false;
    }
    
    mapscr *s1, *s2;
    /*
      s1=(((*tmpscr).layermap[0]-1)>=0)?
      (TheMaps+((*tmpscr).layermap[0]-1)*MAPSCRS+((*tmpscr).layerscreen[0])):
      tmpscr;
      s2=(((*tmpscr).layermap[1]-1)>=0)?
      (TheMaps+((*tmpscr).layermap[1]-1)*MAPSCRS+((*tmpscr).layerscreen[1])):
      tmpscr;
      */
    s1=(((*tmpscr).layermap[0]-1)>=0)?tmpscr2:tmpscr;
    s2=(((*tmpscr).layermap[1]-1)>=0)?tmpscr2+1:tmpscr;
    
    int bx=(x>>4)+(y&0xF0);
    newcombo c = combobuf[tmpscr->data[bx]];
    newcombo c1 = combobuf[s1->data[bx]];
    newcombo c2 = combobuf[s2->data[bx]];
    int b=1;
    
    if(x&8) b<<=2;
    
    if(y&8) b<<=1;
    
    if((c.walk&b) && !iswater_type(c.type))
        return true;
        
    if((c1.walk&b) && !iswater_type(c1.type))
        return true;
        
    if((c2.walk&b) && !iswater_type(c2.type))
        return true;
        
    if(cnt==1) return false;
    
    if(x&8)
        b<<=2;
    else
    {
        c = combobuf[tmpscr->data[++bx]];
        c1 = combobuf[s1->data[bx]];
        c2 = combobuf[s2->data[bx]];
        b=1;
        
        if(y&8) b<<=1;
    }
    
    return (c.walk&b) ? !iswater_type(c.type) :
           (c1.walk&b) ? !iswater_type(c1.type) :
           (c2.walk&b) ? !iswater_type(c2.type) :false;
}

bool hit_walkflag(int x,int y,int cnt)
{
    if(dlevel)
        if(x<32 || y<40 || (x+(cnt-1)*8)>=224 || y>=144)
            return true;
            
    if(blockpath && y<((get_bit(quest_rules,qr_LTTPCOLLISION))?80:88))
        return true;
        
    if(x<16 || y<16 || (x+(cnt-1)*8)>=240 || y>=160)
        return true;
        
    //  for(int i=0; i<4; i++)
    if(mblock2.clk && mblock2.hit(x,y,0,cnt*8,1,16))
        return true;
        
    return _walkflag(x,y,cnt);
}

void map_bkgsfx(bool on)
{
    if(on)
    {
        cont_sfx(tmpscr->oceansfx);
        
        if(tmpscr->bosssfx && !(game->lvlitems[dlevel]&liBOSS))
            cont_sfx(tmpscr->bosssfx);
    }
    else
    {
        adjust_sfx(tmpscr->oceansfx,128,false);
        adjust_sfx(tmpscr->bosssfx,128,false);
        
        for(int i=0; i<guys.Count(); i++)
        {
            if(((enemy*)guys.spr(i))->bgsfx)
                stop_sfx(((enemy*)guys.spr(i))->bgsfx);
        }
    }
}

/****  View Map  ****/

//BITMAP *mappic = NULL;
int mapres = 0;

bool displayOnMap(int x, int y)
{
    int s = (y<<4) + x;
    if(!(game->maps[(currmap*MAPSCRSNORMAL)+s]&mVISITED))
        return false;

    // Don't display if not part of DMap
    if(((DMaps[currdmap].flags&dmfDMAPMAP) &&
       (DMaps[currdmap].type != dmOVERW) &&
       !(x >= DMaps[currdmap].xoff &&
         x < DMaps[currdmap].xoff+8 &&
         DMaps[currdmap].grid[y]&(128>>(x-DMaps[currdmap].xoff)))))
        return false;
    else
        return true;
}

void ViewMap()
{
    mapscr tmpscr_b[2];
    mapscr tmpscr_c[6];
    
    for(int i=0; i<6; ++i)
    {
        tmpscr_c[i] = tmpscr2[i];
        tmpscr2[i].zero_memory();
        
        if(i>=2)
        {
            continue;
        }
        
        tmpscr_b[i] = tmpscr[i];
        tmpscr[i].zero_memory();
    }
    
    BITMAP* mappic = NULL;
    static double scales[17] =
    {
        0.03125, 0.04419, 0.0625, 0.08839, 0.125, 0.177, 0.25, 0.3535,
        0.50, 0.707, 1.0, 1.414, 2.0, 2.828, 4.0, 5.657, 8.0
    };
    
    int px = ((8-(currscr&15)) << 9)  - 256;
    int py = ((4-(currscr>>4)) * 352) - 176;
    int lx = ((currscr&15)<<8)  + LinkX()+8;
    int ly = ((currscr>>4)*176) + LinkY()+8;
    int sc = 6;
    
    bool done=false, redraw=true;
    
    mappic = create_bitmap_ex(8,(256*16)>>mapres,(176*8)>>mapres);
    
    if(!mappic)
    {
        system_pal();
        jwin_alert("View Map","Not enough memory.",NULL,NULL,"OK",NULL,13,27,lfont);
        game_pal();
        return;
    }
    
    // draw the map
    set_clip_rect(scrollbuf, 0, 0, scrollbuf->w, scrollbuf->h);
    
    for(int y=0; y<8; y++)
    {
        for(int x=0; x<16; x++)
        {
            if(!displayOnMap(x, y))
            {
                rectfill(scrollbuf, 256, 0, 511, 223, WHITE);
            }
            else
            {
                int s = (y<<4) + x;
                loadscr2(1,s,-1);
                
                for(int i=0; i<6; i++)
                {
                    if(tmpscr[1].layermap[i]<=0)
                        continue;
                    
                    if((ZCMaps[tmpscr[1].layermap[i]-1].tileWidth==ZCMaps[currmap].tileWidth) &&
                       (ZCMaps[tmpscr[1].layermap[i]-1].tileHeight==ZCMaps[currmap].tileHeight))
                    {
                        const int _mapsSize = (ZCMaps[currmap].tileWidth)*(ZCMaps[currmap].tileHeight);
                        
                        tmpscr2[i]=TheMaps[(tmpscr[1].layermap[i]-1)*MAPSCRS+tmpscr[1].layerscreen[i]];
                        
                        tmpscr2[i].data.resize(_mapsSize, 0);
                        tmpscr2[i].sflag.resize(_mapsSize, 0);
                        tmpscr2[i].cset.resize(_mapsSize, 0);
                    }
                }
                
                if((tmpscr+1)->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG) do_layer(scrollbuf, 1, tmpscr+1, -256, playing_field_offset, 2);
                
                if((tmpscr+1)->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG) do_layer(scrollbuf, 2, tmpscr+1, -256, playing_field_offset, 2);
                
                putscr(scrollbuf,256,0,tmpscr+1);
                do_layer(scrollbuf, 0, tmpscr+1, -256, playing_field_offset, 2);
                
                if(!(((tmpscr+1)->flags7&fLAYER2BG) || DMaps[currdmap].flags&dmfLAYER2BG) ) do_layer(scrollbuf, 1, tmpscr+1, -256, playing_field_offset, 2);
                
                putscrdoors(scrollbuf,256,0,tmpscr+1);
                do_layer(scrollbuf,-2, tmpscr+1, -256, playing_field_offset, 2);
                do_layer(scrollbuf,-3, tmpscr+1, -256, playing_field_offset, 2); // Freeform combos!
                
                if(!(((tmpscr+1)->flags7&fLAYER3BG) || DMaps[currdmap].flags&dmfLAYER2BG)) do_layer(scrollbuf, 2, tmpscr+1, -256, playing_field_offset, 2);
                
                do_layer(scrollbuf, 3, tmpscr+1, -256, playing_field_offset, 2);
                do_layer(scrollbuf,-1, tmpscr+1, -256, playing_field_offset, 2);
                do_layer(scrollbuf, 4, tmpscr+1, -256, playing_field_offset, 2);
                do_layer(scrollbuf, 5, tmpscr+1, -256, playing_field_offset, 2);
                
            }
            
            stretch_blit(scrollbuf, mappic, 256, 0, 256, 176, x<<(8-mapres), (y*176)>>mapres, 256>>mapres, 176>>mapres);
        }
    }
    
    for(int i=0; i<6; ++i)
    {
        tmpscr2[i]=tmpscr_c[i];
        
        if(i>=2)
        {
            continue;
        }
        
        tmpscr[i]=tmpscr_b[i];
    }
    
    
    clear_keybuf();
    pause_all_sfx();
    
    // view it
    int delay = 0;
    static int show  = 3;
    
    do
    {
        load_control_state();
        int step = int(16.0/scales[sc]);
        step = (step>>1) + (step&1);
        bool r = cRbtn();
        
        if(cLbtn())
        {
            step <<= 2;
            delay = 0;
        }
        
        if(r)
        {
            if(rUp())
            {
                py+=step;
                redraw=true;
            }
            
            if(rDown())
            {
                py-=step;
                redraw=true;
            }
            
            if(rLeft())
            {
                px+=step;
                redraw=true;
            }
            
            if(rRight())
            {
                px-=step;
                redraw=true;
            }
        }
        else
        {
            if(Up())
            {
                py+=step;
                redraw=true;
            }
            
            if(Down())
            {
                py-=step;
                redraw=true;
            }
            
            if(Left())
            {
                px+=step;
                redraw=true;
            }
            
            if(Right())
            {
                px-=step;
                redraw=true;
            }
        }
        
        if(delay)
            --delay;
        else
        {
            bool a = cAbtn();
            bool b = cBbtn();
            
            if(a && !b)
            {
                sc=zc_min(sc+1,16);
                delay=8;
                redraw=true;
            }
            
            if(b && !a)
            {
                sc=zc_max(sc-1,0);
                delay=8;
                redraw=true;
            }
        }
        
        if(rPbtn())
            --show;
            
        px = vbound(px,-4096,4096);
        py = vbound(py,-1408,1408);
        
        double scale = scales[sc];
        
        if(!redraw)
        {
            blit(scrollbuf,framebuf,256,0,0,0,256,224);
        }
        else
        {
            clear_to_color(framebuf,BLACK);
            stretch_blit(mappic,framebuf,0,0,mappic->w,mappic->h,
                         int(256+(px-mappic->w)*scale)/2,int(224+(py-mappic->h)*scale)/2,
                         int(mappic->w*scale),int(mappic->h*scale));
                         
            blit(framebuf,scrollbuf,0,0,256,0,256,224);
            redraw=false;
        }
        
        int x = int(256+(px-((2048-lx)*2))*scale)/2;
        int y = int(224+(py-((704-ly)*2))*scale)/2;
        
        if(show&1)
        {
            line(framebuf,x-7,y-7,x+7,y+7,(frame&3)+252);
            line(framebuf,x+7,y-7,x-7,y+7,(frame&3)+252);
        }
        
        //    text_mode(BLACK);
        
        if(show&2 || r)
            textprintf_ex(framebuf,font,224,216,WHITE,BLACK,"%1.2f",scale);
            
        if(r)
        {
            textprintf_ex(framebuf,font,0,208,WHITE,BLACK,"m: %d %d",px,py);
            textprintf_ex(framebuf,font,0,216,WHITE,BLACK,"x: %d %d",x,y);
        }
        
        //since stuff in here accesses tmpscr and tmpscr2... -DD
        advanceframe(false, false);
        
        
        if(getInput(btnS, true, false, true)) //rSbtn
            done = true;
            
    }
    while(!done && !Quit);
    
    destroy_bitmap(mappic);
    
    resume_all_sfx();
}

int onViewMap()
{
    if(Playing && currscr<128 && DMaps[currdmap].flags&dmfVIEWMAP)
    {
        clear_to_color(framebuf,BLACK);
        //      text_mode(BLACK);
        textout_centre_ex(framebuf,font,"Drawing map...",128,108,WHITE,BLACK);
        advanceframe(true);
        ViewMap();
    }
    
    return D_O_K;
}

bool isGrassType(int type)
{
    switch(type)
    {
    case cTALLGRASS:
    case cTALLGRASSNEXT:
    case cTALLGRASSTOUCHY:
        return true;
    }
    
    return false;
}

bool isFlowersType(int type)
{
    switch(type)
    {
    case cFLOWERS:
    case cFLOWERSTOUCHY:
        return true;
    }
    
    return false;
}

bool isGenericType(int type)
{
    switch(type)
    {
    case cTRIGGERGENERIC:
        return true;
    }
    
    return false;
}

bool isBushType(int type)
{
    switch(type)
    {
    case cBUSH:
    case cBUSHNEXT:
    case cBUSHTOUCHY:
    case cBUSHNEXTTOUCHY:

        return true;
    }
    
    return false;
}

bool isSlashType(int type)
{
    switch(type)
    {
    case cSLASH:
    case cSLASHITEM:
    case cSLASHTOUCHY:
    case cSLASHITEMTOUCHY:
    case cSLASHNEXT:
    case cSLASHNEXTITEM:
    case cSLASHNEXTTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
        return true;
    }
    
    return false;
}

bool isCuttableNextType(int type)
{
    switch(type)
    {
    case cSLASHNEXT:
    case cSLASHNEXTITEM:
    case cTALLGRASSNEXT:
    case cBUSHNEXT:
    case cSLASHNEXTTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
    case cBUSHNEXTTOUCHY:
        return true;
    }
    
    return false;
}

bool isTouchyType(int type)
{
    switch(type)
    {
    case cSLASHTOUCHY:
    case cSLASHITEMTOUCHY:
    case cBUSHTOUCHY:
    case cFLOWERSTOUCHY:
    case cTALLGRASSTOUCHY:
    case cSLASHNEXTTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
    case cBUSHNEXTTOUCHY:
        return true;
    }
    
    return false;
}

bool isCuttableType(int type)
{
    switch(type)
    {
    case cSLASH:
    case cSLASHITEM:
    case cBUSH:
    case cFLOWERS:
    case cTALLGRASS:
    case cTALLGRASSNEXT:
    case cSLASHNEXT:
    case cSLASHNEXTITEM:
    case cBUSHNEXT:
    
    case cSLASHTOUCHY:
    case cSLASHITEMTOUCHY:
    case cBUSHTOUCHY:
    case cFLOWERSTOUCHY:
    case cTALLGRASSTOUCHY:
    case cSLASHNEXTTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
    case cBUSHNEXTTOUCHY:
        return true;
    }
    
    return false;
}

bool isCuttableItemType(int type)
{
    switch(type)
    {
    case cSLASHITEM:
    case cBUSH:
    case cFLOWERS:
    case cTALLGRASS:
    case cTALLGRASSNEXT:
    case cSLASHNEXTITEM:
    case cBUSHNEXT:
    
    case cSLASHITEMTOUCHY:
    case cBUSHTOUCHY:
    case cFLOWERSTOUCHY:
    case cTALLGRASSTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
    case cBUSHNEXTTOUCHY:
        return true;
    }
    
    return false;
}


/*** end of maps.cc ***/

