#ifndef ZC_GUI_LABEL_H
#define ZC_GUI_LABEL_H

#include "widget.h"
#include "dialog_ref.h"
#include <string>

namespace GUI
{

class Label: public Widget
{
public:
	Label();

	/* Sets the text to display. */
	void setText(std::string newText);

	/* Sets the maximum number of lines the text can be split into. */
	void setMaxLines(size_t newMax);
	
	void setAlign(int32_t ta);
	
	void setNoHLine(bool noHLine)
	{
		nohline = noHLine;
	}

private:
	std::string text;
	std::string text_fit;
	size_t maxLines;
	DialogRef alDialog;
	int32_t contX, contY, contW, contH;
	int32_t textAlign;
	bool nohline;

	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH) override;
	void realize(DialogRunner& runner) override;
	void applyFont(FONT* newFont) override;

	/* Inserts line breaks into the text if it's longer than
	 * the alloted width.
	 */
	void fitText();
};

}

#endif
