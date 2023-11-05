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

class ToolTipRTI : public LegacyBitmapRTI
{
public:
	ToolTipRTI(std::string name);

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
static bool initialized = false;
extern int TTipHLCol;

void ttip_add(std::string text, int x, int y, float scale)
{
	if (!initialized)
	{
		initialized = true;
		rti_tooltip.add_child(&rti_highlight);
		rti_tooltip.add_child(&rti_text);
	}

	rti_text.dirty |= rti_text.text != text;
	rti_text.text = text;
	rti_text.set_transform({x, y, scale, scale});
	get_root_rti()->add_child(&rti_tooltip);
	rti_highlight.visible = false;
}

void ttip_add_highlight(int x, int y, int w, int h, int fw, int fh)
{
	rti_highlight.set_transform({x, y});
	rti_highlight.visible = true;

	size_and_pos pos = {};
	pos.set(0, 0, w, h);
	pos.fw = fw;
	pos.fh = fh;
	pos.data[0] = 2;
	pos.data[1] = vc(TTipHLCol);

	if (pos == rti_highlight.pos)
		return;

	rti_highlight.pos = pos;
	rti_highlight.set_size(w, h);
	rti_highlight.dirty = true;
}

void ttip_set_highlight_thickness(int thickness)
{
	rti_highlight.pos.data[0] = zoomed_minimap ? 2 : 1;
}

void ttip_clear_timer()
{
	rti_tooltip.timer = tooltip_maxtimer + 1;
}

void ttip_remove()
{
	rti_tooltip.remove();
	rti_tooltip.visible = false;
	rti_tooltip.timer = 0;
}

ToolTipRTI::ToolTipRTI(std::string name) : LegacyBitmapRTI(name) {}

void ToolTipRTI::prepare()
{
	if (timer <= tooltip_maxtimer)
	{
		++timer;
		visible = false;
	}
	else
	{
		visible = true;
	}
}
