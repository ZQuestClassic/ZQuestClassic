#ifndef ZC_GUI_DMAPXMAP_H
#define ZC_GUI_DMAPXMAP_H

#include "gui/widget.h"
#include "gui/dialog_ref.h"

class RenderTreeItem;

int32_t new_xmaplist_proc(int32_t msg, DIALOG* d, int32_t c);

namespace GUI
{

	class DMapMinimap : public Widget
	{
	public:
		DMapMinimap();

		void setCurMap(int32_t newmap)
		{
			if (cur_map != newmap)
				pendDraw();
			cur_map = newmap;
		}
		int32_t getCurMap()
		{
			return cur_map;
		}

		void setSmallDMap(bool state)
		{
			if (small_dmap != state)
				pendDraw();
			small_dmap = state;
		}
		bool getSmallDMap()
		{
			return small_dmap;
		}

		void setOffset(int32_t newoffset)
		{
			if (newoffset != offset)
				pendDraw();
			offset = newoffset;
		}
		int32_t getOffset()
		{
			return offset;
		}

	private:
		int32_t cur_map;
		bool small_dmap;
		int32_t offset;
		RenderTreeItem* rti;

		DialogRef alDialog;

		void applyVisibility(bool visible) override;
		void applyDisabled(bool dis) override;
		void realize(DialogRunner& runner) override;

		friend int32_t new_xmaplist_proc(int32_t msg, DIALOG* d, int32_t c);
	};

}

#endif
