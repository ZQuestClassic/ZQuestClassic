#ifndef ZC_GUI_CORNERSELECT_H_
#define ZC_GUI_CORNERSELECT_H_

#include "gui/widget.h"
#include "gui/dialog_ref.h"

namespace GUI
{

class CornerSwatch: public Widget
{
public:
	CornerSwatch();
	
	void setVal(int32_t value);
	void setColor(int32_t value);
	
	int32_t getVal();
	int32_t getColor();

	void setOnSelectFunc(std::function<void(int32_t)> newOnSelect)
	{
		onSelectFunc = newOnSelect;
	}
	
	template<typename T>
	RequireMessage<T> onSelectionChanged(T m)
	{
		message = static_cast<int32_t>(m);
	}
protected:
	int32_t message;
private:
	int32_t val;
	DialogRef alDialog;
	std::function<void(int32_t)> onSelectFunc;
	
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void realize(DialogRunner& runner) override;
	void calculateSize() override;
	int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
};

}

#endif
