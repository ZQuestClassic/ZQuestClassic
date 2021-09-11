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
	hPadding(sized(2, 3)), vPadding(sized(2, 3)),
	hAlign(0.5), vAlign(0.5),
	width(0), height(0),
	flags(0),
	hideCount(0)
{}

void Widget::overrideWidth(Size newWidth) noexcept
{
	flags|=f_WIDTH_OVERRIDDEN;
	width=newWidth;
}

void Widget::overrideHeight(Size newHeight) noexcept
{
	flags|=f_HEIGHT_OVERRIDDEN;
	height=newHeight;
}

void Widget::setPreferredWidth(Size newWidth) noexcept
{
	if((flags&f_WIDTH_OVERRIDDEN)==0)
		width=newWidth;
}

void Widget::setPreferredHeight(Size newHeight) noexcept
{
	if((flags&f_HEIGHT_OVERRIDDEN)==0)
		height=newHeight;
}

void Widget::setVisible(bool visible)
{
	bool wasVisible=(flags&f_INVISIBLE)==0;
	if(wasVisible!=visible)
	{
		flags^=f_INVISIBLE;
		if(hideCount==0)
			applyVisibility(visible);
	}
}

void Widget::setExposed(bool exposed)
{
	if(exposed)
	{
		assert(hideCount>0);
		if(hideCount==1 && (flags&f_INVISIBLE)==0)
			applyVisibility(true);
		hideCount--;
	}
	else
	{
		assert(hideCount<MAX_HIDE_COUNT);
		if(hideCount==0 && (flags&f_INVISIBLE)==0)
			applyVisibility(false);
		hideCount++;
	}
}

int Widget::onEvent(int, MessageDispatcher&)
{
	assert(false); // This function should have been overridden
	return -1;
}

void Widget::arrange(int contX, int contY, int contW, int contH)
{
	if(width>contW)
		width=contW;
	if(height>contH)
		height=contH;

	auto hExcess=contW-width;
	x=contX+hExcess*hAlign;
	auto vExcess=contH-height;
	y=contY+vExcess*vAlign;
}

int Widget::getFlags() const noexcept
{
	int ret=D_NEW_GUI;
	if(hideCount>0 || (flags&f_INVISIBLE)!=0)
		ret|=D_HIDDEN;
	return ret;
}

}
