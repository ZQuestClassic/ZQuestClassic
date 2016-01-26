#include "contents.h"

namespace GUI
{

Contents::Contents():
    numWidgets(0)
{
}

Contents::Contents(Widget* w1):
    numWidgets(1)
{
    widgets[0]=w1;
}

Contents::Contents(Widget* w1, Widget* w2):
    numWidgets(2)
{
    widgets[0]=w1;
    widgets[1]=w2;
}

Contents::Contents(Widget* w1, Widget* w2, Widget* w3):
    numWidgets(3)
{
    widgets[0]=w1;
    widgets[1]=w2;
    widgets[2]=w3;
}

Contents::Contents(Widget* w1, Widget* w2, Widget* w3, Widget* w4):
    numWidgets(4)
{
    widgets[0]=w1;
    widgets[1]=w2;
    widgets[2]=w3;
    widgets[3]=w4;
}

Contents::Contents(Widget* w1, Widget* w2, Widget* w3, Widget* w4, Widget* w5):
    numWidgets(5)
{
    widgets[0]=w1;
    widgets[1]=w2;
    widgets[2]=w3;
    widgets[3]=w4;
    widgets[4]=w5;
}

Contents::Contents(Widget* w1, Widget* w2, Widget* w3, Widget* w4, Widget* w5,
  Widget* w6):
    numWidgets(6)
{
    widgets[0]=w1;
    widgets[1]=w2;
    widgets[2]=w3;
    widgets[3]=w4;
    widgets[4]=w5;
    widgets[5]=w6;
}

Contents::Contents(Widget* w1, Widget* w2, Widget* w3, Widget* w4, Widget* w5,
  Widget* w6, Widget* w7):
    numWidgets(6)
{
    widgets[0]=w1;
    widgets[1]=w2;
    widgets[2]=w3;
    widgets[3]=w4;
    widgets[4]=w5;
    widgets[5]=w6;
    widgets[6]=w7;
}

}
