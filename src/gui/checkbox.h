#ifndef ZC_GUI_CHECKBOX_H
#define ZC_GUI_CHECKBOX_H

#include "gui/widget.h"
#include "gui/dialog_ref.h"

namespace GUI
{
class TextField;
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

	/* Sets a function to be called on toggle. */
	void setOnToggleFunc(std::function<void(bool)> newOnToggleFunc);
	
	template<typename T>
	RequireMessage<T> onToggle(T m)
	{
		message = static_cast<int32_t>(m);
	}

	void calculateSize() override;

protected:
	int32_t message;
private:
	bool checked;
	std::string text;
	boxPlacement placement;
	DialogRef alDialog;
	std::function<void(bool)> onToggleFunc;

	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void realize(DialogRunner& runner) override;
	int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
	void applyFont(FONT* newFont) override;
	friend class TextField;
};

}

#endif
