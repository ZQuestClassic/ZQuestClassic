#ifndef ZC_GUI_WIDGET_H
#define ZC_GUI_WIDGET_H

#include "common.h"
#include "dialog_message.h"
#include "size.h"
#include "../zc_alleg.h"
#include <memory>
#include <type_traits>
#include <vector>

namespace GUI
{

class DialogRunner;

class Widget: public std::enable_shared_from_this<Widget>
{
public:
	Widget() noexcept;
	virtual ~Widget() {}

	/* Set the widget's width, overriding its preferred size.
	 * This should not be used by widgets.
	 */
	void overrideWidth(Size newWidth) noexcept;

	/* Set the widget's height, overriding its preferred size.
	 * This should not be used by widgets.
	 */
	void overrideHeight(Size newHeight) noexcept;

	/* Cap the widget's width, if it is below the given width
	 * This should not be used by widgets.
	 */
	void capWidth(Size newWidth) noexcept;

	/* Cap the widget's height, if it is below the given height
	 * This should not be used by widgets.
	 */
	void capHeight(Size newHeight) noexcept;

	inline void setLeftMargin(Size size) noexcept
	{
		leftMargin = size.resolve();
	}

	inline void setRightMargin(Size size) noexcept
	{
		rightMargin = size.resolve();
	}

	inline void setTopMargin(Size size) noexcept
	{
		topMargin = size.resolve();
	}

	inline void setBottomMargin(Size size) noexcept
	{
		bottomMargin = size.resolve();
	}

	/* Set the left and right margins to the same value. */
	void setHMargins(Size size) noexcept;

	/* Set the top and bottom margins to the same value. */
	void setVMargins(Size size) noexcept;

	/* Set all four margins to the same value. */
	void setMargins(Size size) noexcept;

	inline void setHAlign(float align) noexcept
	{
		hAlign = align;
	}

	inline void setVAlign(float align) noexcept
	{
		vAlign = align;
	}

	/* Sets this widget to be visible or invisible. This function should
	 * be used by dialogs, e.g. for a warning label shown conditionally.
	 * This simply sets a flag indicating whether the widget is visible.
	 */
	void setVisible(bool visible);

	/* Sets this widget to be visible or invisible. This function should
	 * be called by container widgets, e.g. a tab container hiding its children.
	 * Calls to setExposed() must be balanced. It is an error if a widget is
	 * exposed more times than it's unexposed.
	 */
	void setExposed(bool exposed);

	/* Set the widget's width and height to their preferred values.
	 * This doesn't need to be implemented if they're set already
	 * before the dialog is realized.
	 */
	virtual void calculateSize() {}

	/* Size and position the widget within the available space.
	 * This is very quick and dirty; all of the sizing and positioning stuff
	 * needs a lot of work.
	 */
	virtual void arrange(int contX, int contY, int contW, int contH);

	/* Creates DIALOGs for this widget and any children. */
	virtual void realize(DialogRunner& runner) = 0;

	/* This function is called when an event occurs (e.g. a button is clicked
	 * or a list selection is changed). It should send the appropriate message
	 * through the provided function. event is a NewGuiEvent (jwin.h).
	 * If this function returns a D_* value, it will immediately be returned
	 * from the backend proc. If this returns -1, the proc will keep going.
	 * This should not return D_EXIT. Instead, the dialog's message handler
	 * should return true.
	 */
	virtual int onEvent(int event, MessageDispatcher& sendMessage);

	/* Returns the width of the widget. This should only be used by widgets. */
	int getWidth() const { return width; }

	/* Returns the height of the widget. This should only be used by widgets. */
	int getHeight() const { return height; }

	/* Returns the width of the widget plus left and right margins.
	 * This should only be used by widgets.
	 */
	int getTotalWidth() const noexcept { return width+leftMargin+rightMargin; }

	/* Returns the height of the widget plus top and bottom margins.
	 * This should only be used by widgets.
	 */
	int getTotalHeight() const noexcept { return height+topMargin+bottomMargin; }

	/* Called when the widget actually switches from visible to invisible
	* or vice-versa. This should set or unset DIALOGs' D_HIDDEN flag.
	* This is only public so containers can call it on their children.
	*/
	virtual void applyVisibility(bool visible) = 0;

	/* If this is true, this widget should be focused when the dialog opens.
	 * This does not give the widget focus if the dialog is open already.
	 */
	void setFocused(bool focused) noexcept;

	/* Returns true if this widget should have focus initially. */
	inline bool getFocused() const noexcept
	{
		return flags&f_FOCUSED;
	}
	
	/* If this is true, this widget is 'disabled' (greyed-out) when the dialog starts
	 * This does not affect the status if the dialog is open already
	 */
	void setDisabled(bool focused) noexcept;

	/* Returns true if this widget should be disabled initially. */
	inline bool getDisabled() const noexcept
	{
		return flags&f_DISABLED;
	}

protected:
	/* Use this as the return type of message setters so non-integer types
	 * are properly caught.
	 */
	template<typename T>
	using RequireMessage = std::enable_if_t<
			std::is_enum_v<T> || std::is_integral_v<T>
		>;

	int x, y;
	int fgColor, bgColor;
	unsigned short leftMargin, rightMargin, topMargin, bottomMargin;
	float hAlign, vAlign;

	/* Sets the widget's width if it hasn't been overridden. */
	void setPreferredWidth(Size newWidth) noexcept;

	/* Sets the widget's width if it hasn't been overridden. */
	void setPreferredHeight(Size newHeight) noexcept;

	/* Returns a set of flags with which the DIALOG should be initialized.
	 * The widget should add its own
	 */
	int getFlags() const noexcept;

private:
	enum
	{
		f_WIDTH_OVERRIDDEN =  0b00001,
		f_HEIGHT_OVERRIDDEN = 0b00010,
		f_INVISIBLE =         0b00100,
		f_FOCUSED =           0b01000,
		f_DISABLED =          0b10000
	};

	int width, height, maxwidth, maxheight;
	unsigned char flags: 5;

	/* The number of containers hiding this widget. Shouldn't be too many,
	 * but there might be, say, a switcher in nested tab containers.
	 */
	unsigned char hideCount: 4;

	static constexpr unsigned char MAX_HIDE_COUNT = 15;
};

}

#endif
