#include "widget.h"
#include "common.h"
#include "dialog_runner.h"
#include "../jwin.h"
#include "../zdefs.h"
#include <cassert>

namespace GUI
{

Widget::Widget() noexcept:
	x(0), y(0),
	fgColor(vc(14)), bgColor(vc(1)),
	leftMargin(0), rightMargin(0),
	topMargin(0), bottomMargin(0),
	leftPadding(sized(2, 3)), rightPadding(sized(2, 3)),
	topPadding(sized(2, 3)), bottomPadding(sized(2, 3)),
	hAlign(0.5), vAlign(0.5),
	width(0), height(0),
	maxwidth(-1), maxheight(-1),
	flags(0),
	hideCount(0)
{}

void Widget::overrideWidth(Size newWidth) noexcept
{
	flags |= f_WIDTH_OVERRIDDEN;
	width = newWidth.resolve();
}

void Widget::overrideHeight(Size newHeight) noexcept
{
	flags |= f_HEIGHT_OVERRIDDEN;
	height = newHeight.resolve();
}

void Widget::capWidth(Size newWidth) noexcept
{
	maxwidth = newWidth.resolve();
	if(width > maxwidth) width = maxwidth;
}

void Widget::capHeight(Size newHeight) noexcept
{
	maxheight = newHeight.resolve();
	if(height > maxheight) height = maxheight;
}

void Widget::setPreferredWidth(Size newWidth) noexcept
{
	if(flags&f_WIDTH_OVERRIDDEN) return;
	
	width = newWidth.resolve();
	if(maxwidth > -1 && width > maxwidth)
		width = maxwidth;
}

void Widget::setPreferredHeight(Size newHeight) noexcept
{
	if(flags&f_HEIGHT_OVERRIDDEN) return;
	
	height = newHeight.resolve();
	if(maxheight > -1 && height > maxheight)
		height = maxheight;
}

void Widget::setHMargins(Size size) noexcept
{
	leftMargin = size.resolve();
	rightMargin = size.resolve();
}

void Widget::setVMargins(Size size) noexcept
{
	topMargin = size.resolve();
	bottomMargin = size.resolve();
}

void Widget::setMargins(Size size) noexcept
{
	leftMargin = size.resolve();
	rightMargin = size.resolve();
	topMargin = size.resolve();
	bottomMargin = size.resolve();
}

void Widget::setHPadding(Size size) noexcept
{
	leftPadding = size.resolve();
	rightPadding = size.resolve();
}

void Widget::setVPadding(Size size) noexcept
{
	topPadding = size.resolve();
	bottomPadding = size.resolve();
}

void Widget::setPadding(Size size) noexcept
{
	leftPadding = size.resolve();
	rightPadding = size.resolve();
	topPadding = size.resolve();
	bottomPadding = size.resolve();
}

void Widget::applyVisibility(bool visible)
{
	if(frameDialog) frameDialog.applyVisibility(visible);
}

void Widget::setVisible(bool visible)
{
	bool wasVisible = (flags&f_INVISIBLE) == 0;
	if(wasVisible != visible)
	{
		flags ^= f_INVISIBLE;
		if(hideCount == 0)
			applyVisibility(visible);
	}
}

void Widget::setExposed(bool exposed)
{
	if(exposed)
	{
		assert(hideCount > 0);
		if(hideCount == 1 && (flags&f_INVISIBLE) ==0)
			applyVisibility(true);
		--hideCount;
	}
	else
	{
		assert(hideCount < MAX_HIDE_COUNT);
		if(hideCount == 0 && (flags&f_INVISIBLE) == 0)
			applyVisibility(false);
		++hideCount;
	}
}

int Widget::onEvent(int, MessageDispatcher&)
{
	assert(false); // This function should have been overridden
	return -1;
}

void Widget::arrange(int contX, int contY, int contW, int contH)
{
	if(flags&f_FIT_PARENT)
	{
		setPreferredWidth(Size::pixels(contW));
		setPreferredHeight(Size::pixels(contH));
	}
	contX += leftMargin+leftPadding;
	contW -= leftMargin+rightMargin+leftPadding+rightPadding;
	contY += topMargin+topPadding;
	contH -= topMargin+bottomMargin+topPadding+bottomPadding;

	if(width > contW)
		width = contW;
	if(height > contH)
		height = contH;

	auto hExcess = contW-width;
	x = contX+hExcess*hAlign;
	auto vExcess = contH-height;
	y = contY+vExcess*vAlign;
	if(frameDialog)
	{
		frameDialog->x = x-leftPadding;
		frameDialog->y = y-topPadding;
		frameDialog->w = getPaddedWidth();
		frameDialog->h = getPaddedHeight();
	}
}

void Widget::realize(DialogRunner& runner)
{
	if(flags&f_FRAMED)
	{
		frameDialog = runner.push(shared_from_this(), DIALOG {
			jwin_frame_proc,
			x-leftPadding, y-topPadding, getPaddedWidth(), getPaddedHeight(),
			fgColor, bgColor,
			0,
			getFlags(),
			FR_ETCHED, 0, // d1, d2,
			nullptr, nullptr, nullptr // dp, dp2, dp3
		});
	}
}

void Widget::setFocused(bool focused) noexcept
{
	if(focused)
		flags |= f_FOCUSED;
	else
		// Why even set it? Whatever. Might as well work.
		flags &= ~f_FOCUSED;
}

void Widget::setDisabled(bool disabled) noexcept
{
	if(disabled)
		flags |= f_DISABLED;
	else
		flags &= ~f_DISABLED;
}

void Widget::setFramed(bool framed) noexcept
{
	if(framed)
		flags |= f_FRAMED;
	else
		flags &= ~f_FRAMED;
}

void Widget::setFitParent(bool fit) noexcept
{
	if(fit)
		flags |= f_FIT_PARENT;
	else
		flags &= ~f_FIT_PARENT;
}

int Widget::getFlags() const noexcept
{
	int ret = D_NEW_GUI;
	if(hideCount > 0 || (flags&f_INVISIBLE) != 0)
		ret |= D_HIDDEN;
	if(flags&f_DISABLED)
		ret |= D_DISABLED;
	return ret;
}

}
