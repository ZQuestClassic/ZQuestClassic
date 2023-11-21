#ifndef ZC_GUI_MENU_H
#define ZC_GUI_MENU_H

#include "gui/widget.h"
#include "base/new_menu.h"
#include <vector>

namespace GUI
{

class Menu: public Widget
{
public:
	Menu();
	void setMenu(GuiMenu* menu) {inner_menu = menu;}
	GuiMenu* getMenu() {return inner_menu;}
	GuiMenu const* getMenu() const {return inner_menu;}
	
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void calculateSize() override;
	void realize(DialogRunner& runner) override;
private:
	GuiMenu* inner_menu;
	DialogRef alDialog;
};

}

#endif
