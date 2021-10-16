#ifndef ZC_GUI_SCROLLINGPANE_H
#define ZC_GUI_SCROLLINGPANE_H

#include "widget.h"
#include "dialog_ref.h"
#include <utility>

struct DIALOG;

namespace GUI
{

/* A simple scrolling pane.
 *
 * Don't ever nest scrolling panes. It will crash.
 */
class ScrollingPane: public Widget
{
public:
	ScrollingPane();

	/* Sets the widget that will appear in the pane. */
	inline void setContent(std::shared_ptr<Widget> newContent) noexcept
	{
		content = std::move(newContent);
	}

private:
	using mousePosFunc=int32_t();


	std::shared_ptr<Widget> content;
	DialogRef alDialog;
	size_t childrenEnd;
	int32_t scrollPos, maxScrollPos, contentHeight;
	mousePosFunc* oldMouseX;
	mousePosFunc* oldMouseY;

	void scroll(int32_t amount) noexcept;
	bool scrollToShowChild(int32_t childPos);
	void applyVisibility(bool visible) override;
	void calculateSize() override;
	void arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH) override;
	void realize(DialogRunner& runner) override;

	static int32_t mouseBreakerProc(int32_t msg, DIALOG* d, int32_t c);
	static int32_t mouseFixerProc(int32_t msg, DIALOG* d, int32_t c);

	friend int32_t scrollProc(int32_t msg, DIALOG* d, int32_t c);
};

}

#endif
