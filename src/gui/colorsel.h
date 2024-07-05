#ifndef ZC_GUI_COLORSEL_H_
#define ZC_GUI_COLORSEL_H_

#include "gui/widget.h"
#include "gui/dialog_ref.h"
#include <memory>
#include <string_view>

namespace GUI
{

class ColorSel: public Widget
{
public:
	
	ColorSel();
	
	void setVal(byte val);
	byte getVal();
	
	/* Sets the message to send whenever the text changes. Like onEnter,
	 * the type of the argument varies depending on the text field's type.
	 */
	template<typename T>
	RequireMessage<T> onValueChanged(T m)
	{
		onValueChangedMsg = static_cast<int32_t>(m);
	}

	/* Sets a function to be called on value change. */
	void setOnValChanged(std::function<void(byte)> newOnValChanged);
	
private:
	DialogRef alDialog;
	int32_t onValueChangedMsg;
	byte colorVal;
	std::function<void(byte)> onValChanged;

	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void realize(DialogRunner& runner) override;
	int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
};

}

#endif
