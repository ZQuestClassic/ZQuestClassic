#include "render.h"

static void render_tree_layout(RenderTreeItem* rti, RenderTreeItem* rti_parent)
{
	if (!rti_parent)
	{
		rti->computed.scale = rti->transform.scale;
		rti->computed.x = rti->transform.scale * rti->transform.x;
		rti->computed.y = rti->transform.scale * rti->transform.y;
	}
	else
	{
		rti->computed.scale = rti->transform.scale * rti_parent->computed.scale;
		rti->computed.x = rti->computed.scale * rti->transform.x + rti_parent->transform.x;
		rti->computed.y = rti->computed.scale * rti->transform.y + rti_parent->transform.y;
	}
	
	for (auto rti_child : rti->children)
	{
		render_tree_layout(rti_child, rti);
	}
}

static void render_tree_draw_item(RenderTreeItem* rti)
{
	if (!rti->visible)
		return;

	if (rti->bitmap)
	{
		if (rti->a4_bitmap && !rti->freeze_a4_bitmap_render)
		{
			all_set_transparent_palette_index(rti->transparency_index);
			all_render_a5_bitmap(rti->a4_bitmap, rti->bitmap);
		}

		int w = al_get_bitmap_width(rti->bitmap);
		int h = al_get_bitmap_height(rti->bitmap);

		if (rti->tint)
		{
			al_draw_tinted_scaled_bitmap(rti->bitmap, *rti->tint, 0, 0, w, h, rti->computed.x, rti->computed.y, w*rti->computed.scale, h*rti->computed.scale, 0);
		}
		else
		{
			al_draw_scaled_bitmap(rti->bitmap, 0, 0, w, h, rti->computed.x, rti->computed.y, w*rti->computed.scale, h*rti->computed.scale, 0);
		}
	}

	for (auto rti_child : rti->children)
	{
		render_tree_draw_item(rti_child);
	}
}

void render_tree_draw(RenderTreeItem* rti)
{
	render_tree_layout(rti, nullptr);
	render_tree_draw_item(rti);
}
