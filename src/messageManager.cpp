// This program is free software; you can redistribute it and/or modify it under the terms of the
// modified version 3 of the GNU General Public License. See License.txt for details.

#include "precompiled.h" //always first

#include "messageManager.h"
#include "link.h"
#include "message.h"
#include "weapons.h"
#include "zc_sys.h"
#include "zdefs.h"
#include "zelda.h"
#include <allegro.h>

extern LinkClass Link;

namespace
{
    bool hasContinuation(const MsgStr* msg)
    {
        if(!msg || msg->nextstring==0)
            return false;
        
        MsgStr* nextMsg=&MsgStrings[msg->nextstring];
        return (nextMsg->stringflags&STRINGFLAG_CONT)!=0;
    }
}

MessageManager::MessageManager():
    currMsgStr(0),
    activeMessage(0)
{
}

MessageManager::~MessageManager()
{
    delete activeMessage;
}

void MessageManager::showMessage(int id)
{
    if(onMessageEnd)
    {
        onMessageEnd();
        onMessageEnd.clear();
    }
    delete activeMessage;
    
    currMsgStr=&MsgStrings[id];
    activeMessage=new Message(currMsgStr, *this);
    
    if(activeMessage->atEnd() && hasContinuation(currMsgStr))
    {
        // Empty message that continues to another. It's also possible
        // that one's also empty and has yet another continuation,
        // so it needs to be checked in a loop.
        do
        {
            currMsgStr=&MsgStrings[currMsgStr->nextstring];
            activeMessage->continueTo(currMsgStr);
        } while(activeMessage->atEnd() && hasContinuation(currMsgStr));
    }
    
    // We'll just assume we didn't end up with an empty message.
    // If that did happen, it'll be resolved on the first update.
    state=mst_active;
}

void MessageManager::showDMapIntro(int dmap)
{
    if(onMessageEnd)
    {
        onMessageEnd();
        onMessageEnd.clear();
    }
    delete activeMessage;
    currMsgStr=0;
    activeMessage=new Message(dmap, *this);
    state=mst_active;
}

void MessageManager::clear(bool runCallback)
{
    if(onMessageEnd && runCallback)
    {
        onMessageEnd();
        onMessageEnd.clear();
    }
    delete activeMessage;
    activeMessage=0;
    state=mst_inactive;
}

void MessageManager::update()
{
    if(!activeMessage || state==mst_inactive)
        return;
    
    if(state==mst_active)
    {
        Message::updateSpeed speed;
        if(cBbtn() && get_bit(quest_rules,qr_ALLOWMSGBYPASS))
            speed=Message::spd_instant;
        else if(cAbtn() && get_bit(quest_rules,qr_ALLOWFASTMSG))
            speed=Message::spd_fast;
        else
            speed=Message::spd_normal;
        
        activeMessage->update(speed);
        while(activeMessage->atEnd() && hasContinuation(currMsgStr))
        {
            // Continue to the next message. Check in a loop in case
            // there's an empty continuation.
            currMsgStr=&MsgStrings[currMsgStr->nextstring];
            activeMessage->continueTo(currMsgStr);
        }
        
        if(activeMessage->atEnd())
        {
            if(!currMsgStr || // DMap intro?
              currMsgStr->nextstring!=0 || // Another message?
              get_bit(quest_rules,qr_MSGDISAPPEAR)) // Messages disappear?
            {
                state=mst_waitingToAdvance;
                timer=51;
            }
            else // Do nothing
            {
                state=mst_inactive;
                if(onMessageEnd)
                {
                    onMessageEnd();
                    onMessageEnd.clear();
                }
            }
        }
    }
    else if(state==mst_waitingToAdvance)
    {
        timer--;
        if(timer==0)
        {
            // Timer ran out; what to do next?
            if(!currMsgStr) // DMap intro; clear it
            {
                delete activeMessage;
                activeMessage=0;
                state=mst_inactive;
                // Currently, DMap intros will never use onMessageEnd,
                // so it isn't checked
            }
            else // Must be something that needs the player to press A
                state=mst_awaitingInput;
        }
    }
    else // Awaiting input
    {
        if(cAbtn())
        {
            // Proceed to the next message if there is one; otherwise, clear it
            if(currMsgStr && currMsgStr->nextstring!=0)
                switchTo(currMsgStr->nextstring);
            else
            {
                delete activeMessage;
                activeMessage=0;
                state=mst_inactive;
                if(onMessageEnd)
                {
                    onMessageEnd();
                    onMessageEnd.clear();
                }
            }
        }
    }
}

void MessageManager::switchTo(int msg)
{
    if(onMessageEnd)
    {
        onMessageEnd();
        onMessageEnd.clear();
    }
    
    if(msg<0 || msg>msg_count)
        msg=0; // Currently shows "(none)" instead of nothing
    currMsgStr=&MsgStrings[msg];
    activeMessage->reset(currMsgStr);
    state=mst_active;
}

void MessageManager::drawToScreen(BITMAP* target, int yOffset)
{
    if(activeMessage)
        activeMessage->draw(target, yOffset);
    
    if(state==mst_awaitingInput)
    {
        // Draw "more..." icon
        int moreX=zinit.msg_more_x;
        int moreY=zinit.msg_more_y+yOffset;
        if(zinit.msg_more_is_offset)
            moreY+=currMsgStr->y;
        putMiscSprite(target, iwMore, moreX, moreY);
    }
}

bool MessageManager::messageIsActive() const
{
    return (activeMessage && state!=mst_inactive);
}
