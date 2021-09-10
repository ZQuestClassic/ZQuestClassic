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
	flags(0)
{}

void Widget::overrideWidth(Size newWidth) noexcept
{
	flags|=f_widthOverridden;
	width=newWidth;
}

void Widget::overrideHeight(Size newHeight) noexcept
{
	flags|=f_heightOverridden;
	height=newHeight;
}

void Widget::setPreferredWidth(Size newWidth) noexcept
{
	if((flags&f_widthOverridden)==0)
		width=newWidth;
}

void Widget::setPreferredHeight(Size newHeight) noexcept
{
	if((flags&f_heightOverridden)==0)
		height=newHeight;
}

void Widget::setVisible(bool visible)
{
	// XXX This approach may not be good enough. A widget might be hidden
	// for multiple reasons at once. It may be necessary to keep a counter.
	if(visible)
		flags&=~f_hidden;
	else
		flags|=f_hidden;
}

int Widget::onEvent(int, MessageDispatcher)
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

int Widget::getFlags() noexcept
{
	int ret=D_NEW_GUI;
	if(flags&f_hidden)
		ret|=D_HIDDEN;
	return ret;
}

}
