// This program is free software; you can redistribute it and/or modify it under the terms of the
// modified version 3 of the GNU General Public License. See License.txt for details.

#include "precompiled.h" //always first
#include "messageRenderer.h"

#include "subscr.h"
#include "zelda.h"
#include <allegro.h>

MessageRenderer::MessageRenderer():
    fullBmp(0),
    textBmp(0)
{
}

MessageRenderer::~MessageRenderer()
{
    if(fullBmp)
    {
        destroy_bitmap(textBmp);
        destroy_bitmap(fullBmp);
    }
}

void MessageRenderer::initialize(const MsgStr* msg)
{
    // If msg is null, initialize for DMap intros
    int width, height;
    if(msg)
    {
        font=Font::getByID(msg->font);
        hSpace=msg->hspace;
        vSpace=msg->vspace;
        
        width=msg->w;
        height=msg->h;
    }
    else
    {
        font=Font::getByID(0); // Not sure what to put here, exactly
        hSpace=0;
        vSpace=0;
        
        width=192;
        height=24;
    }
    
    if(!fullBmp)
    {
        fullBmp=create_bitmap_ex(8, width+16, height+16);
        textBmp=create_sub_bitmap(fullBmp, 8, 8, width, height);
    }
    else if(textBmp->w!=width || textBmp->h!=height)
    {
        destroy_bitmap(textBmp);
        destroy_bitmap(fullBmp);
        fullBmp=create_bitmap_ex(8, width+16, height+16);
        textBmp=create_sub_bitmap(fullBmp, 8, 8, width, height);
    }
    
    clear_bitmap(fullBmp);
    if(msg && msg->tile!=0)
    {
        frame2x2(fullBmp, &QMisc, 0, 0, msg->tile, msg->cset,
          fullBmp->w/8, fullBmp->h/8, 0, true, 0);
    }
    
    color=QMisc.colors.msgtext;
    cursorX=0;
    cursorY=0;
}

void MessageRenderer::loadContinuationStyle(const MsgStr* msg)
{
    font=Font::getByID(msg->font);
    hSpace=msg->hspace;
    vSpace=msg->vspace;
}

bool MessageRenderer::wordFits(int wordWidth, int numChars) const
{
    wordWidth+=(numChars-1)*hSpace;
    return cursorX+wordWidth<=textBmp->w;
}

void MessageRenderer::putChar(char c)
{
    int width=font.getWidth(c);
    if(cursorX+width>textBmp->w)
    {
        cursorX=0;
        cursorY+=font.getHeight()+vSpace;
    }
    
    char charStr[2]={c, 0};
    textout_ex(textBmp, font.get(), charStr, cursorX, cursorY, color, -1);
    cursorX+=width+hSpace;
}

void MessageRenderer::draw(BITMAP* target, int x, int y)
{
    masked_blit(fullBmp, target, 0, 0, x, y, fullBmp->w, fullBmp->h);
}
