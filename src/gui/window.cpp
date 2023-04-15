#include "window.h"
#include "common.h"
#include "dialog.h"
#include "dialog_runner.h"
#include "../jwin_a5.h"
#include <algorithm>
#include <cassert>
#include <utility>

using std::max, std::shared_ptr;

extern int32_t zq_screen_w, zq_screen_h;

namespace GUI
{

Window::Window(): content(nullptr), title(""), closeMessage(-1), use_vsync(false)
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

void Window::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
	if(content)
		content->applyVisibility(visible);
}

void Window::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog) alDialog.applyDisabled(dis);
	if(content)
		content->setDisabled(dis);
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
	if(content)
	{
		// Sized to content plus a bit of padding and space for the title bar.
		content->calculateSize();
		setPreferredWidth(Size::pixels(max(
			content->getTotalWidth()+8,
			text_length(get_zc_font(font_lfont), title.c_str())+40)));
		setPreferredHeight(Size::pixels(content->getTotalHeight()+30));
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
	if(content)
		content->arrange(x+6, y+28, getWidth()-12, getHeight()-30);
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

	if(content)
		content->realize(runner);
	
	if(use_vsync)
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

}
