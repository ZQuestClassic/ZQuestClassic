#ifndef ZC_GUI_CHECKBOX_H
#define ZC_GUI_CHECKBOX_H

#include "widget.h"
#include "dialog_ref.h"

namespace GUI
{

class Checkbox: public Widget
{
public:
	enum class boxPlacement { RIGHT, LEFT };

	Checkbox();

	/* Sets the text to appear next to the checkbox. */
	void setText(std::string newText);

	/* Sets whether the checkbox is checked or not. */
	void setChecked(bool value);

	/* Returns true if the checkbox is checked. */
	bool getChecked();

	/* Sets whether the box is to the left or right of the text. */
	inline void setBoxPlacement(boxPlacement newPlacement) noexcept
	{
		placement = newPlacement;
	}

	template<typename T>
	RequireMessage<T> onToggle(T m)
	{
		message = static_cast<int>(m);
	}

private:
	bool checked;
	std::string text;
	boxPlacement placement;
	DialogRef alDialog;
	int message;

	void applyVisibility(bool visible) override;
	void realize(DialogRunner& runner) override;
	int onEvent(int event, MessageDispatcher& sendMessage) override;
};

}

#endif
