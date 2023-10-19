#include "render.h"
#include "base/zapp.h"
#include "base/zdefs.h"
#include "base/fonts.h"
#include "fmt/core.h"
#include "jwin_a5.h"
#include <atomic>

using namespace std::chrono_literals;

void RenderTreeItem::add_child(RenderTreeItem* child)
{
	if (child->parent)
		child->parent->remove_child(child);
	children.push_back(child);
	child->parent = this;
}
void RenderTreeItem::add_child_before(RenderTreeItem* child, RenderTreeItem* before_child)
{
	if (child->parent)
		child->parent->remove_child(child);
	auto it = std::find(children.begin(), children.end(), before_child);
	ASSERT(it != children.end());
	children.insert(it, child);
	child->parent = this;
}
void RenderTreeItem::remove_child(RenderTreeItem* child)
{
	auto it = std::find(children.begin(), children.end(), child);
	if (it != children.end())
	{
		children.erase(it);
		child->parent = nullptr;
	}
}
std::vector<RenderTreeItem*> const& RenderTreeItem::get_children() const
{
	return children;
}
std::vector<RenderTreeItem*>& RenderTreeItem::get_children()
{
	return children;
}
bool RenderTreeItem::has_children() const
{
	return !children.empty();
}
void RenderTreeItem::handle_dirty()
{
	if (!transform_dirty) return;

	const Matrix& parent_transform = parent ? parent->get_transform_matrix() : Matrix::Identity();
	transform_matrix = Matrix::Translate(transform.x, transform.y).mul(Matrix::Scale(transform.xscale, transform.yscale));
	transform_matrix = parent_transform.mul(transform_matrix);
	transform_dirty = false;
}
void RenderTreeItem::mark_dirty()
{
	transform_dirty = transform_inverse_dirty = true;
	for (auto child : children) child->mark_dirty();
}
void RenderTreeItem::set_transform(Transform new_transform)
{
	if (transform == new_transform)
		return;

	transform = new_transform;
	mark_dirty();
}
const Transform& RenderTreeItem::get_transform() const
{
	return transform;
}
const Matrix& RenderTreeItem::get_transform_matrix()
{
	handle_dirty();
	return transform_matrix;
}
std::pair<int, int> RenderTreeItem::world_to_local(int x, int y)
{
	handle_dirty();
	if (transform_inverse_dirty)
	{
		transform_matrix_inverse = transform_matrix.inverse();
		transform_inverse_dirty = false;
	}
	return transform_matrix_inverse.apply(x, y);
}
std::pair<int, int> RenderTreeItem::local_to_world(int x, int y)
{
	handle_dirty();
	return transform_matrix.apply(x, y);
}
std::pair<int, int> RenderTreeItem::pos()
{
	return local_to_world(0, 0);
}
std::pair<int, int> RenderTreeItem::rel_mouse()
{
	return world_to_local(mouse_x, mouse_y);
}

RenderTreeItem rti_dialogs("dialogs");

extern int32_t zq_screen_w, zq_screen_h;
unsigned char info_opacity = 255;
bool use_linear_bitmaps();
ALLEGRO_COLOR AL5_INVIS = al_map_rgba(0,0,0,0),
	AL5_BLACK = al_map_rgb(0,0,0),
	AL5_WHITE = al_map_rgb(255,255,255),
	AL5_YELLOW = al_map_rgb(255,255,0),
	AL5_PINK = al_map_rgb(255,0,255),
	AL5_DGRAY = al_map_rgb(85,85,85),
	AL5_LGRAY = al_map_rgb(170,170,170),
	AL5_BLUE = al_map_rgb(85,85,255),
	AL5_LRED = al_map_rgb(255,85,85),
	AL5_DRED = al_map_rgb(178,36,36),
	AL5_LGREEN = al_map_rgb(85,255,85),
	AL5_LAQUA = al_map_rgb(85,255,255);
void set_bitmap_create_flags(bool preserve_texture)
{
	int flags = ALLEGRO_CONVERT_BITMAP;
	if(!preserve_texture)
		flags |= ALLEGRO_NO_PRESERVE_TEXTURE;
	if (use_linear_bitmaps())
		flags |= ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR;
	al_set_new_bitmap_flags(flags);
}

void clear_a5_bmp(ALLEGRO_COLOR col, ALLEGRO_BITMAP* bmp)
{
	if (is_headless())
		return;

	if(bmp)
	{
		ALLEGRO_STATE old_state;
		al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP);
		
		al_set_target_bitmap(bmp);
		
		al_clear_to_color(col);
		
		al_restore_state(&old_state);
	}
	else al_clear_to_color(col);
}
void clear_a5_bmp(ALLEGRO_BITMAP* bmp)
{
	clear_a5_bmp(AL5_INVIS,bmp);
}

ALLEGRO_BITMAP* create_a5_bitmap(int w, int h)
{
	ALLEGRO_BITMAP* bitmap = al_create_bitmap(w, h);
	clear_a5_bmp(bitmap);
	return bitmap;
}

RenderTreeItem::RenderTreeItem(std::string name, RenderTreeItem* parent) : name(name), parent(parent)
{
}

RenderTreeItem::~RenderTreeItem()
{
	if(owned)
	{
		if(bitmap)
			al_destroy_bitmap(bitmap);
		if(a4_bitmap)
			destroy_bitmap(a4_bitmap);
	}
	if(owned_tint)
	{
		if(tint)
			delete tint;
	}
	for(RenderTreeItem* child : children)
	{
		if(owned || child->owned)
		{
			delete child;
		}
	}
}

ALLEGRO_COLOR a5colors[256];
uint32_t zc_backend_palette[256];
static int backend_fmt = ALLEGRO_PIXEL_FORMAT_ABGR_8888;
void _init_render(int fmt)
{
	backend_fmt = fmt;
}
uint32_t get_backend_a5_col(RGB const& c)
{
	unsigned char r = c.r*4, g = c.g*4, b = c.b*4, a = 255;
	switch(backend_fmt)
	{
		case ALLEGRO_PIXEL_FORMAT_ABGR_8888: default:
			return r | (g << 8) | (b << 16) | (a << 24);
		case ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE:
			return r | (g << 8) | (b << 16) | (a << 24);
		case ALLEGRO_PIXEL_FORMAT_ARGB_8888:
			return b | (g << 8) | (r << 16) | (a << 24);
		case ALLEGRO_PIXEL_FORMAT_RGBA_8888:
			return a | (b << 8) | (g << 16) | (r << 24);
	}
}
uint32_t repl_a5_backend_alpha(uint32_t back_col, unsigned char a)
{
	switch(backend_fmt)
	{
		case ALLEGRO_PIXEL_FORMAT_ABGR_8888: default:
		case ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE:
		case ALLEGRO_PIXEL_FORMAT_ARGB_8888:
			back_col &= ~(0xFF << 24);
			return back_col | (a << 24);
		case ALLEGRO_PIXEL_FORMAT_RGBA_8888:
			back_col &= ~(0xFF);
			return a | back_col;
	}
}
void load_palette(uint32_t* backpal, ALLEGRO_COLOR* backcols, PALETTE pal, int start, int end)
{
	if(start>end) zc_swap(start,end);
	for(int q = start; q <= end; ++q)
	{
		if(backcols) backcols[q] = a5color(pal[q]);
		if(backpal) backpal[q] = get_backend_a5_col(pal[q]);
	}
}

void zc_set_palette(PALETTE pal)
{
	load_palette(zc_backend_palette, a5colors, pal);
	set_palette(pal);
}
void zc_set_palette_range(PALETTE pal, int start, int end, bool)
{
	load_palette(zc_backend_palette, a5colors, pal, start, end);
	set_palette_range(pal,start,end,false);
}

void render_a4_a5(BITMAP* src,int sx,int sy,int dx,int dy,int w,int h,int maskind,uint32_t* backpal)
{
	if(!backpal) backpal = zc_backend_palette;
	ALLEGRO_BITMAP* buf = al_create_bitmap(w,h);
	ALLEGRO_LOCKED_REGION * lr;
	uint8_t * line_8;
	uint32_t * line_32;
	int x, y;

	lr = al_lock_bitmap(buf, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
	if(lr)
	{
		line_8 = (uint8_t*)lr->data;
		line_32 = (uint32_t*)lr->data;
		for(y = 0; y < h; y++)
		{
			if(y+sy >= src->h) break;
			for(x = 0; x < w; x++)
			{
				if(x+sx >= src->w) break;
				int index = src->line[y+sy][x+sx];
				if (index == maskind)
					line_32[x] = 0;
				else
					line_32[x] = backpal[index];
			}
			line_8 += lr->pitch;
			line_32 = (uint32_t *)line_8;
		}
		al_unlock_bitmap(buf);
	}
	else
	{
		al_destroy_bitmap(buf);
		return;
	}
	
	al_draw_bitmap(buf, dx, dy, 0);
}

static void render_text(ALLEGRO_FONT* font, std::string text, int x, int y, int scale)
{
	ALLEGRO_STATE oldstate;
	al_store_state(&oldstate, ALLEGRO_STATE_TARGET_BITMAP);

	int resx = al_get_display_width(all_get_display());
	int w = al_get_text_width(font, text.c_str());
	int h = al_get_font_line_height(font);

	static ALLEGRO_BITMAP* text_bitmap;
	if (text_bitmap == nullptr || resx != al_get_bitmap_width(text_bitmap))
	{
		if (text_bitmap)
			al_destroy_bitmap(text_bitmap);
		al_set_new_bitmap_flags(ALLEGRO_NO_PRESERVE_TEXTURE);
		text_bitmap = al_create_bitmap(resx, h);
	}

	al_set_target_bitmap(text_bitmap);
	al_clear_to_color(al_map_rgba(0, 0, 0, 0));
	al_draw_filled_rectangle(0, 0, w, h, al_map_rgba_f(0, 0, 0, 0.6));
	al_draw_text(font, al_map_rgb_f(1,1,1), 0, 0, 0, text.c_str());

	al_set_target_backbuffer(all_get_display());
	al_draw_scaled_bitmap(text_bitmap,
		0, 0,
		al_get_bitmap_width(text_bitmap), al_get_bitmap_height(text_bitmap),
		x, y,
		al_get_bitmap_width(text_bitmap) * scale, al_get_bitmap_height(text_bitmap) * scale,
		0
	);
	al_restore_state(&oldstate);
}

void render_text_lines(ALLEGRO_FONT* font, std::vector<std::string> lines, TextJustify justify, TextAlign align, int scale)
{
	int resx = al_get_display_width(all_get_display());
	int resy = al_get_display_height(all_get_display());
	int font_height = al_get_font_line_height(font);
	int text_y = align == TextAlign::bottom ?
		resy - scale*font_height - 5 :
		// Offset just a bit so it doesn't obscure the title bar.
		resy*0.04;
	for (std::string line : lines)
	{
		int x = justify == TextJustify::left ?
			5 :
			resx - al_get_text_width(font, line.c_str())*scale - 5;
		render_text(font, line.c_str(), x, text_y, scale);
		text_y += (scale*font_height + 3) * (align == TextAlign::bottom ? -1 : 1);
	}
}

static void render_tree_draw_item(RenderTreeItem* rti)
{
	if (!rti->visible)
		return;

	if (rti->cb && !rti->freeze)
		rti->cb();

	if (rti->bitmap)
	{
		if (rti->bitmap && rti->a4_bitmap && (!rti->a4_bitmap_rendered_once || !rti->freeze))
		{
			all_set_transparent_palette_index(rti->transparency_index);
			all_render_a5_bitmap(rti->a4_bitmap, rti->bitmap);
			rti->a4_bitmap_rendered_once = true;
		}

		int w = al_get_bitmap_width(rti->bitmap);
		int h = al_get_bitmap_height(rti->bitmap);

		auto& matrix = rti->get_transform_matrix();
		auto [x0, y0] = matrix.apply(0, 0);
		auto [x1, y1] = matrix.apply(w, h);
		int tw = x1 - x0;
		int th = y1 - y0;
		if (rti->tint)
		{
			al_draw_scaled_bitmap(rti->bitmap, 0, 0, w, h, x0, y0, tw, th, 0);
			al_draw_tinted_scaled_bitmap(rti->bitmap, *rti->tint, 0, 0, w, h, x0, y0, tw, th, 0);
		}
		else
		{
			al_draw_scaled_bitmap(rti->bitmap, 0, 0, w, h, x0, y0, tw, th, 0);
		}
	}

	for (auto rti_child : rti->get_children())
	{
		render_tree_draw_item(rti_child);
	}
}

static void render_tree_draw_item_debug(RenderTreeItem* rti, int depth, std::vector<std::string>& lines)
{
	std::string line;
	line += fmt::format("{:>{}}", "", depth * 4);
	line += fmt::format(" > {} ", rti->name);
	if (!rti->visible)
		line += "[HIDDEN] ";
	if (rti->bitmap)
	{
		int w = al_get_bitmap_width(rti->bitmap);
		int h = al_get_bitmap_height(rti->bitmap);
		line += fmt::format("[BITMAP {}x{}] ", w, h);
		if (rti->freeze)
			line += "[FROZEN] ";
		if (rti->tint)
		{
			unsigned char r, g, b, a;
			al_unmap_rgba(*rti->tint, &r, &g, &b, &a);
			line += fmt::format("[TINT rgba {} {} {} {}] ", r, g, b, a);
		}
	}
	lines.push_back(line);

	for (auto rti_child : rti->get_children())
	{
		render_tree_draw_item_debug(rti_child, depth + 1, lines);
	}
}

void render_tree_draw(RenderTreeItem* rti)
{
	render_tree_draw_item(rti);
}

void render_tree_draw_debug(RenderTreeItem* rti)
{
	std::vector<std::string> lines;
	ALLEGRO_FONT* a5font = get_zc_font_a5(font_lfont_l);
	render_tree_draw_item_debug(rti, 0, lines);
	int font_scale = 4;
	render_text_lines(a5font, lines, TextJustify::left, TextAlign::top, font_scale);
}

static bool render_debug;
void render_set_debug(bool debug)
{
	render_debug = debug;
}
bool render_get_debug()
{
	return render_debug;
}

namespace MouseSprite
{
	static int active_mouse_sprite = -1;
	ALLEGRO_MOUSE_CURSOR* zc_mouse_sprites[MAX_MOUSESPRITE] = {nullptr};
	ALLEGRO_MOUSE_CURSOR* nullmouse = nullptr;
	static void set_nullmouse()
	{
		if(!nullmouse)
		{
			static char mouse_arrow_data[16*16] =
			{
				2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0,
				2, 1, 1, 1, 1, 1, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 1, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 2, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 2, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0
			};
			ALLEGRO_COLOR white = al_map_rgb(255,255,255);
			ALLEGRO_COLOR black = al_map_rgb(0,0,0);
			ALLEGRO_BITMAP* bmp = al_create_bitmap(16,16);
			auto* lock = al_lock_bitmap(bmp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
			for(int y = 0; y < 16; ++y)
			{
				for(int x = 0; x < 16; ++x)
				{
					ALLEGRO_COLOR* col = nullptr;
					switch(mouse_arrow_data[x+y*16])
					{
						case 1: col = &white; break;
						case 2: col = &black; break;
					}
					if(col)
						al_put_pixel(x, y, *col);
				}
			}
			al_unlock_bitmap(bmp);
			nullmouse = al_create_mouse_cursor(bmp, 1, 1);
			al_destroy_bitmap(bmp);
		}
		active_mouse_sprite = -1;
		al_show_mouse_cursor(all_get_display());
		al_set_mouse_cursor(all_get_display(), nullmouse);
	}
	void assign(int index, BITMAP* spr, int xf, int yf)
	{
		if(index < 0 || index >= MAX_MOUSESPRITE)
			return;
		ALLEGRO_MOUSE_CURSOR* old_cursor = zc_mouse_sprites[index];
		all_set_transparent_palette_index(0);
		ALLEGRO_BITMAP* a5_mouse_sprite = all_get_a5_bitmap(spr);
		zc_mouse_sprites[index] = al_create_mouse_cursor(a5_mouse_sprite, xf, yf);
		al_destroy_bitmap(a5_mouse_sprite);
		
		if(index == active_mouse_sprite)
		{
			active_mouse_sprite = -2;
			set(index);
		}
		if(old_cursor)
			al_destroy_mouse_cursor(old_cursor);
	}
	bool set(int index)
	{
		if(index < 0 || index >= MAX_MOUSESPRITE)
		{
			active_mouse_sprite = -1;
			set_nullmouse();
			return true;
		}
		if(index == active_mouse_sprite) return true;
		if(zc_mouse_sprites[index])
		{
			active_mouse_sprite = index;
			al_show_mouse_cursor(all_get_display());
			al_set_mouse_cursor(all_get_display(), zc_mouse_sprites[index]);
			return true;
		}
		return false;
	}
	void clear(int index)
	{
		if(index < 0 || index >= MAX_MOUSESPRITE)
			return;
		if(zc_mouse_sprites[index])
			al_destroy_mouse_cursor(zc_mouse_sprites[index]);
		zc_mouse_sprites[index] = nullptr;
	}
}

BITMAP* zqdialog_bg_bmp = nullptr;
static RenderTreeItem* active_dlg_rti = nullptr;
void popup_zqdialog_start(int x, int y, int w, int h, int transp)
{
	if(w < 0) w = zq_screen_w;
	if(h < 0) h = zq_screen_h;
	if(!zqdialog_bg_bmp)
		zqdialog_bg_bmp = screen;
	BITMAP* tmp_bmp = create_bitmap_ex(8, w, h);
	
	if(tmp_bmp)
	{
		if(transp > 0)
			clear_to_color(tmp_bmp, transp);
		else clear_bitmap(tmp_bmp);
		screen = tmp_bmp;
		
		RenderTreeItem* rti = new RenderTreeItem("zqdialog");
		set_bitmap_create_flags(false);
		rti->bitmap = create_a5_bitmap(w, h);
		rti->a4_bitmap = tmp_bmp;
		rti->transparency_index = transp;
		rti->set_transform({.x = x, .y = y});
		rti->visible = true;
		rti->owned = true;
		rti_dialogs.add_child(rti);
		rti_dialogs.visible = true;
		active_dlg_rti = rti;
		al_set_new_bitmap_flags(0);
	}
	else
	{
		*allegro_errno = ENOMEM;
	}
}

void popup_zqdialog_end()
{
	if (active_dlg_rti)
	{
		RenderTreeItem* to_del = active_dlg_rti;
		rti_dialogs.remove_child(to_del);
		if(rti_dialogs.has_children())
		{
			active_dlg_rti = rti_dialogs.get_children().back();
			screen = active_dlg_rti->a4_bitmap;
		}
		else
		{
			active_dlg_rti = nullptr;
			screen = zqdialog_bg_bmp;
			zqdialog_bg_bmp = nullptr;
		}
		delete to_del;
	}
	position_mouse_z(0);
}

static std::vector<ALLEGRO_STATE> old_a5_states;
void popup_zqdialog_start_a5()
{
	if(!zqdialog_bg_bmp)
		zqdialog_bg_bmp = screen;
	
	RenderTreeItem* rti = new RenderTreeItem("zqdialog_a5");
	set_bitmap_create_flags(true);
	rti->bitmap = create_a5_bitmap(zq_screen_w, zq_screen_h);
	rti->visible = true;
	rti->owned = true;
	rti_dialogs.add_child(rti);
	rti_dialogs.visible = true;
	active_dlg_rti = rti;
	al_set_new_bitmap_flags(0);
	
	old_a5_states.emplace_back();
	ALLEGRO_STATE& oldstate = old_a5_states.back();
	al_store_state(&oldstate, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(rti->bitmap);
}

void popup_zqdialog_end_a5()
{
	if (active_dlg_rti && old_a5_states.size())
	{
		RenderTreeItem* to_del = active_dlg_rti;
		rti_dialogs.remove_child(to_del);
		if(rti_dialogs.has_children())
			active_dlg_rti = rti_dialogs.get_children().back();
		else
		{
			active_dlg_rti = nullptr;
			zqdialog_bg_bmp = nullptr;
		}
		ALLEGRO_STATE& oldstate = old_a5_states.back();
		al_restore_state(&oldstate);
		old_a5_states.pop_back();

		delete to_del;
	}
	position_mouse_z(0);
}

RenderTreeItem* add_dlg_layer(int x, int y, int w, int h)
{
	if(!active_dlg_rti) return nullptr;
	if(w<0) w = screen->w-x;
	if(h<0) h = screen->h-y;
	set_bitmap_create_flags(true);
	
	RenderTreeItem* rti = new RenderTreeItem("dlg");
	rti->bitmap = al_create_bitmap(w,h);
	clear_a5_bmp(rti->bitmap);
	rti->set_transform({.x = x, .y = y});
	rti->a4_bitmap = nullptr;
	rti->visible = true;
	rti->owned = true;
	active_dlg_rti->add_child(rti);

	al_set_new_bitmap_flags(0);
	return rti;
}
void remove_dlg_layer(RenderTreeItem* rti)
{
	if(active_dlg_rti)
	{
		active_dlg_rti->remove_child(rti);
	}
	delete rti;
}

ALLEGRO_COLOR dialog_tint = al_premul_rgba(0, 0, 0, 64);
ALLEGRO_COLOR* override_dlg_tint = nullptr;
static size_t dlg_tint_pause = 0;
void reload_dialog_tints()
{
	std::vector<RenderTreeItem*>& children = rti_dialogs.get_children();
	if(children.empty()) return;
	for(size_t q = 0; q < children.size()-1; ++q)
	{
		children[q]->tint = dlg_tint_pause ? nullptr :
			(override_dlg_tint ? override_dlg_tint : &dialog_tint);
	}
	children.back()->tint = nullptr;
}
ALLEGRO_COLOR& get_dlg_tint()
{
	return override_dlg_tint ? *override_dlg_tint : dialog_tint;
}
void pause_dlg_tint(bool pause)
{
	if(pause)
		++dlg_tint_pause;
	else if(dlg_tint_pause)
		--dlg_tint_pause;
}
bool dlg_tint_paused()
{
	return dlg_tint_pause;
}


static std::atomic<bool> throttle_counter;
void update_throttle_counter()
{
	throttle_counter.store(true, std::memory_order_relaxed);
}
END_OF_FUNCTION(update_throttle_counter)

// https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/
static void preciseThrottle(double seconds)
{
	static double estimate = 5e-3;
	static double mean = 5e-3;
	static double m2 = 0;
	static int64_t count = 1;

	while (seconds > estimate) {
		auto start = std::chrono::high_resolution_clock::now();
		rest(1);
		auto end = std::chrono::high_resolution_clock::now();

		double observed = (end - start).count() / 1e9;
		seconds -= observed;

		++count;
		double delta = observed - mean;
		mean += delta / count;
		m2   += delta * (observed - mean);
		double stddev = sqrt(m2 / (count - 1));
		estimate = mean + stddev;
	}

	// spin lock
#ifdef __EMSCRIPTEN__
	while (!throttle_counter.load(std::memory_order_relaxed))
	{
		volatile int i = 0;
		while (i < 10000000)
		{
			if (throttle_counter.load(std::memory_order_relaxed)) return;
			i += 1;
		}

		rest(1);
	}
#else
	while(!throttle_counter.load(std::memory_order_relaxed));
#endif
}

void throttleFPS(bool throttle)
{
	static auto last_time = std::chrono::high_resolution_clock::now();

	if( throttle )
	{
		if (!throttle_counter.load(std::memory_order_relaxed))
		{
			int freq = 60;
			double target = 1.0 / freq;
			auto now_time = std::chrono::high_resolution_clock::now();
			double delta = (now_time - last_time).count() / 1e9;
			if (delta < target)
				preciseThrottle(target - delta);
		}
	}

	throttle_counter.store(false, std::memory_order_relaxed);
	last_time = std::chrono::high_resolution_clock::now();
}
