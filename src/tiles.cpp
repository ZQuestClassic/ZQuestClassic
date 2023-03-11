//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  tiles.cc
//
//  Tile drawing routines for ZC.
//
//  !! Don't use them on "screen"; use memory BITMAPs only.
//
//--------------------------------------------------------

#include "precompiled.h" //always first

#include "base/zc_alleg.h"
#include <string.h>

#include "base/zdefs.h"
#include "base/zsys.h"
#include "tiles.h"

extern RGB_MAP rgb_table;
extern COLOR_MAP trans_table;
extern itemdata   *itemsbuf;
extern wpndata    *wpnsbuf;
extern byte        quest_rules[QUESTRULES_SIZE];
//extern byte *tilebuf;
//BITMAP* tilebuf[NEWMAXTILES];
tiledata *newtilebuf, *grabtilebuf;
newcombo *combobuf;
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

byte unpackbuf[UNPACKSIZE];

bool isblanktile(tiledata *buf, int32_t i)
{
    //  byte *tilestart=tilebuf+(i*128);
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
    "(Invalid)", "4-bit", "8-bit", "16-bit", "24-bit", "32-bit"
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
    //int32_t tiles_used=count_tiles(newtilebuf);
    for(int32_t i=0; i<NEWMAXTILES; ++i)
    {
	    //al_trace("Registering a blank tile\n");
        register_blank_tile_quarters(i);
        blank_tile_table[i]=isblanktile(newtilebuf, i);
    }
}

void register_blank_tiles(int32_t max)
{
    //int32_t tiles_used=count_tiles(newtilebuf);
    for(int32_t i=0; i<max; ++i)
    {
	    //al_trace("Registering a blank tile, iteration: \n", i);
        register_blank_tile_quarters(i);
        blank_tile_table[i]=isblanktile(newtilebuf, i);
    }
}

//returns the number of tiles
int32_t count_tiles(tiledata *buf)
{
    int32_t tiles_used;
    
    //  bool used;
    //  int32_t x;
    for(tiles_used=(NEWMAXTILES); tiles_used>0; --tiles_used)
    {
        /*
          used=false;
          for (x=0; x<128; ++x)
          {
          used=used || (tilebuf[(tiles_used-1)*128+x]!=0);
          }
          if (used)
          {
          break;
          }
          */
	    //al_trace("count_tiles() is checking if tile %d is blank.\n", tiles_used-1);
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
            return;
        }
    }
}

void reset_combo_animation2(int32_t c)
{
    for(word x=0; x<animated_combos2; ++x)
    {
        int32_t y=animated_combo_table24[x][0];                      //combo number
        
        if(y==c)
        {
            combobuf[y].tile=combobuf[y].o_tile;        //reset tile
            combobuf[y].cur_frame=0;
            combobuf[y].aclk=0;                        //reset clock
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
    }
}

extern void update_combo_cycling();

//Returns true if 'tile' is the LAST tile in the animation defined by the other parameters.
bool combocheck(newcombo& cdata)
{
	if(get_bit(quest_rules, qr_BROKEN_ASKIP_Y_FRAMES))
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

void animate(newcombo& cdata, bool forceNextFrame)
{
	if(cdata.aclk>=cdata.speed || forceNextFrame)      //time to animate
	{
		if(get_bit(quest_rules, qr_NEW_COMBO_ANIMATION))
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
	}
	else
	{
		if(get_bit(quest_rules, qr_NEW_COMBO_ANIMATION))
		{
			cdata.tile = cdata.o_tile + ((1+cdata.skipanim)*cdata.cur_frame);
			if(int32_t rowoffset = TILEROW(cdata.tile)-TILEROW(cdata.o_tile))
			{
				cdata.tile += cdata.skipanimy * rowoffset * TILES_PER_ROW;
			}
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
        
		animate(combobuf[y]);
    }
    
    for(word x=0; x<animated_combos2; ++x)
    {
        int32_t y=animated_combo_table24[x][0];                      //combo number
        
        animate(combobuf[y]);
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

//clears a tile
/*
void clear_tile(tiledata *buf, word tile)
{
  buf[tile].format=tf4Bit;
  if (buf[tile].data!=NULL)
  {
    free(buf[tile].data);
    buf[tile].data = NULL;
  }
  buf[tile].data=(byte *)malloc(tilesize(buf[tile].format));
  if (buf[tile].data==NULL)
  {
    Z_error_fatal("Unable to initialize tile #%d.\n", tile);
    exit(1);
  }
  memset(buf[tile].data,0,tilesize(buf[tile].format));
}
*/

void reset_tile(tiledata *buf, int32_t t, int32_t format=1)
{
    buf[t].format=format;
    
    if(buf[t].data!=NULL)
    {
        free(buf[t].data);
    }
    
    buf[t].data=(byte *)malloc(tilesize(buf[t].format));
    
    if(buf[t].data==NULL)
    {
        quit_game();
        Z_error_fatal("Unable to initialize tile #%d.\n", t);
    }
    
    for(int32_t i=0; i<tilesize(buf[t].format); i++)
    {
        buf[t].data[i]=0;
    }
    
    /*if(zctiles)
    {
    if(tilebuf[t] != NULL) destroy_bitmap(tilebuf[t]);
    tilebuf = create_bitmap_ex(8,16,16);
    if(buf[t].format == tf4Bit)
    {
      for(int32_t i=0; i<128; i++)
      {
        tilebuf[t]->line[i/8][(i%8)*2] = buf[t].data[i]>>4;
    	tilebuf[t]->line[i/8][(i%8)*2+1] = buf[t].data[i]&0xF;
      }
    }
    else
    {
      for(int32_t i=0; i<256; i++)
      {
        tilebuf[t]->line[i/16][i%16] = buf[t].data[i];
      }
    }
    }*/
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
    
    if(swap)
    {
        for(int32_t j=0; j<tilesize(tempformat); j++)
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
        
        for(int32_t j=0; j<tilesize(tempformat); j++)
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


// unpacks from tilebuf to unpackbuf
void unpack_tile(tiledata *buf, int32_t tile, int32_t flip, bool force)
{
    static byte *si, *di;
    static byte *oldnewtilebuf=buf[tile].data;
    static int32_t i, j, oldtile=-5, oldflip=-5;
    
    if(tile==oldtile&&(flip&5)==(oldflip&5)&&oldnewtilebuf==buf[tile].data&&!force)
    {
        return;
    }
    
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
                di=unpackbuf + (i<<4) - 1;
                
                for(j=7; j>=0; --j)
                {
                    (*(++di)) = (*(--si)) >> 4;
                    (*(++di)) = (*si) & 15;
                }
                
                break;
                
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
                di=unpackbuf + 271 - i; //256 + 15 - i
                
                for(j=7; j>=0; --j)
                {
                    di-=16;
                    *di = (*(--si)) >> 4;
                    di-=16;
                    *di = (*si) & 15;
                }
                
                break;
                
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
                di=unpackbuf + 256 + i;
                
                for(j=7; j>=0; --j)
                {
                    di-=16;
                    *di = (*(--si)) >> 4;
                    di-=16;
                    *di = (*si) & 15;
                }
                
                break;
                
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
            si = buf[tile].data+tilesize(buf[tile].format);
            di = unpackbuf + 256;
            
            for(i=127; i>=0; --i)
            {
                (*(--di)) = (*(--si)) >> 4;
                (*(--di)) = (*si) & 15;
            }
            
            break;
            
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
        for(int32_t si=0; si<256; si+=2)
        {
            *di = (src[si]&15) + ((src[si+1]&15) << 4);
            ++di;
        }
        
        break;
        
    case tf8Bit:
        for(int32_t si=0; si<32; si+=1)
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

// packs a whole set of tiles from old size to new size
void pack_tiles(byte *buf)
{
    int32_t di = 0;
    
    for(int32_t si=0; si<TILEBUF_SIZE; si+=2)
        buf[di++] = (buf[si]&15) + ((buf[si+1]&15) << 4);
        
    for(; di<NEWTILE_SIZE2; ++di)
        buf[di]=0;
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
    unpack_tile(newtilebuf, tile>>2, 0, false);
    byte *si = unpackbuf + ((tile&2)<<6) + ((tile&1)<<3);
    
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
                    //            *(di) = (opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
                    *(di) = trans_table.data[(*di)][((*si) + cset)];
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
                    //          *(di) = (opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
                    *(di) = trans_table.data[(*di)][((*si) + cset)];
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
    unpack_tile(newtilebuf, tile>>2, 0, false);
    byte *si = unpackbuf + ((tile&2)<<6) + ((tile&1)<<3);
    
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
                        //            *(di) = (opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
                        *(di) = trans_table.data[(*di)][((*si) + cset)];
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
                        //            *(di) = (opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
                        *(di) = trans_table.data[(*di)][((*si) + cset)];
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
    unpack_tile(newtilebuf, tile, 0, false);
    byte *si = unpackbuf;
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
                    //          *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
                    *di=trans_table.data[(*di)][((*si) + cset)];
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
                        //            *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
                        *di=trans_table.data[(*di)][((*si) + cset)];
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
                    //          *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
                    *di=trans_table.data[(*di)][((*si) + cset)];
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
                        //            *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
                        *di=trans_table.data[(*di)][((*si) + cset)];
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
    unpack_tile(newtilebuf, tile,flip&5, false);
    byte *si = unpackbuf;
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
                        //            *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
                        *di=trans_table.data[(*di)][((*si) + cset)];
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
                            //              *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
                            *di=trans_table.data[(*di)][((*si) + cset)];
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
                        //            *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
                        *di=trans_table.data[(*di)][((*si) + cset)];
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
                            //              *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
                            *di=trans_table.data[(*di)][((*si) + cset)];
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
    
    unpack_tile(newtilebuf, tile, 0, false);
    byte *si = unpackbuf;
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

//  cmbdat: fffffsss cccccccc
//          (f:flags, s:cset, c:combo)

int32_t combo_tile(const newcombo &c, int32_t x, int32_t y)
{
    int32_t drawtile=c.tile;
    int32_t tframes=zc_max(1, c.frames);
    
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

int32_t combo_tile(int32_t cmbdat, int32_t x, int32_t y)
{
    const newcombo & c = combobuf[cmbdat];
    return combo_tile(c, x, y);
}

void putcombotranslucent(BITMAP* dest,int32_t x,int32_t y,int32_t cmbdat,int32_t cset,int32_t opacity)
{
    newcombo c = combobuf[cmbdat];
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

void overcombotranslucent(BITMAP* dest,int32_t x,int32_t y,int32_t cmbdat,int32_t cset,int32_t opacity)
{
    overcomboblocktranslucent(dest, x, y, cmbdat, cset, 1, 1, opacity);
}

void overcomboblocktranslucent(BITMAP *dest, int32_t x, int32_t y, int32_t cmbdat, int32_t cset, int32_t w, int32_t h, int32_t opacity)
{
    if ((unsigned)cmbdat >= MAXCOMBOS) return;
    newcombo c = combobuf[cmbdat];
    int32_t drawtile=combo_tile(c, x, y);
    
    for(int32_t woff=0; woff<w; woff++)
    {
        for(int32_t hoff = 0; hoff<h; hoff++)
        {
            int32_t tiletodraw = drawtile + 20*hoff+woff;
            
            // If this block goes past the edge of the tile page and
            // animation skip Y is used, skip rows accordingly
            if(tiletodraw%TILES_PER_ROW<woff) // Past the end?
                tiletodraw+=TILES_PER_ROW*combobuf[cmbdat].skipanimy;
                
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

void puttile8(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip)
{
    if(x<0 || y<0)
        return;
        
    if(y > dest->h-8)
        return;
        
    if(y == dest->h-8 && x > dest->w-8)
        return;
        
    if(newtilebuf[tile>>2].format>tf4Bit)
    {
        cset=0;
    }
    
    cset &= 15;
    cset <<= CSET_SHFT;
    dword lcset = (cset<<24)+(cset<<16)+(cset<<8)+cset;
    unpack_tile(newtilebuf, tile>>2, 0, false);
    
    //  to go to 24-bit color, do this kind of thing...
    //  ((int32_t *)bmp->line[y])[x] = color;
    
    switch(flip&3)
    {
    case 1:                                                 // 1 byte at a time
    {
        byte *si = unpackbuf + ((tile&2)<<6) + ((tile&1)<<3);
        
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
        dword *si = ((dword*)unpackbuf) + ((tile&2)<<4) + ((tile&1)<<1);
        
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
        byte *si = unpackbuf + ((tile&2)<<6) + ((tile&1)<<3);
        
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
        dword *si = ((dword*)unpackbuf) + ((tile&2)<<4) + ((tile&1)<<1);
        
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
    unpack_tile(newtilebuf, tile>>2, 0, false);
    byte *si = unpackbuf + ((tile&2)<<6) + ((tile&1)<<3);
    
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
    unpack_tile(newtilebuf, tile>>2, 0, false);
    byte *si = unpackbuf + ((tile&2)<<6) + ((tile&1)<<3);
    
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
                        //            *(di) = (opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
                        *(di) = (*si) + cset;
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
                        //            *(di) = (opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
                        *(di) = (*si) + cset;
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

void puttile16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip) //fixed
{
    if(x<0 || y<0)
        return;
        
    if(y > dest->h-16)
        return;
        
    if((y == dest->h-16) && (x > dest->w-16))
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
    
    unpack_tile(newtilebuf, tile, flip&5, false);
    
    switch(flip&2)
    {
        /*
          case 1:
          {
          byte *si = unpackbuf;
          for(int32_t dy=0; dy<16; ++dy)
          {
          // 1 byte at a time
          byte *di = &(dest->line[y+dy][x+15]);
          for(int32_t i=0; i<16; ++i)
          *(di--) = *(si++) + cset;
          }
          } break;
          */
    case 2: //vertical
    {
        /*
          dword *si = (dword*)unpackbuf;
          for(int32_t dy=15; dy>=0; --dy)
          {
          // 4 bytes at a time
          dword *di=&((dword*)dest->line[y+dy])[x>>2];
          for(int32_t i=0; i<16; i+=4)
          *(di++) = *(si++) + lcset;
          }
          */
        qword llcset = (((qword)cset)<<56)+(((qword)cset)<<48)+(((qword)cset)<<40)+(((qword)cset)<<32)+(((qword)cset)<<24)+(cset<<16)+(cset<<8)+cset;
        //      qword llcset = (((qword)cset)<<56)|(((qword)cset)<<48)|(((qword)cset)<<40)|(((qword)cset)<<32)|(((qword)cset)<<24)|(cset<<16)|(cset<<8)|cset;
        qword *si = (qword*)unpackbuf;
        
        for(int32_t dy=15; dy>=0; --dy)
        {
            // 4 bytes at a time
            //        qword *di=&((qword*)dest->line[y+dy])[x>>3];
            qword *di=(qword*)(dest->line[y+dy]+x);
            
            for(int32_t i=0; i<16; i+=8)
                *(di++) = *(si++) + llcset;
        }
    }
    break;
    
    /*
      case 3:
      {
      byte *si = unpackbuf;
      for(int32_t dy=15; dy>=0; --dy)
      {
      // 1 byte at a time
      byte *di = &(dest->line[y+dy][x+15]);
      for(int32_t i=0; i<16; ++i)
      *(di--) = *(si++) + cset;
      }
      } break;
      */
    default: //none or invalid
    {
        /*
          dword *si = (dword*)unpackbuf;
          for(int32_t dy=0; dy<16; ++dy)
          {
          // 4 bytes at a time
          dword *di=&((dword*)dest->line[y+dy])[x>>2];
          for(int32_t i=0; i<16; i+=4)
          *(di++) = *(si++) + lcset;
          }
          */
        qword llcset = (((qword)cset)<<56)+(((qword)cset)<<48)+(((qword)cset)<<40)+(((qword)cset)<<32)+(((qword)cset)<<24)+(cset<<16)+(cset<<8)+cset;
        //      qword llcset = (((qword)cset)<<56)|(((qword)cset)<<48)|(((qword)cset)<<40)|(((qword)cset)<<32)|(((qword)cset)<<24)|(cset<<16)|(cset<<8)|cset;
        qword *si = (qword*)unpackbuf;
        
        for(int32_t dy=0; dy<16; ++dy)
        {
            // 4 bytes at a time
            //        qword *di=&((qword*)dest->line[y+dy])[x>>3];
            qword *di=(qword*)(dest->line[y+dy]+x);
            
            for(int32_t i=0; i<16; i+=8)
                *(di++) = *(si++) + llcset;
        }
    }
    break;
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
    unpack_tile(newtilebuf, tile, flip&5, false);
    byte *si = unpackbuf;
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
void overtile16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip) //fixed
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
    unpack_tile(newtilebuf, tile, flip&5, false);
    byte *si = unpackbuf;
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
                        if(*si)
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
                    if(*si)
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
                        if(*si)
                            *di=*si+cset;
                            
                        ++di;
                    }
                    
                    ++si;
                }
            }
        }
    }
}

void putblock8(BITMAP *dest,int32_t tile,int32_t x,int32_t y,int32_t csets[],int32_t flip,int32_t mask)
{
    int32_t t[4];
    
    for(int32_t i=0; i<4; ++i)
        t[i]=tile+i;
        
    switch(mask)
    {
    case 1: //top-left quarter
        puttile8(dest,tile,x,y,csets[0],flip);
        break;
        
    case 3: //vertical
        if(flip&2)
        {
            zc_swap(t[0],t[1]);
        }
        
        puttile8(dest,t[0],x,y,  csets[0],flip);
        puttile8(dest,t[1],x,y+8,csets[1],flip);
        break;
        
    case 5: //horizontal
        if(flip&1)
        {
            zc_swap(t[0],t[1]);
        }
        
        puttile8(dest,t[0],x,  y,csets[0],flip);
        puttile8(dest,t[1],x+8,y,csets[1],flip);
        break;
        
    case 15: //all 4 quarters
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
        
        puttile8(dest,t[0],x,  y,  csets[0],flip);
        puttile8(dest,t[1],x+8,y,  csets[1],flip);
        puttile8(dest,t[2],x,  y+8,csets[2],flip);
        puttile8(dest,t[3],x+8,y+8,csets[3],flip);
        break;
    }
}

void oldputblock8(BITMAP *dest,int32_t tile,int32_t x,int32_t y,int32_t csets[],int32_t flip,int32_t mask)
{
    int32_t t[4];
    
    for(int32_t i=0; i<4; ++i)
        t[i]=tile+i;
        
    switch(mask)
    {
    case 1:
        oldputtile8(dest,tile,x,y,csets[0],flip);
        break;
        
    case 3:
        if(flip&2)
        {
            zc_swap(t[0],t[1]);
        }
        
        oldputtile8(dest,t[0],x,y,  csets[0],flip);
        oldputtile8(dest,t[1],x,y+8,csets[1],flip);
        break;
        
    case 5:
        if(flip&1)
        {
            zc_swap(t[0],t[1]);
        }
        
        oldputtile8(dest,t[0],x,  y,csets[0],flip);
        oldputtile8(dest,t[1],x+8,y,csets[1],flip);
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
        
        oldputtile8(dest,t[0],x,  y,  csets[0],flip);
        oldputtile8(dest,t[1],x+8,y,  csets[1],flip);
        oldputtile8(dest,t[2],x,  y+8,csets[2],flip);
        oldputtile8(dest,t[3],x+8,y+8,csets[3],flip);
        break;
    }
}

void overblock8(BITMAP *dest,int32_t tile,int32_t x,int32_t y,int32_t csets[],int32_t flip,int32_t mask)
{
    int32_t t[4];
    
    for(int32_t i=0; i<4; ++i)
        t[i]=tile+i;
        
    switch(mask)
    {
    case 1:
        overtile8(dest,tile,x,y,csets[0],flip);
        break;
        
    case 3:
        if(flip&2)
        {
            zc_swap(t[0],t[1]);
        }
        
        overtile8(dest,t[0],x,y,  csets[0],flip);
        overtile8(dest,t[1],x,y+8,csets[1],flip);
        break;
        
    case 5:
        if(flip&1)
        {
            zc_swap(t[0],t[1]);
        }
        
        overtile8(dest,t[0],x,  y,csets[0],flip);
        overtile8(dest,t[1],x+8,y,csets[1],flip);
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
        
        overtile8(dest,t[0],x,  y,  csets[0],flip);
        overtile8(dest,t[1],x+8,y,  csets[1],flip);
        overtile8(dest,t[2],x,  y+8,csets[2],flip);
        overtile8(dest,t[3],x+8,y+8,csets[3],flip);
        break;
    }
}

//  cmbdat: fffffsss cccccccc
//          (f:flags, s:cset, c:combo)

void putcombo(BITMAP* dest,int32_t x,int32_t y,int32_t cmbdat,int32_t cset)
{
    newcombo c = combobuf[cmbdat];
    int32_t drawtile=combo_tile(c, x, y);
    
    if(!(c.csets&0xF0) || !(c.csets&0x0F) || (newtilebuf[drawtile].format>tf4Bit))
        puttile16(dest,drawtile,x,y,cset,c.flip);
    //    puttile16(dest,c.drawtile,x,y,cset,c.flip);
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
        
        putblock8(dest,drawtile<<2,x,y,csets,c.flip,15);
        //    putblock8(dest,c.drawtile<<2,x,y,csets,c.flip,15);
    }
}

void oldputcombo(BITMAP* dest,int32_t x,int32_t y,int32_t cmbdat,int32_t cset)
{
    newcombo c = combobuf[cmbdat];
    int32_t drawtile=combo_tile(c, x, y);
    
    if(!(c.csets&0xF0) || !(c.csets&0x0F) || (newtilebuf[drawtile].format>tf4Bit))
        oldputtile16(dest,drawtile,x,y,cset,c.flip);
    //    oldputtile16(dest,c.drawtile,x,y,cset,c.flip);
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
        
        oldputblock8(dest,drawtile<<2,x,y,csets,c.flip,15);
        //    oldputblock8(dest,c.drawtile<<2,x,y,csets,c.flip,15);
    }
}

void overcombo(BITMAP* dest,int32_t x,int32_t y,int32_t cmbdat,int32_t cset)
{
    overcomboblock(dest, x, y, cmbdat, cset, 1, 1);
}

void overcomboblock(BITMAP *dest, int32_t x, int32_t y, int32_t cmbdat, int32_t cset, int32_t w, int32_t h)
{
    if ((unsigned)cmbdat >= MAXCOMBOS) return;
    newcombo c = combobuf[cmbdat];
    int32_t drawtile=combo_tile(c, x, y);
    
    for(int32_t woff = 0; woff < w; woff++)
    {
        for(int32_t hoff =0; hoff < h; hoff++)
        {
            int32_t tiletodraw = drawtile + 20*hoff+woff;
            
            // If this block goes past the edge of the tile page and
            // animation skip Y is used, skip rows accordingly
            if(tiletodraw%TILES_PER_ROW<woff) // Past the end?
                tiletodraw+=TILES_PER_ROW*combobuf[cmbdat].skipanimy;
                
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
                    
                overblock8(dest,tiletodraw<<2,x+16*woff,y+16*hoff,csets,c.flip,15);
            }
        }
    }
}

void overcombo2(BITMAP* dest,int32_t x,int32_t y,int32_t cmbdat,int32_t cset)
{
    if(cmbdat!=0)
    {
        overcombo(dest,x,y,cmbdat,cset);
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
		case tf8Bit:
		case tf4Bit:
			return (64<<format);
	}
	
	al_trace("Invalid tile format encountered.\n");
	
	// BUG: This is triggered by the 'grab' option, and certainly others as well.
	// if at any point a selected tile is 'blank' (newtilebuf[0] for example), then we have might a problem.
	// These should probably be dealt with where they really occur, however simply returning a
	// sane amount right now should fix any crashes associated with this. -Gleeok
	return 256;
}

int32_t comboa_lmasktotal(byte layermask)
{
    int32_t result=0;
    result+=layermask&1;
    result+=(layermask&2) >> 1;
    result+=(layermask&4) >> 2;
    result+=(layermask&8) >> 3;
    result+=(layermask&16) >> 4;
    result+=(layermask&32) >> 5;
    return result;
}

/* end of tiles.cc */

