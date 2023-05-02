#ifndef ZC_GUI_FRAME_H
#define ZC_GUI_FRAME_H

#include "widget.h"
#include "dialog_ref.h"
#include <string>
#include <utility>

namespace GUI
{

class Frame: public Widget
{
public:
	// Ordered to match the old jwin enum
	enum class style
	{
		RAISED, RAISED_2, SUNKEN_2, SUNKEN, THICK_LINE, ETCHED, LINE
	};

	Frame();

	/* Sets the frame's title. */
	void setTitle(const std::string& newTitle);

	/* Sets the frame's style. */
	inline void setStyle(style newStyle)
	{
		frameStyle = newStyle;
	}

	/* Sets the widget that will appear in the frame. */
	void setContent(std::shared_ptr<Widget> newContent) noexcept
	{
		content = std::move(newContent);
	}

	/* Sets the maximum number of lines the text can be split into. */
	void setMaxLines(size_t newMax);

private:
	std::shared_ptr<Widget> content;
	style frameStyle;
	std::string title;
	DialogRef alDialog, titleDlg;

	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void calculateSize() override;
	void arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH) override;
	void realize(DialogRunner& runner) override;
};

}

#endif
