#include "screenFreezeState.h"
#include "maps.h"
#include "messageManager.h"
#include "tiles.h"
#include "zdefs.h"
#include "zelda.h"


ScreenFreezeState::ScreenFreezeState(const MessageManager& mm):
    messageMgr(mm),
    messageFreeze(false) // The rest will be reset in update()
{
}

void ScreenFreezeState::update()
{
    if(get_bit(quest_rules,qr_MSGFREEZE)!=0)
        messageFreeze = messageMgr.messageIsActive();
    freezeFFCCombo = false;
    freezeNonFFCCombo = false;
    
    for(int i=0; i<32; i++)
    {
        if(combobuf[tmpscr->ffcs[i].getCombo()].type==cSCREENFREEZE)
            freezeNonFFCCombo=true;
        if(combobuf[tmpscr->ffcs[i].getCombo()].type==cSCREENFREEZEFF)
            freezeFFCCombo=true;
    }
    
    if(!(freezeNonFFCCombo && freezeFFCCombo))
    {
        for(int i=0; i<176; i++)
        {
            if(combobuf[tmpscr->data[i]].type == cSCREENFREEZE)
                freezeNonFFCCombo=true;
            if(combobuf[tmpscr->data[i]].type == cSCREENFREEZEFF)
                freezeFFCCombo=true;
        }
    }
}

bool ScreenFreezeState::scriptDrawingFrozen() const
{
    return messageFreeze;
}

bool ScreenFreezeState::ffcsFrozen() const
{
    return messageFreeze || freezeFFCCombo;
}

bool ScreenFreezeState::globalScriptFrozen() const
{
    return messageFreeze;
}

bool ScreenFreezeState::entitiesFrozen() const
{
    return messageFreeze || freezeNonFFCCombo;
}

bool ScreenFreezeState::itemRefillsFrozen() const
{
    return messageFreeze;
}

bool ScreenFreezeState::guysAndFiresFrozen() const
{
    return freezeNonFFCCombo;
}
