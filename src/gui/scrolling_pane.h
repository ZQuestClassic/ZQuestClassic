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
	using mousePosFunc=int();


	std::shared_ptr<Widget> content;
	DialogRef alDialog;
	size_t childrenEnd;
	int scrollPos, maxScrollPos, contentHeight;
	mousePosFunc* oldMouseX;
	mousePosFunc* oldMouseY;

	void scroll(int amount) noexcept;
	bool scrollToShowChild(int childPos);
	void applyVisibility(bool visible) override;
	void calculateSize() override;
	void arrange(int contX, int contY, int contW, int contH) override;
	void realize(DialogRunner& runner) override;

	static int mouseBreakerProc(int msg, DIALOG* d, int c);
	static int mouseFixerProc(int msg, DIALOG* d, int c);

	friend int scrollProc(int msg, DIALOG* d, int c);
};

}

#endif
