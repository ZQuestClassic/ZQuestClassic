#ifndef ZC_GUI_PALETTE_FRAME_H_
#define ZC_GUI_PALETTE_FRAME_H_

#include "gui/widget.h"
#include "gui/dialog_ref.h"

int32_t d_cset_proc(int32_t msg,DIALOG *d,int32_t c);

namespace GUI
{

class PaletteFrame: public Widget
{
public:
	PaletteFrame();

	void setBitmap(BITMAP* value);
	void setColorData(byte* value);
	void setPal(PALETTE* value);
	void setCount(uint8_t value);
	void setOnUpdate(std::function<void()> newOnUpdate);
	int32_t getSelection();
	
private:
	BITMAP* bmp;
	byte* ColorData;
	PALETTE* plt;
	uint8_t count;
	DialogRef alDialog;
	std::function<void()> onUpdate;
	
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void calculateSize() override;
	void realize(DialogRunner& runner) override;
	int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
};

}

#endif
