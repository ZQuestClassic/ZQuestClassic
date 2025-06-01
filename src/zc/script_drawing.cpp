//! ritate_sprite_trans doesn't seem to be supported by or allegro header !?

//glibc 2.28 and later require this: -Z
#include <optional>
#include <utility>
#ifdef __GNUG__
	#define ALLEGRO_NO_FIX_ALIASES
#endif

#include "base/qrs.h"
#include "base/dmap.h"
#include "base/zdefs.h"
#include "base/zc_alleg.h"
#include "zc/script_drawing.h"
#include "zc/rendertarget.h"
#include "zc/maps.h"
#include "tiles.h"
#include "zc/zelda.h"
#include "zc/ffscript.h"
#include "base/util.h"
#include "subscr.h"
#include "drawing.h"
#include "base/mapscr.h"
#include "base/misctypes.h"
using namespace util;
extern refInfo *ri;
extern script_bitmaps scb;
#include <stdio.h>
#include <fstream>

#define DegtoFix(d)     ((d)*0.7111111111111)
#define RadtoFix(d)     ((d)*40.743665431525)

static int32_t secondary_draw_origin_xoff;
static int32_t secondary_draw_origin_yoff;

static std::optional<std::pair<int, int>> get_draw_origin_offset(DrawOrigin draw_origin, int draw_origin_target_uid, int xoff, int yoff)
{
	int xoffset;
	int yoffset;
	if (draw_origin == DrawOrigin::Region)
	{
		xoffset = xoff - viewport.x;
		yoffset = yoff - viewport.y;
	}
	else if (draw_origin == DrawOrigin::RegionScrollingNew)
	{
		xoffset = xoff + FFCore.ScrollingData[SCROLLDATA_NRX];
		yoffset = yoff + FFCore.ScrollingData[SCROLLDATA_NRY];
	}
	else if (draw_origin == DrawOrigin::PlayingField)
	{
		xoffset = xoff;
		yoffset = yoff;
	}
	else if (draw_origin == DrawOrigin::Screen)
	{
		xoffset = 0;
		yoffset = 0;
	}
	else if (draw_origin == DrawOrigin::Sprite)
	{
		sprite* draw_origin_target = sprite::getByUID(draw_origin_target_uid);
		if (!draw_origin_target)
		{
			Z_scripterrlog("Warning: Ignoring draw command using DRAW_ORIGIN_SPRITE with non-existent sprite uid: %d.\n", draw_origin_target_uid);
			return std::nullopt;
		}

		xoffset = xoff - viewport.x + draw_origin_target->x.getInt();
		yoffset = yoff - viewport.y + draw_origin_target->y.getInt();
	}
	else
	{
		// Unexpected.
		xoffset = 0;
		yoffset = 0;
	}

	return std::make_pair(xoffset, yoffset);
}

std::pair<int, bool> resolveScriptingBitmapId(int scripting_bitmap_id)
{
	if (scripting_bitmap_id < 0)
	{
		// Handles zscript values for RT_SCREEN, etc.
		return {scripting_bitmap_id / 10000, false};
	}
	else if (scripting_bitmap_id - 10 >= -2 && scripting_bitmap_id - 10 <= rtBMP6)
	{
		// Handles Game->LoadBitmapID, which sets the bitmap pointer as a "long" int.
		return {scripting_bitmap_id - 10, false};
	}
	else
	{
		// This is a user bitmap.
		return {scripting_bitmap_id, true};
	}
}

inline double sd_log2( double n )  
{  
    // log(n)/log(2) is log2.  
	double v = log( (double)n ) / log( (double)2 );  
    return v;
}  

inline bool isPowerOfTwo(int32_t n) 
{ 
   if(n==0) 
   return false; 
  
   return (ceil(sd_log2(n)) == floor(sd_log2(n))); 
} 



template<class T> inline
fixed degrees_to_fixed(T d)
{
    return ftofix(DegtoFix(d));
}
template<class T> inline
fixed radians_to_fixed(T d)
{
    return ftofix(RadtoFix(d));
}

BITMAP* ScriptDrawingBitmapPool::_parent_bmp = 0;

class TileHelper
{
public:

    static void OldPutTile(BITMAP* _Dest, int32_t tile, int32_t x, int32_t y, int32_t w, int32_t h, int32_t color, int32_t flip, byte skiprows=0)
    {
        // Past the end of the tile page?
        if(skiprows>0 && tile%TILES_PER_ROW+w>=TILES_PER_ROW)
        {
            byte w2=(tile+w)%TILES_PER_ROW;
            OldPutTile(_Dest, tile, x, y, w-w2, h, color, flip);
            OldPutTile(_Dest, tile+(w-w2)+(skiprows*TILES_PER_ROW), x+16*(w-w2), y, w2, h, color, flip);
            return;
        }
        
        switch(flip)
        {
        case 1:
            for(int32_t j=0; j<h; j++)
                for(int32_t k=w-1; k>=0; k--)
                    oldputtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+j*16, color, flip);
                    
            break;
            
        case 2:
            for(int32_t j=h-1; j>=0; j--)
                for(int32_t k=0; k<w; k++)
                    oldputtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+((h-1)-j)*16, color, flip);
                    
            break;
            
        case 3:
            for(int32_t j=h-1; j>=0; j--)
                for(int32_t k=w-1; k>=0; k--)
                    oldputtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+((h-1)-j)*16, color, flip);
                    
            break;
            
        case 0:
        default:
            for(int32_t j=0; j<h; j++)
                for(int32_t k=0; k<w; k++)
                    oldputtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+j*16, color, flip);
                    
            break;
        }
    }
    
    static void OverTile(BITMAP* _Dest, int32_t tile, int32_t x, int32_t y, int32_t w, int32_t h, int32_t color, int32_t flip, byte skiprows=0)
    {
		overtileblock16(_Dest,tile,x,y,w,h,color,flip,skiprows);
    }
	
	static void OverTileCloaked(BITMAP* _Dest, int32_t tile, int32_t x, int32_t y, int32_t w, int32_t h, int32_t flip, byte skiprows=0)
	{
		if(skiprows>0 && tile%TILES_PER_ROW+w>=TILES_PER_ROW)
		{
			byte w2=(tile+w)%TILES_PER_ROW;
			OverTileCloaked(_Dest, tile, x, y, w-w2, h, flip);
			OverTileCloaked(_Dest, tile+(w-w2)+(skiprows*TILES_PER_ROW), x+16*(w-w2), y, w2, h, flip);
			return;
		}
		
		switch(flip)
		{
			case 1:
				for(int32_t j=0; j<h; j++)
					for(int32_t k=w-1; k>=0; k--)
						overtilecloaked16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+j*16, flip);
						
				break;
				
			case 2:
				for(int32_t j=h-1; j>=0; j--)
					for(int32_t k=0; k<w; k++)
						overtilecloaked16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+((h-1)-j)*16, flip);
						
				break;
				
			case 3:
				for(int32_t j=h-1; j>=0; j--)
					for(int32_t k=w-1; k>=0; k--)
						overtilecloaked16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+((h-1)-j)*16, flip);
						
				break;
				
			default:
				for(int32_t j=0; j<h; j++)
					for(int32_t k=0; k<w; k++)
						overtilecloaked16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+j*16, flip);
						
				break;
		}
	}
    
    static void OverTileTranslucent(BITMAP* _Dest, int32_t tile, int32_t x, int32_t y, int32_t w, int32_t h, int32_t color, int32_t flip, int32_t opacity, byte skiprows=0)
    {
        if(skiprows>0 && tile%TILES_PER_ROW+w>=TILES_PER_ROW)
        {
            byte w2=(tile+w)%TILES_PER_ROW;
            OverTileTranslucent(_Dest, tile, x, y, w-w2, h, color, flip, opacity);
            OverTileTranslucent(_Dest, tile+(w-w2)+(skiprows*TILES_PER_ROW), x+16*(w-w2), y, w2, h, color, flip, opacity);
            return;
        }
        
        switch(flip)
        {
        case 1:
            for(int32_t j=0; j<h; j++)
                for(int32_t k=w-1; k>=0; k--)
                    overtiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+j*16, color, flip, opacity);
                    
            break;
            
        case 2:
            for(int32_t j=h-1; j>=0; j--)
                for(int32_t k=0; k<w; k++)
                    overtiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+((h-1)-j)*16, color, flip, opacity);
                    
            break;
            
        case 3:
            for(int32_t j=h-1; j>=0; j--)
                for(int32_t k=w-1; k>=0; k--)
                    overtiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+((h-1)-j)*16, color, flip, opacity);
                    
            break;
            
        default:
            for(int32_t j=0; j<h; j++)
                for(int32_t k=0; k<w; k++)
                    overtiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+j*16, color, flip, opacity);
                    
            break;
        }
    }
    
    static void PutTileTranslucent(BITMAP* _Dest, int32_t tile, int32_t x, int32_t y, int32_t w, int32_t h, int32_t color, int32_t flip, int32_t opacity, byte skiprows=0)
    {
        if(skiprows>0 && tile%TILES_PER_ROW+w>=TILES_PER_ROW)
        {
            byte w2=(tile+w)%TILES_PER_ROW;
            PutTileTranslucent(_Dest, tile, x, y, w-w2, h, color, flip, opacity);
            PutTileTranslucent(_Dest, tile+(w-w2)+(skiprows*TILES_PER_ROW), x+16*(w-w2), y, w2, h, color, flip, opacity);
            return;
        }
        
        switch(flip)
        {
        case 1:
            for(int32_t j=0; j<h; j++)
                for(int32_t k=w-1; k>=0; k--)
                    puttiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+j*16, color, flip, opacity);
                    
            break;
            
        case 2:
            for(int32_t j=h-1; j>=0; j--)
                for(int32_t k=0; k<w; k++)
                    puttiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+((h-1)-j)*16, color, flip, opacity);
                    
            break;
            
        case 3:
            for(int32_t j=h-1; j>=0; j--)
                for(int32_t k=w-1; k>=0; k--)
                    puttiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+((h-1)-j)*16, color, flip, opacity);
                    
            break;
            
        default:
            for(int32_t j=0; j<h; j++)
                for(int32_t k=0; k<w; k++)
                    puttiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+j*16, color, flip, opacity);
                    
            break;
        }
    }
};




void do_rectr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=x2
    //sdci[5]=y2
    //sdci[6]=color
    //sdci[7]=scale factor
    //sdci[8]=rotation anchor x
    //sdci[9]=rotation anchor y
    //sdci[10]=rotation angle
    //sdci[11]=fill
    //sdci[12]=opacity
    if(sdci[7]==0)  //scale
    {
        return;
    }
    
    int32_t x1=sdci[2]/10000;
    int32_t y1=sdci[3]/10000;
    int32_t x2=sdci[4]/10000;
    int32_t y2=sdci[5]/10000;
    
    if(x1>x2)
    {
        zc_swap(x1,x2);
    }
    
    if(y1>y2)
    {
        zc_swap(y1,y2);
    }
    
    if(sdci[7] != 10000)
    {
        int32_t w=x2-x1+1;
        int32_t h=y2-y1+1;
        int32_t w2=(w*sdci[7])/10000;
        int32_t h2=(h*sdci[7])/10000;
        x1=x1-((w2-w)/2);
        x2=x2+((w2-w)/2);
        y1=y1-((h2-h)/2);
        y2=y2+((h2-h)/2);
    }
    
    int32_t color=sdci[6]/10000;
    
    if(sdci[12]/10000<=127) //translucent
    {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    }
    
    if(sdci[10]==0) //no rotation
    {
        if(sdci[11]) //filled
        {
            rectfill(bmp, x1+xoffset, y1+yoffset, x2+xoffset, y2+yoffset, color);
        }
        else //outline
        {
            rect(bmp, x1+xoffset, y1+yoffset, x2+xoffset, y2+yoffset, color);
        }
    }
    else  //rotate
    {
        int32_t xy[16];
        int32_t rx=sdci[8]/10000;
        int32_t ry=sdci[9]/10000;
        fixed ra1=itofix(sdci[10]%10000)/10000;
        fixed ra2=itofix(sdci[10]/10000);
        fixed ra=ra1+ra2;
        ra = (ra/360)*256;
        
        fixed fcosa = fixcos(ra);
        fixed fsina = fixsin(ra);
        
        xy[ 0]=xoffset+rx + fixtoi((fcosa * (x1 - rx) - fsina * (y1 - ry)));     //x1
        xy[ 1]=yoffset+ry + fixtoi((fsina * (x1 - rx) + fcosa * (y1 - ry)));     //y1
        xy[ 2]=xoffset+rx + fixtoi((fcosa * (x2 - rx) - fsina * (y1 - ry)));     //x2
        xy[ 3]=yoffset+ry + fixtoi((fsina * (x2 - rx) + fcosa * (y1 - ry)));     //y1
        xy[ 4]=xoffset+rx + fixtoi((fcosa * (x2 - rx) - fsina * (y2 - ry)));     //x2
        xy[ 5]=yoffset+ry + fixtoi((fsina * (x2 - rx) + fcosa * (y2 - ry)));     //y2
        xy[ 6]=xoffset+rx + fixtoi((fcosa * (x1 - rx) - fsina * (y2 - ry)));     //x1
        xy[ 7]=yoffset+ry + fixtoi((fsina * (x1 - rx) + fcosa * (y2 - ry)));     //y2
        xy[ 8]=xoffset+rx + fixtoi((fcosa * (x1 - rx) - fsina * (y1 - ry + 1)));         //x1
        xy[ 9]=yoffset+ry + fixtoi((fsina * (x1 - rx) + fcosa * (y1 - ry + 1)));         //y1
        xy[10]=xoffset+rx + fixtoi((fcosa * (x2 - rx - 1) - fsina * (y1 - ry)));         //x2
        xy[11]=yoffset+ry + fixtoi((fsina * (x2 - rx - 1) + fcosa * (y1 - ry)));         //y1
        xy[12]=xoffset+rx + fixtoi((fcosa * (x2 - rx) - fsina * (y2 - ry - 1)));         //x2
        xy[13]=yoffset+ry + fixtoi((fsina * (x2 - rx) + fcosa * (y2 - ry - 1)));         //y2
        xy[14]=xoffset+rx + fixtoi((fcosa * (x1 - rx + 1) - fsina * (y2 - ry)));         //x1
        xy[15]=yoffset+ry + fixtoi((fsina * (x1 - rx + 1) + fcosa * (y2 - ry)));         //y2
        
        if(sdci[11]) //filled
        {
            polygon(bmp, 4, xy, color);
        }
        else //outline
        {
            line(bmp, xy[0], xy[1], xy[10], xy[11], color);
            line(bmp, xy[2], xy[3], xy[12], xy[13], color);
            line(bmp, xy[4], xy[5], xy[14], xy[15], color);
            line(bmp, xy[6], xy[7], xy[ 8], xy[ 9], color);
        }
    }
    
    drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}

void do_framer(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=tile
    //sdci[5]=cset
    //sdci[6]=width
    //sdci[7]=height
    //sdci[8]=overlay
    //sdci[9]=opacity
    
    int32_t x=sdci[2]/10000;
    int32_t y=sdci[3]/10000;
    
    int32_t tile=sdci[4]/10000;
    int32_t cs=sdci[5]/10000;
    int32_t w=sdci[6]/10000;
    int32_t h=sdci[7]/10000;
    bool overlay=sdci[8];
    bool trans=(sdci[9]/10000<=127);
    
	frame2x2(bmp, x + xoffset, y + yoffset, tile, cs, w, h, 0, overlay, trans);
}



void do_circler(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=radius
    //sdci[5]=color
    //sdci[6]=scale factor
    //sdci[7]=rotation anchor x
    //sdci[8]=rotation anchor y
    //sdci[9]=rotation angle
    //sdci[10]=fill
    //sdci[11]=opacity
    if(sdci[6]==0)  //scale
    {
        return;
    }
    
    int32_t x1=sdci[2]/10000;
    int32_t y1=sdci[3]/10000;
    qword r=sdci[4];
    
    if(sdci[6] != 10000)
    {
        r*=sdci[6];
        r/=10000;
    }
    
    r/=10000;
    int32_t color=sdci[5]/10000;
    
    if(sdci[11]/10000<=127) //translucent
    {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    }
    
    if(sdci[9]!=0&&(sdci[2]!=sdci[7]||sdci[3]!=sdci[8])) //rotation
    {
        int32_t xy[2];
        int32_t rx=sdci[7]/10000;
        int32_t ry=sdci[8]/10000;
        fixed ra1=itofix(sdci[9]%10000)/10000;
        fixed ra2=itofix(sdci[9]/10000);
        fixed ra=ra1+ra2;
        ra = (ra/360)*256;
        
        xy[ 0]=rx + fixtoi((fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry)));     //x1
        xy[ 1]=ry + fixtoi((fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry)));     //y1
        x1=xy[0];
        y1=xy[1];
    }
    
    if(sdci[10]) //filled
    {
        circlefill(bmp, x1+xoffset, y1+yoffset, r, color);
    }
    else //outline
    {
        circle(bmp, x1+xoffset, y1+yoffset, r, color);
    }
    
    drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}


void do_arcr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=radius
    //sdci[5]=start angle
    //sdci[6]=end angle
    //sdci[7]=color
    //sdci[8]=scale factor
    //sdci[9]=rotation anchor x
    //sdci[10]=rotation anchor y
    //sdci[11]=rotation angle
    //sdci[12]=closed
    //sdci[13]=fill
    //sdci[14]=opacity
    
    if(sdci[8]==0)  //scale
    {
        return;
    }
    
    int32_t cx=sdci[2]/10000;
    int32_t cy=sdci[3]/10000;
    qword r=sdci[4];
    
    if(sdci[8] != 10000)
    {
        r*=sdci[8];
        r/=10000;
    }
    
    r/=10000;
    
    int32_t color=sdci[7]/10000;
    
    fixed ra1=itofix(sdci[11]%10000)/10000;
    fixed ra2=itofix(sdci[11]/10000);
    fixed ra=ra1+ra2;
    ra = (ra/360)*256;
    
    
    fixed a1=itofix(sdci[5]%10000)/10000;
    fixed a2=itofix(sdci[5]/10000);
    fixed sa=a1+a2;
    sa = (sa/360)*256;
    
    a1=itofix(sdci[6]%10000)/10000;
    a2=itofix(sdci[6]/10000);
    fixed ea=a1+a2;
    ea = (ea/360)*256;
    
    if(sdci[11]!=0) //rotation
    {
        int32_t rx=sdci[9]/10000;
        int32_t ry=sdci[10]/10000;
        
        cx=rx + fixtoi((fixcos(ra) * (cx - rx) - fixsin(ra) * (cy - ry)));     //x1
        cy=ry + fixtoi((fixsin(ra) * (cx - rx) + fixcos(ra) * (cy - ry)));     //y1
        ea-=ra;
        sa-=ra;
    }
    
    int32_t fx=cx+fixtoi(fixcos(-(ea+sa)/2)*r/2);
    int32_t fy=cy+fixtoi(fixsin(-(ea+sa)/2)*r/2);
    
    if(sdci[12]) //closed
    {
        if(sdci[13]) //filled
        {
            clear_bitmap(prim_bmp);
            arc(prim_bmp, cx+xoffset, cy+yoffset, sa, ea, int32_t(r), color);
            line(prim_bmp, cx+xoffset, cy+yoffset, cx+xoffset+fixtoi(fixcos(-sa)*r), cy+yoffset+fixtoi(fixsin(-sa)*r), color);
            line(prim_bmp, cx+xoffset, cy+yoffset, cx+xoffset+fixtoi(fixcos(-ea)*r), cy+yoffset+fixtoi(fixsin(-ea)*r), color);
			int fillx = zc_max(0,fx)+xoffset;
			int filly = zc_max(0,fy)+yoffset;
			zprint2("Screen->Arc fill at prim_bmp (%d,%d) - 512x512\n", fillx, filly);
			floodfill(prim_bmp, fillx, filly, color);
            
            if(sdci[14]/10000<=127) //translucent
            {
                draw_trans_sprite(bmp, prim_bmp, 0,0);
            }
            else
            {
                draw_sprite(bmp, prim_bmp, 0,0);
            }
        }
        else
        {
            arc(bmp, cx+xoffset, cy+yoffset, sa, ea, int32_t(r), color);
            line(bmp, cx+xoffset, cy+yoffset, cx+xoffset+fixtoi(fixcos(-sa)*r), cy+yoffset+fixtoi(fixsin(-sa)*r), color);
            line(bmp, cx+xoffset, cy+yoffset, cx+xoffset+fixtoi(fixcos(-ea)*r), cy+yoffset+fixtoi(fixsin(-ea)*r), color);
        }
    }
    else
    {
        if(sdci[14]/10000<=127) //translucent
        {
            drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
        }
        
        arc(bmp, cx+xoffset, cy+yoffset, sa, ea, int32_t(r), color);
        drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
    }
}


void do_ellipser(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=radiusx
    //sdci[5]=radiusy
    //sdci[6]=color
    //sdci[7]=scale factor
    //sdci[8]=rotation anchor x
    //sdci[9]=rotation anchor y
    //sdci[10]=rotation angle
    //sdci[11]=fill
    //sdci[12]=opacity
    
    if(sdci[7]==0)  //scale
    {
        return;
    }
    
    int32_t x1=sdci[2]/10000;
    int32_t y1=sdci[3]/10000;
    int32_t radx=sdci[4]/10000;
    radx*=sdci[7]/10000;
    int32_t rady=sdci[5]/10000;
    rady*=sdci[7]/10000;
    int32_t color=sdci[6]/10000;
    float rotation = sdci[10]/10000;
    
    int32_t rx=sdci[8]/10000;
    int32_t ry=sdci[9]/10000;
    fixed ra1=itofix(sdci[10]%10000)/10000;
    fixed ra2=itofix(sdci[10]/10000);
    fixed ra=ra1+ra2;
    ra = (ra/360)*256;
    
    int32_t xy[2];
    xy[ 0]=rx + fixtoi((fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry)));     //x1
    xy[ 1]=ry + fixtoi((fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry)));     //y1
    x1=xy[0];
    y1=xy[1];
    
    if(radx<1||rady<1||radx>255||rady>255) return;
    
    BITMAP* bitty = script_drawing_commands.AquireSubBitmap(radx*2+1, rady*2+1);
    
    if(sdci[11]) //filled
    {
    
        if(sdci[12]/10000<128) //translucent
        {
            clear_bitmap(prim_bmp);
            ellipsefill(bitty, radx, rady, radx, rady, color==0?255:color);
            rotate_sprite(prim_bmp, bitty, x1+xoffset-radx,y1+yoffset-rady, degrees_to_fixed(rotation));
            draw_trans_sprite(bmp, prim_bmp, 0, 0);
        }
        else // no opacity
        {
            ellipsefill(bitty, radx, rady, radx, rady, color==0?255:color);
            rotate_sprite(bmp, bitty, x1+xoffset-radx,y1+yoffset-rady, degrees_to_fixed(rotation));
        }
    }
    else //not filled
    {
        if(sdci[12]/10000<128) //translucent
        {
            clear_bitmap(prim_bmp);
            ellipse(bitty, radx, rady, radx, rady, color==0?255:color);
            rotate_sprite(prim_bmp, bitty, x1+xoffset-radx,y1+yoffset-rady, degrees_to_fixed(rotation));
            draw_trans_sprite(bmp, prim_bmp, 0, 0);
        }
        else // no opacity
        {
            ellipse(bitty, radx, rady, radx, rady, color==0?255:color);
            rotate_sprite(bmp, bitty, x1+xoffset-radx,y1+yoffset-rady, degrees_to_fixed(rotation));
        }
    }
    
    // Since 0 is the transparent color, the stuff above will fail if the ellipse color is also 0.
    // Instead, it uses color 255 and replaces it afterward. That'll also screw up color 255 around
    // the ellipse, but it shouldn't be used anyway.
    if(color==0)
    {
        // This is very slow, so check the smallest possible square
        int32_t endx=zc_min(bmp->w-1, x1+zc_max(radx, rady));
        int32_t endy=zc_min(bmp->h-1, y1+zc_max(radx, rady));
        
        for(int32_t y=zc_max(0, y1-zc_max(radx, rady)); y<=endy; y++)
            for(int32_t x=zc_max(0, x1-zc_max(radx, rady)); x<=endx; x++)
                if(getpixel(bmp, x, y)==255)
                    putpixel(bmp, x, y, 0);
    }
    
    script_drawing_commands.ReleaseSubBitmap(bitty);
}


void do_liner(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=x2
    //sdci[5]=y2
    //sdci[6]=color
    //sdci[7]=scale factor
    //sdci[8]=rotation anchor x
    //sdci[9]=rotation anchor y
    //sdci[10]=rotation angle
    //sdci[11]=opacity
    if(sdci[7]==0)  //scale
    {
        return;
    }
    
    int32_t x1=sdci[2]/10000;
    int32_t y1=sdci[3]/10000;
    int32_t x2=sdci[4]/10000;
    int32_t y2=sdci[5]/10000;
    
    if(sdci[7] != 10000)
    {
        int32_t w=x2-x1+1;
        int32_t h=y2-y1+1;
        int32_t w2=int32_t(w*((double)sdci[7]/10000.0));
        int32_t h2=int32_t(h*((double)sdci[7]/10000.0));
        x1=x1-((w2-w)/2);
        x2=x2+((w2-w)/2);
        y1=y1-((h2-h)/2);
        y2=y2+((h2-h)/2);
    }
    
    int32_t color=sdci[6]/10000;
    
    if(sdci[11]/10000<=127) //translucent
    {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    }
    
    if(sdci[10]!=0) //rotation
    {
        int32_t xy[4];
        int32_t rx=sdci[8]/10000;
        int32_t ry=sdci[9]/10000;
        fixed ra1=itofix(sdci[10]%10000)/10000;
        fixed ra2=itofix(sdci[10]/10000);
        fixed ra=ra1+ra2;
        
        xy[ 0]=rx + fixtoi((fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry)));     //x1
        xy[ 1]=ry + fixtoi((fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry)));     //y1
        xy[ 2]=rx + fixtoi((fixcos(ra) * (x2 - rx) - fixsin(ra) * (y2 - ry)));     //x2
        xy[ 3]=ry + fixtoi((fixsin(ra) * (x2 - rx) + fixcos(ra) * (y2 - ry)));     //y2
        x1=xy[0];
        y1=xy[1];
        x2=xy[2];
        y2=xy[3];
    }
    
    line(bmp, x1+xoffset, y1+yoffset, x2+xoffset, y2+yoffset, color);
    drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}

void do_linesr(BITMAP *bmp, int32_t i, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=array[10] = { x, y, x2, y2, colour, scale, rx, ry, angle, opacity }
	
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=x2
    //sdci[5]=y2
    //sdci[6]=color
    //sdci[7]=scale factor
    //sdci[8]=rotation anchor x
    //sdci[9]=rotation anchor y
    //sdci[10]=rotation angle
    //sdci[11]=opacity
    //if(sdci[7]==0)  //scale
    //{
    //    return;
    //}
    
    std::vector<int32_t>* v_ptr = (std::vector<int32_t>*)script_drawing_commands[i].GetPtr();
    
    if(!v_ptr)
    {
        al_trace("Screen->PutPixels: Vector pointer is null! Internal error. \n");
        return;
    }
    
    std::vector<int32_t> &v = *v_ptr;
    
    if(v.empty())
        return;
    
    int32_t* pos = &v[0];
    int32_t sz = v.size();
    
    for ( int32_t q = 0; q < sz; q+=10 )
    {
	
	    int32_t x1 = v.at(q);
	    //Z_scripterrlog("Lines( x1 ) is: %d\n", x1);
	    int32_t y1 = v.at(q+1);
	    //Z_scripterrlog("Lines( x2 ) is: %d\n", y1);
	    int32_t x2 = v.at(q+2);
	    //Z_scripterrlog("Lines( x2 ) is: %d\n", x2);
	    int32_t y2 = v.at(q+3);
	    //Z_scripterrlog("Lines( y2 ) is: %d\n", y2);
	    int32_t color  = v.at(q+4);
	    //Z_scripterrlog("Lines( colour ) is: %d\n", color);
	    //Z_scripterrlog("Lines( scale ) is: %d\n", v.at(q+5));
	    if (v.at(q+5) == 0) { Z_scripterrlog("Lines() aborting due to scale\n"); return; }//scale
	    
	    if( v.at(q+5) != 10000)
	    {
		int32_t w=x2-x1+1;
		int32_t h=y2-y1+1;
		int32_t w2=int32_t(w*((double)v.at(q+5)));
		int32_t h2=int32_t(h*((double)v.at(q+5)));
		x1=x1-((w2-w)/2);
		x2=x2+((w2-w)/2);
		y1=y1-((h2-h)/2);
		y2=y2+((h2-h)/2);
	    }
	    
	    
	    //Z_scripterrlog("Lines( opacity ) is: %d\n", v.at(q+9));
	    if(v.at(q+9) <= 127) //translucent
	    {
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	    }
	    else drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
	    //Z_scripterrlog("Lines( rotation ) is: %d\n", v.at(q+8));
	    //Z_scripterrlog("Lines( rot_x ) is: %d\n", v.at(q+6));
	    //Z_scripterrlog("Lines( rot_x ) is: %d\n", v.at(q+7));
	    if( v.at(q+8) !=0 ) //rotation
	    {
		int32_t xy[4];
		    
		int32_t rx = v.at(q+6);
		    
		int32_t ry = v.at(q+7);
		    
		fixed ra1=itofix(v.at(q+8) % 1);
		fixed ra2=itofix(v.at(q+8));
		fixed ra=ra1+ra2;
		
		xy[ 0]=rx + fixtoi((fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry)));     //x1
		xy[ 1]=ry + fixtoi((fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry)));     //y1
		xy[ 2]=rx + fixtoi((fixcos(ra) * (x2 - rx) - fixsin(ra) * (y2 - ry)));     //x2
		xy[ 3]=ry + fixtoi((fixsin(ra) * (x2 - rx) + fixcos(ra) * (y2 - ry)));     //y2
		x1=xy[0];
		y1=xy[1];
		x2=xy[2];
		y2=xy[3];
	    }
	    //Z_scripterrlog("Lines( xofs ) is: %d\n", xoffset);
	    //Z_scripterrlog("Lines( yofs ) is: %d\n", yoffset);
	    line(bmp, x1+xoffset, y1+yoffset, x2+xoffset, y2+yoffset, color);
    }
    drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}

void do_polygonr(BITMAP *bmp, int32_t i, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=point count
	//sdci[3]array[]
	//sdci[4] = colour
	//sdci[5] = opacity

	int32_t col = sdci[4]/10000;
	int32_t op = sdci[5]/10000;
    
    std::vector<int32_t>* v_ptr = (std::vector<int32_t>*)script_drawing_commands[i].GetPtr();
    
    if(!v_ptr)
    {
        al_trace("Screen->Polygon: Vector pointer is null! Internal error. \n");
        return;
    }
    
    std::vector<int32_t> &v = *v_ptr;
    
    if(v.empty())
        return;
    
    int32_t* pos = &v[0];
    int32_t sz = v.size();
	int32_t numpoints = (sdci[2]/10000);
	if(sz & 1) --sz; //even amount only
	if(numpoints > sz/2) //cap to array
		numpoints = sz/2;
	if(numpoints < 1)
		return; //Don't draw 0 or negative point count

	for (int32_t i = 0; i < sz; i += 2)
	{
		pos[i] += xoffset;
		pos[i + 1] += yoffset;
	}

	if(op <= 127) //translucent
	{
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	}
	else drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
	
	polygon(bmp, numpoints, (int32_t*)pos, col);
	drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}

void bmp_do_polygonr(BITMAP *bmp, int32_t i, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=point count
	//sdci[3]array[]
	//sdci[4] = colour
	//sdci[5] = opacity

	int32_t col = sdci[4]/10000;
	int32_t op = sdci[5]/10000;
	
	if ( sdci[DRAWCMD_BMP_TARGET] <= 0 ) 
	{
		Z_scripterrlog("bitmap->Polygon() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
		return;
	}
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
    
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
    
    std::vector<int32_t>* v_ptr = (std::vector<int32_t>*)script_drawing_commands[i].GetPtr();
    
    if(!v_ptr)
    {
        al_trace("bitmap->Polygon: Vector pointer is null! Internal error. \n");
        return;
    }
    
    std::vector<int32_t> &v = *v_ptr;
    
    if(v.empty())
        return;
    
    int32_t* pos = &v[0];
    int32_t sz = v.size();
	int32_t numpoints = (sdci[2]/10000);
	if(sz & 1) --sz; //even amount only
	if(numpoints > sz/2) //cap to array
		numpoints = sz/2;
	if(numpoints < 1)
		return; //Don't draw 0 or negative point count

	for (int32_t i = 0; i < sz; i += 2)
	{
		pos[i] += xoffset;
		pos[i + 1] += yoffset;
	}

	if(op <= 127) //translucent
	{
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	}
	else drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
	
	polygon(refbmp, numpoints, (int32_t*)pos, col);
	drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}

void do_spliner(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    /* layer, x1, y1, x2, y2, x3, y3, x4, y4, color, opacity */
    
    int32_t points[8] = {    xoffset + (sdci[2]/10000), yoffset + (sdci[3]/10000),
                         xoffset + (sdci[4]/10000), yoffset + (sdci[5]/10000),
                         xoffset + (sdci[6]/10000), yoffset + (sdci[7]/10000),
                         xoffset + (sdci[8]/10000), yoffset + (sdci[9]/10000)
                    };
                    
    if(sdci[11]/10000 < 128)   //translucent
    {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    }
    
    spline(bmp, points, sdci[10]/10000);
    
    drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}


void do_putpixelr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=color
    //sdci[5]=rotation anchor x
    //sdci[6]=rotation anchor y
    //sdci[7]=rotation angle
    //sdci[8]=opacity
    int32_t x1=sdci[2]/10000;
    int32_t y1=sdci[3]/10000;
    int32_t color=sdci[4]/10000;
    
    if(sdci[8]/10000<=127) //translucent
    {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    }
    
    if(sdci[7]!=0) //rotation
    {
        int32_t xy[2];
        int32_t rx=sdci[5]/10000;
        int32_t ry=sdci[6]/10000;
        fixed ra1=itofix(sdci[7]%10000)/10000;
        fixed ra2=itofix(sdci[7]/10000);
        fixed ra=ra1+ra2;
        
        xy[ 0]=rx + fixtoi((fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry)));     //x1
        xy[ 1]=ry + fixtoi((fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry)));     //y1
        x1=xy[0];
        y1=xy[1];
    }
    
    putpixel(bmp, x1+xoffset, y1+yoffset, color);
    drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}

void do_putpixelsr(BITMAP *bmp, int32_t i, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	//Z_scripterrlog("Starting putpixels()%s\n");
    //sdci[1]=layer
    //sdci[2]=array {x,y,colour,opacity}
    //sdci[3]=rotation anchor x
    //sdci[4]=rotation anchor y
    //sdci[5]=rotation angle
	
	
    std::vector<int32_t>* v_ptr = (std::vector<int32_t>*)script_drawing_commands[i].GetPtr();
    
    if(!v_ptr)
    {
        al_trace("Screen->PutPixels: Vector pointer is null! Internal error. \n");
        return;
    }
    
    std::vector<int32_t> &v = *v_ptr;
    
    if(v.empty())
        return;
    
    int32_t* pos = &v[0];
    int32_t sz = v.size();
    
    
	int32_t x1 = 0;
	int32_t y1 = 0;
    
    for ( int32_t q = 0; q < sz; q+=4 )
    {
	    x1 = v.at(q); //pos[q];
	    y1 = v.at(q+1); //pos[q+1];
	    if(sdci[5]!=0) //rotation
	    {
		int32_t xy[2];
		int32_t rx=sdci[3]/10000;
		int32_t ry=sdci[4]/10000;
		fixed ra1=itofix(sdci[5]%10000)/10000;
		fixed ra2=itofix(sdci[5]/10000);
		fixed ra=ra1+ra2;
		
		xy[ 0]=rx + fixtoi((fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry)));     //x1
		xy[ 1]=ry + fixtoi((fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry)));     //y1
		x1=xy[0];
		y1=xy[1];
	    }
	    if ( v.at(q+3) /*pos[q+3]*/ < 128 ) drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	    else drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
	    putpixel(bmp, x1+xoffset, y1+yoffset, v.at(q+2) /*pos[q+2]*/);
    }
    drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}

void do_drawtiler(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=tile
    //sdci[5]=tile width
    //sdci[6]=tile height
    //sdci[7]=color (cset)
    //sdci[8]=scale x
    //sdci[9]=scale y
    //sdci[10]=rotation anchor x
    //sdci[11]=rotation anchor y
    //sdci[12]=rotation angle
    //sdci[13]=flip
    //sdci[14]=transparency
    //sdci[15]=opacity
    
    int32_t w = sdci[5]/10000;
    int32_t h = sdci[6]/10000;
    
    if(w < 1 || h < 1 || h > 20 || w > 20)
    {
        return;
    }
    
    int32_t xscale=sdci[8]/10000;
    int32_t yscale=sdci[9]/10000;
    int32_t rx = sdci[10]/10000;
    int32_t ry = sdci[11]/10000;
    float rotation=sdci[12]/10000;
    int32_t flip=(sdci[13]/10000)&3;
    bool transparency=sdci[14]!=0;
    int32_t opacity=sdci[15]/10000;
    int32_t color=sdci[7]/10000;
    
    int32_t x1=sdci[2]/10000;
    int32_t y1=sdci[3]/10000;
    
    //don't scale if it's not safe to do so
    bool canscale = true;
    
    if(xscale==0||yscale==0)
    {
        return;
    }
    
    if(xscale<=0||yscale<=0)
        canscale = false; //default size
        
    if((xscale>0 && yscale>0) || rotation)   //scaled or rotated
    {
        BITMAP* pbitty = script_drawing_commands.AquireSubBitmap(w*16, h*16);
        
        if(transparency) //transparency
        {
            TileHelper::OverTile(pbitty, (sdci[4]/10000), 0, 0, w, h, color, flip);
        }
        else //no transparency
        {
            TileHelper::OldPutTile(pbitty, (sdci[4]/10000), 0, 0, w, h, color, flip);
        }
        
        if(rotation != 0)
        {
            //low negative values indicate no anchor-point rotation
            if(rx>-777||ry>-777)
            {
                int32_t xy[2];
                fixed ra1=itofix(sdci[12]%10000)/10000;
                fixed ra2=itofix(sdci[12]/10000);
                fixed ra=ra1+ra2;
                xy[ 0]=rx + fixtoi((fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry)));     //x1
                xy[ 1]=ry + fixtoi((fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry)));     //y1
                x1=xy[0];
                y1=xy[1];
            }
            
            if(canscale) //scale first
            {
                //damnit all, .. fixme.
                BITMAP* tempbit = create_bitmap_ex(8, xscale>512?512:xscale, yscale>512?512:yscale);
                clear_bitmap(tempbit);
                
                stretch_sprite(tempbit, pbitty, 0, 0, xscale, yscale);
                
                if(opacity < 128)
                {
                    clear_bitmap(prim_bmp);
                    rotate_sprite(prim_bmp, tempbit, 0, 0, degrees_to_fixed(rotation));
                    draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
                }
                else
                {
                    rotate_sprite(bmp, tempbit, x1+xoffset, y1+yoffset, degrees_to_fixed(rotation));
                }
                
                destroy_bitmap(tempbit);
            }
            else //no scale
            {
                if(opacity < 128)
                {
                    clear_bitmap(prim_bmp);
                    rotate_sprite(prim_bmp, pbitty, 0, 0, degrees_to_fixed(rotation));
                    draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
                }
                else
                {
                    rotate_sprite(bmp, pbitty, x1+xoffset, y1+yoffset, degrees_to_fixed(rotation));
                }
            }
        }
        else //scale only
        {
            if(canscale)
            {
                if(opacity<128)
                {
                    clear_bitmap(prim_bmp);
                    stretch_sprite(prim_bmp, pbitty, 0, 0, xscale, yscale);
                    draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
                }
                else
                {
                    stretch_sprite(bmp, pbitty, x1+xoffset, y1+yoffset, xscale, yscale);
                }
            }
            else //error -do not scale
            {
                if(opacity<128)
                {
                    draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
                }
                else
                {
                    draw_sprite(bmp, pbitty, x1+xoffset, y1+yoffset);
                }
            }
        }
        
        script_drawing_commands.ReleaseSubBitmap(pbitty);
        
    }
    else // no scale or rotation
    {
        if(transparency)
        {
            if(opacity<=127)
                TileHelper::OverTileTranslucent(bmp, (sdci[4]/10000), xoffset+x1, yoffset+y1, w, h, color, flip, opacity);
            else
                TileHelper::OverTile(bmp, (sdci[4]/10000), xoffset+x1, yoffset+y1, w, h, color, flip);
        }
        else
        {
            if(opacity<=127)
                TileHelper::PutTileTranslucent(bmp, (sdci[4]/10000), xoffset+x1, yoffset+y1, w, h, color, flip, opacity);
            else
                TileHelper::OldPutTile(bmp, (sdci[4]/10000), xoffset+x1, yoffset+y1, w, h, color, flip);
        }
    }
}

void do_drawtilecloakedr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	//sdci[1]=layer
	//sdci[2]=x
	//sdci[3]=y
	//sdci[4]=tile
	//sdci[5]=tile width
	//sdci[6]=tile height
	//sdci[7]=flip
	
	int32_t w = sdci[5]/10000;
	int32_t h = sdci[6]/10000;
	
	if(w < 1 || h < 1 || h > 20 || w > 20)
	{
		return;
	}
	
	int32_t flip=(sdci[7]/10000)&3;
	
	int32_t x1=sdci[2]/10000;
	int32_t y1=sdci[3]/10000;
	
	TileHelper::OverTileCloaked(bmp, (sdci[4]/10000), xoffset+x1, yoffset+y1, w, h, flip);
}


void do_drawcombor(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=combo
    //sdci[5]=tile width
    //sdci[6]=tile height
    //sdci[7]=color (cset)
    //sdci[8]=scale x
    //sdci[9]=scale y
    //sdci[10]=rotation anchor x
    //sdci[11]=rotation anchor y
    //sdci[12]=rotation angle
    //sdci[13]=frame
    //sdci[14]=flip
    //sdci[15]=transparency
    //sdci[16]=opacity
    
    int32_t w = sdci[5]/10000;
    int32_t h = sdci[6]/10000;
    
    if(w<1||h<1||h>20||w>20)
    {
        return;
    }
    int32_t cmb = (sdci[4]/10000);
	if((unsigned)cmb >= MAXCOMBOS)
	{
		Z_scripterrlog("DrawCombo() cannot draw combo '%d', as it is out of bounds.\n", cmb);
		return;
	}
	
    int32_t xscale=sdci[8]/10000;
    int32_t yscale=sdci[9]/10000;
    int32_t rx = sdci[10]/10000; //these work now
    int32_t ry = sdci[11]/10000; //these work now
    float rotation=sdci[12]/10000;
    
    bool transparency=sdci[15]!=0;
    int32_t opacity=sdci[16]/10000;
    int32_t color=sdci[7]/10000;
    int32_t x1=sdci[2]/10000;
    int32_t y1=sdci[3]/10000;
    
    auto& c = GET_DRAWING_COMBO(cmb);
    int32_t tiletodraw = combo_tile(c, x1, y1);
    int32_t flip = ((sdci[14]/10000) & 3) ^ c.flip;
    int32_t skiprows=c.skipanimy;
    
    
    //don't scale if it's not safe to do so
    bool canscale = true;
    
    if(xscale==0||yscale==0)
    {
        return;
    }
    
    if(xscale<=0||yscale<=0)
        canscale = false; //default size
        
    if((xscale>0 && yscale>0) || rotation)   //scaled or rotated
    {
        BITMAP* pbitty = script_drawing_commands.AquireSubBitmap(w*16, h*16);   //-pbitty in the hisouse. :D
        
        if(transparency)
        {
            TileHelper::OverTile(pbitty, tiletodraw, 0, 0, w, h, color, flip, skiprows);
        }
        else //no transparency
        {
            TileHelper::OldPutTile(pbitty, tiletodraw, 0, 0, w, h, color, flip, skiprows);
        }
        
        if(rotation != 0) // rotate
        {
            //fixed point sucks ;0
            if(rx>-777||ry>-777) //set the rotation anchor and rotate around that
            {
                int32_t xy[2];
                fixed ra1=itofix(sdci[12]%10000)/10000;
                fixed ra2=itofix(sdci[12]/10000);
                fixed ra=ra1+ra2;
                xy[ 0]=rx + fixtoi((fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry)));     //x1
                xy[ 1]=ry + fixtoi((fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry)));     //y1
                x1=xy[0];
                y1=xy[1];
            }
            
            if(canscale) //scale first
            {
                BITMAP* tempbit = create_bitmap_ex(8, xscale>512?512:xscale, yscale>512?512:yscale);
                clear_bitmap(tempbit);
                
                stretch_sprite(tempbit, pbitty, 0, 0, xscale, yscale);
                
                if(opacity < 128)
                {
                    clear_bitmap(prim_bmp);
                    rotate_sprite(prim_bmp, tempbit, 0, 0, degrees_to_fixed(rotation));
                    draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
                }
                else
                {
                    rotate_sprite(bmp, tempbit, x1+xoffset, y1+yoffset, degrees_to_fixed(rotation));
                }
                
                destroy_bitmap(tempbit);
            }
            else //no scale
            {
                if(opacity < 128)
                {
                    clear_bitmap(prim_bmp);
                    rotate_sprite(prim_bmp, pbitty, 0, 0, degrees_to_fixed(rotation));
                    draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
                }
                else
                {
                    rotate_sprite(bmp, pbitty, x1+xoffset, y1+yoffset, degrees_to_fixed(rotation));
                }
            }
        }
        else //scale only
        {
            if(canscale)
            {
                if(opacity<128)
                {
                    clear_bitmap(prim_bmp);
                    stretch_sprite(prim_bmp, pbitty, 0, 0, xscale, yscale);
                    draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
                }
                else
                {
                    stretch_sprite(bmp, pbitty, x1+xoffset, y1+yoffset, xscale, yscale);
                }
            }
            else //error -do not scale
            {
                if(opacity<128)
                {
                    draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
                }
                else
                {
                    draw_sprite(bmp, pbitty, x1+xoffset, y1+yoffset);
                }
            }
        }
        
        script_drawing_commands.ReleaseSubBitmap(pbitty); //rap sucks
    }
    else // no scale or rotation
    {
        if(transparency)
        {
            if(opacity<=127)
                TileHelper::OverTileTranslucent(bmp, tiletodraw, xoffset+x1, yoffset+y1, w, h, color, flip, opacity, skiprows);
            else
                TileHelper::OverTile(bmp, tiletodraw, xoffset+x1, yoffset+y1, w, h, color, flip, skiprows);
        }
        else
        {
            if(opacity<=127)
                TileHelper::PutTileTranslucent(bmp, tiletodraw, xoffset+x1, yoffset+y1, w, h, color, flip, opacity, skiprows);
            else
                TileHelper::OldPutTile(bmp, tiletodraw, xoffset+x1, yoffset+y1, w, h, color, flip, skiprows);
        }
    }
}

void do_drawcombocloakedr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	//sdci[1]=layer
	//sdci[2]=x
	//sdci[3]=y
	//sdci[4]=combo
	//sdci[5]=tile width
	//sdci[6]=tile height
	//sdci[7]=flip
	
	int32_t w = sdci[5]/10000;
	int32_t h = sdci[6]/10000;
	
	if(w<1||h<1||h>20||w>20)
	{
		return;
	}
    int32_t cmb = (sdci[4]/10000);
	if((unsigned)cmb >= MAXCOMBOS)
	{
		Z_scripterrlog("DrawComboCloaked() cannot draw combo '%d', as it is out of bounds.\n", cmb);
		return;
	}
	
	int32_t x1=sdci[2]/10000;
	int32_t y1=sdci[3]/10000;
	
	auto& c = GET_DRAWING_COMBO(cmb);
	int32_t tiletodraw = combo_tile(c, x1, y1);
	int32_t flip = ((sdci[7]/10000) & 3) ^ c.flip;
	int32_t skiprows=c.skipanimy;
	
	TileHelper::OverTileCloaked(bmp, tiletodraw, xoffset+x1, yoffset+y1, w, h, flip, skiprows);
}


void do_fasttiler(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    /* layer, x, y, tile, color opacity */
    
    int32_t opacity = sdci[6]/10000;
	int x = xoffset+(sdci[2]/10000);
	int y = yoffset+(sdci[3]/10000);
    
    if(opacity < 128)
        overtiletranslucent16(bmp, sdci[4]/10000, x, y, sdci[5]/10000, 0, opacity);
    else
        overtile16(bmp, sdci[4]/10000, x, y, sdci[5]/10000, 0);
}

void do_fasttilesr(BITMAP *bmp, int32_t i, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    /* layer, x, y, tile, color opacity */
    
    //sdci[1]=layer
    //sdci[2]=array {x,y,tile,colour,opacity}
    
    std::vector<int32_t>* v_ptr = (std::vector<int32_t>*)script_drawing_commands[i].GetPtr();
    
    if(!v_ptr)
    {
        al_trace("Screen->PutPixels: Vector pointer is null! Internal error. \n");
        return;
    }
    
    std::vector<int32_t> &v = *v_ptr;
    
    if(v.empty())
        return;
    
    int32_t* pos = &v[0];
    int32_t sz = v.size();
    
    for ( int32_t q = 0; q < sz; q+=5 )
    {
	    
	    if(v.at(q+4) < 128)
		overtiletranslucent16(bmp, v.at(q+2), xoffset+(v.at(q)), yoffset+(v.at(q+1)), v.at(q+3), 0, v.at(q+4));
	    else
		overtile16(bmp, v.at(q+2), xoffset+(v.at(q)), yoffset+(v.at(q+1)), v.at(q+3), 0);
    }
}



void do_fastcombor(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    /* layer, x, y, tile, color opacity */
    
    int32_t opacity = sdci[6] / 10000;
    int32_t x1 = sdci[2] / 10000;
    int32_t y1 = sdci[3] / 10000;
    
    int32_t cmb = (sdci[4]/10000);
	if((unsigned)cmb >= MAXCOMBOS)
	{
		Z_scripterrlog("FastCombo() cannot draw combo '%d', as it is out of bounds.\n", cmb);
		return;
	}

	int x = xoffset+x1;
	int y = yoffset+y1;
	
	if(opacity < 128)
	{
		overcomboblocktranslucent(bmp, x, y, cmb, sdci[5]/10000, 1, 1, 128);
	}
	else
	{
		overcomboblock(bmp, x, y, cmb, sdci[5]/10000, 1, 1);
	}
}

void do_fastcombosr(BITMAP *bmp, int32_t i, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	/* layer, x, y, combo, cset, opacity */
	std::vector<int32_t>* v_ptr = (std::vector<int32_t>*)script_drawing_commands[i].GetPtr();
	
	if(!v_ptr)
	{
		al_trace("Screen->FastCombos: Vector pointer is null! Internal error. \n");
		return;
	}
	
	std::vector<int32_t> &v = *v_ptr;
	
	if(v.empty())
		return;
	
	int32_t* pos = &v[0];
	int32_t sz = v.size();
	
	for ( int32_t q = 0; q < sz; q+=5 )
	{
		if((unsigned)(v.at(q+2)) >= MAXCOMBOS)
		{
			Z_scripterrlog("FastCombos() cannot draw combo '%d', as it is out of bounds.\n", v.at(q+2));
			continue;
		}
		if(v.at(q+4) < 128)
		{
			overcomboblocktranslucent(bmp, xoffset+v.at(q), yoffset+v.at(q+1), v.at(q+2), v.at(q+3), 1, 1, 128);

		}
		else
		{
			overcomboblock(bmp, xoffset+v.at(q), yoffset+v.at(q+1), v.at(q+2), v.at(q+3), 1, 1);
		}
	}
}




void do_drawcharr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	//broken 2.50.2 and earlier drawcharacter()
	if ( get_qr(qr_BROKENCHARINTDRAWING) )
	{
		//sdci[1]=layer
		    //sdci[2]=x
		    //sdci[3]=y
		    //sdci[4]=font
		    //sdci[5]=color
		    //sdci[6]=bg color
		    //sdci[7]=strech x (width)
		    //sdci[8]=stretch y (height)
		    //sdci[9]=char
		    //sdci[10]=opacity
		    
		    int32_t x=sdci[2]/10000;
		    int32_t y=sdci[3]/10000;
		    int32_t font_index=sdci[4]/10000;
		    int32_t color=sdci[5]/10000;
		    int32_t bg_color=sdci[6]/10000; //-1 = transparent
		    int32_t w=sdci[7]/10000;
		    int32_t h=sdci[8]/10000;
		    char glyph=char(sdci[9]/10000);
		    int32_t opacity=sdci[10]/10000;
		    
		    //safe check
		    if(bg_color < -1) bg_color = -1;
		    
		    if(w>512) w=512; //w=vbound(w,0,512);
		    
		    if(h>512) h=512; //h=vbound(h,0,512);
		    
		    //undone
		    if(w>0&&h>0)//stretch the character
		    {
			BITMAP *pbmp = script_drawing_commands.GetSmallTextureBitmap(1,1);
			
			if(opacity < 128)
			{
			    if(w>128||h>128)
			    {
				clear_bitmap(prim_bmp);
				
				textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
				stretch_sprite(prim_bmp, pbmp, 0, 0, w, h);
				draw_trans_sprite(bmp, prim_bmp, x+xoffset, y+yoffset);
			    }
			    else //this is faster
			    {
				BITMAP *pbmp2 = script_drawing_commands.AquireSubBitmap(w,h);
				
				textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
				stretch_sprite(pbmp2, pbmp, 0, 0, w, h);
				draw_trans_sprite(bmp, pbmp2, x+xoffset, y+yoffset);
				
				script_drawing_commands.ReleaseSubBitmap(pbmp2);
			    }
			}
			else // no opacity
			{
			    textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
			    stretch_sprite(bmp, pbmp, x+xoffset, y+yoffset, w, h);
			}
			
		    }
		    else //no stretch
		    {
			if(opacity < 128)
			{
			    BITMAP *pbmp = create_sub_bitmap(prim_bmp,0,0,16,16);
			    clear_bitmap(pbmp);
			    
			    textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
			    draw_trans_sprite(bmp, pbmp, x+xoffset, y+yoffset);
			    
			    destroy_bitmap(pbmp);
			}
			else // no opacity
			{
			    textprintf_ex(bmp, get_zc_font(font_index), x+xoffset, y+yoffset, color, bg_color, "%c", glyph);
			}
		    }		
	}
	
	else //2.53.0 fixed version and later.
	{
	
		//sdci[1]=layer
		    //sdci[2]=x
		    //sdci[3]=y
		    //sdci[4]=font
		    //sdci[5]=color
		    //sdci[6]=bg color
		    //sdci[7]=strech x (width)
		    //sdci[8]=stretch y (height)
		    //sdci[9]=char
		    //sdci[10]=opacity
		    
		    int32_t x=sdci[2]/10000;
		    int32_t y=sdci[3]/10000;
		    int32_t font_index=sdci[4]/10000;
		    int32_t color=sdci[5]/10000;
		    int32_t bg_color=sdci[6]/10000; //-1 = transparent
		    int32_t w=sdci[7]/10000;
		    int32_t h=sdci[8]/10000;
		    char glyph=char(sdci[9]/10000);
		    int32_t opacity=sdci[10]/10000;
		    
		    //safe check
		    if(bg_color < -1) bg_color = -1;
		    
		    if(w>512) w=512; //w=vbound(w,0,512);
		    
		    if(h>512) h=512; //h=vbound(h,0,512);
		    
		    //undone
		    if(w>0&&h>0)//stretch the character
		    {
			BITMAP *pbmp = script_drawing_commands.GetSmallTextureBitmap(1,1);
			
			if(opacity < 128)
			{
			    if(w>128||h>128)
			    {
				clear_bitmap(prim_bmp);
				
				textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
				stretch_sprite(prim_bmp, pbmp, 0, 0, w, h);
				draw_trans_sprite(bmp, prim_bmp, x+xoffset, y+yoffset);
			    }
			    else //this is faster
			    {
				BITMAP *pbmp2 = script_drawing_commands.AquireSubBitmap(w,h);
				
				textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
				stretch_sprite(pbmp2, pbmp, 0, 0, w, h);
				draw_trans_sprite(bmp, pbmp2, x+xoffset, y+yoffset);
				
				script_drawing_commands.ReleaseSubBitmap(pbmp2);
			    }
			}
			else // no opacity
			{
			    textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
			    stretch_sprite(bmp, pbmp, x+xoffset, y+yoffset, w, h);
			}
			
		    }
		    else //no stretch
		    {
			if(opacity < 128)
			{
			    BITMAP *pbmp = create_sub_bitmap(prim_bmp,0,0,16,16);
			    clear_bitmap(pbmp);
			    
			    textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
			    draw_trans_sprite(bmp, pbmp, x+xoffset, y+yoffset);
			    
			    destroy_bitmap(pbmp);
			}
			else // no opacity
			{
			    textprintf_ex(bmp, get_zc_font(font_index), x+xoffset, y+yoffset, color, bg_color, "%c", glyph);
			}
		    }		
		
	}
    
}


void do_drawintr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	//broken 2.50.2 and earlier drawinteger()
	if ( get_qr(qr_BROKENCHARINTDRAWING) )
	{
	    //sdci[1]=layer
	    //sdci[2]=x
	    //sdci[3]=y
	    //sdci[4]=font
	    //sdci[5]=color
	    //sdci[6]=bg color
	    //sdci[7]=strech x (width)
	    //sdci[8]=stretch y (height)
	    //sdci[9]=integer
	    //sdci[10]=num decimal places
	    //sdci[11]=opacity
	    
	    int32_t x=sdci[2]/10000;
	    int32_t y=sdci[3]/10000;
	    int32_t font_index=sdci[4]/10000;
	    int32_t color=sdci[5]/10000;
	    int32_t bg_color=sdci[6]/10000; //-1 = transparent
	    int32_t w=sdci[7]/10000;
	    int32_t h=sdci[8]/10000;
	    int32_t decplace=sdci[10]/10000;
	    int32_t opacity=sdci[11]/10000;
	    
	    //safe check
	    if(bg_color < -1) bg_color = -1;
	    
	    if(w>512) w=512; //w=vbound(w,0,512);
	    
	    if(h>512) h=512; //h=vbound(h,0,512);
	    
	    char numbuf[15];
	    
		switch(decplace)
		{
		    default:
		    case 0:
			sprintf(numbuf,"%d",(sdci[9]/10000)); //For some reason, static casting for zero decimal places was
			break;					//reducing the value by -1, so 8.000 printed as '7'. -Z
			
		    case 1:
			//sprintf(numbuf,"%.01f",number);
			sprintf(numbuf,"%.01f",(static_cast<float>(sdci[9])/10000.0f)); //Would this be slower? 
			break;
			
		    case 2:
			//sprintf(numbuf,"%.02f",number);
			sprintf(numbuf,"%.02f",(static_cast<float>(sdci[9])/10000.0f));
			break;
			
		    case 3:
			//sprintf(numbuf,"%.03f",number);
			sprintf(numbuf,"%.03f",(static_cast<float>(sdci[9])/10000.0f));
			break;
			
		    case 4:
			//sprintf(numbuf,"%.04f",number);
			sprintf(numbuf,"%.04f",(static_cast<float>(sdci[9])/10000.0f));
			break;
		}
	    
	    if(w>0&&h>0)//stretch
	    {
		BITMAP *pbmp = script_drawing_commands.GetSmallTextureBitmap(1,1);
		
		if(opacity < 128)
		{
		    if(w>128||h>128)
		    {
			clear_bitmap(prim_bmp);
			
			textout_ex(pbmp, get_zc_font(font_index), numbuf, 0, 0, color, bg_color);
			stretch_sprite(prim_bmp, pbmp, 0, 0, w, h);
			draw_trans_sprite(bmp, prim_bmp, x+xoffset, y+yoffset);
		    }
		    else
		    {
			BITMAP *pbmp2 = create_sub_bitmap(prim_bmp,0,0,w,h);
			clear_bitmap(pbmp2);
			
			textout_ex(pbmp, get_zc_font(font_index), numbuf, 0, 0, color, bg_color);
			stretch_sprite(pbmp2, pbmp, 0, 0, w, h);
			draw_trans_sprite(bmp, pbmp2, x+xoffset, y+yoffset);
			
			destroy_bitmap(pbmp2);
		    }
		}
		else // no opacity
		{
		    textout_ex(pbmp, get_zc_font(font_index), numbuf, 0, 0, color, bg_color);
		    stretch_sprite(bmp, pbmp, x+xoffset, y+yoffset, w, h);
		}
		
	    }
	    else //no stretch
	    {
		if(opacity < 128)
		{
		    BITMAP *pbmp = create_sub_bitmap(prim_bmp,0,0,16,16);
		    clear_bitmap(pbmp);
		    
		    textout_ex(pbmp, get_zc_font(font_index), numbuf, 0, 0, color, bg_color);
		    draw_trans_sprite(bmp, pbmp, x+xoffset, y+yoffset);
		    
		    destroy_bitmap(pbmp);
		}
		else // no opacity
		{
		    textout_ex(bmp, get_zc_font(font_index), numbuf, x+xoffset, y+yoffset, color, bg_color);
		}
	    }
		
	}
	
	else //2.53.0 fixed version and later.
	{
	    //sdci[1]=layer
	    //sdci[2]=x
	    //sdci[3]=y
	    //sdci[4]=font
	    //sdci[5]=color
	    //sdci[6]=bg color
	    //sdci[7]=strech x (width)
	    //sdci[8]=stretch y (height)
	    //sdci[9]=integer
	    //sdci[10]=num decimal places
	    //sdci[11]=opacity
	    
	    int32_t x=sdci[2]/10000;
	    int32_t y=sdci[3]/10000;
	    int32_t font_index=sdci[4]/10000;
	    int32_t color=sdci[5]/10000;
	    int32_t bg_color=sdci[6]/10000; //-1 = transparent
	    int32_t w=sdci[7]/10000;
	    int32_t h=sdci[8]/10000;
	    int32_t decplace=sdci[10]/10000;
	    int32_t opacity=sdci[11]/10000;
	    
	    //safe check
	    if(bg_color < -1) bg_color = -1;
	    
	    if(w>512) w=512; //w=vbound(w,0,512);
	    
	    if(h>512) h=512; //h=vbound(h,0,512);
	    
	    char numbuf[15];
	    
	    switch(decplace)
	    {
	    default:
	    case 0:
		sprintf(numbuf,"%d",(sdci[9]/10000)); //For some reason, static casting for zero decimal places was
		break;					//reducing the value by -1, so 8.000 printed as '7'. -Z
		
	    case 1:
		//sprintf(numbuf,"%.01f",number);
		sprintf(numbuf,"%.01f",(static_cast<float>(sdci[9])/10000.0f)); //Would this be slower? 
		break;
		
	    case 2:
		//sprintf(numbuf,"%.02f",number);
		sprintf(numbuf,"%.02f",(static_cast<float>(sdci[9])/10000.0f));
		break;
		
	    case 3:
		//sprintf(numbuf,"%.03f",number);
		sprintf(numbuf,"%.03f",(static_cast<float>(sdci[9])/10000.0f));
		break;
		
	    case 4:
		//sprintf(numbuf,"%.04f",number);
		sprintf(numbuf,"%.04f",(static_cast<float>(sdci[9])/10000.0f));
		break;
	    }
	    
	    //FONT* font=get_zc_font(sdci[4]/10000);
	    
	    if(w>0&&h>0)//stretch
	    {
		BITMAP *pbmp = create_sub_bitmap(prim_bmp, 0, 0, text_length(get_zc_font(font_index), numbuf)+1, text_height(get_zc_font(font_index)));
		clear_bitmap(pbmp);
		    //script_drawing_commands.GetSmallTextureBitmap(1,1);
		
		if(opacity < 128)
		{
		    if(w>128||h>128)
		    {
			clear_bitmap(prim_bmp);
			
			textout_ex(pbmp, get_zc_font(font_index), numbuf, 0, 0, color, bg_color);
			stretch_sprite(prim_bmp, pbmp, 0, 0, w, h);
			draw_trans_sprite(bmp, prim_bmp, x+xoffset, y+yoffset);
		    }
		    else
		    {
			BITMAP *pbmp2 = create_sub_bitmap(prim_bmp,0,0,w,h);
			clear_bitmap(pbmp2);
			
			textout_ex(pbmp, get_zc_font(font_index), numbuf, 0, 0, color, bg_color);
			stretch_sprite(pbmp2, pbmp, 0, 0, w, h);
			draw_trans_sprite(bmp, pbmp2, x+xoffset, y+yoffset);
			
			destroy_bitmap(pbmp2);
		    }
		}
		else // no opacity
		{
		    textout_ex(pbmp, get_zc_font(font_index), numbuf, 0, 0, color, bg_color);
		    stretch_sprite(bmp, pbmp, x+xoffset, y+yoffset, w, h);
		}
		
	    }
	    else //no stretch
	    {
		if(opacity < 128)
		{
		    FONT* font = get_zc_font(font_index);
		    BITMAP *pbmp = create_sub_bitmap(prim_bmp, 0, 0, text_length(font, numbuf), text_height(font));
		    clear_bitmap(pbmp);
		    
		    textout_ex(pbmp, font, numbuf, 0, 0, color, bg_color);
		    draw_trans_sprite(bmp, pbmp, x+xoffset, y+yoffset);
		    
		    destroy_bitmap(pbmp);
		}
		else // no opacity
		{
		    textout_ex(bmp, get_zc_font(font_index), numbuf, x+xoffset, y+yoffset, color, bg_color);
		}
	    }
	}
}


void do_drawstringr(BITMAP *bmp, int32_t i, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=font
    //sdci[5]=color
    //sdci[6]=bg color
    //sdci[7]=format_option
    //sdci[8]=string
    //sdci[9]=opacity
    
    std::string* str = (std::string*)script_drawing_commands[i].GetPtr();
    
    if(!str)
    {
        al_trace("String pointer is null! Internal error. \n");
        return;
    }
    
    int32_t x=sdci[2]/10000;
    int32_t y=sdci[3]/10000;
    FONT* font=get_zc_font(sdci[4]/10000);
    int32_t color=sdci[5]/10000;
    int32_t bg_color=sdci[6]/10000; //-1 = transparent
    int32_t format_type=sdci[7]/10000;
    int32_t opacity=sdci[9]/10000;
    //sdci[8] not needed :)
    
    //safe check
    if(bg_color < -1) bg_color = -1;
    
    if(opacity < 128)
    {
        int32_t width=zc_min(text_length(font, str->c_str()), 512);
		if (width < 1) return; //SANITY -Em
        BITMAP *pbmp = create_sub_bitmap(prim_bmp, 0, 0, width, text_height(font));
        clear_bitmap(pbmp);
        textout_ex(pbmp, font, str->c_str(), 0, 0, color, bg_color);
        if(format_type == 2)   // right-sided text
            x-=width;
        else if(format_type == 1)   // centered text
            x-=width/2;
        draw_trans_sprite(bmp, pbmp, x+xoffset, y+yoffset);
        destroy_bitmap(pbmp);
    }
    else // no opacity
    {
        if(format_type == 2)   // right-sided text
        {
            textout_right_ex(bmp, font, str->c_str(), x+xoffset, y+yoffset, color, bg_color);
        }
        else if(format_type == 1)   // centered text
        {
            textout_centre_ex(bmp, font, str->c_str(), x+xoffset, y+yoffset, color, bg_color);
        }
        else // standard left-sided text
        {
            textout_ex(bmp, font, str->c_str(), x+xoffset, y+yoffset, color, bg_color);
        }
    }
}

void do_drawstringr2(BITMAP *bmp, int32_t i, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=font
    //sdci[5]=color
    //sdci[6]=bg color
    //sdci[7]=format_option
    //sdci[8]=string
    //sdci[9]=opacity
	//sdci[10]=shadowtype
	//sdci[11]=shadow_color
    
    std::string* str = (std::string*)script_drawing_commands[i].GetPtr();
    
    if(!str)
    {
        al_trace("String pointer is null! Internal error. \n");
        return;
    }
    
    int32_t x=sdci[2]/10000;
    int32_t y=sdci[3]/10000;
    FONT* font=get_zc_font(sdci[4]/10000);
    int32_t color=sdci[5]/10000;
    int32_t bg_color=sdci[6]/10000; //-1 = transparent
    int32_t format_type=sdci[7]/10000;
    int32_t opacity=sdci[9]/10000;
	int32_t textstyle = sdci[10]/10000;
	int32_t shadow_color = sdci[11]/10000;
    //sdci[8] not needed :)
    
    //safe check
    if(bg_color < -1) bg_color = -1;
    
    if(opacity < 128)
    {
        int32_t width=zc_min(text_length(font, str->c_str()), 512);
		if (width < 1) return; //SANITY -Em
        BITMAP *pbmp = create_sub_bitmap(prim_bmp, 0, 0, width, text_height(font));
        clear_bitmap(pbmp);
		textout_styled_aligned_ex(pbmp, font, str->c_str(), 0, 0, textstyle, sstaLEFT, color, shadow_color, bg_color);
        textout_ex(pbmp, font, str->c_str(), 0, 0, color, bg_color);
        if(format_type == 2)   // right-sided text
            x-=width;
        else if(format_type == 1)   // centered text
            x-=width/2;
        draw_trans_sprite(bmp, pbmp, x+xoffset, y+yoffset);
        destroy_bitmap(pbmp);
    }
    else // no opacity
    {
        textout_styled_aligned_ex(bmp, font, str->c_str(), x+xoffset, y+yoffset, textstyle, format_type, color, shadow_color, bg_color);
    }
}


void do_drawquadr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x1
    //sdci[3]=y1
    //sdci[4]=x2
    //sdci[5]=y2
    //sdci[6]=x3
    //sdci[7]=y3
    //sdci[8]=x4
    //sdci[9]=y4
    //sdci[10]=width
    //sdci[11]=height
    //sdci[12]=cset
    //sdci[13]=flip
    //sdci[14]=tile/combo
    //sdci[15]=polytype
    
    int32_t x1 = sdci[2]/10000;
    int32_t y1 = sdci[3]/10000;
    int32_t x2 = sdci[4]/10000;
    int32_t y2 = sdci[5]/10000;
    int32_t x3 = sdci[6]/10000;
    int32_t y3 = sdci[7]/10000;
    int32_t x4 = sdci[8]/10000;
    int32_t y4 = sdci[9]/10000;
    int32_t w = sdci[10]/10000;
    int32_t h = sdci[11]/10000;
    int32_t color = sdci[12]/10000;
    int32_t flip=(sdci[13]/10000)&3;
    int32_t tile = sdci[14]/10000;
    int32_t polytype = sdci[15]/10000;
    
    //todo: finish palette shading
    /*
    POLYTYPE_FLAT
    POLYTYPE_GCOL
    POLYTYPE_GRGB
    POLYTYPE_ATEX
    POLYTYPE_PTEX
    POLYTYPE_ATEX_MASK
    POLYTYPE_PTEX_MASK
    POLYTYPE_ATEX_LIT
    POLYTYPE_PTEX_LIT
    POLYTYPE_ATEX_MASK_LIT
    POLYTYPE_PTEX_MASK_LIT
    POLYTYPE_ATEX_TRANS
    POLYTYPE_PTEX_TRANS
    POLYTYPE_ATEX_MASK_TRANS
    POLYTYPE_PTEX_MASK_TRANS
    */
    polytype = vbound(polytype, 0, 14);
    
    if(((w-1) & w) != 0 || ((h-1) & h) != 0)
    {
        Z_message("Quad() : Args h, w, must be in powers of two! Power of 2 error with %i, %i.", w, h);
        return; //non power of two error
    }
    
    int32_t tex_width = w*16;
    int32_t tex_height = h*16;
    
    BITMAP *tex;
    
    bool mustDestroyBmp = false;
    
	if ( tile > 65519 ) tex = zscriptDrawingRenderTarget->GetBitmapPtr(tile - 65519);
	else tex = script_drawing_commands.GetSmallTextureBitmap(w,h);
    
    if(!tex)
    {
        mustDestroyBmp = true;
        tex = create_bitmap_ex(8, tex_width, tex_height);
        clear_bitmap(tex);
    }
    
    int32_t col[4];
    /*
    if( color < 0 )
    {
    col[0]=draw_container.color_buffer[0];
    col[1]=draw_container.color_buffer[1];
    col[2]=draw_container.color_buffer[2];
    col[3]=draw_container.color_buffer[3];
    }
    else */
    {
        col[0]=col[1]=col[2]=col[3]=color;
    }
    
    if(tile > 0 && tile <= 65519)   // TILE
    {
        TileHelper::OverTile(tex, tile, 0, 0, w, h, color, flip);
    }
    
    if ( tile < 0 )        // COMBO
    {
        auto& c = GET_DRAWING_COMBO(vbound(abs(tile), 0, 0xffff));
        const int32_t tiletodraw = combo_tile(c, x1, y1);
        flip = flip ^ c.flip;
        
        TileHelper::OldPutTile(tex, tiletodraw, 0, 0, w, h, color, flip);
    }
    
    V3D_f V1 = { static_cast<float>(x1+xoffset), static_cast<float>(y1+yoffset), 0, 0,                             0,                              col[0] };
    V3D_f V2 = { static_cast<float>(x2+xoffset), static_cast<float>(y2+yoffset), 0, 0,                             static_cast<float>(tex_height), col[1] };
    V3D_f V3 = { static_cast<float>(x3+xoffset), static_cast<float>(y3+yoffset), 0, static_cast<float>(tex_width), static_cast<float>(tex_height), col[2] };
    V3D_f V4 = { static_cast<float>(x4+xoffset), static_cast<float>(y4+yoffset), 0, static_cast<float>(tex_width), 0,                              col[3] };
    
    quad3d_f(bmp, polytype, tex, &V1, &V2, &V3, &V4);
    
    if(mustDestroyBmp)
        destroy_bitmap(tex);
        
}


void do_drawtriangler(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x1
    //sdci[3]=y1
    //sdci[4]=x2
    //sdci[5]=y2
    //sdci[6]=x3
    //sdci[7]=y3
    //sdci[8]=width
    //sdci[9]=height
    //sdci[10]=cset
    //sdci[11]=flip
    //sdci[12]=tile/combo
    //sdci[13]=polytype
    
    int32_t x1 = sdci[2]/10000;
    int32_t y1 = sdci[3]/10000;
    int32_t x2 = sdci[4]/10000;
    int32_t y2 = sdci[5]/10000;
    int32_t x3 = sdci[6]/10000;
    int32_t y3 = sdci[7]/10000;
    int32_t w = sdci[8]/10000;
    int32_t h = sdci[9]/10000;
    int32_t color = sdci[10]/10000;
    int32_t flip=(sdci[11]/10000)&3;
    int32_t tile = sdci[12]/10000;
    int32_t polytype = sdci[13]/10000;
    
    polytype = vbound(polytype, 0, 14);
    
    if(((w-1) & w) != 0 || ((h-1) & h) != 0)
    {
        Z_message("Quad() : Args h, w, must be in powers of two! Power of 2 error with %i, %i.", w, h);
        return; //non power of two error
    }
    
    int32_t tex_width = w*16;
    int32_t tex_height = h*16;
    
    bool mustDestroyBmp = false;
    BITMAP *tex = script_drawing_commands.GetSmallTextureBitmap(w,h);
    
    if(!tex)
    {
        mustDestroyBmp = true;
        tex = create_bitmap_ex(8, tex_width, tex_height);
        clear_bitmap(tex);
    }
    
    int32_t col[3];
    /*
    if( color < 0 )
    {
    col[0]=draw_container.color_buffer[0];
    col[1]=draw_container.color_buffer[1];
    col[2]=draw_container.color_buffer[2];
    }
    else */
    {
        col[0]=col[1]=col[2]=color;
    }
    
    if(tile > 0)   // TILE
    {
        TileHelper::OverTile(tex, tile, 0, 0, w, h, color, flip);
    }
    else        // COMBO
    {
        auto& c = GET_DRAWING_COMBO(vbound(abs(tile), 0, 0xffff));
        const int32_t tiletodraw = combo_tile(c, x1, y1);
        flip = flip ^ c.flip;
        
        TileHelper::OldPutTile(tex, tiletodraw, 0, 0, w, h, color, flip);
    }
    
    V3D_f V1 = { static_cast<float>(x1+xoffset), static_cast<float>(y1+yoffset), 0, 0,                             0,                              col[0] };
    V3D_f V2 = { static_cast<float>(x2+xoffset), static_cast<float>(y2+yoffset), 0, 0,                             static_cast<float>(tex_height), col[1] };
    V3D_f V3 = { static_cast<float>(x3+xoffset), static_cast<float>(y3+yoffset), 0, static_cast<float>(tex_width), static_cast<float>(tex_height), col[2] };
    
    
    triangle3d_f(bmp, polytype, tex, &V1, &V2, &V3);
    
    if(mustDestroyBmp)
        destroy_bitmap(tex);
}


void do_drawbitmapr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	//sdci[1]=layer
	//sdci[2]=bitmap
	//sdci[3]=sourcex
	//sdci[4]=sourcey
	//sdci[5]=sourcew
	//sdci[6]=sourceh
	//sdci[7]=destx
	//sdci[8]=desty
	//sdci[9]=destw
	//sdci[10]=desth
	//sdci[11]=rotation
	//sdci[12]=mask

	int32_t bitmapIndex = sdci[2]/10000;
	int32_t sx = sdci[3]/10000;
	int32_t sy = sdci[4]/10000;
	int32_t sw = sdci[5]/10000;
	int32_t sh = sdci[6]/10000;
	int32_t dx = sdci[7]/10000;
	int32_t dy = sdci[8]/10000;
	int32_t dw = sdci[9]/10000;
	int32_t dh = sdci[10]/10000;
	float rot = sdci[11]/10000;
	bool masked = (sdci[12] != 0);

	//bugfix
	sx = vbound(sx, 0, 512);
	sy = vbound(sy, 0, 512);
	sw = vbound(sw, 0, 512 - sx); //keep the w/h within range as well
	sh = vbound(sh, 0, 512 - sy);


	if(sx >= ZScriptDrawingRenderTarget::BitmapWidth || sy >= ZScriptDrawingRenderTarget::BitmapHeight)
	return;

	bool stretched = (sw != dw || sh != dh);

	BITMAP *sourceBitmap = zscriptDrawingRenderTarget->GetBitmapPtr(bitmapIndex);
    
	if(!sourceBitmap)
	{
		Z_message("Warning: Screen->DrawBitmap(%d) contains invalid data or is not initialized.\n", bitmapIndex);
		Z_message("[Note* Deferred drawing or layering order possibly not set right.]\n");
		return;
	}
    
	BITMAP* subBmp = 0;
    
	if(rot != 0)
	{
		subBmp = script_drawing_commands.AquireSubBitmap(dw, dh);
        
		if(!subBmp)
		{
			Z_scripterrlog("DrawBitmap() failed to create a sub-bitmap to use for %s. Aborting.\n", "rotation");
			return;
		}
	}
    
    
	dx = dx + xoffset;
	dy = dy + yoffset;
    
	if(stretched)
	{
		if(masked)
		{
			if(rot != 0)
			{	
				//if ( rot == 4096 ) { //translucent
				//	masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
				//	//rotate_sprite_trans(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
				//	draw_trans_sprite(bmp, subBmp, dx, dy);
				//	//draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, 0);
			
			
				// }
				//else { 
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					//rotate_sprite(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					//
			
				// }
			}
			else
				masked_stretch_blit(sourceBitmap, bmp, sx, sy, sw, sh, dx, dy, dw, dh);
		}
		else
		{
			if(rot != 0)
			{
				//if ( rot == 4096 ) { //translucent
				//	stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
				//	draw_trans_sprite(bmp, subBmp, dx, dy);
				// }
				//else {
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
				// }
			}
			else
				stretch_blit(sourceBitmap, bmp, sx, sy, sw, sh, dx, dy, dw, dh);
		}
	}
	else
	{
		if(masked)
		{
			if(rot != 0)
			{
				//if ( rot == 4096 ) {//translucent
				//	masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					//rotate_sprite(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
		    
					//masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//rotate_sprite_trans(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
				//	draw_trans_sprite(bmp, subBmp, dx, dy);
				// }
			//else {
				masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
				rotate_sprite(bmp, subBmp, dx, dy, degrees_to_fixed(rot));  
			// }
		}
		else
			masked_blit(sourceBitmap, bmp, sx, sy, dx, dy, dw, dh);
		}
		else
		{
			if(rot != 0)
			{
				//if ( rot == 4096 ) { //translucent
				//	blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);   
				//	draw_trans_sprite(bmp, subBmp, dx, dy);
				// }
				//else {
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
				// }
			}
			else
				blit(sourceBitmap, bmp, sx, sy, dx, dy, dw, dh);
		}
	}
    
	//cleanup
	if(subBmp)
	{
		script_drawing_commands.ReleaseSubBitmap(subBmp);
	}
}


//Draw]()
void do_drawbitmapexr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	/*
	//sdci[1]=layer
	//sdci[2]=bitmap
	//sdci[3]=sourcex
	//sdci[4]=sourcey
	//sdci[5]=sourcew
	//sdci[6]=sourceh
	//sdci[7]=destx
	//sdci[8]=desty
	//sdci[9]=destw
	//sdci[10]=desth
	//sdci[11]=rotation/angle
	//scdi[12] = pivot cx
	//sdci[13] = pivot cy
	//scdi[14] = effect flags
	
		
		const int32_t BITDX_NORMAL = 0;
		const int32_t BITDX_TRANS = 1; //Translucent
		const int32_t BITDX_PIVOT = 2; //THe sprite will rotate at a specific point, instead of its center.
		const int32_t BITDX_HFLIP = 4; //Horizontal Flip
		const int32_t BITDX_VFLIP = 8; //Vertical Flip.
		//Note:	Some modes cannot be combined. if a combination is not supported, an error
		//	detailing this will be shown in allegro.log.
		
	//scdi[15] = litcolour
		//The allegro docs are wrong. The params are: rotate_sprite_lit(bmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour); 
		/not rotate_sprite_lit(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
	
	//sdci[16]=mask
	
	*/

	int32_t bitmapIndex = sdci[2]/10000;
	int32_t sx = sdci[3]/10000;
	int32_t sy = sdci[4]/10000;
	int32_t sw = sdci[5]/10000;
	int32_t sh = sdci[6]/10000;
	int32_t dx = sdci[7]/10000;
	int32_t dy = sdci[8]/10000;
	int32_t dw = sdci[9]/10000;
	int32_t dh = sdci[10]/10000;
	float rot = sdci[11]/10000;
	int32_t cx = sdci[12]/10000;
	int32_t cy = sdci[13]/10000;
	int32_t mode = sdci[14]/10000;
	int32_t litcolour = sdci[15]/10000;
	bool masked = (sdci[16] != 0);
	
	

	//bugfix
	sx = vbound(sx, 0, 512);
	sy = vbound(sy, 0, 512);
	sw = vbound(sw, 0, 512 - sx); //keep the w/h within range as well
	sh = vbound(sh, 0, 512 - sy);


	if(sx >= ZScriptDrawingRenderTarget::BitmapWidth || sy >= ZScriptDrawingRenderTarget::BitmapHeight)
	return;

	bool stretched = (sw != dw || sh != dh);

	BITMAP *sourceBitmap = zscriptDrawingRenderTarget->GetBitmapPtr(bitmapIndex);
    
	if(!sourceBitmap)
	{
		Z_message("Warning: Screen->DrawBitmap(%d) contains invalid data or is not initialized.\n", bitmapIndex);
		Z_message("[Note* Deferred drawing or layering order possibly not set right.]\n");
		return;
	}
    
	BITMAP* subBmp = 0;
	
	/*
	if ( bitmapIndex == -1 ) {
		blit(bmp, sourceBitmap, sx, sy, 0, 0, dw, dh); 
	}
	*/
    
	if(rot != 0 || mode != 0)    
	{
		subBmp = script_drawing_commands.AquireSubBitmap(dw, dh);
        
		if(!subBmp)
		{
			Z_scripterrlog("bitmap->Blit failed to create a sub-bitmap to use for %s. Aborting.\n", "rotation");
			return;
		}
	}
    
    
	dx = dx + xoffset;
	dy = dy + yoffset;
    
	if(stretched) 
	{
		if(masked) //stretched and masked
		{	
			if ( rot == 0 ) //if not rotated
			{ 
				switch(mode) 
				{
					case 1:
					//transparent
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_trans_sprite(bmp, subBmp, dx, dy);
					break;
					
					
					case 2: 
						//pivot?
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite(bmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_trans(bmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					break;
					
					case 4: 
						//flip v
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_v_flip(bmp, subBmp, dx, dy);
					break;
					
					case 5: 
						//trans + v flip
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_V_FLIP);
					break;
					
					case 6: 
						//pivot + v flip
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_v_flip(bmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					break;
					
					case 8: 
						//vlip h
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_h_flip(bmp, subBmp, dx, dy);
					break;
					
					case 9: 
						//trans + h flip
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_H_FLIP);
					break;
					
					case 10: 
						//flip H and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					//return error cannot pivot and h flip
					break;
					
					case 12:
						//vh flip
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_vh_flip(bmp, subBmp, dx, dy);
					break;
					
					case 13: 
						//trans + vh flip
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_lit_sprite(bmp, subBmp, dx, dy, litcolour);
					break;
					
					case 18: 
						//pivot, lit
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_lit(bmp, subBmp, dx, dy, cx, cy,  degrees_to_fixed(rot),litcolour);
					break;
					
					case 20: 
						//lit + v flip
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_V_FLIP);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_v_flip_lit(bmp, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 24: 
						//lit + h flip
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_H_FLIP);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot, Flip, and Lit.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect
					masked_stretch_blit(sourceBitmap, bmp, sx, sy, sw, sh, dx, dy, dw, dh);
					break;
					
					
					default:
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
					
					
				}
			} //end if not rotated
			
			if ( rot != 0 ) //if rotated
			{ 
				switch(mode)
				{
					case 1: 
						//transparent
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_trans(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					
					break;
					
					case 2: 
						//pivot?
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					//return an error, cannot both rotate and pivot
					break;
					
					case 3: 
						//pivot + trans
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 4: 
						//flip v
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 5: 
						//trans + v flip
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip_trans(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 6: 
						//pivot + v flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					//return an error, cannot both rotate and pivot
					break;
					
					case 8: 
						//flip h
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rorate and H-Flip.\n", bitmapIndex);
					//return an error, cannot both rotate and flip H
					break;
					
					case 9: 
						//trans + h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot Rotate and Flip a Trans Sprite.\n", bitmapIndex);
					//return an error, cannot rotate and flip a trans sprite
					break;
					
					case 10: 
						//flip H and pivot
					//return error cannot pivot and h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					break;
					
					case 12: 
						//vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 13: 
						//trans + vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_lit(bmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 18: 
						//pivot, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 20: 
						//lit + vflip
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip_lit(bmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					break;
					
					case 24: 
						//lit + h flip
					//return an error, cannot both rotate and H flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and H-Flip.\n", bitmapIndex);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Flip a Lit Sprite.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					//return an error, cannot both rotate and VH flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect.
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					default:
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
				
				}
			}
		} //end if stretched and masked 
		
		else  //stretched, not masked
		{
			if ( rot == 0 )  //if not rotated
			{
				switch(mode) {
					case 1:
					//transparent
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_trans_sprite(bmp, subBmp, dx, dy);
					break;
					
					
					case 2: 
						//pivot?
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite(bmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_trans(bmp, subBmp, dx,dy,  cx,  cy, degrees_to_fixed(rot));
					break;
					
					case 4: 
						//flip v
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_v_flip(bmp, subBmp, dx, dy);
					break;
					
					case 5: 
						//trans + v flip
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_V_FLIP);
					break;
					
					case 6: 
						//pivot + v flip
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_v_flip(bmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					break;
					
					case 8: 
						//vlip h
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_h_flip(bmp, subBmp, dx, dy);
					break;
					
					case 9: 
						//trans + h flip
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_H_FLIP);
					break;
					
					case 10: 
						//flip H and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					//return error cannot pivot and h flip
					break;
					
					case 12:
						//vh flip
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_vh_flip(bmp, subBmp, dx, dy);
					break;
					
					case 13: 
						//trans + vh flip
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_lit_sprite(bmp, subBmp, dx, dy, litcolour);
					break;
					
					case 18: 
						//pivot, lit
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_lit(bmp, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 20: 
						//lit + v flip
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_V_FLIP);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_v_flip_lit(bmp, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 24: 
						//lit + h flip
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_H_FLIP);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot, Flip, and Lit.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect
					stretch_blit(sourceBitmap, bmp, sx, sy, sw, sh, dx, dy, dw, dh);
					break;
					
					
					default:
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
					
					
				}
			} //end if not rotated
			
			if ( rot != 0 ) //if rotated
			{ 
				switch(mode)
				{
					case 1: 
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);//transparent
					rotate_sprite_trans(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					
					break;
					
					case 2: 
						//pivot?
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					//return an error, cannot both rotate and pivot
					break;
					
					case 3: 
						//pivot + trans
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 4: 
						//flip v
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 5: 
						//trans + v flip
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip_trans(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 6: 
						//pivot + v flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					//return an error, cannot both rotate and pivot
					break;
					
					case 8: 
						//flip h
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rorate and H-Flip.\n", bitmapIndex);
					//return an error, cannot both rotate and flip H
					break;
					
					case 9: 
						//trans + h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot Rotate and Flip a Trans Sprite.\n", bitmapIndex);
					//return an error, cannot rotate and flip a trans sprite
					break;
					
					case 10: 
						//flip H and pivot
					//return error cannot pivot and h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					break;
					
					case 12: 
						//vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 13: 
						//trans + vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);//transparent
					rotate_sprite_lit(bmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 18: 
						//pivot, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 20: 
						//lit + vflip
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);//transparent
					rotate_sprite_v_flip_lit(bmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					break;
					
					case 24: 
						//lit + h flip
					//return an error, cannot both rotate and H flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and H-Flip.\n", bitmapIndex);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Flip a Lit Sprite.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					//return an error, cannot both rotate and VH flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect.
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					default:
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
				
				}
			}
			
		} //end if stretched, but not masked
	}
	else //not stretched
	{ 
		
		if(masked) //if masked, but not stretched
		{ 
			
			if ( rot == 0 ) //if not rotated
			{ 
				switch(mode) 
				{
					case 1:
					//transparent
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					draw_trans_sprite(bmp, subBmp, dx, dy);
					break;
					
					
					case 2: 
						//pivot?
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite(bmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite_trans(bmp, subBmp, dx, dy,  cx, cy, degrees_to_fixed(rot));
					break;
					
					case 4: 
						//flip v
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_v_flip(bmp, subBmp, dx, dy);
					break;
					
					case 5: 
						//trans + v flip
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_V_FLIP);
					break;
					
					case 6: 
						//pivot + v flip
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite_v_flip(bmp, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot));
					break;
					
					case 8: 
						//vlip h
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_h_flip(bmp, subBmp, dx, dy);
					break;
					
					case 9: 
						//trans + h flip
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_H_FLIP);
					break;
					
					case 10: 
						//flip H and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					//return error cannot pivot and h flip
					break;
					
					case 12:
						//vh flip
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_vh_flip(bmp, subBmp, dx, dy);
					break;
					
					case 13: 
						//trans + vh flip
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					draw_lit_sprite(bmp, subBmp, dx, dy, litcolour);
					break;
					
					case 18: 
						//pivot, lit
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite_lit(bmp, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 20: 
						//lit + v flip
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_V_FLIP);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite_v_flip_lit(bmp, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 24: 
						//lit + h flip
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_H_FLIP);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot, Flip, and Lit.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect
					masked_blit(sourceBitmap, bmp, sx, sy, dx, dy, dw, dh);
					break;
					
					
					default:
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
					
					
				}
			} //end if not rotated
			
			if ( rot != 0 )  //if rotated
			{
				switch(mode)
				{
					case 1: 
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);	//transparent
					rotate_sprite_trans(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					
					break;
					
					case 2: 
						//pivot?
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					//return an error, cannot both rotate and pivot
					break;
					
					case 3: 
						//pivot + trans
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 4: 
						//flip v
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_v_flip(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 5: 
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);	//trans + v flip
					rotate_sprite_v_flip_trans(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 6: 
						//pivot + v flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					//return an error, cannot both rotate and pivot
					break;
					
					case 8: 
						//flip h
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rorate and H-Flip.\n", bitmapIndex);
					//return an error, cannot both rotate and flip H
					break;
					
					case 9: 
						//trans + h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot Rotate and Flip a Trans Sprite.\n", bitmapIndex);
					//return an error, cannot rotate and flip a trans sprite
					break;
					
					case 10: 
						//flip H and pivot
					//return error cannot pivot and h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					break;
					
					case 12: 
						//vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 13: 
						//trans + vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_lit(bmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 18: 
						//pivot, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 20: 
						//lit + vflip
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_v_flip_lit(bmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					break;
					
					case 24: 
						//lit + h flip
					//return an error, cannot both rotate and H flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and H-Flip.\n", bitmapIndex);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Flip a Lit Sprite.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					//return an error, cannot both rotate and VH flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect.
					masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					default:
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
				
				}
			} //end rtated, masked
		} //end if masked

		else //not masked, and not stretched; just blit
		{ 
			
			if ( rot == 0 ) //if not rotated
			{ 
				switch(mode) 
				{
					case 1:
					//transparent
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_trans_sprite(bmp, subBmp, dx, dy);
					break;
					
					
					case 2: 
						//pivot?
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite(bmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite_trans(bmp, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot));
					break;
					
					case 4: 
						//flip v
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_v_flip(bmp, subBmp, dx, dy);
					break;
					
					case 5: 
						//trans + v flip
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_V_FLIP);
					break;
					
					case 6: 
						//pivot + v flip
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite_v_flip(bmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					break;
					
					case 8: 
						//vlip h
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_h_flip(bmp, subBmp, dx, dy);
					break;
					
					case 9: 
						//trans + h flip
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_H_FLIP);
					break;
					
					case 10: 
						//flip H and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					//return error cannot pivot and h flip
					break;
					
					case 12:
						//vh flip
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_vh_flip(bmp, subBmp, dx, dy);
					break;
					
					case 13: 
						//trans + vh flip
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_lit_sprite(bmp, subBmp, dx, dy, litcolour);
					break;
					
					case 18: 
						//pivot, lit
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite_lit(bmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 20: 
						//lit + v flip
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_V_FLIP);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite_v_flip_lit(bmp, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 24: 
						//lit + h flip
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_H_FLIP);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot, Flip, and Lit.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(bmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect
					blit(sourceBitmap, bmp, sx, sy, dx, dy, dw, dh);
					break;
					
					
					default:
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
					
					
				}
			} //end if not rotated
			
			if ( rot != 0 ) //if rotated
			{ 
				switch(mode)
				{
					case 1: 
						blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);//transparent
						rotate_sprite_trans(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					 
					break;
					
					case 2: 
						//pivot?
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					//return an error, cannot both rotate and pivot
					break;
					
					case 3: 
						//pivot + trans
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 4: 
						//flip v
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					rotate_sprite_v_flip(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 5: 
						//trans + v flip
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_v_flip_trans(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 6: 
						//pivot + v flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					//return an error, cannot both rotate and pivot
					break;
					
					case 8: 
						//flip h
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rorate and H-Flip.\n", bitmapIndex);
					//return an error, cannot both rotate and flip H
					break;
					
					case 9: 
						//trans + h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot Rotate and Flip a Trans Sprite.\n", bitmapIndex);
					//return an error, cannot rotate and flip a trans sprite
					break;
					
					case 10: 
						//flip H and pivot
					//return error cannot pivot and h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					break;
					
					case 12: 
						//vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 13: 
						//trans + vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_lit(bmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 18: 
						//pivot, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 20: 
						//lit + vflip
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_v_flip_lit(bmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					break;
					
					case 24: 
						//lit + h flip
					//return an error, cannot both rotate and H flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and H-Flip.\n", bitmapIndex);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Flip a Lit Sprite.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					//return an error, cannot both rotate and VH flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect.
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh); 
					rotate_sprite(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					default:
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
				
				}
			} //end if rotated
		} //end if not masked
	} //end if not stretched
    
	//cleanup
	if(subBmp) 
	{
		script_drawing_commands.ReleaseSubBitmap(subBmp); //purge the temporary bitmap.
	}
}


void do_drawquad3dr(BITMAP *bmp, int32_t i, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=pos[12]
    //sdci[3]=uv[8]
    //sdci[4]=color[4]
    //sdci[5]=size[2]
    //sdci[6]=flip
    //sdci[7]=tile/combo
    //sdci[8]=polytype
    
    std::vector<int32_t>* v_ptr = (std::vector<int32_t>*)script_drawing_commands[i].GetPtr();
    
    if(!v_ptr)
    {
        al_trace("Quad3d: Vector pointer is null! Internal error. \n");
        return;
    }
    
    std::vector<int32_t> &v = *v_ptr;
    
    if(v.empty())
        return;
        
    int32_t* pos = &v[0];
    int32_t* uv = &v[12];
    int32_t* col = &v[20];
    int32_t* size = &v[24];
    
    int32_t w = size[0]; //magic numerical constants... yuck.
    int32_t h = size[1];
    int32_t flip = (sdci[6]/10000)&3;
    int32_t tile = sdci[7]/10000;
    int32_t polytype = sdci[8]/10000;
    
    polytype = vbound(polytype, 0, 14);
    
	if(((w-1) & w) != 0 || ((h-1) & h) != 0)
	{
		Z_message("Quad3d() : Args h, w, must be in powers of two! Power of 2 error with %i, %i.", w, h);
		return; //non power of two error
	}
    
    int32_t tex_width = w*16;
    int32_t tex_height = h*16;
    
    bool mustDestroyBmp = false;
    BITMAP *tex = script_drawing_commands.GetSmallTextureBitmap(w,h);
    
    if(!tex)
    {
        mustDestroyBmp = true;
        tex = create_bitmap_ex(8, tex_width, tex_height);
        clear_bitmap(tex);
    }
    
    if(tile > 0)   // TILE
    {
        TileHelper::OverTile(tex, tile, 0, 0, w, h, col[0], flip);
    }
    else        // COMBO
    {
        auto& c = GET_DRAWING_COMBO(vbound(abs(tile), 0, 0xffff));
        const int32_t tiletodraw = combo_tile(c, 0, 0);
        flip = flip ^ c.flip;
        
        TileHelper::OldPutTile(tex, tiletodraw, 0, 0, w, h, col[0], flip);
    }
    
    V3D_f V1 = { static_cast<float>(pos[0]+xoffset), static_cast<float>(pos[1] +yoffset), static_cast<float>(pos[2]),  static_cast<float>(uv[0]), static_cast<float>(uv[1]), col[0] };
    V3D_f V2 = { static_cast<float>(pos[3]+xoffset), static_cast<float>(pos[4] +yoffset), static_cast<float>(pos[5]),  static_cast<float>(uv[2]), static_cast<float>(uv[3]), col[1] };
    V3D_f V3 = { static_cast<float>(pos[6]+xoffset), static_cast<float>(pos[7] +yoffset), static_cast<float>(pos[8]),  static_cast<float>(uv[4]), static_cast<float>(uv[5]), col[2] };
    V3D_f V4 = { static_cast<float>(pos[9]+xoffset), static_cast<float>(pos[10]+yoffset), static_cast<float>(pos[11]), static_cast<float>(uv[6]), static_cast<float>(uv[7]), col[3] };
    
    quad3d_f(bmp, polytype, tex, &V1, &V2, &V3, &V4);
    
    if(mustDestroyBmp)
        destroy_bitmap(tex);
        
}



void do_drawtriangle3dr(BITMAP *bmp, int32_t i, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=pos[9]
    //sdci[3]=uv[6]
    //sdci[4]=color[3]
    //sdci[5]=size[2]
    //sdci[6]=flip
    //sdci[7]=tile/combo
    //sdci[8]=polytype
    
    std::vector<int32_t>* v_ptr = (std::vector<int32_t>*)script_drawing_commands[i].GetPtr();
    
    if(!v_ptr)
    {
        al_trace("Triange3d: Vector pointer is null! Internal error. \n");
        return;
    }
    
    std::vector<int32_t> &v = *v_ptr;
    
    if(v.empty())
        return;
        
    int32_t* pos = &v[0];
    int32_t* uv = &v[9];
    int32_t* col = &v[15];
    int32_t* size = &v[18];
    
    int32_t w = size[0]; //magic numerical constants... yuck.
    int32_t h = size[1];
    int32_t flip = (sdci[6]/10000)&3;
    int32_t tile = sdci[7]/10000;
    int32_t polytype = sdci[8]/10000;
    
    polytype = vbound(polytype, 0, 14);
    
	if(((w-1) & w) != 0 || ((h-1) & h) != 0)
	{
		Z_message("Triangle3d() : Args h, w, must be in powers of two! Power of 2 error with %i, %i.", w, h);
		return; //non power of two error
	}
    
    int32_t tex_width = w*16;
    int32_t tex_height = h*16;
    
    bool mustDestroyBmp = false;
    BITMAP *tex = script_drawing_commands.GetSmallTextureBitmap(w,h);
    
    if(!tex)
    {
        mustDestroyBmp = true;
        tex = create_bitmap_ex(8, tex_width, tex_height);
        clear_bitmap(tex);
    }
    
    if(tile > 0)   // TILE
    {
        TileHelper::OverTile(tex, tile, 0, 0, w, h, col[0], flip);
    }
    else        // COMBO
    {
        auto& c = GET_DRAWING_COMBO(vbound(abs(tile), 0, 0xffff));
        const int32_t tiletodraw = combo_tile(c, 0, 0);
        flip = flip ^ c.flip;
        
        TileHelper::OldPutTile(tex, tiletodraw, 0, 0, w, h, col[0], flip);
    }
    
    V3D_f V1 = { static_cast<float>(pos[0]+xoffset), static_cast<float>(pos[1] +yoffset), static_cast<float>(pos[2]), static_cast<float>(uv[0]), static_cast<float>(uv[1]), col[0] };
    V3D_f V2 = { static_cast<float>(pos[3]+xoffset), static_cast<float>(pos[4] +yoffset), static_cast<float>(pos[5]), static_cast<float>(uv[2]), static_cast<float>(uv[3]), col[1] };
    V3D_f V3 = { static_cast<float>(pos[6]+xoffset), static_cast<float>(pos[7] +yoffset), static_cast<float>(pos[8]), static_cast<float>(uv[4]), static_cast<float>(uv[5]), col[2] };
    
    triangle3d_f(bmp, polytype, tex, &V1, &V2, &V3);
    
    if(mustDestroyBmp)
        destroy_bitmap(tex);
        
}

void bmp_do_rectr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	//Z_scripterrlog("rect sdci[13] is: %d\n", sdci[13]);
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=x2
    //sdci[5]=y2
    //sdci[6]=color
    //sdci[7]=scale factor
    //sdci[8]=rotation anchor x
    //sdci[9]=rotation anchor y
    //sdci[10]=rotation angle
    //sdci[11]=fill
    //sdci[12]=opacity
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
    if(sdci[7]==0)  //scale
    {
        return;
    }
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 ) 
    {
	Z_scripterrlog("bitmap->Rectangle() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
    BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
    
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
    
    int32_t x1=sdci[2]/10000;
    int32_t y1=sdci[3]/10000;
    int32_t x2=sdci[4]/10000;
    int32_t y2=sdci[5]/10000;
    
    
    if(x1>x2)
    {
        zc_swap(x1,x2);
    }
    
    if(y1>y2)
    {
        zc_swap(y1,y2);
    }
    
    if(sdci[7] != 10000)
    {
        int32_t w=x2-x1+1;
        int32_t h=y2-y1+1;
        int32_t w2=(w*sdci[7])/10000;
        int32_t h2=(h*sdci[7])/10000;
        x1=x1-((w2-w)/2);
        x2=x2+((w2-w)/2);
        y1=y1-((h2-h)/2);
        y2=y2+((h2-h)/2);
    }
    
    int32_t color=sdci[6]/10000;
    
    if(sdci[12]/10000<=127) //translucent
    {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    }
    
    if(sdci[10]==0) //no rotation
    {
        if(sdci[11]) //filled
        {
            rectfill(refbmp, x1+xoffset, y1+yoffset, x2+xoffset, y2+yoffset, color);
        }
        else //outline
        {
            rect(refbmp, x1+xoffset, y1+yoffset, x2+xoffset, y2+yoffset, color);
        }
    }
    else  //rotate
    {
        int32_t xy[16];
        int32_t rx=sdci[8]/10000;
        int32_t ry=sdci[9]/10000;
        fixed ra1=itofix(sdci[10]%10000)/10000;
        fixed ra2=itofix(sdci[10]/10000);
        fixed ra=ra1+ra2;
        ra = (ra/360)*256;
        
        fixed fcosa = fixcos(ra);
        fixed fsina = fixsin(ra);
        
        xy[ 0]=xoffset+rx + fixtoi((fcosa * (x1 - rx) - fsina * (y1 - ry)));     //x1
        xy[ 1]=yoffset+ry + fixtoi((fsina * (x1 - rx) + fcosa * (y1 - ry)));     //y1
        xy[ 2]=xoffset+rx + fixtoi((fcosa * (x2 - rx) - fsina * (y1 - ry)));     //x2
        xy[ 3]=yoffset+ry + fixtoi((fsina * (x2 - rx) + fcosa * (y1 - ry)));     //y1
        xy[ 4]=xoffset+rx + fixtoi((fcosa * (x2 - rx) - fsina * (y2 - ry)));     //x2
        xy[ 5]=yoffset+ry + fixtoi((fsina * (x2 - rx) + fcosa * (y2 - ry)));     //y2
        xy[ 6]=xoffset+rx + fixtoi((fcosa * (x1 - rx) - fsina * (y2 - ry)));     //x1
        xy[ 7]=yoffset+ry + fixtoi((fsina * (x1 - rx) + fcosa * (y2 - ry)));     //y2
        xy[ 8]=xoffset+rx + fixtoi((fcosa * (x1 - rx) - fsina * (y1 - ry + 1)));         //x1
        xy[ 9]=yoffset+ry + fixtoi((fsina * (x1 - rx) + fcosa * (y1 - ry + 1)));         //y1
        xy[10]=xoffset+rx + fixtoi((fcosa * (x2 - rx - 1) - fsina * (y1 - ry)));         //x2
        xy[11]=yoffset+ry + fixtoi((fsina * (x2 - rx - 1) + fcosa * (y1 - ry)));         //y1
        xy[12]=xoffset+rx + fixtoi((fcosa * (x2 - rx) - fsina * (y2 - ry - 1)));         //x2
        xy[13]=yoffset+ry + fixtoi((fsina * (x2 - rx) + fcosa * (y2 - ry - 1)));         //y2
        xy[14]=xoffset+rx + fixtoi((fcosa * (x1 - rx + 1) - fsina * (y2 - ry)));         //x1
        xy[15]=yoffset+ry + fixtoi((fsina * (x1 - rx + 1) + fcosa * (y2 - ry)));         //y2
        
        if(sdci[11]) //filled
        {
            polygon(refbmp, 4, xy, color);
        }
        else //outline
        {
            line(refbmp, xy[0], xy[1], xy[10], xy[11], color);
            line(refbmp, xy[2], xy[3], xy[12], xy[13], color);
            line(refbmp, xy[4], xy[5], xy[14], xy[15], color);
            line(refbmp, xy[6], xy[7], xy[ 8], xy[ 9], color);
        }
    }
    
    drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}

void bmp_do_framer(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=tile
    //sdci[5]=cset
    //sdci[6]=width
    //sdci[7]=height
    //sdci[8]=overlay
    //sdci[9]=opacity

	if ( sdci[DRAWCMD_BMP_TARGET] <= 0 ) 
	{
		Z_scripterrlog("bitmap->DrawFrame() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
		return;
	}
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;

	if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop.

    int32_t x=sdci[2]/10000;
    int32_t y=sdci[3]/10000;
    
    int32_t tile=sdci[4]/10000;
    int32_t cs=sdci[5]/10000;
    int32_t w=sdci[6]/10000;
    int32_t h=sdci[7]/10000;
    bool overlay=sdci[8];
    bool trans=(sdci[9]/10000<=127);
    
	frame2x2(refbmp, x + xoffset, y + yoffset, tile, cs, w, h, 0, overlay, trans);
}


void bmp_do_circler(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=radius
    //sdci[5]=color
    //sdci[6]=scale factor
    //sdci[7]=rotation anchor x
    //sdci[8]=rotation anchor y
    //sdci[9]=rotation angle
    //sdci[10]=fill
    //sdci[11]=opacity
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
    if(sdci[6]==0)  //scale
    {
        return;
    }
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
	Z_scripterrlog("bitmap->Circle() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
    BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
    
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
    
    int32_t x1=sdci[2]/10000;
    int32_t y1=sdci[3]/10000;
    qword r=sdci[4];
    
    if(sdci[6] != 10000)
    {
        r*=sdci[6];
        r/=10000;
    }
    
    r/=10000;
    int32_t color=sdci[5]/10000;
    
    if(sdci[11]/10000<=127) //translucent
    {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    }
    
    if(sdci[9]!=0&&(sdci[2]!=sdci[7]||sdci[3]!=sdci[8])) //rotation
    {
        int32_t xy[2];
        int32_t rx=sdci[7]/10000;
        int32_t ry=sdci[8]/10000;
        fixed ra1=itofix(sdci[9]%10000)/10000;
        fixed ra2=itofix(sdci[9]/10000);
        fixed ra=ra1+ra2;
        ra = (ra/360)*256;
        
        xy[ 0]=rx + fixtoi((fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry)));     //x1
        xy[ 1]=ry + fixtoi((fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry)));     //y1
        x1=xy[0];
        y1=xy[1];
    }
    
    if(sdci[10]) //filled
    {
        circlefill(refbmp, x1+xoffset, y1+yoffset, r, color);
    }
    else //outline
    {
        circle(refbmp, x1+xoffset, y1+yoffset, r, color);
    }
    
    drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}


void bmp_do_arcr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=radius
    //sdci[5]=start angle
    //sdci[6]=end angle
    //sdci[7]=color
    //sdci[8]=scale factor
    //sdci[9]=rotation anchor x
    //sdci[10]=rotation anchor y
    //sdci[11]=rotation angle
    //sdci[12]=closed
    //sdci[13]=fill
    //sdci[14]=opacity
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
    
    if(sdci[8]==0)  //scale
    {
        return;
    }
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
	Z_scripterrlog("bitmap->Arc() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
    BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
    
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
    
    int32_t cx=sdci[2]/10000;
    int32_t cy=sdci[3]/10000;
    qword r=sdci[4];
    
    if(sdci[8] != 10000)
    {
        r*=sdci[8];
        r/=10000;
    }
    
    r/=10000;
    
    int32_t color=sdci[7]/10000;
    
    fixed ra1=itofix(sdci[11]%10000)/10000;
    fixed ra2=itofix(sdci[11]/10000);
    fixed ra=ra1+ra2;
    ra = (ra/360)*256;
    
    
    fixed a1=itofix(sdci[5]%10000)/10000;
    fixed a2=itofix(sdci[5]/10000);
    fixed sa=a1+a2;
    sa = (sa/360)*256;
    
    a1=itofix(sdci[6]%10000)/10000;
    a2=itofix(sdci[6]/10000);
    fixed ea=a1+a2;
    ea = (ea/360)*256;
    
    if(sdci[11]!=0) //rotation
    {
        int32_t rx=sdci[9]/10000;
        int32_t ry=sdci[10]/10000;
        
        cx=rx + fixtoi((fixcos(ra) * (cx - rx) - fixsin(ra) * (cy - ry)));     //x1
        cy=ry + fixtoi((fixsin(ra) * (cx - rx) + fixcos(ra) * (cy - ry)));     //y1
        ea-=ra;
        sa-=ra;
    }
    
    int32_t fx=cx+fixtoi(fixcos(-(ea+sa)/2)*r/2);
    int32_t fy=cy+fixtoi(fixsin(-(ea+sa)/2)*r/2);
    
    if(sdci[12]) //closed
    {
        if(sdci[13]) //filled
        {
            clear_bitmap(prim_bmp);
            arc(prim_bmp, cx+xoffset, cy+yoffset, sa, ea, int32_t(r), color);
            line(prim_bmp, cx+xoffset, cy+yoffset, cx+xoffset+fixtoi(fixcos(-sa)*r), cy+yoffset+fixtoi(fixsin(-sa)*r), color);
            line(prim_bmp, cx+xoffset, cy+yoffset, cx+xoffset+fixtoi(fixcos(-ea)*r), cy+yoffset+fixtoi(fixsin(-ea)*r), color);
			int fillx = zc_max(0,fx)+xoffset;
			int filly = zc_max(0,fy)+yoffset;
			zprint2("bitmap->Arc fill at prim_bmp (%d,%d) - 512x512\n", fillx, filly);
			floodfill(prim_bmp, fillx, filly, color);
            
            if(sdci[14]/10000<=127) //translucent
            {
                draw_trans_sprite(refbmp, prim_bmp, 0,0);
            }
            else
            {
                draw_sprite(refbmp, prim_bmp, 0,0);
            }
        }
        else
        {
            arc(refbmp, cx+xoffset, cy+yoffset, sa, ea, int32_t(r), color);
            line(refbmp, cx+xoffset, cy+yoffset, cx+xoffset+fixtoi(fixcos(-sa)*r), cy+yoffset+fixtoi(fixsin(-sa)*r), color);
            line(refbmp, cx+xoffset, cy+yoffset, cx+xoffset+fixtoi(fixcos(-ea)*r), cy+yoffset+fixtoi(fixsin(-ea)*r), color);
        }
    }
    else
    {
        if(sdci[14]/10000<=127) //translucent
        {
            drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
        }
        
        arc(refbmp, cx+xoffset, cy+yoffset, sa, ea, int32_t(r), color);
        drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
    }
}


void bmp_do_ellipser(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=radiusx
    //sdci[5]=radiusy
    //sdci[6]=color
    //sdci[7]=scale factor
    //sdci[8]=rotation anchor x
    //sdci[9]=rotation anchor y
    //sdci[10]=rotation angle
    //sdci[11]=fill
    //sdci[12]=opacity
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
    
    if(sdci[7]==0)  //scale
    {
        return;
    }
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
	Z_scripterrlog("bitmap->Ellipse() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
    BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
    
    int32_t x1=sdci[2]/10000;
    int32_t y1=sdci[3]/10000;
    int32_t radx=sdci[4]/10000;
    radx*=sdci[7]/10000;
    int32_t rady=sdci[5]/10000;
    rady*=sdci[7]/10000;
    int32_t color=sdci[6]/10000;
    float rotation = sdci[10]/10000;
    
    int32_t rx=sdci[8]/10000;
    int32_t ry=sdci[9]/10000;
    fixed ra1=itofix(sdci[10]%10000)/10000;
    fixed ra2=itofix(sdci[10]/10000);
    fixed ra=ra1+ra2;
    ra = (ra/360)*256;
    
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
    
    int32_t xy[2];
    xy[ 0]=rx + fixtoi((fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry)));     //x1
    xy[ 1]=ry + fixtoi((fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry)));     //y1
    x1=xy[0];
    y1=xy[1];
    
    if(radx<1||rady<1||radx>255||rady>255) return;
    
    BITMAP* bitty = script_drawing_commands.AquireSubBitmap(radx*2+1, rady*2+1);
    
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
    
    if(sdci[11]) //filled
    {
    
        if(sdci[12]/10000<128) //translucent
        {
            clear_bitmap(prim_bmp);
            ellipsefill(bitty, radx, rady, radx, rady, color==0?255:color);
            rotate_sprite(prim_bmp, bitty, x1+xoffset-radx,y1+yoffset-rady, degrees_to_fixed(rotation));
            draw_trans_sprite(refbmp, prim_bmp, 0, 0);
        }
        else // no opacity
        {
            ellipsefill(bitty, radx, rady, radx, rady, color==0?255:color);
            rotate_sprite(refbmp, bitty, x1+xoffset-radx,y1+yoffset-rady, degrees_to_fixed(rotation));
        }
    }
    else //not filled
    {
        if(sdci[12]/10000<128) //translucent
        {
            clear_bitmap(prim_bmp);
            ellipse(bitty, radx, rady, radx, rady, color==0?255:color);
            rotate_sprite(prim_bmp, bitty, x1+xoffset-radx,y1+yoffset-rady, degrees_to_fixed(rotation));
            draw_trans_sprite(refbmp, prim_bmp, 0, 0);
        }
        else // no opacity
        {
            ellipse(bitty, radx, rady, radx, rady, color==0?255:color);
            rotate_sprite(refbmp, bitty, x1+xoffset-radx,y1+yoffset-rady, degrees_to_fixed(rotation));
        }
    }
    
    // Since 0 is the transparent color, the stuff above will fail if the ellipse color is also 0.
    // Instead, it uses color 255 and replaces it afterward. That'll also screw up color 255 around
    // the ellipse, but it shouldn't be used anyway.
    if(color==0)
    {
        // This is very slow, so check the smallest possible square
        int32_t endx=zc_min(bmp->w-1, x1+zc_max(radx, rady));
        int32_t endy=zc_min(bmp->h-1, y1+zc_max(radx, rady));
        
        for(int32_t y=zc_max(0, y1-zc_max(radx, rady)); y<=endy; y++)
            for(int32_t x=zc_max(0, x1-zc_max(radx, rady)); x<=endx; x++)
                if(getpixel(refbmp, x, y)==255)
                    putpixel(refbmp, x, y, 0);
    }
    
    script_drawing_commands.ReleaseSubBitmap(bitty);
}


void bmp_do_liner(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=x2
    //sdci[5]=y2
    //sdci[6]=color
    //sdci[7]=scale factor
    //sdci[8]=rotation anchor x
    //sdci[9]=rotation anchor y
    //sdci[10]=rotation angle
    //sdci[11]=opacity
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
    if(sdci[7]==0)  //scale
    {
        return;
    }
    
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
	Z_scripterrlog("bitmap->Line() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
    
    BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
    
    int32_t x1=sdci[2]/10000;
    int32_t y1=sdci[3]/10000;
    int32_t x2=sdci[4]/10000;
    int32_t y2=sdci[5]/10000;
    
    if(sdci[7] != 10000)
    {
        int32_t w=x2-x1+1;
        int32_t h=y2-y1+1;
        int32_t w2=int32_t(w*((double)sdci[7]/10000.0));
        int32_t h2=int32_t(h*((double)sdci[7]/10000.0));
        x1=x1-((w2-w)/2);
        x2=x2+((w2-w)/2);
        y1=y1-((h2-h)/2);
        y2=y2+((h2-h)/2);
    }
    
    int32_t color=sdci[6]/10000;
    
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
    
    if(sdci[11]/10000<=127) //translucent
    {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    }
    
    if(sdci[10]!=0) //rotation
    {
        int32_t xy[4];
        int32_t rx=sdci[8]/10000;
        int32_t ry=sdci[9]/10000;
        fixed ra1=itofix(sdci[10]%10000)/10000;
        fixed ra2=itofix(sdci[10]/10000);
        fixed ra=ra1+ra2;
        
        xy[ 0]=rx + fixtoi((fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry)));     //x1
        xy[ 1]=ry + fixtoi((fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry)));     //y1
        xy[ 2]=rx + fixtoi((fixcos(ra) * (x2 - rx) - fixsin(ra) * (y2 - ry)));     //x2
        xy[ 3]=ry + fixtoi((fixsin(ra) * (x2 - rx) + fixcos(ra) * (y2 - ry)));     //y2
        x1=xy[0];
        y1=xy[1];
        x2=xy[2];
        y2=xy[3];
    }
    
    line(refbmp, x1+xoffset, y1+yoffset, x2+xoffset, y2+yoffset, color);
    drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}


void bmp_do_spliner(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    /* layer, x1, y1, x2, y2, x3, y3, x4, y4, color, opacity */
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
    
    int32_t points[8] = {    xoffset + (sdci[2]/10000), yoffset + (sdci[3]/10000),
                         xoffset + (sdci[4]/10000), yoffset + (sdci[5]/10000),
                         xoffset + (sdci[6]/10000), yoffset + (sdci[7]/10000),
                         xoffset + (sdci[8]/10000), yoffset + (sdci[9]/10000)
                    };
                    
    if(sdci[11]/10000 < 128)   //translucent
    {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    }
    
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
	Z_scripterrlog("bitmap->Spline() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
    
    BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
    
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
    
    spline(refbmp, points, sdci[10]/10000);
    
    drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}


void bmp_do_putpixelr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=color
    //sdci[5]=rotation anchor x
    //sdci[6]=rotation anchor y
    //sdci[7]=rotation angle
    //sdci[8]=opacity
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
    int32_t x1=sdci[2]/10000;
    int32_t y1=sdci[3]/10000;
    int32_t color=sdci[4]/10000;
    
    if(sdci[8]/10000<=127) //translucent
    {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    }
    
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
	Z_scripterrlog("bitmap->PutPixel() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
    
    BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
    
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
    
    if(sdci[7]!=0) //rotation
    {
        int32_t xy[2];
        int32_t rx=sdci[5]/10000;
        int32_t ry=sdci[6]/10000;
        fixed ra1=itofix(sdci[7]%10000)/10000;
        fixed ra2=itofix(sdci[7]/10000);
        fixed ra=ra1+ra2;
        
        xy[ 0]=rx + fixtoi((fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry)));     //x1
        xy[ 1]=ry + fixtoi((fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry)));     //y1
        x1=xy[0];
        y1=xy[1];
    }
    
    putpixel(refbmp, x1+xoffset, y1+yoffset, color);
    drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}


void bmp_do_drawtiler(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=tile
    //sdci[5]=tile width
    //sdci[6]=tile height
    //sdci[7]=color (cset)
    //sdci[8]=scale x
    //sdci[9]=scale y
    //sdci[10]=rotation anchor x
    //sdci[11]=rotation anchor y
    //sdci[12]=rotation angle
    //sdci[13]=flip
    //sdci[14]=transparency
    //sdci[15]=opacity
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
    
    int32_t w = sdci[5]/10000;
    int32_t h = sdci[6]/10000;
    
    if(w < 1 || h < 1 || h > 20 || w > 20)
    {
        return;
    }
    
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
	Z_scripterrlog("bitmap->DrawTile() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
    
    BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
    
    int32_t xscale=sdci[8]/10000;
    int32_t yscale=sdci[9]/10000;
    int32_t rx = sdci[10]/10000;
    int32_t ry = sdci[11]/10000;
    float rotation=sdci[12]/10000;
    int32_t flip=(sdci[13]/10000)&3;
    bool transparency=sdci[14]!=0;
    int32_t opacity=sdci[15]/10000;
    int32_t color=sdci[7]/10000;
    
    int32_t x1=sdci[2]/10000;
    int32_t y1=sdci[3]/10000;
    
    
    //don't scale if it's not safe to do so
    bool canscale = true;
    
    if(xscale==0||yscale==0)
    {
        return;
    }
    
    if(xscale<0||yscale<0)
        canscale = false; //default size
        
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
    
    if((xscale>0 && yscale>0) || rotation)   //scaled or rotated
    {
        BITMAP* pbitty = script_drawing_commands.AquireSubBitmap(w*16, h*16);
        
        if(transparency) //transparency
        {
            TileHelper::OverTile(pbitty, (sdci[4]/10000), 0, 0, w, h, color, flip);
        }
        else //no transparency
        {
            TileHelper::OldPutTile(pbitty, (sdci[4]/10000), 0, 0, w, h, color, flip);
        }
        
        if(rotation != 0)
        {
            //low negative values indicate no anchor-point rotation
            if(rx>-777||ry>-777)
            {
                int32_t xy[2];
                fixed ra1=itofix(sdci[12]%10000)/10000;
                fixed ra2=itofix(sdci[12]/10000);
                fixed ra=ra1+ra2;
                xy[ 0]=rx + fixtoi((fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry)));     //x1
                xy[ 1]=ry + fixtoi((fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry)));     //y1
                x1=xy[0];
                y1=xy[1];
            }
            
            if(canscale) //scale first
            {
                //damnit all, .. fixme.
                BITMAP* tempbit = create_bitmap_ex(8, xscale>512?512:xscale, yscale>512?512:yscale);
                clear_bitmap(tempbit);
                
                stretch_sprite(tempbit, pbitty, 0, 0, xscale, yscale);
                
                if(opacity < 128)
                {
                    clear_bitmap(prim_bmp);
                    rotate_sprite(prim_bmp, tempbit, 0, 0, degrees_to_fixed(rotation));
                    draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
                }
                else
                {
                    rotate_sprite(refbmp, tempbit, x1+xoffset, y1+yoffset, degrees_to_fixed(rotation));
                }
                
                destroy_bitmap(tempbit);
            }
            else //no scale
            {
                if(opacity < 128)
                {
                    clear_bitmap(prim_bmp);
                    rotate_sprite(prim_bmp, pbitty, 0, 0, degrees_to_fixed(rotation));
                    draw_trans_sprite(refbmp, prim_bmp, x1+xoffset, y1+yoffset);
                }
                else
                {
                    rotate_sprite(refbmp, pbitty, x1+xoffset, y1+yoffset, degrees_to_fixed(rotation));
                }
            }
        }
        else //scale only
        {
            if(canscale)
            {
                if(opacity<128)
                {
                    clear_bitmap(prim_bmp);
                    stretch_sprite(prim_bmp, pbitty, 0, 0, xscale, yscale);
                    draw_trans_sprite(refbmp, prim_bmp, x1+xoffset, y1+yoffset);
                }
                else
                {
                    stretch_sprite(refbmp, pbitty, x1+xoffset, y1+yoffset, xscale, yscale);
                }
            }
            else //error -do not scale
            {
                if(opacity<128)
                {
                    draw_trans_sprite(refbmp, prim_bmp, x1+xoffset, y1+yoffset);
                }
                else
                {
                    draw_sprite(refbmp, pbitty, x1+xoffset, y1+yoffset);
                }
            }
        }
        
        script_drawing_commands.ReleaseSubBitmap(pbitty);
        
    }
    else // no scale or rotation
    {
        if(transparency)
        {
            if(opacity<=127)
                TileHelper::OverTileTranslucent(refbmp, (sdci[4]/10000), xoffset+x1, yoffset+y1, w, h, color, flip, opacity);
            else
                TileHelper::OverTile(refbmp, (sdci[4]/10000), xoffset+x1, yoffset+y1, w, h, color, flip);
        }
        else
        {
            if(opacity<=127)
                TileHelper::PutTileTranslucent(refbmp, (sdci[4]/10000), xoffset+x1, yoffset+y1, w, h, color, flip, opacity);
            else
                TileHelper::OldPutTile(refbmp, (sdci[4]/10000), xoffset+x1, yoffset+y1, w, h, color, flip);
        }
    }
}

void bmp_do_drawtilecloakedr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	//sdci[1]=layer
	//sdci[2]=x
	//sdci[3]=y
	//sdci[4]=tile
	//sdci[5]=tile width
	//sdci[6]=tile height
	//sdci[7]=flip
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	
	int32_t w = sdci[5]/10000;
	int32_t h = sdci[6]/10000;
	
	if(w < 1 || h < 1 || h > 20 || w > 20)
	{
		return;
	}
	
	if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
	{
		Z_scripterrlog("bitmap->DrawTileCloaked() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
		return;
	}
	
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
	
	int32_t flip=(sdci[7]/10000)&3;
	
	int32_t x1=sdci[2]/10000;
	int32_t y1=sdci[3]/10000;
		
	if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
	
	TileHelper::OverTileCloaked(refbmp, (sdci[4]/10000), xoffset+x1, yoffset+y1, w, h, flip);
}


void bmp_do_drawcombor(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=combo
    //sdci[5]=tile width
    //sdci[6]=tile height
    //sdci[7]=color (cset)
    //sdci[8]=scale x
    //sdci[9]=scale y
    //sdci[10]=rotation anchor x
    //sdci[11]=rotation anchor y
    //sdci[12]=rotation angle
    //sdci[13]=frame
    //sdci[14]=flip
    //sdci[15]=transparency
    //sdci[16]=opacity
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
    int32_t w = sdci[5]/10000;
    int32_t h = sdci[6]/10000;
    
    if(w<1||h<1||h>20||w>20)
    {
        return;
    }
    
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
	Z_scripterrlog("bitmap->DrawCombo() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
    
    BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
    if ( refbmp == NULL ) return;
	int32_t cmb = (sdci[4]/10000);
	if((unsigned)cmb >= MAXCOMBOS)
	{
		Z_scripterrlog("DrawCombo() cannot draw combo '%d', as it is out of bounds.\n", cmb);
		return;
	}
    
    int32_t xscale=sdci[8]/10000;
    int32_t yscale=sdci[9]/10000;
    int32_t rx = sdci[10]/10000; //these work now
    int32_t ry = sdci[11]/10000; //these work now
    float rotation=sdci[12]/10000;
    
    bool transparency=sdci[15]!=0;
    int32_t opacity=sdci[16]/10000;
    int32_t color=sdci[7]/10000;
    int32_t x1=sdci[2]/10000;
    int32_t y1=sdci[3]/10000;
    
    auto& c = GET_DRAWING_COMBO(cmb);
    int32_t tiletodraw = combo_tile(c, x1, y1);
    int32_t flip = ((sdci[14]/10000) & 3) ^ c.flip;
    int32_t skiprows=c.skipanimy;
    
    
    //don't scale if it's not safe to do so
    bool canscale = true;
    
    if(xscale==0||yscale==0)
    {
        return;
    }
    
    if(xscale<0||yscale<0)
        canscale = false; //default size
        
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
    
    if((xscale>0 && yscale>0) || rotation)   //scaled or rotated
    {
        BITMAP* pbitty = script_drawing_commands.AquireSubBitmap(w*16, h*16);   //-pbitty in the hisouse. :D
        
        if(transparency)
        {
            TileHelper::OverTile(pbitty, tiletodraw, 0, 0, w, h, color, flip, skiprows);
        }
        else //no transparency
        {
            TileHelper::OldPutTile(pbitty, tiletodraw, 0, 0, w, h, color, flip, skiprows);
        }
        
        if(rotation != 0) // rotate
        {
            //fixed point sucks ;0
            if(rx>-777||ry>-777) //set the rotation anchor and rotate around that
            {
                int32_t xy[2];
                fixed ra1=itofix(sdci[12]%10000)/10000;
                fixed ra2=itofix(sdci[12]/10000);
                fixed ra=ra1+ra2;
                xy[ 0]=rx + fixtoi((fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry)));     //x1
                xy[ 1]=ry + fixtoi((fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry)));     //y1
                x1=xy[0];
                y1=xy[1];
            }
            
            if(canscale) //scale first
            {
                BITMAP* tempbit = create_bitmap_ex(8, xscale>512?512:xscale, yscale>512?512:yscale);
                clear_bitmap(tempbit);
                
                stretch_sprite(tempbit, pbitty, 0, 0, xscale, yscale);
                
                if(opacity < 128)
                {
                    clear_bitmap(prim_bmp);
                    rotate_sprite(prim_bmp, tempbit, 0, 0, degrees_to_fixed(rotation));
                    draw_trans_sprite(refbmp, prim_bmp, x1+xoffset, y1+yoffset);
                }
                else
                {
                    rotate_sprite(refbmp, tempbit, x1+xoffset, y1+yoffset, degrees_to_fixed(rotation));
                }
                
                destroy_bitmap(tempbit);
            }
            else //no scale
            {
                if(opacity < 128)
                {
                    clear_bitmap(prim_bmp);
                    rotate_sprite(prim_bmp, pbitty, 0, 0, degrees_to_fixed(rotation));
                    draw_trans_sprite(refbmp, prim_bmp, x1+xoffset, y1+yoffset);
                }
                else
                {
                    rotate_sprite(refbmp, pbitty, x1+xoffset, y1+yoffset, degrees_to_fixed(rotation));
                }
            }
        }
        else //scale only
        {
            if(canscale)
            {
                if(opacity<128)
                {
                    clear_bitmap(prim_bmp);
                    stretch_sprite(prim_bmp, pbitty, 0, 0, xscale, yscale);
                    draw_trans_sprite(refbmp, prim_bmp, x1+xoffset, y1+yoffset);
                }
                else
                {
                    stretch_sprite(refbmp, pbitty, x1+xoffset, y1+yoffset, xscale, yscale);
                }
            }
            else //error -do not scale
            {
                if(opacity<128)
                {
                    draw_trans_sprite(refbmp, prim_bmp, x1+xoffset, y1+yoffset);
                }
                else
                {
                    draw_sprite(refbmp, pbitty, x1+xoffset, y1+yoffset);
                }
            }
        }
        
        script_drawing_commands.ReleaseSubBitmap(pbitty); //rap sucks
    }
    else // no scale or rotation
    {
        if(transparency)
        {
            if(opacity<=127)
                TileHelper::OverTileTranslucent(refbmp, tiletodraw, xoffset+x1, yoffset+y1, w, h, color, flip, opacity, skiprows);
            else
                TileHelper::OverTile(refbmp, tiletodraw, xoffset+x1, yoffset+y1, w, h, color, flip, skiprows);
        }
        else
        {
            if(opacity<=127)
                TileHelper::PutTileTranslucent(refbmp, tiletodraw, xoffset+x1, yoffset+y1, w, h, color, flip, opacity, skiprows);
            else
                TileHelper::OldPutTile(refbmp, tiletodraw, xoffset+x1, yoffset+y1, w, h, color, flip, skiprows);
        }
    }
}


void bmp_do_drawcombocloakedr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	//sdci[1]=layer
	//sdci[2]=x
	//sdci[3]=y
	//sdci[4]=combo
	//sdci[5]=tile width
	//sdci[6]=tile height
	//sdci[7]=flip
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	
	int32_t w = sdci[5]/10000;
	int32_t h = sdci[6]/10000;
	
	if(w<1||h<1||h>20||w>20)
	{
		return;
	}
	
	if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
	{
		Z_scripterrlog("bitmap->DrawComboCloaked() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
		return;
	}
	
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
	int32_t cmb = (sdci[4]/10000);
	if((unsigned)cmb >= MAXCOMBOS)
	{
		Z_scripterrlog("DrawComboCloaked() cannot draw combo '%d', as it is out of bounds.\n", cmb);
		return;
	}
	
	if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
	
	int32_t x1=sdci[2]/10000;
	int32_t y1=sdci[3]/10000;
	
	auto& c = GET_DRAWING_COMBO(cmb);
	int32_t tiletodraw = combo_tile(c, x1, y1);
	int32_t flip = ((sdci[7]/10000) & 3) ^ c.flip;
	int32_t skiprows=c.skipanimy;
	
	TileHelper::OverTileCloaked(refbmp, tiletodraw, xoffset+x1, yoffset+y1, w, h, flip, skiprows);
}


void bmp_do_fasttiler(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    /* layer, x, y, tile, color opacity */
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
    
    int32_t opacity = sdci[6]/10000;
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
	Z_scripterrlog("bitmap->FastTile() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
    
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 

	int x = xoffset+(sdci[2]/10000);
	int y = yoffset+(sdci[3]/10000);
    
    if(opacity < 128)
        overtiletranslucent16(refbmp, sdci[4]/10000, x, y, sdci[5]/10000, 0, opacity);
    else
        overtile16(refbmp, sdci[4]/10000, x, y, sdci[5]/10000, 0);
}

void do_bmpwritetile(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	/* layer, x, y, tile, is8bit, mask */
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
	{
		Z_scripterrlog("bitmap->WriteTile() wanted to read from an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
		return;
	}
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
	
	if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
	
	int32_t x = (sdci[2]/10000), y = (sdci[3]/10000), tl = (sdci[4]/10000);
	bool is8bit = sdci[5]!=0, mask = sdci[6]!=0;
	
	write_tile(newtilebuf, refbmp, tl, x+xoffset, y+yoffset, is8bit, mask);
}

void do_bmpdither(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	/* layer, mask, color, ditherType, ditherArg */
	//sdci[2] Mask Bitmap Pointer
	//sdci[3] Color
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
	{
		Z_scripterrlog("bitmap->Dither() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
		return;
	}
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
	if ( sdci[2] <= 0 )
	{
		Z_scripterrlog("bitmap->Dither() wanted to read from an invalid bitmap id: %d. Aborting.\n", sdci[2]);
		return;
	}
	BITMAP *mask = FFCore.GetScriptBitmap(sdci[2]);
	if ( mask == NULL ) return;
	
	int32_t dType = sdci[4] / 10000L;
	if(dType < 0 || dType >= dithMax)
	{
		Z_scripterrlog("bitmap->Dither() used an invalid dither type: %d. Aborting.\n", dType);
		return;
	}
	
	ditherblit(refbmp, mask, byte(sdci[3]/10000L), dType, sdci[5]/10000L);
}

void do_bmpreplcol(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	/* layer, shift, startcol, endcol */
	//sdci[2] NewCol
	//sdci[3] StartCol
	//sdci[4] EndCol
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
	{
		Z_scripterrlog("bitmap->ReplaceColors() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
		return;
	}
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
	replColor(refbmp, sdci[2]/10000L, sdci[3]/10000L, sdci[4]/10000L, false);
}

void do_bmpshiftcol(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	/* layer, shift, startcol, endcol */
	//sdci[2] ShiftAmount
	//sdci[3] StartCol
	//sdci[4] EndCol
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
	{
		Z_scripterrlog("bitmap->ShiftColors() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
		return;
	}
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
	replColor(refbmp, sdci[2]/10000L, sdci[3]/10000L, sdci[4]/10000L, true);
}

void do_bmpmaskdraw(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	/* layer, mask, color */
	//sdci[2] Mask Bitmap Pointer
	//sdci[3] Color
	//sdci[4] start mask color
	//sdci[5] end mask color
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL )
	{
		Z_scripterrlog("bitmap->MaskDraw() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
		return;
	}
	BITMAP *mask = FFCore.GetScriptBitmap(sdci[2]);
	if ( mask == NULL )
	{
		Z_scripterrlog("bitmap->MaskDraw() wanted to read from an invalid bitmap id: %d. Aborting.\n", sdci[2]);
		return;
	}
	auto fillcol = sdci[3]/10000L;
	if(unsigned(fillcol) > 0xFF) return; //invalid color, nothing to draw
	auto startcol = vbound(sdci[4]/10000L,0x00,0xFF);
	auto endcol = vbound(sdci[5]/10000L,0x00,0xFF);
	mask_colorfill(refbmp, mask, fillcol, startcol, endcol);
}

void do_bmpmaskblit(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	/* layer, mask, color */
	//sdci[2] Mask Bitmap Pointer
	//sdci[3] Pattern Bitmap
	//sdci[4] bool 'pattern repeats'
	//sdci[5] start mask color
	//sdci[6] end mask color
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL )
	{
		Z_scripterrlog("bitmap->MaskDraw() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
		return;
	}
	BITMAP *mask = FFCore.GetScriptBitmap(sdci[2]);
	if ( mask == NULL )
	{
		Z_scripterrlog("bitmap->MaskDraw() wanted to read from an invalid bitmap (mask) id: %d. Aborting.\n", sdci[2]);
		return;
	}
	BITMAP *pattern = FFCore.GetScriptBitmap(sdci[3]);
	if ( pattern == NULL )
	{
		Z_scripterrlog("bitmap->MaskDraw() wanted to read from an invalid bitmap (pattern) id: %d. Aborting.\n", sdci[3]);
		return;
	}
	bool repeats = sdci[4]!=0;
	auto startcol = vbound(sdci[5]/10000L,0x00,0xFF);
	auto endcol = vbound(sdci[6]/10000L,0x00,0xFF);
	mask_blit(refbmp, mask, pattern, repeats, startcol, endcol);
}

void bmp_do_fastcombor(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    /* layer, x, y, tile, color opacity */
    //sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
    int32_t opacity = sdci[6] / 10000;
    int32_t x1 = sdci[2] / 10000;
    int32_t y1 = sdci[3] / 10000;
    int32_t index = sdci[4]/10000;
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
	Z_scripterrlog("bitmap->FastCombo() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
	int32_t cmb = (sdci[4]/10000);
	if((unsigned)cmb >= MAXCOMBOS)
	{
		Z_scripterrlog("FastCombo() cannot draw combo '%d', as it is out of bounds.\n", cmb);
		return;
	}
    
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 

	int x = xoffset+x1;
	int y = yoffset+y1;
	
	if(opacity < 128)
	{
		overcomboblocktranslucent(refbmp, x, y, cmb, sdci[5]/10000, 1, 1, 128);

	}
	else
	{
		overcomboblock(refbmp, x, y, cmb, sdci[5]/10000, 1, 1);
	}
}



void bmp_do_drawcharr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
	{
		Z_scripterrlog("bitmap->DrawCharacter() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
		return;
	}
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
	
	if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
	
	//broken 2.50.2 and earlier drawcharacter()
	if ( get_qr(qr_BROKENCHARINTDRAWING) )
	{
		//sdci[1]=layer
		    //sdci[2]=x
		    //sdci[3]=y
		    //sdci[4]=font
		    //sdci[5]=color
		    //sdci[6]=bg color
		    //sdci[7]=strech x (width)
		    //sdci[8]=stretch y (height)
		    //sdci[9]=char
		    //sdci[10]=opacity
		//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
		    
		    int32_t x=sdci[2]/10000;
		    int32_t y=sdci[3]/10000;
		    int32_t font_index=sdci[4]/10000;
		    int32_t color=sdci[5]/10000;
		    int32_t bg_color=sdci[6]/10000; //-1 = transparent
		    int32_t w=sdci[7]/10000;
		    int32_t h=sdci[8]/10000;
		    char glyph=char(sdci[9]/10000);
		    int32_t opacity=sdci[10]/10000;
		    
		    //safe check
		    if(bg_color < -1) bg_color = -1;
		    
		    if(w>512) w=512; //w=vbound(w,0,512);
		    
		    if(h>512) h=512; //h=vbound(h,0,512);
		    
		    //undone
		    if(w>0&&h>0)//stretch the character
		    {
			BITMAP *pbmp = script_drawing_commands.GetSmallTextureBitmap(1,1);
			
			if(opacity < 128)
			{
			    if(w>128||h>128)
			    {
				clear_bitmap(prim_bmp);
				
				textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
				stretch_sprite(prim_bmp, pbmp, 0, 0, w, h);
				draw_trans_sprite(refbmp, prim_bmp, x+xoffset, y+yoffset);
			    }
			    else //this is faster
			    {
				BITMAP *pbmp2 = script_drawing_commands.AquireSubBitmap(w,h);
				
				textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
				stretch_sprite(pbmp2, pbmp, 0, 0, w, h);
				draw_trans_sprite(refbmp, pbmp2, x+xoffset, y+yoffset);
				
				script_drawing_commands.ReleaseSubBitmap(pbmp2);
			    }
			}
			else // no opacity
			{
			    textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
			    stretch_sprite(refbmp, pbmp, x+xoffset, y+yoffset, w, h);
			}
			
		    }
		    else //no stretch
		    {
			if(opacity < 128)
			{
			    BITMAP *pbmp = create_sub_bitmap(prim_bmp,0,0,16,16);
			    clear_bitmap(pbmp);
			    
			    textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
			    draw_trans_sprite(refbmp, pbmp, x+xoffset, y+yoffset);
			    
			    destroy_bitmap(pbmp);
			}
			else // no opacity
			{
			    textprintf_ex(refbmp, get_zc_font(font_index), x+xoffset, y+yoffset, color, bg_color, "%c", glyph);
			}
		    }		
	}
	
	else //2.53.0 fixed version and later.
	{
	
		//sdci[1]=layer
		    //sdci[2]=x
		    //sdci[3]=y
		    //sdci[4]=font
		    //sdci[5]=color
		    //sdci[6]=bg color
		    //sdci[7]=strech x (width)
		    //sdci[8]=stretch y (height)
		    //sdci[9]=char
		    //sdci[10]=opacity
		    
		    int32_t x=sdci[2]/10000;
		    int32_t y=sdci[3]/10000;
		    int32_t font_index=sdci[4]/10000;
		    int32_t color=sdci[5]/10000;
		    int32_t bg_color=sdci[6]/10000; //-1 = transparent
		    int32_t w=sdci[7]/10000;
		    int32_t h=sdci[8]/10000;
		    char glyph=char(sdci[9]/10000);
		    int32_t opacity=sdci[10]/10000;
		    
		    //safe check
		    if(bg_color < -1) bg_color = -1;
		    
		    if(w>512) w=512; //w=vbound(w,0,512);
		    
		    if(h>512) h=512; //h=vbound(h,0,512);
		    
		    //undone
		    if(w>0&&h>0)//stretch the character
		    {
			BITMAP *pbmp = script_drawing_commands.GetSmallTextureBitmap(1,1);
			
			if(opacity < 128)
			{
			    if(w>128||h>128)
			    {
				clear_bitmap(prim_bmp);
				
				textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
				stretch_sprite(prim_bmp, pbmp, 0, 0, w, h);
				draw_trans_sprite(refbmp, prim_bmp, x+xoffset, y+yoffset);
			    }
			    else //this is faster
			    {
				BITMAP *pbmp2 = script_drawing_commands.AquireSubBitmap(w,h);
				
				textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
				stretch_sprite(pbmp2, pbmp, 0, 0, w, h);
				draw_trans_sprite(refbmp, pbmp2, x+xoffset, y+yoffset);
				
				script_drawing_commands.ReleaseSubBitmap(pbmp2);
			    }
			}
			else // no opacity
			{
			    textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
			    stretch_sprite(refbmp, pbmp, x+xoffset, y+yoffset, w, h);
			}
			
		    }
		    else //no stretch
		    {
			if(opacity < 128)
			{
			    BITMAP *pbmp = create_sub_bitmap(prim_bmp,0,0,16,16);
			    clear_bitmap(pbmp);
			    
			    textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
			    draw_trans_sprite(refbmp, pbmp, x+xoffset, y+yoffset);
			    
			    destroy_bitmap(pbmp);
			}
			else // no opacity
			{
			    textprintf_ex(refbmp, get_zc_font(font_index), x+xoffset, y+yoffset, color, bg_color, "%c", glyph);
			}
		    }		
		
	}
    
}


void bmp_do_drawintr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
	{
		Z_scripterrlog("bitmap->DrawInteger() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
		return;
	}
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
	
	if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
	
	//broken 2.50.2 and earlier drawinteger()
	if ( get_qr(qr_BROKENCHARINTDRAWING) )
	{
	    //sdci[1]=layer
	    //sdci[2]=x
	    //sdci[3]=y
	    //sdci[4]=font
	    //sdci[5]=color
	    //sdci[6]=bg color
	    //sdci[7]=strech x (width)
	    //sdci[8]=stretch y (height)
	    //sdci[9]=integer
	    //sdci[10]=num decimal places
	    //sdci[11]=opacity
		//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	    
	    int32_t x=sdci[2]/10000;
	    int32_t y=sdci[3]/10000;
	    int32_t font_index=sdci[4]/10000;
	    int32_t color=sdci[5]/10000;
	    int32_t bg_color=sdci[6]/10000; //-1 = transparent
	    int32_t w=sdci[7]/10000;
	    int32_t h=sdci[8]/10000;
	    int32_t decplace=sdci[10]/10000;
	    int32_t opacity=sdci[11]/10000;
	    
	    //safe check
	    if(bg_color < -1) bg_color = -1;
	    
	    if(w>512) w=512; //w=vbound(w,0,512);
	    
	    if(h>512) h=512; //h=vbound(h,0,512);
	    
	    char numbuf[15];
	    
		switch(decplace)
		{
		    default:
		    case 0:
			sprintf(numbuf,"%d",(sdci[9]/10000)); //For some reason, static casting for zero decimal places was
			break;					//reducing the value by -1, so 8.000 printed as '7'. -Z
			
		    case 1:
			//sprintf(numbuf,"%.01f",number);
			sprintf(numbuf,"%.01f",(static_cast<float>(sdci[9])/10000.0f)); //Would this be slower? 
			break;
			
		    case 2:
			//sprintf(numbuf,"%.02f",number);
			sprintf(numbuf,"%.02f",(static_cast<float>(sdci[9])/10000.0f));
			break;
			
		    case 3:
			//sprintf(numbuf,"%.03f",number);
			sprintf(numbuf,"%.03f",(static_cast<float>(sdci[9])/10000.0f));
			break;
			
		    case 4:
			//sprintf(numbuf,"%.04f",number);
			sprintf(numbuf,"%.04f",(static_cast<float>(sdci[9])/10000.0f));
			break;
		}
	    
	    if(w>0&&h>0)//stretch
	    {
		BITMAP *pbmp = script_drawing_commands.GetSmallTextureBitmap(1,1);
		
		if(opacity < 128)
		{
		    if(w>128||h>128)
		    {
			clear_bitmap(prim_bmp);
			
			textout_ex(pbmp, get_zc_font(font_index), numbuf, 0, 0, color, bg_color);
			stretch_sprite(prim_bmp, pbmp, 0, 0, w, h);
			draw_trans_sprite(refbmp, prim_bmp, x+xoffset, y+yoffset);
		    }
		    else
		    {
			BITMAP *pbmp2 = create_sub_bitmap(prim_bmp,0,0,w,h);
			clear_bitmap(pbmp2);
			
			textout_ex(pbmp, get_zc_font(font_index), numbuf, 0, 0, color, bg_color);
			stretch_sprite(pbmp2, pbmp, 0, 0, w, h);
			draw_trans_sprite(refbmp, pbmp2, x+xoffset, y+yoffset);
			
			destroy_bitmap(pbmp2);
		    }
		}
		else // no opacity
		{
		    textout_ex(pbmp, get_zc_font(font_index), numbuf, 0, 0, color, bg_color);
		    stretch_sprite(refbmp, pbmp, x+xoffset, y+yoffset, w, h);
		}
		
	    }
	    else //no stretch
	    {
		if(opacity < 128)
		{
		    BITMAP *pbmp = create_sub_bitmap(prim_bmp,0,0,16,16);
		    clear_bitmap(pbmp);
		    
		    textout_ex(pbmp, get_zc_font(font_index), numbuf, 0, 0, color, bg_color);
		    draw_trans_sprite(refbmp, pbmp, x+xoffset, y+yoffset);
		    
		    destroy_bitmap(pbmp);
		}
		else // no opacity
		{
		    textout_ex(refbmp, get_zc_font(font_index), numbuf, x+xoffset, y+yoffset, color, bg_color);
		}
	    }
		
	}
	
	else //2.53.0 fixed version and later.
	{
	    //sdci[1]=layer
	    //sdci[2]=x
	    //sdci[3]=y
	    //sdci[4]=font
	    //sdci[5]=color
	    //sdci[6]=bg color
	    //sdci[7]=strech x (width)
	    //sdci[8]=stretch y (height)
	    //sdci[9]=integer
	    //sdci[10]=num decimal places
	    //sdci[11]=opacity
	    
	    int32_t x=sdci[2]/10000;
	    int32_t y=sdci[3]/10000;
	    int32_t font_index=sdci[4]/10000;
	    int32_t color=sdci[5]/10000;
	    int32_t bg_color=sdci[6]/10000; //-1 = transparent
	    int32_t w=sdci[7]/10000;
	    int32_t h=sdci[8]/10000;
	    int32_t decplace=sdci[10]/10000;
	    int32_t opacity=sdci[11]/10000;
	    
	    //safe check
	    if(bg_color < -1) bg_color = -1;
	    
	    if(w>512) w=512; //w=vbound(w,0,512);
	    
	    if(h>512) h=512; //h=vbound(h,0,512);
	    
	    char numbuf[15];
	    
	    switch(decplace)
	    {
	    default:
	    case 0:
		sprintf(numbuf,"%d",(sdci[9]/10000)); //For some reason, static casting for zero decimal places was
		break;					//reducing the value by -1, so 8.000 printed as '7'. -Z
		
	    case 1:
		//sprintf(numbuf,"%.01f",number);
		sprintf(numbuf,"%.01f",(static_cast<float>(sdci[9])/10000.0f)); //Would this be slower? 
		break;
		
	    case 2:
		//sprintf(numbuf,"%.02f",number);
		sprintf(numbuf,"%.02f",(static_cast<float>(sdci[9])/10000.0f));
		break;
		
	    case 3:
		//sprintf(numbuf,"%.03f",number);
		sprintf(numbuf,"%.03f",(static_cast<float>(sdci[9])/10000.0f));
		break;
		
	    case 4:
		//sprintf(numbuf,"%.04f",number);
		sprintf(numbuf,"%.04f",(static_cast<float>(sdci[9])/10000.0f));
		break;
	    }
	    
	    //FONT* font=get_zc_font(sdci[4]/10000);
	    
	    if(w>0&&h>0)//stretch
	    {
		BITMAP *pbmp = create_sub_bitmap(prim_bmp, 0, 0, text_length(get_zc_font(font_index), numbuf)+1, text_height(get_zc_font(font_index)));
		clear_bitmap(pbmp);
		    //script_drawing_commands.GetSmallTextureBitmap(1,1);
		
		if(opacity < 128)
		{
		    if(w>128||h>128)
		    {
			clear_bitmap(prim_bmp);
			
			textout_ex(pbmp, get_zc_font(font_index), numbuf, 0, 0, color, bg_color);
			stretch_sprite(prim_bmp, pbmp, 0, 0, w, h);
			draw_trans_sprite(refbmp, prim_bmp, x+xoffset, y+yoffset);
		    }
		    else
		    {
			BITMAP *pbmp2 = create_sub_bitmap(prim_bmp,0,0,w,h);
			clear_bitmap(pbmp2);
			
			textout_ex(pbmp, get_zc_font(font_index), numbuf, 0, 0, color, bg_color);
			stretch_sprite(pbmp2, pbmp, 0, 0, w, h);
			draw_trans_sprite(refbmp, pbmp2, x+xoffset, y+yoffset);
			
			destroy_bitmap(pbmp2);
		    }
		}
		else // no opacity
		{
		    textout_ex(pbmp, get_zc_font(font_index), numbuf, 0, 0, color, bg_color);
		    stretch_sprite(refbmp, pbmp, x+xoffset, y+yoffset, w, h);
		}
		
	    }
	    else //no stretch
	    {
		if(opacity < 128)
		{
		    FONT* font = get_zc_font(font_index);
		    BITMAP *pbmp = create_sub_bitmap(prim_bmp, 0, 0, text_length(font, numbuf), text_height(font));
		    clear_bitmap(pbmp);
		    
		    textout_ex(pbmp, font, numbuf, 0, 0, color, bg_color);
		    draw_trans_sprite(refbmp, pbmp, x+xoffset, y+yoffset);
		    
		    destroy_bitmap(pbmp);
		}
		else // no opacity
		{
		    textout_ex(refbmp, get_zc_font(font_index), numbuf, x+xoffset, y+yoffset, color, bg_color);
		}
	    }
	}
}


void bmp_do_drawstringr(BITMAP *bmp, int32_t i, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=font
    //sdci[5]=color
    //sdci[6]=bg color
    //sdci[7]=format_option
    //sdci[8]=string
    //sdci[9]=opacity
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
	Z_scripterrlog("bitmap->DrawString() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
	
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
    
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
    
    std::string* str = (std::string*)script_drawing_commands[i].GetPtr();
    
    if(!str)
    {
        al_trace("String pointer is null! Internal error. \n");
        return;
    }
    
    int32_t x=sdci[2]/10000;
    int32_t y=sdci[3]/10000;
    FONT* font=get_zc_font(sdci[4]/10000);
    int32_t color=sdci[5]/10000;
    int32_t bg_color=sdci[6]/10000; //-1 = transparent
    int32_t format_type=sdci[7]/10000;
    int32_t opacity=sdci[9]/10000;
    //sdci[8] not needed :)
    
    //safe check
    if(bg_color < -1) bg_color = -1;
    
    if(opacity < 128)
    {
        int32_t width=zc_min(text_length(font, str->c_str()), 512);
        BITMAP *pbmp = create_sub_bitmap(prim_bmp, 0, 0, width, text_height(font));
        clear_bitmap(pbmp);
        textout_ex(pbmp, font, str->c_str(), 0, 0, color, bg_color);
        if(format_type == 2)   // right-sided text
            x-=width;
        else if(format_type == 1)   // centered text
            x-=width/2;
        draw_trans_sprite(refbmp, pbmp, x+xoffset, y+yoffset);
        destroy_bitmap(pbmp);
    }
    else // no opacity
    {
        if(format_type == 2)   // right-sided text
        {
            textout_right_ex(refbmp, font, str->c_str(), x+xoffset, y+yoffset, color, bg_color);
        }
        else if(format_type == 1)   // centered text
        {
            textout_centre_ex(refbmp, font, str->c_str(), x+xoffset, y+yoffset, color, bg_color);
        }
        else // standard left-sided text
        {
            textout_ex(refbmp, font, str->c_str(), x+xoffset, y+yoffset, color, bg_color);
        }
    }
}

void bmp_do_drawstringr2(BITMAP *bmp, int32_t i, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=font
    //sdci[5]=color
    //sdci[6]=bg color
    //sdci[7]=format_option
    //sdci[8]=string
    //sdci[9]=opacity
	//sdci[10]=shadowtype
	//sdci[11]=shadow_color
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
	Z_scripterrlog("bitmap->DrawString() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
	
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
    
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
    
    std::string* str = (std::string*)script_drawing_commands[i].GetPtr();
    
    if(!str)
    {
        al_trace("String pointer is null! Internal error. \n");
        return;
    }
    
    int32_t x=sdci[2]/10000;
    int32_t y=sdci[3]/10000;
    FONT* font=get_zc_font(sdci[4]/10000);
    int32_t color=sdci[5]/10000;
    int32_t bg_color=sdci[6]/10000; //-1 = transparent
    int32_t format_type=sdci[7]/10000;
    int32_t opacity=sdci[9]/10000;
	int32_t textstyle = sdci[10]/10000;
	int32_t shadow_color = sdci[11]/10000;
    //sdci[8] not needed :)
    
    //safe check
    if(bg_color < -1) bg_color = -1;
    
    if(opacity < 128)
    {
        int32_t width=zc_min(text_length(font, str->c_str()), 512);
        BITMAP *pbmp = create_sub_bitmap(prim_bmp, 0, 0, width, text_height(font));
        clear_bitmap(pbmp);
        textout_styled_aligned_ex(pbmp, font, str->c_str(), 0, 0, textstyle, sstaLEFT, color, shadow_color, bg_color);
        if(format_type == 2)   // right-sided text
            x-=width;
        else if(format_type == 1)   // centered text
            x-=width/2;
        draw_trans_sprite(refbmp, pbmp, x+xoffset, y+yoffset);
        destroy_bitmap(pbmp);
    }
    else // no opacity
    {
        textout_styled_aligned_ex(refbmp, font, str->c_str(), x+xoffset, y+yoffset, textstyle, format_type, color, shadow_color, bg_color);
    }
}

void bmp_do_clearr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
	Z_scripterrlog("bitmap->Clear() wanted to use to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
	int32_t bitid = sdci[DRAWCMD_BMP_TARGET]; 
	auto& usr_bitmap = scb.get(bitid);
	if (usr_bitmap.u_bmp)
		clear_bitmap(usr_bitmap.u_bmp);
}

void bmp_do_clearcolorr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=color
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	int32_t pal_color = sdci[2]/10000;
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
	Z_scripterrlog("bitmap->ClearToColor() wanted to use to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
	int32_t bitid = sdci[DRAWCMD_BMP_TARGET]; 
	auto& usr_bitmap = scb.get(bitid);
	if (usr_bitmap.u_bmp)
		clear_to_color(usr_bitmap.u_bmp, pal_color);
}


void bmp_do_regenr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
	int32_t h = sdci[3]/10000;
	int32_t w = sdci[2]/10000;
	if ( get_qr(qr_OLDCREATEBITMAP_ARGS) )
	{
		//flip height and width
		h = h ^ w;
		w = h ^ w; 
		h = h ^ w;
	}
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	//Z_scripterrlog("bitmap->Create() pointer is: %d\n", sdci[DRAWCMD_BMP_TARGET]);
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
	Z_scripterrlog("bitmap->Create() wanted to use to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
	int32_t bitid = sdci[DRAWCMD_BMP_TARGET];
	auto& usr_bmp = scb.get(bitid);
	if ( usr_bmp.u_bmp )
		destroy_bitmap(usr_bmp.u_bmp);
	usr_bmp.u_bmp = create_bitmap_ex(8,w,h);
	
	usr_bmp.width = w;
	usr_bmp.height = h;
}

void bmp_do_readr(BITMAP *bmp, int32_t i, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=filename
    //sdci[3]=y
    //sdci[4]=font
    //sdci[5]=color
    //sdci[6]=bg color
    //sdci[7]=format_option
    //sdci[8]=string
    //sdci[9]=opacity
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	//Z_scripterrlog("bitmap->Read() pointer is: %d\n", sdci[DRAWCMD_BMP_TARGET]);
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
		Z_scripterrlog("bitmap->Read() wanted to use to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
		return;
    }
	int32_t bitid = sdci[DRAWCMD_BMP_TARGET]; 
	auto& usr_bitmap = scb.get(bitid);
	usr_bitmap.destroy();
    
    std::string* str = (std::string*)script_drawing_commands[i].GetPtr();

    if(!str)
    {
        al_trace("String pointer is null! Internal error. \n");
        return;
    }
    
    PALETTE tempPal;
    get_palette(tempPal);
    if ( checkPath(str->c_str(), false) )
    {
	    usr_bitmap.u_bmp = load_bitmap(str->c_str(), tempPal);
	    usr_bitmap.width = usr_bitmap.u_bmp->w;
	    usr_bitmap.height = usr_bitmap.u_bmp->h;
	    if ( !usr_bitmap.u_bmp )
	    {
		Z_scripterrlog("Failed to load image file %s.\nMaking a blank bitmap on the pointer.\n", str->c_str());
	    }
	    else 
	    {
		    zprint("Read image file %s\n",str->c_str());
	    }
    }
    else
    {
	    Z_scripterrlog("Failed to load image file: %s. File not found. Creating a blank bitmap on the pointer.\n", str->c_str());
	    usr_bitmap.u_bmp = create_bitmap_ex(8,256,176);
	    clear_bitmap(usr_bitmap.u_bmp);
    }
}



void bmp_do_writer(BITMAP *bmp, int32_t i, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=filename
    //sdci[3]=y
    //sdci[4]=font
    //sdci[5]=color
    //sdci[6]=bg color
    //sdci[7]=format_option
    //sdci[8]=string
    //sdci[9]=opacity
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	//Z_scripterrlog("bitmap->Write() pointer is: %d\n", sdci[DRAWCMD_BMP_TARGET]);
	
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
	Z_scripterrlog("bitmap->Write() wanted to use to an invalid bitmap id: %d. Aborting.\n",  sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
	int32_t bitid = sdci[DRAWCMD_BMP_TARGET]; 
	auto& usr_bitmap = scb.get(bitid);
    if (!usr_bitmap.u_bmp) 
    {
	    Z_scripterrlog("Tried to write from an invalid bitmap pointer %d. Aborting. \n", sdci[DRAWCMD_BMP_TARGET]);
		return;
    }
    
    bool overwrite = (sdci[3] != 0);
    std::string* str = (std::string*)script_drawing_commands[i].GetPtr();
    
    if(!str)
    {
        al_trace("String pointer is null! Internal error. \n");
        return;
    }
    
    //char *cptr = new char[str->size()+1]; // +1 to account for \0 byte
	//std::strncpy(cptr, str->c_str(), str->size());
    //Z_scripterrlog("bitmap->Write extension matches ? : %s\n!", (FFCore.checkExtension(str->c_str(), ".png")) ? "true" : "false");
    //Z_scripterrlog("Trying to write filename %s\n", cptr);
	if 
	(
		( (FFCore.checkExtension(*str, "")) ) ||
		( !(FFCore.checkExtension(*str, ".png")) && !(FFCore.checkExtension(*str, ".gif")) && !(FFCore.checkExtension(*str, ".bmp"))
			&& !(FFCore.checkExtension(*str, ".pcx")) && !(FFCore.checkExtension(*str, ".tga")) )
	)
	{
		Z_scripterrlog("No extension, or invalid extension provided for writing bitmap file %s. Could not write the file.\nValid types are .png, .gif, .pcx, .tgx, and .bmp. Aborting.\n",str->c_str());
	}
	else if ( overwrite || (!checkPath(str->c_str(), false)) )
	{
		if(make_dirs_for_file(*str))
		{
			save_bitmap(str->c_str(), usr_bitmap.u_bmp, RAMpal);
			if(checkPath(str->c_str(), false))
			{
				zprint("Wrote image file %s\n",str->c_str());
			}
			else
			{
				Z_scripterrlog("Failed to create file '%s'\n",str->c_str());
			}
		}
		else
		{
			Z_scripterrlog("Cannot write file '%s' because the directory does not exist, and could not be created.\n", str->c_str());
		}
	}
	else Z_scripterrlog("Cannot write file '%s' because the file already exists in the specified path.\n", str->c_str());
}


void bmp_do_drawquadr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x1
    //sdci[3]=y1
    //sdci[4]=x2
    //sdci[5]=y2
    //sdci[6]=x3
    //sdci[7]=y3
    //sdci[8]=x4
    //sdci[9]=y4
    //sdci[10]=width
    //sdci[11]=height
    //sdci[12]=cset
    //sdci[13]=flip
    //sdci[14]=tile/combo
    //sdci[15]=polytype
	//sdci[16] = other bitmap as texture
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	Z_scripterrlog("bitmap quad pointer: %d\n", sdci[DRAWCMD_BMP_TARGET]);
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
		Z_scripterrlog("bitmap->Quad() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
		return;
    }
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
    
	if ( !refbmp ) return;
    
    int32_t x1 = sdci[2]/10000;
    int32_t y1 = sdci[3]/10000;
    int32_t x2 = sdci[4]/10000;
    int32_t y2 = sdci[5]/10000;
    int32_t x3 = sdci[6]/10000;
    int32_t y3 = sdci[7]/10000;
    int32_t x4 = sdci[8]/10000;
    int32_t y4 = sdci[9]/10000;
    int32_t w = sdci[10]/10000;
    int32_t h = sdci[11]/10000;
    int32_t color = sdci[12]/10000;
    int32_t flip=(sdci[13]/10000)&3;
    int32_t tile = sdci[14]/10000;
    int32_t polytype = sdci[15]/10000;
    int32_t quad_render_source = sdci[16];
    //Z_scripterrlog("bitmap->Quad() render source is: %d\n", quad_render_source);
    
    bool tex_is_bitmap = ( sdci[16] != 0 );
    
    BITMAP *bmptexture=NULL;
    BITMAP *tex=NULL;
    polytype = vbound(polytype, 0, 14);

    int32_t col[4];
        col[0]=col[1]=col[2]=col[3]=color;
    bool mustDestroyBmp = false;
    
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
    
    if ( tex_is_bitmap )
    {
	    bmptexture = FFCore.GetScriptBitmap(quad_render_source);
		if ( !bmptexture ) 
		{
			Z_scripterrlog("Bitmap pointer used as a texture in %s is uninitialised.\n Defaulting to using a tile as a texture.\n", "bitmap->Triangle3()");
			tex_is_bitmap = 0;
		}
    }
    
	if ( tex_is_bitmap ) 
	{
		
		if ( !isPowerOfTwo(bmptexture->h) ) Z_scripterrlog("HEIGHT of Bitmap ( pointer %d ) provided as a render source for bitmap->Quad is not a POWER OF TWO.\nTextels may render improperly!\n", quad_render_source);
		if ( !isPowerOfTwo(bmptexture->w) ) Z_scripterrlog("WIDTH of Bitmap ( pointer %d ) provided as a render source for bitmap->Quad is not a POWER OF TWO.\nTextels may render improperly!\n", quad_render_source);
		if ( !isPowerOfTwo(w) ) Z_scripterrlog("WIDTH ARG (%d) provided as a render source for bitmap->Quad is not a POWER OF TWO.\nTextels may render improperly!\n", w);
		if ( !isPowerOfTwo(h) ) Z_scripterrlog("HEIGHT ARG (%d) provided as a render source for bitmap->Quad is not a POWER OF TWO.\nTextels may render improperly!\n", h);
		
		V3D_f V1 = { static_cast<float>(x1+xoffset), static_cast<float>(y1+yoffset), 0, 0,                             0,                              col[0] };
		V3D_f V2 = { static_cast<float>(x2+xoffset), static_cast<float>(y2+yoffset), 0, 0,                             static_cast<float>(h), col[1] };
		V3D_f V3 = { static_cast<float>(x3+xoffset), static_cast<float>(y3+yoffset), 0, static_cast<float>(w), static_cast<float>(h), col[2] };
		V3D_f V4 = { static_cast<float>(x4+xoffset), static_cast<float>(y4+yoffset), 0, static_cast<float>(w), 0,                              col[3] };
	    
		quad3d_f(refbmp, polytype, bmptexture, &V1, &V2, &V3, &V4);
	}
	else
	{
		tex = script_drawing_commands.GetSmallTextureBitmap(w,h);
		if(!tex)
		{
			//Z_scripterrlog("Bitmap->Quad() found an invalid texture bitmap.\n");
			mustDestroyBmp = true;
			tex = create_bitmap_ex(8, w*16, h*16);
			clear_bitmap(tex);
		}
		
		if(tile > 0)   // TILE
		{
			TileHelper::OverTile(tex, tile, 0, 0, w, h, color, flip);
		}
	    
		if ( tile < 0 )        // COMBO
		{
			auto& c = GET_DRAWING_COMBO(vbound(abs(tile), 0, 0xffff));
			const int32_t tiletodraw = combo_tile(c, x1, y1);
			flip = flip ^ c.flip;
		
			TileHelper::OldPutTile(tex, tiletodraw, 0, 0, w, h, color, flip);
		}
		if(((w-1) & w) != 0 || ((h-1) & h) != 0)
		{
			Z_message("Quad() : Args h, w, must be in powers of two! Power of 2 error with %i, %i.", w, h);
			return; //non power of two error
		}
		//Z_scripterrlog("bitmap->Quad() is trying to blit from a bitmap texture.\n");
		V3D_f V1 = { static_cast<float>(x1+xoffset), static_cast<float>(y1+yoffset), 0, 0,                             0,                              col[0] };
		V3D_f V2 = { static_cast<float>(x2+xoffset), static_cast<float>(y2+yoffset), 0, 0,                             static_cast<float>(h), col[1] };
		V3D_f V3 = { static_cast<float>(x3+xoffset), static_cast<float>(y3+yoffset), 0, static_cast<float>(w), static_cast<float>(h), col[2] };
		V3D_f V4 = { static_cast<float>(x4+xoffset), static_cast<float>(y4+yoffset), 0, static_cast<float>(w), 0,                              col[3] };
	    
		quad3d_f(refbmp, polytype, tex, &V1, &V2, &V3, &V4);
		
	}
   
    
    
    
    //todo: finish palette shading
    /*
    POLYTYPE_FLAT
    POLYTYPE_GCOL
    POLYTYPE_GRGB
    POLYTYPE_ATEX
    POLYTYPE_PTEX
    POLYTYPE_ATEX_MASK
    POLYTYPE_PTEX_MASK
    POLYTYPE_ATEX_LIT
    POLYTYPE_PTEX_LIT
    POLYTYPE_ATEX_MASK_LIT
    POLYTYPE_PTEX_MASK_LIT
    POLYTYPE_ATEX_TRANS
    POLYTYPE_PTEX_TRANS
    POLYTYPE_ATEX_MASK_TRANS
    POLYTYPE_PTEX_MASK_TRANS
    */
    
    if(mustDestroyBmp)
        destroy_bitmap(tex);
        
}


void bmp_do_getpixelr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x1
    //sdci[3]=y1
    
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
	Z_scripterrlog("bitmap->GetPixel() wanted to read from an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
    
    
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
    
    int32_t x1 = sdci[2]/10000;
    int32_t y1 = (sdci[3]/10000)+yoffset;
    int32_t col = getpixel(scb.get(sdci[DRAWCMD_BMP_TARGET]).u_bmp, x1, y1);
    Z_scripterrlog("bitmap->GetPixel col is %d\n",col);
    Z_scripterrlog("bitmap->GetPixel bitmap ptr is is %d\n",(sdci[DRAWCMD_BMP_TARGET]));
    FFCore.set_sarg1(col);
}




void bmp_do_drawtriangler(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x1
    //sdci[3]=y1
    //sdci[4]=x2
    //sdci[5]=y2
    //sdci[6]=x3
    //sdci[7]=y3
    //sdci[8]=width
    //sdci[9]=height
    //sdci[10]=cset
    //sdci[11]=flip
    //sdci[12]=tile/combo
    //sdci[13]=polytype
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
    if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
    {
	Z_scripterrlog("bitmap->Triangle() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
    }
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
    
    
    int32_t render_source = sdci[14];
    //Z_scripterrlog("bitmap->Triangle() render source is: %d\n", render_source);
    
    bool tex_is_bitmap = ( sdci[14] != 0 );
    
    BITMAP *bmptexture=NULL;
	if ( tex_is_bitmap ) 
	{
		bmptexture = FFCore.GetScriptBitmap(render_source);
		if ( !bmptexture ) 
		{
			Z_scripterrlog("Bitmap pointer used as a texture in %s is uninitialised.\n Defaulting to using a tile as a texture.\n", "bitmap->Triangle3()");
			tex_is_bitmap = 0;
		}
	}
    
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
    
    int32_t x1 = sdci[2]/10000;
    int32_t y1 = sdci[3]/10000;
    int32_t x2 = sdci[4]/10000;
    int32_t y2 = sdci[5]/10000;
    int32_t x3 = sdci[6]/10000;
    int32_t y3 = sdci[7]/10000;
    int32_t w = sdci[8]/10000;
    int32_t h = sdci[9]/10000;
    int32_t color = sdci[10]/10000;
    int32_t flip=(sdci[11]/10000)&3;
    int32_t tile = sdci[12]/10000;
    int32_t polytype = sdci[13]/10000;
    
    polytype = vbound(polytype, 0, 14);
    int32_t utex_w = w;
    int32_t utex_h = h;
    
    
    int32_t tex_width = w*16;
    int32_t tex_height = h*16;
    
    bool mustDestroyBmp = false;
    BITMAP *tex = script_drawing_commands.GetSmallTextureBitmap(w,h);
    
    if(!tex)
    {
        mustDestroyBmp = true;
        tex = create_bitmap_ex(8, tex_width, tex_height);
        clear_bitmap(tex);
    }
    
    int32_t col[3];
    /*
    if( color < 0 )
    {
    col[0]=draw_container.color_buffer[0];
    col[1]=draw_container.color_buffer[1];
    col[2]=draw_container.color_buffer[2];
    }
    else */
    {
        col[0]=col[1]=col[2]=color;
    }
    
    if(tile > 0)   // TILE
    {
        TileHelper::OverTile(tex, tile, 0, 0, w, h, color, flip);
    }
    else        // COMBO
    {
        auto& c = GET_DRAWING_COMBO(vbound(abs(tile), 0, 0xffff));
        const int32_t tiletodraw = combo_tile(c, x1, y1);
        flip = flip ^ c.flip;
        
        TileHelper::OldPutTile(tex, tiletodraw, 0, 0, w, h, color, flip);
    }
    if ( !tex_is_bitmap )
    {
	if(((w-1) & w) != 0 || ((h-1) & h) != 0)
	{
		Z_message("bitmap->Triangle() : Args h, w, must be in powers of two! Power of 2 error with %i, %i.", w, h);
		return; //non power of two error
	}
	V3D_f V1 = { static_cast<float>(x1+xoffset), static_cast<float>(y1+yoffset), 0, 0,                             0,                              col[0] };
	V3D_f V2 = { static_cast<float>(x2+xoffset), static_cast<float>(y2+yoffset), 0, 0,                             static_cast<float>(tex_height), col[1] };
	V3D_f V3 = { static_cast<float>(x3+xoffset), static_cast<float>(y3+yoffset), 0, static_cast<float>(tex_width), static_cast<float>(tex_height), col[2] };
    
    
	triangle3d_f(refbmp, polytype, tex, &V1, &V2, &V3);
    
    }
    
    else
    {
	if ( !isPowerOfTwo(bmptexture->h) ) Z_scripterrlog("HEIGHT of Bitmap ( pointer %d ) provided as a render source for bitmap->Triangle is not a POWER OF TWO.\nTextels may render improperly!\n", render_source);
		if ( !isPowerOfTwo(bmptexture->w) ) Z_scripterrlog("WIDTH of Bitmap ( pointer %d ) provided as a render source for bitmap->Triangle is not a POWER OF TWO.\nTextels may render improperly!\n", render_source);
		if ( !isPowerOfTwo(utex_h) ) Z_scripterrlog("WIDTH ARG (%d) provided as a render source for bitmap->Triangle is not a POWER OF TWO.\nTextels may render improperly!\n", utex_w);
		if ( !isPowerOfTwo(utex_w) ) Z_scripterrlog("HEIGHT ARG (%d) provided as a render source for bitmap->Triangle is not a POWER OF TWO.\nTextels may render improperly!\n", utex_h);
		
	V3D_f V1 = { static_cast<float>(x1+xoffset), static_cast<float>(y1+yoffset), 0, 0,                             0,                              col[0] };
	V3D_f V2 = { static_cast<float>(x2+xoffset), static_cast<float>(y2+yoffset), 0, 0,                             static_cast<float>(utex_h), col[1] };
	V3D_f V3 = { static_cast<float>(x3+xoffset), static_cast<float>(y3+yoffset), 0, static_cast<float>(utex_w), static_cast<float>(utex_h), col[2] };
    
    
	triangle3d_f(refbmp, polytype, bmptexture, &V1, &V2, &V3);    
	    
    }
    
    if(mustDestroyBmp)
        destroy_bitmap(tex);
}


void bmp_do_mode7r(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	/*
	int32_t layer, int32_t rt, int32_t srcX, int32_t srcY, int32_t srcW, int32_t srcH, int32_t destW, int32_t destH, int32_t angle, int32_t cx, int32_t cy, int32_t space_z, int32_t horizon, 
	int32_t scale_x, int32_t scale_y){
	
	//sdci[1]=layer 
	//sdci[2]=bitmap target 
		//
		//	-2 is the current Render Target
		//	-1, this is the screen (framebuf). 
		//	0: Render target 0
		//	1: Render target 1
		//	2: Render target 2
		//	3: Render target 3
		//	4: Render target 4
		//	5: Render target 5
		//	6: Render target 6
		//	Otherwise: The pointer to a bitmap. 
		
	//sdci[3]=sourcex
	//sdci[4]=sourcey
	//sdci[5]=sourcew
	//sdci[6]=sourceh

	//sdci[7]=destw
	//sdci[8]=desth
	//sdci[9]=angle
	//scdi[10] = pivot cx
	//sdci[11] = pivot cy
	//sdci[12] = space Z
	//sdci[13] = horizon
	//scdi[14] = scale X
	//scdi[15] = scale Y
	//sdci[16] = masked?
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	
	
	
		// ZScript-side constant values:
		const int32_t BITDX_NORMAL = 0;
		const int32_t BITDX_TRANS = 1; //Translucent
		const int32_t BITDX_PIVOT = 2; //THe sprite will rotate at a specific point, instead of its center.
		const int32_t BITDX_HFLIP = 4; //Horizontal Flip
		const int32_t BITDX_VFLIP = 8; //Vertical Flip.
		//Note:	Some modes cannot be combined. if a combination is not supported, an error
		//	detailing this will be shown in allegro.log.
		
	//scdi[15] = litcolour
		//The allegro docs are wrong. The params are: rotate_sprite_lit(bmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour); 
		//not rotate_sprite_lit(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
	
	//sdci[16]=mask
	
	*/
	

	int32_t bitmapIndex = sdci[2];
	int32_t usr_bitmap_index = sdci[2];
	//Z_scripterrlog("bitmap index is: %d\n",bitmapIndex);
	//Z_scripterrlog("DrawPlane() bitmapIndex is: %d\n", bitmapIndex);
	
	if ( bitmapIndex >= 10000 )
	{
		bitmapIndex = bitmapIndex / 10000; //reduce if ZScript sent a raw value, such as bitmap = <int32_t> 8;
	}
	else if ( usr_bitmap_index > 0 ) 
	{
		bitmapIndex = usr_bitmap_index;
		// Z_scripterrlog("Mode7 is using a user bitmap target, pointer: %d\n", usr_bitmap_index);
		yoffset = 0;
	}
	
	//rendering mode 7 args
	double srcX = sdci[3]/10000.0;
	double srcY = sdci[4]/10000.0; 
	double destX = sdci[5]/10000.0;
	double destY = sdci[6]/10000.0;
	
	double destW = sdci[7]/10000.0;
	double destH = sdci[8]/10000.0;
	double space_z = sdci[9]/10000.0;
	double horizon = sdci[10]/10000.0;
	double scale_x = sdci[11]/10000.0;
	double scale_y = sdci[12]/10000.0;
	byte masked = ( sdci[13] ) ? 1 : 0;
	
	
	int32_t ref = 0;
	
	if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
	//Do we need to also check the render target and do the same thing if the 
		//dest == -2 and the render target is not RT_SCREEN?
		
	ref = sdci[DRAWCMD_BMP_TARGET];
		
	
	if ( ref <= 0 )
	{
		Z_scripterrlog("bitmap->DrawPlane() wanted to use to an invalid source bitmap id: %d. Aborting.\n", ref);
		return;
	}
	BITMAP *sourceBitmap = FFCore.GetScriptBitmap(ref); //This can be the screen, as -1. 
	
	if(!sourceBitmap)
	{
		Z_message("Warning: %d->DrawPlane() source bitmap contains invalid data or is not initialized.\n", ref);
		Z_message("[Note* Deferred drawing or layering order possibly not set right.]\n");
		return;
	}
	
	BITMAP *destBMP=NULL;
	switch(bitmapIndex)
	{
		case -2:
		{
			int32_t curr_rt = zscriptDrawingRenderTarget->GetCurrentRenderTarget();
			if ( curr_rt >= 0 && curr_rt < 7 ) 
				destBMP = zscriptDrawingRenderTarget->GetBitmapPtr(bitmapIndex); //Drawing to the current RenderTarget.
			else destBMP = bmp; //screen
			break;
		}
		case -1:
			destBMP = bmp; //this is framebuf, by default
			break;
		
		//1 through 6 are the old system bitmaps (Render Targets)
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6: 
		{
			//This gets a render target.
			destBMP = zscriptDrawingRenderTarget->GetTargetBitmap(bitmapIndex); 
			break;
		}
		//Otherwise, we are using a user-created bitmap, so, get that pointer insted.
		default: 
		{
			auto& usr_bitmap = scb.get(usr_bitmap_index);
			destBMP = usr_bitmap.u_bmp;
			if ( !usr_bitmap.u_bmp )
			{
				Z_scripterrlog("Target for bitmap->DrawPlane is uninitialised. Aborting.\n");
				break;
			}
		}
	}
	
	if (!destBMP)
	{
		Z_message("Warning: DrawPlane(%d) destination bitmap contains invalid data or is not initialized.\n", bitmapIndex);
		Z_message("[Note* Deferred drawing or layering order possibly not set right.]\n");
		return;
	}
	
	//dx = dx + xoffset; //don't do this here!
	//dy = dy + yoffset; //Nor this. It auto-offsets the bitmap by +56. Hmm. The fix that gleeok made isn't being applied to these functions. -Z ( 17th April, 2019 )
	//All of these are a factor of 10000 as fix. 
	int32_t screen_x = 0; int32_t screen_y = 0;
	
	double distance = 0; double horizontal_scale = 0;
	
	int32_t screen_y_horizon = 0;
	
	double line_dx = 0; double line_dy = 0;
	
	int32_t space_x = 0; int32_t space_y = 0;
	
	for(screen_y = 0; screen_y < destH; screen_y++) //fix, offset by .0000
	{
		//Calculate the distance of each line from the camera point
		screen_y_horizon = screen_y + horizon;
		
		distance = ((space_z * scale_y) / ((screen_y_horizon != 0) ? screen_y_horizon : 1));
			
		//Get the scale of each line based on the distance
		
		horizontal_scale = (distance / (( scale_x != 0 ) ? scale_x : 1));
		
		//There was some math here before I stripped out the rotation step
		line_dx = horizontal_scale;
		line_dy = 0;
		
		//space_x,space_y - where to grab each scanline from on the space bitmap
		space_x = srcX - destW/2.0 * line_dx;
		space_y = srcY - distance + destH/2.0 * line_dy;
		
		//Keep blits within the bounds of both bitmaps to avoid crashes
		int32_t y1 = srcY+space_y;
		int32_t y2 = destY+screen_y;
		if(y1 >=0 && y1 <= (sourceBitmap->h-1) && y2 >=0 && y2 <= (destBMP->h-1) )
		{
			if ( masked ) masked_stretch_blit(sourceBitmap, destBMP, (int32_t)(srcX+space_x), (int32_t)(srcY+space_y), 
				(int32_t)(line_dx*destW), 1, (int32_t)(screen_x), (int32_t)(screen_y)+yoffset, (int32_t)(destW), 1);
			else stretch_blit(sourceBitmap, destBMP, (int32_t)(srcX+space_x), (int32_t)(srcY+space_y), 
				(int32_t)(line_dx*destW), 1, (int32_t)(screen_x), (int32_t)(screen_y)+yoffset, (int32_t)(destW), 1);
		}
	}
}


//Draw]()
void bmp_do_drawbitmapexr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	/*
	//sdci[1]=layer 
	//sdci[2]=bitmap target 
		//
		//	-2 is the current Render Target
		//	-1, this is the screen (framebuf). 
		//	0: Render target 0
		//	1: Render target 1
		//	2: Render target 2
		//	3: Render target 3
		//	4: Render target 4
		//	5: Render target 5
		//	6: Render target 6
		//	Otherwise: The pointer to a bitmap. 
		
	//sdci[3]=sourcex
	//sdci[4]=sourcey
	//sdci[5]=sourcew
	//sdci[6]=sourceh
	//sdci[7]=destx
	//sdci[8]=desty
	//sdci[9]=destw
	//sdci[10]=desth
	//sdci[11]=rotation/angle
	//scdi[12] = pivot cx
	//sdci[13] = pivot cy
	//scdi[14] = effect flags
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	
		// ZScript-side constant values:
		const int32_t BITDX_NORMAL = 0;
		const int32_t BITDX_TRANS = 1; //Translucent
		const int32_t BITDX_PIVOT = 2; //THe sprite will rotate at a specific point, instead of its center.
		const int32_t BITDX_HFLIP = 4; //Horizontal Flip
		const int32_t BITDX_VFLIP = 8; //Vertical Flip.
		//Note:	Some modes cannot be combined. if a combination is not supported, an error
		//	detailing this will be shown in allegro.log.
		
	//scdi[15] = litcolour
		//The allegro docs are wrong. The params are: rotate_sprite_lit(bmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour); 
		//not rotate_sprite_lit(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
	
	//sdci[16]=mask
	
	*/
	
	int32_t usr_bitmap_index = sdci[2];
	auto [bitmapIndex, is_user_bitmap] = resolveScriptingBitmapId(usr_bitmap_index);


	int32_t sx = sdci[3]/10000;
	int32_t sy = sdci[4]/10000;
	int32_t sw = sdci[5]/10000;
	int32_t sh = sdci[6]/10000;
	int32_t dx = sdci[7]/10000;
	int32_t dy = sdci[8]/10000;
	int32_t dw = sdci[9]/10000;
	int32_t dh = sdci[10]/10000;
	float rot = sdci[11]/10000;
	int32_t cx = sdci[12]/10000;
	int32_t cy = sdci[13]/10000;
	int32_t mode = sdci[14]/10000;
	int32_t litcolour = sdci[15]/10000;
	bool masked = (sdci[16] != 0);
	
	int32_t ref = 0;

	if (get_qr(qr_BROKEN_SCRIPTS_BITMAP_DRAW_ORIGIN))
	{
		if (is_user_bitmap)
			yoffset = 0;

		dx += xoffset;
		dy += yoffset;
	}
	else
	{
		dx += secondary_draw_origin_xoff;
		dy += secondary_draw_origin_yoff;

		sx += xoffset;
		sy += yoffset;
	}

	ref = sdci[DRAWCMD_BMP_TARGET];
	
	if ( ref <= 0 )
	{
		Z_scripterrlog("bitmap->blit() wanted to use to an invalid source bitmap id: %d. Aborting.\n", ref);
		return;
	}
	BITMAP *sourceBitmap = FFCore.GetScriptBitmap(ref); //This can be the screen, as -1. 

	if(!sourceBitmap)
	{
		
		Z_message("Warning: blit(%d) source bitmap contains invalid data or is not initialized.\n", ref);
		Z_message("[Note* Deferred drawing or layering order possibly not set right.]\n");
		return;
	}

	BITMAP *destBMP=NULL;
	switch(bitmapIndex)
	{
		case -2:
		{
			int32_t curr_rt = zscriptDrawingRenderTarget->GetCurrentRenderTarget();
			if ( curr_rt >= 0 && curr_rt < 7 ) 
				destBMP = zscriptDrawingRenderTarget->GetBitmapPtr(bitmapIndex); //Drawing to the current RenderTarget.
			else destBMP = bmp; //screen
			break;
		}
		case -1:
			destBMP = bmp; //this is framebuf, by default
			break;
			//zscriptDrawingRenderTarget->SetCurrentRenderTarget(bitmapIndex);
			//destBMP = zscriptDrawingRenderTarget->GetBitmapPtr(bitmapIndex);
			//destBMP = framebuf; //Drawing to the screen.
			//break;
		
		//1 through 6 are the old system bitmaps (Render Targets)
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6: 
		{
			//This gets a render target.
			destBMP = zscriptDrawingRenderTarget->GetBitmapPtr(bitmapIndex); break;
			
			//destBMP = zscriptDrawingRenderTarget->GetTargetBitmap(bitmapIndex); 
			//sdci[18] = bitmapIndex;
			break;
		}
		//Otherwise, we are using a user-created bitmap, so, get that pointer insted.
		default: 
		{
			auto& usr_bitmap = scb.get(usr_bitmap_index);
			destBMP = usr_bitmap.u_bmp;
			//sdci[18] = usr_bitmap_index;
			if ( !usr_bitmap.u_bmp )
			{
				Z_scripterrlog("Target for bitmap->Blit is uninitialised. Aborting.\n");
				break;
			}
		}
	}
	
	if (!destBMP)
	{
		Z_message("Warning: blit(%d) destination bitmap contains invalid data or is not initialized.\n", bitmapIndex);
		Z_message("[Note* Deferred drawing or layering order possibly not set right.]\n");
		return;
	}

	bool stretched = (sw != dw || sh != dh);	
	BITMAP* subBmp = 0;
    
	if(rot != 0 || mode != 0)    
	{
		subBmp = create_bitmap_ex(8,sourceBitmap->w, sourceBitmap->h);//script_drawing_commands.AquireSubBitmap(dw, dh);
		clear_bitmap(subBmp);
        
		if(!subBmp)
		{
			
			Z_scripterrlog("bitmap->Blit failed to create a sub-bitmap to use for %s. Aborting.\n", "rotation");
			return;
		}
	}
	BITMAP* sbmp = sourceBitmap;
	if (sx + sw > sbmp->w || sy + sh > sbmp->h)
	{
		sbmp = create_bitmap_ex(8, sw, sh);
		clear_bitmap(sbmp);
		blit(sourceBitmap, sbmp, sx, sy, 0, 0, std::min(sourceBitmap->w-sx, sw), std::min(sourceBitmap->h-sy, sh));
		sx = 0;
		sy = 0;
	}
	//dx = dx + xoffset; //don't do this here!
	//dy = dy + yoffset; //Nor this. It auto-offsets the bitmap by +56. Hmm. The fix that gleeok made isn't being applied to these functions. -Z ( 17th April, 2019 )
    
	if(stretched) 
	{
		if(masked) 
		{	//stretched and masked
			if ( rot == 0 ) 
			{ //if not rotated
				switch(mode) 
				{
					case 1:
					//transparent
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_trans_sprite(destBMP, subBmp, dx, dy);
					break;
					
					
					case 2: 
						//pivot?
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite(destBMP, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_trans(destBMP, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					break;
					
					case 4: 
						//flip v
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_v_flip(destBMP, subBmp, dx, dy);
					break;
					
					case 5: 
						//trans + v flip
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_V_FLIP);
					break;
					
					case 6: 
						//pivot + v flip
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_v_flip(destBMP, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					break;
					
					case 8: 
						//vlip h
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_h_flip(destBMP, subBmp, dx, dy);
					break;
					
					case 9: 
						//trans + h flip
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_H_FLIP);
					break;
					
					case 10: 
						//flip H and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					//return error cannot pivot and h flip
					break;
					
					case 12:
						//vh flip
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_vh_flip(destBMP, subBmp, dx, dy);
					break;
					
					case 13: 
						//trans + vh flip
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_lit_sprite(destBMP, subBmp, dx, dy, litcolour);
					break;
					
					case 18: 
						//pivot, lit
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_lit(destBMP, subBmp, dx, dy, cx, cy,  degrees_to_fixed(rot),litcolour);
					break;
					
					case 20: 
						//lit + v flip
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_V_FLIP);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_v_flip_lit(destBMP, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 24: 
						//lit + h flip
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_H_FLIP);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot, Flip, and Lit.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect
					masked_stretch_blit(sbmp, destBMP, sx, sy, sw, sh, dx, dy, dw, dh);
					break;
					
					
					default:
						
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
					
					
				}
			} //end if not rotated
			
			if ( rot != 0 ) //if rotated
			{ 
				switch(mode)
				{
					case 1: 
						//transparent
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_trans(destBMP, subBmp, dx, dy, degrees_to_fixed(rot));
					
					break;
					
					case 2: 
						//pivot?
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite(destBMP, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 4: 
						//flip v
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip(destBMP, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 5: 
						//trans + v flip
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip_trans(destBMP, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 6: 
						//pivot + v flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					//return an error, cannot both rotate and pivot
					break;
					
					case 8: 
						//flip h
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rorate and H-Flip.\n", bitmapIndex);
					//return an error, cannot both rotate and flip H
					break;
					
					case 9: 
						//trans + h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot Rotate and Flip a Trans Sprite.\n", bitmapIndex);
					//return an error, cannot rotate and flip a trans sprite
					break;
					
					case 10: 
						//flip H and pivot
					//return error cannot pivot and h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					break;
					
					case 12: 
						//vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 13: 
						//trans + vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_lit(destBMP, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 18: 
						//pivot, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 20: 
						//lit + vflip
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip_lit(destBMP, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					break;
					
					case 24: 
						//lit + h flip
					//return an error, cannot both rotate and H flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and H-Flip.\n", bitmapIndex);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Flip a Lit Sprite.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					//return an error, cannot both rotate and VH flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect.
					masked_stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite(destBMP, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					default:
						
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
				
				}
			}
		} //end if stretched and masked 
		
		else  //stretched, not masked
		{
			
		
			if ( rot == 0 ) //if not rotated
			{
				switch(mode) 
				{
					case 1:
					//transparent
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_trans_sprite(destBMP, subBmp, dx, dy);
					break;
					
					
					case 2: 
						//pivot?
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite(destBMP, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_trans(destBMP, subBmp, dx,dy,  cx,  cy, degrees_to_fixed(rot));
					break;
					
					case 4: 
						//flip v
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_v_flip(destBMP, subBmp, dx, dy);
					break;
					
					case 5: 
						//trans + v flip
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_V_FLIP);
					break;
					
					case 6: 
						//pivot + v flip
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_v_flip(destBMP, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					break;
					
					case 8: 
						//vlip h
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_h_flip(destBMP, subBmp, dx, dy);
					break;
					
					case 9: 
						//trans + h flip
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_H_FLIP);
					break;
					
					case 10: 
						//flip H and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					//return error cannot pivot and h flip
					break;
					
					case 12:
						//vh flip
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_vh_flip(destBMP, subBmp, dx, dy);
					break;
					
					case 13: 
						//trans + vh flip
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_lit_sprite(destBMP, subBmp, dx, dy, litcolour);
					break;
					
					case 18: 
						//pivot, lit
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_lit(destBMP, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 20: 
						//lit + v flip
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_V_FLIP);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_v_flip_lit(destBMP, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 24: 
						//lit + h flip
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_H_FLIP);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot, Flip, and Lit.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect
					stretch_blit(sbmp, destBMP, sx, sy, sw, sh, dx, dy, dw, dh);
					break;
					
					
					default:
						
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
					
					
				}
			} //end if not rotated
			
			if ( rot != 0 )  //if rotated
			{
				switch(mode)
				{
					case 1: 
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);//transparent
					rotate_sprite_trans(destBMP, subBmp, dx, dy, degrees_to_fixed(rot));
					
					break;
					
					case 2: 
						//pivot?
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite(destBMP, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 4: 
						//flip v
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip(destBMP, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 5: 
						//trans + v flip
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip_trans(destBMP, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 6: 
						//pivot + v flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					//return an error, cannot both rotate and pivot
					break;
					
					case 8: 
						//flip h
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rorate and H-Flip.\n", bitmapIndex);
					//return an error, cannot both rotate and flip H
					break;
					
					case 9: 
						//trans + h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot Rotate and Flip a Trans Sprite.\n", bitmapIndex);
					//return an error, cannot rotate and flip a trans sprite
					break;
					
					case 10: 
						//flip H and pivot
					//return error cannot pivot and h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					break;
					
					case 12: 
						//vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 13: 
						//trans + vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);//transparent
					rotate_sprite_lit(destBMP, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 18: 
						//pivot, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 20: 
						//lit + vflip
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);//transparent
					rotate_sprite_v_flip_lit(destBMP, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					break;
					
					case 24: 
						//lit + h flip
					//return an error, cannot both rotate and H flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and H-Flip.\n", bitmapIndex);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Flip a Lit Sprite.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					//return an error, cannot both rotate and VH flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect.
					stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite(destBMP, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					default:
						
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
				
				}
			}
			
		} //end if stretched, but not masked
	}
	else //not stretched
	{ 
		
		if(masked) //if masked, but not stretched
		{ 
			
			if ( rot == 0 ) //if not rotated
			{ 
				switch(mode) 
				{
					case 1:
					//transparent
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_trans_sprite(destBMP, subBmp, dx, dy);
					break;
					
					
					case 2: 
						//pivot?
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite(destBMP, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite_trans(destBMP, subBmp, dx, dy,  cx, cy, degrees_to_fixed(rot));
					break;
					
					case 4: 
						//flip v
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_v_flip(destBMP, subBmp, dx, dy);
					break;
					
					case 5: 
						//trans + v flip
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_V_FLIP);
					break;
					
					case 6: 
						//pivot + v flip
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite_v_flip(destBMP, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot));
					break;
					
					case 8: 
						//vlip h
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_h_flip(destBMP, subBmp, dx, dy);
					break;
					
					case 9: 
						//trans + h flip
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_H_FLIP);
					break;
					
					case 10: 
						//flip H and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					//return error cannot pivot and h flip
					break;
					
					case 12:
						//vh flip
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_vh_flip(destBMP, subBmp, dx, dy);
					break;
					
					case 13: 
						//trans + vh flip
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_lit_sprite(destBMP, subBmp, dx, dy, litcolour);
					break;
					
					case 18: 
						//pivot, lit
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite_lit(destBMP, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 20: 
						//lit + v flip
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_V_FLIP);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite_v_flip_lit(destBMP, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 24: 
						//lit + h flip
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_H_FLIP);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot, Flip, and Lit.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect
					masked_blit(sbmp, destBMP, sx, sy, dx, dy, dw, dh);
					break;
					
					
					default:
						
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
					
					
				}
			} //end if not rotated
			
			if ( rot != 0 )  //if rotated
			{
				switch(mode)
				{
					case 1: 
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);	//transparent
					rotate_sprite_trans(destBMP, subBmp, dx, dy, degrees_to_fixed(rot));
					
					break;
					
					case 2: 
						//pivot?
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite(destBMP, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 4: 
						//flip v
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_v_flip(destBMP, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 5: 
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);	//trans + v flip
					rotate_sprite_v_flip_trans(destBMP, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 6: 
						//pivot + v flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					//return an error, cannot both rotate and pivot
					break;
					
					case 8: 
						//flip h
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rorate and H-Flip.\n", bitmapIndex);
					//return an error, cannot both rotate and flip H
					break;
					
					case 9: 
						//trans + h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot Rotate and Flip a Trans Sprite.\n", bitmapIndex);
					//return an error, cannot rotate and flip a trans sprite
					break;
					
					case 10: 
						//flip H and pivot
					//return error cannot pivot and h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					break;
					
					case 12: 
						//vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 13: 
						//trans + vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_lit(destBMP, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 18: 
						//pivot, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 20: 
						//lit + vflip
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_v_flip_lit(destBMP, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					break;
					
					case 24: 
						//lit + h flip
					//return an error, cannot both rotate and H flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and H-Flip.\n", bitmapIndex);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Flip a Lit Sprite.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					//return an error, cannot both rotate and VH flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//stretch_blit(sbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect.
					masked_blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite(destBMP, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					default:
						
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
				
				}
			} //end rtated, masked
		} //end if masked

		else  //not masked, and not stretched; just blit
		{
			
			if ( rot == 0 ) //if not rotated
			{ 
				switch(mode) 
				{
					case 1:
					//transparent
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_trans_sprite(destBMP, subBmp, dx, dy);
					break;
					
					
					case 2: 
						//pivot?
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite(destBMP, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite_trans(destBMP, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot));
					break;
					
					case 4: 
						//flip v
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_v_flip(destBMP, subBmp, dx, dy);
					break;
					
					case 5: 
						//trans + v flip
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_V_FLIP);
					break;
					
					case 6: 
						//pivot + v flip
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite_v_flip(destBMP, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					break;
					
					case 8: 
						//vlip h
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_h_flip(destBMP, subBmp, dx, dy);
					break;
					
					case 9: 
						//trans + h flip
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_H_FLIP);
					break;
					
					case 10: 
						//flip H and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					//return error cannot pivot and h flip
					break;
					
					case 12:
						//vh flip
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_vh_flip(destBMP, subBmp, dx, dy);
					break;
					
					case 13: 
						//trans + vh flip
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_lit_sprite(destBMP, subBmp, dx, dy, litcolour);
					break;
					
					case 18: 
						//pivot, lit
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite_lit(destBMP, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 20: 
						//lit + v flip
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_V_FLIP);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite_v_flip_lit(destBMP, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 24: 
						//lit + h flip
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_H_FLIP);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot, Flip, and Lit.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(destBMP, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect
					blit(sbmp, destBMP, sx, sy, dx, dy, dw, dh);
					break;
					
					
					default:
						
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
					
					
				}
			} //end if not rotated
			
			if ( rot != 0 )  //if rotated
			{
				switch(mode)
				{
					case 1: 
						blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);//transparent
					rotate_sprite_trans(destBMP, subBmp, dx, dy, degrees_to_fixed(rot));
					 
					break;
					
					case 2: 
						//pivot?
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite(destBMP, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 4: 
						//flip v
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					rotate_sprite_v_flip(destBMP, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 5: 
						//trans + v flip
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_v_flip_trans(destBMP, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 6: 
						//pivot + v flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					//return an error, cannot both rotate and pivot
					break;
					
					case 8: 
						//flip h
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rorate and H-Flip.\n", bitmapIndex);
					//return an error, cannot both rotate and flip H
					break;
					
					case 9: 
						//trans + h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot Rotate and Flip a Trans Sprite.\n", bitmapIndex);
					//return an error, cannot rotate and flip a trans sprite
					break;
					
					case 10: 
						//flip H and pivot
					//return error cannot pivot and h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					break;
					
					case 12: 
						//vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 13: 
						//trans + vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_lit(destBMP, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 18: 
						//pivot, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 20: 
						//lit + vflip
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_v_flip_lit(destBMP, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					break;
					
					case 24: 
						//lit + h flip
					//return an error, cannot both rotate and H flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and H-Flip.\n", bitmapIndex);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Flip a Lit Sprite.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					//return an error, cannot both rotate and VH flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect.
					blit(sbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					rotate_sprite(destBMP, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					default:
						
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
				
				}
			} //end if rotated
		} //end if not masked
	} //end if not stretched
    
	//cleanup
	if(subBmp) 
	{
		//script_drawing_commands.ReleaseSubBitmap(subBmp); //purge the temporary bitmap.
		destroy_bitmap(subBmp);
	}
	if (sbmp != sourceBitmap)
	{
		destroy_bitmap(sbmp);
	}
}



void bmp_do_blittor(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	/*
	//sdci[1]=layer 
	//sdci[2]=bitmap target 
		//
		//	-2 is the current Render Target
		//	-1, this is the screen (framebuf). 
		//	0: Render target 0
		//	1: Render target 1
		//	2: Render target 2
		//	3: Render target 3
		//	4: Render target 4
		//	5: Render target 5
		//	6: Render target 6
		//	Otherwise: The pointer to a bitmap. 
		
	//sdci[3]=sourcex
	//sdci[4]=sourcey
	//sdci[5]=sourcew
	//sdci[6]=sourceh
	//sdci[7]=destx
	//sdci[8]=desty
	//sdci[9]=destw
	//sdci[10]=desth
	//sdci[11]=rotation/angle
	//scdi[12] = pivot cx
	//sdci[13] = pivot cy
	//scdi[14] = effect flags
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	
		// ZScript-side constant values:
		const int32_t BITDX_NORMAL = 0;
		const int32_t BITDX_TRANS = 1; //Translucent
		const int32_t BITDX_PIVOT = 2; //THe sprite will rotate at a specific point, instead of its center.
		const int32_t BITDX_HFLIP = 4; //Horizontal Flip
		const int32_t BITDX_VFLIP = 8; //Vertical Flip.
		//Note:	Some modes cannot be combined. if a combination is not supported, an error
		//	detailing this will be shown in allegro.log.
		
	//scdi[15] = litcolour
		//The allegro docs are wrong. The params are: rotate_sprite_lit(bmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour); 
		//not rotate_sprite_lit(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
	
	//sdci[16]=mask
	
	*/

	int32_t srcyoffset = yoffset, srcxoffset = xoffset;

	int32_t usr_bitmap_index = sdci[2];
	auto [bitmapIndex, is_user_bitmap] = resolveScriptingBitmapId(usr_bitmap_index);

	int32_t sx = sdci[3]/10000;
	int32_t sy = sdci[4]/10000;
	int32_t sw = sdci[5]/10000;
	int32_t sh = sdci[6]/10000;
	int32_t dx = sdci[7]/10000;
	int32_t dy = sdci[8]/10000;
	int32_t dw = sdci[9]/10000;
	int32_t dh = sdci[10]/10000;
	float rot = sdci[11]/10000;
	int32_t cx = sdci[12]/10000;
	int32_t cy = sdci[13]/10000;
	int32_t mode = sdci[14]/10000;
	int32_t litcolour = sdci[15]/10000;
	bool masked = (sdci[16] != 0);
	
	int32_t ref = 0;

	if (get_qr(qr_BROKEN_SCRIPTS_BITMAP_DRAW_ORIGIN))
	{
		if (is_user_bitmap)
			srcyoffset = 0;
		if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
		if ( (bitmapIndex) != -2 && (bitmapIndex) != -1 ) srcyoffset = 0; //Don't crop. 

		dx += xoffset;
		dy += yoffset;

		sx += srcxoffset;
		sy += srcyoffset;
	}
	else
	{
		dx += xoffset;
		dy += yoffset;

		sx += secondary_draw_origin_xoff;
		sy += secondary_draw_origin_yoff;
	}

	ref = sdci[DRAWCMD_BMP_TARGET];
	
	if ( ref <= 0 )
	{
		Z_scripterrlog("bitmap->blit() wanted to use to an invalid source bitmap id: %d. Aborting.\n", ref);
		return;
	}
	BITMAP *sourceBitmap = FFCore.GetScriptBitmap(ref); //This can be the screen, as -1. 
	if(!sourceBitmap)
	{
		Z_message("Warning: blit(%d) source bitmap contains invalid data or is not initialized.\n", ref);
		Z_message("[Note* Deferred drawing or layering order possibly not set right.]\n");
		return;
	}
	
	BITMAP *destBMP=NULL;
	
	switch(bitmapIndex)
	{
		case -2:
		{
			int32_t curr_rt = zscriptDrawingRenderTarget->GetCurrentRenderTarget();
			if ( curr_rt >= 0 && curr_rt < 7 ) 
				destBMP = zscriptDrawingRenderTarget->GetBitmapPtr(bitmapIndex); //Drawing to the current RenderTarget.
			else destBMP = bmp; //screen
			break;
		}
		case -1:
			destBMP = bmp; //this is framebuf, by default
			break;
			//zscriptDrawingRenderTarget->SetCurrentRenderTarget(bitmapIndex);
			//destBMP = zscriptDrawingRenderTarget->GetBitmapPtr(bitmapIndex);
			//destBMP = framebuf; //Drawing to the screen.
			//break;
		
		//1 through 6 are the old system bitmaps (Render Targets)
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6: 
		{
			//This gets a render target.
			//destBMP = zscriptDrawingRenderTarget->GetBitmapPtr(bitmapIndex); break;
			
			destBMP = zscriptDrawingRenderTarget->GetTargetBitmap(bitmapIndex); 
			//sdci[18] = bitmapIndex;
			break;
		}
		//Otherwise, we are using a user-created bitmap, so, get that pointer insted.
		default: 
		{
			auto& usr_bitmap = scb.get(usr_bitmap_index);
			destBMP = usr_bitmap.u_bmp;
			//sdci[18] = usr_bitmap_index;
			if ( !usr_bitmap.u_bmp )
			{
				Z_scripterrlog("Target for bitmap->Blit is uninitialised. Aborting.\n");
				break;
			}
		}
	}
	
	if (!destBMP)
	{
		Z_message("Warning: blit(%d) destination bitmap contains invalid data or is not initialized.\n", bitmapIndex);
		Z_message("[Note* Deferred drawing or layering order possibly not set right.]\n");
		return;
	}
	
	bool stretched = (sw != dw || sh != dh);
	
	BITMAP* newDest = sourceBitmap;
	BITMAP* newSource = destBMP; //Flip them. 
    
	BITMAP* subBmp = 0;
    
	if(rot != 0 || mode != 0)    
	{
		subBmp = create_bitmap_ex(8,sourceBitmap->w, sourceBitmap->h);//script_drawing_commands.AquireSubBitmap(dw, dh);
		clear_bitmap(subBmp);
        
		if(!subBmp)
		{
			Z_scripterrlog("bitmap->Blit failed to create a sub-bitmap to use for %s. Aborting.\n", "rotation");
			return;
		}
	}
    
	if (sx + sw > destBMP->w || sy + sh > destBMP->h)
	{
		newSource = create_bitmap_ex(8, sw, sh);
		clear_bitmap(newSource);
		blit(destBMP, newSource, sx, sy, 0, 0, std::min(destBMP->w-sx, sw), std::min(destBMP->h-sy, sh));
		sx = 0;
		sy = 0;
	}
	//dx = dx + xoffset; //don't do this here!
	//dy = dy + yoffset; //Nor this. It auto-offsets the bitmap by +56. Hmm. The fix that gleeok made isn't being applied to these functions. -Z ( 17th April, 2019 )
    
	if(stretched) 
	{
		if(masked) 
		{	//stretched and masked
			if ( rot == 0 ) 
			{ //if not rotated
				switch(mode) 
				{
					case 1:
					//transparent
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_trans_sprite(newDest, subBmp, dx, dy);
					break;
					
					
					case 2: 
						//pivot?
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite(newDest, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_trans(newDest, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					break;
					
					case 4: 
						//flip v
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_v_flip(newDest, subBmp, dx, dy);
					break;
					
					case 5: 
						//trans + v flip
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_V_FLIP);
					break;
					
					case 6: 
						//pivot + v flip
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_v_flip(newDest, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					break;
					
					case 8: 
						//vlip h
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_h_flip(newDest, subBmp, dx, dy);
					break;
					
					case 9: 
						//trans + h flip
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_H_FLIP);
					break;
					
					case 10: 
						//flip H and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					//return error cannot pivot and h flip
					break;
					
					case 12:
						//vh flip
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_vh_flip(newDest, subBmp, dx, dy);
					break;
					
					case 13: 
						//trans + vh flip
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_lit_sprite(newDest, subBmp, dx, dy, litcolour);
					break;
					
					case 18: 
						//pivot, lit
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_lit(newDest, subBmp, dx, dy, cx, cy,  degrees_to_fixed(rot),litcolour);
					break;
					
					case 20: 
						//lit + v flip
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_V_FLIP);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_v_flip_lit(newDest, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 24: 
						//lit + h flip
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_H_FLIP);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot, Flip, and Lit.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect
					masked_stretch_blit(newSource, newDest, sx, sy, sw, sh, dx, dy, dw, dh);
					break;
					
					
					default:
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
					
					
				}
			} //end if not rotated
			
			if ( rot != 0 ) //if rotated
			{ 
				switch(mode)
				{
					case 1: 
						//transparent
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_trans(newDest, subBmp, dx, dy, degrees_to_fixed(rot));
					
					break;
					
					case 2: 
						//pivot?
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite(newDest, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 4: 
						//flip v
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip(newDest, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 5: 
						//trans + v flip
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip_trans(newDest, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 6: 
						//pivot + v flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					//return an error, cannot both rotate and pivot
					break;
					
					case 8: 
						//flip h
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rorate and H-Flip.\n", bitmapIndex);
					//return an error, cannot both rotate and flip H
					break;
					
					case 9: 
						//trans + h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot Rotate and Flip a Trans Sprite.\n", bitmapIndex);
					//return an error, cannot rotate and flip a trans sprite
					break;
					
					case 10: 
						//flip H and pivot
					//return error cannot pivot and h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					break;
					
					case 12: 
						//vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 13: 
						//trans + vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_lit(newDest, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 18: 
						//pivot, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 20: 
						//lit + vflip
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip_lit(newDest, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					break;
					
					case 24: 
						//lit + h flip
					//return an error, cannot both rotate and H flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and H-Flip.\n", bitmapIndex);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Flip a Lit Sprite.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					//return an error, cannot both rotate and VH flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect.
					masked_stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite(newDest, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					default:
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
				
				}
			}
		} //end if stretched and masked 
		
		else  //stretched, not masked
		{
			
		
			if ( rot == 0 ) //if not rotated
			{
				switch(mode) 
				{
					case 1:
					//transparent
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_trans_sprite(newDest, subBmp, dx, dy);
					break;
					
					
					case 2: 
						//pivot?
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite(newDest, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_trans(newDest, subBmp, dx,dy,  cx,  cy, degrees_to_fixed(rot));
					break;
					
					case 4: 
						//flip v
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_v_flip(newDest, subBmp, dx, dy);
					break;
					
					case 5: 
						//trans + v flip
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_V_FLIP);
					break;
					
					case 6: 
						//pivot + v flip
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_v_flip(newDest, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					break;
					
					case 8: 
						//vlip h
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_h_flip(newDest, subBmp, dx, dy);
					break;
					
					case 9: 
						//trans + h flip
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_H_FLIP);
					break;
					
					case 10: 
						//flip H and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					//return error cannot pivot and h flip
					break;
					
					case 12:
						//vh flip
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_vh_flip(newDest, subBmp, dx, dy);
					break;
					
					case 13: 
						//trans + vh flip
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_lit_sprite(newDest, subBmp, dx, dy, litcolour);
					break;
					
					case 18: 
						//pivot, lit
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_lit(newDest, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 20: 
						//lit + v flip
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_V_FLIP);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_v_flip_lit(newDest, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 24: 
						//lit + h flip
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_H_FLIP);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot, Flip, and Lit.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect
					stretch_blit(newSource, newDest, sx, sy, sw, sh, dx, dy, dw, dh);
					break;
					
					
					default:
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
					
					
				}
			} //end if not rotated
			
			if ( rot != 0 )  //if rotated
			{
				switch(mode)
				{
					case 1: 
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);//transparent
					rotate_sprite_trans(newDest, subBmp, dx, dy, degrees_to_fixed(rot));
					
					break;
					
					case 2: 
						//pivot?
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite(newDest, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 4: 
						//flip v
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip(newDest, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 5: 
						//trans + v flip
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip_trans(newDest, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 6: 
						//pivot + v flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					//return an error, cannot both rotate and pivot
					break;
					
					case 8: 
						//flip h
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rorate and H-Flip.\n", bitmapIndex);
					//return an error, cannot both rotate and flip H
					break;
					
					case 9: 
						//trans + h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot Rotate and Flip a Trans Sprite.\n", bitmapIndex);
					//return an error, cannot rotate and flip a trans sprite
					break;
					
					case 10: 
						//flip H and pivot
					//return error cannot pivot and h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					break;
					
					case 12: 
						//vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 13: 
						//trans + vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);//transparent
					rotate_sprite_lit(newDest, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 18: 
						//pivot, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 20: 
						//lit + vflip
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);//transparent
					rotate_sprite_v_flip_lit(newDest, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					break;
					
					case 24: 
						//lit + h flip
					//return an error, cannot both rotate and H flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and H-Flip.\n", bitmapIndex);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Flip a Lit Sprite.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					//return an error, cannot both rotate and VH flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect.
					stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite(newDest, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					default:
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
				
				}
			}
			
		} //end if stretched, but not masked
	}
	else //not stretched
	{ 
		
		if(masked) //if masked, but not stretched
		{ 
			
			if ( rot == 0 ) //if not rotated
			{ 
				switch(mode) 
				{
					case 1:
					//transparent
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					draw_trans_sprite(newDest, subBmp, dx, dy);
					break;
					
					
					case 2: 
						//pivot?
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite(newDest, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite_trans(newDest, subBmp, dx, dy,  cx, cy, degrees_to_fixed(rot));
					break;
					
					case 4: 
						//flip v
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_v_flip(newDest, subBmp, dx, dy);
					break;
					
					case 5: 
						//trans + v flip
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_V_FLIP);
					break;
					
					case 6: 
						//pivot + v flip
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite_v_flip(newDest, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot));
					break;
					
					case 8: 
						//vlip h
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_h_flip(newDest, subBmp, dx, dy);
					break;
					
					case 9: 
						//trans + h flip
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_H_FLIP);
					break;
					
					case 10: 
						//flip H and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					//return error cannot pivot and h flip
					break;
					
					case 12:
						//vh flip
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_vh_flip(newDest, subBmp, dx, dy);
					break;
					
					case 13: 
						//trans + vh flip
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					draw_lit_sprite(newDest, subBmp, dx, dy, litcolour);
					break;
					
					case 18: 
						//pivot, lit
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite_lit(newDest, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 20: 
						//lit + v flip
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_V_FLIP);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite_v_flip_lit(newDest, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 24: 
						//lit + h flip
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_H_FLIP);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot, Flip, and Lit.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect
					masked_blit(newSource, newDest, sx, sy, dx, dy, dw, dh);
					break;
					
					
					default:
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
					
					
				}
			} //end if not rotated
			
			if ( rot != 0 )  //if rotated
			{
				switch(mode)
				{
					case 1: 
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);	//transparent
					rotate_sprite_trans(newDest, subBmp, dx, dy, degrees_to_fixed(rot));
					
					break;
					
					case 2: 
						//pivot?
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite(newDest, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 4: 
						//flip v
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_v_flip(newDest, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 5: 
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);	//trans + v flip
					rotate_sprite_v_flip_trans(newDest, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 6: 
						//pivot + v flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					//return an error, cannot both rotate and pivot
					break;
					
					case 8: 
						//flip h
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rorate and H-Flip.\n", bitmapIndex);
					//return an error, cannot both rotate and flip H
					break;
					
					case 9: 
						//trans + h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot Rotate and Flip a Trans Sprite.\n", bitmapIndex);
					//return an error, cannot rotate and flip a trans sprite
					break;
					
					case 10: 
						//flip H and pivot
					//return error cannot pivot and h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					break;
					
					case 12: 
						//vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 13: 
						//trans + vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_lit(newDest, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 18: 
						//pivot, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 20: 
						//lit + vflip
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_v_flip_lit(newDest, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					break;
					
					case 24: 
						//lit + h flip
					//return an error, cannot both rotate and H flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and H-Flip.\n", bitmapIndex);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Flip a Lit Sprite.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					//return an error, cannot both rotate and VH flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//stretch_blit(newSource, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect.
					masked_blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite(newDest, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					default:
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
				
				}
			} //end rtated, masked
		} //end if masked

		else  //not masked, and not stretched; just blit
		{
			
			if ( rot == 0 ) //if not rotated
			{ 
				switch(mode) 
				{
					case 1:
					//transparent
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_trans_sprite(newDest, subBmp, dx, dy);
					break;
					
					
					case 2: 
						//pivot?
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite(newDest, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite_trans(newDest, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot));
					break;
					
					case 4: 
						//flip v
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_v_flip(newDest, subBmp, dx, dy);
					break;
					
					case 5: 
						//trans + v flip
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_V_FLIP);
					break;
					
					case 6: 
						//pivot + v flip
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite_v_flip(newDest, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					break;
					
					case 8: 
						//vlip h
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_h_flip(newDest, subBmp, dx, dy);
					break;
					
					case 9: 
						//trans + h flip
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_H_FLIP);
					break;
					
					case 10: 
						//flip H and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					//return error cannot pivot and h flip
					break;
					
					case 12:
						//vh flip
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_vh_flip(newDest, subBmp, dx, dy);
					break;
					
					case 13: 
						//trans + vh flip
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_lit_sprite(newDest, subBmp, dx, dy, litcolour);
					break;
					
					case 18: 
						//pivot, lit
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite_lit(newDest, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 20: 
						//lit + v flip
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_V_FLIP);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite_v_flip_lit(newDest, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 24: 
						//lit + h flip
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_H_FLIP);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot, Flip, and Lit.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(newDest, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect
					blit(newSource, newDest, sx, sy, dx, dy, dw, dh);
					break;
					
					
					default:
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
					
					
				}
			} //end if not rotated
			
			if ( rot != 0 )  //if rotated
			{
				switch(mode)
				{
					case 1: 
						blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);//transparent
					rotate_sprite_trans(newDest, subBmp, dx, dy, degrees_to_fixed(rot));
					 
					break;
					
					case 2: 
						//pivot?
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite(newDest, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 4: 
						//flip v
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					rotate_sprite_v_flip(newDest, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 5: 
						//trans + v flip
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_v_flip_trans(newDest, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 6: 
						//pivot + v flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					//return an error, cannot both rotate and pivot
					break;
					
					case 8: 
						//flip h
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rorate and H-Flip.\n", bitmapIndex);
					//return an error, cannot both rotate and flip H
					break;
					
					case 9: 
						//trans + h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot Rotate and Flip a Trans Sprite.\n", bitmapIndex);
					//return an error, cannot rotate and flip a trans sprite
					break;
					
					case 10: 
						//flip H and pivot
					//return error cannot pivot and h flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and H-Flip.\n", bitmapIndex);
					break;
					
					case 12: 
						//vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 13: 
						//trans + vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and VH-Flip.\n", bitmapIndex);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_lit(newDest, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 18: 
						//pivot, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rorate.\n", bitmapIndex);
					break;
					
					case 20: 
						//lit + vflip
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_v_flip_lit(newDest, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Rotate.\n", bitmapIndex);
					break;
					
					case 24: 
						//lit + h flip
					//return an error, cannot both rotate and H flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Rotate and H-Flip.\n", bitmapIndex);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and Flip a Lit Sprite.\n", bitmapIndex);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					//return an error, cannot both rotate and VH flip
					Z_message("Warning: Screen->DrawBitmap(%d) cannot both Pivot and VH-Flip.\n", bitmapIndex);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect.
					blit(newSource, subBmp, sx, sy, 0, 0, dw, dh); 
					rotate_sprite(newDest, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					default:
						return Z_message("Warning: Screen->DrawBitmap(%d) mode flags not possible in this combination!\n", bitmapIndex);
				
				}
			} //end if rotated
		} //end if not masked
	} //end if not stretched
    
	//cleanup
	if(subBmp) 
	{
		//script_drawing_commands.ReleaseSubBitmap(subBmp); //purge the temporary bitmap.
		destroy_bitmap(subBmp);
	}
	if(newSource != destBMP)
	{
		destroy_bitmap(newSource);
	}
}

void do_tileblit(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset, bool is_bmp, char const* funcstr)
{
	/*
	//sdci[1]=layer 
	//sdci[2]=tile
	//sdci[3]=cset
	//sdci[4]=sourcex
	//sdci[5]=sourcey
	//sdci[6]=sourcew
	//sdci[7]=sourceh
	//sdci[8]=destx
	//sdci[9]=desty
	//sdci[10]=destw
	//sdci[11]=desth
	//sdci[12]=rotation/angle
	//scdi[13] = pivot cx
	//sdci[14] = pivot cy
	//scdi[15] = effect flags
	
		// ZScript-side constant values:
		const int32_t BITDX_NORMAL = 0;
		const int32_t BITDX_TRANS = 1; //Translucent
		const int32_t BITDX_PIVOT = 2; //THe sprite will rotate at a specific point, instead of its center.
		const int32_t BITDX_HFLIP = 4; //Horizontal Flip
		const int32_t BITDX_VFLIP = 8; //Vertical Flip.
		//Note:	Some modes cannot be combined. if a combination is not supported, an error
		//	detailing this will be shown in allegro.log.
		
	//scdi[16] = litcolour
		//The allegro docs are wrong. The params are: rotate_sprite_lit(bmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour); 
		//not rotate_sprite_lit(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
	
	//sdci[17]=mask
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	
	*/
	
	int32_t tile = sdci[2]/10000;
	int32_t cset = WRAP_CS(sdci[3]/10000);
	
	int32_t sx = sdci[4]/10000;
	int32_t sy = sdci[5]/10000;
	int32_t sw = sdci[6]/10000;
	//Z_scripterrlog("sh is: %d\n",sdci[5]/10000);
	int32_t sh = sdci[7]/10000;
	//Z_scripterrlog("sh is: %d\n",sdci[6]/10000);
	int32_t dx = sdci[8]/10000;
	int32_t dy = sdci[9]/10000;
	int32_t dw = sdci[10]/10000;
	int32_t dh = sdci[11]/10000;
	float rot = sdci[12]/10000;
	int32_t cx = sdci[13]/10000;
	int32_t cy = sdci[14]/10000;
	int32_t mode = sdci[15]/10000;
	int32_t litcolour = sdci[16]/10000;
	bool masked = (sdci[17] != 0);
	
	int32_t ref = 0;
	
	if ( is_bmp && (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop.
	//Do we need to also check the render target and do the same thing if the 
		//dest == -2 and the render target is not RT_SCREEN?
	dx += xoffset;
	dy += yoffset;
	
	BITMAP *destbmp = nullptr;
	if(is_bmp)
	{
		ref = sdci[DRAWCMD_BMP_TARGET];
		
		if ( ref <= 0 )
		{
			Z_scripterrlog("%s wanted to use to an invalid dest bitmap id: %d. Aborting.\n", funcstr, ref);
			return;
		}
		destbmp = FFCore.GetScriptBitmap(ref);
		if(!destbmp)
		{
			Z_message("Warning: %s dest bitmap %d contains invalid data or is not initialized.\n", ref);
			Z_message("[Note* Deferred drawing or layering order possibly not set right.]\n");
			return;
		}
	}
	else destbmp = bmp;
	
	bool stretched = (sw != dw || sh != dh);
	
	BITMAP* srcbmp = create_bitmap_ex(8, sw, sh);
	//Draw tiles to srcbmp
	{
		clear_bitmap(srcbmp);
		int tx = 0, ty = 0;
		if(sx < 0)
			tx = (sx-15)/16;
		else if(sx > 15)
			tx = sx/16;
		if(sy < 0)
			ty = (sy-15)/16;
		else if(sy > 15)
			ty = sy/16;
		
		int gxoff = -wrap(sx,0,15), gyoff = -wrap(sy,0,15);
		for(int ix = 0; ix <= sw; ix += 16)
		{
			for(int iy = 0; iy <= sh; iy += 16)
			{
				int t = tile+(tx+ix/16);
				int rowdiff = TILEROW(t) - TILEROW(tile);
				t += rowdiff * (sh/16) * TILES_PER_ROW;
				t += (ty+iy/16)*TILES_PER_ROW;
				overtile16(srcbmp, t, ix+gxoff, iy+gyoff, cset, 0);
			}
		}
		
		sx = sy = 0;
	}
    
	BITMAP* subBmp = nullptr;
    
	if(rot != 0 || mode != 0)    
	{
		subBmp = create_bitmap_ex(8,destbmp->w, destbmp->h);//script_drawing_commands.AquireSubBitmap(dw, dh);
		clear_bitmap(subBmp);
        
		if(!subBmp)
		{
			Z_scripterrlog("%s failed to create a sub-bitmap to use for %s. Aborting.\n", funcstr, "rotation");
			return;
		}
	}
    
	//dx = dx + xoffset; //don't do this here!
	//dy = dy + yoffset; //Nor this. It auto-offsets the bitmap by +56. Hmm. The fix that gleeok made isn't being applied to these functions. -Z ( 17th April, 2019 )
    
	if(stretched) 
	{
		if(masked) 
		{	//stretched and masked
			if ( rot == 0 ) 
			{ //if not rotated
				switch(mode) 
				{
					case 1:
					//transparent
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_trans_sprite(destbmp, subBmp, dx, dy);
					break;
					
					
					case 2: 
						//pivot?
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite(destbmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_trans(destbmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					break;
					
					case 4: 
						//flip v
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_v_flip(destbmp, subBmp, dx, dy);
					break;
					
					case 5: 
						//trans + v flip
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_V_FLIP);
					break;
					
					case 6: 
						//pivot + v flip
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_v_flip(destbmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					break;
					
					case 8: 
						//vlip h
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_h_flip(destbmp, subBmp, dx, dy);
					break;
					
					case 9: 
						//trans + h flip
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_H_FLIP);
					break;
					
					case 10: 
						//flip H and pivot
					Z_message("Warning: %s cannot both Pivot and H-Flip.\n", funcstr);
					//return error cannot pivot and h flip
					break;
					
					case 12:
						//vh flip
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_vh_flip(destbmp, subBmp, dx, dy);
					break;
					
					case 13: 
						//trans + vh flip
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: %s cannot both Pivot and VH-Flip.\n", funcstr);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_lit_sprite(destbmp, subBmp, dx, dy, litcolour);
					break;
					
					case 18: 
						//pivot, lit
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_lit(destbmp, subBmp, dx, dy, cx, cy,  degrees_to_fixed(rot),litcolour);
					break;
					
					case 20: 
						//lit + v flip
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_V_FLIP);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_v_flip_lit(destbmp, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 24: 
						//lit + h flip
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_H_FLIP);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: %s cannot both Pivot, Flip, and Lit.\n", funcstr);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect
					masked_stretch_blit(srcbmp, destbmp, sx, sy, sw, sh, dx, dy, dw, dh);
					break;
					
					
					default:
						return Z_message("Warning: %s mode flags not possible in this combination!\n", funcstr);
					
					
				}
			} //end if not rotated
			
			if ( rot != 0 ) //if rotated
			{ 
				switch(mode)
				{
					case 1: 
						//transparent
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_trans(destbmp, subBmp, dx, dy, degrees_to_fixed(rot));
					
					break;
					
					case 2: 
						//pivot?
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite(destbmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					//return an error, cannot both rotate and pivot
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					break;
					
					case 4: 
						//flip v
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip(destbmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 5: 
						//trans + v flip
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip_trans(destbmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 6: 
						//pivot + v flip
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					//return an error, cannot both rotate and pivot
					break;
					
					case 8: 
						//flip h
					Z_message("Warning: %s cannot both Rotate and H-Flip.\n", funcstr);
					//return an error, cannot both rotate and flip H
					break;
					
					case 9: 
						//trans + h flip
					Z_message("Warning: %s cannot Rotate and Flip a Trans Sprite.\n", funcstr);
					//return an error, cannot rotate and flip a trans sprite
					break;
					
					case 10: 
						//flip H and pivot
					//return error cannot pivot and h flip
					Z_message("Warning: %s cannot both Pivot and H-Flip.\n", funcstr);
					break;
					
					case 12: 
						//vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: %s cannot both Rotate and VH-Flip.\n", funcstr);
					break;
					
					case 13: 
						//trans + vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: %s cannot both Rotate and VH-Flip.\n", funcstr);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_lit(destbmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 18: 
						//pivot, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					break;
					
					case 20: 
						//lit + vflip
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip_lit(destbmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					break;
					
					case 24: 
						//lit + h flip
					//return an error, cannot both rotate and H flip
					Z_message("Warning: %s cannot both Rotate and H-Flip.\n", funcstr);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: %s cannot both Pivot and Flip a Lit Sprite.\n", funcstr);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					//return an error, cannot both rotate and VH flip
					Z_message("Warning: %s cannot both Pivot and VH-Flip.\n", funcstr);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect.
					masked_stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite(destbmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					default:
						return Z_message("Warning: %s mode flags not possible in this combination!\n", funcstr);
				
				}
			}
		} //end if stretched and masked 
		
		else  //stretched, not masked
		{
			
		
			if ( rot == 0 ) //if not rotated
			{
				switch(mode) 
				{
					case 1:
					//transparent
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_trans_sprite(destbmp, subBmp, dx, dy);
					break;
					
					
					case 2: 
						//pivot?
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite(destbmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_trans(destbmp, subBmp, dx,dy,  cx,  cy, degrees_to_fixed(rot));
					break;
					
					case 4: 
						//flip v
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_v_flip(destbmp, subBmp, dx, dy);
					break;
					
					case 5: 
						//trans + v flip
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_V_FLIP);
					break;
					
					case 6: 
						//pivot + v flip
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_v_flip(destbmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					break;
					
					case 8: 
						//vlip h
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_h_flip(destbmp, subBmp, dx, dy);
					break;
					
					case 9: 
						//trans + h flip
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_H_FLIP);
					break;
					
					case 10: 
						//flip H and pivot
					Z_message("Warning: %s cannot both Pivot and H-Flip.\n", funcstr);
					//return error cannot pivot and h flip
					break;
					
					case 12:
						//vh flip
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_vh_flip(destbmp, subBmp, dx, dy);
					break;
					
					case 13: 
						//trans + vh flip
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: %s cannot both Pivot and VH-Flip.\n", funcstr);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_lit_sprite(destbmp, subBmp, dx, dy, litcolour);
					break;
					
					case 18: 
						//pivot, lit
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_lit(destbmp, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 20: 
						//lit + v flip
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_V_FLIP);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite_v_flip_lit(destbmp, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 24: 
						//lit + h flip
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_H_FLIP);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: %s cannot both Pivot, Flip, and Lit.\n", funcstr);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect
					stretch_blit(srcbmp, destbmp, sx, sy, sw, sh, dx, dy, dw, dh);
					break;
					
					
					default:
						return Z_message("Warning: %s mode flags not possible in this combination!\n", funcstr);
					
					
				}
			} //end if not rotated
			
			if ( rot != 0 )  //if rotated
			{
				switch(mode)
				{
					case 1: 
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);//transparent
					rotate_sprite_trans(destbmp, subBmp, dx, dy, degrees_to_fixed(rot));
					
					break;
					
					case 2: 
						//pivot?
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					pivot_sprite(destbmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					//return an error, cannot both rotate and pivot
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					break;
					
					case 4: 
						//flip v
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip(destbmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 5: 
						//trans + v flip
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite_v_flip_trans(destbmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 6: 
						//pivot + v flip
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					//return an error, cannot both rotate and pivot
					break;
					
					case 8: 
						//flip h
					Z_message("Warning: %s cannot both Rotate and H-Flip.\n", funcstr);
					//return an error, cannot both rotate and flip H
					break;
					
					case 9: 
						//trans + h flip
					Z_message("Warning: %s cannot Rotate and Flip a Trans Sprite.\n", funcstr);
					//return an error, cannot rotate and flip a trans sprite
					break;
					
					case 10: 
						//flip H and pivot
					//return error cannot pivot and h flip
					Z_message("Warning: %s cannot both Pivot and H-Flip.\n", funcstr);
					break;
					
					case 12: 
						//vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: %s cannot both Rotate and VH-Flip.\n", funcstr);
					break;
					
					case 13: 
						//trans + vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: %s cannot both Rotate and VH-Flip.\n", funcstr);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);//transparent
					rotate_sprite_lit(destbmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 18: 
						//pivot, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					break;
					
					case 20: 
						//lit + vflip
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);//transparent
					rotate_sprite_v_flip_lit(destbmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					break;
					
					case 24: 
						//lit + h flip
					//return an error, cannot both rotate and H flip
					Z_message("Warning: %s cannot both Rotate and H-Flip.\n", funcstr);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: %s cannot both Pivot and Flip a Lit Sprite.\n", funcstr);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					//return an error, cannot both rotate and VH flip
					Z_message("Warning: %s cannot both Pivot and VH-Flip.\n", funcstr);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect.
					stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite(destbmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					default:
						return Z_message("Warning: %s mode flags not possible in this combination!\n", funcstr);
				
				}
			}
			
		} //end if stretched, but not masked
	}
	else //not stretched
	{ 
		
		if(masked) //if masked, but not stretched
		{ 
			
			if ( rot == 0 ) //if not rotated
			{ 
				switch(mode) 
				{
					case 1:
					//transparent
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_trans_sprite(destbmp, subBmp, dx, dy);
					break;
					
					
					case 2: 
						//pivot?
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite(destbmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite_trans(destbmp, subBmp, dx, dy,  cx, cy, degrees_to_fixed(rot));
					break;
					
					case 4: 
						//flip v
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_v_flip(destbmp, subBmp, dx, dy);
					break;
					
					case 5: 
						//trans + v flip
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_V_FLIP);
					break;
					
					case 6: 
						//pivot + v flip
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite_v_flip(destbmp, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot));
					break;
					
					case 8: 
						//vlip h
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_h_flip(destbmp, subBmp, dx, dy);
					break;
					
					case 9: 
						//trans + h flip
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_H_FLIP);
					break;
					
					case 10: 
						//flip H and pivot
					Z_message("Warning: %s cannot both Pivot and H-Flip.\n", funcstr);
					//return error cannot pivot and h flip
					break;
					
					case 12:
						//vh flip
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_vh_flip(destbmp, subBmp, dx, dy);
					break;
					
					case 13: 
						//trans + vh flip
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: %s cannot both Pivot and VH-Flip.\n", funcstr);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_lit_sprite(destbmp, subBmp, dx, dy, litcolour);
					break;
					
					case 18: 
						//pivot, lit
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite_lit(destbmp, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 20: 
						//lit + v flip
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_V_FLIP);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite_v_flip_lit(destbmp, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 24: 
						//lit + h flip
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_H_FLIP);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: %s cannot both Pivot, Flip, and Lit.\n", funcstr);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect
					masked_blit(srcbmp, destbmp, sx, sy, dx, dy, dw, dh);
					break;
					
					
					default:
						return Z_message("Warning: %s mode flags not possible in this combination!\n", funcstr);
					
					
				}
			} //end if not rotated
			
			if ( rot != 0 )  //if rotated
			{
				switch(mode)
				{
					case 1: 
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);	//transparent
					rotate_sprite_trans(destbmp, subBmp, dx, dy, degrees_to_fixed(rot));
					
					break;
					
					case 2: 
						//pivot?
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					pivot_sprite(destbmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					//return an error, cannot both rotate and pivot
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					break;
					
					case 4: 
						//flip v
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_v_flip(destbmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 5: 
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);	//trans + v flip
					rotate_sprite_v_flip_trans(destbmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 6: 
						//pivot + v flip
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					//return an error, cannot both rotate and pivot
					break;
					
					case 8: 
						//flip h
					Z_message("Warning: %s cannot both Rotate and H-Flip.\n", funcstr);
					//return an error, cannot both rotate and flip H
					break;
					
					case 9: 
						//trans + h flip
					Z_message("Warning: %s cannot Rotate and Flip a Trans Sprite.\n", funcstr);
					//return an error, cannot rotate and flip a trans sprite
					break;
					
					case 10: 
						//flip H and pivot
					//return error cannot pivot and h flip
					Z_message("Warning: %s cannot both Pivot and H-Flip.\n", funcstr);
					break;
					
					case 12: 
						//vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: %s cannot both Rotate and VH-Flip.\n", funcstr);
					break;
					
					case 13: 
						//trans + vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: %s cannot both Rotate and VH-Flip.\n", funcstr);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_lit(destbmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 18: 
						//pivot, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					break;
					
					case 20: 
						//lit + vflip
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_v_flip_lit(destbmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					break;
					
					case 24: 
						//lit + h flip
					//return an error, cannot both rotate and H flip
					Z_message("Warning: %s cannot both Rotate and H-Flip.\n", funcstr);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: %s cannot both Pivot and Flip a Lit Sprite.\n", funcstr);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					//return an error, cannot both rotate and VH flip
					Z_message("Warning: %s cannot both Pivot and VH-Flip.\n", funcstr);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//stretch_blit(srcbmp, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect.
					masked_blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite(destbmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					default:
						return Z_message("Warning: %s mode flags not possible in this combination!\n", funcstr);
				
				}
			} //end rtated, masked
		} //end if masked

		else  //not masked, and not stretched; just blit
		{
			
			if ( rot == 0 ) //if not rotated
			{ 
				switch(mode) 
				{
					case 1:
					//transparent
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_trans_sprite(destbmp, subBmp, dx, dy);
					break;
					
					
					case 2: 
						//pivot?
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite(destbmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite_trans(destbmp, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot));
					break;
					
					case 4: 
						//flip v
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_v_flip(destbmp, subBmp, dx, dy);
					break;
					
					case 5: 
						//trans + v flip
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_V_FLIP);
					break;
					
					case 6: 
						//pivot + v flip
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite_v_flip(destbmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					break;
					
					case 8: 
						//vlip h
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_h_flip(destbmp, subBmp, dx, dy);
					break;
					
					case 9: 
						//trans + h flip
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_H_FLIP);
					break;
					
					case 10: 
						//flip H and pivot
					Z_message("Warning: %s cannot both Pivot and H-Flip.\n", funcstr);
					//return error cannot pivot and h flip
					break;
					
					case 12:
						//vh flip
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_vh_flip(destbmp, subBmp, dx, dy);
					break;
					
					case 13: 
						//trans + vh flip
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_TRANS, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: %s cannot both Pivot and VH-Flip.\n", funcstr);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_lit_sprite(destbmp, subBmp, dx, dy, litcolour);
					break;
					
					case 18: 
						//pivot, lit
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite_lit(destbmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 20: 
						//lit + v flip
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_V_FLIP);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite_v_flip_lit(destbmp, subBmp, dx, dy,  cx,  cy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 24: 
						//lit + h flip
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_H_FLIP);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: %s cannot both Pivot, Flip, and Lit.\n", funcstr);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					draw_sprite_ex(destbmp, subBmp, dx, dy, DRAW_SPRITE_LIT, DRAW_SPRITE_VH_FLIP);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect
					blit(srcbmp, destbmp, sx, sy, dx, dy, dw, dh);
					break;
					
					
					default:
						return Z_message("Warning: %s mode flags not possible in this combination!\n", funcstr);
					
					
				}
			} //end if not rotated
			
			if ( rot != 0 )  //if rotated
			{
				switch(mode)
				{
					case 1: 
						blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);//transparent
					rotate_sprite_trans(destbmp, subBmp, dx, dy, degrees_to_fixed(rot));
					 
					break;
					
					case 2: 
						//pivot?
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					pivot_sprite(destbmp, subBmp, dx, dy, cx, cy, degrees_to_fixed(rot));
					//Pivoting requires two more args
					break;
					
					case 3: 
						//pivot + trans
					//return an error, cannot both rotate and pivot
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					break;
					
					case 4: 
						//flip v
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					rotate_sprite_v_flip(destbmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 5: 
						//trans + v flip
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_v_flip_trans(destbmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					case 6: 
						//pivot + v flip
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					//return an error, cannot both rotate and pivot
					break;
					
					case 8: 
						//flip h
					Z_message("Warning: %s cannot both Rotate and H-Flip.\n", funcstr);
					//return an error, cannot both rotate and flip H
					break;
					
					case 9: 
						//trans + h flip
					Z_message("Warning: %s cannot Rotate and Flip a Trans Sprite.\n", funcstr);
					//return an error, cannot rotate and flip a trans sprite
					break;
					
					case 10: 
						//flip H and pivot
					//return error cannot pivot and h flip
					Z_message("Warning: %s cannot both Pivot and H-Flip.\n", funcstr);
					break;
					
					case 12: 
						//vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: %s cannot both Rotate and VH-Flip.\n", funcstr);
					break;
					
					case 13: 
						//trans + vh flip
					//return an error, cannot rotate and VH flip a trans sprite
					Z_message("Warning: %s cannot both Rotate and VH-Flip.\n", funcstr);
					break;
					
					case 14: 
						//pivot and vh flip
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					//return error cannot both pivot and vh flip
					break;
					
					case 16: 
						//lit
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_lit(destbmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 18: 
						//pivot, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					break;
					
					case 20: 
						//lit + vflip
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite_v_flip_lit(destbmp, subBmp, dx, dy, degrees_to_fixed(rot),litcolour);
					break;
					
					case 22: 
						//Pivot, vflip, lit
					//return an error, cannot both rotate and pivot
					Z_message("Warning: %s cannot both Pivot and Rotate.\n", funcstr);
					break;
					
					case 24: 
						//lit + h flip
					//return an error, cannot both rotate and H flip
					Z_message("Warning: %s cannot both Rotate and H-Flip.\n", funcstr);
					break;
					
					case 26: 
						//pivot + lit + hflip
					Z_message("Warning: %s cannot both Pivot and Flip a Lit Sprite.\n", funcstr);
					//return error cannot pivot, lit, and flip
					break;
					
					case 28: 
						//lit + vh flip
					//return an error, cannot both rotate and VH flip
					Z_message("Warning: %s cannot both Pivot and VH-Flip.\n", funcstr);
					break;
					
					case 32: //gouraud
						//Probably not wort supporting. 
					//blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					//draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int32_t x, int32_t y, int32_t c1, int32_t c2, int32_t c3, int32_t c4);
					break;
					
					case 0: 
						//no effect.
					blit(srcbmp, subBmp, sx, sy, 0, 0, dw, dh); 
					rotate_sprite(destbmp, subBmp, dx, dy, degrees_to_fixed(rot));
					break;
					
					default:
						return Z_message("Warning: %s mode flags not possible in this combination!\n", funcstr);
				
				}
			} //end if rotated
		} //end if not masked
	} //end if not stretched
    
	//cleanup
	if(subBmp) 
	{
		//script_drawing_commands.ReleaseSubBitmap(subBmp); //purge the temporary bitmap.
		destroy_bitmap(subBmp);
	}
	destroy_bitmap(srcbmp);
}

void do_comboblit(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset, bool is_bmp)
{
	//sdci[2]: combo -> tile
	int cid = sdci[2]/10000;
	if(unsigned(cid) >= MAXCOMBOS)
	{
		Z_scripterrlog("ComboBlit tried to draw invalid combo id '%d'\n", cid);
		return;
	}
	sdci[2] = GET_DRAWING_COMBO(cid).tile * 10000;
	do_tileblit(bmp, sdci, xoffset, yoffset, is_bmp, "ComboBlit()");
}

void bmp_do_drawquad3dr(BITMAP *bmp, int32_t i, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	
	//sdci[1]=layer
	//sdci[2]=pos[12]
	//sdci[3]=uv[8]
	//sdci[4]=color[4]
	//sdci[5]=size[2]
	//sdci[6]=flip
	//sdci[7]=tile/combo
	//sdci[8]=polytype
	//sdci[9] = other bitmap as texture
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
	{
	Z_scripterrlog("bitmap->Quad3D() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
	}
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
	
	std::vector<int32_t>* v_ptr = (std::vector<int32_t>*)script_drawing_commands[i].GetPtr();
	
	if(!v_ptr)
	{
		al_trace("Quad3d: Vector pointer is null! Internal error. \n");
		return;
	}
	
	std::vector<int32_t> &v = *v_ptr;
	
	if(v.empty())
		return;
		
	int32_t* pos = &v[0];
	int32_t* uv = &v[12];
	int32_t* col = &v[20];
	int32_t* size = &v[24];
	
	int32_t w = size[0]; //magic numerical constants... yuck.
	int32_t h = size[1];
	int32_t flip = (sdci[6]/10000)&3;
	int32_t tile = sdci[7]/10000;
	int32_t polytype = sdci[8]/10000;
	int32_t quad_render_source = sdci[9];
	Z_scripterrlog("Quad3D texture is %d\n", quad_render_source);
	
	polytype = vbound(polytype, 0, 14);
	
	int32_t tex_width = w*16;
	int32_t tex_height = h*16;
	
	bool mustDestroyBmp = false;
	BITMAP *tex=NULL; 
	
	
	bool tex_is_bitmap = ( sdci[9] != 0 );
	//Z_scripterrlog("sdci[9] is %d\n", quad_render_source);
	//Z_scripterrlog("sdci[DRAWCMD_BMP_TARGET] is %d\n", sdci[DRAWCMD_BMP_TARGET]);
	BITMAP *bmptexture;
	
	if ( tex_is_bitmap ) bmptexture = FFCore.GetScriptBitmap(quad_render_source);
	
	if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
   
	
	if ( !tex_is_bitmap )
	{
	tex = script_drawing_commands.GetSmallTextureBitmap(w,h);
	
	if(!tex)
	{
		mustDestroyBmp = true;
		tex = create_bitmap_ex(8, tex_width, tex_height);
		clear_bitmap(tex);
	}
	if(((w-1) & w) != 0 || ((h-1) & h) != 0)
	{
		Z_message("Quad3d() : Args h, w, must be in powers of two! Power of 2 error with %i, %i.", w, h);
		return; //non power of two error
	}
	if(tile > 0)   // TILE
		{
		TileHelper::OverTile(tex, tile, 0, 0, w, h, col[0], flip);
		}
		else		// COMBO
		{
		auto& c = GET_DRAWING_COMBO(vbound(abs(tile), 0, 0xffff));
		const int32_t tiletodraw = combo_tile(c, 0, 0);
		flip = flip ^ c.flip;
		
		TileHelper::OldPutTile(tex, tiletodraw, 0, 0, w, h, col[0], flip);
		}
		
	V3D_f V1 = { static_cast<float>(pos[0]+xoffset), static_cast<float>(pos[1] +yoffset), static_cast<float>(pos[2]),  static_cast<float>(uv[0]), static_cast<float>(uv[1]), col[0] };
	V3D_f V2 = { static_cast<float>(pos[3]+xoffset), static_cast<float>(pos[4] +yoffset), static_cast<float>(pos[5]),  static_cast<float>(uv[2]), static_cast<float>(uv[3]), col[1] };
	V3D_f V3 = { static_cast<float>(pos[6]+xoffset), static_cast<float>(pos[7] +yoffset), static_cast<float>(pos[8]),  static_cast<float>(uv[4]), static_cast<float>(uv[5]), col[2] };
	V3D_f V4 = { static_cast<float>(pos[9]+xoffset), static_cast<float>(pos[10]+yoffset), static_cast<float>(pos[11]), static_cast<float>(uv[6]), static_cast<float>(uv[7]), col[3] };
	
	quad3d_f(refbmp, polytype, tex, &V1, &V2, &V3, &V4);
	if(mustDestroyBmp)
		destroy_bitmap(tex);
	}
	else
	{
		
		if ( !bmptexture ) 
		{
			Z_scripterrlog("Bitmap pointer used as a texture in %s is uninitialised.\n Defaulting to using a tile as a texture.\n", "bitmap->Quad3D()");
			tex_is_bitmap = 0;
			return;
		}
	if ( !isPowerOfTwo(bmptexture->h) ) Z_scripterrlog("HEIGHT of Bitmap ( pointer %d ) provided as a render source for bitmap->Quad3D is not a POWER OF TWO.\nTextels may render improperly!\n", quad_render_source);
		if ( !isPowerOfTwo(bmptexture->w) ) Z_scripterrlog("WIDTH of Bitmap ( pointer %d ) provided as a render source for bitmap->Quad3D is not a POWER OF TWO.\nTextels may render improperly!\n", quad_render_source);
		if ( !isPowerOfTwo(h) ) Z_scripterrlog("WIDTH ARG (%d) provided as a render source for bitmap->Quad3D is not a POWER OF TWO.\nTextels may render improperly!\n", h);
		if ( !isPowerOfTwo(w) ) Z_scripterrlog("HEIGHT ARG (%d) provided as a render source for bitmap->Quad3D is not a POWER OF TWO.\nTextels may render improperly!\n", w);
		
	V3D_f V1 = { static_cast<float>(pos[0]+xoffset), static_cast<float>(pos[1] +yoffset), static_cast<float>(pos[2]),  static_cast<float>(uv[0]), static_cast<float>(uv[1]), col[0] };
	V3D_f V2 = { static_cast<float>(pos[3]+xoffset), static_cast<float>(pos[4] +yoffset), static_cast<float>(pos[5]),  static_cast<float>(uv[2]), static_cast<float>(uv[3]), col[1] };
	V3D_f V3 = { static_cast<float>(pos[6]+xoffset), static_cast<float>(pos[7] +yoffset), static_cast<float>(pos[8]),  static_cast<float>(uv[4]), static_cast<float>(uv[5]), col[2] };
	V3D_f V4 = { static_cast<float>(pos[9]+xoffset), static_cast<float>(pos[10]+yoffset), static_cast<float>(pos[11]), static_cast<float>(uv[6]), static_cast<float>(uv[7]), col[3] };
		
	BITMAP *foo = create_bitmap_ex(8, 256, 176);
		
	//quad3d_f(refbmp, polytype, foo, &V1, &V2, &V3, &V4);	
	quad3d_f(refbmp, polytype, bmptexture, &V1, &V2, &V3, &V4); 
	destroy_bitmap(foo);
		
	}
	
	
		
}



void bmp_do_drawtriangle3dr(BITMAP *bmp, int32_t i, int32_t *sdci, int32_t xoffset, int32_t yoffset)
{
	//sdci[1]=layer
	//sdci[2]=pos[9]
	//sdci[3]=uv[6]
	//sdci[4]=color[3]
	//sdci[5]=size[2]
	//sdci[6]=flip
	//sdci[7]=tile/combo
	//sdci[8]=polytype
	//sdci[9] bitmap as texture
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	if ( sdci[DRAWCMD_BMP_TARGET] <= 0 )
	{
	Z_scripterrlog("bitmap->Triangle3D() wanted to write to an invalid bitmap id: %d. Aborting.\n", sdci[DRAWCMD_BMP_TARGET]);
	return;
	}
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
	
	std::vector<int32_t>* v_ptr = (std::vector<int32_t>*)script_drawing_commands[i].GetPtr();
	
	if(!v_ptr)
	{
		al_trace("bitmap->Triangle3d: Vector pointer is null! Internal error. \n");
		return;
	}
	
	std::vector<int32_t> &v = *v_ptr;
	
	if(v.empty())
		return;
		
	int32_t* pos = &v[0];
	int32_t* uv = &v[9];
	int32_t* col = &v[15];
	int32_t* size = &v[18];
	
	int32_t w = size[0]; //magic numerical constants... yuck.
	int32_t h = size[1];
	int32_t flip = (sdci[6]/10000)&3;
	int32_t tile = sdci[7]/10000;
	int32_t polytype = sdci[8]/10000;
	int32_t quad_render_source = sdci[9];
	polytype = vbound(polytype, 0, 14);
	
	if(((w-1) & w) != 0 || ((h-1) & h) != 0)
	{
		Z_message("bitmap->Triangle3d() : Args h, w, must be in powers of two! Power of 2 error with %i, %i.", w, h);
		return; //non power of two error
	}
	
	int32_t tex_width = w*16;
	int32_t tex_height = h*16;
	
	bool mustDestroyBmp = false;
	BITMAP *tex = script_drawing_commands.GetSmallTextureBitmap(w,h);
	
	if(!tex)
	{
		mustDestroyBmp = true;
		tex = create_bitmap_ex(8, tex_width, tex_height);
		clear_bitmap(tex);
	}
	
	bool tex_is_bitmap = ( sdci[9] != 0 );
	BITMAP *bmptexture=NULL;
	if ( tex_is_bitmap ) 
	{
		bmptexture = FFCore.GetScriptBitmap(quad_render_source);
		if ( !bmptexture ) 
		{
			Z_scripterrlog("Bitmap pointer used as a texture in %s is uninitialised.\n Defaulting to using a tile as a texture.\n", "bitmap->Triangle3()");
			tex_is_bitmap = 0;
		}
	}
	
	if ( !tex_is_bitmap )
	{
		if(tile > 0)   // TILE
		{
		TileHelper::OverTile(tex, tile, 0, 0, w, h, col[0], flip);
		}
		else		// COMBO
		{
		auto& c = GET_DRAWING_COMBO(vbound(abs(tile), 0, 0xffff));
		const int32_t tiletodraw = combo_tile(c, 0, 0);
		flip = flip ^ c.flip;
		
		TileHelper::OldPutTile(tex, tiletodraw, 0, 0, w, h, col[0], flip);
		}
		
		V3D_f V1 = { static_cast<float>(pos[0]+xoffset), static_cast<float>(pos[1] +yoffset), static_cast<float>(pos[2]), static_cast<float>(uv[0]), static_cast<float>(uv[1]), col[0] };
		V3D_f V2 = { static_cast<float>(pos[3]+xoffset), static_cast<float>(pos[4] +yoffset), static_cast<float>(pos[5]), static_cast<float>(uv[2]), static_cast<float>(uv[3]), col[1] };
		V3D_f V3 = { static_cast<float>(pos[6]+xoffset), static_cast<float>(pos[7] +yoffset), static_cast<float>(pos[8]), static_cast<float>(uv[4]), static_cast<float>(uv[5]), col[2] };
		
		triangle3d_f(refbmp, polytype, tex, &V1, &V2, &V3);
	}
	else
	{
	if ( !isPowerOfTwo(bmptexture->h) ) Z_scripterrlog("HEIGHT of Bitmap ( pointer %d ) provided as a render source for bitmap->Triangle3D is not a POWER OF TWO.\nTextels may render improperly!\n", quad_render_source);
		if ( !isPowerOfTwo(bmptexture->w) ) Z_scripterrlog("WIDTH of Bitmap ( pointer %d ) provided as a render source for bitmap->Triangle3D is not a POWER OF TWO.\nTextels may render improperly!\n", quad_render_source);
		if ( !isPowerOfTwo(w) ) Z_scripterrlog("WIDTH ARG (%d) provided as a render source for bitmap->Triangle3D is not a POWER OF TWO.\nTextels may render improperly!\n", w);
		if ( !isPowerOfTwo(h) ) Z_scripterrlog("HEIGHT ARG (%d) provided as a render source for bitmap->Triangle3D is not a POWER OF TWO.\nTextels may render improperly!\n", h);
		
	V3D_f V1 = { static_cast<float>(pos[0]+xoffset), static_cast<float>(pos[1] +yoffset), static_cast<float>(pos[2]), static_cast<float>(uv[0]), static_cast<float>(uv[1]), col[0] };
	V3D_f V2 = { static_cast<float>(pos[3]+xoffset), static_cast<float>(pos[4] +yoffset), static_cast<float>(pos[5]), static_cast<float>(uv[2]), static_cast<float>(uv[3]), col[1] };
	V3D_f V3 = { static_cast<float>(pos[6]+xoffset), static_cast<float>(pos[7] +yoffset), static_cast<float>(pos[8]), static_cast<float>(uv[4]), static_cast<float>(uv[5]), col[2] };
		
	triangle3d_f(refbmp, polytype, bmptexture, &V1, &V2, &V3);	
		
		
	}
	if(mustDestroyBmp)
		destroy_bitmap(tex);
		
}


bool is_layer_transparent(const mapscr& m, int32_t layer)
{
	layer = vbound(layer, 0, 5);
	return m.layeropacity[layer] == 128;
}

mapscr *getmapscreen(int32_t map_index, int32_t screen, int32_t layer)   //returns NULL for invalid or non-existent layer
{
	mapscr *base_scr;
	int32_t index = map_index*MAPSCRS+screen;
	
	if((uint32_t)layer > 6 || (uint32_t)index >= TheMaps.size())
		return NULL;
		
	if(layer != 0)
	{
		layer = layer - 1;
		
		base_scr=&(TheMaps[index]);
		
		if(base_scr->layermap[layer]==0)
			return NULL;
			
		index=(base_scr->layermap[layer]-1)*MAPSCRS+base_scr->layerscreen[layer];
		
		if((uint32_t)index >= TheMaps.size())   // Might as well make sure
			return NULL;
	}
	
	return &(TheMaps[index]);
}

static bool transparent_combo(int32_t id)
{
	if(unsigned(id) >= MAXCOMBOS) return false;
	return bool(combobuf[id].animflags & AF_TRANSPARENT);
}

void draw_mapscr(BITMAP *b, const mapscr& m, int32_t x, int32_t y, bool transparent)
{
	for(int32_t i(0); i < 176; ++i)
	{
		const int32_t x2 = ((i&15)<<4) + x;
		const int32_t y2 = (i&0xF0) + y;
		
		if(transparent != transparent_combo(m.data[i]))
		{
			overcomboblocktranslucent(b, x2, y2, m.data[i], m.cset[i], 1, 1, 128);
		}
		else
		{
			overcomboblock(b, x2, y2, m.data[i], m.cset[i], 1, 1);
		}
	}
}

void draw_map_solidity(BITMAP *b, const mapscr& m, int32_t x, int32_t y)
{
	BITMAP* square = create_bitmap_ex(8,16,16);
	
	for(int32_t i(0); i < 176; ++i)
	{
		const int32_t x2 = ((i&15)<<4) + x;
		const int32_t y2 = (i&0xF0) + y;
		//Blit the palette index of the solidity value.
		clear_to_color(square,(combobuf[m.data[i]].walk&15));
		if (get_qr(qr_BROKEN_DRAWSCREEN_FUNCTIONS)) blit(square, b, 0, 0, x2, y2, square->w, square->h);
		else masked_blit(square, b, 0, 0, x2, y2, square->w, square->h);
	}
	destroy_bitmap(square);
}

void do_bmpdrawscreen_solidmaskr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset, bool isOffScreen)
{
	//sdci[1]=layer
	//sdci[2]=map
	//sdci[3]=screen
	//sdci[4]=x
	//sdci[5]=y
	//sdci[6]=rotation
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;

	if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
	
	int32_t map = (sdci[2]/10000)-1; //zscript map indices start at 1.
	int32_t screen = sdci[3]/10000;
	int32_t x = sdci[4]/10000;
	int32_t y = sdci[5]/10000;
	int32_t x1 = x + xoffset;
	int32_t y1 = y + yoffset;
	int32_t rotation = sdci[6]/10000;
	uint32_t index = (uint32_t)map_screen_index(map, screen);

	if(index >= TheMaps.size())
	{
		al_trace("DrawScreen: invalid map or screen index. \n");
		return;
	}
	
	const mapscr & m = TheMaps[index];
	
	
	BITMAP* b = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
	
	if(rotation != 0)
		b = script_drawing_commands.AquireSubBitmap(256, 176);
		
	//draw layer 0
	draw_map_solidity(b, m, x1, y1);
	if (get_qr(qr_BROKEN_DRAWSCREEN_FUNCTIONS))
	{
		for(int32_t i(0); i < 6; ++i)
		{
			if(m.layermap[i] == 0) continue;
			
			uint32_t layer_screen_index = (m.layermap[i]-1) * MAPSCRS + m.layerscreen[i];
			
			if(layer_screen_index >= TheMaps.size())
				continue;
			
			//draw valid layers
			draw_map_solidity(b, TheMaps[ layer_screen_index ], x1, y1);
		}
	}
	
	if(rotation != 0) // rotate
	{
		rotate_sprite(refbmp, b, x1, y1, degrees_to_fixed(rotation));
		script_drawing_commands.ReleaseSubBitmap(b);
	}
}

void draw_map_solid(BITMAP *b, const mapscr& m, int32_t x, int32_t y)
{
	BITMAP* square = create_bitmap_ex(8,16,16);
	BITMAP* subsquare = create_bitmap_ex(8,16,16);
	clear_to_color(subsquare,1);
	
	for(int32_t i(0); i < 176; ++i)
	{
		const int32_t x2 = ((i&15)<<4) + x;
		const int32_t y2 = (i&0xF0) + y;
		//Blit the palette index of the solidity value.
		clear_bitmap(square);
		int32_t sol = (combobuf[m.data[i]].walk);
		if ( sol & 1 )
		{
			blit(subsquare, square, 0, 0, 0, 0, 8, 8);
		}
		if ( sol & 2 )
		{
			blit(subsquare, square, 0, 0, 0, 8, 8, 8);
		}
		if ( sol & 4 )
		{
			blit(subsquare, square, 0, 0, 8, 0, 8, 8);
		}
		if ( sol &8 )	{
			blit(subsquare, square, 0, 0, 8, 8, 8, 8);
		}
		
		if (get_qr(qr_BROKEN_DRAWSCREEN_FUNCTIONS)) blit(square, b, 0, 0, x2, y2, square->w, square->h);
		else masked_blit(square, b, 0, 0, x2, y2, square->w, square->h);
	}
	destroy_bitmap(square);
	destroy_bitmap(subsquare);
}

void do_bmpdrawscreen_solidr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset, bool isOffScreen)
{
	//sdci[1]=layer
	//sdci[2]=map
	//sdci[3]=screen
	//sdci[4]=x
	//sdci[5]=y
	//sdci[6]=rotation
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;

	if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
	
	int32_t map = (sdci[2]/10000)-1; //zscript map indices start at 1.
	int32_t screen = sdci[3]/10000;
	int32_t x = sdci[4]/10000;
	int32_t y = sdci[5]/10000;
	int32_t x1 = x + xoffset;
	int32_t y1 = y + yoffset;
	int32_t rotation = sdci[6]/10000;
	
	uint32_t index = (uint32_t)map_screen_index(map, screen);
	
	if(index >= TheMaps.size())
	{
		al_trace("DrawScreen: invalid map or screen index. \n");
		return;
	}
	
	const mapscr & m = TheMaps[index];
	
	
	BITMAP* b = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
	
	if(rotation != 0)
		b = script_drawing_commands.AquireSubBitmap(256, 176);
		
	//draw layer 0
	draw_map_solid(b, m, x1, y1);
	
	for(int32_t i(0); i < 6; ++i) //This one doesn't need the QR; it works just fine.
	{
		if(m.layermap[i] == 0) continue;
		
		uint32_t layer_screen_index = (m.layermap[i]-1) * MAPSCRS + m.layerscreen[i];
		
		if(layer_screen_index >= TheMaps.size())
			continue;
		
		//draw valid layers
		draw_map_solid(b, TheMaps[ layer_screen_index ], x1, y1);
	}
	
	if(rotation != 0) // rotate
	{
		rotate_sprite(refbmp, b, x1, y1, degrees_to_fixed(rotation));
		script_drawing_commands.ReleaseSubBitmap(b);
	}
}

void draw_map_cflag(BITMAP *b, const mapscr& m, int32_t x, int32_t y)
{
	BITMAP* square = create_bitmap_ex(8,16,16);
	
	for(int32_t i(0); i < 176; ++i)
	{
		const int32_t x2 = ((i&15)<<4) + x;
		const int32_t y2 = (i&0xF0) + y;
		//Blit the palette index of the solidity value.
		clear_to_color(square,m.sflag[i]);
		if (get_qr(qr_BROKEN_DRAWSCREEN_FUNCTIONS)) blit(square, b, 0, 0, x2, y2, square->w, square->h);
		else masked_blit(square, b, 0, 0, x2, y2, square->w, square->h);
	}
	destroy_bitmap(square);
}

void do_bmpdrawscreen_cflagr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset, bool isOffScreen)
{
	//sdci[1]=layer
	//sdci[2]=map
	//sdci[3]=screen
	//sdci[4]=x
	//sdci[5]=y
	//sdci[6]=rotation
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;

	if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
	
	int32_t map = (sdci[2]/10000)-1; //zscript map indices start at 1.
	int32_t screen = sdci[3]/10000;
	int32_t x = sdci[4]/10000;
	int32_t y = sdci[5]/10000;
	int32_t x1 = x + xoffset;
	int32_t y1 = y + yoffset;
	int32_t rotation = sdci[6]/10000;
	
	uint32_t index = (uint32_t)map_screen_index(map, screen);
	
	if(index >= TheMaps.size())
	{
		al_trace("DrawScreen: invalid map or screen index. \n");
		return;
	}
	
	const mapscr & m = TheMaps[index];
	
	
	BITMAP* b = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
	
	if(rotation != 0)
		b = script_drawing_commands.AquireSubBitmap(256, 176);
		
	//draw layer 0
	draw_map_cflag(b, m, x1, y1);
	if (get_qr(qr_BROKEN_DRAWSCREEN_FUNCTIONS))
	{
		for(int32_t i(0); i < 6; ++i)
		{
			if(m.layermap[i] == 0) continue;
			
			uint32_t layer_screen_index = (m.layermap[i]-1) * MAPSCRS + m.layerscreen[i];
			
			if(layer_screen_index >= TheMaps.size())
				continue;
			
			//draw valid layers
			draw_map_cflag(b, TheMaps[ layer_screen_index ], x1, y1);
		}
	}
	
	if(rotation != 0) // rotate
	{
		rotate_sprite(refbmp, b, x1, y1, degrees_to_fixed(rotation));
		script_drawing_commands.ReleaseSubBitmap(b);
	}
}


void draw_map_combotype(BITMAP *b, const mapscr& m, int32_t x, int32_t y)
{
	BITMAP* square = create_bitmap_ex(8,16,16);
	
	for(int32_t i(0); i < 176; ++i)
	{
		const int32_t x2 = ((i&15)<<4) + x;
		const int32_t y2 = (i&0xF0) + y;
		//Blit the palette index of the solidity value.
		clear_to_color(square,(combobuf[m.data[i]].type));
		if (get_qr(qr_BROKEN_DRAWSCREEN_FUNCTIONS)) blit(square, b, 0, 0, x2, y2, square->w, square->h);
		else masked_blit(square, b, 0, 0, x2, y2, square->w, square->h);
	}
	destroy_bitmap(square);
}

void do_bmpdrawscreen_ctyper(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset, bool isOffScreen)
{
	//sdci[1]=layer
	//sdci[2]=map
	//sdci[3]=screen
	//sdci[4]=x
	//sdci[5]=y
	//sdci[6]=rotation
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;

	if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
	
	int32_t map = (sdci[2]/10000)-1; //zscript map indices start at 1.
	int32_t screen = sdci[3]/10000;
	int32_t x = sdci[4]/10000;
	int32_t y = sdci[5]/10000;
	int32_t x1 = x + xoffset;
	int32_t y1 = y + yoffset;
	int32_t rotation = sdci[6]/10000;
	
	uint32_t index = (uint32_t)map_screen_index(map, screen);
	
	if(index >= TheMaps.size())
	{
		al_trace("DrawScreen: invalid map or screen index. \n");
		return;
	}
	
	const mapscr & m = TheMaps[index];
	
	
	BITMAP* b = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
	
	if(rotation != 0)
		b = script_drawing_commands.AquireSubBitmap(256, 176);
		
	//draw layer 0
	draw_map_combotype(b, m, x1, y1);
	
	if (get_qr(qr_BROKEN_DRAWSCREEN_FUNCTIONS))
	{
		for(int32_t i(0); i < 6; ++i)
		{
			if(m.layermap[i] == 0) continue;
			
			uint32_t layer_screen_index = (m.layermap[i]-1) * MAPSCRS + m.layerscreen[i];
			
			if(layer_screen_index >= TheMaps.size())
				continue;
			
			//draw valid layers
			draw_map_combotype(b, TheMaps[ layer_screen_index ], x1, y1);
		}
	}
	
	if(rotation != 0) // rotate
	{
		rotate_sprite(refbmp, b, x1, y1, degrees_to_fixed(rotation));
		script_drawing_commands.ReleaseSubBitmap(b);
	}
}


void draw_map_comboiflag(BITMAP *b, const mapscr& m, int32_t x, int32_t y)
{
	BITMAP* square = create_bitmap_ex(8,16,16);
	
	for(int32_t i(0); i < 176; ++i)
	{
		const int32_t x2 = ((i&15)<<4) + x;
		const int32_t y2 = (i&0xF0) + y;
		//Blit the palette index of the solidity value.
		clear_to_color(square,(combobuf[m.data[i]].flag));
		if (get_qr(qr_BROKEN_DRAWSCREEN_FUNCTIONS)) blit(square, b, 0, 0, x2, y2, square->w, square->h);
		else masked_blit(square, b, 0, 0, x2, y2, square->w, square->h);
	}
	destroy_bitmap(square);
}

void do_bmpdrawscreen_ciflagr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset, bool isOffScreen)
{
	//sdci[1]=layer
	//sdci[2]=map
	//sdci[3]=screen
	//sdci[4]=x
	//sdci[5]=y
	//sdci[6]=rotation
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;

	if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
	
	int32_t map = (sdci[2]/10000)-1; //zscript map indices start at 1.
	int32_t screen = sdci[3]/10000;
	int32_t x = sdci[4]/10000;
	int32_t y = sdci[5]/10000;
	int32_t x1 = x + xoffset;
	int32_t y1 = y + yoffset;
	int32_t rotation = sdci[6]/10000;
	
	uint32_t index = (uint32_t)map_screen_index(map, screen);
	
	if(index >= TheMaps.size())
	{
		al_trace("DrawScreen: invalid map or screen index. \n");
		return;
	}
	
	const mapscr & m = TheMaps[index];
	
	
	BITMAP* b = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
	
	if(rotation != 0)
		b = script_drawing_commands.AquireSubBitmap(256, 176);
		
	//draw layer 0
	draw_map_comboiflag(b, m, x1, y1);
	
	if (get_qr(qr_BROKEN_DRAWSCREEN_FUNCTIONS))
	{
		for(int32_t i(0); i < 6; ++i)
		{
			if(m.layermap[i] == 0) continue;
			
			uint32_t layer_screen_index = (m.layermap[i]-1) * MAPSCRS + m.layerscreen[i];
			
			if(layer_screen_index >= TheMaps.size())
				continue;
			
			//draw valid layers
			draw_map_comboiflag(b, TheMaps[ layer_screen_index ], x1, y1);
		}
	}
	
	if(rotation != 0) // rotate
	{
		rotate_sprite(refbmp, b, x1, y1, degrees_to_fixed(rotation));
		script_drawing_commands.ReleaseSubBitmap(b);
	}
}

void do_drawlayerr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset, bool isOffScreen)
{
	//sdci[1]=layer
	//sdci[2]=map
	//sdci[3]=screen
	//sdci[4]=layer
	//sdci[5]=x
	//sdci[6]=y
	//sdci[7]=rotation
	//sdci[8]=opacity
	
	int32_t map = (sdci[2]/10000)-1; //zscript map indices start at 1.
	int32_t screen = sdci[3]/10000;
	int32_t sourceLayer = vbound(sdci[4]/10000, 0, 6);
	int32_t x = sdci[5]/10000;
	int32_t y = sdci[6]/10000;
	int32_t x1 = x + xoffset;
	int32_t y1 = y + yoffset;
	int32_t rotation = sdci[7]/10000;
	int32_t opacity = sdci[8]/10000;
	
	uint32_t index = (uint32_t)map_screen_index(map, screen);
	const mapscr* m = getmapscreen(map, screen, sourceLayer);
	
	if(!m) //no need to log it.
		return;

	if(index >= TheMaps.size())
	{
		al_trace("DrawLayer: invalid map index \"%i\". Map count is %lu.\n", index, TheMaps.size());
		return;
	}
	
	const mapscr & l = *m;
	
	BITMAP* b = bmp;
	
	if(rotation != 0)
		b = script_drawing_commands.AquireSubBitmap(256, 176);
		
		
	const int32_t maxX = isOffScreen ? 512 : 256;
	const int32_t maxY = isOffScreen ? 512 : 176 + yoffset;
	bool transparent = opacity <= 128;
	
	if(rotation != 0) // rotate
	{
		draw_mapscr(b, l, x1, y1, transparent);
		
		rotate_sprite(bmp, b, x1, y1, degrees_to_fixed(rotation));
		script_drawing_commands.ReleaseSubBitmap(b);
	}
	else
	{
		for(int32_t i(0); i < 176; ++i)
		{
			const int32_t x2 = ((i&15)<<4) + x1;
			const int32_t y2 = (i&0xF0) + y1;
			
			if(x2 > -16 && x2 < maxX && y2 > -16 && y2 < maxY)   //in clipping rect
			{
				if(opacity < 128 != transparent_combo(l.data[i]))
				{	
					overcomboblocktranslucent(b, x2, y2, l.data[i], l.cset[i], 1, 1, 128);
				}
				else
				{
					overcomboblock(b, x2, y2, l.data[i], l.cset[i], 1, 1);
				}
			}
		}
	}
	
	//putscr
}



void do_drawscreenr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset, bool isOffScreen)
{
    //sdci[1]=layer
    //sdci[2]=map
    //sdci[3]=screen
    //sdci[4]=x
    //sdci[5]=y
    //sdci[6]=rotation
    
    int32_t map = (sdci[2]/10000)-1; //zscript map indices start at 1.
    int32_t screen = sdci[3]/10000;
    int32_t x = sdci[4]/10000;
    int32_t y = sdci[5]/10000;
    int32_t x1 = x + xoffset;
    int32_t y1 = y + yoffset;
    int32_t rotation = sdci[6]/10000;
    
    uint32_t index = (uint32_t)map_screen_index(map, screen);
    
    if(index >= TheMaps.size())
    {
        al_trace("DrawScreen: invalid map or screen index. \n");
        return;
    }
    
    const mapscr & m = TheMaps[index];
    
    
    BITMAP* b = bmp;
    
    if(rotation != 0)
        b = script_drawing_commands.AquireSubBitmap(256, 176);
        
    //draw layer 0
    draw_mapscr(b, m, x1, y1, false);
    
    for(int32_t i(0); i < 6; ++i)
    {
        if(m.layermap[i] == 0) continue;
        
        uint32_t layer_screen_index = (m.layermap[i]-1) * MAPSCRS + m.layerscreen[i];
        
        if(layer_screen_index >= TheMaps.size())
            continue;
            
        bool trans = m.layeropacity[i] == 128;
        
        //draw valid layers
        draw_mapscr(b, TheMaps[ layer_screen_index ], x1, y1, trans);
    }
    
    if(rotation != 0) // rotate
    {
        rotate_sprite(bmp, b, x1, y1, degrees_to_fixed(rotation));
        script_drawing_commands.ReleaseSubBitmap(b);
    }
}


void do_bmpdrawlayerr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset, bool isOffScreen)
{
    //sdci[1]=layer
    //sdci[2]=map
    //sdci[3]=screen
    //sdci[4]=layer
    //sdci[5]=x
    //sdci[6]=y
    //sdci[7]=rotation
	//[8] noclip
    //sdci[9]=opacity
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
    
    int32_t map = (sdci[2]/10000)-1; //zscript map indices start at 1.
    int32_t screen = sdci[3]/10000;
    int32_t sourceLayer = vbound(sdci[4]/10000, 0, 6);
    int32_t x = sdci[5]/10000;
    int32_t y = sdci[6]/10000;
    int32_t rotation = sdci[7]/10000;

	byte noclip = 0;//(sdci[8]!=0);
    int32_t opacity = sdci[8]/10000;
    uint32_t index = (uint32_t)map_screen_index(map, screen);
    const mapscr* m = getmapscreen(map, screen, sourceLayer);
    
    if(!m) //no need to log it.
        return;

	if(index >= TheMaps.size())
	{
		Z_scripterrlog("DrawLayer: invalid map index \"%i\". Map count is %d.\n", index, TheMaps.size());
		return;
	}
    
    const mapscr & l = *m;
    
    BITMAP* b = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
    if(rotation != 0)
        b = script_drawing_commands.AquireSubBitmap(256, 176);
        
        
    const int32_t maxX = isOffScreen ? 512 : 256;
    const int32_t maxY = isOffScreen ? 512 : 176 + yoffset;
    bool transparent = opacity <= 128;
    
    if(rotation != 0) // rotate
    {
        draw_mapscr(b, l, x, y, transparent);
        
        rotate_sprite(refbmp, b, x, y, degrees_to_fixed(rotation));
        script_drawing_commands.ReleaseSubBitmap(b);
    }
    else
    {
		for(int32_t i(0); i < 176; ++i)
        {
            const int32_t x2 = ((i&15)<<4) + x;
            const int32_t y2 = (i&0xF0) + y;
            
            //if(noclip&&(x2 > -16 && x2 < maxX && y2 > -16 && y2 < maxY))   //in clipping rect
            {
                auto& c = GET_DRAWING_COMBO(l.data[i]);
                const int32_t tile = combo_tile(c, x2, y2);

                if(opacity < 128 != transparent_combo(l.data[i]))
                    overtiletranslucent16(refbmp, tile, x2, y2, l.cset[i], c.flip, opacity);
                else
                    overtile16(refbmp, tile, x2, y2, l.cset[i], c.flip);
                    
                //putcombo( b, xx, yy, l.data[i], l.cset[i] );
            }
        }
    }
    
    //putscr
}



void do_bmpdrawscreenr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset, bool isOffScreen)
{
    //sdci[1]=layer
    //sdci[2]=map
    //sdci[3]=screen
    //sdci[4]=x
    //sdci[5]=y
    //sdci[6]=rotation
	//sdci[DRAWCMD_BMP_TARGET] Bitmap Pointer
	
	BITMAP *refbmp = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;

    if ( (sdci[DRAWCMD_BMP_TARGET]-10) != -2 && (sdci[DRAWCMD_BMP_TARGET]-10) != -1 ) yoffset = 0; //Don't crop. 
	
    int32_t map = (sdci[2]/10000)-1; //zscript map indices start at 1.
    int32_t screen = sdci[3]/10000;
    int32_t x = sdci[4]/10000;
    int32_t y = sdci[5]/10000;
    int32_t x1 = x + xoffset;
    int32_t y1 = y + yoffset;
    int32_t rotation = sdci[6]/10000;
    
    uint32_t index = (uint32_t)map_screen_index(map, screen);
    
    if(index >= TheMaps.size())
    {
        al_trace("DrawScreen: invalid map or screen index. \n");
        return;
    }
    
    const mapscr & m = TheMaps[index];
    
    
    BITMAP* b = FFCore.GetScriptBitmap(sdci[DRAWCMD_BMP_TARGET]);
	if ( refbmp == NULL ) return;
    
    if(rotation != 0)
        b = script_drawing_commands.AquireSubBitmap(256, 176);
        
    //draw layer 0
    draw_mapscr(b, m, x1, y1, false);
    
    for(int32_t i(0); i < 6; ++i)
    {
        if(m.layermap[i] == 0) continue;
        
        uint32_t layer_screen_index = (m.layermap[i]-1) * MAPSCRS + m.layerscreen[i];
        
        if(layer_screen_index >= TheMaps.size())
            continue;
            
        bool trans = m.layeropacity[i] == 128;
        
        //draw valid layers
        draw_mapscr(b, TheMaps[ layer_screen_index ], x1, y1, trans);
    }
    
    if(rotation != 0) // rotate
    {
        rotate_sprite(refbmp, b, x1, y1, degrees_to_fixed(rotation));
        script_drawing_commands.ReleaseSubBitmap(b);
    }
}

void do_bmpdrawlayersolidmaskr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset, bool isOffScreen)
{
    //sdci[1]=layer
    //sdci[2]=map
    //sdci[3]=screen
    //sdci[4]=layer
    //sdci[5]=x
    //sdci[6]=y
    //sdci[7]=rotation
    //sdci[8]=bool noclip
	//sdci[9] == opacity
    
    int32_t map = (sdci[2]/10000)-1; //zscript map indices start at 1.
    int32_t screen = sdci[3]/10000;
    int32_t sourceLayer = vbound(sdci[4]/10000, 0, 6);
    int32_t x = sdci[5]/10000;
    int32_t y = sdci[6]/10000;
    int32_t x1 = x + xoffset;
    int32_t y1 = y + yoffset;
    int32_t rotation = sdci[7]/10000;
    byte noclip = (sdci[8]!=0);
    int32_t opacity = sdci[9]/10000;
    
    uint32_t index = (uint32_t)map_screen_index(map, screen);
    const mapscr* m = getmapscreen(map, screen, sourceLayer);
    
    if(!m) //no need to log it.
        return;

	if(index >= TheMaps.size())
	{
		al_trace("DrawLayer: invalid map index \"%i\". Map count is %lu.\n", index, TheMaps.size());
		return;
	}
    
    const mapscr & l = *m;
    
    BITMAP* b = bmp;
    
    if(rotation != 0)
        b = script_drawing_commands.AquireSubBitmap(256, 176);
        
        
    const int32_t maxX = isOffScreen ? 512 : 256;
    const int32_t maxY = isOffScreen ? 512 : 176 + yoffset;
    bool transparent = opacity <= 128;
    
    if(rotation != 0) // rotate
    {
        draw_map_solid(b, l, x1, y1);
        
        rotate_sprite(bmp, b, x1, y1, degrees_to_fixed(rotation));
        script_drawing_commands.ReleaseSubBitmap(b);
    }
    else
    {
		BITMAP* square = create_bitmap_ex(8,16,16);
		BITMAP* subsquare = create_bitmap_ex(8,16,16);
		clear_to_color(subsquare,1);
        for(int32_t i(0); i < 176; ++i)
        {
            const int32_t x2 = ((i&15)<<4) + x1;
            const int32_t y2 = (i&0xF0) + y1;
            
            if(noclip&&(x2 > -16 && x2 < maxX && y2 > -16 && y2 < maxY))   //in clipping rect
            {
                int32_t sol = (combobuf[l.data[i]].walk);
                
                if ( sol & 1 )
				{
					blit(subsquare, square, 0, 0, 0, 0, 8, 8);
				}
				if ( sol & 2 )
				{
					blit(subsquare, square, 0, 0, 0, 8, 8, 8);
				}
				if ( sol & 4 )
				{
					blit(subsquare, square, 0, 0, 8, 0, 8, 8);
				}
				if ( sol &8 )	{
					blit(subsquare, square, 0, 0, 8, 8, 8, 8);
				}
				
				blit(square, b, 0, 0, x2, y2, square->w, square->h);
            }
        }
		destroy_bitmap(square);
		destroy_bitmap(subsquare);
    }
    
    //putscr
}

void do_bmpdrawlayersolidityr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset, bool isOffScreen)
{
    //sdci[1]=layer
    //sdci[2]=map
    //sdci[3]=screen
    //sdci[4]=layer
    //sdci[5]=x
    //sdci[6]=y
    //sdci[7]=rotation
	//[8] noclip
    //sdci[9]=opacity
	
    
    int32_t map = (sdci[2]/10000)-1; //zscript map indices start at 1.
    int32_t screen = sdci[3]/10000;
    int32_t sourceLayer = vbound(sdci[4]/10000, 0, 6);
    int32_t x = sdci[5]/10000;
    int32_t y = sdci[6]/10000;
    int32_t x1 = x + xoffset;
    int32_t y1 = y + yoffset;
    int32_t rotation = sdci[7]/10000;
	byte noclip = (sdci[8]!=0);
    int32_t opacity = sdci[9]/10000;
    
    uint32_t index = (uint32_t)map_screen_index(map, screen);
    const mapscr* m = getmapscreen(map, screen, sourceLayer);
    
    if(!m) //no need to log it.
        return;

	if(index >= TheMaps.size())
	{
		al_trace("DrawLayer: invalid map index \"%i\". Map count is %lu.\n", index, TheMaps.size());
		return;
	}
    
    const mapscr & l = *m;
    
    BITMAP* b = bmp;
    
    if(rotation != 0)
        b = script_drawing_commands.AquireSubBitmap(256, 176);
        
        
    const int32_t maxX = isOffScreen ? 512 : 256;
    const int32_t maxY = isOffScreen ? 512 : 176 + yoffset;
    bool transparent = opacity <= 128;
    
    if(rotation != 0) // rotate
    {
        draw_map_solidity(b, l, x1, y1);
        
        rotate_sprite(bmp, b, x1, y1, degrees_to_fixed(rotation));
        script_drawing_commands.ReleaseSubBitmap(b);
    }
    else
    {
	BITMAP* square = create_bitmap_ex(8,16,16);
        for(int32_t i(0); i < 176; ++i)
        {
            const int32_t x2 = ((i&15)<<4) + x1;
            const int32_t y2 = (i&0xF0) + y1;
            
            if(noclip && (x2 > -16 && x2 < maxX && y2 > -16 && y2 < maxY))   //in clipping rect
            {
                clear_to_color(square,(combobuf[l.data[i]].walk&15));
		blit(square, b, 0, 0, x2, y2, square->w, square->h);
            }
        }
	destroy_bitmap(square);
    }
    
    //putscr
}

void do_bmpdrawlayercflagr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset, bool isOffScreen)
{
    //sdci[1]=layer
    //sdci[2]=map
    //sdci[3]=screen
    //sdci[4]=layer
    //sdci[5]=x
    //sdci[6]=y
    //sdci[7]=rotation
	//[8] noclip
    //sdci[9]=opacity
	
    
    int32_t map = (sdci[2]/10000)-1; //zscript map indices start at 1.
    int32_t screen = sdci[3]/10000;
    int32_t sourceLayer = vbound(sdci[4]/10000, 0, 6);
    int32_t x = sdci[5]/10000;
    int32_t y = sdci[6]/10000;
    int32_t x1 = x + xoffset;
    int32_t y1 = y + yoffset;
    int32_t rotation = sdci[7]/10000;

	byte noclip = (sdci[8]!=0);
    int32_t opacity = sdci[9]/10000;
    
    uint32_t index = (uint32_t)map_screen_index(map, screen);
    const mapscr* m = getmapscreen(map, screen, sourceLayer);
    
    if(!m) //no need to log it.
        return;

	if(index >= TheMaps.size())
	{
		al_trace("DrawLayer: invalid map index \"%i\". Map count is %lu.\n", index, TheMaps.size());
		return;
	}
    
    const mapscr & l = *m;
    
    BITMAP* b = bmp;
    
    if(rotation != 0)
        b = script_drawing_commands.AquireSubBitmap(256, 176);
        
        
    const int32_t maxX = isOffScreen ? 512 : 256;
    const int32_t maxY = isOffScreen ? 512 : 176 + yoffset;
    bool transparent = opacity <= 128;
    
    if(rotation != 0) // rotate
    {
        draw_map_cflag(b, l, x1, y1);
        
        rotate_sprite(bmp, b, x1, y1, degrees_to_fixed(rotation));
        script_drawing_commands.ReleaseSubBitmap(b);
    }
    else
    {
	BITMAP* square = create_bitmap_ex(8,16,16);
        for(int32_t i(0); i < 176; ++i)
        {
            const int32_t x2 = ((i&15)<<4) + x1;
            const int32_t y2 = (i&0xF0) + y1;
            
            if(noclip&&(x2 > -16 && x2 < maxX && y2 > -16 && y2 < maxY))   //in clipping rect
            {
                clear_to_color(square,l.sflag[i]);
		blit(square, b, 0, 0, x2, y2, square->w, square->h);
            }
        }
	destroy_bitmap(square);
    }
    
    //putscr
}

void do_bmpdrawlayerctyper(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset, bool isOffScreen)
{
    //sdci[1]=layer
    //sdci[2]=map
    //sdci[3]=screen
    //sdci[4]=layer
    //sdci[5]=x
    //sdci[6]=y
    //sdci[7]=rotation
	//[8] noclip
    //sdci[9]=opacity
    
    int32_t map = (sdci[2]/10000)-1; //zscript map indices start at 1.
    int32_t screen = sdci[3]/10000;
    int32_t sourceLayer = vbound(sdci[4]/10000, 0, 6);
    int32_t x = sdci[5]/10000;
    int32_t y = sdci[6]/10000;
    int32_t x1 = x + xoffset;
    int32_t y1 = y + yoffset;
    int32_t rotation = sdci[7]/10000;

    byte noclip = (sdci[8]!=0);
    int32_t opacity = sdci[9]/10000;
    uint32_t index = (uint32_t)map_screen_index(map, screen);
    const mapscr* m = getmapscreen(map, screen, sourceLayer);
    
    if(!m) //no need to log it.
        return;

	if(index >= TheMaps.size())
	{
		al_trace("DrawLayer: invalid map index \"%i\". Map count is %lu.\n", index, TheMaps.size());
		return;
	}
    
    const mapscr & l = *m;
    
    BITMAP* b = bmp;
    
    if(rotation != 0)
        b = script_drawing_commands.AquireSubBitmap(256, 176);
        
        
    const int32_t maxX = isOffScreen ? 512 : 256;
    const int32_t maxY = isOffScreen ? 512 : 176 + yoffset;
    bool transparent = opacity <= 128;
    
    if(rotation != 0) // rotate
    {
        draw_map_combotype(b, l, x1, y1);
        
        rotate_sprite(bmp, b, x1, y1, degrees_to_fixed(rotation));
        script_drawing_commands.ReleaseSubBitmap(b);
    }
    else
    {
	BITMAP* square = create_bitmap_ex(8,16,16);
        for(int32_t i(0); i < 176; ++i)
        {
            const int32_t x2 = ((i&15)<<4) + x1;
            const int32_t y2 = (i&0xF0) + y1;
            
            if(noclip&&(x2 > -16 && x2 < maxX && y2 > -16 && y2 < maxY))   //in clipping rect
            {
                clear_to_color(square,(combobuf[l.data[i]].type));
		blit(square, b, 0, 0, x2, y2, square->w, square->h);
            }
        }
	destroy_bitmap(square);
    }
    
    //putscr
}

void do_bmpdrawlayerciflagr(BITMAP *bmp, int32_t *sdci, int32_t xoffset, int32_t yoffset, bool isOffScreen)
{
    //sdci[1]=layer
    //sdci[2]=map
    //sdci[3]=screen
    //sdci[4]=layer
    //sdci[5]=x
    //sdci[6]=y
    //sdci[7]=rotation
	//[8] noclip
    //sdci[9]=opacity
    
    int32_t map = (sdci[2]/10000)-1; //zscript map indices start at 1.
    int32_t screen = sdci[3]/10000;
    int32_t sourceLayer = vbound(sdci[4]/10000, 0, 6);
    int32_t x = sdci[5]/10000;
    int32_t y = sdci[6]/10000;
    int32_t x1 = x + xoffset;
    int32_t y1 = y + yoffset;
    int32_t rotation = sdci[7]/10000;
    byte noclip = (sdci[8]!=0);
    int32_t opacity = sdci[9]/10000;
    
    uint32_t index = (uint32_t)map_screen_index(map, screen);
    const mapscr* m = getmapscreen(map, screen, sourceLayer);
    
    if(!m) //no need to log it.
        return;

	if(index >= TheMaps.size())
	{
		al_trace("DrawLayer: invalid map index \"%i\". Map count is %lu.\n", index, TheMaps.size());
		return;
	}
    
    const mapscr & l = *m;
    
    BITMAP* b = bmp;
    
    if(rotation != 0)
        b = script_drawing_commands.AquireSubBitmap(256, 176);
        
        
    const int32_t maxX = isOffScreen ? 512 : 256;
    const int32_t maxY = isOffScreen ? 512 : 176 + yoffset;
    bool transparent = opacity <= 128;
    
    if(rotation != 0) // rotate
    {
        draw_map_comboiflag(b, l, x1, y1);
        
        rotate_sprite(bmp, b, x1, y1, degrees_to_fixed(rotation));
        script_drawing_commands.ReleaseSubBitmap(b);
    }
    else
    {
	BITMAP* square = create_bitmap_ex(8,16,16);
        for(int32_t i(0); i < 176; ++i)
        {
            const int32_t x2 = ((i&15)<<4) + x1;
            const int32_t y2 = (i&0xF0) + y1;
            
            if(noclip&&(x2 > -16 && x2 < maxX && y2 > -16 && y2 < maxY))   //in clipping rect
            {
                clear_to_color(square,(combobuf[l.data[i]].flag));
		blit(square, b, 0, 0, x2, y2, square->w, square->h);
            }
        }
	destroy_bitmap(square);
    }
    
    //putscr
}



/////////////////////////////////////////////////////////
// do primitives
////////////////////////////////////////////////////////

// Draw commands can vary in terms of the origin/coordinate system to draw as. This
// is controlled via `DrawOrigin`. Previous to `DrawOrigin`, this always drew
// relative to the playing field (except for offscreen bitmaps).
void do_primitives(BITMAP *targetBitmap, int32_t type)
{
	do_primitives(targetBitmap, type, 0, playing_field_offset);
}
void do_primitives(BITMAP *targetBitmap, int32_t type, int32_t xoff, int32_t yoff)
{
	color_map = &trans_table2;
	
	if(type > 7)
		return;
	if(type >= 0 && origin_scr->hidescriptlayers & (1<<type))
		return; //Script draws hidden for this layer
	if(!script_drawing_commands.is_dirty(type))
		return; //No draws to this layer
	//--script_drawing_commands[][] reference--
	//[][0]: type
	//[][1-16]: defined by type
	//...
	//[][DRAWCMD_BMP_TARGET]: bitmap pointer
	//[][DRAWCMD_CURRENT_TARGET]: current render target at time command is queued? unused?

	const int32_t type_mul_10000 = type * 10000;
	const int32_t numDrawCommandsToProcess = script_drawing_commands.Count();
	FFCore.numscriptdraws = numDrawCommandsToProcess;
	
	for (int i = 0; i < numDrawCommandsToProcess; i++)
	{
		auto& command = script_drawing_commands[i];
		int32_t *sdci = &script_drawing_commands[i][0];

		if (sdci[1] != type_mul_10000)
			continue;

		DrawOrigin draw_origin = command.draw_origin;

		// get the correct render target, if set via Screen->SetRenderTarget
		// Note: This is a deprecated feature.
		BITMAP *bmp = zscriptDrawingRenderTarget->GetTargetBitmap(sdci[DRAWCMD_CURRENT_TARGET]);
		bool isTargetOffScreenBmp;

		if(!bmp)
		{
			bmp = targetBitmap;
			isTargetOffScreenBmp = false;
		}
		else
		{
			// Render target was set to a internal bitmap (but not the screen bitmap).
			isTargetOffScreenBmp = true;
			draw_origin = DrawOrigin::Screen;
		}

		int xoffset, yoffset;
		if (auto r = get_draw_origin_offset(draw_origin, command.draw_origin_target, xoff, yoff))
		{
			std::tie(xoffset, yoffset) = *r;
		}
		else
		{
			continue;
		}

		secondary_draw_origin_xoff = 0;
		secondary_draw_origin_yoff = 0;
		if (command.secondary_draw_origin != DrawOrigin::Default)
		{
			if (auto r = get_draw_origin_offset(command.secondary_draw_origin, command.secondary_draw_origin_target, xoff, yoff))
			{
				std::tie(secondary_draw_origin_xoff, secondary_draw_origin_yoff) = *r;
			}
			else
			{
				continue;
			}
		}

		switch(sdci[0])
		{
			case RECTR:
			{
				do_rectr(bmp, sdci, xoffset, yoffset);
			}
			break;
			case FRAMER:
			{
				do_framer(bmp, sdci, xoffset, yoffset);
			}
			break;
			
			
			case CIRCLER:
			{
				do_circler(bmp, sdci, xoffset, yoffset);
			}
			break;
			
			case ARCR:
			{
				do_arcr(bmp, sdci, xoffset, yoffset);
			}
			break;
			
			case ELLIPSER:
			{
				do_ellipser(bmp, sdci, xoffset, yoffset);
			}
			break;
			
			case LINER:
			{
				do_liner(bmp, sdci, xoffset, yoffset);
			}
			break;
			
			case SPLINER:
			{
				do_spliner(bmp, sdci, xoffset, yoffset);
			}
			break;
			
			case PUTPIXELR:
			{
				do_putpixelr(bmp, sdci, xoffset, yoffset);
			}
			break;
			case PIXELARRAYR:
			{
				do_putpixelsr(bmp, i, sdci, xoffset, yoffset);
			}
			break;
			
			case TILEARRAYR:
			{
				do_fasttilesr(bmp, i, sdci, xoffset, yoffset);
			}
			break;
			
			case LINESARRAY:
			{
				do_linesr(bmp, i, sdci, xoffset, yoffset);
			}
			break;
			
			case COMBOARRAYR:
			{
				do_fastcombosr(bmp, i, sdci, xoffset, yoffset);
			}
			break;
			
			
			
			case DRAWTILER:
			{
				do_drawtiler(bmp, sdci, xoffset, yoffset);
			}
			break;
			
			case DRAWTILECLOAKEDR:
			{
				do_drawtilecloakedr(bmp, sdci, xoffset, yoffset);
			}
			break;
			
			case DRAWCOMBOR:
			{
				do_drawcombor(bmp, sdci, xoffset, yoffset);
			}
			break;
			
			case DRAWCOMBOCLOAKEDR:
			{
				do_drawcombocloakedr(bmp, sdci, xoffset, yoffset);
			}
			break;
			
			case FASTTILER:
			{
				do_fasttiler(bmp, sdci, xoffset, yoffset);
			}
			break;
			
			case FASTCOMBOR:
			{
				do_fastcombor(bmp, sdci, xoffset, yoffset);
			}
			break;
			
			case DRAWCHARR:
			{
				do_drawcharr(bmp, sdci, xoffset, yoffset);
			}
			break;
			
			case DRAWINTR:
			{
				do_drawintr(bmp, sdci, xoffset, yoffset);
			}
			break;
			
			case DRAWSTRINGR:
			{
				do_drawstringr(bmp, i, sdci, xoffset, yoffset);
			}
			break;
			
			case DRAWSTRINGR2:
			{
				do_drawstringr2(bmp, i, sdci, xoffset, yoffset);
			}
			break;
			
			case QUADR:
			{
				do_drawquadr(bmp, sdci, xoffset, yoffset);
			}
			break;
			
			case QUAD3DR:
			{
				do_drawquad3dr(bmp, i, sdci, xoffset, yoffset);
			}
			break;
			
			case TRIANGLER:
			{
				do_drawtriangler(bmp, sdci, xoffset, yoffset);
			}
			break;
			
			case TRIANGLE3DR:
			{
				do_drawtriangle3dr(bmp, i, sdci, xoffset, yoffset);
			}
			break;
			
			case POLYGONR:
			{
				do_polygonr(bmp, i, sdci, xoffset, yoffset);
			}
			break;
			
			
			case BITMAPR:
			{
				do_drawbitmapr(bmp, sdci, xoffset, yoffset);
			}
			break;
			
			case BITMAPEXR:
			{
				do_drawbitmapexr(bmp, sdci, xoffset, yoffset);
			}
			break;
			
			case DRAWLAYERR:
			{
				do_drawlayerr(bmp, sdci, xoffset, yoffset, isTargetOffScreenBmp);
			}
			break;
			
			case DRAWSCREENR:
			{
				do_drawscreenr(bmp, sdci, xoffset, yoffset, isTargetOffScreenBmp);
			}
			break;
			
			case BMPRECTR: bmp_do_rectr(bmp, sdci, xoffset, yoffset); break;
			case BMPFRAMER: bmp_do_framer(bmp, sdci, xoffset, yoffset); break;
			case BMPCIRCLER: bmp_do_circler(bmp, sdci, xoffset, yoffset); break;
			case BMPARCR: bmp_do_arcr(bmp, sdci, xoffset, yoffset); break;
			case BMPELLIPSER: bmp_do_ellipser(bmp, sdci, xoffset, yoffset); break;
			case BMPLINER: bmp_do_liner(bmp, sdci, xoffset, yoffset); break;
			case BMPSPLINER: bmp_do_spliner(bmp, sdci, xoffset, yoffset); break;
			case BMPPUTPIXELR: bmp_do_putpixelr(bmp, sdci, xoffset, yoffset); break;
			case BMPDRAWTILER: bmp_do_drawtiler(bmp, sdci, xoffset, yoffset); break;
			case BMPDRAWTILECLOAKEDR: bmp_do_drawtilecloakedr(bmp, sdci, xoffset, yoffset); break;
			case BMPDRAWCOMBOR: bmp_do_drawcombor(bmp, sdci, xoffset, yoffset); break;
			case BMPDRAWCOMBOCLOAKEDR: bmp_do_drawcombocloakedr(bmp, sdci, xoffset, yoffset); break;
			case BMPFASTTILER: bmp_do_fasttiler(bmp, sdci, xoffset, yoffset); break;
			case BMPFASTCOMBOR: bmp_do_fastcombor(bmp, sdci, xoffset, yoffset); break;
			case BMPDRAWCHARR: bmp_do_drawcharr(bmp, sdci, xoffset, yoffset); break;
			case BMPDRAWINTR: bmp_do_drawintr(bmp, sdci, xoffset, yoffset); break;
			case BMPDRAWSTRINGR: bmp_do_drawstringr(bmp, i, sdci, xoffset, yoffset); break;
			case BMPDRAWSTRINGR2: bmp_do_drawstringr2(bmp, i, sdci, xoffset, yoffset); break;
			case BMPQUADR: bmp_do_drawquadr(bmp, sdci, xoffset, yoffset); break;
			case BMPQUAD3DR: bmp_do_drawquad3dr(bmp, i, sdci, xoffset, yoffset); break;
				
			case BITMAPGETPIXEL: bmp_do_getpixelr(bmp, sdci, xoffset, yoffset); break;
			case BMPTRIANGLER: bmp_do_drawtriangler(bmp, sdci, xoffset, yoffset); break;
			case BMPTRIANGLE3DR: bmp_do_drawtriangle3dr(bmp, i, sdci, xoffset, yoffset); break;
			case BMPPOLYGONR: bmp_do_polygonr(bmp, i, sdci, xoffset, yoffset); break;
			case BMPDRAWLAYERR: do_bmpdrawlayerr(bmp, sdci, xoffset, yoffset, isTargetOffScreenBmp); break;
			case BMPDRAWSCREENR: do_bmpdrawscreenr(bmp, sdci, xoffset, yoffset, isTargetOffScreenBmp); break;
			case BMPDRAWSCREENSOLIDR: do_bmpdrawscreen_solidmaskr(bmp, sdci, xoffset, yoffset, isTargetOffScreenBmp); break;
			case BMPDRAWSCREENSOLID2R: do_bmpdrawscreen_solidr(bmp, sdci, xoffset, yoffset, isTargetOffScreenBmp); break;
			case BMPDRAWSCREENCOMBOFR: do_bmpdrawscreen_cflagr(bmp, sdci, xoffset, yoffset, isTargetOffScreenBmp); break;
			case BMPDRAWSCREENCOMBOIR: do_bmpdrawscreen_ciflagr(bmp, sdci, xoffset, yoffset, isTargetOffScreenBmp); break;
			case BMPDRAWSCREENCOMBOTR: do_bmpdrawscreen_ctyper(bmp, sdci, xoffset, yoffset, isTargetOffScreenBmp); break;
			case BMPBLIT: bmp_do_drawbitmapexr(bmp, sdci, xoffset, yoffset); break;
			case BMPMODE7: bmp_do_mode7r(bmp, sdci, xoffset, yoffset); break;
			case BMPBLITTO: bmp_do_blittor(bmp, sdci, xoffset, yoffset); break;
			case TILEBLIT: do_tileblit(bmp, sdci, xoffset, yoffset, false, "TileBlit()"); break;
			case COMBOBLIT: do_comboblit(bmp, sdci, xoffset, yoffset, false); break;
			case BMPTILEBLIT: do_tileblit(bmp, sdci, xoffset, yoffset, true, "TileBlit()"); break;
			case BMPCOMBOBLIT: do_comboblit(bmp, sdci, xoffset, yoffset, true); break;
			case READBITMAP: bmp_do_readr(bmp, i, sdci, xoffset, yoffset); break;
			case WRITEBITMAP: bmp_do_writer(bmp, i, sdci, xoffset, yoffset); break;
			case CLEARBITMAP: bmp_do_clearr(bmp, sdci, xoffset, yoffset); break;
			case BITMAPCLEARTOCOLOR: bmp_do_clearcolorr(bmp, sdci, xoffset, yoffset); break;
			case REGENERATEBITMAP: bmp_do_regenr(bmp, sdci, xoffset, yoffset); break;
			
			case BMPDRAWLAYERSOLIDR: do_bmpdrawlayersolidmaskr(bmp, sdci, xoffset, yoffset, isTargetOffScreenBmp); break;
			case BMPDRAWLAYERCFLAGR: do_bmpdrawlayercflagr(bmp, sdci, xoffset, yoffset, isTargetOffScreenBmp); break;
			case BMPDRAWLAYERCTYPER: do_bmpdrawlayerctyper(bmp, sdci, xoffset, yoffset, isTargetOffScreenBmp); break;
			case BMPDRAWLAYERCIFLAGR: do_bmpdrawlayerciflagr(bmp, sdci, xoffset, yoffset, isTargetOffScreenBmp); break;
			case BMPDRAWLAYERSOLIDITYR: do_bmpdrawlayersolidityr(bmp, sdci, xoffset, yoffset, isTargetOffScreenBmp); break;
			case BMPWRITETILE: do_bmpwritetile(bmp, sdci, xoffset, yoffset); break;
			case BMPDITHER: do_bmpdither(bmp, sdci, xoffset, yoffset); break;
			case BMPREPLCOLOR: do_bmpreplcol(bmp, sdci, xoffset, yoffset); break;
			case BMPSHIFTCOLOR: do_bmpshiftcol(bmp, sdci, xoffset, yoffset); break;
			case BMPMASKDRAW: do_bmpmaskdraw(bmp, sdci, xoffset, yoffset); break;
			case BMPMASKBLIT: do_bmpmaskblit(bmp, sdci, xoffset, yoffset); break;

			// The following are special cases, in that the target bitmap is fixed (darkscr_bmp).

			case DRAWLIGHT_CONE:
			{
				int32_t cx = sdci[2]/10000 + xoffset;
				int32_t cy = sdci[3]/10000 + yoffset;
				int32_t dir = sdci[4]/10000;
				int32_t length = sdci[5];
				int32_t transp_rad = sdci[6];
				int32_t dith_rad = sdci[7];
				int32_t dith_type = sdci[8];
				int32_t dith_arg = sdci[9];

				if(length >= 0) length /= 10000;
				else length = game->get_light_rad()*2;
				if(!length) break;
				if(dir < 0) break;
				else dir = NORMAL_DIR(dir);
				if(transp_rad >= 0) transp_rad /= 10000;
				if(dith_rad >= 0) dith_rad /= 10000;
				if(dith_type >= 0) dith_type /= 10000;
				if(dith_arg >= 0) dith_arg /= 10000;

				// Undo the inherit offset applied within the function. xoffset/yoffset handles for us here.
				cx += viewport.x;
				cy += viewport.y;

				doDarkroomCone(cx,cy,length,dir,darkscr_bmp,nullptr,dith_rad,transp_rad,dith_type,dith_arg);
			}
			break;

			case DRAWLIGHT_CIRCLE:
			case DRAWLIGHT_SQUARE:
			{
				int32_t cx = sdci[2]/10000 + xoffset;
				int32_t cy = sdci[3]/10000 + yoffset;
				int32_t radius = sdci[4];
				int32_t transp_rad = sdci[5];
				int32_t dith_rad = sdci[6];
				int32_t dith_type = sdci[7];
				int32_t dith_arg = sdci[8];

				if(radius >= 0) radius /= 10000;
				else radius = game->get_light_rad();
				if(!radius) break;
				if(transp_rad >= 0) transp_rad /= 10000;
				if(dith_rad >= 0) dith_rad /= 10000;
				if(dith_type >= 0) dith_type /= 10000;
				if(dith_arg >= 0) dith_arg /= 10000;

				// Undo the inherit offset applied within the function. xoffset/yoffset handles for us here.
				cx += viewport.x;
				cy += viewport.y;

				if (sdci[0] == DRAWLIGHT_CIRCLE)
					doDarkroomCircle(cx,cy,radius,darkscr_bmp,nullptr,dith_rad,transp_rad,dith_type,dith_arg);
				else
					doDarkroomSquare(cx,cy,radius,darkscr_bmp,nullptr,dith_rad,transp_rad,dith_type,dith_arg);
			}
			break;
		}
	}
	
	
	color_map=&trans_table;
}

void CScriptDrawingCommands::Clear()
{
	scb.update();
	dirty_layers.clear();
	if(commands.empty())
		return;
	
	//only clear what was used.
	memset((void*)&commands[0], 0, count * sizeof(CScriptDrawingCommandVars));
	count = 0;
	
	draw_container.Clear();
}
CScriptDrawingCommands* CScriptDrawingCommands::pop_commands()
{
	CScriptDrawingCommands* ret = new CScriptDrawingCommands();
	if(commands.empty())
		return ret;
	ret->push_commands(this, false);
	
	memset((void*)&commands[0], 0, count * sizeof(CScriptDrawingCommandVars));
	count = 0;
	
	draw_container.Clear();
	return ret;
}
void CScriptDrawingCommands::push_commands(CScriptDrawingCommands* other, bool del)
{
	commands.insert(commands.end(), other->commands.begin(), other->commands.end());
	count += other->count;
	if(del) delete other;
}

void do_script_draws(BITMAP *targetBitmap, mapscr* scr, int32_t xoff, int32_t yoff, bool hideLayer7)
{
	if(XOR(scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG)) do_primitives(targetBitmap, 2, xoff, yoff);
	if(XOR(scr->flags7&fLAYER3BG, DMaps[cur_dmap].flags&dmfLAYER3BG)) do_primitives(targetBitmap, 3, xoff, yoff);
	do_primitives(targetBitmap, 0, xoff, yoff);
	do_primitives(targetBitmap, 1, xoff, yoff);
	if(!XOR(scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG)) do_primitives(targetBitmap, 2, xoff, yoff);
	if(!XOR(scr->flags7&fLAYER3BG, DMaps[cur_dmap].flags&dmfLAYER3BG)) do_primitives(targetBitmap, 3, xoff, yoff);
	do_primitives(targetBitmap, 4, xoff, yoff);
	do_primitives(targetBitmap, 5, xoff, yoff);
	do_primitives(targetBitmap, 6, xoff, yoff);
	if(!hideLayer7) do_primitives(targetBitmap, 7, xoff, yoff);
}
