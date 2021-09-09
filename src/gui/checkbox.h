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
	void setText(std::string newText);
	void setBoxPlacement(boxPlacement newPlacement);
	void setChecked(bool value);
	bool getChecked();
	void setVisible(bool visible) override;

	template<typename T>
	void onToggle(T m)
	{
		message=static_cast<int>(m);
	}

private:
	bool checked;
	std::string text;
	boxPlacement placement;
	DialogRef alDialog;
	int message;

	void realize(DialogRunner& runner) override;
	int onEvent(int event, MessageDispatcher sendMessage) override;
};

}

#endif
