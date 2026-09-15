#include "render.h"
#include "a5alleg.h"
#include "allegro5/bitmap.h"
#include "allegro5/display.h"
#include "base/zapp.h"
#include "core/zdefs.h"
#include "core/fonts.h"
#include <fmt/format.h>
#include "gui/jwin_a5.h"
#include <chrono>
#include <map>

using namespace std::chrono_literals;

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

int get_bitmap_create_flags(bool preserve_texture)
{
	int flags = ALLEGRO_CONVERT_BITMAP;
	if(!preserve_texture)
		flags |= ALLEGRO_NO_PRESERVE_TEXTURE;
	if (use_linear_bitmaps())
		flags |= ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR;
	return flags;
}

void set_bitmap_create_flags(bool preserve_texture)
{
	al_set_new_bitmap_flags(get_bitmap_create_flags(preserve_texture));
}

void clear_a5_bmp(ALLEGRO_COLOR col, ALLEGRO_BITMAP* bmp)
{
	if (is_headless())
		return;

	if(bmp)
	{
		ALLEGRO_STATE old_state;
		al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP);

		zc_set_target_bitmap(bmp);

		al_clear_to_color(col);

		al_restore_state(&old_state);
	}
	else
	{
		// Clearing whatever is already bound - the tree can't see that either.
		render_mark_dirty();
		al_clear_to_color(col);
	}
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

ALLEGRO_COLOR a5colors[256];
uint32_t zc_backend_palette[256];
static int backend_fmt = ALLEGRO_PIXEL_FORMAT_ABGR_8888;
void _init_render(int fmt)
{
	backend_fmt = fmt;
}
uint32_t get_backend_a5_col(RGB const& c)
{
	unsigned char r = c.r, g = c.g, b = c.b, a = 255;
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
	set_bitmap_create_flags(true);
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

void render_text(ALLEGRO_BITMAP* bitmap, ALLEGRO_FONT* font, const std::string& text, int x, int y, int scale, ALLEGRO_COLOR color, ALLEGRO_COLOR bgcolor)
{
	ALLEGRO_STATE oldstate;
	al_store_state(&oldstate, ALLEGRO_STATE_TARGET_BITMAP);

	int resx = al_get_bitmap_width(bitmap);
	int w = al_get_text_width(font, text.c_str());
	int h = al_get_font_line_height(font);

	static ALLEGRO_BITMAP* text_bitmap;
	if (text_bitmap == nullptr || resx != al_get_bitmap_width(text_bitmap) || h != al_get_bitmap_height(text_bitmap))
	{
		if (text_bitmap)
			al_destroy_bitmap(text_bitmap);
		set_bitmap_create_flags(true);
		text_bitmap = al_create_bitmap(resx, h);
	}

	al_set_target_bitmap(text_bitmap);
	al_clear_to_color(al_map_rgba(0, 0, 0, 0));
	al_draw_filled_rectangle(0, 0, w, h, bgcolor);
	al_draw_text(font, color, 0, 0, 0, text.c_str());

	al_set_target_bitmap(bitmap);
	al_draw_scaled_bitmap(text_bitmap,
		0, 0,
		al_get_bitmap_width(text_bitmap), al_get_bitmap_height(text_bitmap),
		x, y,
		al_get_bitmap_width(text_bitmap) * scale, al_get_bitmap_height(text_bitmap) * scale,
		0
	);
	al_restore_state(&oldstate);
}

void render_text_lines(ALLEGRO_BITMAP* bitmap, ALLEGRO_FONT* font, const std::vector<std::string>& lines, TextJustify justify, TextAlignment align, int scale)
{
	int resx = al_get_bitmap_width(bitmap);
	int resy = al_get_bitmap_height(bitmap);
	int font_height = al_get_font_line_height(font);
	int text_y = align == TextAlignment::bottom ?
		resy - scale*font_height - 5 :
		// Offset just a bit so it doesn't obscure the title bar.
		resy*0.04;
	for (std::string line : lines)
	{
		int x = justify == TextJustify::left ?
			5 :
			resx - al_get_text_width(font, line.c_str())*scale - 5;
		render_text(bitmap, font, line, x, text_y, scale, al_map_rgb_f(1, 1, 1), al_map_rgba_f(0, 0, 0, 0.6));
		text_y += (scale*font_height + 3) * (align == TextAlignment::bottom ? -1 : 1);
	}
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
			set_bitmap_create_flags(true);
			ALLEGRO_BITMAP* bmp = al_create_bitmap(16,16);
			al_lock_bitmap(bmp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
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
		all_set_transparent_palette_index(-1);
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
	void set_link_hover(bool hover)
	{
		static bool active = false;
		if(hover == active)
			return;
		active = hover;
		if(hover)
			al_set_system_mouse_cursor(all_get_display(), ALLEGRO_SYSTEM_MOUSE_CURSOR_LINK);
		else
		{
			// Reapply whatever mouse sprite was active before the hover.
			int index = active_mouse_sprite;
			active_mouse_sprite = -2;
			if(index < 0 || !set(index))
				set(-1);
		}
	}
}

static int last_render_timer_freq = -1;
static ALLEGRO_MUTEX* render_timer_mutex;
static ALLEGRO_COND* render_timer_cond;
static ALLEGRO_MUTEX* fake_vsync_timer_mutex;
static ALLEGRO_COND* fake_vsync_timer_cond;
static volatile int32_t render_timer_counter;
static volatile int32_t fake_vsync_counter;

static void render_timer_callback()
{
	al_lock_mutex(render_timer_mutex);
	render_timer_counter += 1;
	al_signal_cond(render_timer_cond);
	al_unlock_mutex(render_timer_mutex);
}

static void fake_vsync_callback()
{
	al_lock_mutex(fake_vsync_timer_mutex);
	fake_vsync_counter += 1;
	al_signal_cond(fake_vsync_timer_cond);
	al_unlock_mutex(fake_vsync_timer_mutex);
}

bool render_timer_start(int freq)
{
	if (is_headless())
		return true;

	if (freq == 0)
		freq = 10000;
	freq = std::clamp(freq, 1, 10000);

	static bool has_done_setup;
	if (!has_done_setup)
	{
		render_timer_mutex = al_create_mutex();
		if (!render_timer_mutex)
			return false;

		render_timer_cond = al_create_cond();
		if (!render_timer_cond)
			return false;

		fake_vsync_timer_mutex = al_create_mutex();
		if (!fake_vsync_timer_mutex)
			return false;

		fake_vsync_timer_cond = al_create_cond();
		if (!fake_vsync_timer_cond)
			return false;

		has_done_setup = true;
	}

	if (freq == last_render_timer_freq)
		return true;

	if (freq > 60)
	{
		if (install_int_ex(fake_vsync_callback, BPS_TO_TIMER(60)))
			return false;
	}
	else
	{
		remove_int(fake_vsync_callback);
	}

	if (install_int_ex(render_timer_callback, BPS_TO_TIMER(freq)))
		return false;

	last_render_timer_freq = freq;
	return true;
}

void render_timer_wait()
{
	al_lock_mutex(render_timer_mutex);
	while (render_timer_counter == 0)
		al_wait_cond(render_timer_cond, render_timer_mutex);
	render_timer_counter = 0;
	al_unlock_mutex(render_timer_mutex);
}

bool render_fake_vsync_check()
{
	if (last_render_timer_freq < 60)
		return true;

	al_lock_mutex(fake_vsync_timer_mutex);
	bool vsync = fake_vsync_counter > 0;
	fake_vsync_counter = 0;
	al_unlock_mutex(fake_vsync_timer_mutex);
	return vsync;
}
