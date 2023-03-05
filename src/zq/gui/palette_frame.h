#ifndef ZC_GUI_PALETTE_FRAME_H
#define ZC_GUI_PALETTE_FRAME_H

#include "gui/widget.h"
#include "gui/dialog_ref.h"

int d_bitmap_proc_a5(DIALOG* d, int msg, int);
void draw_cset_proc_a5(DIALOG *d);
int d_cset_proc_a5(int msg,DIALOG *d,int c);

namespace GUI
{

class PaletteFrame: public Widget
{
public:
	PaletteFrame();

	void setColorData(byte* value);
	void setPal(PALETTE* value);
	void setCount(uint8_t value);
	void setOnUpdate(std::function<void()> newOnUpdate);
	int getSelection();
	void setScale(int newscale);
	
private:
	byte* ColorData;
	PALETTE* plt;
	uint8_t count;
	DialogRef alDialog;
	std::function<void()> onUpdate;
	int grcol, scale;
	
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void calculateSize() override;
	void realize(DialogRunner& runner) override;
	int onEvent(int event, MessageDispatcher& sendMessage) override;
	friend void ::draw_cset_proc_a5(DIALOG *d);
	friend int ::d_cset_proc_a5(int msg,DIALOG *d,int c);
};

}

#endif
