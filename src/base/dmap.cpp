#include "dmap.h"
#include "subscr.h"

dmap *DMaps;

void dmap::validate_subscreens()
{
	if(active_subscreen >= subscreens_active.size())
		active_subscreen = 0;
	if(passive_subscreen >= subscreens_passive.size())
		passive_subscreen = 0;
	if(overlay_subscreen >= subscreens_overlay.size())
		overlay_subscreen = 0;
}

