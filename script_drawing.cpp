
#include <allegro.h>
#include "script_drawing.h"
#include "rendertarget.h"
#include "maps.h"
#include "tiles.h"
#include "zelda.h"
#include "ffscript.h"
#include <stdio.h>


#define DegtoFix(d)     ((d)*0.71111111)

BITMAP* ScriptDrawingBitmapPool::_parent_bmp = 0;



FONT *get_zc_font(int index)
{
  //return getfont(index);
  switch(index)
  {
    default:               return zfont;
    case font_z3font:         return z3font;
    case font_z3smallfont:    return z3smallfont;
    case font_deffont:        return deffont;
    case font_lfont:       return lfont;
    case font_lfont_l:        return lfont_l;
    case font_pfont:       return pfont;
    case font_mfont:       return mfont;
    case font_ztfont:         return ztfont;
    case font_sfont:       return sfont;
    case font_sfont2:         return sfont2;
    case font_spfont:         return spfont;
    case font_ssfont1:        return ssfont1;
    case font_ssfont2:        return ssfont2;
    case font_ssfont3:        return ssfont3;
    case font_ssfont4:        return ssfont4;
    case font_gblafont:       return gblafont;
    case font_goronfont:      return goronfont;
    case font_zoranfont:      return zoranfont;
    case font_hylian1font:    return hylian1font;
    case font_hylian2font:    return hylian2font;
    case font_hylian3font:    return hylian3font;
    case font_hylian4font:    return hylian4font;
    case font_gboraclefont:      return gboraclefont;
    case font_gboraclepfont:  return gboraclepfont;
    case font_dsphantomfont:  return dsphantomfont;
    case font_dsphantompfont: return dsphantompfont;
  }
}


class TileHelper
{
public:

   static void OldPutTile( BITMAP* _Dest, int tile, int x, int y, int w, int h, int color, int flip )
   {
      switch (flip)
      {
         case 1:
            for(int j=0;j<h;j++)
               for(int k=w-1;k>=0;k--)
                  oldputtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+j*16, color, flip);
            break;
         case 2:
            for(int j=h-1;j>=0;j--)
               for(int k=0;k<w;k++)
                  oldputtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+((h-1)-j)*16, color, flip);
            break;
         case 3:
            for(int j=h-1;j>=0;j--)
               for(int k=w-1;k>=0;k--)
                  oldputtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+((h-1)-j)*16, color, flip);
            break;
         case 0:
         default:
            for(int j=0;j<h;j++)
               for(int k=0;k<w;k++)
                  oldputtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+j*16, color, flip);
            break;
      }
   }

   static void OverTile( BITMAP* _Dest, int tile, int x, int y, int w, int h, int color, int flip )
   {
      switch (flip)
      {
         case 1:
            for(int j=0;j<h;j++)
               for(int k=w-1;k>=0;k--)
                  overtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+j*16, color, flip);
            break;
         case 2:
            for(int j=h-1;j>=0;j--)
               for(int k=0;k<w;k++)
                  overtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+((h-1)-j)*16, color, flip);
            break;
         case 3:
            for(int j=h-1;j>=0;j--)
               for(int k=w-1;k>=0;k--)
                  overtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+((h-1)-j)*16, color, flip);
            break;
         default:
            for(int j=0;j<h;j++)
               for(int k=0;k<w;k++)
                  overtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+j*16, color, flip);
            break;
      }
   }

   static void OverTileTranslucent( BITMAP* _Dest, int tile, int x, int y, int w, int h, int color, int flip, int opacity )
   {
      switch (flip)
      {
         case 1:
            for(int j=0;j<h;j++)
               for(int k=w-1;k>=0;k--)
                  overtiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+j*16, color, flip, opacity);
            break;
         case 2:
            for(int j=h-1;j>=0;j--)
               for(int k=0;k<w;k++)
                  overtiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+((h-1)-j)*16, color, flip, opacity);
            break;
         case 3:
            for(int j=h-1;j>=0;j--)
               for(int k=w-1;k>=0;k--)
                  overtiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+((h-1)-j)*16, color, flip, opacity);
            break;
         default:
            for(int j=0;j<h;j++)
               for(int k=0;k<w;k++)
                  overtiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+j*16, color, flip, opacity);
            break;
      }
   }

   static void PutTileTranslucent( BITMAP* _Dest, int tile, int x, int y, int w, int h, int color, int flip, int opacity )
   {
      switch (flip)
      {
         case 1:
            for(int j=0;j<h;j++)
               for(int k=w-1;k>=0;k--)
                  puttiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+j*16, color, flip, opacity);
            break;
         case 2:
            for(int j=h-1;j>=0;j--)
               for(int k=0;k<w;k++)
                  puttiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+((h-1)-j)*16, color, flip, opacity);
            break;
         case 3:
            for(int j=h-1;j>=0;j--)
               for(int k=w-1;k>=0;k--)
                  puttiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+((h-1)-j)*16, color, flip, opacity);
            break;
         default:
            for(int j=0;j<h;j++)
               for(int k=0;k<w;k++)
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
	if (sdci[7]==0) //scale
	{
	  return;
	}
	int x1=sdci[2]/10000;
	int y1=sdci[3]/10000;
	int x2=sdci[4]/10000;
	int y2=sdci[5]/10000;
	if (x1>x2)
	{
	  zc_swap(x1,x2);
	}
	if (y1>y2)
	{
	  zc_swap(y1,y2);
	}
	if (sdci[7] != 10000)
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

	  xy[ 0]=xoffset+rx + fixtoi( ( fcosa * (x1 - rx) - fsina * (y1 - ry) ) ); //x1
	  xy[ 1]=yoffset+ry + fixtoi( ( fsina * (x1 - rx) + fcosa * (y1 - ry) ) ); //y1
	  xy[ 2]=xoffset+rx + fixtoi( ( fcosa * (x2 - rx) - fsina * (y1 - ry) ) ); //x2
	  xy[ 3]=yoffset+ry + fixtoi( ( fsina * (x2 - rx) + fcosa * (y1 - ry) ) ); //y1
	  xy[ 4]=xoffset+rx + fixtoi( ( fcosa * (x2 - rx) - fsina * (y2 - ry) ) ); //x2
	  xy[ 5]=yoffset+ry + fixtoi( ( fsina * (x2 - rx) + fcosa * (y2 - ry) ) ); //y2
	  xy[ 6]=xoffset+rx + fixtoi( ( fcosa * (x1 - rx) - fsina * (y2 - ry) ) ); //x1
	  xy[ 7]=yoffset+ry + fixtoi( ( fsina * (x1 - rx) + fcosa * (y2 - ry) ) ); //y2
	  xy[ 8]=xoffset+rx + fixtoi( ( fcosa * (x1 - rx    ) - fsina * (y1 - ry + 1) ) ); //x1
	  xy[ 9]=yoffset+ry + fixtoi( ( fsina * (x1 - rx    ) + fcosa * (y1 - ry + 1) ) ); //y1
	  xy[10]=xoffset+rx + fixtoi( ( fcosa * (x2 - rx - 1) - fsina * (y1 - ry    ) ) ); //x2
	  xy[11]=yoffset+ry + fixtoi( ( fsina * (x2 - rx - 1) + fcosa * (y1 - ry    ) ) ); //y1
	  xy[12]=xoffset+rx + fixtoi( ( fcosa * (x2 - rx    ) - fsina * (y2 - ry - 1) ) ); //x2
	  xy[13]=yoffset+ry + fixtoi( ( fsina * (x2 - rx    ) + fcosa * (y2 - ry - 1) ) ); //y2
	  xy[14]=xoffset+rx + fixtoi( ( fcosa * (x1 - rx + 1) - fsina * (y2 - ry    ) ) ); //x1
	  xy[15]=yoffset+ry + fixtoi( ( fsina * (x1 - rx + 1) + fcosa * (y2 - ry    ) ) ); //y2
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
	if (sdci[6]==0) //scale
	{
	  return;
	}
	int x1=sdci[2]/10000;
	int y1=sdci[3]/10000;
	qword r=sdci[4];
	if (sdci[6] != 10000)
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

	  xy[ 0]=rx + fixtoi( ( fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry) ) ); //x1
	  xy[ 1]=ry + fixtoi( ( fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry) ) ); //y1
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

	if (sdci[8]==0) //scale
	{
	  return;
	}
	int cx=sdci[2]/10000;
	int cy=sdci[3]/10000;
	qword r=sdci[4];
	if (sdci[8] != 10000)
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

	  cx=rx + fixtoi( ( fixcos(ra) * (cx - rx) - fixsin(ra) * (cy - ry) ) ); //x1
	  cy=ry + fixtoi( ( fixsin(ra) * (cx - rx) + fixcos(ra) * (cy - ry) ) ); //y1
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

	if (sdci[7]==0) //scale
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
	xy[ 0]=rx + fixtoi( ( fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry) ) ); //x1
	xy[ 1]=ry + fixtoi( ( fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry) ) ); //y1
	x1=xy[0];
	y1=xy[1];

	if(radx<1||rady<1||radx>255||rady>255) return;

	BITMAP* bitty = script_drawing_commands.AquireSubBitmap( radx*2+1, rady*2+1 );

	if(sdci[11]) //filled
	{

	  if(sdci[12]/10000<128) //translucent
	  {
		  clear_bitmap(prim_bmp);
		  ellipsefill(bitty, radx, rady, radx, rady, color);
		  rotate_sprite(prim_bmp, bitty, x1+xoffset-radx,y1+yoffset-rady, ftofix(DegtoFix(rotation)));
		  draw_trans_sprite(bmp, prim_bmp, 0, 0);
	  }
	  else // no opacity
	  {
		  ellipsefill(bitty, radx, rady, radx, rady, color);
		  rotate_sprite(bmp, bitty, x1+xoffset-radx,y1+yoffset-rady, ftofix(DegtoFix(rotation)));
	  }
	}
	else //not filled
	{
	  if(sdci[12]/10000<128) //translucent
	  {
		  clear_bitmap(prim_bmp);
		  ellipse(bitty, radx, rady, radx, rady, color);
		  rotate_sprite(prim_bmp, bitty, x1+xoffset-radx,y1+yoffset-rady, ftofix(DegtoFix(rotation)));
		  draw_trans_sprite(bmp, prim_bmp, 0, 0);
	  }
	  else // no opacity
	  {
		  ellipse(bitty, radx, rady, radx, rady, color);
		  rotate_sprite(bmp, bitty, x1+xoffset-radx,y1+yoffset-rady, ftofix(DegtoFix(rotation)));
	  }
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
	if (sdci[7]==0) //scale
	{
	  return;
	}
	int x1=sdci[2]/10000;
	int y1=sdci[3]/10000;
	int x2=sdci[4]/10000;
	int y2=sdci[5]/10000;

	if (sdci[7] != 10000)
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

	  xy[ 0]=rx + fixtoi( ( fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry) ) ); //x1
	  xy[ 1]=ry + fixtoi( ( fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry) ) ); //y1
	  xy[ 2]=rx + fixtoi( ( fixcos(ra) * (x2 - rx) - fixsin(ra) * (y2 - ry) ) ); //x2
	  xy[ 3]=ry + fixtoi( ( fixsin(ra) * (x2 - rx) + fixcos(ra) * (y2 - ry) ) ); //y2
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
	if( sdci[11]/10000 < 128 ) //translucent
	{
	  drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	}
	spline( bmp, points, sdci[10]/10000 );

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

	  xy[ 0]=rx + fixtoi( ( fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry) ) ); //x1
	  xy[ 1]=ry + fixtoi( ( fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry) ) ); //y1
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
	if (xscale==0||yscale==0)
	{
		return;
	}
	if (xscale<0||yscale<0)
	canscale = false; //default size

	if ( (xscale>0 && yscale>0) || rotation) //scaled or rotated
	{
		BITMAP* pbitty = script_drawing_commands.AquireSubBitmap( w*16, h*16 );

		if(transparency) //transparency
		{
			TileHelper::OverTile( pbitty, (sdci[4]/10000), 0, 0, w, h, color, flip );
		}
		else //no transparency
		{
			TileHelper::OldPutTile( pbitty, (sdci[4]/10000), 0, 0, w, h, color, flip );
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
			 xy[ 0]=rx + fixtoi( ( fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry) ) ); //x1
			 xy[ 1]=ry + fixtoi( ( fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry) ) ); //y1
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
				rotate_sprite(prim_bmp, tempbit, 0, 0, ftofix(DegtoFix(rotation)) );
				draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
			 }
			 else
			 {
				rotate_sprite(bmp, tempbit, x1+xoffset, y1+yoffset, ftofix(DegtoFix(rotation)) );
			 }

			 destroy_bitmap(tempbit);
		  }
		  else //no scale
		  {
			 if(opacity < 128)
			 {
				clear_bitmap(prim_bmp);
				rotate_sprite(prim_bmp, pbitty, 0, 0, ftofix(DegtoFix(rotation)) );
				draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
			 }
			 else
			 {
				rotate_sprite(bmp, pbitty, x1+xoffset, y1+yoffset, ftofix(DegtoFix(rotation)) );
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
			   TileHelper::OverTileTranslucent( bmp, (sdci[4]/10000), xoffset+x1, yoffset+y1, w, h, color, flip, opacity );
			else
			   TileHelper::OverTile( bmp, (sdci[4]/10000), xoffset+x1, yoffset+y1, w, h, color, flip );
		}
		else
		{
			if(opacity<=127)
			   TileHelper::PutTileTranslucent( bmp, (sdci[4]/10000), xoffset+x1, yoffset+y1, w, h, color, flip, opacity );
			else
			   TileHelper::OldPutTile( bmp, (sdci[4]/10000), xoffset+x1, yoffset+y1, w, h, color, flip );
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
	int flip=(sdci[14]/10000)&3;

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
	int tiletodraw = combo_tile(sdci[4]/10000, x1, y1);

	//don't scale if it's not safe to do so
	bool canscale = true;
	if (xscale==0||yscale==0)
	{
		return;
	}
	if (xscale<0||yscale<0)
	canscale = false; //default size

	if ( (xscale>0 && yscale>0) || rotation) //scaled or rotated
	{
		BITMAP* pbitty = script_drawing_commands.AquireSubBitmap( w*16, h*16 ); //-pbitty in the hisouse. :D

		if(transparency)
		{
		  TileHelper::OverTile( pbitty, tiletodraw, 0, 0, w, h, color, flip );
		}
		else //no transparency
		{
		  TileHelper::OldPutTile( pbitty, tiletodraw, 0, 0, w, h, color, flip );
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
			 xy[ 0]=rx + fixtoi( ( fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry) ) ); //x1
			 xy[ 1]=ry + fixtoi( ( fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry) ) ); //y1
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
				rotate_sprite(prim_bmp, tempbit, 0, 0, ftofix(DegtoFix(rotation)) );
				draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
			 }
			 else
			 {
				rotate_sprite(bmp, tempbit, x1+xoffset, y1+yoffset, ftofix(DegtoFix(rotation)) );
			 }

			 destroy_bitmap(tempbit);
		  }
		  else //no scale
		  {
			 if(opacity < 128)
			 {
				clear_bitmap(prim_bmp);
				rotate_sprite(prim_bmp, pbitty, 0, 0, ftofix(DegtoFix(rotation)) );
				draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
			 }
			 else
			 {
				rotate_sprite(bmp, pbitty, x1+xoffset, y1+yoffset, ftofix(DegtoFix(rotation)) );
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
			  TileHelper::OverTileTranslucent( bmp, tiletodraw, xoffset+x1, yoffset+y1, w, h, color, flip, opacity );
		   else
			  TileHelper::OverTile( bmp, tiletodraw, xoffset+x1, yoffset+y1, w, h, color, flip );
		}
		else
		{
		   if(opacity<=127)
			  TileHelper::PutTileTranslucent( bmp, tiletodraw, xoffset+x1, yoffset+y1, w, h, color, flip, opacity );
		   else
			  TileHelper::OldPutTile( bmp, tiletodraw, xoffset+x1, yoffset+y1, w, h, color, flip);
		}
	}
}


void do_fasttiler(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
{
	/* layer, x, y, tile, color opacity */

	int opacity = sdci[6]/10000;

	if( opacity < 128 )
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

	if( opacity < 128 )
		overtiletranslucent16(bmp, combo_tile(sdci[4]/10000, x1, y1), xoffset+x1, yoffset+y1, sdci[5]/10000, 0, opacity);
	else
		overtile16(bmp, combo_tile(sdci[4]/10000, x1, y1), xoffset+x1, yoffset+y1, sdci[5]/10000, 0);
}



void do_drawcharr(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
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
		BITMAP *pbmp = create_bitmap_ex(8,16,16);
		clear_bitmap(pbmp);

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

		destroy_bitmap(pbmp);
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


void do_drawintr(BITMAP *bmp, int *sdci, int xoffset, int yoffset)
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
	float number =(sdci[9]/10000);
	int decplace = sdci[10]/10000;
	int opacity=sdci[11]/10000;

	//safe check
	if(bg_color < -1) bg_color = -1;
	if(w>512) w=512; //w=vbound(w,0,512);
	if(h>512) h=512; //h=vbound(h,0,512);

	char numbuf[15];
	switch(decplace)
	{
		default:
		case 0:  sprintf(numbuf,"%d",int(number)); break;
		case 1: sprintf(numbuf,"%.01f",number); break;
		case 2:  sprintf(numbuf,"%.02f",number); break;
		case 3: sprintf(numbuf,"%.03f",number); break;
		case 4: sprintf(numbuf,"%.04f",number); break;
	}

	if(w>0&&h>0)//stretch
	{
		BITMAP *pbmp = create_bitmap_ex(8,16,16);
		clear_bitmap(pbmp);

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

		destroy_bitmap(pbmp);
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


void do_drawstringr(BITMAP *bmp, int i, int *sdci, int xoffset, int yoffset)
{
	//sdci[1]=layer
	//sdci[2]=x
	//sdci[3]=y
	//sdci[4]=font
	//sdci[5]=color
	//sdci[6]=bg color
	//sdci[7]=format_option
	//sdci[8]=opacity
	//sdci[9]=char

	std::string* str = (std::string*)script_drawing_commands[i].GetPtr();
	if( !str )
	{
		al_trace( "String pointer is null! Internal error. \n");
		return;
	}

	int x=sdci[2]/10000;
	int y=sdci[3]/10000;
	int font_index=sdci[4]/10000;
	int color=sdci[5]/10000;
	int bg_color=sdci[6]/10000; //-1 = transparent
	int format_type=sdci[7]/10000;
	int opacity=sdci[9]/10000;
	//sdci[8] not needed :)

	//safe check
	if(bg_color < -1) bg_color = -1;

	if(opacity < 128)
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);

	if( format_type == 2 ) // right-sided text
	{
		textout_right_ex( bmp, get_zc_font(font_index), str->c_str(), x+xoffset, y+yoffset, color, bg_color );
	}
	else if( format_type == 1 ) // centered text
	{
		textout_centre_ex( bmp, get_zc_font(font_index), str->c_str(), x+xoffset, y+yoffset, color, bg_color );
	}
	else // standard left-sided text
	{
		textout_ex( bmp, get_zc_font(font_index), str->c_str(), x+xoffset, y+yoffset, color, bg_color );
	}

	if(opacity < 128)
		drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
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
	if( ((w-1) & w) != 0 || ((h-1) & h) != 0 ) return; //non power of two error

	BITMAP *tex = script_drawing_commands.AquireSubBitmap(16*w,16*h);

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

	if(tile > 0)   // TILE
	{
		TileHelper::OverTile( tex, tile, 0, 0, w, h, color, flip );
	}
	else        // COMBO
	{
		const int tiletodraw = combo_tile( abs(tile), x1, y1 );
		TileHelper::OldPutTile( tex, tiletodraw, 0, 0, w, h, color, flip );
	}

	int tex_width = w*16;
	int tex_height = h*16;

	V3D_f V1 = { x1+xoffset, y1+yoffset, 0,        0,          0, col[0] };
	V3D_f V2 = { x2+xoffset, y2+yoffset, 0,        0, tex_height, col[1] };
	V3D_f V3 = { x3+xoffset, y3+yoffset, 0, tex_width,tex_height, col[2] };
	V3D_f V4 = { x4+xoffset, y4+yoffset, 0, tex_width,         0, col[3] };

	quad3d_f(bmp, polytype, tex, &V1, &V2, &V3, &V4);

	script_drawing_commands.ReleaseSubBitmap(tex);
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
	if( ((w-1) & w) != 0 || ((h-1) & h) != 0 ) return; //non power of two error

	BITMAP *tex = script_drawing_commands.AquireSubBitmap(16*w,16*h);

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
		TileHelper::OverTile( tex, tile, 0, 0, w, h, color, flip );
	}
	else        // COMBO
	{
		const int tiletodraw = combo_tile( abs(tile), x1, y1 );
		TileHelper::OldPutTile( tex, tiletodraw, 0, 0, w, h, color, flip );
	}

	int tex_width = w*16;
	int tex_height = h*16;

	V3D_f V1 = { x1+xoffset, y1+yoffset, 0,    0 ,          0 , col[0] };
	V3D_f V2 = { x2+xoffset, y2+yoffset, 0,    0 , tex_height , col[1] };
	V3D_f V3 = { x3+xoffset, y3+yoffset, 0, tex_width, tex_height , col[2] };


	triangle3d_f(bmp, polytype, tex, &V1, &V2, &V3);
	script_drawing_commands.ReleaseSubBitmap(tex);
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
	//sdci[11]=mask

	int bitmapIndex = sdci[2]/10000;
	int sx = sdci[3]/10000;
	int sy = sdci[4]/10000;
	int sw = sdci[5]/10000;
	int sh = sdci[6]/10000;
	int dx = sdci[7]/10000;
	int dy = sdci[8]/10000;
	int dw = sdci[9]/10000;
	int dh = sdci[10]/10000;
	bool masked = (sdci[11] != 0);

	if(sx >= ZScriptDrawingRenderTarget::BitmapWidth || sy >= ZScriptDrawingRenderTarget::BitmapHeight)
	 return;

	bool stretched = (sw != dw || sh != dh);

	BITMAP *sourceBitmap = zscriptDrawingRenderTarget->GetBitmapPtr(bitmapIndex);
	if( !sourceBitmap)
	{
		Z_message("Warning: Screen->DrawBitmap(%d) contains invalid data or is not initialized.\n", bitmapIndex);
		Z_message("[Note* Deferred drawing or layering order possibly not set right.]\n");
		return;
	}

	dx = dx + xoffset;
	dy = dy + yoffset;

	if( stretched )
	{
		if( masked )
		{
			masked_stretch_blit(sourceBitmap, bmp, sx, sy, sw, sh, dx, dy, dw, dh);
		}
		else
		{
			stretch_blit(sourceBitmap, bmp, sx, sy, sw, sh, dx, dy, dw, dh);
		}
	}
	else
	{
		if( masked )
		{
			masked_blit(sourceBitmap, bmp, sx, sy, dx, dy, dw, dh);
		}
		else
		{
			blit(sourceBitmap, bmp, sx, sy, dx, dy, dw, dh);
		}
	}
}












/////////////////////////////////////////////////////////
// do primitives
////////////////////////////////////////////////////////

void do_primitives(BITMAP *targetBitmap, int type, mapscr *, int xoffset, int yoffset)
{
	color_map = &trans_table2;

	//was this next variable ever used? -- DN
	//bool drawsubscr=false;

	if( type > 7 )
	 return;

	//--script_drawing_commands[][] reference--
	//[][0]: type
	//[][1-16]: defined by type
	//[][17]: unused
	//[][18]: rendertarget
	//[][19]: unused

	const int type_mul_10000 = type * 10000;
	const int numDrawCommandsToProcess = script_drawing_commands.Count();

	for( int i(0); i < numDrawCommandsToProcess; ++i )
	{
		int *sdci = &script_drawing_commands[i][0];

		if( sdci[1] != type_mul_10000 )
			continue;

		// get the correct render target, if set.
		BITMAP *bmp = zscriptDrawingRenderTarget->GetTargetBitmap(sdci[18]);
		if(!bmp)
		{
			bmp = targetBitmap;
		}
		else
		{
			//not drawing to screen, so no subscreen offset
			xoffset = 0;
			yoffset = 0;
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
         //sdci[1]=layer
         //sdci[2]=pos[12]
         //sdci[3]=uv[8]
         //sdci[4]=color[4]
         //sdci[5]=size[2]
         //sdci[6]=tile/combo
         //sdci[7]=polytype

		  /*
         if( draw_container.quad3D.empty() )
         {
            return;
         }

         std::deque< quad3Dstruct > ::iterator q3d_it
         = draw_container.getQuad3dIterator( sdci[19] );

         // dereferenced, for her pleasure. :p
         const quad3Dstruct *q = & (*q3d_it);

         int w = q->size[0];
         int h = q->size[1];
         int flip = (sdci[6]/10000)&3;
         int tile = sdci[7]/10000;
         int polytype = sdci[8]/10000;

         polytype = vbound(polytype, 0, 14);
         if( !(zc::math::IsPowerOfTwo(w) && zc::math::IsPowerOfTwo(h)) )
         {
            draw_container.quad3D.erase( q3d_it );
            break;
         }

         BITMAP *tex = create_bitmap_ex(8,16*w,16*h);
         clear_bitmap(tex);

         if(tile > 0)   // TILE
         {
            TileHelper::OverTile( tex, tile, 0, 0, w, h, q->color[0], flip );
         }
         else        // COMBO
         {
            const int tiletodraw = combo_tile( abs(tile), int(q->pos[0]), int(q->pos[1]) );
            TileHelper::OldPutTile( tex, tiletodraw, 0, 0, w, h, q->color[0], flip );
         }

         V3D_f V1 = { q->pos[0]+xoffset, q->pos[1] +yoffset, q->pos[2] , q->uv[0], q->uv[1], q->color[0] };
         V3D_f V2 = { q->pos[3]+xoffset, q->pos[4] +yoffset, q->pos[5] , q->uv[2], q->uv[3], q->color[1] };
         V3D_f V3 = { q->pos[6]+xoffset, q->pos[7] +yoffset, q->pos[8] , q->uv[4], q->uv[5], q->color[2] };
         V3D_f V4 = { q->pos[9]+xoffset, q->pos[10]+yoffset, q->pos[11], q->uv[6], q->uv[7], q->color[3] };

         quad3d_f(bmp, polytype, tex, &V1, &V2, &V3, &V4);
         destroy_bitmap(tex);

         draw_container.quad3D.erase( q3d_it );
		 */
      }
      break;

		case TRIANGLER:
		{
			do_drawtriangler(bmp, sdci, xoffset, yoffset);
		}
		break;

      case TRIANGLE3DR:
      {
         //sdci[1]=layer
         //sdci[6]=flip
         //sdci[7]=tile/combo
         //sdci[8]=polytype

		  /*
         if( draw_container.triangle3D.empty() )
         {
            return;
         }
         std::deque< triangle3Dstruct > ::iterator t3d_it
            = draw_container.getTriangle3dIterator( sdci[19] );

         // dereferenced, for her pleasure. :p
         const triangle3Dstruct *q = & (*t3d_it);

         int w = q->size[0];
         int h = q->size[1];
         int flip = (sdci[6]/10000)&3;
         int tile = sdci[7]/10000;
         int polytype = sdci[8]/10000;

         polytype = vbound(polytype, 0, 14);
         if( !(zc::math::IsPowerOfTwo(w) && zc::math::IsPowerOfTwo(h)) )
            break;

         BITMAP *tex = create_bitmap_ex(8,16*w,16*h);
         clear_bitmap(tex);

         if(tile > 0)   // TILE
         {
            TileHelper::OverTile( tex, tile, 0, 0, w, h, q->color[0], flip );
         }
         else        // COMBO
         {
            const int tiletodraw = combo_tile( abs(tile), int(q->pos[0]), int(q->pos[1]) );
            TileHelper::OldPutTile( tex, tiletodraw, 0, 0, w, h, q->color[0], flip );
         }

         V3D_f V1 = { q->pos[0]+xoffset, q->pos[1] +yoffset, q->pos[2] ,   q->uv[0], q->uv[1], q->color[0] };
         V3D_f V2 = { q->pos[3]+xoffset, q->pos[4] +yoffset, q->pos[5] ,   q->uv[2], q->uv[3], q->color[1] };
         V3D_f V3 = { q->pos[6]+xoffset, q->pos[7] +yoffset, q->pos[8] , q->uv[4], q->uv[5], q->color[2] };

         triangle3d_f(bmp, polytype, tex, &V1, &V2, &V3);
         destroy_bitmap(tex);

         draw_container.triangle3D.erase(t3d_it);
		 */
      }
      break;

		case BITMAPR:
		{
			do_drawbitmapr(bmp, sdci, xoffset, yoffset);
		}
		break;

    }
  }


  color_map=&trans_table;
}
