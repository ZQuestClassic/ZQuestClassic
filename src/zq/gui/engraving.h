#ifndef ZC_GUI_ENGRAVING_H_
#define ZC_GUI_ENGRAVING_H_

#include "gui/widget.h"
#include "gui/dialog_ref.h"

namespace GUI
{

class Engraving: public Widget
{
public:
	Engraving();
	Engraving(int32_t useslot);

	void setSlot(int32_t newslot)
	{
		slot = newslot;
		drawx = 1 + (slot % 16) * 17;
		drawy = 1 + (slot / 16) * 17;
		pendDraw();
	}
	int32_t getSlot()
	{
		return slot;
	}
	
private:
	int32_t slot;
	int32_t drawx, drawy;
	DialogRef alDialog;
	
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void realize(DialogRunner& runner) override;
};

}

#endif
