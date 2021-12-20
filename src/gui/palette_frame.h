#ifndef ZC_GUI_PALETTE_FRAME_H
#define ZC_GUI_PALETTE_FRAME_H

#include "widget.h"
#include "dialog_ref.h"

int32_t d_cset_proc(int32_t msg,DIALOG *d,int32_t c);

namespace GUI
{

class PaletteFrame: public Widget
{
public:
	PaletteFrame();

	void setBitmap(BITMAP* value);
	void setColorData(byte* value);
	void setPal(PALETTE value);
	void setCount(uint8_t value);
	
private:
	BITMAP* bmp;
	byte* ColorData;
	PALETTE plt;
	uint8_t count;
	DialogRef alDialog;
	
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void realize(DialogRunner& runner) override;
};

}

#endif
