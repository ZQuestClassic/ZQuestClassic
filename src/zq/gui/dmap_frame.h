#ifndef ZC_GUI_DMAP_FRAME_H_
#define ZC_GUI_DMAP_FRAME_H_

#include "gui/widget.h"
#include "gui/dialog_ref.h"

int dmap_frame_proc(int32_t msg, DIALOG* d, int32_t c);

namespace GUI
{
	class DMapFrame : public Widget
	{
	public:
		DMapFrame();

		void setDMap(int32_t value);

	private:
		int32_t Dmap = 0;
		DialogRef alDialog;

		void applyVisibility(bool visible) override;
		void applyDisabled(bool dis) override;
		void realize(DialogRunner& runner) override;
		friend int ::dmap_frame_proc(int32_t, DIALOG*, int32_t);
	};
}

#endif
#pragma once
