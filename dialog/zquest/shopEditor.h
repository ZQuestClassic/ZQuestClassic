#ifndef _ZC_DIALOG_ZQUEST_SHOPEDITOR_H_
#define _ZC_DIALOG_ZQUEST_SHOPEDITOR_H_

#include <gui/dialog.h>
#include <string>

struct item_struct;
struct shoptype;
class TilePreviewBitmap;
namespace GUI
{
    class TextField;
    class List;
}

// Edits shops.
class ShopEditor: public GUI::Dialog
{
public:
    ShopEditor(shoptype& shop, const item_struct* itemList);
    GUI::Widget* createDialog(const GUI::WidgetFactory& f);
    
private:
    shoptype& shop;
    const item_struct* itemList;
    GUI::TextField* shopName;
    GUI::TextField* prices[3];
    GUI::List* shopItems[3];
    TilePreviewBitmap* itemPreview[3];
    
    void onItemChanged(int id, int newItem);
    void onOK();
    std::string getItemText(int id) const;
    int getNumItems() const;
};

#endif
