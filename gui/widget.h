#ifndef _ZC_GUI_WIDGET_H_
#define _ZC_GUI_WIDGET_H_

namespace GUI
{

/// Base class for all GUI widgets.
class Widget
{
public:
    Widget(): userID(0) {}
    virtual ~Widget() {}
    
    /// Set an ID number to identify this widget later.
    inline void setUserID(int id) { userID=id; }
    
    /// Get the previously set ID number.
    inline int getUserID() const { return userID; }
    
private:
    int userID;
};

}

#endif
