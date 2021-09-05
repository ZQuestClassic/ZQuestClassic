#include "common.h"
#include <iostream>
namespace gui
{

int getAccelKey(const std::string_view text)
{
    bool lastWasAmpersand=false;
    for(auto& c: text)
    {
        if(c=='&')
        {
            if(lastWasAmpersand)
                lastWasAmpersand=false;
            else
                lastWasAmpersand=true;
        }
        else if(lastWasAmpersand)
            return c;
    }
    return 0;
}

}
