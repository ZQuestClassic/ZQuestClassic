#include <cstring>
#include <assert.h>
#include <math.h>
#include <vector>
#include <deque>
#include <string>
#include <set>
#include <sstream>
using std::set;

#include "base/qrs.h"
#include "base/dmap.h"
#include "base/mapscr.h"
#include "base/misctypes.h"
#include "base/initdata.h"
#include "zc/maps.h"
#include "zc/zelda.h"
#include "zc/zc_ffc.h"
#include "tiles.h"
#include "sprite.h"
#include "gui/jwin.h"
#include "base/zsys.h"
#include "subscr.h"
#include "zc/zc_subscr.h"
#include "zc/hero.h"
#include "zc/guys.h"
#include "zc/ffscript.h"
#include "drawing.h"
#include "zc/combos.h"
#include "zc/replay.h"
#include "slopes.h"
extern FFScript FFCore;
#include "particles.h"
#include <fmt/format.h>
#include "zc/render.h"


#define EPSILON 0.01 // Define your own tolerance
#define FLOAT_EQ(x,v) (((v - EPSILON) < x) && (x <( v + EPSILON)))
#define DegtoFix(d)     ((d)*0.71111111)


//MSVC does not provide a log2 funcion in <cmath>
// adjust version number as needed. MSVC 2015 (1900) does *not* need these functions
#if defined _MSC_VER && _MSC_VER < 1900
double log2(double n)
{
    return log(n) / log(2.0);
}
float log2(float n)
{
    return log(n) / log(2.f);
}
#endif

int32_t COMBOPOS(int32_t x, int32_t y)
{
    return (((y) & 0xF0) + ((x) >> 4));
}
int32_t COMBOPOS_B(int32_t x, int32_t y)
{
	if(unsigned(x) >= 256 || unsigned(y) >= 176)
		return -1;
	return COMBOPOS(x,y);
}
int32_t COMBOX(int32_t pos)
{
    return ((pos) % 16 * 16);
}
int32_t COMBOY(int32_t pos)
{
    return ((pos) & 0xF0);
}

int32_t mapind(int32_t map, int32_t scr)
{
	return (map<<7)+scr;
}

FONT *get_zc_font(int index);

extern sprite_list  guys, items, Ewpns, Lwpns, chainlinks, decorations;
extern particle_list particles;
extern movingblock mblock2;                                 //mblock[4]?
extern portal mirror_portal;
extern HeroClass Hero;
int32_t current_ffcombo=-1;
bool triggered_screen_secrets=false;

int16_t ffposx[MAXFFCS];
int16_t ffposy[MAXFFCS];
int32_t ffprvx[MAXFFCS];
int32_t ffprvy[MAXFFCS];
void init_ffpos()
{
	for(word q = 0; q < MAXFFCS; ++q)
	{
		ffposx[q] = -1000;
		ffposy[q] = -1000;
		ffprvx[q] = -10000000;
		ffprvy[q] = -10000000;
	}
}




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



bool checktrigger=false;

void debugging_box(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    //reference/optimization: the start of the unused drawing command index can now be queried. -Gleeok
    int32_t index = script_drawing_commands.GetNext();
    
    if(index < 0)
        return;
        
    int32_t *sdci = &script_drawing_commands[index][0];
    
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
	DMaps[i].clear();
}

void clear_dmaps()
{
    for(int32_t i=0; i<MAXDMAPS; i++)
    {
        clear_dmap(i);
    }
}

int32_t isdungeon(int32_t dmap, int32_t scr) // The arg is only used by loadscr2 and loadscr
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

bool canPermSecret(int32_t dmap, int32_t scr)
{
	return (!isdungeon(dmap, scr) || get_qr(qr_DUNGEON_DMAPS_PERM_SECRETS));
}

int32_t MAPCOMBO(int32_t x,int32_t y)
{
    //extend combos outwards if out of bounds -DD
    x = vbound(x, 0, (16*16)-1);
    y = vbound(y, 0, (11*16)-1);
    int32_t combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return tmpscr->data[combo];                               // entire combo code
}

int32_t MAPCOMBOzq(int32_t x,int32_t y)
{
	return MAPCOMBO(x,y);
}

//specific layers 1 to 6
int32_t MAPCOMBOL(int32_t layer,int32_t x,int32_t y)
{
    if(tmpscr2[layer-1].valid==0) return 0;
    
    int32_t combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return tmpscr2[layer-1].data[combo];                        // entire combo code
}

int32_t MAPCSETL(int32_t layer,int32_t x,int32_t y)
{
    if(tmpscr2[layer-1].valid==0) return 0;
    
    int32_t combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return tmpscr2[layer-1].cset[combo];                        // entire combo code
}

int32_t MAPFLAGL(int32_t layer,int32_t x,int32_t y)
{
    if(tmpscr2[layer-1].valid==0) return 0;
    
    int32_t combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return tmpscr2[layer-1].sflag[combo];                       // flag
}

int32_t COMBOTYPEL(int32_t layer,int32_t x,int32_t y)
{
    if(!layer || tmpscr2[layer-1].valid==0)
    {
        return 0;
    }
    
    return combobuf[MAPCOMBO2(layer-1,x,y)].type;
}

int32_t MAPCOMBOFLAGL(int32_t layer,int32_t x,int32_t y)
{
    if(layer==-1) return MAPCOMBOFLAG(x,y);
    
    if(tmpscr2[layer-1].valid==0) return 0;
    
    int32_t combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return combobuf[tmpscr2[layer-1].data[combo]].flag;                        // entire combo code
}


// True if the FFC covers x, y and is not ethereal or a changer.
// Used by MAPFFCOMBO(), MAPFFCOMBOFLAG, and getFFCAt().
bool ffcIsAt(int32_t index, int32_t x, int32_t y)
{
	if(tmpscr->ffcs[index].data<=0)
        return false;

    int32_t fx=tmpscr->ffcs[index].x.getInt();
    if(x<fx || x>fx+(tmpscr->ffEffectWidth(index)-1)) // FFC sizes are weird.
        return false;
    
    int32_t fy=tmpscr->ffcs[index].y.getInt();
    if(y<fy || y>fy+(tmpscr->ffEffectHeight(index)-1))
        return false;
    
    if((tmpscr->ffcs[index].flags&(ffc_changer|ffc_ethereal))!=0)
        return false;
    
    return true;
}

int32_t MAPFFCOMBO(int32_t x,int32_t y)
{
	int32_t ffcid = getFFCAt(x,y);
	if(ffcid > -1)
	{
		return tmpscr->ffcs[ffcid].data;
	}
    return 0;
}

int32_t MAPCSET(int32_t x,int32_t y)
{
    if(x<0 || x>255 || y<0 || y>175)
        return 0;
        
    int32_t combo = COMBOPOS(x,y);
    return tmpscr->cset[combo];                               // entire combo code
}

int32_t MAPFLAG(int32_t x,int32_t y)
{
    if(x<0 || x>255 || y<0 || y>175)
        return 0;
        
    int32_t combo = COMBOPOS(x,y);
    return tmpscr->sflag[combo];                              // flag
}

int32_t COMBOTYPE(int32_t x,int32_t y)
{
	for (int32_t i = 0; i <= 1; ++i)
	{
		if(tmpscr2[i].valid!=0)
		{
			if (get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, &(tmpscr2[i]))) return cNONE;
			}
			else
			{
				if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && _effectflag_layer(x,y,1, &(tmpscr2[i]))) return cNONE;
			}
		}
	}
	int32_t b=1;
    
	if(x&8) b<<=2;
    
	if(y&8) b<<=1;
	newcombo const& cmb = combobuf[MAPCOMBO(x,y)];
	if (cmb.type == cWATER && (cmb.walk&b) && ((cmb.walk>>4)&b))
	{
		if(cmb.usrflags&cflag4) return cSHALLOWWATER;
		if(cmb.usrflags&cflag3) return cNONE;
	}
	return cmb.type;
}

int32_t FFCOMBOTYPE(int32_t x,int32_t y)
{
    return combobuf[MAPFFCOMBO(x,y)].type;
}

int32_t FFORCOMBO(int32_t x, int32_t y)
{
	int32_t ffcid = getFFCAt(x,y);
	if(ffcid > -1)
	{
		return tmpscr->ffcs[ffcid].data;
	}
	
	return MAPCOMBO(x,y);
}

int32_t FFORCOMBOTYPE(int32_t x, int32_t y)
{
	for (int32_t i = 0; i <= 1; ++i)
	{
		if(tmpscr2[i].valid!=0)
		{
			if (get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, &(tmpscr2[i]))) return cNONE;
			}
			else
			{
				if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && _effectflag_layer(x,y,1, &(tmpscr2[i]))) return cNONE;
			}
		}
	}
	int32_t b=1;
    
	if(x&8) b<<=2;
    
	if(y&8) b<<=1;
	newcombo const& cmb = combobuf[FFORCOMBO(x,y)];
	if (cmb.type == cWATER && (cmb.usrflags&cflag4) && (cmb.walk&b)) return cSHALLOWWATER;
	if (cmb.type == cWATER && (cmb.usrflags&cflag3) && (cmb.walk&b)) return cNONE;
	return cmb.type;
}

int32_t FFORCOMBO_L(int32_t layer, int32_t x, int32_t y)
{
	int32_t ffcid = getFFCAt(x,y);
	if(ffcid > -1)
	{
		return tmpscr->ffcs[ffcid].data;
	}
	
	return layer ? MAPCOMBOL(layer, x, y) : MAPCOMBO(x,y);
}

int32_t FFORCOMBOTYPE_L(int32_t layer, int32_t x, int32_t y)
{
	return combobuf[FFORCOMBO_L(layer,x,y)].type;
}

int32_t MAPCOMBOFLAG(int32_t x,int32_t y)
{
    if(x<0 || x>255 || y<0 || y>175)
        return 0;
        
    int32_t combo = COMBOPOS(x,y);
    return combobuf[tmpscr->data[combo]].flag;                               // entire combo code
}

int32_t MAPFFCOMBOFLAG(int32_t x,int32_t y)
{
	current_ffcombo = getFFCAt(x,y);
	if(current_ffcombo > -1)
	{
		return combobuf[tmpscr->ffcs[current_ffcombo].data].flag;
	}
    return 0;
}

int32_t getFFCAt(int32_t x, int32_t y)
{
	word c = tmpscr->numFFC();
    for(word i=0; i<c; i++)
    {
        if(ffcIsAt(i, x, y))
            return i;
    }
    
    return -1;
}

int32_t MAPCOMBO2(int32_t layer,int32_t x,int32_t y)
{
    if(layer<=-1) return MAPCOMBO(x,y);
    
    if(tmpscr2[layer].valid==0) return 0;
    
    int32_t combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return tmpscr2[layer].data[combo];                        // entire combo code
}

static int32_t MAPCOMBO3_impl(int32_t map, int32_t screen, int32_t layer, int32_t pos, bool secrets)
{ 
	if (map < 0 || screen < 0) return 0;
	
	if(pos>175 || pos < 0)
		return 0;
		
	mapscr *m = &TheMaps[(map*MAPSCRS)+screen];
    
	if(m->valid==0) return 0;
	
	int32_t mi = (map*MAPSCRSNORMAL)+screen;
	int32_t flags = 0;
	
	if(secrets)
	{
		flags = game->maps[mi];
	}
	
	int32_t mapid = (layer < 0 ? -1 : ((m->layermap[layer] - 1) * MAPSCRS + m->layerscreen[layer]));
	
	if (layer >= 0 && (mapid < 0 || mapid > MAXMAPS*MAPSCRS)) return 0;
	
	mapscr scr = ((mapid < 0 || mapid > MAXMAPS*MAPSCRS) ? *m : TheMaps[mapid]);
    
	if(scr.valid==0) return 0;
	
	if ((flags & mSECRET) && canPermSecret(currdmap, screen))
	{
		hiddenstair2(&scr, false);
		hidden_entrance2(&scr, (mapscr*)NULL, false, -3);
	}
	if(flags & mLIGHTBEAM)
	{
		for(size_t pos = 0; pos < 176; ++pos)
		{
			newcombo const* cmb = &combobuf[scr.data[pos]];
			if(cmb->type == cLIGHTTARGET)
			{
				if(!(cmb->usrflags&cflag1)) //Unlit version
				{
					scr.data[pos] += 1;
				}
			}
		}
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
	
	clear_xdoors2(&scr, nullptr, mi);
	clear_xstatecombos2(&scr, nullptr, mi);
	
	return scr.data[pos];						// entire combo code
}

// Read from the current temporary screens or, if (map, screen) is not loaded,
// load that screen and apply the relevant secrets before evaluating the combo at that position.
int32_t MAPCOMBO3(int32_t map, int32_t screen, int32_t layer, int32_t x, int32_t y, bool secrets)
{
	if (map == currmap && currscr == screen) return MAPCOMBO2(layer, x, y);

	// Screen is not currently loaded, so we have to load and trigger some secrets.
	int pos = COMBOPOS(x, y);
	return MAPCOMBO3_impl(map, screen, layer, pos, secrets);
}

int32_t MAPCOMBO3(int32_t map, int32_t screen, int32_t layer, int32_t pos, bool secrets)
{
	if (map == currmap && currscr == screen) return MAPCOMBO2(layer, COMBOX(pos), COMBOY(pos));

	// Screen is not currently loaded, so we have to load and trigger some secrets.
	return MAPCOMBO3_impl(map, screen, layer, pos, secrets);
}

int32_t MAPCSET2(int32_t layer,int32_t x,int32_t y)
{
	if(layer==-1) return MAPCSET(x,y);
	
	if(tmpscr2[layer].valid==0) return 0;
	
	int32_t combo = COMBOPOS(x,y);
	
	if(combo>175 || combo < 0)
		return 0;
		
	return tmpscr2[layer].cset[combo];						// entire combo code
}

int32_t MAPFLAG2(int32_t layer,int32_t x,int32_t y)
{
    if(!get_qr(qr_BUGGED_LAYERED_FLAGS) && (x<0 || x>255 || y<0 || y>175))
        return 0;
    if(layer==-1) return MAPFLAG(x,y);
    
    if(tmpscr2[layer].valid==0) return 0;
    
    int32_t combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return tmpscr2[layer].sflag[combo];                       // flag
}

int32_t COMBOTYPE2(int32_t layer,int32_t x,int32_t y)
{
    if(layer < 1)
    {
	for (int32_t i = layer+1; i <= 1; ++i)
	{
		if(tmpscr2[i].valid!=0)
		{
			if (get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, &(tmpscr2[i]))) return cNONE;
			}
			else
			{
				if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && _effectflag_layer(x,y,1, &(tmpscr2[i]))) return cNONE;
			}
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

int32_t MAPCOMBOFLAG2(int32_t layer,int32_t x,int32_t y)
{
    if(layer==-1) return MAPCOMBOFLAG(x,y);
    
    if(tmpscr2[layer].valid==0) return 0;
    
    int32_t combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return combobuf[tmpscr2[layer].data[combo]].flag;                        // entire combo code
}

bool HASFLAG(int32_t flag, int32_t layer, int32_t pos)
{
	if(unsigned(pos) > 175) return false;
	if(unsigned(layer) > 6) return false;
	mapscr* m = (layer ? &tmpscr2[layer-1] : tmpscr);
	if(!m->valid) return false;
	
	if(m->sflag[pos] == flag) return true;
	if(combobuf[m->data[pos]].flag == flag) return true;
	
	return false;
}

bool HASFLAG_ANY(int32_t flag, int32_t pos)
{
	if(unsigned(pos) > 175) return false;
	for(auto q = 0; q < 7; ++q)
	{
		if(HASFLAG(flag, q, pos))
			return true;
	}
	return false;
}

void setmapflag(int32_t flag)
{
    setmapflag((currmap*MAPSCRSNORMAL)+homescr,flag);
}

const char *screenstate_string[16] =
{
    "Door Up", "Door Down", "Door Left", "Door Right", "Item", "Special Item", "No Return",
    "Temporary No Return", "Lock Blocks", "Boss Lock Blocks", "Chests", "Locked Chests",
    "Boss Locked Chests", "Secrets", "Visited", "Light Beams"
};

void eventlog_mapflags()
{
	std::ostringstream oss;
	
	int32_t mi = (currmap*MAPSCRSNORMAL)+homescr;
    word g = game->maps[mi] &0x3FFF;
    
	oss << fmt::format("Screen ({}, {:02X})", currmap+1, homescr);
	if(g) // Main States
	{
		static const int order[] =
		{
			mSECRET, mITEM, mSPECIALITEM, mLOCKBLOCK, mBOSSLOCKBLOCK,
			mCHEST, mLOCKEDCHEST, mBOSSCHEST,
			mDOOR_UP, mDOOR_DOWN, mDOOR_LEFT, mDOOR_RIGHT,
			mNEVERRET, mTMPNORET
		};
		
		oss << " [";
		bool comma = false;
		for(int fl : order)
		{
			if(!(g&fl))
				continue;
			byte ind = byte(log2(double(fl)));
			if(comma)
				oss << ", ";
			oss << screenstate_string[ind];
			comma = true;
		}
		oss << "]";
	}
	if(game->xstates[mi]) // ExStates
	{
		oss << " Ex[";
		bool comma = false;
		for(byte fl = 0; fl < 32; ++fl)
		{
			if(game->xstates[mi] & (1<<fl))
			{
				if(comma)
					oss << ", ";
				oss << fl;
				comma = true;
			}
		}
		oss << "]";
	}
	{ // ExDoors
		for(int q = 0; q < 4; ++q)
		{
			bool comma = false;
			if(auto v = game->xdoors[mi][q])
			{
				if(comma)
					oss << ",";
				else oss << " ExDoor";
				oss << "[" << dirstr[q];
				for(int fl = 0; fl < 8; ++fl)
					if(v & (1<<fl))
						oss << " " << fl;
				oss << "]";
				comma = true;
			}
		}
	}
	Z_eventlog("%s\n", oss.str().c_str());
}

// set specific flag
void setmapflag(int32_t mi2, int32_t flag)
{
    byte cscr = mi2&((1<<7)-1);
    byte cmap = (mi2>>7);
    char buf[20];
    sprintf(buf,"Screen (%d, %02X)",cmap+1,cscr);
    
    float temp=log2((float)flag);
    if (replay_is_active() && !(game->maps[mi2] & flag))
        replay_step_comment(fmt::format("map {} scr {} flag {}", cmap, cscr, flag > 0 ? screenstate_string[(int32_t)temp] : "<Unknown>"));
    game->maps[mi2] |= flag;
    Z_eventlog("%s's State was set: %s\n",
               mi2 != (currmap*MAPSCRSNORMAL)+homescr ? buf : "Current screen",
               flag>0 ? screenstate_string[(int32_t)temp] : "<Unknown>");
               
    if(flag==mSECRET||flag==mITEM||flag==mSPECIALITEM||flag==mLOCKBLOCK||
            flag==mBOSSLOCKBLOCK||flag==mCHEST||flag==mBOSSCHEST||flag==mLOCKEDCHEST)
    {
        byte nmap=TheMaps[((cmap)*MAPSCRS)+cscr].nextmap;
        byte nscr=TheMaps[((cmap)*MAPSCRS)+cscr].nextscr;
        
        std::vector<int32_t> done;
        bool looped = (nmap==cmap+1 && nscr==cscr);
        
        while((nmap!=0) && !looped && !(nscr>=128))
        {
            if((tmpscr->nocarry&flag)!=flag)
            {
                Z_eventlog("State change carried over to (%d, %02X)\n",nmap+1,nscr);
                if (replay_is_active() & !(game->maps[((nmap-1)<<7)+nscr] & flag))
                    replay_step_comment(fmt::format("map {} scr {} flag {} carry", nmap, nscr, flag > 0 ? screenstate_string[(int32_t)temp] : "<Unknown>"));
                game->maps[((nmap-1)<<7)+nscr] |= flag;
            }
            
            cmap=nmap;
            cscr=nscr;
            nmap=TheMaps[((cmap-1)*MAPSCRS)+cscr].nextmap;
            nscr=TheMaps[((cmap-1)*MAPSCRS)+cscr].nextscr;
            
            for(std::vector<int32_t>::iterator it = done.begin(); it != done.end(); it++)
            {
                if(*it == ((nmap-1)<<7)+nscr)
                    looped = true;
            }
            
            done.push_back(((nmap-1)<<7)+nscr);
        }
    }
}

void unsetmapflag(int32_t flag, bool anyflag)
{
    unsetmapflag((currmap*MAPSCRSNORMAL)+homescr,flag,anyflag);
}

void unsetmapflag(int32_t mi2, int32_t flag, bool anyflag)
{
    if(anyflag)
        game->maps[mi2] &= ~flag;
    else if(flag==mITEM || flag==mSPECIALITEM)
    {
        if(!(tmpscr->flags4&fNOITEMRESET))
            game->maps[mi2] &= ~flag;
    }
    else game->maps[mi2] &= ~flag;
    
    byte cscr = mi2&((1<<7)-1);
    byte cmap = (mi2>>7);
    char buf[20];
    sprintf(buf,"Screen (%d, %02X)",cmap+1,cscr);
    
    float temp=log2((float)flag);
    Z_eventlog("%s's State was unset: %s\n",
               mi2 != (currmap*MAPSCRSNORMAL)+homescr ? buf : "Current screen",
               flag>0 ? screenstate_string[(int32_t)temp] : "<Unknown>");
               
    if(flag==mSECRET||flag==mITEM||flag==mSPECIALITEM||flag==mLOCKBLOCK||
            flag==mBOSSLOCKBLOCK||flag==mCHEST||flag==mBOSSCHEST||flag==mLOCKEDCHEST)
    {
        byte nmap=TheMaps[((cmap)*MAPSCRS)+cscr].nextmap;
        byte nscr=TheMaps[((cmap)*MAPSCRS)+cscr].nextscr;
        
        std::vector<int32_t> done;
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
            
            for(std::vector<int32_t>::iterator it = done.begin(); it != done.end(); it++)
            {
                if(*it == ((nmap-1)<<7)+nscr)
                    looped = true;
            }
            
            done.push_back(((nmap-1)<<7)+nscr);
        }
    }
}

bool getmapflag(int32_t flag)
{
    return (game->maps[(currmap*MAPSCRSNORMAL)+homescr] & flag) != 0;
}

void setxmapflag(int32_t mi2, uint32_t flag)
{
	if(game->xstates[mi2] & flag) return;
    byte cscr = mi2&((1<<7)-1);
    byte cmap = (mi2>>7);
    char buf[20];
    sprintf(buf,"Screen (%d, %02X)",cmap+1,cscr);
    
    byte temp=(byte)log2((double)flag);
	Z_eventlog("%s's ExtraState was set: %d\n",
		mi2 != (currmap*MAPSCRSNORMAL)+homescr ? buf : "Current screen", temp);
	
	game->xstates[mi2] |= flag;
}
void setxmapflag(uint32_t flag)
{
	setxmapflag((currmap*MAPSCRSNORMAL)+homescr, flag);
}
void unsetxmapflag(int32_t mi2, uint32_t flag)
{
	if(!(game->xstates[mi2] & flag)) return;
    byte cscr = mi2&((1<<7)-1);
    byte cmap = (mi2>>7);
    char buf[20];
    sprintf(buf,"Screen (%d, %02X)",cmap+1,cscr);
    
    byte temp=(byte)log2((double)flag);
	Z_eventlog("%s's ExtraState was unset: %d\n",
		mi2 != (currmap*MAPSCRSNORMAL)+homescr ? buf : "Current screen", temp);
	
	game->xstates[mi2] &= ~flag;
}
void unsetxmapflag(uint32_t flag)
{
	unsetxmapflag((currmap*MAPSCRSNORMAL)+homescr, flag);
}
bool getxmapflag(int32_t mi2, uint32_t flag)
{
	return (game->xstates[mi2] & flag) != 0;
}
bool getxmapflag(uint32_t flag)
{
	return getxmapflag((currmap*MAPSCRSNORMAL)+homescr, flag);
}

void setxdoor(uint mi, uint dir, uint ind, bool state)
{
	if(mi > game->xdoors.size() || dir > 3 || ind > 8)
		return;
	if(!(game->xdoors[mi][dir] & (1<<ind)) == !state)
		return;
	SETFLAG(game->xdoors[mi][dir], 1<<ind, state);
    int cscr = mi % MAPSCRSNORMAL;
    int cmap = mi / MAPSCRSNORMAL;
	bool iscurrscr = mi == ((currmap*MAPSCRSNORMAL)+homescr);
	Z_eventlog("%s's ExDoor[%s][%d] was %sset\n",
		iscurrscr ? "Current screen" : fmt::format("Screen ({}, {:02X})",cmap+1,cscr).c_str(),
		dirstr[dir], ind, state ? "" : "un");
}
void setxdoor(uint dir, uint ind, bool state)
{
	setxdoor((currmap*MAPSCRSNORMAL)+homescr,dir,ind,state);
}
bool getxdoor(uint mi, uint dir, uint ind)
{
	if(mi >= game->xdoors.size() || dir >= 4 || ind >= 8)
		return false;
	return (game->xdoors[mi][dir] & (1<<ind));
}
bool getxdoor(uint dir, uint ind)
{
	return getxdoor((currmap*MAPSCRSNORMAL)+homescr,dir,ind);
}

void set_doorstate(uint mi, uint dir)
{
	if(dir >= 4)
		return;
	setmapflag(mi, mDOOR_UP << dir);
	if(auto di = nextscr(mi, dir, true))
		setmapflag(*di, mDOOR_UP << oppositeDir[dir]);
}
void set_doorstate(uint dir)
{
	set_doorstate((currmap*MAPSCRSNORMAL) + currscr, dir);
}

void set_xdoorstate(uint mi, uint dir, uint ind)
{
	if(mi >= game->xdoors.size() || dir >= 4 || ind >= 8)
		return;
	setxdoor(mi, dir, ind, true);
	if(auto di = nextscr(mi, dir, true))
		setxdoor(*di, oppositeDir[dir], ind);
}
void set_xdoorstate(uint dir, uint ind)
{
	set_xdoorstate((currmap*MAPSCRSNORMAL) + currscr, dir, ind);
}

int32_t WARPCODE(int32_t dmap,int32_t scr,int32_t dw)
// returns: -1 = not a warp screen
//          0+ = warp screen code ( high byte=dmap, low byte=scr )
{
    mapscr *s = &TheMaps[DMaps[dmap].map*MAPSCRS+scr];
    
    if(s->room!=rWARP)
        return -1;
        
    int32_t ring=s->catchall;
    int32_t size=QMisc.warp[ring].size;
    
    if(size==0)
        return -2;
        
    int32_t index=-1;
    
    for(int32_t i=0; i<size; i++)
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
    int32_t x,y;
    y = 0;
    static int32_t newdata[176];
    static int32_t newcset[176];
    static int32_t newdata2[176];
    static int32_t newcset2[176];
    std::set<uint16_t> restartanim;
    std::set<uint16_t> restartanim2;
    static bool initialized=false;
    
    // Just a simple bit of optimization
    if(!initialized)
    {
        for(int32_t i=0; i<176; i++)
        {
            newdata[i]=-1;
            newcset[i]=-1;
            newdata2[i]=-1;
            newcset2[i]=-1;
        }
        
        initialized=true;
    }
    
    for(int32_t i=0; i<176; i++)
    {
        x=tmpscr->data[i];
        
        if(combobuf[x].animflags & AF_FRESH) continue;
        
        //time to restart
		
        if ((combobuf[x].aclk>=combobuf[x].speed) && combobuf[x].nextcombo!=0 && combocheck(combobuf[x]))
        {
            newdata[i]=combobuf[x].nextcombo;
			if(!(combobuf[x].animflags & AF_CYCLENOCSET))
				newcset[i]=combobuf[x].nextcset;
            int32_t c=newdata[i];
            
            if(combobuf[c].animflags & AF_CYCLE)
            {
                restartanim.insert(c);
            }
        }
    }
    
    for(int32_t i=0; i<176; i++)
    {
        x=tmpscr->data[i];
        
        if(!(combobuf[x].animflags & AF_FRESH)) continue;
        
        //time to restart
        if ((combobuf[x].aclk>=combobuf[x].speed) && combobuf[x].nextcombo!=0 && combocheck(combobuf[x]))
        {
            newdata[i]=combobuf[x].nextcombo;
            if(!(combobuf[x].animflags & AF_CYCLENOCSET))
				newcset[i]=combobuf[x].nextcset;
            int32_t c=newdata[i];
            
            if(combobuf[c].animflags & AF_CYCLE)
            {
                restartanim2.insert(c);
            }
        }
    }
    
    for(int32_t i=0; i<176; i++)
    {
        if(newdata[i]==-1)
            continue;
            
        screen_combo_modify_preroutine(tmpscr,i);
        tmpscr->data[i]=newdata[i];
		if(newcset[i]>-1)
			tmpscr->cset[i]=newcset[i];
        screen_combo_modify_postroutine(tmpscr,i);
        
        newdata[i]=-1;
        newcset[i]=-1;
    }
    
	word c = tmpscr->numFFC();
    for(word i=0; i<c; i++)
    {
		ffcdata& ffc = tmpscr->ffcs[i];
		newcombo const& cmb = combobuf[ffc.data];
        
		bool fresh = cmb.animflags & AF_FRESH;
        
        //time to restart
        if ((cmb.aclk>=cmb.speed) && cmb.nextcombo!=0 && combocheck(cmb))
        {
            zc_ffc_set(ffc, cmb.nextcombo);
            if(!(cmb.animflags & AF_CYCLENOCSET))
				ffc.cset=cmb.nextcset;
            
            if(combobuf[ffc.data].animflags & AF_CYCLE)
            {
                auto& animset = fresh ? restartanim2 : restartanim;
                animset.insert(ffc.data);
            }
        }
    }
    
    if(get_qr(qr_CMBCYCLELAYERS))
    {
        for(int32_t j=0; j<6; j++)
        {
            for(int32_t i=0; i<176; i++)
            {
                x=(tmpscr2+j)->data[i];
                
                if(combobuf[x].animflags & AF_FRESH) continue;
                
                //time to restart
                if ((combobuf[x].aclk>=combobuf[x].speed) && combobuf[x].nextcombo!=0 && combocheck(combobuf[x]))
                {
                    newdata[i]=combobuf[x].nextcombo;
                    if(!(combobuf[x].animflags & AF_CYCLENOCSET))
						newcset[i]=combobuf[x].nextcset;
                    int32_t c=newdata[i];
                    
                    if(combobuf[c].animflags & AF_CYCLE)
                    {
                        restartanim.insert(c);
                    }
                }
            }
            
            for(int32_t i=0; i<176; i++)
            {
                x=(tmpscr2+j)->data[i];
                
                if(!(combobuf[x].animflags & AF_FRESH)) continue;
                
                //time to restart
                if ((combobuf[x].aclk>=combobuf[x].speed) && combobuf[x].nextcombo!=0 && combocheck(combobuf[x]))
                {
                    newdata2[i]=combobuf[x].nextcombo;
					if(!(combobuf[x].animflags & AF_CYCLENOCSET))
						newcset2[i]=combobuf[x].nextcset;
					else newcset2[i]=(tmpscr2+j)->cset[i];
                    int32_t c=newdata2[i];
                    int32_t cs=newcset2[i];
                    
                    if(combobuf[c].animflags & AF_CYCLE)
                    {
                        restartanim2.insert(c);
                    }
                    
                    if(combobuf[c].type==cSPINTILE1)
                    {
                        // Uses animated_combo_table2
                        addenemy((i&15)<<4,i&0xF0,(cs<<12)+eSPINTILE1,combobuf[c].o_tile+zc_max(1,combobuf[c].frames));
                    }
                }
            }
            
            for(int32_t i=0; i<176; i++)
            {
                if(newdata[i]!=-1)
                {
                    screen_combo_modify_preroutine(tmpscr2+j,i);
                    (tmpscr2+j)->data[i]=newdata[i];
                    if(newcset[i]>-1)
						(tmpscr2+j)->cset[i]=newcset[i];
                    screen_combo_modify_postroutine(tmpscr2+j,i);
                    
                    newdata[i]=-1;
                    newcset[i]=-1;
                }
                
                if(newdata2[i]!=-1)
                {
                    (tmpscr2+j)->data[i]=newdata2[i];
                    if(newcset2[i]>-1)
						(tmpscr2+j)->cset[i]=newcset2[i];
                    newdata2[i]=-1;
                    newcset2[i]=-1;
                }
            }
        }
    }

    for (auto i : restartanim)
    {
        combobuf[i].tile = combobuf[i].o_tile;
        combobuf[i].cur_frame=0;
        combobuf[i].aclk = 0;
    }

    for (auto i : restartanim2)
    {
        combobuf[i].tile = combobuf[i].o_tile;
        combobuf[i].cur_frame=0;
        combobuf[i].aclk = 0;
    }
}

bool iswater_type(int32_t type)
{
//  return type==cOLD_WATER || type==cSWIMWARP || type==cDIVEWARP || type==cDIVEWARPB || type==cDIVEWARPC || type==cDIVEWARPD || type==cSWIMWARPB || type==cSWIMWARPC || type==cSWIMWARPD;
    return (combo_class_buf[type].water!=0);
}

bool iswater(int32_t combo)
{
    return iswater_type(combobuf[combo].type) && !DRIEDLAKE;
}
int32_t iswaterexzq(int32_t combo, int32_t map, int32_t screen, int32_t layer, int32_t x, int32_t y, bool secrets, bool fullcheck, bool LayerCheck)
{
	return iswaterex(combo, map, screen, layer, x, y, secrets, fullcheck, LayerCheck);
}
int32_t iswaterex(int32_t combo, int32_t map, int32_t screen, int32_t layer, int32_t x, int32_t y, bool secrets, bool fullcheck, bool LayerCheck, bool ShallowCheck, bool hero)
{
	//Honestly, fullcheck is kinda useless... I made this function back when I thought it was checking the entire combo and not just a glorified x/y value.
	//Fullcheck makes no sense to ever be on, but hey I guess it's here in case you ever need it... 
	
	//Oh hey, Zoras might actually need it. Nevermind, this had a use!
	if (get_qr(qr_SMARTER_WATER))
	{
		if (DRIEDLAKE) return 0;
		if (LayerCheck && (get_qr(qr_WATER_ON_LAYER_1) || get_qr(qr_WATER_ON_LAYER_2))) //LayerCheck is a bit dumber, but it lets me add this QR without having to replace all calls, again.
		{
			for (int32_t m = layer; m <= 1; m++)
			{
				if (m < 0 || m == 0 && get_qr(qr_WATER_ON_LAYER_1)
				|| m == 1 && get_qr(qr_WATER_ON_LAYER_2))
				{
					int32_t checkwater = iswaterex(combo, map, screen, m, x, y, secrets, fullcheck, false, ShallowCheck);
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
			word ffc_count = tmpscr->numFFC();
			for(int32_t i=(fullcheck?3:0); i>=0; i--)
			{
				int32_t tx2=((i&2)<<2)+x;
				int32_t ty2=((i&1)<<3)+y;
				int32_t b = i; //Originally b was not needed and I read off i, but then I added the boolean for fullcheck.
				//In which case it's just easier to change b if fullcheck is false instead of changing i and potentially screwing up the for loop.
				if (!fullcheck)
				{
					tx2 = x;
					ty2 = y;
					if(tx2&8) b+=2;
					if(ty2&8) b+=1;
				}
				for (int32_t m = layer; m <= 1; m++)
				{
					newcombo const& cmb = combobuf[MAPCOMBO3(map, screen, m,tx2,ty2, true)];
					if (get_qr(qr_OLD_BRIDGE_COMBOS))
					{
						if (cmb.type == cBRIDGE && !(cmb.walk&(1<<b))) 
						{
							return 0;
						}
					}
					else
					{
						if (cmb.type == cBRIDGE && (cmb.walk&(0x10<<b))) 
						{
							return 0;
						}
					}
					if (get_qr(qr_NO_SOLID_SWIM))
					{
						if ((cmb.type != cBRIDGE || (!get_qr(qr_OLD_BRIDGE_COMBOS) && !(cmb.walk&(0x10<<b)))) && (cmb.walk&(1<<b)) && !((cmb.usrflags&cflag4) && cmb.type == cWATER && (cmb.walk&(0x10<<b)) && ShallowCheck))
						{
							return 0;
						}						
					}
					if (iswater_type(cmb.type) && (cmb.walk&(1<<b)) && ((cmb.usrflags&cflag3) || (cmb.usrflags&cflag4)
						|| (hero && current_item(itype_flippers) < cmb.attribytes[0])
						|| (hero && ((cmb.usrflags&cflag1) && !(itemsbuf[current_item_id(itype_flippers)].flags & item_flag3)))))
					{
						if (!(ShallowCheck && (cmb.walk&(1<<b)) && (cmb.usrflags&cflag4))) return 0;
					}
				}
				for(word k=0; k<ffc_count; k++)
				{
					if(ffcIsAt(k, tx2, ty2))
					{
						ffcdata const& ffc = tmpscr->ffcs[k];
						auto ty = combobuf[ffc.data].type;
						if(!combo_class_buf[ty].water && !(ShallowCheck && ty == cSHALLOWWATER))
							return 0;
					}
				}
				if(!i)
				{
					for(word k=0; k<ffc_count; k++)
					{
						if(ffcIsAt(k, tx2, ty2))
						{
							ffcdata const& ffc = tmpscr->ffcs[k];
							auto ty = combobuf[ffc.data].type;
							if(combo_class_buf[ty].water || (ShallowCheck && ty == cSHALLOWWATER))
							{
								return ffc.data;
							}
						}
					}
				}
				int32_t checkcombo = MAPCOMBO3(map, screen, layer, tx2, ty2, secrets);
				if (!(combobuf[checkcombo].walk&(1<<(b+4)))) return 0;
				if (iswater_type(combobuf[checkcombo].type)||(ShallowCheck && (combobuf[checkcombo].type == cSHALLOWWATER || (iswater_type(combobuf[checkcombo].type) && (combobuf[checkcombo].walk&(1<<b)) && (combobuf[checkcombo].usrflags&cflag4))))) 
				{
					if (i == 0) return checkcombo;
				}
			}
			return 0;
		}
	}
	else
	{
		int32_t b = 0;
		if(x&8) b+=2;
		if(y&8) b+=1;
		if (get_qr(qr_NO_SOLID_SWIM))
		{
			if (combobuf[combo].walk&(1<<b))
			{
				return 0;
			}						
		}
		if (!(combobuf[combo].walk&(1<<(b+4)))) return 0;
		return (((iswater_type(combobuf[combo].type) || (ShallowCheck && combobuf[combo].type == cSHALLOWWATER)) && !DRIEDLAKE)?combo:0);//These used to return booleans; returning the combo id of the water combo it caught is essential for Emily's proposed water changes.
	}
}

bool isdamage_type(int32_t type)
{
	switch(type)
	{
		case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
		case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
			return true;
	}
	return false;
}

bool ispitfall_type(int32_t type)
{
	return combo_class_buf[type].pit != 0;
}

bool ispitfall(int32_t combo)
{
    return ispitfall_type(combobuf[combo].type);
}

bool ispitfall(int32_t x, int32_t y)
{
	/*
	if(int32_t c = MAPFFCOMBO(x,y))
		return ispitfall(c);
	return ispitfall(MAPCOMBO(x,y)) || ispitfall(MAPCOMBOL(1,x,y)) || ispitfall(MAPCOMBOL(2,x,y));
	*/
	if(int32_t c = MAPFFCOMBO(x,y))
	{
		return ispitfall(c) ? true : false;
	}
	int32_t c = MAPCOMBOL(2,x,y);
	if(ispitfall(c)) return true;
	if(tmpscr2[1].valid!=0)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, &(tmpscr2[1]))) return false;
		}
		else
		{
			if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && _effectflag_layer(x,y,1, &(tmpscr2[1]))) return false;
		}
	}
	c = MAPCOMBOL(1,x,y);
	if(ispitfall(c)) return true;
	if(tmpscr2[0].valid!=0)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(0,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, &(tmpscr2[0]))) return false;
		}
		else
		{
			if (combobuf[MAPCOMBO2(0,x,y)].type == cBRIDGE && _effectflag_layer(x,y,1, &(tmpscr2[0]))) return false;
		}
	}
	c = MAPCOMBO(x,y);
	if(ispitfall(c)) return true;
	return false;
}

int32_t getpitfall(int32_t x, int32_t y) //Return the highest-layer active pit combo at the given position
{
	if(int32_t c = MAPFFCOMBO(x,y))
	{
		return ispitfall(c) ? c : 0;
	}
	int32_t c = MAPCOMBOL(2,x,y);
	if(ispitfall(c)) return c;
	if(tmpscr2[1].valid!=0)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, &(tmpscr2[1]))) return 0;
		}
		else
		{
			if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && _effectflag_layer(x,y,1, &(tmpscr2[1]))) return 0;
		}
	}
	c = MAPCOMBOL(1,x,y);
	if(ispitfall(c)) return c;
	if(tmpscr2[0].valid!=0)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(0,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, &(tmpscr2[0]))) return 0;
		}
		else
		{
			if (combobuf[MAPCOMBO2(0,x,y)].type == cBRIDGE && _effectflag_layer(x,y,1, &(tmpscr2[0]))) return 0;
		}
	}
	c = MAPCOMBO(x,y);
	if(ispitfall(c)) return c;
	return 0;
}
bool check_icy(newcombo const& cmb, int type)
{
	if(cmb.type != cICY)
		return false;
	switch(type)
	{
		case ICY_BLOCK:
			return cmb.usrflags&cflag1;
		case ICY_PLAYER:
			return cmb.usrflags&cflag2;
	}
	return false;
}
int get_icy(int x, int y, int type)
{
	int32_t c = MAPCOMBOL(2,x,y);
	if(check_icy(combobuf[c], type)) return c;
	if(tmpscr2[1].valid!=0)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, &(tmpscr2[1]))) return 0;
		}
		else
		{
			if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && _effectflag_layer(x,y,1, &(tmpscr2[1]))) return 0;
		}
	}
	c = MAPCOMBOL(1,x,y);
	if(check_icy(combobuf[c], type)) return c;
	if(tmpscr2[0].valid!=0)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(0,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, &(tmpscr2[0]))) return 0;
		}
		else
		{
			if (combobuf[MAPCOMBO2(0,x,y)].type == cBRIDGE && _effectflag_layer(x,y,1, &(tmpscr2[0]))) return 0;
		}
	}
	c = MAPCOMBO(x,y);
	if(check_icy(combobuf[c], type)) return c;
	return 0;
}

bool isSVLadder(int32_t x, int32_t y)
{
	if(x<0 || x>255 || y<0 || y>175)
        return false;
	
    mapscr *s1, *s2;
    s1=(tmpscr2->valid)?tmpscr2:tmpscr;
    s2=(tmpscr2[1].valid)?tmpscr2+1:tmpscr;
	
    int32_t combo = COMBOPOS(x,y);
    return (tmpscr->sflag[combo] == mfSIDEVIEWLADDER) || (combobuf[tmpscr->data[combo]].flag == mfSIDEVIEWLADDER) ||
		(s1->sflag[combo] == mfSIDEVIEWLADDER) || (combobuf[s1->data[combo]].flag == mfSIDEVIEWLADDER) ||
		(s2->sflag[combo] == mfSIDEVIEWLADDER) || (combobuf[s2->data[combo]].flag == mfSIDEVIEWLADDER);
}

bool isSVPlatform(int32_t x, int32_t y)
{
	if(x<0 || x>255 || y<0 || y>175)
        return false;
	
    mapscr *s1, *s2;
    s1=(tmpscr2->valid)?tmpscr2:tmpscr;
    s2=(tmpscr2[1].valid)?tmpscr2+1:tmpscr;
	
    int32_t combo = COMBOPOS(x,y);
    return (tmpscr->sflag[combo] == mfSIDEVIEWPLATFORM) || (combobuf[tmpscr->data[combo]].flag == mfSIDEVIEWPLATFORM) ||
		(s1->sflag[combo] == mfSIDEVIEWPLATFORM) || (combobuf[s1->data[combo]].flag == mfSIDEVIEWPLATFORM) ||
		(s2->sflag[combo] == mfSIDEVIEWPLATFORM) || (combobuf[s2->data[combo]].flag == mfSIDEVIEWPLATFORM);
}

bool checkSVLadderPlatform(int32_t x, int32_t y)
{
	return isSVPlatform(x,y) || (isSVLadder(x,y) && !isSVLadder(x,y-16));
}

bool isstepable(int32_t combo)                                  //can use ladder on it
{
    if(combo_class_buf[combobuf[combo].type].ladder_pass) return true;
	if(combo_class_buf[combobuf[combo].type].pit)
	{
		if(combobuf[combo].usrflags&cflag4)
		{
			int32_t ldrid = current_item_id(itype_ladder);
			return (ldrid > -1 && itemsbuf[ldrid].flags & item_flag1);
		}
	}
	return false;
}

bool isHSGrabbable(newcombo const& cmb)
{
	if(cmb.type == cHSGRAB) return true;
	return cmb.genflags & cflag1;
}

bool isSwitchHookable(newcombo const& cmb)
{
	if(cmb.type == cSWITCHHOOK) return true;
	return cmb.genflags & cflag2;
}

bool check_hshot(int32_t layer, int32_t x, int32_t y, bool switchhook, int32_t *retcpos, int32_t *retffcpos)
{
	int32_t cpos = -1;
	if(retcpos)
	{
		int32_t id = MAPCOMBO2(layer-1,x,y);
		if(id > 0)
		{
			newcombo const& cmb = combobuf[id];
			cpos = (switchhook ? isSwitchHookable(cmb) : isHSGrabbable(cmb)) ? COMBOPOS(x,y) : -1;
		}
	}
	int32_t ffcpos = -1;
	if(retffcpos && !get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		word c = tmpscr->numFFC();
		for(word i=0; i<c; i++)
		{
			if(ffcIsAt(i, x, y))
			{
				ffcdata& ffc2 = tmpscr->ffcs[i];
				newcombo const& cmb = combobuf[ffc2.data];
				if (switchhook ? isSwitchHookable(cmb) : isHSGrabbable(cmb))
				{
					ffcpos = i;
					break;
				}
			}
		}
	}
	if (retcpos && cpos > -1) *retcpos = cpos;
	if (retffcpos && ffcpos > -1) *retffcpos = ffcpos;
	return (cpos > -1 || ffcpos > -1);
}

bool ishookshottable(int32_t bx, int32_t by)
{
	if(!_walkflag(bx,by,1))
		return true;
	
	if (collide_object(bx, by, 1, 1))
		return false;
	
	bool ret = true;
	for(int32_t i=2; i>=0; i--)
	{
		int32_t c = MAPCOMBO2(i-1,bx,by);
		int32_t t = combobuf[c].type;
		
		if(i == 0 && (t == cHOOKSHOTONLY || t == cLADDERHOOKSHOT)) return true;
		
		bool dried = (iswater_type(t) && DRIEDLAKE);
		
		int32_t b=1;
		
		if(bx&8) b<<=2;
		
		if(by&8) b<<=1;
		
		if(combobuf[c].walk&b && !dried && !(combo_class_buf[t].ladder_pass && t!=cLADDERONLY) && t!=cHOOKSHOTONLY)
			ret = false;
	}
	
	return ret;
}

bool ishookshottable(int32_t map, int32_t screen, int32_t bx, int32_t by)
{
	if (map < 0 || screen < 0) return false;
		
	mapscr *m = &TheMaps[(map*MAPSCRS)+screen];
	
	if(m->valid==0) return false;
	
	if(!_walkflag(bx,by,1, m))
		return true;
	
	if (collide_object(bx, by, 1, 1))
		return false;
	
	bool ret = true;
	for(int32_t i=2; i>=0; i--)
	{
		int32_t c = MAPCOMBO3(map, screen, i-1,bx,by);
		int32_t t = combobuf[c].type;
		
		if(i == 0 && (t == cHOOKSHOTONLY || t == cLADDERHOOKSHOT)) return true;
		
		//bool dried = (iswater_type(t) && DRIEDLAKE);
		
		int32_t b=1;
		
		if(bx&8) b<<=2;
		
		if(by&8) b<<=1;
		
		if(combobuf[c].walk&b && !(combo_class_buf[t].ladder_pass && t!=cLADDERONLY) && t!=cHOOKSHOTONLY)
			ret = false;
	}
	
	return ret;
}

bool hiddenstair(int32_t tmp,bool redraw)                       // tmp = index of tmpscr[]
{
    return hiddenstair2(tmpscr + tmp,redraw);
}

bool hiddenstair2(mapscr *s,bool redraw)                       // tmp = index of tmpscr[]
{
    
    if((s->stairx || s->stairy) && s->secretcombo[sSTAIRS])
    {
        int32_t di = COMBOPOS(s->stairx,s->stairy);
        s->data[di] = s->secretcombo[sSTAIRS];
        s->cset[di] = s->secretcset[sSTAIRS];
        s->sflag[di] = s->secretflag[sSTAIRS];
        
        if(redraw)
            putcombo(scrollbuf,s->stairx,s->stairy,s->data[di],s->cset[di]);
            
        return true;
    }
    
    return false;
}

bool remove_screenstatecombos(int32_t tmp, int32_t what1, int32_t what2)
{
	mapscr *s = tmpscr + tmp;
	mapscr *t = tmpscr2;
	return remove_screenstatecombos2(s, t, what1, what2);
}

bool remove_screenstatecombos2(mapscr *s, mapscr *t, int32_t what1, int32_t what2)
{
	bool didit=false;
	
	for(int32_t i=0; i<176; i++)
	{
		newcombo const& cmb = combobuf[s->data[i]];
		if(cmb.usrflags&cflag16) continue; //custom state instead of normal state
		if((cmb.type == what1) || (cmb.type== what2))
		{
			s->data[i]++;
			didit=true;
		}
	}
	
	if(t)
	{
		for(int32_t j=0; j<6; j++)
		{
			for(int32_t i=0; i<176; i++)
			{
				newcombo const& cmb = combobuf[t[j].data[i]];
				if(cmb.usrflags&cflag16) continue; //custom state instead of normal state
				if((cmb.type== what1) || (cmb.type== what2))
				{
					t[j].data[i]++;
					didit=true;
				}
			}
		}
	}
	
	if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		word c = tmpscr->numFFC();
		for(word i=0; i<c; i++)
		{
			ffcdata& ffc2 = tmpscr->ffcs[i];
			newcombo const& cmb = combobuf[ffc2.data];
			if(cmb.usrflags&cflag16) continue; //custom state instead of normal state
			if((cmb.type== what1) || (cmb.type== what2))
			{
				zc_ffc_modify(ffc2, 1);
				didit=true;
			}
		}
	}
	
	
	
	return didit;
}

bool remove_xstatecombos(int32_t tmp, byte xflag)
{
	return remove_xstatecombos(tmp, (currmap*MAPSCRSNORMAL)+homescr, xflag);
}
bool remove_xstatecombos(int32_t tmp, int32_t mi, byte xflag)
{
	mapscr *s = tmpscr + tmp;
	mapscr *t = tmpscr2;
	return remove_xstatecombos2(s, t, mi, xflag, false);
}
bool remove_xstatecombos2(mapscr *s, mapscr *t, byte xflag, bool triggers)
{
	return remove_xstatecombos2(s, t, (currmap*MAPSCRSNORMAL)+homescr, xflag, triggers);
}
bool remove_xstatecombos2(mapscr *s, mapscr *t, int32_t mi, byte xflag, bool triggers)
{
	bool didit=false;
	if(!getxmapflag(mi, 1<<xflag)) return false;
	mapscr *scrs[7] = {s};
	if(t)
	{
		scrs[1] = &t[0];
		scrs[2] = &t[1];
		scrs[3] = &t[2];
		scrs[4] = &t[3];
		scrs[5] = &t[4];
		scrs[6] = &t[5];
	}
	for(uint lyr = 0; lyr < 7; ++lyr)
	{
		mapscr* scr = scrs[lyr];
		if(!scr) continue;
		for(uint pos = 0; pos < 176; ++pos)
		{
			newcombo const& cmb = combobuf[scr->data[pos]];
			if(triggers && force_ex_trigger(lyr,pos,xflag))
				didit = true;
			else switch(cmb.type)
			{
				case cLOCKBLOCK: case cLOCKBLOCK2:
				case cBOSSLOCKBLOCK: case cBOSSLOCKBLOCK2:
				case cCHEST: case cCHEST2:
				case cLOCKEDCHEST: case cLOCKEDCHEST2:
				case cBOSSCHEST: case cBOSSCHEST2:
				{
					if(!(cmb.usrflags&cflag16)) continue; //custom state instead of normal state
					if(cmb.attribytes[5] == xflag)
					{
						scr->data[pos]++;
						didit=true;
					}
					break;
				}
			}
		}
	}
	if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		word c = s->numFFC();
		for(word i=0; i<c; i++)
		{
			ffcdata& ffc2 = s->ffcs[i];
			newcombo const& cmb = combobuf[ffc2.data];
			if(triggers && force_ex_trigger_ffc(i,xflag))
				didit = true;
			else switch(cmb.type)
			{
				case cLOCKBLOCK: case cLOCKBLOCK2:
				case cBOSSLOCKBLOCK: case cBOSSLOCKBLOCK2:
				case cCHEST: case cCHEST2:
				case cLOCKEDCHEST: case cLOCKEDCHEST2:
				case cBOSSCHEST: case cBOSSCHEST2:
				{
					if(!(cmb.usrflags&cflag16)) continue; //custom state instead of normal state
					if(cmb.attribytes[5] == xflag)
					{
						zc_ffc_modify(ffc2, 1);
						didit=true;
					}
					break;
				}
			}
		}
	}
	
	return didit;
}

void clear_xstatecombos(int32_t tmp)
{
	clear_xstatecombos(tmp, (currmap*MAPSCRSNORMAL)+homescr);
}
void clear_xstatecombos(int32_t tmp, int32_t mi)
{
	mapscr *s = tmpscr + tmp;
	mapscr *t = tmpscr2;
	clear_xstatecombos2(s, t, mi, tmp==0);
}
void clear_xstatecombos2(mapscr *s, mapscr *t)
{
	clear_xstatecombos2(s, t, (currmap*MAPSCRSNORMAL)+homescr);
}
void clear_xstatecombos2(mapscr *s, mapscr *t, int32_t mi, bool triggers)
{
	for(byte q = 0; q < 32; ++q)
	{
		remove_xstatecombos2(s,t,mi,q,triggers);
	}
}


bool remove_xdoors(int32_t tmp, uint dir, uint ind)
{
	return remove_xdoors(tmp, (currmap*MAPSCRSNORMAL)+homescr, dir, ind);
}
bool remove_xdoors(int32_t tmp, int32_t mi, uint dir, uint ind)
{
	mapscr *s = tmpscr + tmp;
	mapscr *t = tmpscr2;
	return remove_xdoors2(s, t, mi, dir, ind, false);
}
bool remove_xdoors2(mapscr *s, mapscr *t, uint dir, uint ind, bool triggers)
{
	return remove_xdoors2(s, t, (currmap*MAPSCRSNORMAL)+homescr, dir, ind, triggers);
}
bool remove_xdoors2(mapscr *s, mapscr *t, int32_t mi, uint dir, uint ind, bool triggers)
{
	bool didit=false;
	if(!getxdoor(mi, dir, ind)) return false;
	mapscr *scrs[7] = {s};
	if(t)
	{
		scrs[1] = &t[0];
		scrs[2] = &t[1];
		scrs[3] = &t[2];
		scrs[4] = &t[3];
		scrs[5] = &t[4];
		scrs[6] = &t[5];
	}
	for(uint lyr = 0; lyr < 7; ++lyr)
	{
		mapscr* scr = scrs[lyr];
		if(!scr) continue;
		for(uint pos = 0; pos < 176; ++pos)
		{
			newcombo const& cmb = combobuf[scr->data[pos]];
			if(triggers && force_ex_door_trigger(lyr,pos,dir,ind))
				didit = true;
			else; //future door combo types?
		}
	}
	if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		word c = s->numFFC();
		for(word i=0; i<c; i++)
		{
			ffcdata& ffc2 = s->ffcs[i];
			newcombo const& cmb = combobuf[ffc2.data];
			if(triggers && force_ex_door_trigger_ffc(i,dir,ind))
				didit = true;
			else; //future door combo types?
		}
	}
	
	return didit;
}
void clear_xdoors(int32_t tmp)
{
	clear_xdoors(tmp, (currmap*MAPSCRSNORMAL)+homescr);
}
void clear_xdoors(int32_t tmp, int32_t mi)
{
	mapscr *s = tmpscr + tmp;
	mapscr *t = tmpscr2;
	clear_xdoors2(s, t, mi, tmp==0);
}
void clear_xdoors2(mapscr *s, mapscr *t)
{
	clear_xdoors2(s, t, (currmap*MAPSCRSNORMAL)+homescr);
}
void clear_xdoors2(mapscr *s, mapscr *t, int32_t mi, bool triggers)
{
	for(uint dir = 0; dir < 4; ++dir)
		for(uint ind = 0; ind < 8; ++ind)
			remove_xdoors2(s, t, mi, dir, ind, triggers);
}

bool remove_lockblocks(int32_t tmp)
{
    return remove_screenstatecombos(tmp, cLOCKBLOCK, cLOCKBLOCK2);
}

bool remove_bosslockblocks(int32_t tmp)
{
    return remove_screenstatecombos(tmp, cBOSSLOCKBLOCK, cBOSSLOCKBLOCK2);
}

bool remove_chests(int32_t tmp)                 // tmp = index of tmpscr[]
{
    return remove_screenstatecombos(tmp, cCHEST, cCHEST2);
}

bool remove_lockedchests(int32_t tmp)                 // tmp = index of tmpscr[]
{
    return remove_screenstatecombos(tmp, cLOCKEDCHEST, cLOCKEDCHEST2);
}

bool remove_bosschests(int32_t tmp)                 // tmp = index of tmpscr[]
{
    return remove_screenstatecombos(tmp, cBOSSCHEST, cBOSSCHEST2);
}


bool overheadcombos(mapscr *s)
{
    for(int32_t i=0; i<176; i++)
    {
//    if (combobuf[s->data[i]].type==cOLD_OVERHEAD)
        if(combo_class_buf[combobuf[s->data[i]].type].overhead)
        {
            return true;
        }
    }
    
    return false;
}

void delete_fireball_shooter(mapscr *s, int32_t i)
{
    int32_t cx=0, cy=0;
    int32_t ct=combobuf[s->data[i]].type;
    
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
    
    for(int32_t j=0; j<guys.Count(); j++)
    {
        // Finds the smallest enemy ID
        if((int32_t(guys.spr(j)->x)==cx)&&(int32_t(guys.spr(j)->y)==cy)&&(guysbuf[(guys.spr(j)->id)&0xFFF].flags & guy_fire))
        {
            guys.del(j);
        }
    }
}

int32_t findtrigger(int32_t scombo, bool ff)
{
    int32_t checkflag=0;
    int32_t ret = 0;
    
	word c = ff ? tmpscr->numFFC() : 176;
	bool sflag = false;
    for(word j=0; j<c; j++)
    {
        for(int32_t layer = -2; layer < 6; ++layer)
		{
			if(layer == -2)
			{
				if(ff)
					checkflag = combobuf[tmpscr->ffcs[j].data].flag;
				else continue;
			}
			else
			{
				if(layer>-1 && !tmpscr2[layer].valid) continue;
                if(sflag)
                    checkflag = (layer>-1 ? tmpscr2[layer].sflag[j] : tmpscr->sflag[j]);
                else
                    checkflag = (layer>-1 ? combobuf[tmpscr2[layer].data[j]].flag : combobuf[tmpscr->data[j]].flag);
				sflag = !sflag;
				if (sflag) --layer;
			}
			switch(checkflag)
			{
				case mfANYFIRE:
				case mfSTRONGFIRE:
				case mfMAGICFIRE:
				case mfDIVINEFIRE:
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
					[[fallthrough]];
				default:
					break;
			}
		}
    }
    
    return ret;
}

// single:
// >-1 : the singular triggering combo
// -1: triggered by some other cause
void hidden_entrance(int32_t tmp,bool refresh, bool high16only,int32_t single) //Perhaps better known as 'Trigger Secrets'
{
	//There are no calls to 'hidden_entrance' in the code where tmp != 0
	Z_eventlog("%sScreen Secrets triggered%s.\n",
			   single>-1? "Restricted ":"",
			   single==-2? " by the 'Enemies->Secret' screen flag":
			   single==-3? " by the 'Secrets' screen state" :
			   single==-4? " by a script":
			   single==-5? " by Items->Secret":
			   single==-6? " by Generic Combo":
			   single==-7? " by Light Triggers":
			   single==-8? " by SCC":
			   single==-9? " by Cheat (Temp)":
			   single==-10? " by Cheat (Perm)":
			   "");
	if(single < 0)
		triggered_screen_secrets = true;
	for(auto lyr = 0; lyr < 7; ++lyr)
	{
		for(auto pos = 0; pos < 176; ++pos)
		{
			newcombo const& cmb = combobuf[FFCore.tempScreens[lyr]->data[pos]];
			if(cmb.triggerflags[2] & combotriggerSECRETSTR)
			{
				do_trigger_combo(lyr,pos,ctrigSECRETS);
			}
		}
	}
	if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		word c = tmpscr->numFFC();
		for(word i=0; i<c; i++)
		{
			ffcdata& ffc = tmpscr->ffcs[i];
			newcombo const& cmb = combobuf[ffc.data];
			if(cmb.triggerflags[2] & combotriggerSECRETSTR)
				do_trigger_combo_ffc(i);
		}
	}
	hidden_entrance2(tmpscr + tmp, tmpscr2, high16only, single);
}
void hidden_entrance2(mapscr *s, mapscr *t, bool high16only,int32_t single) //Perhaps better known as 'Trigger Secrets'
{
	if (replay_is_active())
		replay_step_comment(fmt::format("trigger secrets scr={}", currscr));

	/*
	mapscr *s = tmpscr + tmp;
	mapscr *t = tmpscr2;
	*/
	int32_t ft=0; //Flag trigger?
	int32_t msflag=0; // Misc. secret flag
	
	for(int32_t i=0; i<176; i++) //Do the 'trigger flags' (non 16-31)
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
			int32_t newflag = -1;
			
			
			for(int32_t iter=0; iter<2; ++iter)
			{
				putit=true;
				int32_t checkflag=combobuf[s->data[i]].flag; //Inherent
				
				if(iter==1) checkflag=s->sflag[i]; //Placed
				
				switch(checkflag)
				{
				case mfANYFIRE:
					ft=sBCANDLE;
					break;
					
				case mfSTRONGFIRE:
					ft=sRCANDLE;
					break;
					
				case mfMAGICFIRE:
					ft=sWANDFIRE;
					break;
					
				case mfDIVINEFIRE:
					ft=sDIVINEFIRE;
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
				
				case mfSECRETSNEXT:
					ft=sSECNEXT;
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
					if(ft==sSECNEXT)
					{
						s->data[i]++;
					}
					else
					{
						s->data[i] = s->secretcombo[ft];
						s->cset[i] = s->secretcset[ft];
					}
					newflag = s->secretflag[ft];
					screen_combo_modify_postroutine(s,i);
				}
			}
			
			if(newflag >-1) s->sflag[i] = newflag; //Tiered secret
			
			if (t)
			{
				for(int32_t j=0; j<6; j++)  //Layers
				{
					if(single>=0 && i!=single) continue; //If it's got a singular flag and i isn't where the flag is
					
					int32_t newflag2 = -1;
					
					// Remember the misc. secret flag; if triggered, use this instead
					if(t[j].sflag[i]>=mfSECRETS01 && t[j].sflag[i]<=mfSECRETS16)
						msflag=sSECRET01+(t[j].sflag[i]-mfSECRETS01);
					else if(combobuf[t[j].data[i]].flag>=mfSECRETS01 && combobuf[t[j].data[i]].flag<=mfSECRETS16)
						msflag=sSECRET01+(combobuf[t[j].data[i]].flag-mfSECRETS01);
					else
						msflag=0;
						
					for(int32_t iter=0; iter<2; ++iter)
					{
						putit=true;
						int32_t checkflag=combobuf[t[j].data[i]].flag; //Inherent
						
						if(iter==1) checkflag=t[j].sflag[i];  //Placed
						
						switch(checkflag)
						{
							case mfANYFIRE:
								ft=sBCANDLE;
								break;
							
							case mfSTRONGFIRE:
								ft=sRCANDLE;
								break;
							
							case mfMAGICFIRE:
								ft=sWANDFIRE;
								break;
							
							case mfDIVINEFIRE:
								ft=sDIVINEFIRE;
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
							
							case mfSECRETSNEXT:
								ft=sSECNEXT;
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
							
							if(ft==sSECNEXT)
							{
								t[j].data[i]++;
							}
							else
							{
								t[j].data[i] = t[j].secretcombo[ft];
								t[j].cset[i] = t[j].secretcset[ft];
							}
							newflag2 = t[j].secretflag[ft];
							int32_t c=t[j].data[i];
							int32_t cs=t[j].cset[i];
							
							if(combobuf[c].type==cSPINTILE1)  //Surely this means we can have spin tiles on layers 3+? Isn't that bad? ~Joe123
								addenemy((i&15)<<4,i&0xF0,(cs<<12)+eSPINTILE1,combobuf[c].o_tile+zc_max(1,combobuf[c].frames));
						}
					}
					
					if(newflag2 >-1) t[j].sflag[i] = newflag2;  //Tiered secret
				}
			}
		}
	}
	
	word c = s->numFFC();
	for(word i=0; i<c; i++) //FFC 'trigger flags'
	{
		if(single>=0) if(i+176!=single) continue;
		
		bool putit;
		
		if((!high16only)||(single>=0))
		{
			//for (int32_t iter=0; iter<1; ++iter) // Only one kind of FFC flag now.
			{
				putit=true;
				int32_t checkflag=combobuf[s->ffcs[i].data].flag; //Inherent
				
				//No placed flags yet
				switch(checkflag)
				{
					case mfANYFIRE:
						ft=sBCANDLE;
						break;
						
					case mfSTRONGFIRE:
						ft=sRCANDLE;
						break;
						
					case mfMAGICFIRE:
						ft=sWANDFIRE;
						break;
						
					case mfDIVINEFIRE:
						ft=sDIVINEFIRE;
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
					
					case mfSECRETSNEXT:
						ft=sSECNEXT;
						break;
						
					default:
						putit = false;
						break;
				}
				
				if(putit)  //Change the ffc's combo
				{
					if(ft==sSECNEXT)
					{
						zc_ffc_modify(s->ffcs[i], 1);
					}
					else
					{
						zc_ffc_set(s->ffcs[i], s->secretcombo[ft]);
						s->ffcs[i].cset = s->secretcset[ft];
					}
				}
			}
		}
	}
	
	if(checktrigger) //Hit all triggers->16-31
	{
		checktrigger=false;
		
		if(tmpscr->flags6&fTRIGGERF1631)
		{
			int32_t tr = findtrigger(-1,false);  //Normal flags
			
			if(tr)
			{
				Z_eventlog("Hit All Triggers->16-31 not fulfilled (%d trigger flag%s remain).\n", tr, tr>1?"s":"");
				goto endhe;
			}
		}
	}
	
	for(int32_t i=0; i<176; i++) // Do the 16-31 secrets
	{
		//If it's an enemies->secret screen, only do the high 16 if told to
		//That way you can have secret and burn/bomb entrance separately
		bool old_enem_secret = get_qr(qr_ENEMIES_SECRET_ONLY_16_31);
		if(((!(old_enem_secret && (s->flags2&fCLEARSECRET)) /*Enemies->Secret*/ && single < 0) || high16only || s->flags4&fENEMYSCRTPERM))
		{
			int32_t newflag = -1;
			
			for(int32_t iter=0; iter<2; ++iter)
			{
				int32_t checkflag=combobuf[s->data[i]].flag; //Inherent
				
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
				for(int32_t j=0; j<6; j++)  //Layers
				{
					int32_t newflag2 = -1;
					
					for(int32_t iter=0; iter<2; ++iter)
					{
						int32_t checkflag=combobuf[t[j].data[i]].flag; //Inherent
						
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
	}
	
	for(word i=0; i<c; i++) // FFCs
	{
		if((!(s->flags2&fCLEARSECRET) /*Enemies->Secret*/ && single < 0) || high16only || s->flags4&fENEMYSCRTPERM)
		{
			int32_t checkflag=combobuf[s->ffcs[i].data].flag; //Inherent
			
			//No placed flags yet
			if((checkflag > 15)&&(checkflag < 32)) //If we find a flag, change the combo
			{
				zc_ffc_set(s->ffcs[i], s->secretcombo[checkflag - 16 + 4]);
				s->ffcs[i].cset = s->secretcset[checkflag-16+4];
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
    int32_t ffcombos[4] = {-1, -1, -1, -1};
    bool single16=false;
    int32_t scombo=-1;
    
    for(int32_t i=-1; i<6; i++)  // Layers. -1 = screen.
    {
        if(MAPFLAG2(i,x,y)==flag || MAPFLAG2(i,x+15,y)==flag ||
                MAPFLAG2(i,x,y+15)==flag || MAPFLAG2(i,x+15,y+15)==flag)
        {
            foundflag=true;
            foundnflag=true;
        }
    }
    
    for(int32_t i=-1; i<6; i++)  // Layers. -1 = screen.
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
    
    for(int32_t i=-1; i<6; i++)  // Look for Trigger->Self on all layers
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
        int32_t tr = findtrigger(-1,false);  //Normal flags
        
        if(tr)
        {
            Z_eventlog("Hit All Triggers->Perm Secret not fulfilled (%d trigger flag%s remain).\n", tr, tr>1?"s":"");
            setflag=false;
        }
		
		if(!tr && !get_qr(qr_ALLTRIG_PERMSEC_NO_TEMP))
		{
			hidden_entrance(0,true,(tmpscr->flags6&fTRIGGERF1631));
		}
    }
    
    if(setflag && canPermSecret())
        if(!(tmpscr->flags5&fTEMPSECRETS))
            setmapflag(mSECRET);
            
    return true;
}

bool triggerfire(int x, int y, bool setflag, bool any, bool strong, bool magic, bool divine)
{
	int trigflags = (any?combotriggerANYFIRE:0)
		| (strong?combotriggerSTRONGFIRE:0)
		| (magic?combotriggerMAGICFIRE:0)
		| (divine?combotriggerDIVINEFIRE:0);
	if(!trigflags) return false;
	bool ret = false;
	if(any)
		ret = ret||findentrance(x,y,mfANYFIRE,setflag);
	if(strong)
		ret = ret||findentrance(x,y,mfSTRONGFIRE,setflag);
	if(magic)
		ret = ret||findentrance(x,y,mfMAGICFIRE,setflag);
	if(divine)
		ret = ret||findentrance(x,y,mfDIVINEFIRE,setflag);
	
	std::set<int> poses({COMBOPOS_B(x,y),COMBOPOS_B(x,y+15),COMBOPOS_B(x+15,y),COMBOPOS_B(x+15,y+15)});
	for(int q = 0; q < 7; ++q)
	{
		mapscr* m = FFCore.tempScreens[q];
		for(int pos : poses)
			if(pos != -1 && combobuf[m->data[pos]].triggerflags[2] & trigflags)
			{
				do_trigger_combo(q,pos);
				ret = true;
			}
	}
	word c = tmpscr->numFFC();
	for(word i=0; i<c; i++)
	{
		ffcdata& ffc = tmpscr->ffcs[i];
		if((combobuf[ffc.data].triggerflags[2] & trigflags)
			&& ffc.collide(x,y,16,16))
		{
			do_trigger_combo_ffc(i);
			ret = true;
		}
	}
	return ret;
}

void update_slopes()
{
	for (auto& p : slopes)
	{
		slope_object& s = p.second;
		s.updateslope(); //sets old x/y poses
	}
}

void update_freeform_combos()
{
	ffscript_engine(false);
	if ( !FFCore.system_suspend[susptUPDATEFFC] )
	{
		word c = tmpscr->numFFC();
		for(word i=0; i<c; i++)
		{
			ffcdata& thisffc = tmpscr->ffcs[i];
			// Combo 0?
			if(thisffc.data==0)
				continue;
				
			// Changer?
			if(thisffc.flags&ffc_changer)
				continue;
				
			// Stationary?
			if(thisffc.flags&ffc_stationary)
				continue;
				
			// Frozen because Hero's holding up an item?
			if(Hero.getHoldClk()>0 && (thisffc.flags&ffc_ignoreholdup)==0)
				continue;
				
			// Check for changers
			if(thisffc.link==0)
			{
				for(word j=0; j<c; j++)
				{
					ffcdata& otherffc = tmpscr->ffcs[j];
					// Combo 0?
					if(otherffc.data==0)
						continue;
						
					// Not a changer?
					if(!(otherffc.flags&ffc_changer))
						continue;
						
					// Ignore this changer? (ffposx and ffposy are last changer position)
					if((otherffc.x.getInt()==ffposx[i]&&otherffc.y.getInt()==ffposy[i]) || thisffc.flags&ffc_ignorechanger)
						continue;
						
					if((isonline(thisffc.x.getZLong(), thisffc.y.getZLong(), ffprvx[i],ffprvy[i], otherffc.x.getZLong(), otherffc.y.getZLong()) || // Along the line, or...
						( // At exactly the same position, 
							(thisffc.x==otherffc.x && thisffc.y==otherffc.y)) 
							||
							//or imprecision and close enough
							( (thisffc.flags&ffc_imprecisionchanger) && ((abs(thisffc.x.getZLong() - otherffc.x.getZLong()) < 10000) && abs(thisffc.y.getZLong() - otherffc.y.getZLong()) < 10000) )
						)
					&& //and...
						(ffprvx[i]>-10000000 && ffprvy[i]>-10000000)) // This isn't the first frame on this screen
					{
						zc_ffc_changer(thisffc, otherffc, i, j);
						break;
					}
				}
			}
			
			if(thisffc.link ? !tmpscr->ffcs[thisffc.link - 1].delay : !thisffc.delay)
			{
				if(thisffc.link&&(thisffc.link-1)!=i)
				{
					ffprvx[i] = thisffc.x.getZLong();
					ffprvy[i] = thisffc.y.getZLong();
					thisffc.x+=tmpscr->ffcs[thisffc.link-1].vx;
					thisffc.y+=tmpscr->ffcs[thisffc.link-1].vy;
				}
				else
				{
					ffprvx[i] = thisffc.x.getZLong();
					ffprvy[i] = thisffc.y.getZLong();
					thisffc.x+=thisffc.vx;
					thisffc.y+=thisffc.vy;
					thisffc.vx+=thisffc.ax;
					thisffc.vy+=thisffc.ay;
					
					
					if(get_qr(qr_OLD_FFC_SPEED_CAP))
					{
						if(thisffc.vx>128) thisffc.vx=128;
						
						if(thisffc.vx<-128) thisffc.vx=-128;
						
						if(thisffc.vy>128) thisffc.vy=128;
						
						if(thisffc.vy<-128) thisffc.vy=-128;
					}
				}
			}
			else
			{
				if(!thisffc.link || (thisffc.link-1)==i)
					thisffc.delay--;
			}
			
			// Check if the FFC's off the side of the screen
			
			// Left
			if(thisffc.x<-32)
			{
				if(tmpscr->flags6&fWRAPAROUNDFF)
				{
					thisffc.x = 288+(thisffc.x+32);
					thisffc.solid_update(false);
					ffprvy[i] = thisffc.y.getZLong();
					ffposx[i]=-1000; // Re-enable previous changer
					ffposy[i]=-1000;
				}
				else if(thisffc.x<-64)
				{
					zc_ffc_set(thisffc, 0);
					thisffc.flags&=~ffc_carryover;
				}
			}
			// Right
			else if(thisffc.x>=288)
			{
				if(tmpscr->flags6&fWRAPAROUNDFF)
				{
					thisffc.x = thisffc.x-288-32;
					thisffc.solid_update(false);
					ffprvy[i] = thisffc.y.getZLong();
					ffposx[i]=-1000;
					ffposy[i]=-1000;
				}
				else
				{
					zc_ffc_set(thisffc, 0);
					thisffc.flags&=~ffc_carryover;
				}
			}
			
			// Top
			if(thisffc.y<-32)
			{
				if(tmpscr->flags6&fWRAPAROUNDFF)
				{
					thisffc.y = 208+(thisffc.y+32);
					thisffc.solid_update(false);
					ffprvx[i] = thisffc.x.getZLong();
					ffposx[i]=-1000;
					ffposy[i]=-1000;
				}
				else if(thisffc.y<-64)
				{
					zc_ffc_set(thisffc, 0);
					thisffc.flags&=~ffc_carryover;
				}
			}
			// Bottom
			else if(thisffc.y>=208)
			{
				if(tmpscr->flags6&fWRAPAROUNDFF)
				{
					thisffc.y = thisffc.y-208-32;
					thisffc.solid_update(false);
					ffprvy[i] = thisffc.x.getZLong();
					ffposx[i]=-1000;
					ffposy[i]=-1000;
				}
				else
				{
					zc_ffc_set(thisffc, 0);
					thisffc.flags&=~ffc_carryover;
				}
			}
			thisffc.solid_update();
		}
	}
}

bool hitcombo(int32_t x, int32_t y, int32_t combotype, byte layers)
{
	for(int q = 0; q < 7; ++q)
	{
		if(layers&(1<<q)) //if layer is to be checked
			if(COMBOTYPE2(q-1,x,y)==combotype) //matching type
				return true;
	}
	return false;
}

int gethitcombo(int32_t x, int32_t y, int32_t combotype, byte layers)
{
	for(int q = 0; q < 7; ++q)
	{
		if(layers&(1<<q)) //if layer is to be checked
			if(COMBOTYPE2(q-1,x,y)==combotype) //matching type
				return MAPCOMBO2(q-1,x,y);
	}
	return -1;
}

bool hitflag(int32_t x, int32_t y, int32_t flagtype, byte layers)
{
	for(int q = 0; q < 7; ++q)
	{
		if(layers&(1<<q)) //if layer is to be checked
			if(MAPFLAG2(q-1,x,y)==flagtype||MAPCOMBOFLAG2(q-1,x,y)==flagtype) //matching flag
				return true;
	}
	return false;
}

optional<int> nextscr(int map, int screen, int dir, bool normal)
{
	mapscr* m = nullptr;
	if(map==currmap && screen==currscr)
		m = tmpscr;
	else if((map*MAPSCRS)+screen < TheMaps.size())
		m = &TheMaps[(map*MAPSCRS)+screen];
	int index = (tmpscr->sidewarpindex >> (dir*2))&3;
	bool warp = false;
	// Fun fact: when a scrolling warp is triggered, this function
	// is never even called! - Saf
	if(m && m->sidewarptype[index] == 3) // scrolling warp
	{
		switch(dir)
		{
			case up:
				warp = (tmpscr->flags2&wfUP);
				break;
				
			case down:
				warp = (tmpscr->flags2&wfDOWN);
				break;
				
			case left:
				warp = (tmpscr->flags2&wfLEFT);
				break;
				
			case right:
				warp = (tmpscr->flags2&wfRIGHT);
				break;
		}
	}
	if(m && warp)
	{
		auto const& dm = DMaps[tmpscr->sidewarpdmap[index]];
		map = dm.map;
		screen = tmpscr->sidewarpscr[index] + dm.xoff;
	}
	else
	{
		switch(dir)
		{
			case up:
				screen -= 16;
				break;
				
			case down:
				screen += 16;
				break;
				
			case left:
				screen -= 1;
				break;
				
			case right:
				screen += 1;
				break;
		}
	}
	
	if(unsigned(screen) < 0x80 && unsigned(map) < map_count)
		return (map*(normal ? MAPSCRSNORMAL : MAPSCRS)) + screen;
	return nullopt;
}
optional<int> nextscr(int mi, int dir, bool normal)
{
	return nextscr(mi/(normal?MAPSCRSNORMAL:MAPSCRS),
		mi%(normal?MAPSCRSNORMAL:MAPSCRS), dir, normal);
}
optional<int> nextscr(int dir, bool normal)
{
	return nextscr(currmap, currscr, dir, normal);
}

void bombdoor(int32_t x,int32_t y)
{
    if(tmpscr->door[0]==dBOMB && isinRect(x,y,100,0,139,48))
    {
        tmpscr->door[0]=dBOMBED;
        putdoor(scrollbuf,0,0,dBOMBED);
        setmapflag(mDOOR_UP);
        markBmap(-1);
        
        if(auto v = nextscr(up, true))
        {
            setmapflag(*v, mDOOR_DOWN);
            markBmap(-1,*v-(get_currdmap()<<7));
        }
    }
    
    if(tmpscr->door[1]==dBOMB && isinRect(x,y,100,112,139,176))
    {
        tmpscr->door[1]=dBOMBED;
        putdoor(scrollbuf,0,1,dBOMBED);
        setmapflag(mDOOR_DOWN);
        markBmap(-1);
        
        if(auto v = nextscr(down, true))
        {
            setmapflag(*v, mDOOR_UP);
            markBmap(-1,*v-(get_currdmap()<<7));
        }
    }
    
    if(tmpscr->door[2]==dBOMB && isinRect(x,y,0,60,48,98))
    {
        tmpscr->door[2]=dBOMBED;
        putdoor(scrollbuf,0,2,dBOMBED);
        setmapflag(mDOOR_LEFT);
        markBmap(-1);
        
        if(auto v = nextscr(left, true))
        {
            setmapflag(*v, mDOOR_RIGHT);
            markBmap(-1,*v-(get_currdmap()<<7));
        }
    }
    
    if(tmpscr->door[3]==dBOMB && isinRect(x,y,192,60,240,98))
    {
        tmpscr->door[3]=dBOMBED;
        putdoor(scrollbuf,0,3,dBOMBED);
        setmapflag(mDOOR_RIGHT);
        markBmap(-1);
        
        if(auto v = nextscr(right, true))
        {
            setmapflag(*v, mDOOR_LEFT);
            markBmap(-1,*v-(get_currdmap()<<7));
        }
    }
}

void draw_cmb(BITMAP* dest, int32_t x, int32_t y, int32_t cid, int32_t cset,
	bool over, bool transp)
{
	if(over)
	{
		if(combobuf[cid].animflags & AF_TRANSPARENT)
			transp = !transp;
		if(transp)
			overcombotranslucent(dest, x, y, cid, cset, 128);
		else overcombo(dest, x, y, cid, cset);
	}
	else putcombo(dest, x, y, cid, cset);
}
void draw_cmb_pos(BITMAP* dest, int32_t x, int32_t y, byte pos, int32_t cid,
	int32_t cset, byte layer, bool over, bool transp)
{
	byte plpos = COMBOPOS(Hero.x+8, Hero.y+8);
	bool dosw = false;
	if(pos == hooked_combopos && (hooked_layerbits & (1<<layer)))
	{
		if(hooked_undercombos[layer] > -1)
		{
			draw_cmb(dest, COMBOX(pos)+x, COMBOY(pos)+y,
				hooked_undercombos[layer], hooked_undercombos[layer+7], over, transp);
		}
		dosw = true;
	}
	else if(pos == plpos && (hooked_layerbits & (1<<(layer+8))))
	{
		dosw = true;
	}
	if(dosw)
	{
		switch(Hero.switchhookstyle)
		{
			default: case swPOOF:
				break; //Nothing special here
			case swFLICKER:
			{
				if(abs(Hero.switchhookclk-33)&0b1000)
					break; //Drawn this frame
				return; //Not drawn this frame
			}
			case swRISE:
			{
				//Draw rising up
				y -= 8-(abs(Hero.switchhookclk-32)/4);
				break;
			}
		}
	}
	draw_cmb(dest, COMBOX(pos)+x, COMBOY(pos)+y, cid, cset, over, transp);
}

void do_scrolling_layer(BITMAP *bmp, int32_t type, int32_t layer, mapscr* basescr, int32_t x, int32_t y, bool scrolling, int32_t tempscreen)
{
	mapscr const* tmp = (layer > 0 ? (&(tempscreen==2?tmpscr2[layer-1]:tmpscr3[layer-1]))
		: (layer ? NULL : (tempscreen==2?0:1)+tmpscr));
	bool over = true, transp = false;
	
	switch(type ? type : layer)
	{
		case -4: //overhead FFCs
		case -3:                                                //freeform combos
			for(int32_t i = (basescr->numFFC()-1); i >= 0; --i)
			{
				if(scrolling && (basescr->ffcs[i].flags & ffc_carryover) != 0 && tempscreen == 3)
					continue; //If scrolling, only draw carryover ffcs from newscr and not oldscr,
				basescr->ffcs[i].draw(bmp, -x, -y+playing_field_offset, (type==-4));
			}
			
			return;
			
		case -2:                                                //push blocks
			if(tmp && tmp->valid)
			{
				for(int32_t i=0; i<176; i++)
				{
					int32_t mf=tmp->sflag[i], mf2 = combobuf[tmp->data[i]].flag;
					
					if(mf==mfPUSHUD || mf==mfPUSH4 || mf==mfPUSHED || ((mf>=mfPUSHLR)&&(mf<=mfPUSHRINS))
						|| mf2==mfPUSHUD || mf2==mfPUSH4 || mf2==mfPUSHED || ((mf2>=mfPUSHLR)&&(mf2<=mfPUSHRINS)))
					{
						draw_cmb_pos(bmp, -x, playing_field_offset-y, i, tmp->data[i], tmp->cset[i], layer, true, false);
					}
				}
			}
			return;
			
		case -1:                                                //over combo
			if(tmp && tmp->valid)
			{
				for(int32_t i=0; i<176; i++)
				{
					if(combo_class_buf[combobuf[tmp->data[i]].type].overhead)
					{
						draw_cmb_pos(bmp, -x, playing_field_offset-y, i, tmp->data[i], tmp->cset[i], layer, true, false);
					}
				}
			}
			return;
			
		case 1:
		case 4:
		case 5:
		case 6:
			if(TransLayers || basescr->layeropacity[layer-1]==255)
			{
				if(tmp && tmp->valid)
				{
					if(basescr->layeropacity[layer-1]!=255)
						transp = true;
					break;
				}
			}
			return;
			
		case 2:
			if(TransLayers || basescr->layeropacity[layer-1]==255)
			{
				if(tmp && tmp->valid)
				{
					if(basescr->layeropacity[layer-1]!=255)
						transp = true;
					
					if(XOR(basescr->flags7&fLAYER2BG, DMaps[currdmap].flags&dmfLAYER2BG))
						over = false;
					
					break;
				}
			}
			return;
			
		case 3:
			if(TransLayers || basescr->layeropacity[layer-1]==255)
			{
				if(tmp && tmp->valid)
				{
					if(basescr->layeropacity[layer-1]!=255)
						transp = true;
					
					if(XOR(basescr->flags7&fLAYER3BG, DMaps[currdmap].flags&dmfLAYER3BG)
						&& !XOR(basescr->flags7&fLAYER2BG, DMaps[currdmap].flags&dmfLAYER2BG))
						over = false;
					
					break;
				}
			}
			return;
	}
	
	for(int32_t i=0; i<176; i++)
	{
		draw_cmb_pos(bmp, -x, playing_field_offset-y, i, tmp->data[i], tmp->cset[i], layer, over, transp);
	}
}

bool lenscheck(mapscr* basescr, int layer)
{
	if(layer < 0 || layer > 6) return true;
	if(get_qr(qr_OLD_LENS_LAYEREFFECT))
	{
		if(!layer) return true;
		if((layer==(int32_t)(basescr->lens_layer&7)+1) && ((basescr->lens_layer&llLENSSHOWS && !lensclk) || (basescr->lens_layer&llLENSHIDES && lensclk)))
			return false;
	}
	else
	{
		if((lensclk ? basescr->lens_hide : basescr->lens_show) & (1<<layer))
			return false;
	}
	return true;
}
void do_layer(BITMAP *bmp, int32_t type, int32_t layer, mapscr* basescr, int32_t x, int32_t y, int32_t tempscreen, bool scrolling, bool drawprimitives)
{
    bool showlayer = true;
    
    switch(type ? type : layer)
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
        
    case 1:
        if(!show_layer_1)
        {
            showlayer = false;
        }
        
        break;
        
    case 2:
        if(!show_layer_2)
        {
            showlayer = false;
        }
        
        break;
        
    case 3:
        if(!show_layer_3)
        {
            showlayer = false;
        }
        
        break;
        
    case 4:
        if(!show_layer_4)
        {
            showlayer = false;
        }
        
        break;
        
    case 5:
        if(!show_layer_5)
        {
            showlayer = false;
        }
        
        break;
        
    case 6:
        if(!show_layer_6)
        {
            showlayer = false;
        }
        
        break;
    }
	
	if(!type || type == -2)
	{
		if(!lenscheck(basescr,layer))
			showlayer = false;
	}
    
	
    if(showlayer)
    {
		if(type || !(basescr->hidelayers & (1 << (layer))))
		{
			do_scrolling_layer(bmp, type, layer, basescr, x, y, scrolling, tempscreen);
			if(!type && !get_qr(qr_PUSHBLOCK_SPRITE_LAYER))
				if(mblock2.draw(bmp,layer))
					do_primitives(bmp, SPLAYER_MOVINGBLOCK, basescr, 0, playing_field_offset);
		}
        
        if(!type && drawprimitives && layer > 0 && layer <= 6)
        {
            do_primitives(bmp, layer, basescr, 0, playing_field_offset);
        }
    }
}

#define SOLID_COL makecol(178,36,36)
//makecol(255,85,85)
// Called by do_walkflags
void put_walkflags(BITMAP *dest,int32_t x,int32_t y,int32_t xofs,int32_t yofs, word cmbdat,int32_t lyr)
{
	newcombo const &c = combobuf[cmbdat];
	
	if (c.type == cBRIDGE && get_qr(qr_OLD_BRIDGE_COMBOS)) return;
	
	int32_t xx = x-xofs;
	int32_t yy = y+playing_field_offset-yofs;
	
	int32_t bridgedetected = 0;
	
	for(int32_t i=0; i<4; i++)
	{
		int32_t tx=((i&2)<<2)+xx;
		int32_t ty=((i&1)<<3)+yy;
		int32_t tx2=((i&2)<<2)+x;
		int32_t ty2=((i&1)<<3)+y;
		for (int32_t m = lyr-1; m <= 1; m++)
		{
			if (get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				if (combobuf[MAPCOMBO2(m,tx2,ty2)].type == cBRIDGE && !_walkflag_layer(tx2,ty2,1, &(tmpscr2[m]))) 
				{
					bridgedetected |= (1<<i);
				}
			}
			else
			{
				if (combobuf[MAPCOMBO2(m,tx2,ty2)].type == cBRIDGE && _effectflag_layer(tx2,ty2,1, &(tmpscr2[m]))) 
				{
					bridgedetected |= (1<<i);
				}
			}
		}
		if ((bridgedetected & (1<<i))) 
		{
			if (i >= 3) break;
			else continue;
		}
		bool doladdercheck = true;
		//if ( iswaterex(cmbdat, currmap, currscr, lyr, tx2, ty2, true, false, false)!=0 )
		
		if ((c.walk&(1<<(i+4))) && ((c.walk&(1<<i) && ((c.usrflags&cflag4)) && c.type == cWATER) || c.type == cSHALLOWWATER)) 
		{
			for(int32_t k=0; k<8; k+=2)
				for(int32_t j=0; j<8; j+=2)
					if(((k+j)/2)%2)
						rectfill(dest,tx+k,ty+j,tx+k+1,ty+j+1,makecol(85,85,255));
		}
		if (iswater_type(c.type) && !DRIEDLAKE && !((c.walk&(1<<(i+4))) && ((c.walk&(1<<i) && ((c.usrflags&cflag3) || (c.usrflags&cflag4)) && c.type == cWATER)))) //Yes, I realize this is horribly inaccurate; the alternative is the game chugging every time you turn on walk cheats.
		{
			if (get_qr(qr_NO_SOLID_SWIM)) doladdercheck = false;
			if((lyr==0 || (get_qr(qr_WATER_ON_LAYER_1) && lyr == 1) || (get_qr(qr_WATER_ON_LAYER_2) && lyr == 2)) && get_qr(qr_DROWN))
				rectfill(dest,tx,ty,tx+7,ty+7,makecol(85,85,255));
			else rectfill(dest,tx,ty,tx+7,ty+7,makecol(170,170,170));
		}
		
		if(c.walk&(1<<i) && !(iswater_type(c.type) && DRIEDLAKE) && !((c.walk&(1<<(i+4))) && ((c.walk&(1<<i) && ((c.usrflags&cflag3) || (c.usrflags&cflag4)) && c.type == cWATER))))  // Check for dried lake (watertype && not water)
		{
			if(c.type==cLADDERHOOKSHOT && isstepable(cmbdat) && ishookshottable(xx,yy))
			{
				for(int32_t k=0; k<8; k+=2)
					for(int32_t j=0; j<8; j+=2)
						rectfill(dest,tx+k,ty+j,tx+k+1,ty+j+1,((k+j)/2)%2 ? makecol(165,105,8) : makecol(170,170,170));
			}
			else
			{
				int32_t color = SOLID_COL;
				
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
		int32_t color = makecol(255,255,0);
		if (bridgedetected <= 0)
		{
			for(int32_t k=0; k<16; k+=2)
				for(int32_t j=0; j<16; j+=2)
				if(((k+j)/2)%2)
					rectfill(dest,x+k,y+j,x+k+1,y+j+1,color);
		}
		else
		{
			for(int32_t i=0; i<4; i++)
			{
				if (!(bridgedetected & (1<<i)))
				{
					int32_t tx=((i&2)<<2)+x;
					int32_t ty=((i&1)<<3)+y;
					for(int32_t k=0; k<8; k+=2)
						for(int32_t j=0; j<8; j+=2)
							if((k+j)%4 < 2) rectfill(dest,tx+k,ty+j,tx+k+1,ty+j+1,color);
				}
			}
		}
	}
}
void put_walkflags_a5(int32_t x,int32_t y,int32_t xofs,int32_t yofs, word cmbdat,int32_t lyr)
{
	ALLEGRO_COLOR col_solid = al_map_rgba(178,36,36,info_opacity);
	ALLEGRO_COLOR col_water1 = al_map_rgba(85,85,255,info_opacity);
	ALLEGRO_COLOR col_lhook = al_map_rgba(170,170,170,info_opacity);
	ALLEGRO_COLOR col_stepable = al_map_rgba(165,105,8,info_opacity);
	ALLEGRO_COLOR col_dmg = al_map_rgba(255,255,0,info_opacity);
	newcombo const &c = combobuf[cmbdat];
	
	if (c.type == cBRIDGE && get_qr(qr_OLD_BRIDGE_COMBOS)) return;
	
	int32_t xx = x-xofs;
	int32_t yy = y+playing_field_offset-yofs;
	
	int32_t bridgedetected = 0;
	
	// Draw damage combos
	bool dmg = combo_class_buf[c.type].modify_hp_amount;
	
	for(int32_t i=0; i<4; i++)
	{
		int32_t tx=((i&2)<<2)+xx;
		int32_t ty=((i&1)<<3)+yy;
		int32_t tx2=((i&2)<<2)+x;
		int32_t ty2=((i&1)<<3)+y;
		for (int32_t m = lyr-1; m <= 1; m++)
		{
			if (get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				if (combobuf[MAPCOMBO2(m,tx2,ty2)].type == cBRIDGE && !_walkflag_layer(tx2,ty2,1, &(tmpscr2[m]))) 
				{
					bridgedetected |= (1<<i);
				}
			}
			else
			{
				if (combobuf[MAPCOMBO2(m,tx2,ty2)].type == cBRIDGE && _effectflag_layer(tx2,ty2,1, &(tmpscr2[m]))) 
				{
					bridgedetected |= (1<<i);
				}
			}
		}
		if ((bridgedetected & (1<<i)))
			continue;
		bool doladdercheck = true;
		//if ( iswaterex(cmbdat, currmap, currscr, lyr, tx2, ty2, true, false, false)!=0 )
		
		if ((c.walk&(1<<(i+4))) && ((c.walk&(1<<i) && ((c.usrflags&cflag4)) && c.type == cWATER) || c.type == cSHALLOWWATER)) 
		{
			for(int32_t k=0; k<8; k+=2)
				for(int32_t j=0; j<8; j+=2)
					if(((k+j)/2)%2)
						al_draw_filled_rectangle(tx+k,ty+j,tx+k+2,ty+j+2,col_water1);
		}
		if (iswater_type(c.type) && !DRIEDLAKE && !((c.walk&(1<<(i+4))) && ((c.walk&(1<<i) && ((c.usrflags&cflag3) || (c.usrflags&cflag4)) && c.type == cWATER)))) //Yes, I realize this is horribly inaccurate; the alternative is the game chugging every time you turn on walk cheats.
		{
			if (get_qr(qr_NO_SOLID_SWIM)) doladdercheck = false;
			if((lyr==0 || (get_qr(qr_WATER_ON_LAYER_1) && lyr == 1) || (get_qr(qr_WATER_ON_LAYER_2) && lyr == 2)) && get_qr(qr_DROWN))
				al_draw_filled_rectangle(tx,ty,tx+8,ty+8,col_water1);
			else al_draw_filled_rectangle(tx,ty,tx+8,ty+8,col_lhook);
		}
		
		if(c.walk&(1<<i) && !(iswater_type(c.type) && DRIEDLAKE) && !((c.walk&(1<<(i+4))) && ((c.walk&(1<<i) && ((c.usrflags&cflag3) || (c.usrflags&cflag4)) && c.type == cWATER))))  // Check for dried lake (watertype && not water)
		{
			if(c.type==cLADDERHOOKSHOT && isstepable(cmbdat) && ishookshottable(xx,yy))
			{
				for(int32_t k=0; k<8; k+=2)
					for(int32_t j=0; j<8; j+=2)
						al_draw_filled_rectangle(tx+k,ty+j,tx+k+2,ty+j+2,((k+j)/2)%2 ? col_stepable : col_lhook);
			}
			else
			{
				ALLEGRO_COLOR* color = &col_solid;
				
				if(isstepable(cmbdat)&& (!doladdercheck))
					color=&col_stepable;
				else if((c.type==cHOOKSHOTONLY || c.type==cLADDERHOOKSHOT) && ishookshottable(xx,yy))
					color=&col_lhook;
					
				al_draw_filled_rectangle(tx,ty,tx+8,ty+8,*color);
			}
		}
		
		if(dmg)
		{
			for(int32_t k=0; k<8; k+=2)
				for(int32_t j=0; j<8; j+=2)
					if((k+j)%4 < 2) al_draw_filled_rectangle(tx+k,ty+j,tx+k+2,ty+j+2,col_dmg);
		}
	}
}

void put_effectflags(BITMAP *dest,int32_t x,int32_t y,int32_t xofs,int32_t yofs, word cmbdat,int32_t lyr)
{
	newcombo const &c = combobuf[cmbdat];
	
	int32_t xx = x-xofs;
	int32_t yy = y+playing_field_offset-yofs;
	
	for(int32_t i=0; i<4; i++)
	{
		int32_t tx=((i&2)<<2)+xx;
		int32_t ty=((i&1)<<3)+yy;
		int32_t tx2=((i&2)<<2)+x;
		int32_t ty2=((i&1)<<3)+y;
	
	
		if(((c.walk>>4)&(1<<i)) && c.type != cNONE)
		{
				int32_t color = vc(10);
					
				rectfill(dest,tx,ty,tx+7,ty+7,color);
		}
	}
}
void put_effectflags_a5(int32_t x,int32_t y,int32_t xofs,int32_t yofs, word cmbdat,int32_t lyr)
{
	ALLEGRO_COLOR col_eff = al_map_rgba(85,255,85,info_opacity);
	newcombo const &c = combobuf[cmbdat];
	
	int32_t xx = x-xofs;
	int32_t yy = y+playing_field_offset-yofs;
	
	for(int32_t i=0; i<4; i++)
	{
		int32_t tx=((i&2)<<2)+xx;
		int32_t ty=((i&1)<<3)+yy;
		int32_t tx2=((i&2)<<2)+x;
		int32_t ty2=((i&1)<<3)+y;
	
	
		if(((c.walk>>4)&(1<<i)) && c.type != cNONE)
		{
			al_draw_filled_rectangle(tx,ty,tx+8,ty+8,col_eff);
		}
	}
}

void draw_ladder_platform(BITMAP* dest, int32_t x, int32_t y, int32_t c)
{
	for(auto cx = 0; cx < 256; cx += 16)
	{
		for(auto cy = 0; cy < 176; cy += 16)
		{
			if(isSVLadder(cx,cy))
			{
				auto nx = cx+x, ny = cy+y;
				if(cy && !isSVLadder(cx,cy-16))
					line(dest,nx,ny,nx+15,ny,c);
				rectfill(dest,nx,ny,nx+3,ny+15,c);
				rectfill(dest,nx+12,ny,nx+15,ny+15,c);
				rectfill(dest,nx+4,ny+2,nx+11,ny+5,c);
				rectfill(dest,nx+4,ny+10,nx+11,ny+13,c);
			}
			else if(isSVPlatform(cx,cy))
			{
				line(dest,cx+x,cy+y,cx+x+15,cy+y,c);
			}
		}
	}
}
void draw_ladder_platform_a5(int32_t x, int32_t y, ALLEGRO_COLOR c)
{
	for(auto cx = 0; cx < 256; cx += 16)
	{
		for(auto cy = 0; cy < 176; cy += 16)
		{
			if(isSVLadder(cx,cy))
			{
				auto nx = cx+x, ny = cy+y;
				if(cy && !isSVLadder(cx,cy-16))
					al_draw_line(nx,ny,nx+15,ny,c,1);
				al_draw_filled_rectangle(nx,ny,nx+4,ny+16,c);
				al_draw_filled_rectangle(nx+12,ny,nx+16,ny+16,c);
				al_draw_filled_rectangle(nx+4,ny+2,nx+12,ny+6,c);
				al_draw_filled_rectangle(nx+4,ny+10,nx+12,ny+14,c);
			}
			else if(isSVPlatform(cx,cy))
			{
				al_draw_line(cx+x,cy+y,cx+x+15,cy+y,c,1);
			}
		}
	}
}

// Walkflags L4 cheat
void do_walkflags(mapscr* layer,int32_t x, int32_t y, int32_t tempscreen)
{
	if(show_walkflags)
	{
		start_info_bmp();
		
		for(int32_t i=0; i<176; i++)
		{
			put_walkflags_a5(((i&15)<<4),(i&0xF0),x,y,layer->data[i], 0);
		}
		
		for(int32_t k=0; k<2; k++)
		{
			mapscr* lyr = (tempscreen==2 ? tmpscr2+k : tmpscr3+k);
			
			if(lyr->valid)
			{
				for(int32_t i=0; i<176; i++)
				{
					put_walkflags_a5(((i&15)<<4),(i&0xF0),x,y,lyr->data[i], k%2+1);
				}
			}
		}
		
		if (tempscreen == 2)
		{
			draw_ladder_platform_a5(-x,-y+playing_field_offset,al_map_rgba(165,105,8,info_opacity));
			draw_solid_objects_a5(-x,-y+playing_field_offset,al_map_rgba(178,36,36,info_opacity));
			draw_slopes_a5(-x,-y+playing_field_offset,al_map_rgba(255,85,255,info_opacity));
		}
		
		end_info_bmp();
	}
}

// Effectflags L4 cheat
void do_effectflags(mapscr* layer,int32_t x, int32_t y, int32_t tempscreen)
{
	if(show_effectflags)
	{
		start_info_bmp();
		
		for(int32_t i=0; i<176; i++)
		{
			put_effectflags_a5(((i&15)<<4),(i&0xF0),x,y,layer->data[i], 0);
		}
		
		end_info_bmp();
	}
}

void calc_darkroom_combos(bool scrolling)
{
	if(!get_qr(qr_NEW_DARKROOM)) return;
	int32_t scrolldir = get_qr(qr_NEWDARK_SCROLLEDGE) ? FFCore.ScrollingData[SCROLLDATA_DIR] : -1;
	int32_t scrollxoffs = 0, scrollyoffs = 0;
	switch(scrolldir)
	{
		case up:
			scrollyoffs = -176;
			break;
		case down:
			scrollyoffs = 176;
			break;
		case left:
			scrollxoffs = -256;
			break;
		case right:
			scrollxoffs = 256;
			break;
	}
	for(int32_t lyr = 0; lyr < 7; ++lyr)
	{
		mapscr* m = FFCore.tempScreens[lyr];
		if(lyr && !m->valid) continue; //invalid layer
		for(int32_t q = 0; q < 176; ++q)
		{
			newcombo const& cmb = combobuf[m->data[q]];
			if(cmb.type == cTORCH)
			{
				do_torch_combo(cmb, COMBOX(q)+8, COMBOY(q)+8, darkscr_bmp_curscr);
				if(scrolldir > -1)
					do_torch_combo(cmb, COMBOX(q)+8+scrollxoffs, COMBOY(q)+8+scrollyoffs, darkscr_bmp_scrollscr);
			}
		}
	}
	word c = tmpscr->numFFC();
	for(word q = 0; q < c; ++q)
	{
		newcombo const& cmb = combobuf[tmpscr->ffcs[q].data];
		if(cmb.type == cTORCH)
		{
			int cx = (tmpscr->ffcs[q].x.getInt())+(tmpscr->ffEffectWidth(q)/2),
			    cy = (tmpscr->ffcs[q].y.getInt())+(tmpscr->ffEffectHeight(q)/2);
			do_torch_combo(cmb, cx, cy, darkscr_bmp_curscr);
			if(scrolldir > -1)
				do_torch_combo(cmb, cx+scrollxoffs, cy+scrollyoffs, darkscr_bmp_scrollscr);
		}
	}
	if(!scrolling && mblock2.clk)
	{
		newcombo const& cmb = combobuf[mblock2.bcombo];
		if(cmb.type == cTORCH)
			do_torch_combo(cmb, mblock2.x+8, mblock2.y+8, darkscr_bmp_curscr);
	}
	
	if(!scrolling) return; //not a scrolling call, don't run code for scrolling screen
	
	for(int32_t lyr = 0; lyr < 7; ++lyr)
	{
		mapscr* m = FFCore.ScrollingScreens[lyr];
		if(lyr && !m->valid) continue; //invalid layer
		for(int32_t q = 0; q < 176; ++q)
		{
			newcombo const& cmb = combobuf[m->data[q]];
			if(cmb.type == cTORCH)
			{
				do_torch_combo(cmb, COMBOX(q)+8, COMBOY(q)+8, darkscr_bmp_scrollscr);
				if(scrolldir > -1)
					do_torch_combo(cmb, COMBOX(q)+8-scrollxoffs, COMBOY(q)+8-scrollyoffs, darkscr_bmp_curscr);
			}
		}
	}
	c = tmpscr[1].numFFC();
	for(word q = 0; q < c; ++q)
	{
		newcombo const& cmb = combobuf[tmpscr[1].ffcs[q].data];
		if(cmb.type == cTORCH)
		{
			int cx = (tmpscr[1].ffcs[q].x.getInt())+(tmpscr[1].ffEffectWidth(q)/2),
			    cy = (tmpscr[1].ffcs[q].y.getInt())+(tmpscr[1].ffEffectHeight(q)/2);
			do_torch_combo(cmb, cx, cy, darkscr_bmp_scrollscr);
			if(scrolldir > -1)
				do_torch_combo(cmb, cx-scrollxoffs, cy-scrollyoffs, darkscr_bmp_curscr);
		}
	}
}

void draw_msgstr(byte layer)
{
	if(layer != msgstr_layer) return;

	if(!(msg_bg_display_buf->clip))
	{
		blit_msgstr_bg(framebuf,0,0,0,playing_field_offset,256,168);
	}
	
	if(!(msg_portrait_display_buf->clip))
	{
		blit_msgstr_prt(framebuf,0,0,0,playing_field_offset,256,168);
	}
	
	if(!(msg_txt_display_buf->clip))
	{
		blit_msgstr_fg(framebuf,0,0,0,playing_field_offset,256,168);
	}
}

void draw_screen(mapscr* this_screen, bool showhero, bool runGeneric)
{
	clear_info_bmp();
	if((GameFlags & (GAMEFLAG_SCRIPTMENU_ACTIVE|GAMEFLAG_F6SCRIPT_ACTIVE))!=0)
	{
		FFCore.doScriptMenuDraws();
		return;
	}
	
	if(runGeneric) FFCore.runGenericPassiveEngine(SCR_TIMING_PRE_DRAW);
	
	//The Plan:
	//1. Draw some background layers
	//2. Blit scrollbuf onto framebuf
	//3. Draw some sprites
	//4. -----
	//5. Draw some layers
	//6. -----
	//6b. Draw the subscreen, without clipping
	//7. Draw some flying sprites
	//8. -----
	//9. Draw some layers
	//10. ----
	//11. Draw some text
	//12. Draw the subscreen, without clipping
	clear_bitmap(framebuf);
	clear_clip_rect(framebuf);
	
	int32_t cmby2=0;
	
	//1. Draw some background layers
	clear_bitmap(scrollbuf);
	
	if(XOR(this_screen->flags7&fLAYER2BG, DMaps[currdmap].flags&dmfLAYER2BG))
	{
		do_layer(scrollbuf, 0, 2, this_screen, 0, 0, 2, false, true);
		
		particles.draw(framebuf, true, 1);
		draw_msgstr(2);
	}
	
	if(XOR(this_screen->flags7&fLAYER3BG, DMaps[currdmap].flags&dmfLAYER3BG))
	{
		do_layer(scrollbuf, 0, 3, this_screen, 0, 0, 2, false, true);
		
		particles.draw(framebuf, true, 2);
		draw_msgstr(3);
	}
	
	if(lenscheck(this_screen,0))
	{
		putscr(scrollbuf,0,playing_field_offset,this_screen);
		if(!get_qr(qr_PUSHBLOCK_SPRITE_LAYER))
			if(mblock2.draw(scrollbuf,0))
				do_primitives(scrollbuf, SPLAYER_MOVINGBLOCK, this_screen, 0, playing_field_offset);
	}
	
	// Lens hints, then primitives, then particles.
	if((lensclk || (get_debug() && zc_getkey(KEY_L))) && !get_qr(qr_OLDLENSORDER))
	{
		draw_lens_under(scrollbuf, false);
		do_primitives(scrollbuf, SPLAYER_LENS_UNDER_1, this_screen, 0, playing_field_offset);
	}
	
	if(show_layer_0 && lenscheck(this_screen,0))
		do_primitives(scrollbuf, 0, this_screen, 0, playing_field_offset);
		
	particles.draw(framebuf, true, -3);
	draw_msgstr(0);
	
	set_clip_rect(scrollbuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
	
	if(!(get_qr(qr_LAYER12UNDERCAVE)))
	{
		if(showhero &&
				((Hero.getAction()==climbcovertop)||(Hero.getAction()==climbcoverbottom)))
		{
			if(Hero.getAction()==climbcovertop)
			{
				cmby2=16;
			}
			else if(Hero.getAction()==climbcoverbottom)
			{
				cmby2=-16;
			}
			
			decorations.draw2(scrollbuf,true);
			Hero.draw(scrollbuf);
			decorations.draw(scrollbuf,true);
			int32_t ccx = (int32_t)(Hero.getClimbCoverX());
			int32_t ccy = (int32_t)(Hero.getClimbCoverY());
			
			overcombo(scrollbuf,ccx,ccy+cmby2+playing_field_offset,MAPCOMBO(ccx,ccy+cmby2),MAPCSET(ccx,ccy+cmby2));
			putcombo(scrollbuf,ccx,ccy+playing_field_offset,MAPCOMBO(ccx,ccy),MAPCSET(ccx,ccy));
			
			if(int32_t(Hero.getX())&15)
			{
				overcombo(scrollbuf,ccx+16,ccy+cmby2+playing_field_offset,MAPCOMBO(ccx+16,ccy+cmby2),MAPCSET(ccx+16,ccy+cmby2));
				putcombo(scrollbuf,ccx+16,ccy+playing_field_offset,MAPCOMBO(ccx+16,ccy),MAPCSET(ccx+16,ccy));
			}
		}
	}
	
	do_layer(scrollbuf, 0, 1, this_screen, 0, 0, 2, false, true); // LAYER 1
	
	particles.draw(framebuf, true, 0);
	draw_msgstr(1);
	
	do_layer(scrollbuf, -3, 0, this_screen, 0, 0, 2); // freeform combos!
	do_primitives(framebuf, SPLAYER_FFC_DRAW, this_screen, 0, playing_field_offset);
	
	if(!XOR(this_screen->flags7&fLAYER2BG, DMaps[currdmap].flags&dmfLAYER2BG))
	{
		do_layer(scrollbuf, 0, 2, this_screen, 0, 0, 2, false, true); // LAYER 2
		
		particles.draw(framebuf, true, 1);
		draw_msgstr(2);
	}
	
	if(get_qr(qr_LAYER12UNDERCAVE))
	{
		if(showhero &&
				((Hero.getAction()==climbcovertop)||(Hero.getAction()==climbcoverbottom)))
		{
			if(Hero.getAction()==climbcovertop)
			{
				cmby2=16;
			}
			else if(Hero.getAction()==climbcoverbottom)
			{
				cmby2=-16;
			}
			
			decorations.draw2(scrollbuf,true);
			Hero.draw(scrollbuf);
			decorations.draw(scrollbuf,true);
			int32_t ccx = (int32_t)(Hero.getClimbCoverX());
			int32_t ccy = (int32_t)(Hero.getClimbCoverY());
			
			overcombo(scrollbuf,ccx,ccy+cmby2+playing_field_offset,MAPCOMBO(ccx,ccy+cmby2),MAPCSET(ccx,ccy+cmby2));
			putcombo(scrollbuf,ccx,ccy+playing_field_offset,MAPCOMBO(ccx,ccy),MAPCSET(ccx,ccy));
			
			if(int32_t(Hero.getX())&15)
			{
				overcombo(scrollbuf,ccx+16,ccy+cmby2+playing_field_offset,MAPCOMBO(ccx+16,ccy+cmby2),MAPCSET(ccx+16,ccy+cmby2));
				putcombo(scrollbuf,ccx+16,ccy+playing_field_offset,MAPCOMBO(ccx+16,ccy),MAPCSET(ccx+16,ccy));
			}
		}
	}
	
	if(get_qr(qr_PUSHBLOCK_SPRITE_LAYER))
	{
		do_layer(scrollbuf, -2, 0, this_screen, 0, 0, 2); // push blocks!
		if(get_qr(qr_PUSHBLOCK_LAYER_1_2))
		{
			do_layer(scrollbuf, -2, 1, this_screen, 0, 0, 2); // push blocks!
			do_layer(scrollbuf, -2, 2, this_screen, 0, 0, 2); // push blocks!
		}
		do_primitives(scrollbuf, SPLAYER_PUSHBLOCK, this_screen, 0, playing_field_offset);
	}
	//Show walkflags cheat
	do_walkflags(this_screen,0,0,2);
	do_effectflags(this_screen,0,0,2);
	
	putscrdoors(scrollbuf,0,playing_field_offset,this_screen);
	
	// Lens hints, doors etc.
	if(lensclk || (get_debug() && zc_getkey(KEY_L)))
	{
		if(get_qr(qr_OLDLENSORDER))
		{
			draw_lens_under(scrollbuf, false);
			do_primitives(scrollbuf, SPLAYER_LENS_UNDER_1, this_screen, 0, playing_field_offset);
		}
		
		draw_lens_under(scrollbuf, true);
		do_primitives(scrollbuf, SPLAYER_LENS_UNDER_2, this_screen, 0, playing_field_offset);
	}
	
	//2. Blit those layers onto framebuf
	
	set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
	blit(scrollbuf, framebuf, 0, 0, 0, 0, 256, 224);

	//6b. Draw the subscreen, without clipping
	if(!get_qr(qr_SUBSCREENOVERSPRITES))
	{
		bool dotime = false;
		if (replay_version_check(22) || !replay_is_active()) dotime = game->should_show_time();
		put_passive_subscr(framebuf, 0, passive_subscreen_offset, dotime, sspUP);
	}
	
	//3. Draw some sprites onto framebuf
	set_clip_rect(framebuf,0,0,256,224);
	
	if(!(pricesdisplaybuf->clip))
	{
		masked_blit(pricesdisplaybuf,framebuf,0,0,0,playing_field_offset,256,168);
	}
	
	if(showhero && ((Hero.getAction()!=climbcovertop)&&(Hero.getAction()!=climbcoverbottom)))
	{
		Hero.draw_under(framebuf);
		
		if(Hero.isSwimming())
		{
			decorations.draw2(framebuf,true);
			Hero.draw(framebuf);
			decorations.draw(framebuf,true);
		}
	}
	
	if(drawguys)
	{
		if(get_qr(qr_NOFLICKER) || (frame&1))
		{
			for(int32_t i=0; i<Ewpns.Count(); i++)
			{
				if(((weapon *)Ewpns.spr(i))->behind)
					Ewpns.spr(i)->draw(framebuf);
			}
			do_primitives(framebuf, SPLAYER_EWEAP_BEHIND_DRAW, this_screen, 0, playing_field_offset);
			
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				if(((weapon *)Lwpns.spr(i))->behind)
					Lwpns.spr(i)->draw(framebuf);
			}
			do_primitives(framebuf, SPLAYER_LWEAP_BEHIND_DRAW, this_screen, 0, playing_field_offset);
			
			if(get_qr(qr_SHADOWS)&&(!get_qr(qr_SHADOWSFLICKER)||frame&1))
			{
				guys.drawshadow(framebuf,get_qr(qr_TRANSSHADOWS)!=0,true);
			}
			
			guys.draw(framebuf,true);
			do_primitives(framebuf, SPLAYER_NPC_DRAW, this_screen, 0, playing_field_offset);
			chainlinks.draw(framebuf,true);
			do_primitives(framebuf, SPLAYER_CHAINLINK_DRAW, this_screen, 0, playing_field_offset);
			//Lwpns.draw(framebuf,true);
			
			for(int32_t i=0; i<Ewpns.Count(); i++)
			{
				if(!((weapon *)Ewpns.spr(i))->behind)
					Ewpns.spr(i)->draw(framebuf);
			}
			do_primitives(framebuf, SPLAYER_EWEAP_FRONT_DRAW, this_screen, 0, playing_field_offset);
			
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				if(!((weapon *)Lwpns.spr(i))->behind)
					Lwpns.spr(i)->draw(framebuf);
			}
			do_primitives(framebuf, SPLAYER_LWEAP_FRONT_DRAW, this_screen, 0, playing_field_offset);
			
			
			items.draw(framebuf,true);
			do_primitives(framebuf, SPLAYER_ITEMSPRITE_DRAW, this_screen, 0, playing_field_offset);
		}
		else
		{
			for(int32_t i=0; i<Ewpns.Count(); i++)
			{
				if(((weapon *)Ewpns.spr(i))->behind)
					Ewpns.spr(i)->draw(framebuf);
			}
			do_primitives(framebuf, SPLAYER_EWEAP_BEHIND_DRAW, this_screen, 0, playing_field_offset);
		
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				if(((weapon *)Lwpns.spr(i))->behind)
					Lwpns.spr(i)->draw(framebuf);
			}
			do_primitives(framebuf, SPLAYER_LWEAP_BEHIND_DRAW, this_screen, 0, playing_field_offset);
			
			if(get_qr(qr_SHADOWS)&&(!get_qr(qr_SHADOWSFLICKER)||frame&1))
			{
				guys.drawshadow(framebuf,get_qr(qr_TRANSSHADOWS)!=0,true);
			}
			
			items.draw(framebuf,false);
			do_primitives(framebuf, SPLAYER_ITEMSPRITE_DRAW, this_screen, 0, playing_field_offset);
			chainlinks.draw(framebuf,false);
			do_primitives(framebuf, SPLAYER_CHAINLINK_DRAW, this_screen, 0, playing_field_offset);
			//Lwpns.draw(framebuf,false);
			guys.draw(framebuf,false);
			do_primitives(framebuf, SPLAYER_NPC_DRAW, this_screen, 0, playing_field_offset);
			
			for(int32_t i=0; i<Ewpns.Count(); i++)
			{
				if(!((weapon *)Ewpns.spr(i))->behind)
				{
					Ewpns.spr(i)->draw(framebuf);
				}
			}
			do_primitives(framebuf, SPLAYER_EWEAP_FRONT_DRAW, this_screen, 0, playing_field_offset);
		
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				if(!((weapon *)Lwpns.spr(i))->behind)
				{
					Lwpns.spr(i)->draw(framebuf);
				}
			}
			do_primitives(framebuf, SPLAYER_LWEAP_FRONT_DRAW, this_screen, 0, playing_field_offset);
		}
		
		guys.draw2(framebuf,true);
	}
	
	if(mirror_portal.destdmap > -1)
		mirror_portal.draw(framebuf);
	portals.draw(framebuf,true);
	
	if(showhero && ((Hero.getAction()!=climbcovertop)&& (Hero.getAction()!=climbcoverbottom)))
	{
		if(get_qr(qr_PUSHBLOCK_SPRITE_LAYER))
		{
			mblock2.draw(framebuf,-1);
			do_primitives(framebuf, SPLAYER_MOVINGBLOCK, this_screen, 0, playing_field_offset);
		}
		if(!Hero.isSwimming())
		{
			if((Hero.getZ()>0 || Hero.getFakeZ()>0) &&(!get_qr(qr_SHADOWSFLICKER)||frame&1))
			{
				Hero.drawshadow(framebuf,get_qr(qr_TRANSSHADOWS)!=0);
			}
			
			if(Hero.getZ() <= (zfix)zinit.jump_hero_layer_threshold)
			{
				decorations.draw2(framebuf,true);
				Hero.draw(framebuf);
				decorations.draw(framebuf,true);
			}
		}
	}
	
	for(int32_t i=0; i<guys.Count(); i++)
	{
		if(((enemy*)guys.spr(i))->family == eeWALK)
		{
			if(((eStalfos*)guys.spr(i))->hashero)
			{
				guys.spr(i)->draw(framebuf);
			}
		}
		
		if(((enemy*)guys.spr(i))->family == eeWALLM)
		{
			if(((eWallM*)guys.spr(i))->hashero)
			{
				guys.spr(i)->draw(framebuf);
			}
		}
		
		if(guys.spr(i)->z+guys.spr(i)->fakez > Hero.getZ()+Hero.getFakeZ())
		{
			//Jumping enemies in front of Hero.
			guys.spr(i)->draw(framebuf);
		}
		do_primitives(framebuf, SPLAYER_NPC_ABOVEPLAYER_DRAW, this_screen, 0, playing_field_offset);
	}
	
	//5. Draw some layers onto framebuf
	set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
	
	if(!XOR(this_screen->flags7&fLAYER3BG, DMaps[currdmap].flags&dmfLAYER3BG))
	{
		do_layer(framebuf, 0, 3, this_screen, 0, 0, 2, false, true);
		
		particles.draw(framebuf, true, 2);
		draw_msgstr(3);
	}
	
	do_layer(framebuf, 0, 4, this_screen, 0, 0, 2, false, true);
	
	particles.draw(framebuf, true, 3);
	draw_msgstr(4);
	
	do_layer(framebuf, -1, 0, this_screen, 0, 0, 2);
	if(get_qr(qr_OVERHEAD_COMBOS_L1_L2))
	{
		do_layer(framebuf, -1, 1, this_screen, 0, 0, 2);
		do_layer(framebuf, -1, 2, this_screen, 0, 0, 2);
	}
	do_primitives(framebuf, SPLAYER_OVERHEAD_CMB, this_screen, 0, playing_field_offset);
	
	particles.draw(framebuf, true, -1);
	
	// //6b. Draw the subscreen, without clipping
	// if(!get_qr(qr_SUBSCREENOVERSPRITES))
	// {
	// 	bool dotime = false;
	// 	if (replay_version_check(22) || !replay_is_active()) dotime = game->should_show_time();
	// 	put_passive_subscr(framebuf, 0, passive_subscreen_offset, dotime, sspUP);
	// }
	
	
	//7. Draw some flying sprites onto framebuf
	clear_clip_rect(framebuf);
	// set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
	
	//Jumping Hero and jumping enemies are drawn on this layer.
	if(Hero.getZ() > (zfix)zinit.jump_hero_layer_threshold)
	{
		decorations.draw2(framebuf,false);
		Hero.draw(framebuf);
		chainlinks.draw(framebuf,true);
		do_primitives(framebuf, SPLAYER_CHAINLINK_DRAW, this_screen, 0, playing_field_offset);
		
		for(int32_t i=0; i<Lwpns.Count(); i++)
		{
			if(Lwpns.spr(i)->z+Lwpns.spr(i)->fakez > (zfix)zinit.jump_hero_layer_threshold)
			{
				Lwpns.spr(i)->draw(framebuf);
			}
		}
		do_primitives(framebuf, SPLAYER_LWEAP_ABOVE_DRAW, this_screen, 0, playing_field_offset);
		
		decorations.draw(framebuf,false);
	}
	
	if(!get_qr(qr_ENEMIESZAXIS)) for(int32_t i=0; i<guys.Count(); i++)
		{
			if((isflier(guys.spr(i)->id)) || (guys.spr(i)->z+guys.spr(i)->fakez) > (zfix)zinit.jump_hero_layer_threshold)
			{
				guys.spr(i)->draw(framebuf);
			}
		}
	else
	{
		for(int32_t i=0; i<guys.Count(); i++)
		{
			if((isflier(guys.spr(i)->id)) || guys.spr(i)->z > 0 || guys.spr(i)->fakez > 0)
			{
				guys.spr(i)->draw(framebuf);
			}
		}
	}
	do_primitives(framebuf, SPLAYER_NPC_AIRBORNE_DRAW, this_screen, 0, playing_field_offset);
	
	// Draw the Moving Fairy above layer 3
	for(int32_t i=0; i<items.Count(); i++)
		if(itemsbuf[items.spr(i)->id].family == itype_fairy && itemsbuf[items.spr(i)->id].misc3)
			items.spr(i)->draw(framebuf);
	do_primitives(framebuf, SPLAYER_FAIRYITEM_DRAW, this_screen, 0, playing_field_offset);
	
	//9. Draw some layers onto framebuf

	set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
	
	if (lightbeam_present)
	{
		color_map = &trans_table2;
		if(get_qr(qr_LIGHTBEAM_TRANSPARENT))
			draw_trans_sprite(framebuf, lightbeam_bmp, 0, playing_field_offset);
		else 
			masked_blit(lightbeam_bmp, framebuf, 0, 0, 0, playing_field_offset, 256, 176);
		color_map = &trans_table;
	}
	
	do_layer(framebuf, 0, 5, this_screen, 0, 0, 2, false, true);
	
	particles.draw(framebuf, true, 4);
	draw_msgstr(5);
	
	do_layer(framebuf, -4, 0, this_screen, 0, 0, 2); // overhead freeform combos!
	do_primitives(framebuf, SPLAYER_OVERHEAD_FFC, this_screen, 0, playing_field_offset);
	
	do_layer(framebuf, 0, 6, this_screen, 0, 0, 2, false, true);
	
	particles.draw(framebuf, true, 5);
		
	//11. Handle low drawn darkness
	if(get_qr(qr_NEW_DARKROOM)&& room_is_dark)
	{
		calc_darkroom_combos();
		if(showhero)
			Hero.calc_darkroom_hero();
	}
	//Darkroom if under the subscreen
	if(get_qr(qr_NEW_DARKROOM) && get_qr(qr_NEWDARK_L6) && room_is_dark)
	{
		do_primitives(framebuf, SPLAYER_DARKROOM_UNDER, this_screen, 0, playing_field_offset);
		set_clip_rect(framebuf, 0, playing_field_offset, 256, 168+playing_field_offset);
		if(this_screen->flags9 & fDARK_DITHER) //dither the entire bitmap
		{
			ditherblit(darkscr_bmp_curscr,darkscr_bmp_curscr,0,game->get_dither_type(),game->get_dither_arg());
			ditherblit(darkscr_bmp_curscr_trans,darkscr_bmp_curscr_trans,0,game->get_dither_type(),game->get_dither_arg());
		}
		
		color_map = &trans_table2;
		if(this_screen->flags9 & fDARK_TRANS) //draw the dark as transparent
		{
			draw_trans_sprite(framebuf, darkscr_bmp_curscr, 0, playing_field_offset);
			if(get_qr(qr_NEWDARK_TRANS_STACKING))
				draw_trans_sprite(framebuf, darkscr_bmp_curscr_trans, 0, playing_field_offset);
		}
		else
		{
			masked_blit(darkscr_bmp_curscr, framebuf, 0, 0, 0, playing_field_offset, 256, 168);
			draw_trans_sprite(framebuf, darkscr_bmp_curscr_trans, 0, playing_field_offset);
		}
		color_map = &trans_table;
		
		set_clip_rect(framebuf, 0, 0, framebuf->w, framebuf->h);
		do_primitives(framebuf, SPLAYER_DARKROOM_OVER, this_screen, 0, playing_field_offset);
	}
	
	
	//12. Draw some text on framebuf
	
	set_clip_rect(framebuf,0,0,256,224);
	
	draw_msgstr(6);
	
	//13. Draw the subscreen, without clipping
	if(get_qr(qr_SUBSCREENOVERSPRITES))
	{
		put_passive_subscr(framebuf, 0, passive_subscreen_offset, game->should_show_time(), sspUP);
		
		// Draw primitives over subscren
		do_primitives(framebuf, 7, this_screen, 0, playing_field_offset); //Layer '7' appears above subscreen if quest rule is set
	}
	
	//14. Handle high-drawn darkness
	if(get_qr(qr_NEW_DARKROOM) && !get_qr(qr_NEWDARK_L6) && room_is_dark)
	{
		do_primitives(framebuf, SPLAYER_DARKROOM_UNDER, this_screen, 0, playing_field_offset);
		set_clip_rect(framebuf, 0, playing_field_offset, 256, 168+playing_field_offset);
		if(this_screen->flags9 & fDARK_DITHER) //dither the entire bitmap
		{
			ditherblit(darkscr_bmp_curscr,darkscr_bmp_curscr,0,game->get_dither_type(),game->get_dither_arg());
			ditherblit(darkscr_bmp_curscr_trans,darkscr_bmp_curscr_trans,0,game->get_dither_type(),game->get_dither_arg());
		}
		
		color_map = &trans_table2;
		if(this_screen->flags9 & fDARK_TRANS) //draw the dark as transparent
		{
			draw_trans_sprite(framebuf, darkscr_bmp_curscr, 0, playing_field_offset);
			if(get_qr(qr_NEWDARK_TRANS_STACKING))
				draw_trans_sprite(framebuf, darkscr_bmp_curscr_trans, 0, playing_field_offset);
		}
		else
		{
			masked_blit(darkscr_bmp_curscr, framebuf, 0, 0, 0, playing_field_offset, 256, 168);
			draw_trans_sprite(framebuf, darkscr_bmp_curscr_trans, 0, playing_field_offset);
		}
		color_map = &trans_table;
		
		set_clip_rect(framebuf, 0, 0, framebuf->w, framebuf->h);
		do_primitives(framebuf, SPLAYER_DARKROOM_OVER, this_screen, 0, playing_field_offset);
	}
	
	draw_msgstr(7);
	
	set_clip_rect(scrollbuf, 0, 0, scrollbuf->w, scrollbuf->h);
	if(runGeneric) FFCore.runGenericPassiveEngine(SCR_TIMING_POST_DRAW);
}

void put_door(BITMAP *dest,int32_t t,int32_t pos,int32_t side,int32_t type,bool redraw,bool even_walls)
{
	int32_t d=tmpscr[t].door_combo_set;
	if (type > 8) return;
	
	switch(type)
	{
	case dt_wall:
	case dt_walk:
		if(!even_walls)
			break;
		[[fallthrough]];
	case dt_pass:
		if(!get_qr(qr_REPLACEOPENDOORS) && !even_walls)
			break;
		[[fallthrough]];
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

void over_door(BITMAP *dest,int32_t t, int32_t pos,int32_t side, int32_t xoff, int32_t yoff)
{
	int32_t d=tmpscr[t].door_combo_set;
	int32_t x=(pos&15)<<4;
	int32_t y=(pos&0xF0);
	
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

void putdoor(BITMAP *dest,int32_t t,int32_t side,int32_t door,bool redraw,bool even_walls)
{
	/*
	  #define dWALL		   0  //  000	0
	  #define dBOMB		   6  //  011	0
	  #define			  8  //  100	0
	  enum {dt_pass=0, dt_lock, dt_shut, dt_boss, dt_olck, dt_osht, dt_obos, dt_wall, dt_bomb, dt_walk, dt_max};
	  */
	
	if(door == dNONE || (!even_walls&&(door==dWALL||door==dWALK)))
		return;
	
	int32_t doortype;
	
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
		if(screenscrolling && ((HeroDir()^1)==side))
		{
			doortype=dt_osht;
			opendoors=-4;
			break;
		}

		[[fallthrough]];
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
			[[fallthrough]];
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
			[[fallthrough]];
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
			[[fallthrough]];
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
			[[fallthrough]];
		default:
			put_door(dest,t,78,side,doortype,redraw, even_walls);
			break;
		}
		
		break;
	}
}

void putcombo_not_zero(BITMAP *dest, int32_t x, int32_t y, int32_t combo, int32_t cset)
{
	if(combo!=0)
	{
		putcombo(dest,x, y, combo, cset);
	}
}

void overcombo_not_zero(BITMAP *dest, int32_t x, int32_t y, int32_t combo, int32_t cset)
{
    if(combo!=0)
    {
        overcombo(dest,x, y, combo, cset);
    }
}

void showbombeddoor(BITMAP *dest, int32_t side)
{
    int32_t d=tmpscr->door_combo_set;
    
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
	bool opened_door = false;
	for(int32_t i=0; i<4; i++)
		if(tmpscr->door[i]==dSHUTTER)
		{
			putdoor(scrollbuf,0,i,dOPENSHUTTER);
			tmpscr->door[i]=dOPENSHUTTER;
			opened_door = true;
		}
	
	for(auto lyr = 0; lyr < 7; ++lyr)
	{
		mapscr* scr = FFCore.tempScreens[lyr];
		for(auto pos = 0; pos < 176; ++pos)
		{
			newcombo const& cmb = combobuf[scr->data[pos]];
			if(cmb.triggerflags[0] & combotriggerSHUTTER)
				do_trigger_combo(lyr,pos);
		}
	}
	if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		word c = tmpscr->numFFC();
		for(word i=0; i<c; i++)
		{
			ffcdata& ffc = tmpscr->ffcs[i];
			newcombo const& cmb = combobuf[ffc.data];
			if(cmb.triggerflags[0] & combotriggerSHUTTER)
				do_trigger_combo_ffc(i);
		}
	}
	
	if(opened_door)
		sfx(WAV_DOOR,128);
}

void clear_darkroom_bitmaps()
{
	clear_to_color(darkscr_bmp_curscr, game->get_darkscr_color());
	clear_to_color(darkscr_bmp_curscr_trans, game->get_darkscr_color());
	clear_to_color(darkscr_bmp_scrollscr, game->get_darkscr_color());
	clear_to_color(darkscr_bmp_scrollscr_trans, game->get_darkscr_color());
}

void loadscr(int32_t tmp,int32_t destdmap, int32_t scr,int32_t ldir,bool overlay=false)
{
	if (tmp == 0)
	{
		zapp_reporting_set_tag("screen", scr);
		if (destdmap != -1)
			zapp_reporting_set_tag("dmap", destdmap);
	}

	auto oscr = homescr;
	homescr = scr;
	if (tmp == 0 && replay_is_active())
	{
		if (replay_get_mode() == ReplayMode::ManualTakeover)
			replay_stop_manual_takeover();

		if (destdmap != -1)
		{
			if (strlen(DMaps[destdmap].name) > 0)
			{
				replay_step_comment(fmt::format("dmap={} {}", destdmap, DMaps[destdmap].name));
			}
			else
			{
				replay_step_comment(fmt::format("dmap={}", destdmap));
			}
		}
		replay_step_comment_loadscr(scr);

		// Reset the rngs and frame count so that recording steps can be modified without impacting
		// behavior of later screens.
		replay_sync_rng();
	}
	if(!tmp)
	{
		slopes.clear();
		triggered_screen_secrets = false; //Reset var
		cpos_clear_all();
		Hero.clear_platform_ffc();
		timeExitAllGenscript(GENSCR_ST_CHANGE_SCREEN);
	}
	clear_darkroom_bitmaps();
	int32_t destlvl = DMaps[destdmap < 0 ? currdmap : destdmap].level;
	
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
	if (!tmp)
	{
		room_is_dark = (tmpscr[tmp].flags & fDARK);
		for(int i = 0; i < MAXFFCS; ++i)
		{
			tmpscr[tmp].ffcs[i].setLoaded(true);
			tmpscr[tmp].ffcs[i].solid_update(false);
			screen_ffc_modify_postroutine(i);
		}
	}
	
	tmpscr[tmp].valid |= mVALID; //layer 0 is always valid
	memcpy(tmpscr[tmp].data, TheMaps[currmap*MAPSCRS+scr].data, sizeof(tmpscr[tmp].data));
	memcpy(tmpscr[tmp].sflag, TheMaps[currmap*MAPSCRS+scr].sflag, sizeof(tmpscr[tmp].sflag));
	memcpy(tmpscr[tmp].cset, TheMaps[currmap*MAPSCRS+scr].cset, sizeof(tmpscr[tmp].cset));
	
	//screen / screendata script
	FFCore.reset_script_engine_data(ScriptType::Screen, currscr);
	FFCore.deallocateAllScriptOwnedOfType(ScriptType::Screen);
	FFCore.deallocateAllScriptOwnedOfType(ScriptType::Combo);
	FFCore.clear_combo_scripts();
	if ( TheMaps[currmap*MAPSCRS+scr].script > 0 )
	{
		tmpscr[tmp].script = TheMaps[currmap*MAPSCRS+scr].script;
		for ( int32_t q = 0; q < 8; q++ )
		{
			tmpscr[tmp].screeninitd[q] = TheMaps[currmap*MAPSCRS+scr].screeninitd[q];
		}
	}
	else
	{
		tmpscr[tmp].script = 0;
		FFCore.doscript(ScriptType::Screen, currscr) = false;
	}
	
	if(overlay)
	{
		for(int32_t c=0; c< 176; ++c)
		{
			if(tmpscr[tmp].data[c]==0)
			{
				tmpscr[tmp].data[c]=ffscr.data[c];
				tmpscr[tmp].sflag[c]=ffscr.sflag[c];
				tmpscr[tmp].cset[c]=ffscr.cset[c];
			}
		}
		
		for(int32_t i=0; i<6; i++)
		{
			if(ffscr.layermap[i]>0 && tmpscr[tmp].layermap[i]>0)
			{
				int32_t lm = (tmpscr[tmp].layermap[i]-1)*MAPSCRS+tmpscr[tmp].layerscreen[i];
				int32_t fm = (ffscr.layermap[i]-1)*MAPSCRS+ffscr.layerscreen[i];
				
				for(int32_t c=0; c< 176; ++c)
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
	
	if(tmp==0)
	{
		FFCore.deallocateAllScriptOwned(ScriptType::Screen, currscr);
		
		init_ffpos();
		for(word i = 0; i < MAXFFCS; i++)
		{
			if((ffscr.ffcs[i].flags&ffc_carryover) && !(ffscr.flags5&fNOFFCARRYOVER))
			{
				tmpscr[tmp].ffcs[i] = ffscr.ffcs[i];
				
				if (ffscr.ffcs[i].flags&ffc_scriptreset)
				{
					FFCore.reset_script_engine_data(ScriptType::FFC, i);
				}
			}
			else
			{
				FFCore.deallocateAllScriptOwned(ScriptType::FFC, i, false);
				memset(ffmisc[i], 0, 16 * sizeof(int32_t));
				FFCore.reset_script_engine_data(ScriptType::FFC, i);
			}
		}
	}
	
	if(tmp==0)
	{
		for(int32_t i=0; i<6; i++)
		{
			mapscr layerscr = tmpscr2[i];
			
			// Don't delete the old tmpscr2's data yet!
			if(tmpscr[tmp].layermap[i]>0)
			{
				tmpscr2[i]=TheMaps[(tmpscr[tmp].layermap[i]-1)*MAPSCRS+tmpscr[tmp].layerscreen[i]];
				
				if(overlay)
				{
					for(int32_t y=0; y<11; ++y)
					{
						for(int32_t x=0; x<16; ++x)
						{
							int32_t c=y*16+x;
							
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
	
	if(!tmp)
	{
		cpos_force_update();
		trig_trigger_groups();
	}
	if(canPermSecret(destdmap,scr)/*||TheMaps[(currmap*MAPSCRS)+currscr].flags6&fTRIGGERFPERM*/)
	{
		if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mSECRET)			   // if special stuff done before
		{
			hiddenstair(tmp,false);
			hidden_entrance(tmp,false,false,-3);
		}
		if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mLIGHTBEAM) // if special stuff done before
		{
			for(size_t layer = 0; layer < 7; ++layer)
			{
				mapscr* tscr = (tmp==0) ? FFCore.tempScreens[layer] : FFCore.ScrollingScreens[layer];
				for(size_t pos = 0; pos < 176; ++pos)
				{
					newcombo const* cmb = &combobuf[tscr->data[pos]];
					if(cmb->type == cLIGHTTARGET)
					{
						if(!(cmb->usrflags&cflag1)) //Unlit version
						{
							tscr->data[pos] += 1;
						}
					}
				}
			}
		}
	}
	
	toggle_switches(game->lvlswitches[destlvl], true, tmpscr + tmp, tmpscr2);
	toggle_gswitches_load(tmpscr+tmp, tmpscr2);
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mLOCKBLOCK)			  // if special stuff done before
	{
		remove_lockblocks(tmp);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mBOSSLOCKBLOCK)		  // if special stuff done before
	{
		remove_bosslockblocks(tmp);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mCHEST)			  // if special stuff done before
	{
		remove_chests(tmp);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mLOCKEDCHEST)			  // if special stuff done before
	{
		remove_lockedchests(tmp);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mBOSSCHEST)			  // if special stuff done before
	{
		remove_bosschests(tmp);
	}
	
	clear_xdoors(tmp, (currmap*MAPSCRSNORMAL)+scr);
	clear_xstatecombos(tmp, (currmap*MAPSCRSNORMAL)+scr);
	
	if(!tmp)
	{
		cpos_force_update();
		trig_trigger_groups();
	}
	// check doors
	if(isdungeon(destdmap,scr))
	{
		for(int32_t i=0; i<4; i++)
		{
			int32_t door=tmpscr[tmp].door[i];
			bool putit=true;
			
			switch(door)
			{
			case d1WAYSHUTTER:
			case dSHUTTER:
				if((ldir^1)==i)
				{
					tmpscr[tmp].door[i]=dOPENSHUTTER;
					//		  putit=false;
				}
				
				opendoors = -4;
				break;
				
			case dLOCKED:
				if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&(1<<i))
				{
					tmpscr[tmp].door[i]=dUNLOCKED;
					//		  putit=false;
				}
				
				break;
				
			case dBOSS:
				if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&(1<<i))
				{
					tmpscr[tmp].door[i]=dOPENBOSS;
					//		  putit=false;
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
	
	if(tmp == 0) //Reload slopes
		update_slope_comboposes();
	for(int32_t j=-1; j<6; ++j)  // j == -1 denotes the current screen
	{
		if(j<0 || (tmpscr[tmp].layermap[j]>0))
		{
			mapscr *layerscreen= (j<0 ? &tmpscr[tmp] : tmpscr2[j].valid ? &tmpscr2[j] :
								  &TheMaps[(tmpscr[tmp].layermap[j]-1)*MAPSCRS]+tmpscr[tmp].layerscreen[j]);
								  
			for(int32_t i=0; i<176; ++i)
			{
				int32_t c=layerscreen->data[i];
				int32_t cs=layerscreen->cset[i];
				
				// New screen flag: Cycle Combos At Screen Init
				if(combobuf[c].nextcombo != 0 && (tmpscr[tmp].flags3 & fCYCLEONINIT) && (j<0 || get_qr(qr_CMBCYCLELAYERS)))
				{
					int32_t r = 0;
					
					while(combobuf[c].nextcombo != 0 && r++ < 10)
					{
						layerscreen->data[i] = combobuf[c].nextcombo;
						if(!(combobuf[c].animflags & AF_CYCLENOCSET))
							layerscreen->cset[i] = combobuf[c].nextcset;
						c=layerscreen->data[i];
						cs=layerscreen->cset[i];
					}
				}
			}
		}
	}
	game->load_portals();
	if(!tmp)
	{
		throwGenScriptEvent(GENSCR_EVENT_CHANGE_SCREEN);
		if(Hero.lift_wpn && get_qr(qr_CARRYABLE_NO_ACROSS_SCREEN))
		{
			delete Hero.lift_wpn;
			Hero.lift_wpn = nullptr;
		}
	}
	homescr = oscr;
}

// Screen is being viewed by the Overworld Map viewer.
void loadscr2(int32_t tmp,int32_t scr,int32_t)
{
	auto oscr = homescr;
	homescr = scr;
	for(word x=0; x<animated_combos; x++)
	{
		if(combobuf[animated_combo_table4[x][0]].nextcombo!=0)
		{
			combobuf[animated_combo_table4[x][0]].aclk=0;
		}
	}
	
	tmpscr[tmp] = TheMaps[currmap*MAPSCRS+scr];
	
	if(tmp==0)
	{
		for(int32_t i=0; i<6; i++)
		{
			if(tmpscr[tmp].layermap[i]>0)
			{
				tmpscr2[i]=TheMaps[(tmpscr[tmp].layermap[i]-1)*MAPSCRS+tmpscr[tmp].layerscreen[i]];
			}
			else
			{
				(tmpscr2+i)->zero_memory();
			}
		}
	}
	
	if(canPermSecret(-1,scr))
	{
		if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mSECRET)			   // if special stuff done before
		{
			hiddenstair2(tmpscr+tmp,false);
			hidden_entrance2(tmpscr+tmp,tmpscr2,false,-3);
		}
		if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mLIGHTBEAM) // if special stuff done before
		{
			for(size_t layer = 0; layer < 7; ++layer)
			{
				mapscr* tscr = (tmp==0) ? FFCore.tempScreens[layer] : FFCore.ScrollingScreens[layer];
				for(size_t pos = 0; pos < 176; ++pos)
				{
					newcombo const* cmb = &combobuf[tscr->data[pos]];
					if(cmb->type == cLIGHTTARGET)
					{
						if(!(cmb->usrflags&cflag1)) //Unlit version
						{
							tscr->data[pos] += 1;
						}
					}
				}
			}
		}
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mLOCKBLOCK)			  // if special stuff done before
	{
		remove_lockblocks(tmp);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mBOSSLOCKBLOCK)		  // if special stuff done before
	{
		remove_bosslockblocks(tmp);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mCHEST)			  // if special stuff done before
	{
		remove_chests(tmp);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mLOCKEDCHEST)			  // if special stuff done before
	{
		remove_lockedchests(tmp);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mBOSSCHEST)			  // if special stuff done before
	{
		remove_bosschests(tmp);
	}
	
	clear_xdoors(tmp, (currmap*MAPSCRSNORMAL)+scr);
	clear_xstatecombos(tmp, (currmap*MAPSCRSNORMAL)+scr);
	
	// check doors
	if(isdungeon(scr))
	{
		for(int32_t i=0; i<4; i++)
		{
			int32_t door=tmpscr[tmp].door[i];
			bool putit=true;
			
			switch(door)
			{
			case d1WAYSHUTTER:
			case dSHUTTER:
				/*
						if((ldir^1)==i)
						{
						  tmpscr[tmp].door[i]=dOPENSHUTTER;
						  //		  putit=false;
						}
				*/
				break;
				
			case dLOCKED:
				if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&(1<<i))
				{
					tmpscr[tmp].door[i]=dUNLOCKED;
					//		  putit=false;
				}
				
				break;
				
			case dBOSS:
				if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&(1<<i))
				{
					tmpscr[tmp].door[i]=dOPENBOSS;
					//		  putit=false;
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
	
	for(int32_t j=-1; j<6; ++j)  // j == -1 denotes the current screen
	{
		if(j<0 || ((tmpscr[tmp].layermap[j]>0)))
		{
			mapscr *layerscreen= (j<0 ? &tmpscr[tmp]
								  : &(TheMaps[(tmpscr[tmp].layermap[j]-1)*MAPSCRS+tmpscr[tmp].layerscreen[j]]));
								  
			for(int32_t i=0; i<176; ++i)
			{
				int32_t c=layerscreen->data[i];
				int32_t cs=layerscreen->cset[i];
				
				// New screen flag: Cycle Combos At Screen Init
				if((tmpscr[tmp].flags3 & fCYCLEONINIT) && (j<0 || get_qr(qr_CMBCYCLELAYERS)))
				{
					int32_t r = 0;
					
					while(combobuf[c].nextcombo != 0 && r++ < 10)
					{
						layerscreen->data[i] = combobuf[c].nextcombo;
						if(!(combobuf[c].animflags & AF_CYCLENOCSET))
							layerscreen->cset[i] = combobuf[c].nextcset;
						c=layerscreen->data[i];
						cs=layerscreen->cset[i];
					}
				}
			}
		}
	}
	homescr = oscr;
}

void putscr(BITMAP* dest,int32_t x,int32_t y, mapscr* scrn)
{
	if(scrn->valid==0||!show_layer_0||scrn->hidelayers & 1)
	{
		rectfill(dest,x,y,x+255,y+175,0);
		return;
	}
	
	bool over = XOR(scrn->flags7&fLAYER2BG,DMaps[currdmap].flags&dmfLAYER2BG)
		|| XOR(scrn->flags7&fLAYER3BG, DMaps[currdmap].flags&dmfLAYER3BG);

	for(int32_t i=0; i<176; ++i)
	{
		draw_cmb_pos(dest, x, y, i, scrn->data[i], scrn->cset[i], 0, over, false);
	}
}

void putscrdoors(BITMAP *dest,int32_t x,int32_t y, mapscr* scrn)
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
static inline bool onSwitch(newcombo const& cmb, zfix const& switchblockstate)
{
	return (switchblockstate < 0 || (cmb.attributes[2]>0 && (zslongToFix(cmb.attributes[2]) - zslongToFix(zc_max(cmb.attributes[3], 0))) <=switchblockstate));
}
bool _walkflag(zfix_round zx,zfix_round zy,int32_t cnt)
{
	return _walkflag(zx,zy,cnt,0_zf);
}
bool _walkflag(zfix_round zx,zfix_round zy,int32_t cnt,zfix const& switchblockstate)
{
	int x = zx.getRound(), y = zy.getRound();
	//  walkflagx=x; walkflagy=y;
	if(get_qr(qr_LTTPWALK))
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
    s1=(tmpscr2->valid)?tmpscr2:tmpscr;
    s2=(tmpscr2[1].valid)?tmpscr2+1:tmpscr;
	//  s2=TheMaps+((*tmpscr).layermap[1]-1)MAPSCRS+((*tmpscr).layerscreen[1]);
	
	int32_t bx=(x>>4)+(y&0xF0);
	const newcombo* c = &combobuf[tmpscr->data[bx]];
	const newcombo* c1 = &combobuf[s1->data[bx]];
	const newcombo* c2 = &combobuf[s2->data[bx]];
	bool dried = (((iswater_type(c->type)) || (iswater_type(c1->type)) ||
				   (iswater_type(c2->type))) && DRIEDLAKE);
	int32_t b=1;
	
	if(x&8) b<<=2;
	
	if(y&8) b<<=1;
	
	int32_t cwalkflag = c->walk;
	if(onSwitch(*c,switchblockstate) && c->type == cCSWITCHBLOCK && c->usrflags&cflag9) cwalkflag &= (c->walk>>4)^0xF;
	else if ((c->type == cBRIDGE && get_qr(qr_OLD_BRIDGE_COMBOS)) || (iswater_type(c->type) && ((c->walk>>4)&b) && ((c->usrflags&cflag3) || (c->usrflags&cflag4)))) cwalkflag = 0;
	if (s1 != tmpscr)
	{
		if(onSwitch(*c1,switchblockstate) && c1->type == cCSWITCHBLOCK && c1->usrflags&cflag9) cwalkflag &= (c1->walk>>4)^0xF;
		else if ((iswater_type(c1->type) && ((c1->walk>>4)&b) && get_qr(qr_WATER_ON_LAYER_1) && !((c1->usrflags&cflag3) || (c1->usrflags&cflag4)))) cwalkflag &= c1->walk;
		else if (c1->type == cBRIDGE)
		{
			if (!get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				int efflag = (c1->walk & 0xF0)>>4;
				int newsolid = (c1->walk & 0xF);
				cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
			}
			else cwalkflag &= c1->walk;
		}
		else if ((iswater_type(c1->type) && get_qr(qr_WATER_ON_LAYER_1) && ((c1->usrflags&cflag3) || (c1->usrflags&cflag4)) && ((c1->walk>>4)&b))) cwalkflag = 0;
		else cwalkflag |= c1->walk;
	}
	if (s2 != tmpscr)
	{
		if(onSwitch(*c2,switchblockstate) && c2->type == cCSWITCHBLOCK && c2->usrflags&cflag9) cwalkflag &= (c2->walk>>4)^0xF;
		else if ((iswater_type(c2->type) && ((c2->walk>>4)&b) && get_qr(qr_WATER_ON_LAYER_2) && !((c2->usrflags&cflag3) || (c2->usrflags&cflag4)))) cwalkflag &= c2->walk;
		else if (c2->type == cBRIDGE)
		{
			if (!get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				int efflag = (c2->walk & 0xF0)>>4;
				int newsolid = (c2->walk & 0xF);
				cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
			}
			else cwalkflag &= c2->walk;
		}
		else if ((iswater_type(c2->type) && get_qr(qr_WATER_ON_LAYER_2) && ((c2->usrflags&cflag3) || (c2->usrflags&cflag4))) && ((c2->walk>>4)&b)) cwalkflag = 0;
		else cwalkflag |= c2->walk;
	}
	
	if((cwalkflag&b) && !dried)
		return true;
	
	if(cnt == 1)
		return collide_object(zx,zy,0.0001_zf, 0.0001_zf);
	else if(collide_object(zx, zy, 8.0001_zf, 0.0001_zf)) return true;
	
	++bx;
	
	if(!(x&8))
		b<<=2;
	else
	{
		c  = &combobuf[tmpscr->data[bx]];
		c1 = &combobuf[s1->data[bx]];
		c2 = &combobuf[s2->data[bx]];
		dried = (((iswater_type(c->type)) || (iswater_type(c1->type)) ||
				  (iswater_type(c2->type))) && DRIEDLAKE);
		b=1;
		
		if(y&8) b<<=1;
	}
	cwalkflag = c->walk;
	if(onSwitch(*c,switchblockstate) && c->type == cCSWITCHBLOCK && c->usrflags&cflag9) cwalkflag &= (c->walk>>4)^0xF;
	else if ((c->type == cBRIDGE && get_qr(qr_OLD_BRIDGE_COMBOS)) || (iswater_type(c->type) && ((c->walk>>4)&b) && ((c->usrflags&cflag3) || (c->usrflags&cflag4)))) cwalkflag = 0;
	if (s1 != tmpscr)
	{
		if(onSwitch(*c1,switchblockstate) && c1->type == cCSWITCHBLOCK && c1->usrflags&cflag9) cwalkflag &= (c1->walk>>4)^0xF;
		else if ((iswater_type(c1->type) && ((c1->walk>>4)&b) && get_qr(qr_WATER_ON_LAYER_1) && !((c1->usrflags&cflag3) || (c1->usrflags&cflag4)))) cwalkflag &= c1->walk;
		else if (c1->type == cBRIDGE)
		{
			if (!get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				int efflag = (c1->walk & 0xF0)>>4;
				int newsolid = (c1->walk & 0xF);
				cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
			}
			else cwalkflag &= c1->walk;
		}
		else if ((iswater_type(c1->type) && get_qr(qr_WATER_ON_LAYER_1) && ((c1->usrflags&cflag3) || (c1->usrflags&cflag4))) && ((c1->walk>>4)&b)) cwalkflag = 0;
		else cwalkflag |= c1->walk;
	}
	if (s2 != tmpscr)
	{
		if(onSwitch(*c2,switchblockstate) && c2->type == cCSWITCHBLOCK && c2->usrflags&cflag9) cwalkflag &= (c2->walk>>4)^0xF;
		else if ((iswater_type(c2->type) && ((c2->walk>>4)&b) && get_qr(qr_WATER_ON_LAYER_2) && !((c2->usrflags&cflag3) || (c2->usrflags&cflag4)))) cwalkflag &= c2->walk;
		else if (c2->type == cBRIDGE)
		{
			if (!get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				int efflag = (c2->walk & 0xF0)>>4;
				int newsolid = (c2->walk & 0xF);
				cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
			}
			else cwalkflag &= c2->walk;
		}
		else if ((iswater_type(c2->type) && get_qr(qr_WATER_ON_LAYER_2) && ((c2->usrflags&cflag3) || (c2->usrflags&cflag4))) && ((c2->walk>>4)&b)) cwalkflag = 0;
		else cwalkflag |= c2->walk;
	}
	return (cwalkflag&b) ? !dried : false;
}

bool _effectflag(int32_t x,int32_t y,int32_t cnt, int32_t layer, bool notLink)
{
	//  walkflagx=x; walkflagy=y;
	if(get_qr(qr_LTTPWALK) || notLink)
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
    s1=(tmpscr2->valid)?tmpscr2:tmpscr;
    s2=(tmpscr2[1].valid)?tmpscr2+1:tmpscr;
	//  s2=TheMaps+((*tmpscr).layermap[1]-1)MAPSCRS+((*tmpscr).layerscreen[1]);
	
	if (layer == 0 && (s1 == tmpscr)) return false;
	if (layer == 1 && (s2 == tmpscr)) return false;
	
	int32_t bx=(x>>4)+(y&0xF0);
	const newcombo* c = &combobuf[tmpscr->data[bx]];
	const newcombo* c1 = &combobuf[s1->data[bx]];
	const newcombo* c2 = &combobuf[s2->data[bx]];
	bool dried = (((iswater_type(c->type)) || (iswater_type(c1->type)) ||
				   (iswater_type(c2->type))) && DRIEDLAKE);
	int32_t b=1;
	
	if(x&8) b<<=2;
	
	if(y&8) b<<=1;
	
	int32_t cwalkflag = (c->walk>>4);
	if (layer == 0) cwalkflag = (c1->walk>>4);
	if (layer == 1) cwalkflag = (c2->walk>>4);
	//if (c->type == cBRIDGE || (iswater_type(c->type) && ((c->usrflags&cflag3) || (c->usrflags&cflag4)))) cwalkflag = 0;
	if (s1 != tmpscr && layer < 0)
	{
		if (c1->type == cBRIDGE) cwalkflag &= (~(c1->walk>>4));
	}
	if (s2 != tmpscr && layer < 1)
	{
		if (c2->type == cBRIDGE) cwalkflag &= (~(c2->walk>>4));
	}
	
	if((cwalkflag&b) && !dried)
		return true;
		
	if(cnt==1) return false;
	
	++bx;
	
	if(!(x&8))
		b<<=2;
	else
	{
		c  = &combobuf[tmpscr->data[bx]];
		c1 = &combobuf[s1->data[bx]];
		c2 = &combobuf[s2->data[bx]];
		dried = (((iswater_type(c->type)) || (iswater_type(c1->type)) ||
				  (iswater_type(c2->type))) && DRIEDLAKE);
		b=1;
		
		if(y&8) b<<=1;
	}
	cwalkflag = (c->walk>>4);
	if (layer == 0) cwalkflag = (c1->walk>>4);
	if (layer == 1) cwalkflag = (c2->walk>>4);
	//if (c->type == cBRIDGE || (iswater_type(c->type) && ((c->usrflags&cflag3) || (c->usrflags&cflag4)))) cwalkflag = 0;
	if (s1 != tmpscr && layer < 0)
	{
		if (c1->type == cBRIDGE) 
		{
			cwalkflag &= (~(c1->walk>>4));
		}
	}
	if (s2 != tmpscr && layer < 1)
	{
		if (c2->type == cBRIDGE) 
		{
			cwalkflag &= (~(c2->walk>>4));
		}
	}
	return (cwalkflag&b) ? !dried : false;
}

//used by mapdata->isSolid(x,y) in ZScript:
bool _walkflag(zfix_round zx,zfix_round zy,int32_t cnt, mapscr* m)
{
	int x = zx.getRound(), y = zy.getRound();
	//  walkflagx=x; walkflagy=y;
	if(get_qr(qr_LTTPWALK))
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
	
	int32_t bx=(x>>4)+(y&0xF0);
	const newcombo* c = &combobuf[m->data[bx]];
	const newcombo* c1 = &combobuf[s1->data[bx]];
	const newcombo* c2 = &combobuf[s2->data[bx]];
	bool dried = (((iswater_type(c->type)) || (iswater_type(c1->type)) ||
				   (iswater_type(c2->type))) && DRIEDLAKE);
	int32_t b=1;
	
	if(x&8) b<<=2;
	
	if(y&8) b<<=1;
	
	int32_t cwalkflag = c->walk;
	if (c1->type == cBRIDGE)
	{
		if (!get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			int efflag = (c1->walk & 0xF0)>>4;
			int newsolid = (c1->walk & 0xF);
			cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
		}
		else cwalkflag &= c1->walk;
	}
	else if (s1 != m) cwalkflag |= c1->walk;
	if (c2->type == cBRIDGE)
	{
		if (!get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			int efflag = (c2->walk & 0xF0)>>4;
			int newsolid = (c2->walk & 0xF);
			cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
		}
		else cwalkflag &= c2->walk;
	}
	else if (s2 != m) cwalkflag |= c2->walk;
	
	if((cwalkflag&b) && !dried)
		return true;
		
	if(cnt==1) return false;
	
	++bx;
	
	if(!(x&8))
		b<<=2;
	else
	{
		c  = &combobuf[m->data[bx]];
		c1 = &combobuf[s1->data[bx]];
		c2 = &combobuf[s2->data[bx]];
		dried = (((iswater_type(c->type)) || (iswater_type(c1->type)) ||
				  (iswater_type(c2->type))) && DRIEDLAKE);
		b=1;
		
		if(y&8) b<<=1;
	}
	
	cwalkflag = c->walk;
	if (c1->type == cBRIDGE)
	{
		if (!get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			int efflag = (c1->walk & 0xF0)>>4;
			int newsolid = (c1->walk & 0xF);
			cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
		}
		else cwalkflag &= c1->walk;
	}
	else if (s1 != m) cwalkflag |= c1->walk;
	if (c2->type == cBRIDGE)
	{
		if (!get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			int efflag = (c2->walk & 0xF0)>>4;
			int newsolid = (c2->walk & 0xF);
			cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
		}
		else cwalkflag &= c2->walk;
	}
	else if (s2 != m) cwalkflag |= c2->walk;
	return (cwalkflag&b) ? !dried : false;
}

bool _walkflag(zfix_round zx,zfix_round zy,int32_t cnt, mapscr* m, mapscr* s1, mapscr* s2)
{
	int x = zx.getRound(), y = zy.getRound();
	//  walkflagx=x; walkflagy=y;
	if(get_qr(qr_LTTPWALK))
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
	
	int32_t bx=(x>>4)+(y&0xF0);
	const newcombo* c = &combobuf[m->data[bx]];
	const newcombo* c1 = &combobuf[s1->data[bx]];
	const newcombo* c2 = &combobuf[s2->data[bx]];
	bool dried = (((iswater_type(c->type)) || (iswater_type(c1->type)) ||
				   (iswater_type(c2->type))) && DRIEDLAKE);
	int32_t b=1;
	
	if(x&8) b<<=2;
	
	if(y&8) b<<=1;
	
	int32_t cwalkflag = c->walk;
	if (c1->type == cBRIDGE)
	{
		if (!get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			int efflag = (c1->walk & 0xF0)>>4;
			int newsolid = (c1->walk & 0xF);
			cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
		}
		else cwalkflag &= c1->walk;
	}
	else if (s1 != m) cwalkflag |= c1->walk;
	if (c2->type == cBRIDGE)
	{
		if (!get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			int efflag = (c2->walk & 0xF0)>>4;
			int newsolid = (c2->walk & 0xF);
			cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
		}
		else cwalkflag &= c2->walk;
	}
	else if (s2 != m) cwalkflag |= c2->walk;
	
	if((cwalkflag&b) && !dried)
		return true;
		
	if(cnt==1) return false;
	
	++bx;
	
	if(!(x&8))
		b<<=2;
	else
	{
		c  = &combobuf[m->data[bx]];
		c1 = &combobuf[s1->data[bx]];
		c2 = &combobuf[s2->data[bx]];
		dried = (((iswater_type(c->type)) || (iswater_type(c1->type)) ||
				  (iswater_type(c2->type))) && DRIEDLAKE);
		b=1;
		
		if(y&8) b<<=1;
	}
	
	cwalkflag = c->walk;
	if (c1->type == cBRIDGE)
	{
		if (!get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			int efflag = (c1->walk & 0xF0)>>4;
			int newsolid = (c1->walk & 0xF);
			cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
		}
		else cwalkflag &= c1->walk;
	}
	else if (s1 != m) cwalkflag |= c1->walk;
	if (c2->type == cBRIDGE) 
	{
		if (!get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			int efflag = (c2->walk & 0xF0)>>4;
			int newsolid = (c2->walk & 0xF);
			cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
		}
		else cwalkflag &= c2->walk;
	}
	else if (s2 != m) cwalkflag |= c2->walk;
	return (cwalkflag&b) ? !dried : false;
}

//Only check the given mapscr*, not its layer 1&2
bool _walkflag_layer(zfix_round zx,zfix_round zy,int32_t cnt, mapscr* m)
{
	int x = zx.getRound(), y = zy.getRound();
	//  walkflagx=x; walkflagy=y;
	if(get_qr(qr_LTTPWALK))
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
	
	int32_t bx=(x>>4)+(y&0xF0);
	const newcombo* c = &combobuf[m->data[bx]];
	bool dried = ((iswater_type(c->type)) && DRIEDLAKE);
	int32_t b=1;
	
	if(x&8) b<<=2;
	
	if(y&8) b<<=1;
	
	if((c->walk&b) && !dried)
		return true;
		
	if(cnt==1) return false;
	
	++bx;
	
	if(!(x&8))
		b<<=2;
	else
	{
		c  = &combobuf[m->data[bx]];
		dried = ((iswater_type(c->type)) && DRIEDLAKE);
		b=1;
		
		if(y&8) b<<=1;
	}
	
	return (c->walk&b) ? !dried : false;
}

bool _effectflag_layer(int32_t x,int32_t y,int32_t cnt, mapscr* m, bool notLink)
{
	//  walkflagx=x; walkflagy=y;
	if(get_qr(qr_LTTPWALK) || notLink)
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
	
	int32_t bx=(x>>4)+(y&0xF0);
	const newcombo* c = &combobuf[m->data[bx]];
	bool dried = ((iswater_type(c->type)) && DRIEDLAKE);
	int32_t b=1;
	
	if(x&8) b<<=2;
	
	if(y&8) b<<=1;
	
	if(((c->walk>>4)&b) && !dried)
		return true;
		
	if(cnt==1) return false;
	
	++bx;
	
	if(!(x&8))
		b<<=2;
	else
	{
		c  = &combobuf[m->data[bx]];
		dried = ((iswater_type(c->type)) && DRIEDLAKE);
		b=1;
		
		if(y&8) b<<=1;
	}
	
	return ((c->walk>>4)&b) ? !dried : false;
}

bool water_walkflag(int32_t x,int32_t y,int32_t cnt)
{
	if(get_qr(qr_LTTPWALK))
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
    s1=(tmpscr2->valid)?tmpscr2:tmpscr;
    s2=(tmpscr2[1].valid)?tmpscr2+1:tmpscr;
	
	int32_t bx=(x>>4)+(y&0xF0);
	const newcombo* c = &combobuf[tmpscr->data[bx]];
	const newcombo* c1 = &combobuf[s1->data[bx]];
	const newcombo* c2 = &combobuf[s2->data[bx]];
	int32_t b=1;
	
	if(x&8) b<<=2;
	
	if(y&8) b<<=1;
	
	if(get_qr(qr_NO_SOLID_SWIM))
	{
		if(c->walk&b)
			return true;
			
		if(c1->walk&b)
			return true;
			
		if(c2->walk&b)
			return true;
	}
	else
	{
		if((c->walk&b) && !iswater_type(c->type))
			return true;
			
		if((c1->walk&b) && !iswater_type(c1->type))
			return true;
			
		if((c2->walk&b) && !iswater_type(c2->type))
			return true;
	}
	if(cnt==1) return false;
	
	if(x&8)
		b<<=2;
	else
	{
		c = &combobuf[tmpscr->data[++bx]];
		c1 = &combobuf[s1->data[bx]];
		c2 = &combobuf[s2->data[bx]];
		b=1;
		
		if(y&8) b<<=1;
	}
	
	if(get_qr(qr_NO_SOLID_SWIM))
	{
		return (c->walk&b) || (c1->walk&b) || (c2->walk&b);
	}
	else return (c->walk&b) ? !iswater_type(c->type) :
		   (c1->walk&b) ? !iswater_type(c1->type) :
		   (c2->walk&b) ? !iswater_type(c2->type) :false;
}

bool hit_walkflag(int32_t x,int32_t y,int32_t cnt)
{
	if(dlevel)
		if(x<32 || y<40 || (x+(cnt-1)*8)>=224 || y>=144)
			return true;
			
	if(blockpath && y<((get_qr(qr_LTTPCOLLISION))?80:88))
		return true;
		
	if(x<16 || y<16 || (x+(cnt-1)*8)>=240 || y>=160)
		return true;
		
	//  for(int32_t i=0; i<4; i++)
	if(mblock2.clk && mblock2.hit(x,y,0,cnt*8,1,16))
		return true;
	
	if (collide_object(x, y,cnt*8, 1))
		return true;
		
	return _walkflag(x,y,cnt);
}

bool solpush_walkflag(int32_t x, int32_t y, int32_t cnt, solid_object const* ign)
{
	if(x<0 || y<0 || x>=256 || y>=176)
		return true;
		
	//  for(int32_t i=0; i<4; i++)
	if(mblock2.clk && mblock2.hit(x,y,0,cnt*8,1,16))
		return true;
	
	if (collide_object(x, y,cnt*8, 1, ign))
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
		
		for(int32_t i=0; i<guys.Count(); i++)
		{
			if(((enemy*)guys.spr(i))->bgsfx)
				stop_sfx(((enemy*)guys.spr(i))->bgsfx);
		}
	}
}

void toggle_switches(dword flags, bool entry)
{
	if(!flags) return; //No flags to toggle
	toggle_switches(flags,entry,tmpscr,tmpscr2);
}
void toggle_switches(dword flags, bool entry, mapscr* m, mapscr* t)
{
	if(!flags) return; //No flags to toggle
	bool iscurscr = (m==tmpscr && t==tmpscr2);
	byte togglegrid[176] = {0};
	for(int32_t lyr = 0; lyr < 7; ++lyr)
	{
		mapscr* scr = (lyr ? &t[lyr-1] : m);
		for(int32_t pos = 0; pos < 176; ++pos)
		{
			newcombo const& cmb = combobuf[scr->data[pos]];
			if(iscurscr)
				if((cmb.triggerflags[3] & combotriggerTRIGLEVELSTATE) && cmb.trig_lstate < 32)
					if(flags&(1<<cmb.trig_lstate))
						do_trigger_combo(lyr,pos,ctrigSWITCHSTATE);
			if((cmb.type == cCSWITCH || cmb.type == cCSWITCHBLOCK) && cmb.attribytes[0] < 32
				&& !(cmb.usrflags & cflag11)) //global state
			{
				if(flags&(1<<cmb.attribytes[0]))
				{
					set<int32_t> oldData;
					//Increment the combo/cset by the attributes
					int32_t cmbofs = (cmb.attributes[0]/10000L);
					int32_t csofs = (cmb.attributes[1]/10000L);
					oldData.insert(scr->data[pos]);
					scr->data[pos] = BOUND_COMBO(scr->data[pos] + cmbofs);
					scr->cset[pos] = (scr->cset[pos] + csofs) & 15;
					if(entry && (cmb.usrflags&cflag8))
					{
						newcombo const* tmp = &combobuf[scr->data[pos]];
						while(tmp->nextcombo && (oldData.find(tmp->nextcombo) == oldData.end()))
						{
							scr->data[pos] = tmp->nextcombo;
							if(!(tmp->animflags & AF_CYCLENOCSET))
								scr->cset[pos] = tmp->nextcset;
							oldData.insert(tmp->nextcombo);
							tmp = &combobuf[tmp->nextcombo];
						}
					}
					int32_t cmbid = scr->data[pos];
					if(combobuf[cmbid].animflags & AF_CYCLE)
					{
						combobuf[cmbid].tile = combobuf[cmbid].o_tile;
						combobuf[cmbid].cur_frame=0;
						combobuf[cmbid].aclk = 0;
					}
					togglegrid[pos] |= (1<<lyr); //Mark this pos toggled for this layer
					if(cmb.type == cCSWITCH) continue; //Switches don't toggle other layers
					for(int32_t lyr2 = 0; lyr2 < 7; ++lyr2) //Toggle same pos on other layers, if flag set
					{
						if(lyr==lyr2) continue;
						if(!(cmb.usrflags&(1<<lyr2))) continue;
						if(togglegrid[pos]&(1<<lyr2)) continue;
						mapscr* scr_2 = (lyr2 ? &t[lyr2-1] : m);
						if(!scr_2->data[pos]) //Don't increment empty space
							continue;
						newcombo const& cmb_2 = combobuf[scr_2->data[pos]];
						if(lyr2 > lyr && (cmb_2.type == cCSWITCH || cmb_2.type == cCSWITCHBLOCK) && !(cmb.usrflags & cflag11)
								&& cmb_2.attribytes[0] < 32 && (flags&(1<<cmb_2.attribytes[0])))
							continue; //This is a switch/block that will be hit later in the loop!
						set<int32_t> oldData2;
						//Increment the combo/cset by the original cmb's attributes
						oldData2.insert(scr_2->data[pos]);
						scr_2->data[pos] = BOUND_COMBO(scr_2->data[pos] + cmbofs);
						scr_2->cset[pos] = (scr_2->cset[pos] + csofs) & 15;
						if(entry && (cmb.usrflags&cflag8)) //Skip cycling on screen entry
						{
							newcombo const* tmp = &combobuf[scr_2->data[pos]];
							while(tmp->nextcombo && (oldData2.find(tmp->nextcombo) == oldData2.end()))
							{
								scr_2->data[pos] = tmp->nextcombo;
								if(!(tmp->animflags & AF_CYCLENOCSET))
									scr_2->cset[pos] = tmp->nextcset;
								oldData2.insert(tmp->nextcombo);
								tmp = &combobuf[tmp->nextcombo];
							}
						}
						int32_t cmbid2 = scr_2->data[pos];
						if(combobuf[cmbid2].animflags & AF_CYCLE)
						{
							combobuf[cmbid2].tile = combobuf[cmbid2].o_tile;
							combobuf[cmbid2].cur_frame=0;
							combobuf[cmbid2].aclk = 0;
						}
						togglegrid[pos] |= (1<<lyr2); //Mark this pos toggled for this layer
					}
				}
			}
		}
	}
	if(get_qr(qr_SWITCHES_AFFECT_MOVINGBLOCKS) && mblock2.clk)
	{
		newcombo const& cmb = combobuf[mblock2.bcombo];
		if(!(cmb.usrflags & cflag11) && (cmb.type == cCSWITCH || cmb.type == cCSWITCHBLOCK) && cmb.attribytes[0] < 32)
		{
			if(flags&(1<<cmb.attribytes[0]))
			{
				//Increment the combo/cset by the attributes
				int32_t cmbofs = (cmb.attributes[0]/10000L);
				int32_t csofs = (cmb.attributes[1]/10000L);
				mblock2.bcombo = BOUND_COMBO(mblock2.bcombo + cmbofs);
				mblock2.cs = (mblock2.cs + csofs) & 15;
				int32_t cmbid = mblock2.bcombo;
				if(combobuf[cmbid].animflags & AF_CYCLE)
				{
					combobuf[cmbid].tile = combobuf[cmbid].o_tile;
					combobuf[cmbid].cur_frame=0;
					combobuf[cmbid].aclk = 0;
				}
			}
		}
	}
	
	if(iscurscr)
	{
		word c = m->numFFC();
		for(word q=0; q<c; ++q)
		{
			newcombo const& cmb = combobuf[m->ffcs[q].data];
			if((cmb.triggerflags[3] & combotriggerTRIGLEVELSTATE) && cmb.trig_lstate < 32)
				if(flags&(1<<cmb.trig_lstate))
					do_trigger_combo_ffc(q,ctrigSWITCHSTATE);
		}
	}
}

void toggle_gswitches(int32_t state, bool entry)
{
	toggle_gswitches(state,entry,tmpscr,tmpscr2);
}
void toggle_gswitches(int32_t state, bool entry, mapscr* m, mapscr* t)
{
	bool states[256] = {false};
	states[state] = true;
	toggle_gswitches(states, entry, m, t);
}
void toggle_gswitches(bool* states, bool entry, mapscr* m, mapscr* t)
{
	if(!states) return;
	bool iscurscr = (m==tmpscr && t==tmpscr2);
	byte togglegrid[176] = {0};
	for(int32_t lyr = 0; lyr < 7; ++lyr)
	{
		mapscr* scr = (lyr ? &t[lyr-1] : m);
		for(int32_t pos = 0; pos < 176; ++pos)
		{
			newcombo const& cmb = combobuf[scr->data[pos]];
			if(iscurscr)
				if(cmb.triggerflags[3] & combotriggerTRIGGLOBALSTATE)
					if(states[cmb.trig_gstate])
						do_trigger_combo(lyr,pos,ctrigSWITCHSTATE);
			if(!(cmb.usrflags & cflag11)) //not global state
				continue;
			if(cmb.type == cCSWITCH || cmb.type == cCSWITCHBLOCK)
			{
				if(states[cmb.attribytes[0]])
				{
					set<int32_t> oldData;
					//Increment the combo/cset by the attributes
					int32_t cmbofs = (cmb.attributes[0]/10000L);
					int32_t csofs = (cmb.attributes[1]/10000L);
					oldData.insert(scr->data[pos]);
					scr->data[pos] = BOUND_COMBO(scr->data[pos] + cmbofs);
					scr->cset[pos] = (scr->cset[pos] + csofs) & 15;
					if(entry && (cmb.usrflags&cflag8))
					{
						newcombo const* tmp = &combobuf[scr->data[pos]];
						while(tmp->nextcombo && (oldData.find(tmp->nextcombo) == oldData.end()))
						{
							scr->data[pos] = tmp->nextcombo;
							if(!(tmp->animflags & AF_CYCLENOCSET))
								scr->cset[pos] = tmp->nextcset;
							oldData.insert(tmp->nextcombo);
							tmp = &combobuf[tmp->nextcombo];
						}
					}
					int32_t cmbid = scr->data[pos];
					if(combobuf[cmbid].animflags & AF_CYCLE)
					{
						combobuf[cmbid].tile = combobuf[cmbid].o_tile;
						combobuf[cmbid].cur_frame=0;
						combobuf[cmbid].aclk = 0;
					}
					togglegrid[pos] |= (1<<lyr); //Mark this pos toggled for this layer
					if(cmb.type == cCSWITCH) continue; //Switches don't toggle other layers
					for(int32_t lyr2 = 0; lyr2 < 7; ++lyr2) //Toggle same pos on other layers, if flag set
					{
						if(lyr==lyr2) continue;
						if(!(cmb.usrflags&(1<<lyr2))) continue;
						if(togglegrid[pos]&(1<<lyr2)) continue;
						mapscr* scr_2 = (lyr2 ? &t[lyr2-1] : m);
						if(!scr_2->data[pos]) //Don't increment empty space
							continue;
						newcombo const& cmb_2 = combobuf[scr_2->data[pos]];
						if(lyr2 > lyr && (cmb_2.type == cCSWITCH || cmb_2.type == cCSWITCHBLOCK)
							&& (cmb_2.usrflags & cflag11) && (states[cmb_2.attribytes[0]]))
							continue; //This is a switch/block that will be hit later in the loop!
						set<int32_t> oldData2;
						//Increment the combo/cset by the original cmb's attributes
						oldData2.insert(scr_2->data[pos]);
						scr_2->data[pos] = BOUND_COMBO(scr_2->data[pos] + cmbofs);
						scr_2->cset[pos] = (scr_2->cset[pos] + csofs) & 15;
						if(entry && (cmb.usrflags&cflag8)) //Skip cycling on screen entry
						{
							newcombo const* tmp = &combobuf[scr_2->data[pos]];
							while(tmp->nextcombo && (oldData2.find(tmp->nextcombo) == oldData2.end()))
							{
								scr_2->data[pos] = tmp->nextcombo;
								if(!(tmp->animflags & AF_CYCLENOCSET))
									scr_2->cset[pos] = tmp->nextcset;
								oldData2.insert(tmp->nextcombo);
								tmp = &combobuf[tmp->nextcombo];
							}
						}
						int32_t cmbid2 = scr_2->data[pos];
						if(combobuf[cmbid2].animflags & AF_CYCLE)
						{
							combobuf[cmbid2].tile = combobuf[cmbid2].o_tile;
							combobuf[cmbid2].cur_frame=0;
							combobuf[cmbid2].aclk = 0;
						}
						togglegrid[pos] |= (1<<lyr2); //Mark this pos toggled for this layer
					}
				}
			}
		}
	}
	
	if(get_qr(qr_SWITCHES_AFFECT_MOVINGBLOCKS) && mblock2.clk)
	{
		newcombo const& cmb = combobuf[mblock2.bcombo];
		if((cmb.type == cCSWITCH || cmb.type == cCSWITCHBLOCK) && (cmb.usrflags & cflag11))
		{
			if(states[cmb.attribytes[0]])
			{
				//Increment the combo/cset by the attributes
				int32_t cmbofs = (cmb.attributes[0]/10000L);
				int32_t csofs = (cmb.attributes[1]/10000L);
				mblock2.bcombo = BOUND_COMBO(mblock2.bcombo + cmbofs);
				mblock2.cs = (mblock2.cs + csofs) & 15;
				int32_t cmbid = mblock2.bcombo;
				if(combobuf[cmbid].animflags & AF_CYCLE)
				{
					combobuf[cmbid].tile = combobuf[cmbid].o_tile;
					combobuf[cmbid].cur_frame=0;
					combobuf[cmbid].aclk = 0;
				}
			}
		}
	}
	
	if(iscurscr)
	{
		word c = m->numFFC();
		for(word q=0; q<c; ++q)
		{
			newcombo const& cmb = combobuf[m->ffcs[q].data];
			if(cmb.triggerflags[3] & combotriggerTRIGGLOBALSTATE)
				if(states[cmb.trig_gstate])
					do_trigger_combo_ffc(q,ctrigSWITCHSTATE);
		}
	}
}
void toggle_gswitches_load(mapscr* m, mapscr* t)
{
	bool states[256] = {false};
	for(auto q = 0; q < 256; ++q)
	{
		states[q] = game->gswitch_timers[q] != 0;
	}
	toggle_gswitches(states, true, m, t);
}
void run_gswitch_timers()
{
	bool states[256] = {false};
	auto& m = game->gswitch_timers.mut_inner();
	for(auto it = m.begin(); it != m.end();)
	{
		if(it->second > 0)
			if(!--it->second)
			{
				states[it->first] = true;
				it = m.erase(it);
				continue;
			}
		++it;
	}
	toggle_gswitches(states, false, tmpscr, tmpscr2);
}
void onload_gswitch_timers() //Reset all timers that were counting down, no trigger necessary
{
	for(auto q = 0; q < 256; ++q)
	{
		if(game->gswitch_timers[q] > 0)
			game->gswitch_timers[q] = 0;
	}
}

/****  View Map  ****/

//BITMAP *mappic = NULL;
int32_t mapres = 0;
int32_t view_map_show_mode = 3;

bool displayOnMap(int32_t x, int32_t y)
{
    int32_t s = (y<<4) + x;
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
	
	for(int32_t i=0; i<6; ++i)
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
	
	int32_t px = ((8-(currscr&15)) << 9)  - 256;
	int32_t py = ((4-(currscr>>4)) * 352) - 176;
	int32_t lx = ((currscr&15)<<8)  + HeroX()+8;
	int32_t ly = ((currscr>>4)*176) + HeroY()+8;
	int32_t sc = 6;
	
	bool done=false, redraw=true;
	
	mappic = create_bitmap_ex(8,(256*16)>>mapres,(176*8)>>mapres);
	
	if(!mappic)
	{
		enter_sys_pal();
		jwin_alert("View Map","Not enough memory.",NULL,NULL,"OK",NULL,13,27,get_zc_font(font_lfont));
		exit_sys_pal();
		return;
	}
	
	// draw the map
	set_clip_rect(scrollbuf, 0, 0, scrollbuf->w, scrollbuf->h);
	
	for(int32_t y=0; y<8; y++)
	{
		for(int32_t x=0; x<16; x++)
		{
			rectfill(scrollbuf, 256, 0, 511, 223, WHITE);
			if(displayOnMap(x, y))
			{
				int32_t s = (y<<4) + x;
				tmpscr[0].zero_memory();
				tmpscr[1].zero_memory();
				loadscr2(1,s,-1);
				tmpscr[0] = tmpscr[1];
				if(tmpscr[0].valid&mVALID)
				{
					for(int32_t i=0; i<6; i++)
					{
						tmpscr2[i].zero_memory();
						if(tmpscr[0].layermap[i]<=0)
							continue;
						
						tmpscr2[i]=TheMaps[(tmpscr[0].layermap[i]-1)*MAPSCRS+tmpscr[0].layerscreen[i]];
					}
					
					if(XOR((tmpscr)->flags7&fLAYER2BG, DMaps[currdmap].flags&dmfLAYER2BG)) do_layer(scrollbuf, 0, 2, tmpscr, -256, playing_field_offset, 2);
					
					if(XOR((tmpscr)->flags7&fLAYER3BG, DMaps[currdmap].flags&dmfLAYER3BG)) do_layer(scrollbuf, 0, 3, tmpscr, -256, playing_field_offset, 2);
					
					if(lenscheck(tmpscr,0)) putscr(scrollbuf,256,0,tmpscr);
					do_layer(scrollbuf, 0, 1, tmpscr, -256, playing_field_offset, 2);
					
					if(!XOR(((tmpscr)->flags7&fLAYER2BG), DMaps[currdmap].flags&dmfLAYER2BG)) do_layer(scrollbuf, 0, 2, tmpscr, -256, playing_field_offset, 2);
					
					putscrdoors(scrollbuf,256,0,tmpscr);
					if(get_qr(qr_PUSHBLOCK_SPRITE_LAYER))
					{
						do_layer(scrollbuf,-2, 0, tmpscr, -256, playing_field_offset, 2);
						if(get_qr(qr_PUSHBLOCK_LAYER_1_2))
						{
							do_layer(scrollbuf,-2, 1, tmpscr, -256, playing_field_offset, 2);
							do_layer(scrollbuf,-2, 2, tmpscr, -256, playing_field_offset, 2);
						}
					}
					do_layer(scrollbuf,-3, 0, tmpscr, -256, playing_field_offset, 2); // Freeform combos!
					
					if(!XOR(((tmpscr)->flags7&fLAYER3BG), DMaps[currdmap].flags&dmfLAYER3BG)) do_layer(scrollbuf, 0, 3, tmpscr, -256, playing_field_offset, 2);
					
					do_layer(scrollbuf, 0, 4, tmpscr, -256, playing_field_offset, 2);
					do_layer(scrollbuf,-1, 0, tmpscr, -256, playing_field_offset, 2);
					if(get_qr(qr_OVERHEAD_COMBOS_L1_L2))
					{
						do_layer(scrollbuf,-1, 1, tmpscr, -256, playing_field_offset, 2);
						do_layer(scrollbuf,-1, 2, tmpscr, -256, playing_field_offset, 2);
					}
					do_layer(scrollbuf, 0, 5, tmpscr, -256, playing_field_offset, 2);
					do_layer(scrollbuf, 0, 6, tmpscr, -256, playing_field_offset, 2);
				}
			}
			
			stretch_blit(scrollbuf, mappic, 256, 0, 256, 176, x<<(8-mapres), (y*176)>>mapres, 256>>mapres, 176>>mapres);
		}
	}
	
	for(int32_t i=0; i<6; ++i)
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
	int32_t delay = 0;
	
	do
	{
		if (replay_version_check(0, 11))
			load_control_state();

		int32_t step = int32_t(16.0/scales[sc]);
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
			--view_map_show_mode;
			
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
						 int32_t(256+(int64_t(px)-mappic->w)*scale)/2,int32_t(224+(int64_t(py)-mappic->h)*scale)/2,
						 int32_t(mappic->w*scale),int32_t(mappic->h*scale));
						 
			blit(framebuf,scrollbuf,0,0,256,0,256,224);
			redraw=false;
		}
		
		int32_t x = int32_t(256+(px-((2048-int64_t(lx))*2))*scale)/2;
		int32_t y = int32_t(224+(py-((704-int64_t(ly))*2))*scale)/2;
		
		if(view_map_show_mode&1)
		{
			line(framebuf,x-7,y-7,x+7,y+7,(frame&3)+252);
			line(framebuf,x+7,y-7,x-7,y+7,(frame&3)+252);
		}
		
		if(view_map_show_mode&2 || r)
			textprintf_ex(framebuf,font,224,216,WHITE,BLACK,"%1.2f",scale);
			
		if(r)
		{
			textprintf_ex(framebuf,font,0,208,WHITE,BLACK,"m: %d %d",px,py);
			textprintf_ex(framebuf,font,0,216,WHITE,BLACK,"x: %d %d",x,y);
		}
		
		advanceframe(false, false);
		if (replay_version_check(11))
			load_control_state();
		
		if(getInput(btnS, true, false, true)) //rSbtn
			done = true;
			
	}
	while(!done && !Quit);
	
	destroy_bitmap(mappic);
	
	resume_all_sfx();
}

int32_t onViewMap()
{
    if(Playing && currscr<128 && DMaps[currdmap].flags&dmfVIEWMAP)
    {
        clear_to_color(framebuf,BLACK);
        textout_centre_ex(framebuf,font,"Drawing map...",128,108,WHITE,BLACK);
        advanceframe(true);
        ViewMap();
    }
    
    return D_O_K;
}

bool isGrassType(int32_t type)
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

bool isFlowersType(int32_t type)
{
    switch(type)
    {
    case cFLOWERS:
    case cFLOWERSTOUCHY:
        return true;
    }
    
    return false;
}

bool isGenericType(int32_t type)
{
    switch(type)
    {
    case cTRIGGERGENERIC:
        return true;
    }
    
    return false;
}

bool isBushType(int32_t type)
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

bool isSlashType(int32_t type)
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

bool isCuttableNextType(int32_t type)
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

bool isTouchyType(int32_t type)
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

bool isCuttableType(int32_t type)
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

bool isCuttableItemType(int32_t type)
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

bool is_push(mapscr* m, int32_t pos)
{
	if(is_push_flag(m->sflag[pos]))
		return true;
	newcombo const& cmb = combobuf[m->data[pos]];
	if(is_push_flag(cmb.flag))
		return true;
	if(cmb.type == cPUSHBLOCK)
		return true;
	if(cmb.type == cSWITCHHOOK && (cmb.usrflags&cflag7))
		return true; //Counts as 'pushblock' flag
	return false;
}

optional<int32_t> get_combo(int x, int y, int maxlayer, bool ff, std::function<bool(newcombo const&)> proc)
{
	if(unsigned(x) >= 256 || unsigned(y) >= 176)
		return nullopt;
	if(ff)
	{
		int ffcid = MAPFFCOMBO(x,y);
		if(ffcid && proc(combobuf[ffcid]))
			return ffcid;
	}
	if(maxlayer > 6)
		maxlayer = 6;
	int pos = COMBOPOS(x,y);
	for(int lyr = maxlayer; lyr >= 0; --lyr)
	{
		mapscr* m = FFCore.tempScreens[lyr];
		int cid = m->data[pos];
		newcombo const& cmb = combobuf[cid];
		if(_effectflag_layer(x,y,1,m,true) && proc(cmb))
		{
			for(int i = lyr; i <=1; ++i)
			{
				if(!tmpscr2[i].valid) continue;
				
				auto tcid = MAPCOMBO2(i,x,y);
				if(combobuf[tcid].type == cBRIDGE)
				{
					if (get_qr(qr_OLD_BRIDGE_COMBOS))
					{
						if (!_walkflag_layer(x,y,1,&(tmpscr2[i]))) return nullopt;
					}
					else
					{
						if (_effectflag_layer(x,y,1,&(tmpscr2[i]), true)) return nullopt;
					}
				}
			}
			return cid;
		}
	}
	return nullopt;
}

