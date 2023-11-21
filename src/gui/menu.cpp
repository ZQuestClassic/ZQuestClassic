#include "gui/menu.h"
#include "base/zc_alleg.h"
#include "gui/dialog_runner.h"
#include <utility>

namespace GUI
{

Menu::Menu(): inner_menu(nullptr)
{
	hAlign = 0.0;
}

void Menu::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog)
		alDialog.applyVisibility(visible);
}

void Menu::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog)
		alDialog.applyDisabled(dis);
}

void Menu::calculateSize()
{
	if(inner_menu)
	{
		setPreferredWidth(Size::pixels(inner_menu->width()));
		setPreferredHeight(Size::pixels(inner_menu->height()));
	}
	Widget::calculateSize();
}

void Menu::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<GuiMenu::proc>,
		x, y, getWidth(), getHeight(),
		0, 0,
		0,
		getFlags(),
		0, 0, // d1, d2
		inner_menu, widgFont, nullptr // dp, dp2, dp3
	});
}

}
