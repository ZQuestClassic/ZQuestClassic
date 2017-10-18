#include "renderer.h"
#include "common.h"
#include <allegro.h>
#include <algorithm>

using std::min;
using std::max;

extern BITMAP* hw_screen;

namespace GUI
{

AllegroGUIRenderer::AllegroGUIRenderer():
    target(screen),
    originX(0),
    originY(0)
{
}

AllegroGUIRenderer::~AllegroGUIRenderer()
{
}

void AllegroGUIRenderer::pushClipRect(int x, int y, int w, int h)
{
    adjustPoint(x, y);
    ClipRect newCR;
    
    if(clipRectStack.empty())
    {
        newCR.x1=x;
        newCR.y1=y;
        newCR.x2=x+w-1;
        newCR.y2=y+h-1;
    }
    else
    {
        ClipRect& current=clipRectStack.top();
        newCR.x1=max(x, current.x1);
        newCR.y1=max(y, current.y1);
        newCR.x2=min(x+w-1, current.x2);
        newCR.y2=min(y+h-1, current.y2);
    }
    
    set_clip_rect(target, newCR.x1, newCR.y1, newCR.x2, newCR.y2);
    clipRectStack.push(newCR);
}

void AllegroGUIRenderer::popClipRect()
{
    clipRectStack.pop();
    if(clipRectStack.empty())
        set_clip_rect(target, 0, 0, target->w-1, target->h-1);
    else
    {
        ClipRect& top=clipRectStack.top();
        set_clip_rect(target, top.x1, top.y1, top.x2, top.y2);
    }
}

void AllegroGUIRenderer::drawLine(int x1, int y1, int x2, int y2, const GUIColor& color)
{
    adjustPoint(x1, y1);
    adjustPoint(x2, y2);
    line(target, x1, y1, x2, y2, color);
}

void AllegroGUIRenderer::drawText(const std::string& text, int x, int y,
  FONT* font, const GUIColor& color, textAlignment alignment)
{
    adjustPoint(x, y);
    FONT* tempFont=::font;
    ::font=font;
    gui_textout_ex(target, text.c_str(), x, y, color, -1, alignment==ta_center);
    ::font=tempFont;
}

void AllegroGUIRenderer::drawRect(int x, int y, int width, int height, const GUIColor& color)
{
    adjustPoint(x, y);
    rect(target, x, y, x+width-1, y+height-1, color);
}

void AllegroGUIRenderer::fillRect(int x, int y, int width, int height, const GUIColor& color)
{
    adjustPoint(x, y);
    rectfill(target, x, y, x+width-1, y+height-1, color);
}

void AllegroGUIRenderer::fillCircle(int x, int y, int radius, const GUIColor& color)
{
    adjustPoint(x, y);
    circlefill(target, x, y, radius, color);
}

void AllegroGUIRenderer::drawRoundedRect(int x, int y, int width, int height,
  int radius, const GUIColor& color)
{
    adjustPoint(x, y);
    arc(target, x+radius, y+radius, itofix(64), itofix(128), radius, color);
    arc(target, x+width-radius-1, y+radius, itofix(0), itofix(64), radius, color);
    arc(target, x+radius, y+height-radius-1, itofix(128), itofix(192), radius, color);
    arc(target, x+width-radius-1, y+height-radius-1, itofix(192), itofix(256), radius, color);
    hline(target, x+radius, y, x+width-radius, color);
    hline(target, x+radius, y+height-1, x+width-radius-1, color);
    vline(target, x, y+radius, y+height-radius-1, color);
    vline(target, x+width-1, y+radius, y+height-radius-1, color);
}

void AllegroGUIRenderer::fillRoundedRect(int x, int y, int width, int height,
  int radius, const GUIColor& color)
{
    adjustPoint(x, y);
    circlefill(target, x+radius, y+radius, radius, color);
    circlefill(target, x+width-radius-1, y+radius, radius, color);
    circlefill(target, x+radius, y+height-radius-1, radius, color);
    circlefill(target, x+width-radius-1, y+height-radius-1, radius, color);
    rectfill(target, x, y+radius, x+radius, y+height-radius-1, color);
    rectfill(target, x+width-radius, y+radius, x+width-1, y+height-radius-1, color);
    rectfill(target, x+radius, y, x+width-radius-1, y+height-1, color);
}

void AllegroGUIRenderer::drawPartialRoundedRect(int x, int y, int width,
  int height, int radius, partialRoundedRect part, const GUIColor& color)
{
    adjustPoint(x, y);
    switch(part)
    {
    case prr_top:
        arc(target, x+radius, y+radius, itofix(64), itofix(128), radius, color);
        arc(target, x+width-radius-1, y+radius, itofix(0), itofix(64), radius, color);
        hline(target, x+radius, y, x+width-radius, color);
        vline(target, x, y+radius, y+height-1, color);
        vline(target, x+width-1, y+radius, y+height-1, color);
        break;
        
    case prr_bottom:
        arc(target, x+radius, y+height-radius-1, itofix(128), itofix(192), radius, color);
        arc(target, x+width-radius-1, y+height-radius-1, itofix(192), itofix(256), radius, color);
        hline(target, x+radius, y+height-1, x+width-radius-1, color);
        vline(target, x, y, y+height-radius-1, color);
        vline(target, x+width-1, y, y+height-radius-1, color);
        break;
        
    case prr_topRight:
        break;
        
    case prr_bottomRight:
        break;
    }
}

void AllegroGUIRenderer::fillPartialRoundedRect(int x, int y, int width,
  int height, int radius,partialRoundedRect part, const GUIColor& color)
{
    adjustPoint(x, y);
    switch(part)
    {
    case prr_top:
        circlefill(target, x+radius, y+radius, radius, color);
        circlefill(target, x+width-radius-1, y+radius, radius, color);
        rectfill(target, x, y+radius, x+width-1, y+height-1, color);
        rectfill(target, x+radius, y, x+width-radius-1, y+radius-1, color);
        break;
    case prr_bottom:
        circlefill(target, x+radius, y+height-radius-1, radius, color);
        circlefill(target, x+width-radius-1, y+height-radius-1, radius, color);
        rectfill(target, x, y, x+width-1, y+height-radius-1, color);
        rectfill(target, x+radius, y+height-radius, x+width-radius-1, y+height-1, color);
        break;
        
    case prr_topRight:
        break;
        
    case prr_bottomRight:
        break;
    }
}

void AllegroGUIRenderer::fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const GUIColor& color)
{
    adjustPoint(x1, y1);
    adjustPoint(x2, y2);
    adjustPoint(x3, y3);
    triangle(target, x1, y1, x2, y2, x3, y3, color);
}

void AllegroGUIRenderer::drawFrame(int x1, int y1, int width, int height, const GUIColor& color, frameType type, int depth)
{
    adjustPoint(x1, y1);
    int x2=x1+width-1;
    int y2=y1+height-1;
    int lightColor=makecol(color.red*1.2, color.green*1.2, color.blue*1.2);
    int darkColor=makecol(color.red*0.8, color.green*0.8, color.blue*0.8);
    
    int topLeft, bottomRight;
    if(type==ft_convex)
    {
        topLeft=lightColor;
        bottomRight=darkColor;
    }
    else
    {
        topLeft=darkColor;
        bottomRight=lightColor;
    }
    
    for(int i=0; i<depth; i++)
    {
        line(target, x1, y1+i, x2-1-i, y1+i, topLeft);
        line(target, x1+i, y1, x1+i, y2-1-i, topLeft);
        line(target, x1+1+i, y2-i, x2, y2-i, bottomRight);
        line(target, x2-i, y1+1+i, x2-i, y2, bottomRight);
    }
    
    depth-=1;
    line(target, x2, y1, x2-depth, y1+depth, color);
    line(target, x1, y2, x1+depth, y2-depth, color);
}

void AllegroGUIRenderer::blitBitmap(BITMAP* bmp, int x, int y)
{
    adjustPoint(x, y);
    blit(bmp, target, 0, 0, x, y, bmp->w, bmp->h);
}

}
