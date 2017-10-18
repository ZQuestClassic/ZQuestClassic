#include "messageList.h"
#include "types.h"
#include "zdefs.h"
#include <stdio.h>

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

extern word msg_count;
extern MsgStr* MsgStrings;

int MessageList::getNumMessages() const
{
    return msg_count;
}

int MessageList::getMessageNum(int listPos) const
{
    for(int i=0; i<msg_count; i++)
    {
        if(MsgStrings[i].listpos==listPos)
            return i;
    }
    
    return 0;
}

int MessageList::getMessageListPos(int msgNum) const
{
    return MsgStrings[msgNum].listpos;
}

std::string MessageList::getMessageText(int msgNum) const
{
    return MsgStrings[msgNum].s;
}

std::string MessageList::getListMessageText(int listPos) const
{
    // TODO: Make sure the indent calculation is correct when
    // listPos and/or msgNum is 0 or 1
    int msgNum=getMessageNum(listPos);
    int prevMsgNum=listPos>0 ? getMessageNum(listPos-1) : -1;
    char buf[128];
    bool indent=msgNum>0 && MsgStrings[prevMsgNum].nextstring==msgNum;
    snprintf(buf, 128, "%s%3d: %s", indent ? "--> " : "", msgNum, MsgStrings[msgNum].s);
    buf[127]='\0';
    return buf;
    
}
