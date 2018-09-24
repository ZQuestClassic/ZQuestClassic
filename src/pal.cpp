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

int CSET_SIZE = 16;                                         // this is only changed to 4 in the NES title screen
int CSET_SHFT = 4;                                          // log2 of CSET_SIZE

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

RGB _RGB(int r,int g,int b)
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

RGB mixRGB(int r1,int g1,int b1,int r2,int g2,int b2,int ratio)
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
    for(int i=0; i<256; i++)
        dest[i]=src[i];
}

void loadfullpal()
{
    for(int i=0; i<240; i++)
        RAMpal[i]=_RGB(colordata+i*3);
        
    for(int i=240; i<255; i++)
        RAMpal[i]=((RGB*)data[PAL_GUI].dat)[i];
        
    refreshpal=true;
}

/*void loadlvlpal256(int level)
  {
  byte *si = colordata + */

extern PALETTE tempgreypal;

void loadlvlpal(int level)
{
    byte *si = colordata + CSET(level*pdLEVEL+poLEVEL)*3;
    
    for(int i=0; i<16*3; i++)
    {
        RAMpal[CSET(2)+i] = _RGB(si);
	    tempgreypal[CSET(2)+i] = _RGB(si); //preserve monochrome
        si+=3;
    }
    
    for(int i=0; i<16; i++)
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
    
    for(int q=0; q<PAL_SIZE; q++)
    {
        trans_table2.data[0][q] = q;
        trans_table2.data[q][q] = q;
    }
    
    //! We need to store the new palette into the monochrome scratch palette. 
    //memcpy(tempgreypal, RAMpal, PAL_SIZE*sizeof(RGB));
    //! Doing this is bad, because we are also copying over the sprite palettes.
    
    
    
    if ( isMonochrome () ) {
	//memcpy(tempgreypal, RAMpal, PAL_SIZE*sizeof(RGB));
	    
	//Refresh the monochrome palette to avoid gfx glitches from loading the lpal.  
	setMonochrome(false);
	setMonochrome(true);
    }
    
    refreshpal=true;
}

void loadpalset(int cset,int dataset)
{
    int j = CSET(dataset)*3;
    
    for(int i=0; i<16; i++,j+=3)
    {
	   // if ( isMonochrome() ) tempgreypal[CSET(2)+i] = _RGB(&colordata[j]); //Use monochrome sprites and Link pal... 
	    if ( isMonochrome() ) tempgreypal[CSET(cset)+i] = _RGB(&colordata[j]); //Use monochrome sprites and Link pal... 
		else 
			RAMpal[CSET(cset)+i] = _RGB(&colordata[j]); 
    }
    
    if(cset==6 && !get_bit(quest_rules,qr_NOLEVEL3FIX) && DMaps[currdmap].color==3){

	    RAMpal[CSET(6)+2] = NESpal(0x37);
    }
    refreshpal=true;
}

void ringcolor(bool forceDefault)
{
    int itemid = current_item_id(itype_ring);
    
    if(!forceDefault && itemid>-1)
    {
        loadpalset(6,itemsbuf[itemid].misc1 ? pSprite(zc_min(29,itemsbuf[itemid].misc1)):6);
    }
    else
    {
        loadpalset(6,6);
    }
    
    refreshpal=true;
}

void loadfadepal(int dataset)
{
    byte *si = colordata + CSET(dataset)*3;
    
    for(int i=0; i<pdFADE*16; i++)
    {
        RAMpal[CSET(2)+i] = _RGB(si);
        si+=3;
    }
    
    refreshpal=true;
}

void interpolatedfade()
{
    int dpos = 64;
    int lpos = 32;
    int last = CSET(5)-1;
    
    if(get_bit(quest_rules,qr_FADECS5))
    {
        last += 16;
        loadpalset(5,5);
    }
    
    loadlvlpal(DMaps[currdmap].color);
    byte *si = colordata + CSET(DMaps[currdmap].color*pdLEVEL+poFADE1)*3;
    
    for(int i=0; i<16; i++)
    {
        int light = si[0]+si[1]+si[2];
        si+=3;
        fade_interpolate(RAMpal,black_palette,RAMpal,light?lpos:dpos,CSET(2)+i,CSET(2)+i);
    }
    
    fade_interpolate(RAMpal,black_palette,RAMpal,dpos,CSET(3),last);
    refreshpal=true;
}

void fade(int level,bool blackall,bool fromblack)
{
    int cx = fromblack ? 30 : 0;
    
    for(int i=0; i<=30; i+=(get_bit(quest_rules,qr_FADE))?2:1)
    {
        if(get_bit(quest_rules,qr_FADE))
        {
            int dpos = (cx<<6)/30;
            int lpos = zc_min(dpos,blackall?64:32);
            int last = CSET(5)-1;
            
            if(get_bit(quest_rules,qr_FADECS5))
            {
                last += 16;
                loadpalset(5,5);
            }
            
            loadlvlpal(level);
            byte *si = colordata + CSET(level*pdLEVEL+poFADE1)*3;
            
            for(int j=0; j<16; ++j)
            {
                int light = si[0]+si[1]+si[2];
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
                    for(int j=0; j<pdFADE*16; j++)
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
void lighting(bool existslight, bool setnaturaldark, int specialstate)
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
    
    if(darkroom != newstate)
    {
int level = (Link.getSpecialCave()>0) ? (Link.getSpecialCave()>=GUYCAVE) ? 10 : 11 : DMaps[currdmap].color;

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
                int last = CSET(5)-1;
                int light;
                
                if(get_bit(quest_rules,qr_FADECS5))
                {
                    last += 16;
                    loadpalset(5,5);
                }
                
                byte *si = colordata + CSET(level*pdLEVEL+poFADE1)*3;
                
                for(int j=0; j<16; ++j)
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
        
        for(int q=0; q<PAL_SIZE; q++)
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
            for(int i=0; i<4 && !Quit; i++)
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

static int palclk[3];
static int palpos[3];

void reset_pal_cycling()
{
    for(int i=0; i<3; i++)
        palclk[i]=palpos[i]=0;
}

void cycle_palette()
{
    if(!get_bit(quest_rules,qr_FADE) || darkroom)
        return;
        
    int level = (Link.getSpecialCave()==0) ? DMaps[currdmap].color : (Link.getSpecialCave()<GUYCAVE ? 11 : 10);
    
    for(int i=0; i<3; i++)
    {
        palcycle c = QMisc.cycles[level][i];
        
        if(c.count&0xF0)
        {
            if(++palclk[i] >= c.speed)
            {
                palclk[i]=0;
                
                if(++palpos[i] >= (c.count>>4))
                    palpos[i]=0;
                    
                byte *si = colordata + CSET(level*pdLEVEL+poFADE1+1+palpos[i])*3;
                si += (c.first&15)*3;
                
                for(int col=c.first&15; col<=(c.count&15); col++)
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

int reverse_NESpal(RGB c)
{
    int dist = 12000;
    int index = 0;
    
    for(int i = 0; (i < 64) && (dist != 0); i++)
    {
        int r = (c.r - NESpal(i).r);
        int g = (c.g - NESpal(i).g);
        int b = (c.b - NESpal(i).b);
        int d = r*r + g*g + b*b;
        
        if(d < dist)
        {
            dist = d;
            index = i;
        }
    }
    
    return index;
}

/* end of pal.cc */

