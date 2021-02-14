// This program is free software; you can redistribute it and/or modify it under the terms of the
// modified version 3 of the GNU General Public License. See License.txt for details.

//! ritate_sprite_trans doesn't seem to be supported by or allegro header !?

//glibc 2.28 and later require this: -Z
#ifdef __GNUG__
	#define ALLEGRO_NO_FIX_ALIASES
#endif

#include "precompiled.h" //always first

#include <allegro.h>
#include "script_drawing.h"
#include "rendertarget.h"
#include "maps.h"
#include "tiles.h"
#include "zelda.h"
#include "ffscript.h"
#include <stdio.h>

#define DegtoFix(d)     ((d)*0.7111111111111)
#define RadtoFix(d)     ((d)*40.743665431525)

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




FONT *get_zc_font(int index)
{
    //return getfont(index);
    switch(index)
    {
    default:
        return zfont;
        
    case font_z3font:
        return z3font;
        
    case font_z3smallfont:
        return z3smallfont;
        
    case font_deffont:
        return deffont;
        
    case font_lfont:
        return lfont;
        
    case font_lfont_l:
        return lfont_l;
        
    case font_pfont:
        return pfont;
        
    case font_mfont:
        return mfont;
        
    case font_ztfont:
        return ztfont;
        
    case font_sfont:
        return sfont;
        
    case font_sfont2:
        return sfont2;
        
    case font_spfont:
        return spfont;
        
    case font_ssfont1:
        return ssfont1;
        
    case font_ssfont2:
        return ssfont2;
        
    case font_ssfont3:
        return ssfont3;
        
    case font_ssfont4:
        return ssfont4;
        
    case font_gblafont:
        return gblafont;
        
    case font_goronfont:
        return goronfont;
        
    case font_zoranfont:
        return zoranfont;
        
    case font_hylian1font:
        return hylian1font;
        
    case font_hylian2font:
        return hylian2font;
        
    case font_hylian3font:
        return hylian3font;
        
    case font_hylian4font:
        return hylian4font;
        
    case font_gboraclefont:
        return gboraclefont;
        
    case font_gboraclepfont:
        return gboraclepfont;
        
    case font_dsphantomfont:
        return dsphantomfont;
        
    case font_dsphantompfont:
        return dsphantompfont;
    }
}


class TileHelper
{
public:

    static void OldPutTile(BITMAP* _Dest, int tile, int x, int y, int w, int h, int color, int flip, byte skiprows=0)
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
            for(int j=0; j<h; j++)
                for(int k=w-1; k>=0; k--)
                    oldputtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+j*16, color, flip);
                    
            break;
            
        case 2:
            for(int j=h-1; j>=0; j--)
                for(int k=0; k<w; k++)
                    oldputtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+((h-1)-j)*16, color, flip);
                    
            break;
            
        case 3:
            for(int j=h-1; j>=0; j--)
                for(int k=w-1; k>=0; k--)
                    oldputtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+((h-1)-j)*16, color, flip);
                    
            break;
            
        case 0:
        default:
            for(int j=0; j<h; j++)
                for(int k=0; k<w; k++)
                    oldputtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+j*16, color, flip);
                    
            break;
        }
    }
    
    static void OverTile(BITMAP* _Dest, int tile, int x, int y, int w, int h, int color, int flip, byte skiprows=0)
    {
        if(skiprows>0 && tile%TILES_PER_ROW+w>=TILES_PER_ROW)
        {
            byte w2=(tile+w)%TILES_PER_ROW;
            OverTile(_Dest, tile, x, y, w-w2, h, color, flip);
            OverTile(_Dest, tile+(w-w2)+(skiprows*TILES_PER_ROW), x+16*(w-w2), y, w2, h, color, flip);
            return;
        }
        
        switch(flip)
        {
        case 1:
            for(int j=0; j<h; j++)
                for(int k=w-1; k>=0; k--)
                    overtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+j*16, color, flip);
                    
            break;
            
        case 2:
            for(int j=h-1; j>=0; j--)
                for(int k=0; k<w; k++)
                    overtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+((h-1)-j)*16, color, flip);
                    
            break;
            
        case 3:
            for(int j=h-1; j>=0; j--)
                for(int k=w-1; k>=0; k--)
                    overtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+((h-1)-j)*16, color, flip);
                    
            break;
            
        default:
            for(int j=0; j<h; j++)
                for(int k=0; k<w; k++)
                    overtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+j*16, color, flip);
                    
            break;
        }
    }
    
    static void OverTileTranslucent(BITMAP* _Dest, int tile, int x, int y, int w, int h, int color, int flip, int opacity, byte skiprows=0)
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
            for(int j=0; j<h; j++)
                for(int k=w-1; k>=0; k--)
                    overtiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+j*16, color, flip, opacity);
                    
            break;
            
        case 2:
            for(int j=h-1; j>=0; j--)
                for(int k=0; k<w; k++)
                    overtiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+((h-1)-j)*16, color, flip, opacity);
                    
            break;
            
        case 3:
            for(int j=h-1; j>=0; j--)
                for(int k=w-1; k>=0; k--)
                    overtiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+((h-1)-j)*16, color, flip, opacity);
                    
            break;
            
        default:
            for(int j=0; j<h; j++)
                for(int k=0; k<w; k++)
                    overtiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+j*16, color, flip, opacity);
                    
            break;
        }
    }
    
    static void PutTileTranslucent(BITMAP* _Dest, int tile, int x, int y, int w, int h, int color, int flip, int opacity, byte skiprows=0)
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
            for(int j=0; j<h; j++)
                for(int k=w-1; k>=0; k--)
                    puttiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+j*16, color, flip, opacity);
                    
            break;
            
        case 2:
            for(int j=h-1; j>=0; j--)
                for(int k=0; k<w; k++)
                    puttiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+((h-1)-j)*16, color, flip, opacity);
                    
            break;
            
        case 3:
            for(int j=h-1; j>=0; j--)
                for(int k=w-1; k>=0; k--)
                    puttiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+((h-1)-j)*16, color, flip, opacity);
                    
            break;
            
        default:
            for(int j=0; j<h; j++)
                for(int k=0; k<w; k++)
                    puttiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+j*16, color, flip, opacity);
                    
            break;
        }
    }
};




void do_rectr(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
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
    
    int x1=sdci[2]/10000;
    int y1=sdci[3]/10000;
    int x2=sdci[4]/10000;
    int y2=sdci[5]/10000;
    
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
        int w=x2-x1+1;
        int h=y2-y1+1;
        int w2=(w*sdci[7])/10000;
        int h2=(h*sdci[7])/10000;
        x1=x1-((w2-w)/2);
        x2=x2+((w2-w)/2);
        y1=y1-((h2-h)/2);
        y2=y2+((h2-h)/2);
    }
    
    int color=sdci[6]/10000;
    
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
        int xy[16];
        int rx=sdci[8]/10000;
        int ry=sdci[9]/10000;
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



void do_circler(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
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
    
    int x1=sdci[2]/10000;
    int y1=sdci[3]/10000;
    qword r=sdci[4];
    
    if(sdci[6] != 10000)
    {
        r*=sdci[6];
        r/=10000;
    }
    
    r/=10000;
    int color=sdci[5]/10000;
    
    if(sdci[11]/10000<=127) //translucent
    {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    }
    
    if(sdci[9]!=0&&(sdci[2]!=sdci[7]||sdci[3]!=sdci[8])) //rotation
    {
        int xy[2];
        int rx=sdci[7]/10000;
        int ry=sdci[8]/10000;
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


void do_arcr(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
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
    
    int cx=sdci[2]/10000;
    int cy=sdci[3]/10000;
    qword r=sdci[4];
    
    if(sdci[8] != 10000)
    {
        r*=sdci[8];
        r/=10000;
    }
    
    r/=10000;
    
    int color=sdci[7]/10000;
    
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
        int rx=sdci[9]/10000;
        int ry=sdci[10]/10000;
        
        cx=rx + fixtoi((fixcos(ra) * (cx - rx) - fixsin(ra) * (cy - ry)));     //x1
        cy=ry + fixtoi((fixsin(ra) * (cx - rx) + fixcos(ra) * (cy - ry)));     //y1
        ea-=ra;
        sa-=ra;
    }
    
    int fx=cx+fixtoi(fixcos(-(ea+sa)/2)*r/2);
    int fy=cy+fixtoi(fixsin(-(ea+sa)/2)*r/2);
    
    if(sdci[12]) //closed
    {
        if(sdci[13]) //filled
        {
            clear_bitmap(prim_bmp);
            arc(prim_bmp, cx+xoffset, cy+yoffset, sa, ea, int(r), color);
            line(prim_bmp, cx+xoffset, cy+yoffset, cx+xoffset+fixtoi(fixcos(-sa)*r), cy+yoffset+fixtoi(fixsin(-sa)*r), color);
            line(prim_bmp, cx+xoffset, cy+yoffset, cx+xoffset+fixtoi(fixcos(-ea)*r), cy+yoffset+fixtoi(fixsin(-ea)*r), color);
            floodfill(prim_bmp, zc_max(0,fx)+xoffset, zc_max(0,fy)+yoffset, color);
            
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
            arc(bmp, cx+xoffset, cy+yoffset, sa, ea, int(r), color);
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
        
        arc(bmp, cx+xoffset, cy+yoffset, sa, ea, int(r), color);
        drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
    }
}


void do_ellipser(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
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
    
    int x1=sdci[2]/10000;
    int y1=sdci[3]/10000;
    int radx=sdci[4]/10000;
    radx*=sdci[7]/10000;
    int rady=sdci[5]/10000;
    rady*=sdci[7]/10000;
    int color=sdci[6]/10000;
    float rotation = sdci[10]/10000;
    
    int rx=sdci[8]/10000;
    int ry=sdci[9]/10000;
    fixed ra1=itofix(sdci[10]%10000)/10000;
    fixed ra2=itofix(sdci[10]/10000);
    fixed ra=ra1+ra2;
    ra = (ra/360)*256;
    
    int xy[2];
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
        int x;
        int y;
        
        // This is very slow, so check the smallest possible square
        int endx=zc_min(bmp->w-1, x1+zc_max(radx, rady));
        int endy=zc_min(bmp->h-1, y1+zc_max(radx, rady));
        
        for(int y=zc_max(0, y1-zc_max(radx, rady)); y<=endy; y++)
            for(int x=zc_max(0, x1-zc_max(radx, rady)); x<=endx; x++)
                if(getpixel(bmp, x, y)==255)
                    putpixel(bmp, x, y, 0);
    }
    
    script_drawing_commands.ReleaseSubBitmap(bitty);
}


void do_liner(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
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
    
    int x1=sdci[2]/10000;
    int y1=sdci[3]/10000;
    int x2=sdci[4]/10000;
    int y2=sdci[5]/10000;
    
    if(sdci[7] != 10000)
    {
        int w=x2-x1+1;
        int h=y2-y1+1;
        int w2=int(w*((double)sdci[7]/10000.0));
        int h2=int(h*((double)sdci[7]/10000.0));
        x1=x1-((w2-w)/2);
        x2=x2+((w2-w)/2);
        y1=y1-((h2-h)/2);
        y2=y2+((h2-h)/2);
    }
    
    int color=sdci[6]/10000;
    
    if(sdci[11]/10000<=127) //translucent
    {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    }
    
    if(sdci[10]!=0) //rotation
    {
        int xy[4];
        int rx=sdci[8]/10000;
        int ry=sdci[9]/10000;
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


void do_spliner(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
{
    /* layer, x1, y1, x2, y2, x3, y3, x4, y4, color, opacity */
    
    int points[8] = {    xoffset + (sdci[2]/10000), yoffset + (sdci[3]/10000),
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


void do_putpixelr(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
{
    //sdci[1]=layer
    //sdci[2]=x
    //sdci[3]=y
    //sdci[4]=color
    //sdci[5]=rotation anchor x
    //sdci[6]=rotation anchor y
    //sdci[7]=rotation angle
    //sdci[8]=opacity
    int x1=sdci[2]/10000;
    int y1=sdci[3]/10000;
    int color=sdci[4]/10000;
    
    if(sdci[8]/10000<=127) //translucent
    {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    }
    
    if(sdci[7]!=0) //rotation
    {
        int xy[2];
        int rx=sdci[5]/10000;
        int ry=sdci[6]/10000;
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


void do_drawtiler(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
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
    
    int w = sdci[5]/10000;
    int h = sdci[6]/10000;
    
    if(w < 1 || h < 1 || h > 20 || w > 20)
    {
        return;
    }
    
    int xscale=sdci[8]/10000;
    int yscale=sdci[9]/10000;
    int rx = sdci[10]/10000;
    int ry = sdci[11]/10000;
    float rotation=sdci[12]/10000;
    int flip=(sdci[13]/10000)&3;
    bool transparency=sdci[14]!=0;
    int opacity=sdci[15]/10000;
    int color=sdci[7]/10000;
    
    int x1=sdci[2]/10000;
    int y1=sdci[3]/10000;
    
    //don't scale if it's not safe to do so
    bool canscale = true;
    
    if(xscale==0||yscale==0)
    {
        return;
    }
    
    if(xscale<0||yscale<0)
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
                int xy[2];
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


void do_drawcombor(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
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
    
    int w = sdci[5]/10000;
    int h = sdci[6]/10000;
    
    if(w<1||h<1||h>20||w>20)
    {
        return;
    }
    
    int xscale=sdci[8]/10000;
    int yscale=sdci[9]/10000;
    int rx = sdci[10]/10000; //these work now
    int ry = sdci[11]/10000; //these work now
    float rotation=sdci[12]/10000;
    
    bool transparency=sdci[15]!=0;
    int opacity=sdci[16]/10000;
    int color=sdci[7]/10000;
    int x1=sdci[2]/10000;
    int y1=sdci[3]/10000;
    
    const newcombo & c = combobuf[(sdci[4]/10000)];
    int tiletodraw = combo_tile(c, x1, y1);
    int flip = ((sdci[14]/10000) & 3) ^ c.flip;
    int skiprows=combobuf[(sdci[4]/10000)].skipanimy;
    
    
    //don't scale if it's not safe to do so
    bool canscale = true;
    
    if(xscale==0||yscale==0)
    {
        return;
    }
    
    if(xscale<0||yscale<0)
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
                int xy[2];
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


void do_fasttiler(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
{
    /* layer, x, y, tile, color opacity */
    
    int opacity = sdci[6]/10000;
    
    if(opacity < 128)
        overtiletranslucent16(bmp, sdci[4]/10000, xoffset+(sdci[2]/10000), yoffset+(sdci[3]/10000), sdci[5]/10000, 0, opacity);
    else
        overtile16(bmp, sdci[4]/10000, xoffset+(sdci[2]/10000), yoffset+(sdci[3]/10000), sdci[5]/10000, 0);
}



void do_fastcombor(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
{
    /* layer, x, y, tile, color opacity */
    
    int opacity = sdci[6] / 10000;
    int x1 = sdci[2] / 10000;
    int y1 = sdci[3] / 10000;
    int index = sdci[4]/10000;
    
    //if( index >= MAXCOMBOS ) return; //bleh.
	/*
    const newcombo & c = combobuf[index];
    
    if(opacity < 128)
        overtiletranslucent16(bmp, combo_tile(c, x1, y1), xoffset+x1, yoffset+y1, sdci[5]/10000, (int)c.flip, opacity);
    else
        overtile16(bmp, combo_tile(c, x1, y1), xoffset+x1, yoffset+y1, sdci[5]/10000, (int)c.flip);
	*/
	
	if(opacity < 128)
	{
		//void overcomboblocktranslucent(BITMAP *dest, int x, int y, int cmbdat, int cset, int w, int h, int opacity)
		overcomboblocktranslucent(bmp, xoffset+x1, yoffset+y1, sdci[4]/10000, sdci[5]/10000, 1, 1, 128);

	}
	else
	{
		//overcomboblock(BITMAP *dest, int x, int y, int cmbdat, int cset, int w, int h)
		overcomboblock(bmp, xoffset+x1, yoffset+y1, sdci[4]/10000, sdci[5]/10000, 1, 1);
	}
}




void do_drawcharr(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
{
	//broken 2.50.2 and earlier drawcharacter()
	if ( get_bit(extra_rules, er_BROKENCHARINTDRAWING) )
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
		    
		    int x=sdci[2]/10000;
		    int y=sdci[3]/10000;
		    int font_index=sdci[4]/10000;
		    int color=sdci[5]/10000;
		    int bg_color=sdci[6]/10000; //-1 = transparent
		    int w=sdci[7]/10000;
		    int h=sdci[8]/10000;
		    char glyph=char(sdci[9]/10000);
		    int opacity=sdci[10]/10000;
		    
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
		    
		    int x=sdci[2]/10000;
		    int y=sdci[3]/10000;
		    int font_index=sdci[4]/10000;
		    int color=sdci[5]/10000;
		    int bg_color=sdci[6]/10000; //-1 = transparent
		    int w=sdci[7]/10000;
		    int h=sdci[8]/10000;
		    char glyph=char(sdci[9]/10000);
		    int opacity=sdci[10]/10000;
		    
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


void do_drawintr(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
{
	//broken 2.50.2 and earlier drawinteger()
	if ( get_bit(extra_rules, er_BROKENCHARINTDRAWING) )
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
	    
	    int x=sdci[2]/10000;
	    int y=sdci[3]/10000;
	    int font_index=sdci[4]/10000;
	    int color=sdci[5]/10000;
	    int bg_color=sdci[6]/10000; //-1 = transparent
	    int w=sdci[7]/10000;
	    int h=sdci[8]/10000;
	    //float number=static_cast<float>(sdci[9])/10000.0f;
	    int decplace=sdci[10]/10000;
	    int opacity=sdci[11]/10000;
	    
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
	    
	    int x=sdci[2]/10000;
	    int y=sdci[3]/10000;
	    int font_index=sdci[4]/10000;
	    int color=sdci[5]/10000;
	    int bg_color=sdci[6]/10000; //-1 = transparent
	    int w=sdci[7]/10000;
	    int h=sdci[8]/10000;
	    //float number=static_cast<float>(sdci[9])/10000.0f;
		//int numberint = sdci[9]/10000;
	    int decplace=sdci[10]/10000;
	    int opacity=sdci[11]/10000;
	    
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

void do_drawstringr(BITMAP *bmp, int i, int *sdci, int xoffset, int yoffset)
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
    
    int x=sdci[2]/10000;
    int y=sdci[3]/10000;
    FONT* font=get_zc_font(sdci[4]/10000);
    int color=sdci[5]/10000;
    int bg_color=sdci[6]/10000; //-1 = transparent
    int format_type=sdci[7]/10000;
    int opacity=sdci[9]/10000;
    //sdci[8] not needed :)
    
    //safe check
    if(bg_color < -1) bg_color = -1;
    
    if(opacity < 128)
    {
        int width=zc_min(text_length(font, str->c_str()), 512);
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


void do_drawquadr(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
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
    
    int x1 = sdci[2]/10000;
    int y1 = sdci[3]/10000;
    int x2 = sdci[4]/10000;
    int y2 = sdci[5]/10000;
    int x3 = sdci[6]/10000;
    int y3 = sdci[7]/10000;
    int x4 = sdci[8]/10000;
    int y4 = sdci[9]/10000;
    int w = sdci[10]/10000;
    int h = sdci[11]/10000;
    int color = sdci[12]/10000;
    int flip=(sdci[13]/10000)&3;
    int tile = sdci[14]/10000;
    int polytype = sdci[15]/10000;
    
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
    
    int tex_width = w*16;
    int tex_height = h*16;
    
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
    
    int col[4];
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
        const newcombo & c = combobuf[ vbound(abs(tile), 0, 0xffff) ];
        const int tiletodraw = combo_tile(c, x1, y1);
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


void do_drawtriangler(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
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
    
    int x1 = sdci[2]/10000;
    int y1 = sdci[3]/10000;
    int x2 = sdci[4]/10000;
    int y2 = sdci[5]/10000;
    int x3 = sdci[6]/10000;
    int y3 = sdci[7]/10000;
    int w = sdci[8]/10000;
    int h = sdci[9]/10000;
    int color = sdci[10]/10000;
    int flip=(sdci[11]/10000)&3;
    int tile = sdci[12]/10000;
    int polytype = sdci[13]/10000;
    
    polytype = vbound(polytype, 0, 14);
    
    if(((w-1) & w) != 0 || ((h-1) & h) != 0) return;   //non power of two error
    
    int tex_width = w*16;
    int tex_height = h*16;
    
    bool mustDestroyBmp = false;
    BITMAP *tex = script_drawing_commands.GetSmallTextureBitmap(w,h);
    
    if(!tex)
    {
        mustDestroyBmp = true;
        tex = create_bitmap_ex(8, tex_width, tex_height);
        clear_bitmap(tex);
    }
    
    int col[3];
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
        const newcombo & c = combobuf[ vbound(abs(tile), 0, 0xffff) ];
        const int tiletodraw = combo_tile(c, x1, y1);
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


void do_drawbitmapr(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
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

	int bitmapIndex = sdci[2]/10000;
	int sx = sdci[3]/10000;
	int sy = sdci[4]/10000;
	int sw = sdci[5]/10000;
	int sh = sdci[6]/10000;
	int dx = sdci[7]/10000;
	int dy = sdci[8]/10000;
	int dw = sdci[9]/10000;
	int dh = sdci[10]/10000;
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
			
			
				//}
				//else { 
					masked_stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					//rotate_sprite(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
					//
			
				//}
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
				//}
				//else {
					stretch_blit(sourceBitmap, subBmp, sx, sy, sw, sh, 0, 0, dw, dh);
					rotate_sprite(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
				//}
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
				//}
			//else {
				masked_blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
				rotate_sprite(bmp, subBmp, dx, dy, degrees_to_fixed(rot));  
			//}
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
				//}
				//else {
					blit(sourceBitmap, subBmp, sx, sy, 0, 0, dw, dh);
					rotate_sprite(bmp, subBmp, dx, dy, degrees_to_fixed(rot));
				//}
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


void do_drawquad3dr(BITMAP *bmp, int i, int *sdci, int xoffset, int yoffset)
{
    //sdci[1]=layer
    //sdci[2]=pos[12]
    //sdci[3]=uv[8]
    //sdci[4]=color[4]
    //sdci[5]=size[2]
    //sdci[6]=flip
    //sdci[7]=tile/combo
    //sdci[8]=polytype
    
    std::vector<long>* v_ptr = (std::vector<long>*)script_drawing_commands[i].GetPtr();
    
    if(!v_ptr)
    {
        al_trace("Quad3d: Vector pointer is null! Internal error. \n");
        return;
    }
    
    std::vector<long> &v = *v_ptr;
    
    if(v.empty())
        return;
        
    long* pos = &v[0];
    long* uv = &v[12];
    long* col = &v[20];
    long* size = &v[24];
    
    int w = size[0]; //magic numerical constants... yuck.
    int h = size[1];
    int flip = (sdci[6]/10000)&3;
    int tile = sdci[7]/10000;
    int polytype = sdci[8]/10000;
    
    polytype = vbound(polytype, 0, 14);
    
    if(((w-1) & w) != 0 || ((h-1) & h) != 0) return;   //non power of two error
    
    int tex_width = w*16;
    int tex_height = h*16;
    
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
        const newcombo & c = combobuf[ vbound(abs(tile), 0, 0xffff) ];
        const int tiletodraw = combo_tile(c, 0, 0);
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



void do_drawtriangle3dr(BITMAP *bmp, int i, int *sdci, int xoffset, int yoffset)
{
    //sdci[1]=layer
    //sdci[2]=pos[9]
    //sdci[3]=uv[6]
    //sdci[4]=color[3]
    //sdci[5]=size[2]
    //sdci[6]=flip
    //sdci[7]=tile/combo
    //sdci[8]=polytype
    
    std::vector<long>* v_ptr = (std::vector<long>*)script_drawing_commands[i].GetPtr();
    
    if(!v_ptr)
    {
        al_trace("Quad3d: Vector pointer is null! Internal error. \n");
        return;
    }
    
    std::vector<long> &v = *v_ptr;
    
    if(v.empty())
        return;
        
    long* pos = &v[0];
    long* uv = &v[9];
    long* col = &v[15];
    long* size = &v[18];
    
    int w = size[0]; //magic numerical constants... yuck.
    int h = size[1];
    int flip = (sdci[6]/10000)&3;
    int tile = sdci[7]/10000;
    int polytype = sdci[8]/10000;
    
    polytype = vbound(polytype, 0, 14);
    
    if(((w-1) & w) != 0 || ((h-1) & h) != 0) return;   //non power of two error
    
    int tex_width = w*16;
    int tex_height = h*16;
    
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
        const newcombo & c = combobuf[ vbound(abs(tile), 0, 0xffff) ];
        const int tiletodraw = combo_tile(c, 0, 0);
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


bool is_layer_transparent(const mapscr& m, int layer)
{
    layer = vbound(layer, 0, 5);
    return m.layeropacity[layer] == 128;
}

mapscr *getmapscreen(int map_index, int screen_index, int layer)   //returns NULL for invalid or non-existent layer
{
    mapscr *base_screen;
    int index = map_index*MAPSCRS+screen_index;
    
    if((unsigned int)layer > 6 || (unsigned int)index >= TheMaps.size())
        return NULL;
        
    if(layer != 0)
    {
        layer = layer - 1;
        
        base_screen=&(TheMaps[index]);
        
        if(base_screen->layermap[layer]==0)
            return NULL;
            
        index=(base_screen->layermap[layer]-1)*MAPSCRS+base_screen->layerscreen[layer];
        
        if((unsigned int)index >= TheMaps.size())   // Might as well make sure
            return NULL;
    }
    
    return &(TheMaps[index]);
}

void draw_mapscr(BITMAP *b, const mapscr& m, int x, int y, bool transparent)
{
    for(int i(0); i < 176; ++i)
    {
        const int x2 = ((i&15)<<4) + x;
        const int y2 = (i&0xF0) + y;
        
        //const newcombo & c = combobuf[ m.data[i] ];
	    
	/*
	newcombo c = combobuf[m.data[i]];
	int csets[4];
        int cofs = c.csets&15;
        
        if(cofs&8)
            cofs |= ~int(0xF);
            
        for(int i=0; i<4; ++i)
            csets[i] = c.csets&(16<<i) ? cset + cofs : cset;
	
	
        const int tile = combo_tile(c, x2, y2);
	    */
        
        if(transparent)
	{
		//void overcomboblocktranslucent(BITMAP *dest, int x, int y, int cmbdat, int cset, int w, int h, int opacity)
		overcomboblocktranslucent(b, x2, y2, m.data[i], m.cset[i], 1, 1, 128);

            //overtiletranslucent16(b, tile, x2, y2, m.cset[i], c.flip, 128);
	}
        else
	{
		//overcomboblock(BITMAP *dest, int x, int y, int cmbdat, int cset, int w, int h)
		overcomboblock(b, x2, y2, m.data[i], m.cset[i], 1, 1);
            //overtile16(b, tile, x2, y2, m.cset[i], c.flip);
	}
    }
}




void do_drawlayerr(BITMAP *bmp, int *sdci, int xoffset, int yoffset, bool isOffScreen)
{
    //sdci[1]=layer
    //sdci[2]=map
    //sdci[3]=screen
    //sdci[4]=layer
    //sdci[5]=x
    //sdci[6]=y
    //sdci[7]=rotation
    //sdci[8]=opacity
    
    int map = (sdci[2]/10000)-1; //zscript map indices start at 1.
    int scrn = sdci[3]/10000;
    int sourceLayer = vbound(sdci[4]/10000, 0, 6);
    int x = sdci[5]/10000;
    int y = sdci[6]/10000;
    int x1 = x + xoffset;
    int y1 = y + yoffset;
    int rotation = sdci[7]/10000;
    int opacity = sdci[8]/10000;
    
    const unsigned int index = (unsigned int)(map * MAPSCRS + scrn);
    const mapscr* m = getmapscreen(map, scrn, sourceLayer);
    
    if(!m) //no need to log it.
        return;

	if(index >= TheMaps.size())
	{
		al_trace("DrawLayer: invalid map index \"%i\". Map count is %d.\n", index, TheMaps.size());
		return;
	}
    
    const mapscr & l = *m;
    
    BITMAP* b = bmp;
    
    if(rotation != 0)
        b = script_drawing_commands.AquireSubBitmap(256, 176);
        
        
    const int maxX = isOffScreen ? 512 : 256;
    const int maxY = isOffScreen ? 512 : 176 + yoffset;
    bool transparent = opacity <= 128;
    
    if(rotation != 0) // rotate
    {
        draw_mapscr(b, l, x1, y1, transparent);
        
        rotate_sprite(bmp, b, x1, y1, degrees_to_fixed(rotation));
        script_drawing_commands.ReleaseSubBitmap(b);
    }
    else
    {
        for(int i(0); i < 176; ++i)
        {
            const int x2 = ((i&15)<<4) + x1;
            const int y2 = (i&0xF0) + y1;
            
            if(x2 > -16 && x2 < maxX && y2 > -16 && y2 < maxY)   //in clipping rect
            {
                const newcombo & c = combobuf[ l.data[i] ];
                const int tile = combo_tile(c, x2, y2);
                
                if(opacity < 128)
                    overtiletranslucent16(b, tile, x2, y2, l.cset[i], c.flip, opacity);
                else
                    overtile16(b, tile, x2, y2, l.cset[i], c.flip);
                    
                //putcombo( b, xx, yy, l.data[i], l.cset[i] );
            }
        }
    }
    
    //putscr
}



void do_drawscreenr(BITMAP *bmp, int *sdci, int xoffset, int yoffset, bool isOffScreen)
{
    //sdci[1]=layer
    //sdci[2]=map
    //sdci[3]=screen
    //sdci[4]=x
    //sdci[5]=y
    //sdci[6]=rotation
    
    int map = (sdci[2]/10000)-1; //zscript map indices start at 1.
    int scrn = sdci[3]/10000;
    int x = sdci[4]/10000;
    int y = sdci[5]/10000;
    int x1 = x + xoffset;
    int y1 = y + yoffset;
    int rotation = sdci[6]/10000;
    
    const unsigned int index = (unsigned int)(map * MAPSCRS + scrn);
    
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
    
    for(int i(0); i < 6; ++i)
    {
        if(m.layermap[i] == 0) continue;
        
        unsigned int layer_screen_index = (m.layermap[i]-1) * MAPSCRS + m.layerscreen[i];
        
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



/////////////////////////////////////////////////////////
// do primitives
////////////////////////////////////////////////////////

void do_primitives(BITMAP *targetBitmap, int type, mapscr *, int xoff, int yoff)
{
    color_map = &trans_table2;
    
    //was this next variable ever used? -- DN
    //bool drawsubscr=false;
    
    if(type > 7)
        return;
        
    //--script_drawing_commands[][] reference--
    //[][0]: type
    //[][1-16]: defined by type
    //[][17]: unused
    //[][18]: rendertarget
    //[][19]: unused
    
    // Trying to match the old behavior exactly...
    const bool brokenOffset= ( (get_bit(extra_rules, er_BITMAPOFFSET)!=0) || (get_bit(quest_rules,qr_BITMAPOFFSETFIX)!=0) );
     
    bool isTargetOffScreenBmp = false;
    const int type_mul_10000 = type * 10000;
    const int numDrawCommandsToProcess = script_drawing_commands.Count();
    int xoffset=xoff, yoffset=yoff;
    
    for(int i(0); i < numDrawCommandsToProcess; ++i)
    {
        if(!brokenOffset)
        {
            xoffset = 0;
            yoffset = 0;
        }
        int *sdci = &script_drawing_commands[i][0];
        
        if(sdci[1] != type_mul_10000)
            continue;
            
        // get the correct render target, if set.
        BITMAP *bmp = zscriptDrawingRenderTarget->GetTargetBitmap(sdci[18]);
        
        if(!bmp)
        {
			// draw to screen with subscreen offset
			if(!brokenOffset)
            {
                xoffset = xoff;
                yoffset = yoff;
            }
            bmp = targetBitmap;
        }
        else
        {
            //not drawing to screen, so no subscreen offset
            if(brokenOffset)
            {
                xoffset = 0;
                yoffset = 0;
            }
            isTargetOffScreenBmp = true;
        }
        
        switch(sdci[0])
        {
        case RECTR:
        {
            do_rectr(bmp, sdci, xoffset, yoffset);
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
        
        case DRAWTILER:
        {
            do_drawtiler(bmp, sdci, xoffset, yoffset);
        }
        break;
        
        case DRAWCOMBOR:
        {
            do_drawcombor(bmp, sdci, xoffset, yoffset);
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
	/*
	case POLYGONR:
        {
            do_polygonr(bmp, i, sdci, xoffset, yoffset);
        }
        break;
	*/
        
        case BITMAPR:
        {
            do_drawbitmapr(bmp, sdci, xoffset, yoffset);
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
        
        }
    }
    
    
    color_map=&trans_table;
}

