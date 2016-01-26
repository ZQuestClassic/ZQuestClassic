#ifndef _ZC_MESSAGELIST_H_
#define _ZC_MESSAGELIST_H_

#include <string>

// A class to simplify message number lookups.
class MessageList
{
public:
    int getNumMessages() const;
    
    /// Get a ID of the message at the given list position.
    int getMessageNum(int listPos) const;
    
    /// Get the list position of a message based on its actual ID number.
    int getMessageListPos(int msgNum) const;
    
    /// Get the text of a message.
    std::string getMessageText(int msgNum) const;
    
    /// Get the text of a message as displayed in a list.
    std::string getListMessageText(int listPos) const;
};

#endif
