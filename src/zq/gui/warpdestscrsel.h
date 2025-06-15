#ifndef ZC_GUI_WARPDESTSCRSEL_H_
#define ZC_GUI_WARPDESTSCRSEL_H_

#include "gui/widget.h"
#include "gui/dialog_ref.h"

int32_t new_warpdestscrsel_proc(int32_t msg, DIALOG* d, int32_t c);

namespace GUI
{
	class WarpDestScrSel : public Widget
	{
	public:
		WarpDestScrSel();

		void setScreen(int32_t val);
		void setDMap(int32_t val);
		int16_t getScreen();
		int16_t getDMap();

	private:
		int16_t scr, Dmap, force_16; //d1, d2, fg
		DialogRef alDialog;

		void applyVisibility(bool visible) override;
		void applyDisabled(bool dis) override;
		void realize(DialogRunner& runner) override;
		friend int ::new_warpdestscrsel_proc(int32_t, DIALOG*, int32_t);
	};
}

#endif