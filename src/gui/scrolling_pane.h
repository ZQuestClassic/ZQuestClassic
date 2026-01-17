#ifndef ZC_GUI_SCROLLINGPANE_H_
#define ZC_GUI_SCROLLINGPANE_H_

#include "gui/widget.h"
#include "gui/dialog_ref.h"
#include <utility>

struct DIALOG;

namespace GUI
{

/* A simple scrolling pane.
 *
 * Don't ever nest scrolling panes. It will crash.
 */
class ScrollingPane : public Widget
{
public:
	ScrollingPane();
	enum class Mode
	{
		SCROLL_V, SCROLL_H, SCROLL_HV
	};

	/* Sets the widget that will appear in the pane. */
	inline void setContent(std::shared_ptr<Widget> newContent) noexcept
	{
		content = std::move(newContent);
	}
	inline void setTargWidth(Size const& sz) noexcept
	{
		targWid = sz;
	}
	inline void setTargHeight(Size const& sz) noexcept
	{
		targHei = sz;
	}
	inline void setMode(Mode m) noexcept
	{
		mode = m;
	}
	void setPtrX(int32_t* ptr);
	void setPtrY(int32_t* ptr);

private:
	using mousePosFunc = int32_t();


	std::shared_ptr<Widget> content;
	DialogRef alDialog;
	size_t childrenEnd;
	// TODO: This would be better as size_t
	int32_t scrollPosX, scrollPosY, maxScrollPosX, maxScrollPosY, contentWidth, contentHeight;
	mousePosFunc* oldMouseX;
	mousePosFunc* oldMouseY;
	int32_t *scrollptr_x, *scrollptr_y;
	Size targWid, targHei;
	Mode mode;

	bool can_hscroll() const noexcept;
	bool can_vscroll() const noexcept;
	void scroll_x(int32_t amount) noexcept;
	void scroll_y(int32_t amount) noexcept;
	bool scrollToShowChild(int32_t childPos);
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void calculateSize() override;
	void growToTarget();
	void arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH) override;
	void realize(DialogRunner& runner) override;

	static int32_t mouseBreakerProc(int32_t msg, DIALOG* d, int32_t c);
	static int32_t mouseFixerProc(int32_t msg, DIALOG* d, int32_t c);

	friend int32_t scrollProc(int32_t msg, DIALOG* d, int32_t c);
};

}

#endif
