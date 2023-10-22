#ifndef ZC_GUI_DITHERPREV_H
#define ZC_GUI_DITHERPREV_H

#include "gui/widget.h"
#include "gui/dialog_ref.h"
#include "drawing.h"
#include "base/ints.h"
#include <string>
#include <utility>

namespace GUI
{
int32_t dith_prev_proc(int32_t msg, DIALOG* d, int32_t);

class DitherPreview: public Widget
{
public:
	DitherPreview();

	void setDitherType(byte ty)
	{
		dtype = ty;
		pendDraw();
	}
	void setDitherArg(byte arg)
	{
		darg = arg;
		pendDraw();
	}
	void setDitherColor(byte col)
	{
		dcol = col;
		pendDraw();
	}
	void setPreviewScale(byte newscale)
	{
		scale = newscale;
		pendDraw();
	}

private:
	byte dtype, darg, dcol;
	byte scale;
	DialogRef alDialog;

	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void realize(DialogRunner& runner) override;
	void calculateSize() override;
	friend int32_t dith_prev_proc(int32_t msg, DIALOG* d, int32_t);
};

}

#endif
