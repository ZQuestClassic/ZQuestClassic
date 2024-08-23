#ifndef ZC_GUI_DMAPREGIONGRID_H_
#define ZC_GUI_DMAPREGIONGRID_H_

#include "base/dmap.h"
#include "gui/widget.h"
#include "gui/dialog_ref.h"
#include <zq/zq_class.h>

class RenderTreeItem;

namespace GUI
{

	class DMapRegionGrid : public Widget
	{
	public:
		DMapRegionGrid();

		void setRegionMapPtr(zmap* ptr)
		{
			regionmap = ptr;
		}
		zmap* getRegionMapPtr()
		{
			return regionmap;
		}

		void setLocalDmap(dmap* localDmap_)
		{
			localDmap = localDmap_;
		}
		dmap* getLocalDmap()
		{
			return localDmap;
		}

		void setCurrentRegionIndex(int newindex);
		int getCurrentRegionIndex();

		template<typename T>
		RequireMessage<T> onUpdate(T m)
		{
			message = static_cast<int32_t>(m);
		}

		void setOnUpdate(std::function<void()> newfunc)
		{
			onUpdateFunc = std::move(newfunc);
		}

	private:
		int32_t message;
		std::function<void()> onUpdateFunc;

		zmap* regionmap;
		dmap* localDmap;

		DialogRef alDialog;

		void applyVisibility(bool visible) override;
		void applyDisabled(bool dis) override;
		void realize(DialogRunner& runner) override;

		int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
	};
}

#endif
