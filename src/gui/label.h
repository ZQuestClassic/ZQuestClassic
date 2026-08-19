#ifndef ZC_GUI_LABEL_H_
#define ZC_GUI_LABEL_H_

#include "gui/widget.h"
#include "gui/dialog_ref.h"
#include <string>

namespace GUI
{

class Label: public Widget
{
public:
	Label();

	/* Sets the text to display. */
	void setText(std::string newText);
	std::string const& getText() const
	{
		return text;
	}

	/* Sets the maximum number of lines the text can be split into. */
	void setMaxLines(size_t newMax);
	
	void setAlign(int32_t ta);
	
	void setNoHLine(bool noHLine)
	{
		nohline = noHLine;
	}

	/* Draws the text in the given palette color instead of the scheme's
	 * standard text color. */
	void setTextColor(int32_t color);

	/* Sets a function to be called when the label is clicked. Labels
	 * without one ignore clicks entirely. */
	void setOnPress(GUI::function<void()> newOnPress);

	void calculateSize() override;

private:
	std::string text;
	std::string text_fit;
	size_t maxLines;
	DialogRef alDialog;
	int32_t contX, contY, contW, contH;
	int32_t textAlign;
	int32_t textColor;
	bool nohline;
	GUI::function<void()> onPress;

	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH) override;
	void realize(DialogRunner& runner) override;
	int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
	void applyFont(FONT* newFont) override;

	/* Inserts line breaks into the text if it's longer than
	 * the alloted width.
	 */
	void fitText();

	/* The flag bits passed to new_text_proc through d2. */
	int32_t procFlags() const;
};

}

#endif
