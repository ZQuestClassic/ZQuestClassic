#include "base/zc_alleg.h"
#include <cstring>
#include <emmintrin.h>
#include <tmmintrin.h>

#include "base/zdefs.h"
#include "base/zsys.h"
#include "base/qrs.h"
#include "base/combo.h"
#include "tiles.h"
#include "base/pal_tables.h"
#include "zc/combos.h"
#include "zc/maps.h"
#include "items.h"

extern wpndata    *wpnsbuf;
tiledata *newtilebuf, *grabtilebuf;
int32_t animated_combo_table[MAXCOMBOS][2];                    //[0]=position in act2, [1]=original tile
int32_t animated_combo_table4[MAXCOMBOS][2];                   //[0]=combo, [1]=clock
int32_t animated_combos;
int32_t animated_combo_table2[MAXCOMBOS][2];                    //[0]=position in act2, [1]=original tile
int32_t animated_combo_table24[MAXCOMBOS][2];                   //[0]=combo, [1]=clock
int32_t animated_combos2;
bool blank_tile_table[NEWMAXTILES];                         //keeps track of blank tiles
bool used_tile_table[NEWMAXTILES];                          //keeps track of used tiles
bool blank_tile_quarters_table[NEWMAXTILES*4];              //keeps track of blank tile quarters
extern zfix  HeroModifiedX();
extern zfix  HeroModifiedY();

bool unused_tile_table[NEWMAXTILES];                  //keeps track of unused tiles

byte unpackbuf[256];

extern bool is_in_scrolling_region();

bool isblanktile(tiledata *buf, int32_t i)
{
    byte *tilestart=buf[i].data;
    qword *di=(qword*)tilestart;
    int32_t parts=tilesize(buf[i].format)>>3;
    
    for(int32_t j=0; j<parts; ++j, ++di)
    {
        if(*di!=0)
        {
            return false;
        }
    }
    
    return true;
}


const char *tileformat_string[tfMax] =
{
    "(Invalid)", "4-bit", "8-bit", "16-bit (unsupported)", "24-bit (unsupported)", "32-bit (unsupported)"
};

void register_blank_tile_quarters(int32_t tile)
{
    //  byte *tilestart=tilebuf+(tile*128);
    dword *di=(dword*)newtilebuf[tile].data;
    blank_tile_quarters_table[(tile<<2)]=true;
    blank_tile_quarters_table[(tile<<2)+1]=true;
    blank_tile_quarters_table[(tile<<2)+2]=true;
    blank_tile_quarters_table[(tile<<2)+3]=true;
    
    int32_t parts=tilesize(newtilebuf[tile].format)>>6;
    
    for(int32_t r=0; r<16; ++r)
    {
        for(int32_t j=0; j<parts; ++j, ++di)
        {
            if(*di!=0)
            {
                int32_t q=(tile<<2) + ((r/8)<<1) + ((j>(parts/2-1))?1:0);
                blank_tile_quarters_table[q]=false;
            }
        }
    }
}



void register_blank_tiles()
{
    for(int32_t i=0; i<NEWMAXTILES; ++i)
    {
        register_blank_tile_quarters(i);
        blank_tile_table[i]=isblanktile(newtilebuf, i);
    }
}

void register_blank_tiles(int32_t max)
{
    for(int32_t i=0; i<max; ++i)
    {
        register_blank_tile_quarters(i);
        blank_tile_table[i]=isblanktile(newtilebuf, i);
    }
}

//returns the number of tiles
int32_t count_tiles(tiledata *buf)
{
    int32_t tiles_used;
    
    for(tiles_used=(NEWMAXTILES); tiles_used>0; --tiles_used)
    {
        if(!isblanktile(buf, tiles_used-1))
        {
            break;
        }
    }
    
    return tiles_used;
}

//returns the number of combos
word count_combos()
{
    word combos_used;
    
    for(combos_used=MAXCOMBOS; combos_used>0; --combos_used)
    {
        if(combobuf[combos_used-1].tile!=0)
        {
            break;
        }
    }
    
    return combos_used;
}

void setup_combo_animations()
{
    memset(animated_combo_table, 0, MAXCOMBOS*2*2);
    memset(animated_combo_table4, 0, MAXCOMBOS*2*2);
    int32_t y=0;
    
    for(word x=0; x<MAXCOMBOS; ++x)
    {
        animated_combo_table[x][0]=y;
        
        if((combobuf[x].frames>1 || combobuf[x].nextcombo != 0)&&!(combobuf[x].animflags &	AF_FRESH))
        {
            animated_combo_table4[y][0]=x;
            animated_combo_table4[y][1]=0;
			combobuf[x].aclk = 0;
			combobuf[x].cur_frame = 0;
            ++y;
        }
    }
    
    animated_combos=y;
}

void setup_combo_animations2()
{
    memset(animated_combo_table2, 0, MAXCOMBOS*2*2);
    memset(animated_combo_table24, 0, MAXCOMBOS*2*2);
    int32_t y=0;
    
    for(word x=0; x<MAXCOMBOS; ++x)
    {
        animated_combo_table2[x][0]=y;
        animated_combo_table2[x][1]=combobuf[x].tile;
        
        if((combobuf[x].frames>1 || combobuf[x].nextcombo != 0)&&combobuf[x].animflags & AF_FRESH)
        {
            animated_combo_table24[y][0]=x;
            animated_combo_table24[y][1]=0;
			combobuf[x].aclk = 0;
			combobuf[x].cur_frame = 0;
            ++y;
        }
    }
    
    animated_combos2=y;
}

void reset_combo_animation(newcombo &cmb)
{
	cmb.tile = cmb.o_tile;
	cmb.cur_frame = 0;
	cmb.aclk = 0;

	// this fn is only used in editor ...
#ifdef IS_PLAYER
	assert(false);
	// combo_caches::drawing.refresh(y);
#endif
}

void reset_combo_animation(int32_t c)
{
    for(word x=0; x<animated_combos; ++x)
    {
        int32_t y=animated_combo_table4[x][0];                      //combo number
        
        if(y==c)
        {
            combobuf[y].tile=combobuf[y].o_tile;        //reset tile
			combobuf[y].cur_frame=0;
            combobuf[y].aclk=0;                        //reset clock
#ifdef IS_PLAYER
			combo_caches::drawing.refresh(y);
#endif
            return;
        }
    }
}

void reset_combo_animations()
{
    for(word x=0; x<animated_combos; ++x)
    {
		int32_t y = animated_combo_table4[x][0];
		combobuf[y].tile = combobuf[y].o_tile;
		combobuf[y].aclk = 0;
		combobuf[y].cur_frame = 0;
#ifdef IS_PLAYER
		combo_caches::drawing.refresh(y);
#endif
    }
}

void reset_combo_animations2()
{
    for(word x=0; x<animated_combos; ++x)
    {
		int32_t y = animated_combo_table24[x][0];
		combobuf[y].tile = combobuf[y].o_tile;
		combobuf[y].aclk = 0;
		combobuf[y].cur_frame = 0;
#ifdef IS_PLAYER
		combo_caches::drawing.refresh(y);
#endif
    }
}

void reset_all_combo_animations()
{
	reset_combo_animations();
	reset_combo_animations2();
}

//Returns true if 'tile' is the LAST tile in the animation defined by the other parameters.
bool combocheck(const newcombo& cdata)
{
	if(get_qr(qr_BROKEN_ASKIP_Y_FRAMES) && !get_qr(qr_NEW_COMBO_ANIMATION))
	{
		//This is the old calculation for this, which is just wrong.
		return (cdata.tile-(cdata.frames+((cdata.frames-1)*cdata.skipanim)+(cdata.skipanimy*TILES_PER_ROW)) >=cdata.o_tile-1);
	}
	//New calculation, which actually works properly
    int32_t temp = cdata.o_tile;
    for(int32_t q = 1; q < cdata.frames; ++q)
    {
        int32_t temp2 = temp;
        temp += 1+cdata.skipanim;
        
        if((temp/TILES_PER_ROW)!=(temp2/TILES_PER_ROW))
            temp+=cdata.skipanimy*TILES_PER_ROW;
        if(cdata.tile<temp) return false;
    }
    return true;
}

void animate(newcombo& cdata, bool forceNextFrame, word cid)
{
	if(cdata.aclk>=cdata.speed || forceNextFrame)      //time to animate
	{
		if(get_qr(qr_NEW_COMBO_ANIMATION))
		{
			if(++cdata.cur_frame>=cdata.frames)
			{
				cdata.tile=cdata.o_tile;        //reset tile
				cdata.cur_frame=0;
			}
			else
			{
				cdata.tile = cdata.o_tile + ((1+cdata.skipanim)*cdata.cur_frame);
				if(int32_t rowoffset = TILEROW(cdata.tile)-TILEROW(cdata.o_tile))
				{
					cdata.tile += cdata.skipanimy * rowoffset * TILES_PER_ROW;
				}
			}
		}
		else
		{
			if(combocheck(cdata))
			{
				cdata.tile=cdata.o_tile;        //reset tile
				cdata.cur_frame=0;
			}
			else
			{
				int32_t temp=cdata.tile;
				cdata.tile+=1+(cdata.skipanim); //increment tile
				
				if(temp/TILES_PER_ROW!=cdata.tile/TILES_PER_ROW)
					cdata.tile+=TILES_PER_ROW*cdata.skipanimy;
			}
		}
		cdata.aclk=0;                        //reset clock

#ifdef IS_PLAYER
		combo_caches::drawing.refresh(cid);
#endif
	}
	else
	{
		if(get_qr(qr_NEW_COMBO_ANIMATION))
		{
			cdata.tile = cdata.o_tile + ((1+cdata.skipanim)*cdata.cur_frame);
			if(int32_t rowoffset = TILEROW(cdata.tile)-TILEROW(cdata.o_tile))
			{
				cdata.tile += cdata.skipanimy * rowoffset * TILES_PER_ROW;
			}
#ifdef IS_PLAYER
			combo_caches::drawing.refresh(cid);
#endif
		}
		++cdata.aclk;                        //increment clock
	}
}

void animate_combos()
{
    update_combo_cycling();
    
    for(word x=0; x<animated_combos; ++x)
    {
        int32_t y=animated_combo_table4[x][0];                      //combo number
        
		animate(combobuf[y], false, y);
    }
    
    for(word x=0; x<animated_combos2; ++x)
    {
        int32_t y=animated_combo_table24[x][0];                      //combo number
        
        animate(combobuf[y], false, y);
    }
}

/*
bool isonline(float x1, float y1, float x2, float y2, float x3, float y3)
{
  float slope;
  float intercept;

  slope = (y2-y1)/(x2-x1);
  intercept = y1 - (slope*x1);
  return (y3 == (slope*x3)+intercept) && x3>zc_min(x1,x2) && x3<zc_max(x1,x2) && y3>zc_min(y1,y2) && y3<zc_max(y1,y2) ;
}
*/

//pixel-precise version of the above -DD
bool isonline(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3)
{
    int64_t dx = x2-x1;
    int64_t dy = y2-y1;
    
    int64_t qx = x3-x1;
    int64_t qy = y3-y1;
    
    //if (x3,y3) is on the line, qx,qy must be multiples of dx,dy. Check this without division.
    if(qx*dy != qy*dx)
        return false;
        
    //check for degeneracy
    if(dx == 0 && dy == 0)
        return qx == 0 && qy == 0;
        
    int64_t nondegend;
    int64_t nondegenq;
    
    if(dx == 0)
    {
        nondegend = dy;
        nondegenq = qy;
    }
    else
    {
        nondegend = dx;
        nondegenq = qx;
    }
    
    //flip negatives
    if(nondegend < 0)
    {
        nondegend = -nondegend;
        nondegenq = -nondegenq;
    }
    
    //and compare
    return nondegenq >= 0 && nondegenq <= nondegend;
}

void reset_tile(tiledata *buf, int32_t t, int32_t format=1)
{
    buf[t].format=format;
    
    if(buf[t].data!=NULL)
    {
        free(buf[t].data);
    }
    
    buf[t].data=(byte *)calloc(tilesize(buf[t].format), 1);
    
    if(buf[t].data==NULL)
    {
        quit_game();
        Z_error_fatal("Unable to initialize tile #%d.\n", t);
    }
}

//clears the tile buffer
void clear_tiles(tiledata *buf)
{
    for(int32_t i=0; i<NEWMAXTILES; ++i)
    {
        reset_tile(buf,i,tf4Bit);
    }
}

void overlay_tile(tiledata *buf,int32_t dest,int32_t src,int32_t cs,bool backwards)
{
	// TODO: should mark "Overlay" option invalid if no tile is copied.
	if (dest <= 0 || src <= 0)
		return;

    byte upbuf[256];
    
    unpack_tile(buf, dest, 0, false);
    
    for(int32_t i=0; i<256; i++)
        upbuf[i] = unpackbuf[i];
        
    unpack_tile(buf, src, 0, false);
    
    if(buf[src].format>tf4Bit)
    {
        cs=0;
    }
    
    cs &= 15;
    cs <<= CSET_SHFT;
    
    for(int32_t i=0; i<256; i++)
    {
        if(backwards)
        {
            if(!upbuf[i])
            {
                upbuf[i] = unpackbuf[i]+cs;
            }
        }
        else
        {
            if(unpackbuf[i])
            {
                upbuf[i] = unpackbuf[i]+cs;
            }
        }
    }
    
    if(!blank_tile_table[src])
    {
        blank_tile_table[dest]=false;
    }
    
    pack_tile(buf, upbuf,dest);
}

bool copy_tile(tiledata *buf, int32_t src, int32_t dest, bool swap)
{
    if(src==dest)
    {
        return true;
    }
    
    int32_t tempformat=buf[dest].format;
    byte *temptiledata=(byte *)malloc(tilesize(tempformat));
    int32_t tsize = tilesize(tempformat);
    
    if(swap)
    {
        for(int32_t j=0; j<tsize; j++)
        {
            temptiledata[j]=buf[dest].data[j];
        }
    }
    
    reset_tile(buf, dest, buf[src].format);
    
    for(int32_t j=0; j<tilesize(buf[src].format); j++)
    {
        buf[dest].data[j]=buf[src].data[j];
    }
    
    if(swap)
    {
        reset_tile(buf, src, tempformat);
        
        for(int32_t j=0; j<tsize; j++)
        {
            buf[src].data[j]=temptiledata[j];
        }
    }
	int32_t t = blank_tile_table[dest];
	blank_tile_table[dest] = blank_tile_table[src];
	if(swap) blank_tile_table[src] = t;
    
    free(temptiledata);
    
    return true;
}

bool write_tile(tiledata *buf, BITMAP* src, int32_t dest, int32_t x, int32_t y, bool is8bit, bool overlay)
{
	unpack_tile(buf, dest, 0, false);
	
	bool blank = !is8bit;
	int32_t bitmod = (is8bit ? 256 : 16);
	for(int32_t tx = 0; tx < 16; ++tx)
		for(int32_t ty = 0; ty < 16; ++ty)
		{
			int32_t i = (tx+(16*ty));
			int32_t v = getpixel(src, x+tx, y+ty)%bitmod;
			if(v%bitmod)
			{
				blank = false;
				unpackbuf[i] = v;
			}
			else if(!overlay)
			{
				unpackbuf[i] = v;
			}
		}
	
	if(overlay)
	{
		if(!blank)
			blank_tile_table[dest]=false;
	}
	else blank_tile_table[dest]=blank;
	reset_tile(buf, dest, is8bit ? tf8Bit : tf4Bit);
	pack_tile(buf, unpackbuf, dest);
    return true;
}

// For the editor only, grabbing code mades weird assumptions where it deletes the first tile
// just before drawing it, which relies on it being cached (see top of unpack_tile).
// See draw_grab_scr. Crash would happen from calling `puttile16` after `newtilebuf[0].data=NULL;`.
// No time to work this out just yet, so avoid the fast path in this case.
// This is also need for the bottom two "previous data" tiles to render correctly in grab menu.
bool zq_allow_tile_draw_cache = false;

static const byte* get_tile_bytes(int32_t tile, int32_t flip)
{
#if IS_EDITOR
	if (flip == 0 && !zq_allow_tile_draw_cache)
#else
    if (flip == 0)
#endif
	{
		assert(newtilebuf[tile].data);
        return newtilebuf[tile].data;
	}

    unpack_tile(newtilebuf, tile, flip, false);
    return unpackbuf;
}

// unpacks from tilebuf to unpackbuf
void unpack_tile(tiledata *buf, int32_t tile, int32_t flip, bool force)
{
    static byte *si, *di;
    static byte *oldnewtilebuf=buf[tile].data;
    static int32_t i, j, oldtile=-5, oldflip=-5;
    
    // This is only still here because of a crash during grabbing. See get_tile_bytes.
    // Disabled for player because it causes a flaky failure for:
    //  python tests/run_replay_tests.py --filter freedom_in_chains --frame 143580
    // ...on mac only.
#if IS_EDITOR
    if(tile==oldtile&&(flip&5)==(oldflip&5)&&oldnewtilebuf==buf[tile].data&&!force)
    {
        return;
    }
#endif
    
    oldtile=tile;
    oldflip=flip;
    oldnewtilebuf=buf[tile].data;
    
    switch(flip&5)
    {
    case 1:  //horizontal
        si = buf[tile].data+tilesize(buf[tile].format);
        
        for(i=15; i>=0; --i)
        {
            switch(buf[tile].format)
            {
            case tf4Bit:
            case tf8Bit:
                di=unpackbuf + (i<<4) - 1;
                
                for(j=1; j>=0; --j)
                {
                    (*(++di)) = (*(--si));
                    (*(++di)) = (*(--si));
                    (*(++di)) = (*(--si));
                    (*(++di)) = (*(--si));
                    (*(++di)) = (*(--si));
                    (*(++di)) = (*(--si));
                    (*(++di)) = (*(--si));
                    (*(++di)) = (*(--si));
                }
                
                break;
            }
        }
        
        break;
        
    case 4:  //rotated
        si = buf[tile].data+tilesize(buf[tile].format);
        
        for(i=15; i>=0; --i)
        {
            switch(buf[tile].format)
            {
            case tf4Bit:
            case tf8Bit:
                di=unpackbuf + 271 - i; //256 + 15 - i
                
                for(j=1; j>=0; --j)
                {
                    di-=16;
                    *di = (*(--si));
                    di-=16;
                    *di = (*(--si));
                    di-=16;
                    *di = (*(--si));
                    di-=16;
                    *di = (*(--si));
                    di-=16;
                    *di = (*(--si));
                    di-=16;
                    *di = (*(--si));
                    di-=16;
                    *di = (*(--si));
                    di-=16;
                    *di = (*(--si));
                }
                
                break;
            }
        }
        
        break;
        
    case 5:  //rotated and horizontal
        si = buf[tile].data+tilesize(buf[tile].format);
        
        for(i=15; i>=0; --i)
        {
            switch(buf[tile].format)
            {
            case tf4Bit:
            case tf8Bit:
                di=unpackbuf + 256 + i;
                
                for(j=1; j>=0; --j)
                {
                    di-=16;
                    *di = (*(--si));
                    di-=16;
                    *di = (*(--si));
                    di-=16;
                    *di = (*(--si));
                    di-=16;
                    *di = (*(--si));
                    di-=16;
                    *di = (*(--si));
                    di-=16;
                    *di = (*(--si));
                    di-=16;
                    *di = (*(--si));
                    di-=16;
                    *di = (*(--si));
                }
                
                break;
            }
        }
        
        break;
        
    default: //none or invalid
        switch(buf[tile].format)
        {
        case tf4Bit:
        case tf8Bit:
            si = buf[tile].data+tilesize(buf[tile].format);
            di = unpackbuf + 256;
            
            for(i=31; i>=0; --i)
            {
                (*(--di)) = (*(--si));
                (*(--di)) = (*(--si));
                (*(--di)) = (*(--si));
                (*(--di)) = (*(--si));
                (*(--di)) = (*(--si));
                (*(--di)) = (*(--si));
                (*(--di)) = (*(--si));
                (*(--di)) = (*(--si));
            }
            
            break;
        }
        
        break;
    }
}

// packs from src[256] to tilebuf
void pack_tile(tiledata *buf, byte *src,int32_t tile)
{
    pack_tiledata(buf[tile].data, src, buf[tile].format);
}

void pack_tiledata(byte *dest, byte *src, byte format)
{
    byte *di = dest;
    
    switch(format)
    {
	case tf4Bit:
        for(int32_t si=0; si<256; si++)
        {
            *di = src[si]&15;
            ++di;
        }
        break;

    case tf8Bit:
        // TODO: Could this just be a memcpy ...
        for(int32_t si=0; si<32; si++)
        {
            *di = src[si*8];
            ++di;
            *di = src[si*8+1];
            ++di;
            *di = src[si*8+2];
            ++di;
            *di = src[si*8+3];
            ++di;
            *di = src[si*8+4];
            ++di;
            *di = src[si*8+5];
            ++di;
            *di = src[si*8+6];
            ++di;
            *di = src[si*8+7];
            ++di;
        }
        
        break;
    }
}

void load_tile(byte* buf, int tile)
{
	unpack_tile(newtilebuf, tile, 0, false);
	memcpy(buf, unpackbuf, 256);
}
void load_minitile(byte* buf, int tile, int mini)
{
	unpack_tile(newtilebuf, tile, 0, false);
	int xo = (mini&1)?8:0;
	int yo = (mini&2)?8:0;
	for(int x = 0; x < 8; ++x)
		for(int y = 0; y < 8; ++y)
		{
			buf[x+(y*8)] = unpackbuf[(x+xo)+((y+yo)*16)];
		}
}

int32_t rotate_table[8]=
{
    4, 6, 5, 7, 3, 1, 2, 0
};

int32_t rotate_value(int32_t flip)
{
    return rotate_table[flip&7];
}
byte rotate_cset(byte v)
{
	byte ret = v&0xF;
	ret |= (v & 0x10)<<1;
	ret |= (v & 0x20)<<2;
	ret |= (v & 0x40)>>2;
	ret |= (v & 0x80)>>1;
	return ret;
}
byte rotate_walk(byte v)
{
	byte ret = 0;
	ret |= (v & 0x11)<<2;
	ret |= (v & 0x22)>>1;
	ret |= (v & 0x44)<<1;
	ret |= (v & 0x88)>>2;
	return ret;
}


// A (slow) function to handle any tile8 draw.
static void draw_tile8_unified(BITMAP* dest, byte *si, int32_t x, int32_t y, int32_t cset, int32_t flip, bool transparency)
{
    for (int32_t dy = 0; dy < 8; ++dy)
    {
        for (int32_t dx = 0; dx < 8; ++dx)
        {
            int destx = x + (flip&1 ? 7 - dx : dx);
            int desty = y + (flip&2 ? 7 - dy : dy);
            if (destx >= 0 && desty >= 0 && destx < dest->w && desty < dest->h)
            {
                if (!transparency || *si) dest->line[desty][destx] = *si + cset;
            }
            si++;
        }
        si += 8;
    }
}

static void draw_tile8_transparent_fast(BITMAP* dest, const byte* __restrict si, int32_t x, int32_t y, int32_t cset, int32_t flip)
{
	bool h_flip = flip & 1;
	bool v_flip = flip & 2;

	// Prepare constants
	// cset is added to every pixel. Broadcast it to all bytes in a register.
	__m128i v_cset = _mm_set1_epi8((char)cset);
	__m128i v_zero = _mm_setzero_si128();
	
	// Mask for horizontal flip (SSSE3 _mm_shuffle_epi8)
	// Reverses the first 8 bytes: 7, 6, 5, 4, 3, 2, 1, 0, and don't care about the top 8
	__m128i v_hflip_mask = _mm_set_epi8(
		15, 14, 13, 12, 11, 10, 9, 8, // Upper 64-bits (unused here)
		0, 1, 2, 3, 4, 5, 6, 7        // Lower 64-bits (Reverses 0-7)
	);

	// Vertical flip.
	int start_dy = 0, end_dy = 8, step_dy = 1;
	if (v_flip)
	{
		start_dy = 7;
		end_dy = -1;
		step_dy = -1;
	}

	const byte* __restrict src_row = si;

	for (int dy = start_dy; dy != end_dy; dy += step_dy)
	{
		// Load 8 pixels (64 bits) from source
		// We use loadl_epi64 to load 8 bytes into the lower half of the XMM register
		// Because we are reading bytes, alignment isn't usually guaranteed, so we use loadu equivalent implied by loadl.
		__m128i src_pixels = _mm_loadl_epi64((const __m128i*)src_row);

		// Horizontal flip.
		if (h_flip)
			src_pixels = _mm_shuffle_epi8(src_pixels, v_hflip_mask);

		// Check Transparency (Mask generation)
		// _mm_cmpeq_epi8 returns 0xFF where equal, 0x00 where not.
		// mask is 0xFF where source is Transparent (0), 0x00 where Opaque.
		__m128i v_trans_mask = _mm_cmpeq_epi8(src_pixels, v_zero);

		// Add cset offset to source pixels
		// paddb wraps around, which matches the (byte)(*si + cset) behavior
		src_pixels = _mm_add_epi8(src_pixels, v_cset);

		// Load Destination pixels
		byte* dest_addr = dest->line[y + dy] + x;
		__m128i dest_pixels = _mm_loadl_epi64((const __m128i*)dest_addr);

		// Blend: (Dest & Mask) | (Source & ~Mask)
		// If mask is FF (Transparent), we keep Dest.
		// If mask is 00 (Opaque), we keep Source.
		
		// dest & mask
		__m128i masked_dest = _mm_and_si128(dest_pixels, v_trans_mask);
		// source & ~mask
		__m128i masked_src  = _mm_andnot_si128(v_trans_mask, src_pixels);
		
		__m128i result = _mm_or_si128(masked_dest, masked_src);

		// Store result back to memory (lower 64 bits only).
		_mm_storel_epi64((__m128i*)dest_addr, result);

		src_row += 16;
	}
}

// A (slow) function to handle any transparent tile8 draw.
static void draw_tile8_unified(BITMAP* dest, int cl, int ct, int cr, int cb, const byte* __restrict si, int32_t x, int32_t y, int32_t cset, int32_t flip)
{
	bool h_flip = flip & 1;
	bool v_flip = flip & 2;

	// Calculate the intersection of the tile and the clip rect.
	int start_x = std::max(x, cl);
	int end_x   = std::min(x + 8, cr);
	int start_y = std::max(y, ct);
	int end_y   = std::min(y + 8, cb);

	// Calculate loop offsets relative to the tile (0..7).
	int offset_x = start_x - x;
	int offset_y = start_y - y;
	int draw_w   = end_x - start_x;
	int draw_h   = end_y - start_y;

	// Configure strides based on flipping.
	const int src_pitch_bytes = 16; 
	const byte* __restrict src_ptr = si;

	// Vertical flip.
	int dy_step;
	if (v_flip)
	{
		// Start at bottom row of the tile relative to current Y
		// If we are clipped at the top (offset_y > 0), we skip the *bottom* rows of the source
		src_ptr += (7 - offset_y) * src_pitch_bytes;
		dy_step = -src_pitch_bytes;
	}
	else
	{
		// Normal
		src_ptr += offset_y * src_pitch_bytes;
		dy_step = src_pitch_bytes;
	}

	// Horizontal flip.
	int dx_step;
	if (h_flip)
	{
		// Start at rightmost pixel
		// If clipped on left (offset_x > 0), we step back from (7 - offset_x)
		src_ptr += (7 - offset_x); 
		dx_step = -1;
	}
	else
	{
		src_ptr += offset_x;
		dx_step = 1;
	}

	// Draw.
	for (int dy = 0; dy < draw_h; ++dy)
	{
		byte* __restrict d_ptr = dest->line[start_y + dy] + start_x;
		const byte* __restrict s_row_ptr = src_ptr;

		for (int dx = 0; dx < draw_w; ++dx)
		{
			byte val = *s_row_ptr;
			if (val)
				*d_ptr = val + cset;
			
			d_ptr++;
			s_row_ptr += dx_step;
		}

		src_ptr += dy_step;
	}
}

// Fast drawing for 16x16 tiles, given:
// - Tile is fully on screen (no clipping needed)
// - No transparency
static void draw_tile16_opaque_fast(BITMAP* dest, const byte* __restrict si, int32_t x, int32_t y, int32_t cset, int32_t flip)
{
	__m128i cset_vec = _mm_set1_epi8((char)cset);

	const uint8_t* __restrict src_ptr = si;
	bool v_flip = (flip & 2);

	if (v_flip)
	{
		for (int dy = 0; dy < 16; ++dy)
		{
			uint8_t* __restrict d_ptr = dest->line[y + (15 - dy)] + x;

			__m128i pixels = _mm_loadu_si128((const __m128i*)src_ptr);
			pixels = _mm_add_epi8(pixels, cset_vec);
			_mm_storeu_si128((__m128i*)d_ptr, pixels);

			src_ptr += 16;
		}
	}
	else
	{
		for (int dy = 0; dy < 16; ++dy)
		{
			uint8_t* __restrict d_ptr = dest->line[y + dy] + x;

			// Load 16 bytes from Source (unaligned)
			__m128i pixels = _mm_loadu_si128((const __m128i*)src_ptr);

			// Add CSet to all 16 bytes
			pixels = _mm_add_epi8(pixels, cset_vec);

			// Store 16 bytes to destination (unaligned)
			// Assembly: movdqu
			_mm_storeu_si128((__m128i*)d_ptr, pixels);

			src_ptr += 16;
		}
	}
}

static void draw_tile16_unified(BITMAP* dest, int cl, int ct, int cr, int cb, const byte* __restrict si, int32_t x, int32_t y, int32_t cset, int32_t flip, bool transparency)
{
	int start_x = 0;
	int end_x = 16;

	// Clip x.
	if (x < cl) start_x = cl - x;
	if (x + 16 > cr) end_x = cr - x;
	int visible_width = end_x - start_x;
	if (visible_width <= 0) return;

	// Instead of complex source-Y clipping, we find the
	// visible destination Y range.
	int clip_top    = std::max(ct, 0);
	int clip_bottom = std::min(cb, dest->h);
	int visible_y_start = std::max(clip_top, y);
	int visible_y_end   = std::min(clip_bottom, y + 16);
	if (visible_y_start >= visible_y_end) return;

	bool v_flip = (flip & 2);

	// Loop from the first visible destination line to the last
	for (int dest_y = visible_y_start; dest_y < visible_y_end; ++dest_y)
	{
		// Now, calculate the correct source row (dy) for this dest_y
		int dy;
		if (v_flip)
			dy = (y + 15) - dest_y; // Flipped mapping
		else
			dy = dest_y - y;        // Normal mapping

		// Set up pointers for this row
		const byte* __restrict src_ptr = si + (dy * 16) + start_x;

		// This access is now safe because:
		// dest_y >= visible_y_start >= clip_top >= 0
		// dest_y <  visible_y_end   <= clip_bottom <= dest->h
		uint8_t* __restrict d_ptr = dest->line[dest_y] + x + start_x;

		if (transparency)
		{
			for (int i = 0; i < visible_width; ++i)
			{
				uint8_t pixel = *src_ptr++;
				if (pixel)
					*d_ptr = pixel + cset;
				d_ptr++;
			}
		}
		else
		{
			for (int i = 0; i < visible_width; ++i)
				*d_ptr++ = (*src_ptr++) + cset;
		}
	}
}

static void draw_tile16_unified_translucent(BITMAP* dest, int cl, int ct, int cr, int cb, const byte *si, int32_t x, int32_t y, int32_t cset, int32_t flip, bool transparency)
{
    for (int32_t dy = 0; dy < 16; ++dy)
    {
        for (int32_t dx = 0; dx < 16; ++dx)
        {
            int destx = x + dx;
            int desty = y + (flip&2 ? 15 - dy : dy);
            if (destx >= cl && desty >= ct && destx < cr && desty < cb)
            {
                if (!transparency || *si)
					dest->line[desty][destx] = trans_table->data[dest->line[desty][destx]][*si + cset];
            }
            si++;
        }
    }
}


void puttiletranslucent8(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip,int32_t opacity)
{
    //these are here to bypass compiler warnings about unused arguments
    opacity=opacity;
    
    if(x<-7 || y<-7)
        return;
        
    if(y > dest->h)
        return;
        
    if(y == dest->h && x > dest->w)
        return;
        
    if(newtilebuf[tile>>2].format>tf4Bit)
    {
        cset=0;
    }
    
    cset &= 15;
    cset <<= CSET_SHFT;
    const byte* bytes = get_tile_bytes(tile>>2, 0);
    const byte *si = bytes + ((tile&2)<<6) + ((tile&1)<<3);
    
    if(flip&1)  //horizontal
    {
        si+=7;
    }
    
    if((flip&2)==0)                                           //not flipped vertically
    {
        if(y<0)
        {
            si+=(0-y)<<4;
        }
        
        for(int32_t dy=(y<0 ? 0-y : 0); (dy<8)&&(dy+y<dest->h); ++dy)
        {
            byte* di = &(dest->line[y+dy][x<0 ? 0 : x]);
            
            for(int32_t i=0; i<8; ++i)
            {
                if(x+i<dest->w)
                {
                    //            *(di) = (opacity==255)?((*si) + cset):trans_table->data[(*di)][((*si) + cset)];
                    *(di) = trans_table->data[(*di)][((*si) + cset)];
                    ++di;
                }
                
                flip&1 ? --si : ++si;
            }
            
            if(flip&1)
            {
                si+=24;
            }
            else
            {
                si+=8;
            }
        }
    }                                                         //flipped vertically
    else
    {
        if(y+7>=dest->h)
        {
            si+=(8+y-dest->h)<<4;
        }
        
        for(int32_t dy=(y+7>=dest->h ? dest->h-y-1 : 7); (dy>=0)&&(dy+y>=0); --dy)
        {
            byte* di = &(dest->line[y+dy][x<0 ? 0 : x]);
            
            for(int32_t i=0; i<8; ++i)
            {
                if(x+i<dest->w)
                {
                    //          *(di) = (opacity==255)?((*si) + cset):trans_table->data[(*di)][((*si) + cset)];
                    *(di) = trans_table->data[(*di)][((*si) + cset)];
                    ++di;
                }
                
                flip&1 ? --si : ++si;
            }
            
            if(flip&1)
            {
                si+=24;
            }
            else
            {
                si+=8;
            }
        }
    }
}

void overtiletranslucent8(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip,int32_t opacity)
{
    //these are here to bypass compiler warnings about unused arguments
    opacity=opacity;
    
    if(x<-7 || y<-7)
        return;
        
    if(y > dest->h)
        return;
        
    if(y == dest->h && x > dest->w)
        return;
        
    if(blank_tile_quarters_table[tile])
    {
        return;
    }
    
    if(newtilebuf[tile>>2].format>tf4Bit)
    {
        cset=0;
    }
    
    cset &= 15;
    cset <<= CSET_SHFT;

    const byte* bytes = get_tile_bytes(tile>>2, 0);
    const byte *si = bytes + ((tile&2)<<6) + ((tile&1)<<3);
    
    if(flip&1)
    {
        si+=7;
    }
    
    if((flip&2)==0)                                           //not flipped vertically
    {
        if(y<0)
        {
            si+=(0-y)<<4;
        }
        
        for(int32_t dy=(y<0 ? 0-y : 0); (dy<8)&&(dy+y<dest->h); ++dy)
        {
            byte* di = &(dest->line[y+dy][x<0 ? 0 : x]);
            
            for(int32_t i=0; i<8; ++i)
            {
                if(x+i<dest->w)
                {
                    if(*si)
                    {
                        //            *(di) = (opacity==255)?((*si) + cset):trans_table->data[(*di)][((*si) + cset)];
                        *(di) = trans_table->data[(*di)][((*si) + cset)];
                    }
                    
                    ++di;
                }
                
                flip&1 ? --si : ++si;
            }
            
            if(flip&1)
            {
                si+=24;
            }
            else
            {
                si+=8;
            }
        }
    }                                                         //flipped vertically
    else
    {
        if(y+7>=dest->h)
        {
            si+=(8+y-dest->h)<<4;
        }
        
        for(int32_t dy=(y+7>=dest->h ? dest->h-y-1 : 7); (dy>=0)&&(dy+y>=0); --dy)
        {
            byte* di = &(dest->line[y+dy][x<0 ? 0 : x]);
            
            for(int32_t i=0; i<8; ++i)
            {
                if(x+i<dest->w)
                {
                    if(*si)
                    {
                        //            *(di) = (opacity==255)?((*si) + cset):trans_table->data[(*di)][((*si) + cset)];
                        *(di) = trans_table->data[(*di)][((*si) + cset)];
                    }
                    
                    ++di;
                }
                
                flip&1 ? --si : ++si;
            }
            
            if(flip&1)
            {
                si+=24;
            }
            else
            {
                si+=8;
            }
        }
    }
}

void puttiletranslucent16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip,int32_t opacity)
{
    //these are here to bypass compiler warnings about unused arguments
    opacity=opacity;
    
    if(x<-15 || y<-15)
        return;
        
    if(y > dest->h)
        return;
        
    if(y == dest->h && x > dest->w)
        return;
        
    if(tile<0 || tile>=NEWMAXTILES)
    {
        rectfill(dest,x,y,x+15,y+15,0);
        return;
    }
    
    if(newtilebuf[tile].format>tf4Bit)
    {
        cset=0;
    }
    
    cset &= 15;
    cset <<= CSET_SHFT;

    const byte* si = get_tile_bytes(tile, flip&5);
    byte *di;
    
    if(flip&1)
        si+=15;
        
    if((flip&2)==0)
    {
        if(y<0)
            si+=(0-y)<<4;
            
        for(int32_t dy=(y<0 ? 0-y : 0); (dy<16)&&(dy+y<dest->h); ++dy)
        {
            di = &(dest->line[y+dy][x<0 ? 0 : x]);
            
            if(x+15<dest->w)
            {
                if(x<0)
                    flip&1 ? si-=0-x : si+=0-x;
                    
                for(int32_t dx=(x<0 ? 0-x : 0); dx<16; ++dx)
                {
                    //          *di=(opacity==255)?((*si) + cset):trans_table->data[(*di)][((*si) + cset)];
                    *di=trans_table->data[(*di)][((*si) + cset)];
                    ++di;
                    flip&1 ? --si : ++si;
                }
            }
            else
            {
                for(int32_t i=0; i<16; ++i)
                {
                    if(x+i<dest->w)
                    {
                        //            *di=(opacity==255)?((*si) + cset):trans_table->data[(*di)][((*si) + cset)];
                        *di=trans_table->data[(*di)][((*si) + cset)];
                        ++di;
                    }
                    
                    flip&1 ? --si : ++si;
                }
            }
            
            if(flip&1)
                si+=32;
        }
    }
    else
    {
        if(y+15>=dest->h)
            si+=(16+y-dest->h)<<4;
            
        for(int32_t dy=(y+15>=dest->h ? dest->h-y-1 : 15); (dy>=0)&&(dy+y>=0); --dy)
        {
            di = &(dest->line[y+dy][x<0 ? 0 : x]);
            
            if(x+15<dest->w)
            {
                if(x<0)
                    flip&1 ? si-=0-x : si+=0-x;
                    
                for(int32_t dx=(x<0 ? 0-x : 0); dx<16; ++dx)
                {
                    //          *di=(opacity==255)?((*si) + cset):trans_table->data[(*di)][((*si) + cset)];
                    *di=trans_table->data[(*di)][((*si) + cset)];
                    ++di;
                    flip&1 ? --si : ++si;
                }
            }
            else
            {
                for(int32_t i=0; i<16; ++i)
                {
                    if(x+i<dest->w)
                    {
                        //            *di=(opacity==255)?((*si) + cset):trans_table->data[(*di)][((*si) + cset)];
                        *di=trans_table->data[(*di)][((*si) + cset)];
                        ++di;
                    }
                    
                    flip&1 ? --si : ++si;
                }
            }
            
            if(flip&1)
                si+=32;
        }
    }
}

void overtiletranslucent16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip,int32_t opacity)
{
    //these are here to bypass compiler warnings about unused arguments
    opacity=opacity;

	int cl = 0;
	int ct = 0;
	int cr = dest->w;
	int cb = dest->h;
	if (dest->clip)
	{
		cl = dest->cl;
		ct = dest->ct;
		cr = dest->cr;
		cb = dest->cb;
	}

	if (x + 16 < cl)
		return;
	if (x > cr)
		return;
	if (y + 16 < ct)
		return;
	if (y > cb)
		return;

    if(tile<0 || tile>=NEWMAXTILES)
    {
        rectfill(dest,x,y,x+15,y+15,0);
        return;
    }
    
    if(blank_tile_table[tile])
    {
        return;
    }
    
    if(newtilebuf[tile].format>tf4Bit)
    {
        cset=0;
    }
    
    cset &= 15;
    cset <<= CSET_SHFT;

    const byte* si = get_tile_bytes(tile, flip&5);

	// 0: fast, no bounds checking
    // 1: slow, bounds checking
    int draw_mode = x < cl || y < ct || x >= cr-16 || y >= cb-16 || x%8 || y%8 ? 1 : 0;
    if (draw_mode == 1)
    {
        draw_tile16_unified_translucent(dest, cl, ct, cr, cb, si, x, y, cset, flip, true);
        return;
    }

    byte *di;
    
    if((flip&2)==0)
    {
        if(y<0)
            si+=(0-y)<<4;
            
        for(int32_t dy=(y<0 ? 0-y : 0); (dy<16)&&(dy+y<dest->h); ++dy)
        {
            di = &(dest->line[y+dy][x<0 ? 0 : x]);
            
            if(x+15<dest->w)
            {
                if(x<0)
                    si+=0-x;
                    
                for(int32_t dx=(x<0 ? 0-x : 0); dx<16; ++dx)
                {
                    if(*si)
                    {
                        //            *di=*si+cset;
                        //            *di=(opacity==255)?((*si) + cset):trans_table->data[(*di)][((*si) + cset)];
                        *di=trans_table->data[(*di)][((*si) + cset)];
                    }
                    
                    ++di;
                    ++si;
                }
            }
            else
            {
                for(int32_t i=0; i<16; ++i)
                {
                    if(x+i<dest->w)
                    {
                        if(*si)
                        {
                            //              *di=*si+cset;
                            //              *di=(opacity==255)?((*si) + cset):trans_table->data[(*di)][((*si) + cset)];
                            *di=trans_table->data[(*di)][((*si) + cset)];
                        }
                        
                        ++di;
                    }
                    
                    ++si;
                }
            }
        }
    }
    else
    {
        if(y+15>=dest->h)
            si+=(16+y-dest->h)<<4;
            
        for(int32_t dy=(y+15>=dest->h ? dest->h-y-1 : 15); (dy>=0)&&(dy+y>=0); --dy)
        {
            di = &(dest->line[y+dy][x<0 ? 0 : x]);
            
            if(x+15<dest->w)
            {
                if(x<0)
                    si+=0-x;
                    
                for(int32_t dx=(x<0 ? 0-x : 0); dx<16; ++dx)
                {
                    if(*si)
                    {
                        //            *di=*si+cset;
                        //            *di=(opacity==255)?((*si) + cset):trans_table->data[(*di)][((*si) + cset)];
                        *di=trans_table->data[(*di)][((*si) + cset)];
                    }
                    
                    ++di;
                    ++si;
                }
            }
            else
            {
                for(int32_t i=0; i<16; ++i)
                {
                    if(x+i<dest->w)
                    {
                        if(*si)
                        {
                            //              *di=*si+cset;
                            //              *di=(opacity==255)?((*si) + cset):trans_table->data[(*di)][((*si) + cset)];
                            *di=trans_table->data[(*di)][((*si) + cset)];
                        }
                        
                        ++di;
                    }
                    
                    ++si;
                }
            }
        }
    }
}

void overtilecloaked16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t flip)
{
    if(x<-15 || y<-15)
        return;
        
    if(y > dest->h)
        return;
        
    if(y == dest->h && x > dest->w)
        return;
        
    if(tile<0 || tile>=NEWMAXTILES)
    {
        rectfill(dest,x,y,x+15,y+15,0);
        return;
    }
    
    const byte* si = get_tile_bytes(tile, 0);
    byte *di;
    
    if(flip&1)
        si+=15;
        
    if((flip&2)==0)
    {
        if(y<0)
            si+=(0-y)<<4;
            
        for(int32_t dy=(y<0 ? 0-y : 0); (dy<16)&&(dy+y<dest->h); ++dy)
        {
            di = &(dest->line[y+dy][x<0 ? 0 : x]);
            
            if(x+15<dest->w)
            {
                if(x<0)
                    flip&1 ? si-=0-x : si+=0-x;
                    
                for(int32_t dx=(x<0 ? 0-x : 0); dx<16; ++dx)
                {
                    if(*si)
                    {
                        *di=dest->line[((y+dy)^1)][((x+dx)^1)];
                    }
                    
                    ++di;
                    flip&1 ? --si : ++si;
                }
            }
            else
            {
                for(int32_t i=0; i<16; ++i)
                {
                    if(x+i<dest->w)
                    {
                        if(*si)
                        {
                            *di=dest->line[((y+dy)^1)][(x^1)];
                        }
                        
                        ++di;
                    }
                    
                    flip&1 ? --si : ++si;
                }
            }
            
            if(flip&1)
                si+=32;
        }
    }
    else
    {
        if(y+15>=dest->h)
            si+=(16+y-dest->h)<<4;
            
        for(int32_t dy=(y+15>=dest->h ? dest->h-y-1 : 15); (dy>=0)&&(dy+y>=0); --dy)
        {
            di = &(dest->line[y+dy][x<0 ? 0 : x]);
            
            if(x+15<dest->w)
            {
                if(x<0)
                    flip&1 ? si-=0-x : si+=0-x;
                    
                for(int32_t dx=(x<0 ? 0-x : 0); dx<16; ++dx)
                {
                    if(*si)
                    {
                        *di=dest->line[((y+dy)^1)][((x+dx)^1)];
                    }
                    
                    ++di;
                    flip&1 ? --si : ++si;
                }
            }
            else
            {
                for(int32_t i=0; i<16; ++i)
                {
                    if(x+i<dest->w)
                    {
                        if(*si)
                        {
                            *di=dest->line[((y+dy)^1)][(x^1)];
                        }
                        
                        ++di;
                    }
                    
                    flip&1 ? --si : ++si;
                }
            }
            
            if(flip&1)
                si+=32;
        }
    }
}

void draw_cloaked_sprite(BITMAP* dest,BITMAP* src,int32_t x,int32_t y)
{
	int32_t w = src->w, h = src->h;
    if(x+w<0 || y+h<0)
        return;
        
    if(y > dest->h)
        return;
        
    if(y == dest->h && x > dest->w)
        return;
    
	int32_t sx = 0, sy = 0;
    byte *di;
    
	if(y<0)
		sy+=(0-y);
		
	for(int32_t dy=(y<0 ? 0-y : 0); (dy<h)&&(dy+y<dest->h); ++dy)
	{
		di = &(dest->line[y+dy][x<0 ? 0 : x]);
		
		if(x+w-1<dest->w)
		{
			if(x<0)
				sx+=0-x;
				
			for(int32_t dx=(x<0 ? 0-x : 0); dx<w; ++dx)
			{
				if(src->line[sy][sx])
				{
					*di=dest->line[((y+dy)^1)][((x+dx)^1)];
				}
				
				++di;
				++sx;
				if(sx >= w)
				{
					++sy;
					sx = 0;
				}
			}
		}
		else
		{
			for(int32_t i=0; i<16; ++i)
			{
				if(x+i<dest->w)
				{
					if(src->line[sy][sx])
					{
						*di=dest->line[((y+dy)^1)][(x^1)];
					}
					
					++di;
				}
				
				++sx;
				if(sx >= w)
				{
					++sy;
					sx = 0;
				}
			}
		}
	}
}

void putblocktranslucent8(BITMAP *dest,int32_t tile,int32_t x,int32_t y,int32_t csets[],int32_t flip,int32_t mask,int32_t opacity)
{
    int32_t t[4];
    
    for(int32_t i=0; i<4; ++i)
        t[i]=tile+i;
        
    switch(mask)
    {
    case 1:
        puttiletranslucent8(dest,tile,x,y,csets[0],flip,opacity);
        break;
        
    case 3:
        if(flip&2)
        {
            zc_swap(t[0],t[1]);
        }
        
        puttiletranslucent8(dest,t[0],x,y,  csets[0],flip,opacity);
        puttiletranslucent8(dest,t[1],x,y+8,csets[1],flip,opacity);
        break;
        
    case 5:
        if(flip&1)
        {
            zc_swap(t[0],t[1]);
        }
        
        puttiletranslucent8(dest,t[0],x,  y,csets[0],flip,opacity);
        puttiletranslucent8(dest,t[1],x+8,y,csets[1],flip,opacity);
        break;
        
    case 15:
        if(flip&1)
        {
            zc_swap(t[0],t[1]);
            zc_swap(t[2],t[3]);
        }
        
        if(flip&2)
        {
            zc_swap(t[0],t[2]);
            zc_swap(t[1],t[3]);
        }
        
        puttiletranslucent8(dest,t[0],x,  y,  csets[0],flip,opacity);
        puttiletranslucent8(dest,t[1],x+8,y,  csets[1],flip,opacity);
        puttiletranslucent8(dest,t[2],x,  y+8,csets[2],flip,opacity);
        puttiletranslucent8(dest,t[3],x+8,y+8,csets[3],flip,opacity);
        break;
    }
}

void overblocktranslucent8(BITMAP *dest,int32_t tile,int32_t x,int32_t y,int32_t csets[],int32_t flip,int32_t mask,int32_t opacity)
{
    int32_t t[4];
    
    for(int32_t i=0; i<4; ++i)
        t[i]=tile+i;
        
    switch(mask)
    {
    case 1:
        overtiletranslucent8(dest,tile,x,y,csets[0],flip,opacity);
        break;
        
    case 3:
        if(flip&2)
        {
            zc_swap(t[0],t[1]);
            zc_swap(csets[0],csets[1]);
        }
        
        overtiletranslucent8(dest,t[0],x,y,  csets[0],flip,opacity);
        overtiletranslucent8(dest,t[1],x,y+8,csets[1],flip,opacity);
        break;
        
    case 5:
        if(flip&1)
        {
            zc_swap(t[0],t[1]);
        }
        
        overtiletranslucent8(dest,t[0],x,  y,csets[0],flip,opacity);
        overtiletranslucent8(dest,t[1],x+8,y,csets[1],flip,opacity);
        break;
        
    case 15:
        if(flip&1)
        {
            zc_swap(t[0],t[1]);
            zc_swap(t[2],t[3]);
        }
        
        if(flip&2)
        {
            zc_swap(t[0],t[2]);
            zc_swap(t[1],t[3]);
        }
        
        overtiletranslucent8(dest,t[0],x,  y,  csets[0],flip,opacity);
        overtiletranslucent8(dest,t[1],x+8,y,  csets[1],flip,opacity);
        overtiletranslucent8(dest,t[2],x,  y+8,csets[2],flip,opacity);
        overtiletranslucent8(dest,t[3],x+8,y+8,csets[3],flip,opacity);
        break;
    }
}

//  cid: fffffsss cccccccc
//          (f:flags, s:cset, c:combo)

int combotile_override_x = -1, combotile_override_y = -1;
int combotile_add_x = 0, combotile_add_y = 0;
double combotile_mul_x = 1, combotile_mul_y = 1;
int32_t eyeball_combo_tile(const minicombo_drawing &c, int32_t x, int32_t y, int drawtile)
{
	int tframes = zc_max(1, c.frames);

	// This allows us to adjust how eyeball tiles should be drawn in edge cases, like:
	// - the editor
	// - during screen scroll transitions
	if(combotile_override_x > -1)
		x = combotile_override_x;
	if(combotile_override_y > -1)
		y = combotile_override_y;
	x *= combotile_mul_x;
	y *= combotile_mul_y;
	x += combotile_add_x;
	y += combotile_add_y;
#ifdef IS_PLAYER
	x += viewport.x;
	y += viewport.y;
#endif

    switch(combo_class_buf[c.type].directional_change_type)
    {
    default:
        break;
        
    case 1: //cOLD_EYEBALL_A
    {
        double ddir=atan2((double)(y-HeroModifiedY()-playing_field_offset), (double)(HeroModifiedX()-x));
        
        if((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
        {
            drawtile+=tframes*5;                                //dl
        }
        else if((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
        {
            drawtile+=tframes*4;                                //d
        }
        else if((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
        {
            drawtile+=tframes*3;                                //dr
        }
        else if((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
        {
            drawtile+=tframes*2;                                //r
        }
        else if((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
        {
            drawtile+=tframes*1;                                //ur
        }
        else if((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
        {
            drawtile+=tframes*0;                                //u
        }
        else if((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
        {
            drawtile+=tframes*7;                                //ul
        }
        else
        {
            drawtile+=tframes*6;                                //l
        }
        
        break;
    }
    
    case 3: // 4-way Eyeball (up-down-left-right)
    {
        double ddir=atan2((double)(y-HeroModifiedY()-playing_field_offset), (double)(HeroModifiedX()-x));
        
        if((ddir<=(((-2)*PI)/8))&&(ddir>(((-6)*PI)/8)))
        {
            drawtile+=tframes*1;                                //d
        }
        else if((ddir<=(((2)*PI)/8))&&(ddir>(((-2)*PI)/8)))
        {
            drawtile+=tframes*3;                                //r
        }
        else if((ddir<=(((6)*PI)/8))&&(ddir>(((2)*PI)/8)))
        {
            drawtile+=tframes*0;                                //u
        }
        else
        {
            drawtile+=tframes*2;                                //l
        }
        
        break;
    }
    
    case 2: //cOLD_EYEBALL_B
    {
        double ddir=atan2((double)(y-HeroModifiedY()-playing_field_offset), (double)(HeroModifiedX()-x));
        
        if((ddir<=(((-6)*PI)/8))&&(ddir>(((-8)*PI)/8)))
        {
            drawtile+=tframes*5;                                //dl
        }
        else if((ddir<=(((-4)*PI)/8))&&(ddir>(((-6)*PI)/8)))
        {
            drawtile+=tframes*4;                                //d
        }
        else if((ddir<=(((-2)*PI)/8))&&(ddir>(((-4)*PI)/8)))
        {
            drawtile+=tframes*3;                                //dr
        }
        else if((ddir<=(((0)*PI)/8))&&(ddir>(((-2)*PI)/8)))
        {
            drawtile+=tframes*2;                                //r
        }
        else if((ddir<=(((2)*PI)/8))&&(ddir>(((0)*PI)/8)))
        {
            drawtile+=tframes*1;                                //ur
        }
        else if((ddir<=(((4)*PI)/8))&&(ddir>(((2)*PI)/8)))
        {
            drawtile+=tframes*0;                                //u
        }
        else if((ddir<=(((6)*PI)/8))&&(ddir>(((4)*PI)/8)))
        {
            drawtile+=tframes*7;                                //ul
        }
        else
        {
            drawtile+=tframes*6;                                //l
        }
        
        break;
    }
    }
    
    return drawtile;
}
int32_t combo_tile(const minicombo_drawing &c, int32_t x, int32_t y, int frame)
{
	int drawtile;
	if (frame < 0)
		drawtile = c.tile;
	else
	{
		frame %= zc_max(1, c.frames);
		drawtile = c.o_tile + ((1+c.skipanim)*frame);
		if(int32_t rowoffset = TILEROW(drawtile)-TILEROW(c.o_tile))
			drawtile += c.skipanimy * rowoffset * TILES_PER_ROW;
	}
	int directional_change_type = combo_class_buf[c.type].directional_change_type;
	if (directional_change_type == 0 || directional_change_type > 3)
		return drawtile;
	return eyeball_combo_tile(c, x, y, drawtile);
}

int32_t combo_tile(int32_t cid, int32_t x, int32_t y, int frame)
{
    return combo_tile(GET_DRAWING_COMBO(cid), x, y, frame);
}

void putcombotranslucent(BITMAP* dest,int32_t x,int32_t y,int32_t cid,int32_t cset,int32_t opacity)
{
	auto& c = GET_DRAWING_COMBO(cid);
    int32_t drawtile=combo_tile(c, x, y);
    
    if(!(c.csets&0xF0) || !(c.csets&0x0F) || (newtilebuf[drawtile].format>tf4Bit))
        puttiletranslucent16(dest,drawtile,x,y,cset,c.flip,opacity);
    else
    {
        int32_t csets[4];
        int32_t cofs = c.csets&15;
        if(cofs&8)
            cofs |= ~int32_t(0xF);
            
        for(int32_t i=0; i<4; ++i)
            csets[i] = c.csets&(16<<i) ? WRAP_CS2(cset, cofs) : cset;
            
        putblocktranslucent8(dest,drawtile<<2,x,y,csets,c.flip,15,opacity);
    }
}

void overcombotranslucent(BITMAP* dest,int32_t x,int32_t y,int32_t cid,int32_t cset,int32_t opacity)
{
    overcomboblocktranslucent(dest, x, y, cid, cset, 1, 1, opacity);
}

void overcomboblocktranslucent(BITMAP *dest, int32_t x, int32_t y, int32_t cid, int32_t cset, int32_t w, int32_t h, int32_t opacity)
{
    if ((unsigned)cid >= MAXCOMBOS) return;

    auto& c = GET_DRAWING_COMBO(cid);
    int32_t drawtile=combo_tile(c, x, y);
    
    for(int32_t woff=0; woff<w; woff++)
    {
        for(int32_t hoff = 0; hoff<h; hoff++)
        {
            int32_t tiletodraw = drawtile + 20*hoff+woff;
            
            // If this block goes past the edge of the tile page and
            // animation skip Y is used, skip rows accordingly
            if(tiletodraw%TILES_PER_ROW<woff) // Past the end?
                tiletodraw+=TILES_PER_ROW*c.skipanimy;
                
            if(!(c.csets&0xF0) || !(c.csets&0x0F) || (newtilebuf[tiletodraw].format>tf4Bit))
                overtiletranslucent16(dest,tiletodraw,x+16*woff,y+16*hoff,cset,c.flip,opacity);
            else
            {
                int32_t csets[4];
                int32_t cofs = c.csets&15;
                if(cofs&8)
                    cofs |= ~int32_t(0xF);
                    
                for(int32_t i=0; i<4; ++i)
                    csets[i] = c.csets&(16<<i) ? WRAP_CS2(cset, cofs) : cset;
                    
                overblocktranslucent8(dest,tiletodraw<<2,x+16*woff,y+16*hoff,csets,c.flip,15,opacity);
            }
        }
    }
}

//shnarf
//narp?

void puttile8(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip)
{
    if(x<-7 || y<-7)
        return;
        
    if(x >= dest->w || y >= dest->h)
        return;

    if(newtilebuf[tile>>2].format>tf4Bit)
    {
        cset=0;
    }
    
    cset &= 15;
    cset <<= CSET_SHFT;
    dword lcset = (cset<<24)+(cset<<16)+(cset<<8)+cset;

    const byte* bytes = get_tile_bytes(tile>>2, 0);

    // TODO: only title.cpp uses this function, so don't bother with this yet. Following code noy verified.
    // 0: fast, no bounds checking
    // 1: slow, bounds checking
    // int draw_mode = x < 0 || y < 0 || x >= dest->w-8 || y >= dest->h-8 || x%8 || y%8 ? 1 : 0;
    // if (draw_mode == 1)
    // {
    //     byte *si = unpackbuf + ((tile&2)<<6) + ((tile&1)<<3);
    //     draw_tile8_unified(dest, cl, ct, cr, cb, si, x, y, cset, flip, false);
    //     return;
    // }
    
    switch(flip&3)
    {
    case 1:                                                 // 1 byte at a time
    {
        const byte *si = bytes + ((tile&2)<<6) + ((tile&1)<<3);
        
        for(int32_t dy=0; dy<8; ++dy)
        {
            byte *di = &(dest->line[y+dy][x+7]);
            
            for(int32_t i=0; i<8; ++i)
                *(di--) = *(si++) + cset;
                
            si+=8;
        }
    }
    break;
    
    case 2:                                                 // 4 bytes at a time
    {
        const dword *si = ((const dword*)bytes) + ((tile&2)<<4) + ((tile&1)<<1);
        
        for(int32_t dy=7; dy>=0; --dy)
        {
            dword *di=&((dword*)dest->line[y+dy])[x>>2];
            *(di++) = *(si++) + lcset;
            *(di++) = *(si++) + lcset;
            si+=2;
        }
    }
    break;
    
    case 3:                                                 // 1 byte at a time
    {
        const byte *si = bytes + ((tile&2)<<6) + ((tile&1)<<3);
        
        for(int32_t dy=7; dy>=0; --dy)
        {
            byte *di = &(dest->line[y+dy][x+7]);
            
            for(int32_t i=0; i<8; ++i)
                *(di--) = *(si++) + cset;
                
            si+=8;
        }
    }
    break;
    
    default:                                                // 4 bytes at a time
    {
        const dword *si = ((const dword*)bytes) + ((tile&2)<<4) + ((tile&1)<<1);
        
        for(int32_t dy=0; dy<8; ++dy)
        {
            dword *di = &((dword*)dest->line[y+dy])[x>>2];
            *(di++) = *(si++) + lcset;
            *(di++) = *(si++) + lcset;
            si+=2;
        }
    }
    break;
    }
}


void oldputtile8(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip)
{
    if(x<-7 || y<-7)
        return;
        
    if(y > dest->h)
        return;
        
    if(y == dest->h && x > dest->w)
        return;
        
    if(newtilebuf[tile>>2].format>tf4Bit)
    {
        cset=0;
    }
    
    cset &= 15;
    cset <<= CSET_SHFT;
    const byte* bytes = get_tile_bytes(tile>>2, 0);
    const byte *si = bytes + ((tile&2)<<6) + ((tile&1)<<3);
    
    if(flip&1)
    {
        si+=7;
    }
    
    if((flip&2)==0)                                           //not flipped vertically
    {
        if(y<0)
        {
            si+=(0-y)<<4;
        }
        
        for(int32_t dy=(y<0 ? 0-y : 0); (dy<8)&&(dy+y<dest->h); ++dy)
        {
            byte* di = &(dest->line[y+dy][x<0 ? 0 : x]);
            
            for(int32_t i=0; i<8; ++i)
            {
                if(x+i<dest->w)
                {
                    *(di) = (*si) + cset;
                    ++di;
                }
                
                flip&1 ? --si : ++si;
            }
            
            if(flip&1)
            {
                si+=24;
            }
            else
            {
                si+=8;
                
            }
        }
    }                                                         //flipped vertically
    else
    {
        if(y+7>=dest->h)
        {
            si+=(8+y-dest->h)<<4;
        }
        
        for(int32_t dy=(y+7>=dest->h ? dest->h-y-1 : 7); (dy>=0)&&(dy+y>=0); --dy)
        {
            byte* di = &(dest->line[y+dy][x<0 ? 0 : x]);
            
            for(int32_t i=0; i<8; ++i)
            {
                if(x+i<dest->w)
                {
                    *(di) = (*si) + cset;
                    ++di;
                }
                
                flip&1 ? --si : ++si;
            }
            
            if(flip&1)
            {
                si+=24;
            }
            else
            {
                si+=8;
            }
        }
    }
}

void overtile8(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip)
{
	int w = 8;
	int cl = 0;
	int ct = 0;
	int cr = dest->w;
	int cb = dest->h;
	if (dest->clip)
	{
		cl = dest->cl;
		ct = dest->ct;
		cr = dest->cr;
		cb = dest->cb;
	}

	if (x + w < cl)
		return;
	if (x > cr)
		return;
	if (y + w < ct)
		return;
	if (y > cb)
		return;
		
	if(blank_tile_quarters_table[tile])
	{
		return;
	}
	
	if(newtilebuf[tile>>2].format>tf4Bit)
	{
		cset=0;
	}
	
	cset &= 15;
	cset <<= CSET_SHFT;
	const byte *bytes = get_tile_bytes(tile>>2, 0);
	const byte *si = bytes + ((tile&2)<<6) + ((tile&1)<<3);

	bool use_fast_draw = (x >= cl) && (y >= ct) && (x + w <= cr) && (y + w <= cb);
	if (use_fast_draw)
		draw_tile8_transparent_fast(dest, si, x, y, cset, flip);
	else
		draw_tile8_unified(dest, cl, ct, cr, cb, si, x, y, cset, flip);
}

void puttile16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip) //fixed
{
    int cl = 0;
    int ct = 0;
    int cr = dest->w;
    int cb = dest->h;
    if (dest->clip)
    {
        cl = dest->cl;
        ct = dest->ct;
        cr = dest->cr;
        cb = dest->cb;
    }

	if (x + 16 <= cl)
		return;
	if (x >= cr)
		return;
	if (y + 16 <= ct)
		return;
	if (y >= cb)
		return;
        
    if(tile<0 || tile>=NEWMAXTILES)
    {
        rectfill(dest,x,y,x+15,y+15,0);
        return;
    }
    
    if(newtilebuf[tile].format>tf4Bit)
    {
        cset=0;
    }
    
    cset &= 15;
    cset <<= CSET_SHFT;
    const byte *bytes = get_tile_bytes(tile, flip&5);

    bool use_fast_draw = (x >= cl) && (y >= ct) && (x + 16 <= cr) && (y + 16 <= cb);
	if (use_fast_draw)
	{
		draw_tile16_opaque_fast(dest, bytes, x, y, cset, flip);
	}
	else
	{
		draw_tile16_unified(dest, cl, ct, cr, cb, bytes, x, y, cset, flip, false);
	}
}

void oldputtile16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip) //fixed
{
    if(x<-15 || y<-15)
        return;
        
    if(y > dest->h)
        return;
        
    if(y == dest->h && x > dest->w)
        return;
        
    if(tile<0 || tile>=NEWMAXTILES)
    {
        rectfill(dest,x,y,x+15,y+15,0);
        return;
    }
    
    if(newtilebuf[tile].format>tf4Bit)
    {
        cset=0;
    }
    
    cset &= 15;
    cset <<= CSET_SHFT;

    const byte* si = get_tile_bytes(tile, flip&5);
    byte *di;
    
    if((flip&2)==0)
    {
        if(y<0)
            si+=(0-y)<<4;
            
        for(int32_t dy=(y<0 ? 0-y : 0); (dy<16)&&(dy+y<dest->h); ++dy)
        {
            di = &(dest->line[y+dy][x<0 ? 0 : x]);
            
            if(x+15<dest->w)
            {
                if(x<0)
                    si+=0-x;
                    
                for(int32_t dx=(x<0 ? 0-x : 0); dx<16; ++dx)
                {
                    *di=*si+cset;
                    ++di;
                    ++si;
                }
            }
            else
            {
                for(int32_t i=0; i<16; ++i)
                {
                    if(x+i<dest->w)
                    {
                        *di=*si+cset;
                        ++di;
                    }
                    
                    ++si;
                }
            }
        }
    }
    else
    {
        if(y+15>=dest->h)
            si+=(16+y-dest->h)<<4;
            
        for(int32_t dy=(y+15>=dest->h ? dest->h-y-1 : 15); (dy>=0)&&(dy+y>=0); --dy)
        {
            di = &(dest->line[y+dy][x<0 ? 0 : x]);
            
            if(x+15<dest->w)
            {
                if(x<0)
                    si+=0-x;
                    
                for(int32_t dx=(x<0 ? 0-x : 0); dx<16; ++dx)
                {
                    *di=*si+cset;
                    ++di;
                    ++si;
                }
            }
            else
            {
                for(int32_t i=0; i<16; ++i)
                {
                    if(x+i<dest->w)
                    {
                        *di=*si+cset;
                        ++di;
                    }
                    
                    ++si;
                }
            }
        }
    }
}

void overtileblock16(BITMAP* _Dest, int32_t tile, int32_t x, int32_t y, int32_t w, int32_t h, int32_t color, int32_t flip, byte skiprows)
{
	if(skiprows>0 && tile%TILES_PER_ROW+w>=TILES_PER_ROW)
	{
		byte w2=(tile+w)%TILES_PER_ROW;
		overtileblock16(_Dest, tile, x, y, w-w2, h, color, flip);
		overtileblock16(_Dest, tile+(w-w2)+(skiprows*TILES_PER_ROW), x+16*(w-w2), y, w2, h, color, flip);
		return;
	}
	
	switch(flip)
	{
	case 1:
		for(int32_t j=0; j<h; j++)
			for(int32_t k=w-1; k>=0; k--)
				overtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+j*16, color, flip);
				
		break;
		
	case 2:
		for(int32_t j=h-1; j>=0; j--)
			for(int32_t k=0; k<w; k++)
				overtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+((h-1)-j)*16, color, flip);
				
		break;
		
	case 3:
		for(int32_t j=h-1; j>=0; j--)
			for(int32_t k=w-1; k>=0; k--)
				overtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+((h-1)-j)*16, color, flip);
				
		break;
		
	default:
		for(int32_t j=0; j<h; j++)
			for(int32_t k=0; k<w; k++)
				overtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+j*16, color, flip);
				
		break;
	}
}
void overtile16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip)
{
	int cl = 0;
	int ct = 0;
	int cr = dest->w;
	int cb = dest->h;
	if (dest->clip)
	{
		cl = dest->cl;
		ct = dest->ct;
		cr = dest->cr;
		cb = dest->cb;
	}

	if (x + 16 <= cl)
		return;
	if (x >= cr)
		return;
	if (y + 16 <= ct)
		return;
	if (y >= cb)
		return;

	if(tile<0 || tile>=NEWMAXTILES)
	{
		rectfill(dest,x,y,x+15,y+15,0);
		return;
	}
	
	if(blank_tile_table[tile])
		return;
	
	if(newtilebuf[tile].format>tf4Bit)
	{
		cset=0;
	}
	
	cset &= 15;
	cset <<= CSET_SHFT;
	const byte *si = get_tile_bytes(tile, flip&5);

	draw_tile16_unified(dest, cl, ct, cr, cb, si, x, y, cset, flip, true);
}

void overtile16_scale(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip,int dw, int dh)
{
	if(x<-dw || y<-dh)
		return;
		
	if(y > dest->h)
		return;
		
	if(y == dest->h && x > dest->w)
		return;
		
	if(tile<0 || tile>=NEWMAXTILES)
	{
		rectfill(dest,x,y,x+dw-1,y+dh-1,0);
		return;
	}
	
	if(blank_tile_table[tile])
		return;
	
	static BITMAP* tmp = create_bitmap_ex(8,16,16);
	clear_bitmap(tmp);
	overtile16(tmp,tile,0,0,cset,flip);
	masked_stretch_blit(tmp,dest,0,0,16,16,x,y,dw,dh);
}

static void draw_tile16_cs2_fast(BITMAP *dest, const byte *si, int32_t x, int32_t y, int32_t cset[], int32_t flip, bool over)
{
	// Vertical flip.
	bool v_flip = (flip & 2);
	int i_first = v_flip ? 2 : 0; // Index for the first 8 rows processed
	int i_last  = v_flip ? 0 : 2; // Index for the last 8 rows processed

	// We need to combine two 8-byte colors into one 16-byte register.
	// Low 64 bits = Left 8 pixels, High 64 bits = Right 8 pixels.

	// Top half colors (cset[0] left, cset[1] right)
	__m128i c0 = _mm_set1_epi8((char)cset[i_first]);
	__m128i c1 = _mm_set1_epi8((char)cset[i_first + 1]);
	__m128i top_colors = _mm_unpacklo_epi64(c0, c1); 

	// Bottom half colors (cset[2] left, cset[3] right).
	__m128i c2 = _mm_set1_epi8((char)cset[i_last]);
	__m128i c3 = _mm_set1_epi8((char)cset[i_last + 1]);
	__m128i bot_colors = _mm_unpacklo_epi64(c2, c3);

	const uint8_t* src_ptr = si;

	// Pre-calculate the zero vector for transparency checks.
	__m128i zero = _mm_setzero_si128();

	for (int dy = 0; dy < 16; ++dy)
	{
		// Calculate destination pointer
		// If vertical flip: Map source row 0 to dest row 15.
		int dest_y_offset = v_flip ? (15 - dy) : dy;
		uint8_t* d_ptr = dest->line[y + dest_y_offset] + x;

		// Select the correct color set for this row.
		// Rows 0-7 use top_colors, Rows 8-15 use bot_colors.
		__m128i color_vec = (dy < 8) ? top_colors : bot_colors;

		// Load source pixels (unaligned).
		__m128i src_pixels = _mm_loadu_si128((const __m128i*)src_ptr);

		// Apply colors.
		__m128i out_pixels = _mm_add_epi8(src_pixels, color_vec);

		if (over)
		{
			// Transparency.
			// Check where source is zero (0xFF where zero, 0x00 where pixel exists).
			__m128i is_zero = _mm_cmpeq_epi8(src_pixels, zero);

			// Load existing background.
			__m128i bg_pixels = _mm_loadu_si128((const __m128i*)d_ptr);

			// Combine: (Background AND Mask) OR (NewPixel AND NOT Mask)
			__m128i result = _mm_or_si128(
				_mm_and_si128(is_zero, bg_pixels),
				_mm_andnot_si128(is_zero, out_pixels)
			);

			_mm_storeu_si128((__m128i*)d_ptr, result);
		}
		else
		{
			// Opaque (simply overwrite).
			_mm_storeu_si128((__m128i*)d_ptr, out_pixels);
		}

		src_ptr += 16;
	}
}

static void draw_tile16_cs2_safe(BITMAP *dest, int cl, int ct, int cr, int cb, const byte *si, int32_t x, int32_t y, int32_t cset[], int32_t flip, bool over)
{
	// Clip.
	int start_x = 0;
	int end_x = 16;
	if (x < cl) start_x = cl - x;
	if (x + 16 > cr) end_x = cr - x;
	if (start_x >= end_x) return;

	int clip_top = (ct > 0) ? ct : 0;
	int clip_bottom = (cb < dest->h) ? cb : dest->h;
	int visible_y_start = (clip_top > y) ? clip_top : y;
	int visible_y_end = (clip_bottom < y + 16) ? clip_bottom : y + 16;
	if (visible_y_start >= visible_y_end) return;

	bool v_flip = (flip & 2);

	for (int dest_y = visible_y_start; dest_y < visible_y_end; ++dest_y)
	{
		// Calculate relative Y (0..15) inside the tile
		int rel_y = dest_y - y;

		// Calculate source Y based on flip
		int src_y = v_flip ? (15 - rel_y) : rel_y;

		// Point to the start of the source row
		const byte *src_row = si + (src_y * 16);
		uint8_t *d_ptr = dest->line[dest_y] + x;

		// Select row colors based on relative Y
		// If rel_y < 8, we are in the top quadrants (cset 0/1)
		// If rel_y >= 8, we are in the bottom quadrants (cset 2/3)
		int cs_left, cs_right;
		if (rel_y < 8) {
			cs_left = cset[0]; cs_right = cset[1];
		} else {
			cs_left = cset[2]; cs_right = cset[3];
		}

		// Draw the row loop, handling the Left/Right split manually
		// to avoid "dx < 8" checks inside the loop.
		for (int dx = start_x; dx < end_x; ++dx)
		{
			// Select color based on column (Left 8 or Right 8)
			int cs = (dx < 8) ? cs_left : cs_right;
			
			byte val = src_row[dx];
			if (!over || val) 
			{
				d_ptr[dx] = val + cs;
			}
		}
	}
}

void drawtile16_cs2(BITMAP *dest,int32_t tile,int32_t x,int32_t y,int32_t cset[],int32_t flip,bool over)
{
	int cl = 0;
	int ct = 0;
	int cr = dest->w;
	int cb = dest->h;
	if (dest->clip)
	{
		cl = dest->cl;
		ct = dest->ct;
		cr = dest->cr;
		cb = dest->cb;
	}

	if (x + 16 < cl)
		return;
	if (x > cr)
		return;
	if (y + 16 < ct)
		return;
	if (y > cb)
		return;

	if(tile<0 || tile>=NEWMAXTILES)
	{
		rectfill(dest,x,y,x+15,y+15,0);
		return;
	}

	if (blank_tile_table[tile])
	{
		if(!over)
			rectfill(dest, x, y, x + 15, y + 15, 0);
		return;
	}

	if(newtilebuf[tile].format>tf4Bit)
		cset[0]=cset[1]=cset[2]=cset[3]=0;
	else for(int q = 0; q < 4; ++q)
		cset[q] <<= CSET_SHFT;

	const byte* si = get_tile_bytes(tile, flip&5);

	bool use_fast_draw = (x >= cl) && (y >= ct) && (x + 16 <= cr) && (y + 16 <= cb);
	if (use_fast_draw)
		draw_tile16_cs2_fast(dest, si, x, y, cset, flip, over);
	else
		draw_tile16_cs2_safe(dest, cl, ct, cr, cb, si, x, y, cset, flip, over);
}

//  cid: fffffsss cccccccc
//          (f:flags, s:cset, c:combo)

void putcombo(BITMAP* dest,int32_t x,int32_t y,int32_t cid,int32_t cset)
{
    auto& c = GET_DRAWING_COMBO(cid);
    int32_t drawtile=combo_tile(c, x, y);
    
    if(!(c.csets&0xF0) || !(c.csets&0x0F) || (newtilebuf[drawtile].format>tf4Bit))
        puttile16(dest,drawtile,x,y,cset,c.flip);
    else
    {
        int32_t csets[4];
        int32_t cofs = c.csets&15;
        if(cofs&8)
            cofs |= ~int32_t(0xF);
        
        for(int32_t i=0; i<4; ++i)
        {
            csets[i] = c.csets&(16<<i) ? WRAP_CS2(cset, cofs) : cset;
        }
        
        drawtile16_cs2(dest,drawtile,x,y,csets,c.flip,false);
    }
}

void overcombo(BITMAP* dest,int32_t x,int32_t y,int32_t cid,int32_t cset)
{
    overcomboblock(dest, x, y, cid, cset, 1, 1);
}

void overcomboblock(BITMAP *dest, int32_t x, int32_t y, int32_t cid, int32_t cset, int32_t w, int32_t h)
{
    if ((unsigned)cid >= MAXCOMBOS) return;

    auto& c = GET_DRAWING_COMBO(cid);
    int32_t drawtile=combo_tile(c, x, y);
    
    for(int32_t woff = 0; woff < w; woff++)
    {
        for(int32_t hoff =0; hoff < h; hoff++)
        {
            int32_t tiletodraw = drawtile + 20*hoff+woff;
            
            // If this block goes past the edge of the tile page and
            // animation skip Y is used, skip rows accordingly
            if(tiletodraw%TILES_PER_ROW<woff) // Past the end?
                tiletodraw+=TILES_PER_ROW*c.skipanimy;
                
            if(!(c.csets&0xF0) || !(c.csets&0x0F) || (newtilebuf[tiletodraw].format>tf4Bit))
                overtile16(dest,tiletodraw,x+16*woff,y+16*hoff,cset,c.flip);
            else
            {
                int32_t csets[4];
                int32_t cofs = c.csets&15;
                if(cofs&8)
                    cofs |= ~int32_t(0xF);
                    
                for(int32_t i=0; i<4; ++i)
                    csets[i] = c.csets&(16<<i) ? WRAP_CS2(cset, cofs) : cset;
                    
                drawtile16_cs2(dest,tiletodraw,x+16*woff,y+16*hoff,csets,c.flip,true);
            }
        }
    }
}

void overcombo2(BITMAP* dest,int32_t x,int32_t y,int32_t cid,int32_t cset)
{
    if(cid!=0)
    {
        overcombo(dest,x,y,cid,cset);
    }
}

bool is_valid_format(byte format)
{
    switch(format)
    {
    case tf32Bit:
    case tf24Bit:
    case tf16Bit:
    case tf8Bit:
    case tf4Bit:
        return true;
        
    default:
        return false;
    }
}

int32_t tilesize(byte format)
{
	switch(format)
	{
		case tf32Bit:
			return 1024;
			
		case tf24Bit:
			return 768;
			
		case tf16Bit:
			return 512;

        case tf8Bit:
		case tf4Bit:
            return 256;
	}

	// BUG: This is triggered by the 'grab' option, and certainly others as well.
	// if at any point a selected tile is 'blank' (newtilebuf[0] for example), then we have might a problem.
	// These should probably be dealt with where they really occur, however simply returning a
	// sane amount right now should fix any crashes associated with this. -Gleeok
	return 256;
}

