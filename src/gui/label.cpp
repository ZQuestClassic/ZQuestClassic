#include "label.h"
#include "common.h"
#include "dialog.h"
#include "dialog_runner.h"
#include "../jwin.h"
#include "../zquest.h"
#include <utility>

#define FONT sized(nfont, lfont_l)

namespace GUI
{

Label::Label(): text(), maxLines(1), contX(0), contY(0), contW(0), contH(0)
{
	setPreferredHeight(Size::pixels(text_height(FONT)));
}

void Label::setText(std::string newText)
{
	int textW = text_length(FONT, newText.c_str());
	setPreferredWidth(Size::pixels(textW));
	text=std::move(newText);

	if(textW>getWidth() && maxLines>1)
		fitText();

	if(alDialog)
	{
		Widget::arrange(contX, contY, contW, contH);
		alDialog->x = x;
		alDialog->w = getWidth();
	}
}

void Label::setMaxLines(size_t newMax)
{
	assert(newMax>0);
	maxLines = newMax;
}

void Label::applyVisibility(bool visible)
{
	if(alDialog)
	{
		if(visible)
			alDialog->flags &= ~D_HIDDEN;
		else
			alDialog->flags |= D_HIDDEN;
	}
}

void Label::fitText()
{
	// text_length doesn't understand line breaks, so we'll do it ourselves.
	char* data = text.data();
	auto* f = FONT;
	auto* char_length = f->vtable->char_length;
	int actualWidth = getWidth();
	int lastSpace = -1;
	int widthSoFar = 0;
	size_t currentLine = 1;

	for(int i = 0; data[i] && currentLine < maxLines; ++i)
	{
		char c = data[i];
		if(c == '\n')
		{
			data = data+i+1;
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
				data[lastSpace] = '\n';
				data = data+lastSpace+1;
				widthSoFar = 0;
				lastSpace = -1;
				i = -1;
				++currentLine;
			}
		}
	}

	setPreferredHeight(Size::pixels(text_height(FONT)*currentLine));
}

void Label::arrange(int cx, int cy, int cw, int ch)
{
	// Hang on to these in case the text is changed and
	// the label needs repositioned.
	contX = cx;
	contY = cy;
	contW = cw;
	contH = ch;
	Widget::arrange(cx, cy, cw, ch);
}

void Label::realize(DialogRunner& runner)
{
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<jwin_text_proc>,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		0, // key
		getFlags(), // flags
		0, 0, // d1, d2
		text.data(), FONT, nullptr // dp, dp2, dp3
	});
}

}
