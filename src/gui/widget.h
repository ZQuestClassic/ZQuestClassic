#ifndef ZC_GUI_WIDGET_H
#define ZC_GUI_WIDGET_H

#include "common.h"
#include "dialog_message.h"
#include "dialog_ref.h"
#include "size.h"
#include "base/zc_alleg.h"
#include <any>
#include <memory>
#include <type_traits>
#include <vector>
#include "base/fonts.h"

#define GUI_DEF_FONT get_custom_font(CFONT_DLG)
#define GUI_DEF_FONT_A5 get_custom_font_a5(CFONT_DLG)

#define DEFAULT_PADDING       3_px
#define DEFAULT_PADDING_INT   3
namespace GUI
{

class DialogRunner;

class Widget: public std::enable_shared_from_this<Widget>
{
public:
	Widget() noexcept;
	virtual ~Widget() {}
	virtual bool isTopLevel() const {return false;}

	/* Set the widget's width, overriding its preferred size.
	 * This should not be used by widgets.
	 */
	void overrideWidth(Size newWidth) noexcept;

	/* Set the widget's height, overriding its preferred size.
	 * This should not be used by widgets.
	 */
	void overrideHeight(Size newHeight) noexcept;

	/* Cap the widget's width, if it is above the given width
	 * This should not be used by widgets.
	 */
	void capWidth(Size newWidth) noexcept;

	/* Cap the widget's height, if it is above the given height
	 * This should not be used by widgets.
	 */
	void capHeight(Size newHeight) noexcept;
	
	/* Expand the widget's width, if it is below the given width
	 * This should not be used by widgets.
	 */
	void minWidth(Size newWidth) noexcept;

	/* Expand the widget's height, if it is below the given height
	 * This should not be used by widgets.
	 */
	void minHeight(Size newHeight) noexcept;

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
	
	inline void setLeftPadding(Size size) noexcept
	{
		leftPadding = size.resolve();
	}

	inline void setRightPadding(Size size) noexcept
	{
		rightPadding = size.resolve();
	}

	inline void setTopPadding(Size size) noexcept
	{
		topPadding = size.resolve();
	}

	inline void setBottomPadding(Size size) noexcept
	{
		bottomPadding = size.resolve();
	}

	/* Set the left and right paddings to the same value. */
	void setHPadding(Size size) noexcept;

	/* Set the top and bottom paddings to the same value. */
	void setVPadding(Size size) noexcept;

	/* Set all four paddings to the same value. */
	void setPadding(Size size) noexcept;

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
	
	bool getVisible() const {return !(flags&f_INVISIBLE);}

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
	virtual void calculateSize();

	/* Size and position the widget within the available space.
	 * This is very quick and dirty; all of the sizing and positioning stuff
	 * needs a lot of work.
	 */
	virtual void arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH);

	/* Creates DIALOGs for this widget and any children. */
	virtual void realize(DialogRunner& runner);

	/* This function is called when an event occurs (e.g. a button is clicked
	 * or a list selection is changed). It should send the appropriate message
	 * through the provided function. event is a NewGuiEvent (jwin.h).
	 * If this function returns a D_* value, it will immediately be returned
	 * from the backend proc. If this returns -1, the proc will keep going.
	 * This should not return D_EXIT. Instead, the dialog's message handler
	 * should return true.
	 */
	virtual int32_t onEvent(int32_t event, MessageDispatcher& sendMessage);

	/* Returns the width of the widget. This should only be used by widgets. */
	int32_t getWidth() const { return width; }

	/* Returns the height of the widget. This should only be used by widgets. */
	int32_t getHeight() const { return height; }
	
	/* Returns the width limit of the widget. This should only be used by widgets. */
	int32_t getMaxWidth() const { return maxwidth; }

	/* Returns the height limit of the widget. This should only be used by widgets. */
	int32_t getMaxHeight() const { return maxheight; }

	/* Returns the width limit of the widget. This should only be used by widgets. */
	int32_t getMinWidth() const { return minwidth; }

	/* Returns the height limit of the widget. This should only be used by widgets. */
	int32_t getMinHeight() const { return minheight; }

	/* Returns the width of the widget plus left and right padding.
	 * This should only be used by widgets.
	 */
	int32_t getPaddedWidth() const noexcept { return width+leftPadding+rightPadding; }

	/* Returns the height of the widget plus top and bottom padding.
	 * This should only be used by widgets.
	 */
	int32_t getPaddedHeight() const noexcept { return height+topPadding+bottomPadding; }

	/* Returns the width of the widget plus left and right margins+padding.
	 * This should only be used by widgets.
	 */
	int32_t getTotalWidth() const noexcept { return width+leftPadding+rightPadding+leftMargin+rightMargin; }

	/* Returns the height of the widget plus top and bottom margins+padding.
	 * This should only be used by widgets.
	 */
	int32_t getTotalHeight() const noexcept { return height+topPadding+bottomPadding+topMargin+bottomMargin; }

	/* Called when the widget actually switches from visible to invisible
	* or vice-versa. This should set or unset DIALOGs' D_HIDDEN flag.
	* This is only public so containers can call it on their children.
	*/
	virtual void applyVisibility(bool visible);
	virtual void applyDisabled(bool dis);

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
	void setDisabled(bool disabled) noexcept;

	/* Returns true if this widget should be disabled initially. */
	inline bool getDisabled() const noexcept
	{
		return flags&f_DISABLED;
	}
	
	virtual void updateReadOnly(bool ro){}
	
	/* If this is true, this widget is non-interactable (but NOT greyed-out) 
	 * when the dialog starts.
	 * This does not affect the status if the dialog is open already
	 */
	inline void setReadOnly(bool ro) noexcept
	{
		if(ro)
			flags |= f_READ_ONLY;
		else
			flags &= ~f_READ_ONLY;
		updateReadOnly(ro);
	}

	/* Returns true if this widget should be read-only initially. */
	inline bool getReadOnly() const noexcept
	{
		return flags&f_READ_ONLY;
	}
	
	/* If this is true, a frame proc will be generated around this widget
	 */
	void setFramed(bool framed) noexcept;
	
	//If true, the widget will have no padding/margin
	void setNoPad(bool nopad) noexcept;

	/* Returns true if this widget should be disabled initially. */
	inline bool getFramed() const noexcept
	{
		return flags&f_FRAMED;
	}
	
	/* If this is true, the widget will expand to fill it's parent
	 */
	void setFitParent(bool fit) noexcept;

	/* Returns true if this widget should expand to fill it's parent. */
	inline bool getFitParent() const noexcept
	{
		return flags&f_FIT_PARENT;
	}
	
	/* If true, widget will fit the parent exactly,
	 * and will not be accounted for when sizing grids
	 */
	void setForceFitWid(bool fit) noexcept;
	void setForceFitHei(bool fit) noexcept;

	/* Returns true if this widget should forcibly fit it's parent. */
	inline bool getForceFitWid() const noexcept
	{
		return flags&f_FORCE_FIT_W;
	}
	inline bool getForceFitHei() const noexcept
	{
		return flags&f_FORCE_FIT_H;
	}
	
	void setRowSpan(uint8_t newSpan)
	{
		rowSpan = std::max(uint8_t(1),newSpan);
	}
	void setColSpan(uint8_t newSpan)
	{
		colSpan = std::max(uint8_t(1),newSpan);
	}
	uint8_t getRowSpan() const
	{
		return rowSpan;
	}
	uint8_t getColSpan() const
	{
		return colSpan;
	}
	
	//Sets the text that appears inside the frame, if framed
	void setFrameText(std::string const& newstr);
	
	//Sets the font to use for the widget (overridable)
	virtual void applyFont(FONT* newfont);
	virtual void applyFont_a5(ALLEGRO_FONT* newfont);
	
	//For some reason need this to not be virtual???
	void setFont(FONT* newfont) {applyFont(newfont);}
	void setFont_a5(ALLEGRO_FONT* newfont) {applyFont_a5(newfont);}
	
	template<typename T>
	inline void setUserData(T&& ud)
	{
		// It might be better to have to specify a type, e.g. userData<int32_t>=5.
		// But for now, the use cases are simple.
		userData=std::make_any<std::decay_t<T>>(ud);
	}

	template<typename T>
	inline T getUserData()
	{
		return std::any_cast<T>(userData);
	}

protected:
	inline bool getWidthOverridden() const noexcept {return flags&f_WIDTH_OVERRIDDEN;}
	inline bool getHeightOverridden() const noexcept {return flags&f_HEIGHT_OVERRIDDEN;}
	/* Use this as the return type of message setters so non-integer types
	 * are properly caught.
	 */
	template<typename T>
	using RequireMessage = std::enable_if_t<
			std::is_enum_v<T> || std::is_integral_v<T>
		>;

	int32_t x, y;
	int32_t fgColor, bgColor;
	uint16_t leftMargin, rightMargin, topMargin, bottomMargin;
	uint16_t leftPadding, rightPadding, topPadding, bottomPadding;
	float hAlign, vAlign;
	std::any userData;
	DialogRef frameDialog, frameTextDialog;
	std::string frameText;

	/* Sets the widget's width if it hasn't been overridden. */
	void setPreferredWidth(Size newWidth) noexcept;

	/* Sets the widget's width if it hasn't been overridden. */
	void setPreferredHeight(Size newHeight) noexcept;

	/* Returns a set of flags with which the DIALOG should be initialized.
	 * The widget should add its own
	 */
	int32_t getFlags() const noexcept;
	
	FONT* widgFont;
	ALLEGRO_FONT* widgFont_a5;
	
	/* Returns true if the dialog is running, and thus draws to the screen are permitted */
	bool allowDraw();
	/* Note that the widget has done something requiring redraw */
	void pendDraw();
	void forceDraw();
	/* Returns true if the entire dialog has been realized */
	bool isConstructed();
	
private:
	enum
	{
		f_WIDTH_OVERRIDDEN =  0b00000000001,
		f_HEIGHT_OVERRIDDEN = 0b00000000010,
		f_INVISIBLE =         0b00000000100,
		f_FOCUSED =           0b00000001000,
		f_DISABLED =          0b00000010000,
		f_FRAMED =            0b00000100000,
		f_FIT_PARENT =        0b00001000000,
		f_FORCE_FIT_W =       0b00010000000,
		f_FORCE_FIT_H =       0b00100000000,
		f_READ_ONLY =         0b01000000000,
		f_NO_PAD =            0b10000000000
	};

	int32_t width, height, maxwidth, maxheight, minwidth, minheight;
	uint8_t rowSpan, colSpan;
	DialogRunner *owner;
	uint16_t flags : 10;

	/* The number of containers hiding this widget. Shouldn't be too many,
	 * but there might be, say, a switcher in nested tab containers.
	 */
	uint8_t hideCount: 4;

	static constexpr uint8_t MAX_HIDE_COUNT = 15;
};

class DummyWidget : public Widget
{
public:
	DummyWidget() {}
};

}

#endif
