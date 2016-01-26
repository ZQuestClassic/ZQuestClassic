#ifndef _ZC_DIALOG_ZQUEST_INFOSHOPEDITOR_H_
#define _ZC_DIALOG_ZQUEST_INFOSHOPEDITOR_H_


#include <gui/dialog.h>

struct infotype;
class MessageList;
namespace GUI
{
    class TextField;
    class List;
}

// Edits info shops.
class InfoShopEditor: public GUI::Dialog
{
public:
    InfoShopEditor(infotype& shop, const MessageList& msgList);
    GUI::Widget* createDialog(const GUI::WidgetFactory& f);
    
private:
    infotype& shop;
    GUI::TextField* shopName;
    GUI::TextField* prices[3];
    GUI::List* messages[3];
    const MessageList& msgList;
    
    void onOK();
};

#endif
