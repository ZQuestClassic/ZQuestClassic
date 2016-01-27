// This program is free software; you can redistribute it and/or modify it under the terms of the
// modified version 3 of the GNU General Public License. See License.txt for details.

#include "precompiled.h" //always first
#include "messageStream.h"

MessageStream::MessageStream(const char* text)
{
    reset(text);
}

void MessageStream::reset(const char* text)
{
    rawText=text;
    ptr=0;
    lastNonSpace=-1;
    
    // Messages are padded with spaces, so the end is the last non-space
    // character. It's possible a string control code argument will be
    // interpreted as a space, but that shouldn't cause problems...
    // as long as it's valid. We'll probably want to add validation later.
    for(unsigned int i=0; text[i]!=0; i++)
    {
        if(text[i]!=' ')
            lastNonSpace=i;
    }
}

int MessageStream::getControlCodeArgument()
{
    int arg=rawText[ptr]-1;
    ptr++;
    
    // Three-byte argument?
    if(rawText[ptr]==255)
    {
        arg+=254*rawText[ptr+1];
        ptr+=2;
    }
    
    return arg;
}
