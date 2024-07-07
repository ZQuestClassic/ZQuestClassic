#ifndef ZC_GUI_SELCOMBO_SWATCH_H_
#define ZC_GUI_SELCOMBO_SWATCH_H_

#include "gui/widget.h"
#include "gui/dialog_ref.h"

namespace GUI
{

class SelComboSwatch: public Widget
{
public:
	SelComboSwatch();

	void setCombo(int32_t value);
	void setCSet(int32_t value);
	void setShowVals(bool showVals);

	int32_t getCombo();
	int32_t getCSet();

	void setOnSelectFunc(std::function<void(int32_t,int32_t)> newOnSelect)
	{
		onSelectFunc = newOnSelect;
	}
	
	template<typename T>
	RequireMessage<T> onSelectionChanged(T m)
	{
		message = static_cast<int32_t>(m);
	}
	DialogRef alDialog;
protected:
	int32_t message;
private:
	int32_t combo, cset;
	bool showsVals;
	std::function<void(int32_t,int32_t)> onSelectFunc;
	
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void realize(DialogRunner& runner) override;
	void calculateSize() override;
	int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
	void applyFont(FONT* newFont) override;
};

}

#endif
