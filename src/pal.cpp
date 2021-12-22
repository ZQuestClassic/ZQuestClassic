//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  pal.cc
//
//  Palette code for ZC.
//
//--------------------------------------------------------


#include "precompiled.h" //always first

#include "zc_alleg.h"
#include "zdefs.h"
#include "maps.h"
#include "zelda.h"
#include "link.h"
#include "colors.h"
#include "zsys.h"
#include "pal.h"
#include "subscr.h"

extern LinkClass Link;

int32_t CSET_SIZE = 16;                                         // this is only changed to 4 in the NES title screen
int32_t CSET_SHFT = 4;                                          // log2 of CSET_SIZE

bool stayLit = false;

bool usingdrypal = false; //using dried up lake colors
RGB olddrypal; //palette to restore when lake rehydrates

RGB _RGB(byte *si)
{
    RGB x;
    x.r = si[0];
    x.g = si[1];
    x.b = si[2];
    x.filler=0; // Suppress warning "used undefined"
    return x;
}

RGB _RGB(int32_t r,int32_t g,int32_t b)
{
    RGB x;
    x.r = r;
    x.g = g;
    x.b = b;
    x.filler=0; // Suppress warning "used undefined"
    return x;
}

RGB invRGB(RGB s)
{
    RGB x;
    x.r = 63-s.r;
    x.g = 63-s.g;
    x.b = 63-s.b;
    x.filler=0; // Suppress warning "used undefined"
    return x;
}

RGB mixRGB(int32_t r1,int32_t g1,int32_t b1,int32_t r2,int32_t g2,int32_t b2,int32_t ratio)
{
    RGB x;
    x.r = (r1*(64-ratio) + r2*ratio) >> 6;
    x.g = (g1*(64-ratio) + g2*ratio) >> 6;
    x.b = (b1*(64-ratio) + b2*ratio) >> 6;
    x.filler=0; // Suppress warning "used undefined"
    return x;
}

void copy_pal(RGB *src,RGB *dest)
{
    for(int32_t i=0; i<256; i++)
        dest[i]=src[i];
}

void loadfullpal()
{
    for(int32_t i=0; i<240; i++)
        RAMpal[i]=_RGB(colordata+i*3);
        
    for(int32_t i=240; i<255; i++)
        RAMpal[i]=((RGB*)data[PAL_GUI].dat)[i];
        
    refreshpal=true;
}

/*void loadlvlpal256(int32_t level)
  {
  byte *si = colordata + */

extern PALETTE tempgreypal;
extern PALETTE userPALETTE[256];

void loadlvlpal(int32_t level)
{
    byte *si = colordata + CSET(level*pdLEVEL+poLEVEL)*3;
    
    for(int32_t i=0; i<16*3; i++)
    {
        RAMpal[CSET(2)+i] = _RGB(si);
	    tempgreypal[CSET(2)+i] = _RGB(si); //preserve monochrome
        si+=3;
    }
    
    for(int32_t i=0; i<16; i++)
    {
        RAMpal[CSET(9)+i] = _RGB(si);
	    tempgreypal[CSET(9)+i] = _RGB(si); //preserve monochrome
        si+=3;
    }
    
    if(!get_bit(quest_rules,qr_NOLEVEL3FIX) && level==3) {
        RAMpal[CSET(6)+2] = NESpal(0x37);
	tempgreypal[CSET(6)+2] = NESpal(0x37);
    }
        
    create_rgb_table(&rgb_table, RAMpal, NULL);
    create_zc_trans_table(&trans_table, RAMpal, 128, 128, 128);
    memcpy(&trans_table2, &trans_table, sizeof(COLOR_MAP));
    
    for(int32_t q=0; q<PAL_SIZE; q++)
    {
        trans_table2.data[0][q] = q;
        trans_table2.data[q][q] = q;
    }
    
    //! We need to store the new palette into the monochrome scratch palette. 
    //memcpy(tempgreypal, RAMpal, PAL_SIZE*sizeof(RGB));
    //! Doing this is bad, because we are also copying over the sprite palettes.
    
    
    /* Old handling for monochrome, before tint features added -V
    if ( isMonochrome () ) {
	//memcpy(tempgreypal, RAMpal, PAL_SIZE*sizeof(RGB));
	    
	//Refresh the monochrome palette to avoid gfx glitches from loading the lpal.  
	setMonochrome(false);
	setMonochrome(true);
    }*/
    
    if(isMonochrome()){
	    if(lastMonoPreset){
		    restoreMonoPreset();
	    } else {
			setMonochrome(false);
		    setMonochrome(true);
	    }
    }
    
    if(isUserTinted()){
	    restoreTint();
    }
    
    refreshpal=true;
}

void loadpalset(int32_t cset,int32_t dataset)
{
    int32_t j = CSET(dataset)*3;
    
    for(int32_t i=0; i<16; i++,j+=3)
    {
	   // if ( isMonochrome() ) tempgreypal[CSET(2)+i] = _RGB(&colordata[j]); //Use monochrome sprites and Link pal... 
	    if ( isMonochrome() || isUserTinted() ) tempgreypal[CSET(cset)+i] = _RGB(&colordata[j]); //Use monochrome sprites and Link pal... 
		else 
			RAMpal[CSET(cset)+i] = _RGB(&colordata[j]); 
    }
	
	if(isMonochrome()){
	    if(lastMonoPreset){
		    restoreMonoPreset();
	    } else {
			setMonochrome(false);
		    setMonochrome(true);
	    }
    }
    
    if(isUserTinted()){
	    restoreTint();
    }
    
    if(cset==6 && !get_bit(quest_rules,qr_NOLEVEL3FIX) && DMaps[currdmap].color==3){

	    RAMpal[CSET(6)+2] = NESpal(0x37);
    }
    refreshpal=true;
}

void ringcolor(bool forceDefault)
{
    int32_t itemid = current_item_id(itype_ring);
    
    if(!forceDefault && itemid>-1)
    {
        loadpalset(6,itemsbuf[itemid].misc1 ? pSprite(zc_min((pdSPRITE-1),itemsbuf[itemid].misc1)):6);
    }
    else
    {
        loadpalset(6,6);
    }
    
    refreshpal=true;
}

void loadfadepal(int32_t dataset)
{
    byte *si = colordata + CSET(dataset)*3;
    
    for(int32_t i=0; i<pdFADE*16; i++)
    {
        if(isMonochrome() || isUserTinted())tempgreypal[CSET(2)+i] = _RGB(si);
		else RAMpal[CSET(2)+i] = _RGB(si);
        si+=3;
    }
    
    refreshpal=true;
	
	if(isMonochrome()){
	    if(lastMonoPreset){
		    restoreMonoPreset();
	    } else {
			setMonochrome(false);
		    setMonochrome(true);
	    }
    }
    
    if(isUserTinted()){
	    restoreTint();
    }
}

void interpolatedfade()
{
    int32_t dpos = 64;
    int32_t lpos = 32;
    int32_t last = CSET(5)-1;
    
    if(get_bit(quest_rules,qr_FADECS5))
    {
        last += 16;
        loadpalset(5,5);
    }
    
    loadlvlpal(DMaps[currdmap].color);
    byte *si = colordata + CSET(DMaps[currdmap].color*pdLEVEL+poFADE1)*3;
    
    for(int32_t i=0; i<16; i++)
    {
        int32_t light = si[0]+si[1]+si[2];
        si+=3;
        fade_interpolate(RAMpal,black_palette,RAMpal,light?lpos:dpos,CSET(2)+i,CSET(2)+i);
    }
    
    fade_interpolate(RAMpal,black_palette,RAMpal,dpos,CSET(3),last);
    refreshpal=true;
}

void fade(int32_t level,bool blackall,bool fromblack)
{
    int32_t cx = fromblack ? 30 : 0;
    
    for(int32_t i=0; i<=30; i+=(get_bit(quest_rules,qr_FADE))?2:1)
    {
        if(get_bit(quest_rules,qr_FADE))
        {
            int32_t dpos = (cx<<6)/30;
            int32_t lpos = zc_min(dpos,blackall?64:32);
            int32_t last = CSET(5)-1;
            
            if(get_bit(quest_rules,qr_FADECS5))
            {
                last += 16;
                loadpalset(5,5);
            }
            
            loadlvlpal(level);
            byte *si = colordata + CSET(level*pdLEVEL+poFADE1)*3;
            
            for(int32_t j=0; j<16; ++j)
            {
                int32_t light = si[0]+si[1]+si[2];
                si+=3;
                fade_interpolate(RAMpal,black_palette,RAMpal,light?lpos:dpos,CSET(2)+j,CSET(2)+j);
            }
            
            fade_interpolate(RAMpal,black_palette,RAMpal,dpos,CSET(3),last);
            refreshpal=true;
        }
        else
        {
            switch(cx)
            {
            case 0:
                loadlvlpal(level);
                break;
                
            case 10:
                loadfadepal(level*pdLEVEL+poFADE1);
                break;
                
            case 20:
                loadfadepal(level*pdLEVEL+poFADE2);
                break;
                
            case 30:
                if(blackall)
                {
                    for(int32_t j=0; j<pdFADE*16; j++)
                        RAMpal[CSET(2)+j]=black_palette[0];
                        
                    refreshpal=true;
                }
                else
                    loadfadepal(level*pdLEVEL+poFADE3);
                    
                break;
            }
        }
        
        if(!get_bit(quest_rules,qr_NOLEVEL3FIX) && level==3)
            RAMpal[CSET(6)+2] = NESpal(0x37);
            
        //put_passive_subscr(framebuf,0,passive_subscreen_offset,false,false);
        advanceframe(true);
        
        if(Quit)
            break;
            
        fromblack ? --cx : ++cx;
        
        if(get_bit(quest_rules,qr_FADE))
        {
            fromblack ? --cx : ++cx;
        }
    }
    
    // Make sure the palette is set exactly right before returning...
    if(fromblack)
        loadlvlpal(level);
}


// false: change screen lighting to naturaldark
// true: lighten room
void lighting(bool existslight, bool setnaturaldark, int32_t specialstate)
{
	switch(specialstate){
		case pal_litOVERRIDE:
			stayLit=existslight;
			break;
		case pal_litRESET:
			stayLit=false;
			break;
		case pal_litSET:
			stayLit=true;
			break;
		case pal_litRESETONLY:
			stayLit=false;
			return;
	}
	if(stayLit)
	{
		existslight=true;
	}
    bool newstate = !existslight && (setnaturaldark ? ((TheMaps[currmap*MAPSCRS+currscr].flags&fDARK) != 0) : naturaldark);
    if(get_bit(quest_rules, qr_NEW_DARKROOM)) newstate = false;
    if(darkroom != newstate)
    {
		fade((Link.getSpecialCave()>0) ? (Link.getSpecialCave()>=GUYCAVE) ? 10 : 11 : DMaps[currdmap].color, false, darkroom);
        darkroom = newstate;
    }
    
    if(setnaturaldark)
        naturaldark = newstate;
}

// Only used during Insta-Warps
void lightingInstant()
{
	stayLit=false;
    bool newstate = ((TheMaps[currmap*MAPSCRS+currscr].flags&fDARK) != 0);
    if(get_bit(quest_rules, qr_NEW_DARKROOM)) newstate = false;
    if(darkroom != newstate)
    {
		int32_t level = (Link.getSpecialCave()>0) ? (Link.getSpecialCave()>=GUYCAVE) ? 10 : 11 : DMaps[currdmap].color;

        if(darkroom) // Old room dark, new room lit
        {
            loadlvlpal(level);
            
            if(get_bit(quest_rules,qr_FADECS5))
                loadpalset(5,5);
        }
        else // Old room lit, new room dark
        {
            if(get_bit(quest_rules,qr_FADE))
            {
                int32_t last = CSET(5)-1;
                int32_t light;
                
                if(get_bit(quest_rules,qr_FADECS5))
                {
                    last += 16;
                    loadpalset(5,5);
                }
                
                byte *si = colordata + CSET(level*pdLEVEL+poFADE1)*3;
                
                for(int32_t j=0; j<16; ++j)
                {
                    light = si[0]+si[1]+si[2];
                    si+=3;
                    fade_interpolate(RAMpal,black_palette,RAMpal,light?32:64,CSET(2)+j,CSET(2)+j);
                }
                
                fade_interpolate(RAMpal,black_palette,RAMpal,64,CSET(3),last);
            }
            else // No interpolated fading
                loadfadepal(level*pdLEVEL+poFADE3);
        }
        
        if(!get_bit(quest_rules,qr_NOLEVEL3FIX) && level==3)
            RAMpal[CSET(6)+2] = NESpal(0x37);
            
        create_rgb_table(&rgb_table, RAMpal, NULL);
        create_zc_trans_table(&trans_table, RAMpal, 128, 128, 128);
        memcpy(&trans_table2, &trans_table, sizeof(COLOR_MAP));
        
        for(int32_t q=0; q<PAL_SIZE; q++)
        {
            trans_table2.data[0][q] = q;
            trans_table2.data[q][q] = q;
        }
        
        darkroom = newstate;
    }
    
    naturaldark = newstate;
}

byte drycolors[11] = {0x12,0x11,0x22,0x21,0x31,0x32,0x33,0x35,0x34,0x36,0x37};

void dryuplake()
{
    if(whistleclk<0 || whistleclk>=88)
        return;
        
    if((++whistleclk)&7)
        return;
        
    if(whistleclk<88)
    {
        if(tmpscr->flags7 & fWHISTLEPAL)
        {
            if(!usingdrypal)
            {
                usingdrypal = true;
                olddrypal = RAMpal[CSET(3)+3];
            }
            
            RAMpal[CSET(3)+3] = NESpal(drycolors[whistleclk>>3]);
            refreshpal=true;
            
        }
    }
    else
    {
        if(tmpscr->flags & fWHISTLE)
        {
            if(hiddenstair(0,true))
            {
                sfx(tmpscr->secretsfx);
            }
        }
    }
}

void rehydratelake(bool instant)
{
    if(whistleclk==-1)
        return;
        
    if(instant && usingdrypal)
    {
        usingdrypal = false;
        RAMpal[CSET(3)+3] = olddrypal;
        refreshpal=true;
        return;
    }
    
    whistleclk&=0xF8;
    
    do
    {
        whistleclk-=8;
        
        if(usingdrypal)
        {
            RAMpal[CSET(3)+3] = NESpal(drycolors[whistleclk>>3]);
            refreshpal=true;
        }
        
        advanceframe(true);
        
        if(((whistleclk>>3)&3) == 1)
            for(int32_t i=0; i<4 && !Quit; i++)
                advanceframe(true);
    }
    while(whistleclk!=0 && !Quit);
    
    whistleclk=-1;
    
    if(usingdrypal)
    {
        usingdrypal = false;
        RAMpal[CSET(3)+3] = olddrypal;
        refreshpal=true;
    }
}

static int32_t palclk[3];
static int32_t palpos[3];

void reset_pal_cycling()
{
    for(int32_t i=0; i<3; i++)
        palclk[i]=palpos[i]=0;
}

void cycle_palette()
{
    if(!get_bit(quest_rules,qr_FADE) || darkroom)
        return;
        
    int32_t level = (Link.getSpecialCave()==0) ? DMaps[currdmap].color : (Link.getSpecialCave()<GUYCAVE ? 11 : 10);
    palcycle cycle_none[1][3];  //create a null palette cycle here. -Z
	memset(cycle_none, 0, sizeof(cycle_none)); 
    for(int32_t i=0; i<3; i++)
    {
        palcycle c = ( level < 256 ) ? QMisc.cycles[level][i] : cycle_none[0][i]; //Only 0 through 255 have valid data in 2.50.x. -Z
        
        if(c.count&0xF0)
        {
            if(++palclk[i] >= c.speed)
            {
                palclk[i]=0;
                
                if(++palpos[i] >= (c.count>>4))
                    palpos[i]=0;
                    
                byte *si = colordata + CSET(level*pdLEVEL+poFADE1+1+palpos[i])*3;
                si += (c.first&15)*3;
                
                for(int32_t col=c.first&15; col<=(c.count&15); col++)
                {
                    RAMpal[CSET(c.first>>4)+col] = _RGB(si);
                    si+=3;
                }
                
                refreshpal=true;
            }
        }
    }
    
    // No need to do handle refreshpal here; it's done in updatescr().
}

int32_t reverse_NESpal(RGB c)
{
    int32_t dist = 12000;
    int32_t index = 0;
    
    for(int32_t i = 0; (i < 64) && (dist != 0); i++)
    {
        int32_t r = (c.r - NESpal(i).r);
        int32_t g = (c.g - NESpal(i).g);
        int32_t b = (c.b - NESpal(i).b);
        int32_t d = r*r + g*g + b*b;
        
        if(d < dist)
        {
            dist = d;
            index = i;
        }
    }
    
    return index;
}

/* end of pal.cc */

