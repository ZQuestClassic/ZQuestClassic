#ifndef ZC_DIALOG_DITHERPICK_H_
#define ZC_DIALOG_DITHERPICK_H_

#include <gui/dialog.h>
#include <gui/button.h>
#include <gui/ditherprev.h>
#include <gui/window.h>
#include <functional>

void call_edit_dither(byte& ditherTy, byte& ditherArg, byte& ditherCol, bool editcol);

class DitherPickDialog: public GUI::Dialog<DitherPickDialog>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL
	};

	DitherPickDialog(byte& ditherTy, byte& ditherArg, byte& ditherCol, bool editcol);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	byte &dest_dtype, &dest_darg, &dest_dcol;
	bool edit_color;
	byte dtype, darg, dcol;
	
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::DitherPreview> dithprev;
};

#endif
