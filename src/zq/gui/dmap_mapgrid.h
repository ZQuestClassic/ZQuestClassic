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

		int32_t onEvent(int32_t event, MessageDispatcher& sendMessage);

	private:
		int32_t message;
		std::function<void(byte*, byte, byte)> onUpdateFunc;

		byte* mapspace;
		byte continueScreen;
		byte compassScreen;
		bool small_dmap;

		DialogRef alDialog;

		void applyVisibility(bool visible) override;
		void applyDisabled(bool dis) override;
		void realize(DialogRunner& runner) override;

		friend int32_t new_d_grid_proc(int32_t msg, DIALOG* d, int32_t);
	};

}

#endif
