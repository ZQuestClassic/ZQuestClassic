#ifndef ZC_GUI_MISC_COLOR_SWATCH_H_
#define ZC_GUI_MISC_COLOR_SWATCH_H_

#include "gui/widget.h"
#include "gui/dialog_ref.h"

int32_t newg_misccolors_proc(int32_t msg, DIALOG* d, int32_t c);

namespace GUI
{
	class MiscColorSwatch : public Widget
	{
	public:
		MiscColorSwatch();
		void click();
		void setOnClickFunc(std::function<void()> newOnClick)
		{
			onClickFunc = newOnClick;
		}
		
		void setHexClicked(int16_t val);
		void setColor(byte val);
		byte getHexClicked();
		byte getColor();
		
		template<typename T>
		RequireMessage<T> onClick(T m)
		{
			message = static_cast<int32_t>(m);
		}
		DialogRef alDialog;
	private:
		int32_t hexclicked;
		byte color_buf[17];

		std::function<void()> onClickFunc;

		void applyVisibility(bool visible) override;
		void applyDisabled(bool dis) override;
		void applyFont(FONT* newFont);
		void realize(DialogRunner& runner) override;
		int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
	};
}
#endif