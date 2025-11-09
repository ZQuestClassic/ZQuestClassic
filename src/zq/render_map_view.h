#ifndef RENDER_MAP_VIEW_H_
#define RENDER_MAP_VIEW_H_

#include "base/render.h"

class MapViewRTI : public RenderTreeItem
{
public:
	MapViewRTI(): RenderTreeItem("map_view")
	{
	}

	int sw, sh, flags;
	bool view_map_mode;

private:
	void render(bool bitmap_resized) override;
};

MapViewRTI* mapview_get_rti();
void mapview_open(int flags, int sw, int sh, int bw, int bh);
void mapview_close();

#endif
