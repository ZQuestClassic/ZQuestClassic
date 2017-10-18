#ifndef _ZC_GUI_ALLEGRO_TEXT_H_
#define _ZC_GUI_ALLEGRO_TEXT_H_

#include "../text.h"
#include "standardWidget.h"
#include <functional>
#include <string>

struct FONT;

namespace GUI
{

class AllegroList;
struct GUIColor;

class AllegroText: public Text, public StandardAllegroWidget
{
public:
    AllegroText(const std::string& text, FONT* font, const GUIColor& color);
    
    inline void setCentered(bool val) { centered=val; }
    
    inline void setList(AllegroList* al) { list=al; }
    inline void setListNum(int num) { listNum=num; }
    inline void setSelected(bool s) { selected=s; }
    
    // Inherited
    void draw(AllegroGUIRenderer& renderer) const;
    void getPreferredSize(int& prefWidth, int& prefHeight);
    void setSizeAndPos(int newX, int newY, int newWidth, int newHeight);
    void setText(const std::string& newText);
    int realize(DIALOG dlgArray[], int index);
    
    static int proc(int msg, DIALOG* d, int c);
    
private:
    std::string text;
    FONT* font;
    const GUIColor& color;
    bool centered;
    AllegroList* list;
    int listNum;
    bool selected;
};

}

#endif

