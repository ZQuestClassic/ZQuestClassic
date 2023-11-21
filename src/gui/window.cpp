#include "gui/window.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/jwin_a5.h"
#include <algorithm>
#include <cassert>
#include <utility>

using std::max, std::shared_ptr;

extern int32_t zq_screen_w, zq_screen_h;

namespace GUI
{

Window::Window(): content(nullptr), title(""), closeMessage(-1), use_vsync(false),
	inner_menu(nullptr)
{
	helptext = "";
	setPadding(0_px);
	capWidth(Size::pixels(zq_screen_w));
	capHeight(Size::pixels(zq_screen_h));
}

void Window::setTitle(std::string newTitle)
{
	title = std::move(newTitle);
	if(alDialog)
	{
		alDialog->dp = title.data();
		pendDraw();
	}
}

void Window::setHelp(std::string newHelp)
{
	helptext = std::move(newHelp);
	if(alDialog)
	{
		if(helptext[0])
			alDialog->dp3 = helptext.data();
		else alDialog->dp3 = nullptr;
		pendDraw();
	}
}

void Window::setContent(shared_ptr<Widget> newContent) noexcept
{
	content = std::move(newContent);
}
void Window::setMenu(shared_ptr<Menu> menu) noexcept
{
	inner_menu = menu;
}
void Window::setMenu(TopMenu* menu) noexcept
{
	if(!inner_menu)
		inner_menu = std::make_shared<Menu>();
	inner_menu->setMenu(menu);
}

void Window::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
	if(content)
		content->applyVisibility(visible);
	if(inner_menu)
		inner_menu->applyVisibility(visible);
}

void Window::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog) alDialog.applyDisabled(dis);
	if(content)
		content->setDisabled(dis);
	if(inner_menu)
		inner_menu->setDisabled(dis);
}

void Window::applyFont(FONT* newFont)
{
	if(alDialog)
	{
		alDialog->dp2 = newFont;
	}
	Widget::applyFont(newFont);
}

void Window::calculateSize()
{
	GuiMenu* submenu = nullptr;
	if(inner_menu)
		submenu = inner_menu->getMenu();
	if(content)
	{
		// Sized to content plus a bit of padding and space for the title bar.
		content->calculateSize();
		size_t wids[] = {
			content->getTotalWidth()+8u,
			text_length(get_zc_font(font_lfont), title.c_str())+40u,
			submenu ? submenu->width() : 0
		};
		size_t maxwid = 0;
		for(auto wid : wids)
			if(wid > maxwid)
				maxwid = wid;
		setPreferredWidth(Size::pixels(maxwid));
		size_t height = content->getTotalHeight();
		height += 30;
		if(submenu)
			height += submenu->height();
		setPreferredHeight(Size::pixels(height));
	}
	else
	{
		// No content, so whatever.
		setPreferredWidth(320_px);
		setPreferredHeight(240_px);
	}
	Widget::calculateSize();
}

void Window::arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH)
{
	// For now, at least, we're assuming everything will fit...
	Widget::arrange(contX, contY, contW, contH);
	uint left = 6, right = 6, top = 20, bottom = 2;
	if(inner_menu)
	{
		auto h = inner_menu->getHeight();
		inner_menu->arrange(x+left, y+top, getWidth()-(left+right), h);
		top += h;
	}
	top += 8;
	if(content)
		content->arrange(x+left, y+top, getWidth()-(left+right), getHeight()-(top+bottom));
}

void Window::realize(DialogRunner& runner)
{
	setFramed(false); //don't allow frame on window proc
	Widget::realize(runner);
	
	alDialog = runner.push(shared_from_this(), DIALOG {
		jwin_win_proc,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		0, // key
		getFlags()|(closeMessage >= 0 ? D_EXIT : 0), // flags,
		0, 0, // d1, d2
		title.data(), get_custom_font(CFONT_TITLE), (helptext[0] ? helptext.data() : nullptr) // dp, dp2, dp3
	});
	
	if(inner_menu)
		inner_menu->realize(runner);

	if(content)
		content->realize(runner);
	
	if(use_vsync || inner_menu)
	{
		void* tfunc = onTick ? ((void*)&onTick) : nullptr;
		runner.push(shared_from_this(), DIALOG {
			d_vsync_proc,
			0, 0, 0, 0,
			0, 0,
			0, // key
			D_NEW_GUI, // flags,
			0, 0, // d1, d2
			tfunc, nullptr, nullptr // dp, dp2, dp3
		});
	}
	
	realizeKeys(runner);
}

int32_t Window::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	if(event == geCLOSE)
	{
		if(closeMessage >= 0)
			sendMessage(closeMessage, MessageArg::none);
		return -1;
	}

	return TopLevelWidget::onEvent(event, sendMessage);
}

void Window::setOnTick(std::function<int32_t()> newOnTick)
{
	onTick = std::move(newOnTick);
}
void Window::setOnLoad(std::function<void()> newOnLoad)
{
	onLoad = std::move(newOnLoad);
}
void Window::load()
{
	if(onLoad)
		onLoad();
}

}
