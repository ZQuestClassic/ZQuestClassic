#ifndef ZC_GUI_SELHEROTILE_SWATCH_H_
#define ZC_GUI_SELHEROTILE_SWATCH_H_

#include "gui/widget.h"
#include "gui/dialog_ref.h"

namespace GUI
{
	int32_t newg_selherotile_proc(int32_t msg, DIALOG* d, int32_t);

	class SelHeroTileSwatch : public Widget
	{
	public:
		SelHeroTileSwatch();		

		void setHeroSprite(int32_t value);
		void setDir(int32_t value);
		void setFrames(int32_t value);
		void click();

		int32_t getHeroSprite();
		int32_t getDir();
		int32_t getFrames();

	private:
		int32_t clock, tile, flip, extend, frames;
		int32_t heroSprite;
		int32_t dir;
		
		DialogRef alDialog;

		void applyVisibility(bool visible) override;
		void applyDisabled(bool dis) override;
		void realize(DialogRunner& runner) override;
		int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
		friend int32_t newg_selherotile_proc(int32_t msg, DIALOG* d, int32_t);
	};

}

#endif