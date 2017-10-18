#ifndef _ZC_GUI_LIST_H_
#define _ZC_GUI_LIST_H_

#include "widget.h"
#include <boost/function.hpp>
#include <string>

namespace GUI
{

class List: public Widget
{
public:
    /// Set a function to be called when a different item is selected.
    virtual void setOnValueChanged(const boost::function<void(int)>& func)=0;
    
    /// Set a function to be called when an item is double-clicked.
    // Doesn't make sense for combo boxes; maybe those should
    // use a different interface.
    virtual void setOnDoubleClick(const boost::function<void(int)>& func) {}
    
    /// Set the selected item.
    virtual void setSelected(int index)=0;
    
    /// Get the current selection.
    virtual int getSelected() const=0;
};

class ListData
{
public:
    ListData(const boost::function<std::string(int)>& strF,
      const boost::function<int()>& sizeF): strFunc(strF), sizeFunc(sizeF) {}
    inline std::string get(int index) const { return strFunc(index); }
    inline int size() const { return sizeFunc(); }
    
private:
    boost::function<std::string(int)> strFunc;
    boost::function<int()> sizeFunc;
};

}

#endif


