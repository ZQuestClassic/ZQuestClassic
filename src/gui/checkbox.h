#ifndef ZC_GUI_CHECKBOX_H
#define ZC_GUI_CHECKBOX_H

#include "widget.h"
#include "dialogRef.h"

namespace gui
{

class Checkbox: public Widget
{
public:
	enum class BoxPlacement { right, left };

	Checkbox();
	void setText(std::string newText);
	void setBoxPlacement(BoxPlacement bp);
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
	BoxPlacement boxPlacement;
	DialogRef alDialog;
	int message;

	void realize(DialogRunner& runner) override;
	int onEvent(int event, MessageDispatcher sendMessage) override;
};

}

#endif
