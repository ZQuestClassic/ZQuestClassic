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

private:
	std::string text;
	size_t maxLines;
	DialogRef alDialog;
	int contX, contY, contW, contH;

	void applyVisibility(bool visible) override;
	void arrange(int contX, int contY, int contW, int contH) override;
	void realize(DialogRunner& runner) override;

	/* Inserts line breaks into the text if it's longer than
	 * the alloted width.
	 */
	void fitText();
};

}

#endif
