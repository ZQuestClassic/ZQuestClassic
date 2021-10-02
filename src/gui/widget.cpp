#include "widget.h"
#include "common.h"
#include "../jwin.h"
#include "../zdefs.h"
#include <cassert>

namespace GUI
{

Widget::Widget() noexcept:
	x(0), y(0),
	fgColor(vc(14)), bgColor(vc(1)),
	leftMargin(sized(2, 3)), rightMargin(sized(2, 3)),
	topMargin(sized(2, 3)), bottomMargin(sized(2, 3)),
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
	contX += leftMargin;
	contW -= leftMargin+rightMargin;
	contY += topMargin;
	contH -= topMargin+bottomMargin;

	if(width > contW)
		width = contW;
	if(height > contH)
		height = contH;

	auto hExcess = contW-width;
	x = contX+hExcess*hAlign;
	auto vExcess = contH-height;
	y = contY+vExcess*vAlign;
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
