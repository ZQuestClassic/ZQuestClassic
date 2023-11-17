#include "zq/render_tooltip.h"
#include "base/render.h"
#include "base/zdefs.h"
#include "zq/render.h"
#include "zq/zquest.h"
#include <algorithm>

// Renders:
//   - tooltip
//       - highlight
//       - text

int ttip_global_id = 1;

struct tooltip
{
	int id;
	std::string text;
	size_and_pos trigger_area;
	int tip_x, tip_y;
	int highlight_thickness;
	// Higher = on top.
	int z_index;
};

static std::map<int, tooltip> tooltips;

class ToolTipRTI : public LegacyBitmapRTI
{
public:
	ToolTipRTI(std::string name);

	int active_tooltip_id = 0;
	int timer = 0;

private:
	void prepare();
};

class TextRTI : public LegacyBitmapRTI
{
public:
	TextRTI(std::string name) : LegacyBitmapRTI(name) {}

	std::string text;

private:
	void prepare()
	{
		FONT* oldfont = font;
		font = get_custom_font(CFONT_TTIP);
		auto [w, h] = get_box_text_size(text.c_str(), 1);
		font = oldfont;

		auto t = get_transform();
		t.x = std::clamp(t.x, 0, zq_screen_w - w);
		t.y = std::clamp(t.y, 0, zq_screen_h - h);
		set_transform(t);

		if (!a4_bitmap || a4_bitmap->w != w || a4_bitmap->h != h)
		{
			destroy_bitmap(a4_bitmap);
			a4_bitmap = create_bitmap_ex(8, w, h);
			set_size(w, h);
		}
	}

	void render(bool size_changed)
	{
		size_and_pos pos = {};
		FONT* oldfont = font;
		font = get_custom_font(CFONT_TTIP);
		draw_box(a4_bitmap, &pos, text.c_str());
		font = oldfont;

		LegacyBitmapRTI::render(size_changed);
	}
};

class HighlightRTI : public LegacyBitmapRTI
{
public:
	HighlightRTI(std::string name) : LegacyBitmapRTI(name)
	{
		transparency_index = 0;
	}

	size_and_pos pos;

private:
	void render(bool size_changed)
	{
		int w = pos.w;
		int h = pos.h;
		if (!a4_bitmap || a4_bitmap->w != w || a4_bitmap->h != h)
		{
			destroy_bitmap(a4_bitmap);
			a4_bitmap = create_bitmap_ex(8, w, h);
			clear_bitmap(a4_bitmap);
			set_size(w, h);
		}
		highlight(a4_bitmap, pos);

		LegacyBitmapRTI::render(size_changed);
	}
};

static ToolTipRTI rti_tooltip("tooltip");
static TextRTI rti_text("text");
static HighlightRTI rti_highlight("highlight");
extern int TTipHLCol;
extern int32_t TooltipsHighlight;
extern int EnableTooltips;

int ttip_register_id()
{
	static int next_tooltip_id = 2;
	return next_tooltip_id++;
}

void ttip_install(int id, std::string text, size_and_pos trigger_area, int tip_x, int tip_y)
{
	static bool initialized = false;
	if (!initialized)
	{
		initialized = true;
		rti_tooltip.add_child(&rti_highlight);
		rti_tooltip.add_child(&rti_text);
		get_root_rti()->add_child(&rti_tooltip);
	}

	if (tip_x == -1)
	{
		tip_x = trigger_area.x;
		tip_y = trigger_area.y;
	}
	tip_y += 16;
	int highlight_thickness = 1;
	// For tooltips that reuse an id, check if it changed and reset the timer if so.
	if (id == rti_tooltip.active_tooltip_id && tooltips.contains(id))
		if (tooltips[id].trigger_area != trigger_area || tooltips[id].text != text)
			rti_tooltip.timer = 0;
	tooltips[id] = {id, text, trigger_area, tip_x, tip_y, highlight_thickness, 0};
}

void ttip_install(int id, std::string text, int x, int y, int w, int h, int tip_x, int tip_y, int fw, int fh)
{
	size_and_pos trigger_area;
	trigger_area.x = x;
	trigger_area.y = y;
	trigger_area.w = w;
	trigger_area.h = h;
	trigger_area.fw = fw;
	trigger_area.fh = fh;
	ttip_install(id, text, trigger_area, tip_x, tip_y);
}

void ttip_uninstall(int id)
{
	tooltips.erase(id);
	if (rti_tooltip.active_tooltip_id == id)
		rti_tooltip.active_tooltip_id = 0;
}

void ttip_uninstall_all()
{
	tooltips.clear();
	rti_tooltip.active_tooltip_id = 0;
	rti_tooltip.visible = false;
	rti_tooltip.timer = 0;
}

void ttip_set_highlight_thickness(int id, int thickness)
{
	tooltips[id].highlight_thickness = thickness;
}

void ttip_set_z_index(int id, int z_index)
{
	tooltips[id].z_index = z_index;
}

static void add_highlight(size_and_pos pos, int thickness)
{
	rti_highlight.set_transform({pos.x, pos.y});
	rti_highlight.visible = true;

	size_and_pos pos2 = pos;
	pos2.x = 0;
	pos2.y = 0;
	pos2.data[0] = thickness;
	pos2.data[1] = vc(TTipHLCol);

	if (pos2 == rti_highlight.pos)
		return;

	rti_highlight.pos = pos2;
	rti_highlight.set_size(pos2.w, pos2.h);
	rti_highlight.dirty = true;
}

void ttip_clear_timer()
{
	rti_tooltip.timer = tooltip_maxtimer;
}

ToolTipRTI::ToolTipRTI(std::string name) : LegacyBitmapRTI(name) {}

void ToolTipRTI::prepare()
{
	if (!EnableTooltips)
	{
		visible = false;
		return;
	}

	// Find tooltip mouse is currently over.
	auto [mx, my] = zc_get_mouse();

	tooltip* tooltip = nullptr;
	for (auto& it : tooltips)
	{
		if (tooltip && tooltip->z_index > it.second.z_index)
			continue;

		if (it.second.trigger_area.rect(mx, my))
		{
			tooltip = &it.second;
		}
	}

	if (!tooltip)
	{
		visible = false;
		return;
	}

	if (active_tooltip_id != tooltip->id)
	{
		rti_tooltip.timer = 0;
	}
	active_tooltip_id = tooltip->id;

	if (timer < tooltip_maxtimer)
	{
		++timer;
		visible = false;
		return;
	}

	visible = true;

	if (rti_text.text != tooltip->text)
	{
		rti_text.dirty = true;
		rti_text.text = tooltip->text;
	}

	rti_text.set_transform({tooltip->tip_x, tooltip->tip_y, 1, 1});

	rti_highlight.visible = TooltipsHighlight;
	if (rti_highlight.visible && rti_highlight.pos != tooltip->trigger_area)
	{
		add_highlight(tooltip->trigger_area, tooltip->highlight_thickness);
	}
}
