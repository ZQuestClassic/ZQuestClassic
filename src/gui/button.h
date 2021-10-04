#ifndef ZC_GUI_BUTTON_H
#define ZC_GUI_BUTTON_H

#include "widget.h"
#include "dialog_ref.h"
#include <string>

namespace GUI
{

class Button: public Widget
{
public:
	Button();

	/* Sets the text to appear on the button. */
	void setText(std::string newText);
	
	/* Sets a function to be called on press. */
	void setOnPress(std::function<void()> newOnPress);

	template<typename T>
	RequireMessage<T> onClick(T m)
	{
		message = static_cast<int>(m);
	}

private:
	std::string text;
	DialogRef alDialog;
	int message;
	std::function<void()> onPress;

	void applyVisibility(bool visible) override;
	void realize(DialogRunner& runner) override;
	int onEvent(int event, MessageDispatcher& sendMessage) override;
};

}

#endif
