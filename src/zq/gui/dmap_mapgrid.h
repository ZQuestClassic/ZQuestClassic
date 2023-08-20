#ifndef ZC_GUI_DMAPMAPGRID_H
#define ZC_GUI_DMAPMAPGRID_H

#include "gui/widget.h"
#include "gui/dialog_ref.h"

class RenderTreeItem;

namespace GUI
{

	class DMapMapGrid : public Widget
	{
	public:
		DMapMapGrid();

		void setMapGridPtr(byte* ptr)
		{
			mapspace = ptr;
		}
		byte* getMapGridPtr()
		{
			return mapspace;
		}

		void setContinueScreen(byte scrn)
		{
			if (getContinueScreen() != scrn)
				pendDraw();
			continueScreen = scrn;
		}
		byte getContinueScreen()
		{
			return continueScreen;
		}

		void setCompassScreen(byte scrn)
		{
			if (getCompassScreen() != scrn)
				pendDraw();
			compassScreen = scrn;
		}
		byte getCompassScreen()
		{
			return compassScreen;
		}

		void setShowSquares(bool newcontinue, bool newcompass)
		{
			show_continue = newcontinue;
			show_compass = newcompass;
		}
		bool getShowCompass()
		{
			return show_compass;
		}
		bool getShowContinue() {
			return show_continue;
		}

		void setSmallDMap(bool value)
		{
			if (getSmallDMap() != value)
				pendDraw();
			small_dmap = value;
		}
		bool getSmallDMap()
		{
			return small_dmap;
		}

		template<typename T>
		RequireMessage<T> onUpdate(T m)
		{
			message = static_cast<int32_t>(m);
		}

		void setOnUpdate(std::function<void(byte*, byte, byte)> newfunc)
		{
			onUpdateFunc = std::move(newfunc);
		}

	private:
		int32_t message;
		std::function<void(byte*, byte, byte)> onUpdateFunc;

		byte* mapspace;
		byte continueScreen;
		byte compassScreen;
		bool small_dmap;
		bool show_continue, show_compass;

		DialogRef alDialog;

		void applyVisibility(bool visible) override;
		void applyDisabled(bool dis) override;
		void realize(DialogRunner& runner) override;

		int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;

		friend int32_t new_d_grid_proc(int32_t msg, DIALOG* d, int32_t);
	};

}

#endif
