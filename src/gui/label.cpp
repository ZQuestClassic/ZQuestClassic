#include "label.h"
#include "common.h"
#include "dialog.h"
#include "dialog_runner.h"
#include "../jwin.h"
#include <utility>

extern int32_t zq_screen_w, zq_screen_h;

namespace GUI
{

Label::Label(): text(), text_fit(), maxLines(0), nohline(false),
	contX(0), contY(0), contW(0), contH(0), textAlign(0)
{
	setPreferredHeight(Size::pixels(text_height(widgFont)));
}

void Label::setText(std::string newText)
{
	text=newText;

	fitText();
}

void Label::setMaxLines(size_t newMax)
{
	maxLines = newMax;
}

void Label::setAlign(int32_t ta)
{
	textAlign = vbound(ta,0,2);
	if(alDialog)
	{
		alDialog->d1 = textAlign;
	}
}

void Label::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
}

void Label::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog) alDialog.applyDisabled(dis);
}

void Label::applyFont(FONT* newFont)
{
	if(alDialog)
	{
		alDialog->dp2 = newFont;
	}
	Widget::applyFont(newFont);
	fitText();
}

void Label::fitText()
{
	// text_length doesn't understand line breaks, so we'll do it ourselves.
	text_fit = text;
	char* data = text_fit.data();
	auto* f = widgFont;
	auto* char_length = f->vtable->char_length;
	int32_t actualWidth = getWidthOverridden() ? getWidth() : getMaxWidth();
	if(actualWidth < 0) actualWidth = zq_screen_w;
	int32_t lastSpace = -1;
	int32_t widthSoFar = 0;
	size_t currentLine = 1;
	int32_t max_width = 0;
	int32_t i;
	for(i = 0; data[i] && (!maxLines || currentLine < maxLines); ++i)
	{
		char c = data[i];
		if(c == '\n')
		{
			data = data+i+1;
			if(widthSoFar > max_width)
				max_width = widthSoFar;
			widthSoFar = 0;
			lastSpace = -1;
			i = -1;
			++currentLine;
			continue;
		}
		else if(c == ' ')
			lastSpace = i;
		widthSoFar += char_length(f, c);
		if(widthSoFar > actualWidth)
		{
			// Line's too long; try to put replace the last space with
			// a line break. If there hasn't been one, we'll just
			// keep trying until there's a space.
			if(lastSpace >= 0)
			{
				if(actualWidth > max_width)
					max_width = actualWidth;
				widthSoFar = 0;
				data[lastSpace] = '\n';
				data = data+lastSpace+1;
				lastSpace = -1;
				i = -1;
				++currentLine;
				continue;
			}
			if(widthSoFar > max_width)
				max_width = widthSoFar;
		}
	}
	widthSoFar = text_length(f, data);
	if(widthSoFar > max_width)
		max_width = widthSoFar;
	setPreferredHeight(Size::pixels(text_height(widgFont)*currentLine));
	setPreferredWidth(Size::pixels(max_width)+6_px);
	if(alDialog)
	{
		Widget::arrange(contX, contY, contW, contH);
		// alDialog->x = x;
		// alDialog->y = y;
		alDialog->h = getHeight();
		alDialog->w = getWidth();
		alDialog->dp = text_fit.data();
		pendDraw();
	}
}

void Label::arrange(int32_t cx, int32_t cy, int32_t cw, int32_t ch)
{
	// Hang on to these in case the text is changed and
	// the label needs repositioned.
	contX = cx;
	contY = cy;
	contW = cw;
	contH = ch;
	capWidth(Size::pixels(contW-(leftMargin+rightMargin+leftPadding+rightPadding)));
	fitText();
	Widget::arrange(cx, cy, cw, ch);
}

void Label::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<new_text_proc>,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		0, // key
		getFlags(), // flags
		textAlign, nohline ? 1 : 0, // d1, d2
		text_fit.data(), widgFont, nullptr // dp, dp2, dp3
	});
}

}
