#ifndef ZC_GUI_MISC_COLOR_ROW_H_
#define ZC_GUI_MISC_COLOR_ROW_H_

#include "gui/widget.h"
#include <vector>
#include "gui/dialog_ref.h"

namespace GUI
{

class MiscColorRow: public Widget
{
public:
	MiscColorRow();
	
	void setVal(int32_t newVal);
	int32_t getVal();
	void setCS(int32_t newCS);
	void setSys(bool val) {isSys = val;}
	void setOnUpdate(std::function<void(int32_t)> newOnUpdate);

private:
	DialogRef alDialog;
	std::function<void(int32_t)> onUpdate;
	
	int32_t val, cs;
	bool isSys;
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void calculateSize() override;
	void realize(DialogRunner& runner) override;
	int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
};

}

#endif
