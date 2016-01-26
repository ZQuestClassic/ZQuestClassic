#include "shopEditor.h"
#include <util.h>
#include <zdefs.h>
#include <zquest.h>
#include "../bitmap/tilePreview.h"
#include <gui/factory.h>
#include <boost/bind.hpp>
#include <cstdio>

extern bool saved;

// This one's still pretty ugly. Needs a better interface for the items.

ShopEditor::ShopEditor(shoptype& s, const item_struct* il):
    shop(s),
    itemList(il)
{
}

#define CB(func) boost::bind(&ShopEditor::func, this)
#define CB1(func) boost::bind(&ShopEditor::func, this, _1)
#define CBID1(func, id) boost::bind(&ShopEditor::func, this, id, _1)

GUI::Widget* ShopEditor::createDialog(const GUI::WidgetFactory& f)
{
    GUI::Window* win=f.window("Edit shop",
      f.column(GUI::Contents(
        f.row(GUI::Contents(
          f.text("Name: "),
          shopName=f.textField(31)
        )),
        f.column(GUI::Contents(
          f.row(GUI::Contents(f.text("Price"), f.text("Item"))),
          f.row(GUI::Contents(
            prices[0]=f.textField(5),
            shopItems[0]=f.comboBox(new GUI::ListData(CB1(getItemText), CB(getNumItems))),
            f.bitmap(itemPreview[0]=new TilePreviewBitmap())
          )),
          f.row(GUI::Contents(
            prices[1]=f.textField(5),
            shopItems[1]=f.comboBox(new GUI::ListData(CB1(getItemText), CB(getNumItems))),
            f.bitmap(itemPreview[1]=new TilePreviewBitmap())
          )),
          f.row(GUI::Contents(
            prices[2]=f.textField(5),
            shopItems[2]=f.comboBox(new GUI::ListData(CB1(getItemText), CB(getNumItems))),
            f.bitmap(itemPreview[2]=new TilePreviewBitmap())
          ))
        )),
        f.buttonRow(GUI::Contents(
          f.button("O&K", CB(onOK)),
          f.button("&Cancel", CB(shutDown))
        ))
      ))
    );
    
    shopName->setText(shop.name);
    for(int i=0; i<3; i++)
    {
        shopItems[i]->setOnValueChanged(CBID1(onItemChanged, i));
        
        if((shop.hasitem[i])!=0)
        {
            int it=0;
            for(int j=0; j<257; j++)
            {
                if(itemList[j].i==shop.item[i])
                {
                    it=j;
                    break;
                }
            }
            char buf[6];
            sprintf(buf, "%d", shop.price[i]);
            prices[i]->setText(buf);
            shopItems[i]->setSelected(it);
            itemPreview[i]->setTile(itemsbuf[shop.item[i]].tile);
            itemPreview[i]->setCSet(itemsbuf[shop.item[i]].csets&15);
        }
        else
        {
            prices[i]->setText("0");
            shopItems[i]->setSelected(0);
        }
    }
    
    return win;
}

void ShopEditor::onItemChanged(int id, int newItem)
{
    int sel=shopItems[id]->getSelected();
    int it=itemList[sel].i;
    itemPreview[id]->setTile(itemsbuf[it].tile);
    itemPreview[id]->setCSet(itemsbuf[it].csets&15);
}

void ShopEditor::onOK()
{
    sprintf(shop.name, "%s", shopName->getText().c_str());
    
    int j=0;
    for(int i=0; i<3; i++)
    {
        int it=shopItems[i]->getSelected();
        if(it==0)
            // Don't make nothing buyable...
            continue;
        shop.hasitem[j]=1;
        shop.price[j]=clamp(atoi(prices[i]->getText().c_str()), 0, 65535);
        shop.item[j]=itemList[it].i;
        j++;
    }
    
    for(; j<3; j++)
    {
        shop.hasitem[j]=0;
        shop.price[j]=0;
        shop.item[j]=0;
    }
    
    saved=false;
    
    shutDown();
}

std::string ShopEditor::getItemText(int id) const
{
    return itemList[id].s;
}

int ShopEditor::getNumItems() const
{
    return 257; // 256 + none
}
