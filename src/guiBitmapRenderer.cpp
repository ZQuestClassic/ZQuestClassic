#include "guiBitmapRenderer.h"
#include "tiles.h"
#include <allegro.h>

extern bool blank_tile_table[65520];

static int convertColor(int color)
{
    RGB rgb;
    get_color(color, &rgb);
    return makecol_depth(
      24,
      (rgb.r<<2)|(rgb.r>>4),
      (rgb.g<<2)|(rgb.g>>4),
      (rgb.b<<2)|(rgb.b>>4));
}

GUIBitmapRenderer::GUIBitmapRenderer(int s):
    scale(s),
    bmp(0)
{
}

GUIBitmapRenderer::~GUIBitmapRenderer()
{
    destroy_bitmap(bmp);
}

void GUIBitmapRenderer::initialize(int width, int height)
{
    bmp=create_bitmap_ex(24, width*scale, height*scale);
    clear_bitmap(bmp);
}

void* GUIBitmapRenderer::getData()
{
    return *bmp->line;
}

void GUIBitmapRenderer::getActualSize(int& width, int& height) const
{
    width=bmp->w;
    height=bmp->h;
}

void GUIBitmapRenderer::clear()
{
    clear_bitmap(bmp);
}

void GUIBitmapRenderer::drawRect(int x, int y, int width, int height, int color)
{
    rect(bmp, x*scale, y*scale, (x+width)*scale-1, (y+height)*scale-1, convertColor(color));
}

void GUIBitmapRenderer::fillRect(int x, int y, int width, int height, int color)
{
    rectfill(bmp, x*scale, y*scale, (x+width)*scale-1, (y+height)*scale-1, convertColor(color));
}

void GUIBitmapRenderer::drawLine(int x1, int y1, int x2, int y2, int color)
{
    line(bmp, x1*scale, y1*scale, x2*scale, y2*scale, convertColor(color));
}

void GUIBitmapRenderer::putTile(int tile, int cset, Orientation orientation,
  int x, int y)
{
    if(!blank_tile_table[tile])
    {
        // This can be redone later...
        static BITMAP* tempBMP1=0;
        static BITMAP* tempBMP2=0;
        if(!tempBMP1)
        {
            tempBMP1=create_bitmap_ex(8, 16, 16);
            tempBMP2=create_bitmap_ex(24, 16, 16);
        }
        
        puttile16(tempBMP1, tile, 0, 0, cset, orientation);
        blit(tempBMP1, tempBMP2, 0, 0, 0, 0, 16, 16);
        stretch_blit(tempBMP2, bmp, 0, 0, 16, 16, x*scale, y*scale, 16*scale, 16*scale);
    }
    else // Blank
    {
        int top=(y+2)*scale;
        int bottom=(y+13)*scale-1;
        int left=(x+2)*scale;
        int right=(x+13)*scale-1;
        int black=makecol_depth(24, 0, 0, 0);
        int white=makecol_depth(24, 255, 255, 255);
        
        rectfill(bmp, x*scale, y*scale, (x+16)*scale-1, (y+16)*scale-1, black);
        rect(bmp, left, top, right, bottom, white);
        line(bmp, left, top, right, bottom, white);
        line(bmp, left, bottom, right, top, white);
    }
}
