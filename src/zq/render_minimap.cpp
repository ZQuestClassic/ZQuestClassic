#include "zq/render_minimap.h"

#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "jwin_a5.h"
#include "zq/render.h"
#include "zq/zq_class.h"
#include "zq/zq_misc.h"

extern size_and_pos real_minimap, real_minimap_zoomed;
extern int MMapCursorStyle;
extern zmap Map;

static bool mmap_dirty = true;
static int mmap_blink_count;
static int mmap_cursor_blink_speed = 20;

bool zoomed_minimap = false;

void mmap_mark_dirty()
{
	mmap_dirty = true;
}

void mmap_mark_dirty_delayed()
{
	mmap_dirty = true;
	remove_int(mmap_mark_dirty_delayed);
}

void mmap_set_zoom(bool zoomed)
{
	zoomed_minimap = zoomed;
	size_and_pos *real_mini_sqr = zoomed_minimap ? &real_minimap_zoomed : &real_minimap;
	auto rti_mmap = get_mmap_rti();
	rti_mmap->set_transform({
		.x = real_mini_sqr->x,
		.y = real_mini_sqr->y,
		.xscale = 1,
		.yscale = 1,
	});
	rti_mmap->width = real_mini_sqr->w * real_mini_sqr->xscale;
	rti_mmap->height = real_mini_sqr->h * real_mini_sqr->yscale;
}

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

static void mmap_draw()
{
	if (!Map.Scr(0))
		return;

	size_and_pos *real_mini_sqr = &real_minimap;
	
	if(zoomed_minimap)
	{
		real_mini_sqr = &real_minimap_zoomed;
	}

	int offx = get_mmap_rti()->get_transform().x;
	int offy = get_mmap_rti()->get_transform().y;

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

void mmap_init()
{
	mmap_set_zoom(false);
	get_mmap_rti()->cb = [&]() {
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
			mmap_mark_dirty();
		}

		auto rti_mmap = get_mmap_rti();

		bool size_changed = false;
		if (rti_mmap->bitmap && (al_get_bitmap_width(rti_mmap->bitmap) != rti_mmap->width || al_get_bitmap_height(rti_mmap->bitmap) != rti_mmap->height))
		{
			al_destroy_bitmap(rti_mmap->bitmap);
			rti_mmap->bitmap = nullptr;
			size_changed = true;
		}
		if (!rti_mmap->bitmap)
		{
			ASSERT(rti_mmap->width > 0 && rti_mmap->height > 0);
			set_bitmap_create_flags(true);
			rti_mmap->bitmap = create_a5_bitmap(rti_mmap->width, rti_mmap->height);
			mmap_mark_dirty();
		}

		if (!mmap_dirty) return;
		mmap_dirty = false;

		al_set_target_bitmap(rti_mmap->bitmap);
		al_clear_to_color(al_map_rgba(0, 0, 0, 0));
		mmap_draw();
		al_set_target_backbuffer(all_get_display());

		// For some reason only on the Web version the bitmap will remain black when changing size.
		// Simply marking dirty to draw one more time is no good, but setting a timer seems to work.
		// TODO: why is this happening?
		if (size_changed && is_web())
			install_int(mmap_mark_dirty_delayed, 1);
	};
}
