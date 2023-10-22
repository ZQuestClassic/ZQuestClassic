#include "zq/render_minimap.h"

#include "base/render.h"
#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "jwin_a5.h"
#include "qst.h"
#include "zq/render.h"
#include "zq/zq_class.h"
#include "zq/zq_misc.h"

extern size_and_pos real_minimap, real_minimap_zoomed;
extern int MMapCursorStyle;
extern zmap Map;
extern int32_t prv_mode;

static MiniMapRTI rti_minimap;
static int mmap_blink_count;
static int mmap_cursor_blink_speed = 20;

bool zoomed_minimap = false;

static int get_cursor_color()
{
	switch (MMapCursorStyle)
	{
		case 0:
			return vc(15);
		case 1:
			return (mmap_blink_count%(mmap_cursor_blink_speed*2))>=mmap_cursor_blink_speed ? vc(0) : vc(15);
		case 2:
			return (mmap_blink_count%(mmap_cursor_blink_speed*2))>=mmap_cursor_blink_speed ? vc(12) : vc(9);
	}
	return 0;
}

static void mmap_draw(int offx, int offy)
{
	size_and_pos *real_mini_sqr = &real_minimap;
	
	if(zoomed_minimap)
	{
		real_mini_sqr = &real_minimap_zoomed;
	}

	if(Map.getCurrMap()<Map.getMapCount())
	{
		for(int32_t i=0; i<MAPSCRS; i++)
		{
			auto& sqr = real_mini_sqr->subsquare(i);
			
			if(Map.Scr(i)->valid&mVALID)
			{
				al_draw_filled_rectangle(sqr.x-offx, sqr.y-offy, sqr.x+sqr.w-offx, sqr.y+sqr.h-offy,
					real_lc1(Map.Scr(i)->color));
				
				int scl = 2;
				int woffs = (sqr.w-(sqr.w/scl))/2;
				int hoffs = (sqr.h-(sqr.h/scl))/2;
				al_draw_filled_rectangle(sqr.x+woffs-offx, sqr.y+hoffs-offy, sqr.x+sqr.w-woffs-offx, sqr.y+sqr.h-hoffs-offy,
					real_lc2(Map.Scr(i)->color));
			}
			else
			{
				// Handled by draw_screenunit.
			}
		}
		
		int32_t s=Map.getCurrScr();
		// The white marker rect
		int32_t cursor_color = get_cursor_color();
		if(cursor_color)
		{
			auto& sqr = real_mini_sqr->subsquare(s);
			al_draw_rectangle(sqr.x-offx, sqr.y-offy, sqr.x+sqr.w-offx, sqr.y+sqr.h-offy,
				a5color(cursor_color), 2);
		}
	}
}

MiniMapRTI::MiniMapRTI() : RenderTreeItem("minimap") {}

void MiniMapRTI::prepare()
{
	// Redraw everything anytime the cursor changes.
	// TODO: just redraw the cursor, yo. Maybe make the cursor its own RenderTreeItem.
	static int prev_cursor_color;

	// TODO: for web, changing the target bitmap is really expensive.
	// Seems like a bug. https://discord.com/channels/993415281244393504/1163652238011551816
	// So for now, disable the cursor blinking so we only redraw when the something actually changes.
	if (!is_web()) mmap_blink_count++;

	int32_t cursor_color = get_cursor_color();
	if (prev_cursor_color != cursor_color)
	{
		prev_cursor_color = cursor_color;
		dirty = true;
	}

	visible = !prv_mode;
	freeze = rti_dialogs.visible || is_loading_quest() || !Map.Scr(0);
}

void MiniMapRTI::render(bool bitmap_resized)
{
	auto transform = get_transform();
	mmap_draw(transform.x, transform.y);

	// For some reason only on the Web version the bitmap will remain black when changing size.
	// Simply marking dirty to draw one more time is no good, but setting a timer seems to work.
	// TODO: why is this happening?
	if (bitmap_resized && is_web())
		install_int(mmap_mark_dirty_delayed, 1);
}

void mmap_mark_dirty()
{
	rti_minimap.dirty = true;
}

void mmap_mark_dirty_delayed()
{
	rti_minimap.dirty = true;
	remove_int(mmap_mark_dirty_delayed);
}

void mmap_set_zoom(bool zoomed)
{
	zoomed_minimap = zoomed;
	size_and_pos *real_mini_sqr = zoomed_minimap ? &real_minimap_zoomed : &real_minimap;
	rti_minimap.set_transform({
		.x = real_mini_sqr->x,
		.y = real_mini_sqr->y,
	});
	rti_minimap.width = real_mini_sqr->w * real_mini_sqr->xscale;
	rti_minimap.height = real_mini_sqr->h * real_mini_sqr->yscale;
}

void mmap_init()
{
	get_screen_rti()->add_child(&rti_minimap);
	mmap_set_zoom(false);
}
