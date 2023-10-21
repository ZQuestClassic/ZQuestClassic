#ifndef ZC_GUI_BUTTON_H
#define ZC_GUI_BUTTON_H

#include "base/hotkey.h"
#include "gui/widget.h"
#include "gui/dialog_ref.h"
#include <string>

namespace GUI
{

class Button: public Widget
{
public:
	enum class type
	{
		BASIC, ICON, BIND_KB, BIND_HOTKEY, BIND_KB_CLEAR
	};
	
	Button();
	
	void setType(type newType);
	void setBoundKB(int* kb_ptr);
	void setBoundHotkey(Hotkey* hotkey_ptr);
	void setHotkeyIndx(size_t indx);
	void setIcon(int icon);
	
	/* Sets the text to appear on the button. */
	void setText(std::string newText);
	
	/* Sets a function to be called on press. */
	void setOnPress(std::function<void()> newOnPress);

	template<typename T>
	RequireMessage<T> onClick(T m)
	{
		message = static_cast<int32_t>(m);
	}
	
	/*static std::shared_ptr<Button> infoButton(std::string title, std::string text)
	{
		std::shared_ptr<Button> b = std::make_shared<Button>();
		b->setText("?");
		b->setOnPress([](){InfoDialog(title,text).show();});
		return b;
	}*/
	void calculateSize() override;

private:
	std::string text;
	DialogRef alDialog;
	int32_t message;
	std::function<void()> onPress;
	type btnType;
	
	int* bound_kb;
	Hotkey* bound_hotkey;
	size_t hotkeyindx;
	
	int icontype;
	
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void realize(DialogRunner& runner) override;
	int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
	void applyFont(FONT* newFont) override;
};

}

#define INFO_BUTTON(infotitle, infotext) \
	Button(width = 2_em, leftPadding = 0_px, forceFitH = true, text = "?", \
		onPressFunc = []() \
		{ \
			InfoDialog(infotitle,infotext).show(); \
		}),

#endif
