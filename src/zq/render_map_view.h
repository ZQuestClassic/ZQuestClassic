#pragma once

#include "zalleg/render.h"

class MapViewRTI : public RenderTreeItem
{
public:
	MapViewRTI(): RenderTreeItem("map_view")
	{
		// transparency_index = 0;
	}

	int sw, sh, flags;
	bool view_map_mode;

private:
	void render(bool bitmap_resized) override;
};

MapViewRTI* mapview_get_rti();
void mapview_open(int flags, int sw, int sh, int bw, int bh);
void mapview_close();
