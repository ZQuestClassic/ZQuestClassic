#ifndef _ZC_DIALOG_ZQUEST_SIMPLELISTSELECTOR_H_
#define _ZC_DIALOG_ZQUEST_SIMPLELISTSELECTOR_H_

#include <gui/dialog.h>
#include <boost/function.hpp>
#include <string>

namespace GUI
{
    class List;
}

class SimpleListSelector: public GUI::Dialog
{
public:
    SimpleListSelector(const std::string& title,
      const boost::function<std::string(int)>& stringFunc, int numItems,
      bool editMode, int initialSelection);
    SimpleListSelector(const std::string& title,
      const boost::function<std::string(int)>& stringFunc, int numItems,
      const boost::function<void(int)>& helpFunc, bool editMode,
      int initialSelection);
    GUI::Widget* createDialog(const GUI::WidgetFactory& f);
    
    /// Get the item selected before the dialog was closed.
    /** Returns -1 if nothing was selected. */
    inline int getSelected() const { return selected; }
    
    /// Set the initial selection when the dialog opens.
    /** Mainly useful when closing and opening it repeatedly. */
    inline void setInitialSelection(int sel) { initialSelection=sel; }
    
private:
    const bool editMode; // Show Edit/Done rather than OK/Cancel
    const std::string title;
    const boost::function<std::string(int)> stringFunc;
    const boost::function<void(int)> helpFunc;
    const int numItems;
    int selected;
    int initialSelection;
    GUI::List* list;
    
    void onDoubleClick(int item);
    void onOK(); // Or edit
    void onCancel(); // Or done
    void onHelp();
    int getNumItems() const;
};

#endif
