#include "gui/widget.h"
#include "gui/common.h"
#include "gui/dialog_runner.h"
#include "gui/jwin.h"
#include "base/zdefs.h"
#include <cassert>

namespace GUI
{

Widget::Widget() noexcept:
	x(0), y(0),
	fgColor(vc(14)), bgColor(vc(1)),
	leftMargin(0), rightMargin(0),
	topMargin(0), bottomMargin(0),
	leftPadding(DEFAULT_PADDING_INT), rightPadding(DEFAULT_PADDING_INT),
	topPadding(DEFAULT_PADDING_INT), bottomPadding(DEFAULT_PADDING_INT),
	hAlign(0.5), vAlign(0.5),
	frameText(""), widgFont(GUI_DEF_FONT),
	width(0), height(0),
	maxwidth(-1), maxheight(-1),
	minwidth(-1), minheight(-1),
	rowSpan(1), colSpan(1),
	owner(NULL), flags(0), hideCount(0)
{}

void Widget::overrideWidth(Size newWidth) noexcept
{
	flags |= f_WIDTH_OVERRIDDEN;
	width = newWidth.resolve();
	if(maxwidth > width)
		maxwidth = width;
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

void Widget::minWidth(Size newWidth) noexcept
{
	minwidth = newWidth.resolve();
	if(width < minwidth) width = minwidth;
}

void Widget::minHeight(Size newHeight) noexcept
{
	minheight = newHeight.resolve();
	if(height < minheight) height = minheight;
}

void Widget::setPreferredWidth(Size newWidth) noexcept
{
	if(flags&f_WIDTH_OVERRIDDEN) return;
	
	width = newWidth.resolve();
	if(maxwidth > -1 && width > maxwidth)
		width = maxwidth;
	if(minwidth > -1 && width < minwidth)
		width = minwidth;
}

void Widget::setPreferredHeight(Size newHeight) noexcept
{
	if(flags&f_HEIGHT_OVERRIDDEN) return;
	
	height = newHeight.resolve();
	if(maxheight > -1 && height > maxheight)
		height = maxheight;
	if(minheight > -1 && height < minheight)
		height = minheight;
}

void Widget::setHMargins(Size size) noexcept
{
	if(flags&f_NO_PAD) return;
	leftMargin = size.resolve();
	rightMargin = size.resolve();
}

void Widget::setVMargins(Size size) noexcept
{
	if(flags&f_NO_PAD) return;
	topMargin = size.resolve();
	bottomMargin = size.resolve();
}

void Widget::setMargins(Size size) noexcept
{
	if(flags&f_NO_PAD) return;
	leftMargin = size.resolve();
	rightMargin = size.resolve();
	topMargin = size.resolve();
	bottomMargin = size.resolve();
}

void Widget::setHPadding(Size size) noexcept
{
	if(flags&f_NO_PAD) return;
	leftPadding = size.resolve();
	rightPadding = size.resolve();
}

void Widget::setVPadding(Size size) noexcept
{
	if(flags&f_NO_PAD) return;
	topPadding = size.resolve();
	bottomPadding = size.resolve();
}

void Widget::setPadding(Size size) noexcept
{
	if(flags&f_NO_PAD) return;
	leftPadding = size.resolve();
	rightPadding = size.resolve();
	topPadding = size.resolve();
	bottomPadding = size.resolve();
}

void Widget::applyVisibility(bool visible)
{
	if(frameDialog) frameDialog.applyVisibility(visible);
	if(frameTextDialog) frameTextDialog.applyVisibility(visible);
	pendDraw();
}

void Widget::applyDisabled(bool dis)
{
	if(frameDialog) frameDialog.applyDisabled(dis);
	if(frameTextDialog) frameTextDialog.applyDisabled(dis);
	pendDraw();
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

void Widget::calculateSize()
{
	setPreferredWidth(Size::pixels(width));
	setPreferredHeight(Size::pixels(height));
	if(frameText.size())
	{
		int sz = 1+(0.5_em).resolve();
		if(topMargin < sz)
			topMargin = sz;
		if(topPadding < sz)
			topPadding = sz;
	}
}

int32_t Widget::onEvent(int32_t, MessageDispatcher&)
{
	assert(false); // This function should have been overridden
	return -1;
}

void Widget::arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH)
{
	contX += leftMargin+leftPadding;
	contW -= leftMargin+rightMargin+leftPadding+rightPadding;
	contY += topMargin+topPadding;
	contH -= topMargin+bottomMargin+topPadding+bottomPadding;
	auto origw = contW, origh = contH;
	if(maxwidth > -1 && contW > maxwidth)
		contW = maxwidth;
	if(maxheight > -1 && contH > maxheight)
		contH = maxheight;
	int32_t forceW = contW, forceH = contH;
	if(minwidth > -1 && forceW < minwidth)
		forceW = minwidth;
	if(minheight > -1 && forceH < minheight)
		forceH = minheight;
	if(flags & f_FORCE_FIT_W)
	{
		overrideWidth(Size::pixels(forceW));
	}
	if(flags & f_FORCE_FIT_H)
	{
		overrideHeight(Size::pixels(forceH));
	}
	if(flags&f_FIT_PARENT)
	{
		setPreferredWidth(Size::pixels(contW));
		setPreferredHeight(Size::pixels(contH));
	}
	
	if(width > contW)
		width = contW;
	if(height > contH)
		height = contH;
	
	if(isTopLevel())
	{
		origw = contW;
		origh = contH;
	}
	
	auto hExcess = origw-width;
	x = contX+hExcess*hAlign;
	auto vExcess = origh-height;
	y = contY+vExcess*vAlign;
	if(frameDialog)
	{
		frameDialog->x = x-leftPadding;
		frameDialog->y = y-topPadding;
		frameDialog->w = getPaddedWidth();
		frameDialog->h = getPaddedHeight();
		frameTextDialog->x = x-leftPadding+6_px;
		frameTextDialog->y = y-topPadding-(text_height(widgFont)/2);
		frameTextDialog->w = getPaddedWidth()-6_px;
		frameTextDialog->h = text_height(widgFont);
	}
}

void Widget::realize(DialogRunner& runner)
{
	owner = &runner;
	if(flags&f_FRAMED)
	{
		frameDialog = runner.push(shared_from_this(), DIALOG {
			newGUIProc<jwin_frame_proc>,
			x-leftPadding, y-topPadding, getPaddedWidth(), getPaddedHeight(),
			fgColor, bgColor,
			0,
			getFlags(),
			FR_ETCHED, 0, // d1, d2,
			nullptr, nullptr, nullptr // dp, dp2, dp3
		});
		frameTextDialog = runner.push(shared_from_this(), DIALOG {
			newGUIProc<new_text_proc>,
			x-leftPadding+6_px, y-topPadding-(text_height(widgFont)/2), getPaddedWidth()-6_px, text_height(widgFont),
			fgColor, bgColor,
			0,
			getFlags(),
			0, 0, // d1, d2,
			frameText.data(), widgFont, nullptr // dp, dp2, dp3
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
	applyDisabled(disabled);
}

void Widget::setFramed(bool framed) noexcept
{
	if(framed)
		flags |= f_FRAMED;
	else
		flags &= ~f_FRAMED;
}

void Widget::setNoPad(bool nopad) noexcept
{
	if(nopad)
	{
		flags &= ~f_NO_PAD;
		setPadding(0_px);
		setMargins(0_px);
		flags |= f_NO_PAD;
	}
	else
		flags &= ~f_NO_PAD;
}

void Widget::setFitParent(bool fit) noexcept
{
	if(fit)
		flags |= f_FIT_PARENT;
	else
		flags &= ~f_FIT_PARENT;
}

void Widget::setForceFitWid(bool fit) noexcept
{
	if(fit)
		flags |= f_FORCE_FIT_W;
	else
		flags &= ~f_FORCE_FIT_W;
}

void Widget::setForceFitHei(bool fit) noexcept
{
	if(fit)
		flags |= f_FORCE_FIT_H;
	else
		flags &= ~f_FORCE_FIT_H;
}

void Widget::setFrameText(std::string const& newstr)
{
	frameText = newstr;
	if(frameTextDialog)
	{
		frameTextDialog->dp = frameText.data();
		pendDraw();
	}
	else if(newstr.size())
	{
		int sz = (0.5_em).resolve();
		if(topMargin < sz)
			topMargin = sz;
		if(topPadding < sz)
			topPadding = sz;
	}
}

void Widget::applyFont(FONT* newfont)
{
	widgFont = newfont;
	if(frameTextDialog)
	{
		frameTextDialog->dp2 = widgFont;
		pendDraw();
	}
}

int32_t Widget::getFlags() const noexcept
{
	int32_t ret = D_NEW_GUI;
	if(hideCount > 0 || (flags&f_INVISIBLE) != 0)
		ret |= D_HIDDEN;
	if(flags&f_DISABLED)
		ret |= D_DISABLED;
	if(flags&f_READ_ONLY)
		ret |= D_READONLY;
	return ret;
}

bool Widget::allowDraw()
{
	return owner && owner->allowDraw();
}

void Widget::pendDraw()
{
	if(owner) owner->pendDraw();
}

bool Widget::isConstructed()
{
	return owner && owner->isConstructed();
}

}
