#ifndef ZC_GUI_LABEL_H
#define ZC_GUI_LABEL_H

#include "widget.h"
#include "dialogRef.h"
#include <string>

namespace gui
{

class Label: public Widget
{
public:
	Label();
	void setText(std::string newText);
	void setMaxLines(size_t newMax);
	void setVisible(bool visible) override;

private:
	std::string text;
	size_t maxLines;
	DialogRef alDialog;
	int contX, contY, contW, contH;

	void arrange(int contX, int contY, int contW, int contH) override;
	void realize(DialogRunner& runner) override;

	/* Inserts line breaks into the text if it's longer than
	 * the alloted width.
	 */
	void fitText();
};

}

#endif
