#include "infoShopEditor.h"
#include <messageList.h>
#include <util.h>
#include <zdefs.h>
#include <gui/factory.h>
#include <boost/bind.hpp>
#include <cstdio>

InfoShopEditor::InfoShopEditor(infotype& s, const MessageList& ml):
    shop(s),
    msgList(ml)
{
}

#define CB(func) boost::bind(&InfoShopEditor::func, this)
#define MLCB(func) boost::bind(&MessageList::func, msgList)
#define MLCB1(func) boost::bind(&MessageList::func, msgList, _1)

GUI::Widget* InfoShopEditor::createDialog(const GUI::WidgetFactory& f)
{
    GUI::Window* win=f.window("Edit info shop",
      f.column(GUI::Contents(
        f.row(GUI::Contents(
          f.text("Name: "),
          shopName=f.textField(31)
        )),
        f.column(GUI::Contents(
          f.row(GUI::Contents(f.text("Price"), f.text("Message"))),
          f.row(GUI::Contents(
            prices[0]=f.textField(5),
            messages[0]=f.comboBox(new GUI::ListData(MLCB1(getListMessageText), MLCB(getNumMessages)))
          )),
          f.row(GUI::Contents(
            prices[1]=f.textField(5),
            messages[1]=f.comboBox(new GUI::ListData(MLCB1(getListMessageText), MLCB(getNumMessages)))
          )),
          f.row(GUI::Contents(
            prices[2]=f.textField(5),
            messages[2]=f.comboBox(new GUI::ListData(MLCB1(getListMessageText), MLCB(getNumMessages)))
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
        if((shop.str[i])!=0)
        {
            char buf[6];
            sprintf(buf, "%d", shop.price[i]);
            prices[i]->setText(buf);
            messages[i]->setSelected(msgList.getMessageListPos(shop.str[i]));
        }
        else
        {
            prices[i]->setText("0");
            messages[i]->setSelected(0);
        }
    }
    
    return win;
}

void InfoShopEditor::onOK()
{
    sprintf(shop.name, "%s", shopName->getText().c_str());
    
    int j=0;
    for(int i=0; i<3; i++)
    {
        int msg=messages[i]->getSelected();
        if(msg==0)
            // Don't put a blank message in the shop
            continue;
        shop.price[j]=clamp(atoi(prices[i]->getText().c_str()), 0, 65535);
        shop.str[j]=msgList.getMessageNum(msg);
        j++;
    }
    
    for(; j<3; j++)
    {
        shop.price[j]=0;
        shop.str[j]=0;
    }
    
    shutDown();
}
