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

static int zc_mouse_x, zc_mouse_y;

// Bumped whenever content the render tree composites changes in a way the tree itself
// can't observe: an a4->a5 conversion doing work, direct a5 drawing into a dialog's
// bitmap, or tree structure changes. render_tree_draw_and_flip uses it to know a new
// frame must be presented.
static uint64_t render_dirty_count = 1;
void render_mark_dirty()
{
	render_dirty_count++;
}

// Bumped by the periodic safety refresh below; a LegacyBitmapRTI content-hash input, so
// bumping it forces every legacy bitmap to re-convert.
static uint64_t render_refresh_count;

// Once a second, force the legacy bitmaps to re-convert (and, where frames can be
// skipped, a fresh frame to be presented) even when nothing appears to have changed.
// This bounds the damage of any missed invalidation - an untracked draw path, or a hash
// blind spot - to a second of lag instead of a permanently stale window. Every drawing
// entry point ticks it, so no app is left without the safety net. Returns whether this
// call is the forced refresh.
static bool render_refresh_tick()
{
	static double last_refresh_time = 0;
	double now = al_get_time();
	if (now - last_refresh_time < 1.0)
		return false;

	last_refresh_time = now;
	render_refresh_count++;
	return true;
}

void zc_set_target_bitmap(ALLEGRO_BITMAP* bitmap)
{
	render_mark_dirty();
	al_set_target_bitmap(bitmap);
}

static inline uint64_t rotl64(uint64_t x, int r)
{
	return (x << r) | (x >> (64 - r));
}

// MurmurHash3 (x64/128 variant) body and finalizer, two 8-byte lanes per step.
// The mixing here matters: a multiply-only construction (e.g. chained FNV) provably
// loses single-byte differences when per-row hashes are chained - a multiply can only
// move difference bits upward, where they truncate away, and a lane merge cancels what
// is left - so a stale frame can hash as "unchanged" and be skipped. Murmur's rotations
// spread differences across all bits, and the finalizer fully avalanches the result,
// so chaining per-row hashes is safe.
static uint64_t hash_bytes(const void* data, size_t len, uint64_t h)
{
	constexpr uint64_t c1 = 0x87c37b91114253d5ULL;
	constexpr uint64_t c2 = 0x4cf5ad432745937fULL;
	uint64_t h0 = h;
	uint64_t h1 = h ^ 0x9e3779b97f4a7c15ULL;
	const uint8_t* p = (const uint8_t*)data;
	size_t n = len / 16;
	for (size_t i = 0; i < n; i++)
	{
		uint64_t k0, k1;
		memcpy(&k0, p, 8);
		memcpy(&k1, p + 8, 8);
		p += 16;
		k0 *= c1; k0 = rotl64(k0, 31); k0 *= c2; h0 ^= k0;
		h0 = rotl64(h0, 27); h0 += h1; h0 = h0 * 5 + 0x52dce729;
		k1 *= c2; k1 = rotl64(k1, 33); k1 *= c1; h1 ^= k1;
		h1 = rotl64(h1, 31); h1 += h0; h1 = h1 * 5 + 0x38495ab5;
	}
	for (size_t i = n * 16; i < len; i++)
	{
		h0 ^= p[i - n * 16];
		h0 = rotl64(h0, 27) * 5 + 0x52dce729;
	}
	h = h0 ^ h1;
	h ^= len;
	h ^= h >> 33; h *= 0xff51afd7ed558ccdULL;
	h ^= h >> 33; h *= 0xc4ceb9fe1a85ec53ULL;
	h ^= h >> 33;
	return h;
}

static uint64_t hash_value(uint64_t v, uint64_t h)
{
	h ^= v;
	h ^= h >> 33; h *= 0xff51afd7ed558ccdULL;
	h ^= h >> 33;
	return h;
}

// The a4 pixel hash dominates the cost of the conversion check, and render items can share
// one a4 bitmap - in the launcher `rti_screen` and the active dialog's layer are literally
// the same BITMAP, because init_render_tree runs after popup_zqdialog_start has already
// repointed `screen`. Hash each distinct bitmap once per pass and reuse the result.
static std::vector<std::pair<BITMAP*, uint64_t>> a4_pixel_hashes;

static void a4_pixel_hash_begin_pass()
{
	a4_pixel_hashes.clear();
}

static uint64_t a4_pixel_hash(BITMAP* bmp)
{
	for (auto const& [cached, cached_hash] : a4_pixel_hashes)
		if (cached == bmp)
			return cached_hash;

	uint64_t hash = hash_value(((uint64_t)(uint32_t)bmp->w << 32) | (uint32_t)bmp->h, 0xcbf29ce484222325ULL);
	size_t row_bytes = (size_t)bmp->w * BYTES_PER_PIXEL(bitmap_color_depth(bmp));
	for (int y = 0; y < bmp->h; y++)
		hash = hash_bytes(bmp->line[y], row_bytes, hash);

	a4_pixel_hashes.emplace_back(bmp, hash);
	return hash;
}

void RenderTreeItem::remove()
{
	if (parent)
		parent->remove_child(this);
}
void RenderTreeItem::add_child(RenderTreeItem* child)
{
	if (child->parent == this && child == children.back())
		return;

	if (child->parent)
		child->parent->remove_child(child);
	children.push_back(child);
	child->parent = this;
	child->mark_transform_dirty();
	render_mark_dirty();
}
void RenderTreeItem::add_child_before(RenderTreeItem* child, RenderTreeItem* before_child)
{
	bool already_child = child->parent == this;
	if (already_child)
	{
		// Already in the requested position - avoid pointless churn (this is called
		// every frame for the dialog tint).
		auto it = std::find(children.begin(), children.end(), before_child);
		if (it != children.begin() && *std::prev(it) == child)
			return;
	}
	if (child->parent)
		child->parent->remove_child(child);
	auto it = std::find(children.begin(), children.end(), before_child);
	ASSERT(it != children.end());
	children.insert(it, child);
	child->parent = this;
	if (!already_child)
		child->mark_transform_dirty();
	render_mark_dirty();
}
void RenderTreeItem::remove_child(RenderTreeItem* child)
{
	auto it = std::find(children.begin(), children.end(), child);
	if (it != children.end())
	{
		children.erase(it);
		child->parent = nullptr;
		child->mark_transform_dirty();
		render_mark_dirty();
	}
}
void RenderTreeItem::remove_children()
{
	for (auto it = children.begin(); it != children.end(); it++)
	{
		(*it)->parent = nullptr;
	}
	children.clear();
	render_mark_dirty();
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
bool RenderTreeItem::has_parent() const
{
	return !!parent;
}
void RenderTreeItem::set_size(int width, int height)
{
	this->width = width;
	this->height = height;
}
void RenderTreeItem::handle_dirty_transform()
{
	if (!transform_dirty) return;

	const Matrix& parent_transform = parent ? parent->get_transform_matrix() : Matrix::Identity();
	transform_matrix = Matrix::Translate(transform.x, transform.y).mul(Matrix::Scale(transform.xscale, transform.yscale));
	transform_matrix = parent_transform.mul(transform_matrix);
	transform_dirty = false;
}
void RenderTreeItem::mark_transform_dirty()
{
	transform_dirty = transform_inverse_dirty = true;
	for (auto child : children) child->mark_transform_dirty();
}
void RenderTreeItem::set_transform(Transform new_transform)
{
	if (transform == new_transform)
		return;

	transform = new_transform;
	mark_transform_dirty();
}
const Transform& RenderTreeItem::get_transform() const
{
	return transform;
}
const Matrix& RenderTreeItem::get_transform_matrix()
{
	handle_dirty_transform();
	return transform_matrix;
}
std::pair<int, int> RenderTreeItem::world_to_local(int x, int y)
{
	handle_dirty_transform();
	if (transform_inverse_dirty)
	{
		transform_matrix_inverse = transform_matrix.inverse();
		transform_inverse_dirty = false;
	}
	auto [lx, ly] = transform_matrix_inverse.apply(x, y);
	if (uv_warp && width > 0 && height > 0)
	{
		auto [u, v] = uv_warp((lx + 0.5) / width, (ly + 0.5) / height);
		lx = (int)std::floor(u * width);
		ly = (int)std::floor(v * height);
	}
	return {lx, ly};
}
std::pair<int, int> RenderTreeItem::local_to_world(int x, int y)
{
	handle_dirty_transform();
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

RenderTreeItem::RenderTreeItem(std::string name, RenderTreeItem* parent) : name(name), parent(parent)
{
}

RenderTreeItem::~RenderTreeItem()
{
	if(owned)
	{
		if(bitmap)
			al_destroy_bitmap(bitmap);
	}
	if(owned_tint)
	{
		if(tint)
			delete tint;
	}
	for(RenderTreeItem* child : children)
	{
		child->parent = nullptr;
		if (child->owned)
			delete child;
	}
}

static void render_tree_draw_item_prepare(RenderTreeItem* rti)
{
	rti->prepare();
	if (!rti->visible)
		return;

	for (auto rti_child : rti->get_children())
	{
		render_tree_draw_item_prepare(rti_child);
	}
}

// Draws `rti` and its descendants into the current target bitmap, in that bitmap's own
// coordinate space: (x, y) is where rti's origin lands, (xs, ys) the accumulated scale.
static void render_tree_bake_item(RenderTreeItem* rti, float x, float y, float xs, float ys)
{
	if (!rti->visible)
		return;

	auto& t = rti->get_transform();
	float bx = x + t.x * xs;
	float by = y + t.y * ys;
	float bxs = xs * t.xscale;
	float bys = ys * t.yscale;
	if (rti->bitmap)
	{
		int w = al_get_bitmap_width(rti->bitmap);
		int h = al_get_bitmap_height(rti->bitmap);
		al_draw_scaled_bitmap(rti->bitmap, 0, 0, w, h, bx, by, w * bxs, h * bys, 0);
	}
	for (auto child : rti->get_children())
		render_tree_bake_item(child, bx, by, bxs, bys);
}

// Whether children will be drawn into this item's own bitmap after it renders (see
// render_tree_bake_children) - which means its contents are not a function of its render
// inputs alone, so nothing may cache its way out of the redraw.
static bool bakes_children(RenderTreeItem* rti)
{
	if (!rti->shader)
		return false;
	for (auto child : rti->get_children())
	{
		if (child->fullres_overlay && rti->overlay_shader)
			continue;
		return true;
	}
	return false;
}

// A shader draw covers only the item's own bitmap, so children drawn separately on top of it
// escape the shader - the title logo stayed crisp and flat over a scanlined, curved game.
// Bake them into the item's bitmap right after it renders, so they pass through the same
// shader (and warp geometry) as everything under them. Called only while the item is
// unfrozen, which also means a frozen layer keeps its children frozen in place with it.
static void render_tree_bake_children(RenderTreeItem* rti)
{
	ALLEGRO_STATE oldstate;
	al_store_state(&oldstate, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_BLENDER);
	al_set_target_bitmap(rti->bitmap);
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
	for (auto child : rti->get_children())
	{
		// Full-resolution children get their own overlay pass at draw time instead.
		if (child->fullres_overlay && rti->overlay_shader)
			continue;
		render_tree_bake_item(child, 0, 0, 1, 1);
	}
	al_restore_state(&oldstate);
}

static void render_tree_draw_item(RenderTreeItem* rti, bool do_a4_only)
{
	if (!rti->visible)
		return;

	// Two passes over the tree: the legacy items convert their a4 bitmaps in the first
	// (drawing only into their own a5 bitmap), everything renders and draws to the screen
	// in the second. An item does its work in exactly one of them.
	bool skip = rti->wants_a4_pass() != do_a4_only;

	// Only a sized item's bitmap is the framework's to manage; a display-only item (no size
	// set, bitmap supplied from outside) is left exactly as given.
	bool managed = rti->width > 0 && rti->height > 0;
	if (!skip && !rti->freeze && managed)
	{
		bool size_changed = false;
		int flags = rti->bitmap_flags;
		if (rti->bitmap && (al_get_bitmap_width(rti->bitmap) != rti->width || al_get_bitmap_height(rti->bitmap) != rti->height))
		{
			flags = al_get_bitmap_flags(rti->bitmap);
			al_destroy_bitmap(rti->bitmap);
			rti->bitmap = nullptr;
			size_changed = true;
		}
		if (!rti->bitmap && rti->width > 0 && rti->height > 0)
		{
			if (flags == -1)
				flags = get_bitmap_create_flags(true);
			al_set_new_bitmap_flags(flags);
			rti->bitmap = create_a5_bitmap(rti->width, rti->height);
			rti->dirty = true;
		}

		if (rti->dirty && rti->bitmap)
		{
			rti->dirty = false;
			if (do_a4_only)
			{
				// Special case, we don't need to change the target bitmap or clear it to
				// convert from a4 to a5 bitmaps.
				rti->render(size_changed);
			}
			else
			{
				al_set_target_bitmap(rti->bitmap);
				al_clear_to_color(al_map_rgba(0, 0, 0, 0));
				rti->render(size_changed);
				al_set_target_backbuffer(all_get_display());
			}
			if (bakes_children(rti))
				render_tree_bake_children(rti);
		}
	}

	skip = do_a4_only;
	if (!skip && rti->bitmap)
	{
		int w = al_get_bitmap_width(rti->bitmap);
		int h = al_get_bitmap_height(rti->bitmap);

		auto& matrix = rti->get_transform_matrix();
		auto [x0, y0] = matrix.apply_f(0, 0);
		auto [x1, y1] = matrix.apply_f(w, h);
		float tw = x1 - x0;
		float th = y1 - y0;
		bool shader_active = false;
		if (rti->shader)
		{
			shader_active = al_use_shader(rti->shader);
			if (shader_active && rti->shader_prepare)
				rti->shader_prepare(rti, (int)std::lround(tw), (int)std::lround(th));
		}
		if (rti->tint)
		{
			al_draw_tinted_scaled_bitmap(rti->bitmap, *rti->tint, 0, 0, w, h, x0, y0, tw, th, 0);
		}
		else
		{
			al_draw_scaled_bitmap(rti->bitmap, 0, 0, w, h, x0, y0, tw, th, 0);
		}
		if (shader_active)
			al_use_shader(nullptr);

		// Full-resolution children of a shader-drawn item: each is composited by its own
		// overlay pass covering the parent's whole output rect, so the overlay shader can
		// keep its scanline/curvature geometry identical to the parent pass while sampling
		// the child's bitmap at full resolution.
		if (rti->shader && rti->overlay_shader)
		{
			for (auto child : rti->get_children())
			{
				if (!child->fullres_overlay || !child->visible || !child->bitmap)
					continue;
				bool overlay_active = al_use_shader(rti->overlay_shader);
				if (overlay_active && rti->overlay_prepare)
					rti->overlay_prepare(rti, child, (int)std::lround(tw), (int)std::lround(th));
				int cw = al_get_bitmap_width(child->bitmap);
				int ch = al_get_bitmap_height(child->bitmap);
				if (rti->tint)
					al_draw_tinted_scaled_bitmap(child->bitmap, *rti->tint, 0, 0, cw, ch, x0, y0, tw, th, 0);
				else
					al_draw_scaled_bitmap(child->bitmap, 0, 0, cw, ch, x0, y0, tw, th, 0);
				if (overlay_active)
					al_use_shader(nullptr);
			}
		}
	}

	// A shader-drawn item's children were baked into its bitmap when it rendered (or drawn
	// by an overlay pass above) - drawing them here too would double them up, unshaded.
	if (rti->shader)
		return;

	for (auto rti_child : rti->get_children())
	{
		// Only the on-screen pass draws anything the clip could apply to.
		if (!do_a4_only && rti_child->clip_to_parent && rti->bitmap)
		{
			int cx, cy, cw, ch;
			al_get_clipping_rectangle(&cx, &cy, &cw, &ch);

			// The same corner mapping the draw above uses for the parent's own rect,
			// rounded outward so the clip never shaves a partially-covered pixel.
			auto& matrix = rti->get_transform_matrix();
			auto [fx0, fy0] = matrix.apply_f(0, 0);
			auto [fx1, fy1] = matrix.apply_f(al_get_bitmap_width(rti->bitmap), al_get_bitmap_height(rti->bitmap));

			int nx0 = std::max(cx, (int)std::floor(fx0));
			int ny0 = std::max(cy, (int)std::floor(fy0));
			int nx1 = std::min(cx + cw, (int)std::ceil(fx1));
			int ny1 = std::min(cy + ch, (int)std::ceil(fy1));
			al_set_clipping_rectangle(nx0, ny0, std::max(0, nx1 - nx0), std::max(0, ny1 - ny0));
			render_tree_draw_item(rti_child, do_a4_only);
			al_set_clipping_rectangle(cx, cy, cw, ch);
		}
		else
			render_tree_draw_item(rti_child, do_a4_only);
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
		line += fmt::format("[{}x{}] ", rti->width, rti->height);
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
	render_refresh_tick();
	render_tree_draw_item_prepare(rti);
	// Convert the legacy bitmaps first, then draw. The split exists for
	// render_tree_draw_and_flip, which must run the conversions before it can tell whether
	// the frame changed at all; it also keeps the texture uploads from interleaving with the
	// draw calls.
	a4_pixel_hash_begin_pass();
	render_tree_draw_item(rti, true);
	render_tree_draw_item(rti, false);

	// Ensure the mouse coordinates used to setup the frame are the same that rti prepare/render functions will utilize
	// by storing the mouse coordinates, and fetching them _after_ the above rti drawing.
	// ex: tooltips
	zc_mouse_x = gui_mouse_x();
	zc_mouse_y = gui_mouse_y();
}

// Everything the composited output depends on that the dirty counter doesn't already
// capture: tree structure/order, visibility, bitmap identity, sizes, transforms, tints.
// Also reports whether any visible item has a redraw pending for the upcoming draw pass
// (e.g. the editor minimap marks itself dirty when the map changes) - such a frame must
// be drawn and presented. Legacy a4 items don't factor in here: the conversion pass
// already ran, clearing their dirty flag and bumping the dirty counter if they changed.
static uint64_t render_tree_signature_item(RenderTreeItem* rti, uint64_t h, bool& pending_render)
{
	h = hash_value((uintptr_t)rti, h);
	h = hash_value(rti->visible, h);
	if (!rti->visible)
		return h;

	if (rti->dirty && !rti->freeze && rti->width > 0 && rti->height > 0)
		pending_render = true;

	h = hash_value((uintptr_t)rti->bitmap, h);
	h = hash_value(((uint64_t)(uint32_t)rti->width << 32) | (uint32_t)rti->height, h);
	auto& t = rti->get_transform();
	h = hash_bytes(&t, sizeof(t), h);
	if (rti->tint)
		h = hash_bytes(rti->tint, sizeof(*rti->tint), h);
	h = hash_value((uintptr_t)rti->shader, h);

	for (auto child : rti->get_children())
		h = render_tree_signature_item(child, h, pending_render);

	return h;
}

// -render-verify: a (slow) diagnostic mode that checks the frame-skipping logic every frame.
//
// The invariant behind skipping redrawing a frame is "the composited output is identical to the
// last presented frame" - so composite the tree to an offscreen bitmap every frame, hash the
// pixels, and log an error if a skipped frame's output differs from the frame on screen. Catches
// any draw path that forgot to mark itself dirty (see render_mark_dirty).
//
// Running in this mode after adding new GUI components is a good idea to catch places that are
// missing a call to render_mark_dirty.
static bool render_verify_enabled()
{
	static bool value = get_flag_bool("-render-verify").value_or(false);
	return value;
}

static void render_verify(RenderTreeItem* rti, ALLEGRO_COLOR clear_color, bool presented)
{
	ALLEGRO_DISPLAY* display = all_get_display();
	int w = al_get_display_width(display);
	int h = al_get_display_height(display);

	static ALLEGRO_BITMAP* verify_bitmap;
	static ALLEGRO_BITMAP* baseline_bitmap;
	if (verify_bitmap && (al_get_bitmap_width(verify_bitmap) != w || al_get_bitmap_height(verify_bitmap) != h))
	{
		al_destroy_bitmap(verify_bitmap);
		al_destroy_bitmap(baseline_bitmap);
		verify_bitmap = baseline_bitmap = nullptr;
	}
	if (!verify_bitmap)
	{
		set_bitmap_create_flags(true);
		verify_bitmap = al_create_bitmap(w, h);
		baseline_bitmap = al_create_bitmap(w, h);
		al_set_new_bitmap_flags(0);
	}

	// On a skipped frame no visible item has a redraw pending (that would have forced a
	// present), so this draw pass is free of side effects; on a presented frame the
	// pending work was already consumed by the real draw pass.
	ALLEGRO_STATE oldstate;
	al_store_state(&oldstate, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(verify_bitmap);
	al_clear_to_color(clear_color);
	render_tree_draw_item(rti, false);
	al_restore_state(&oldstate);

	uint64_t hash = 0xcbf29ce484222325ULL;
	ALLEGRO_LOCKED_REGION* lr = al_lock_bitmap(verify_bitmap, ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE, ALLEGRO_LOCK_READONLY);
	if (!lr)
		return;

	for (int y = 0; y < h; y++)
		hash = hash_bytes((uint8_t*)lr->data + (size_t)y * lr->pitch, (size_t)w * 4, hash);
	al_unlock_bitmap(verify_bitmap);

	static uint64_t presented_hash;
	static bool have_baseline;
	static bool prev_frame_failed;
	if (presented || !have_baseline)
	{
		presented_hash = hash;
		have_baseline = true;
		prev_frame_failed = false;
		ALLEGRO_STATE state;
		al_store_state(&state, ALLEGRO_STATE_TARGET_BITMAP);
		al_set_target_bitmap(baseline_bitmap);
		al_draw_bitmap(verify_bitmap, 0, 0, 0);
		al_restore_state(&state);
		return;
	}

	if (hash != presented_hash && !prev_frame_failed)
	{
		static int failures;
		failures++;
		fprintf(stderr, "[render-verify] FAIL #%d: skipped a frame whose output differs from the frame on screen - "
			"some draw path is missing a render_mark_dirty\n", failures);
		if (failures == 1)
		{
			al_save_bitmap("render_verify_stale.png", baseline_bitmap);
			al_save_bitmap("render_verify_expected.png", verify_bitmap);
			fprintf(stderr, "[render-verify] wrote render_verify_stale.png (on screen) and render_verify_expected.png (correct)\n");
		}
	}

	prev_frame_failed = hash != presented_hash;
}

bool render_tree_draw_and_flip(RenderTreeItem* rti, ALLEGRO_COLOR clear_color)
{
#ifdef __EMSCRIPTEN__
	// On web, presenting every frame is also what keeps input flowing: with frames skipped, mouse
	// events stop arriving altogether inside a modal loop (an open menu stops highlighting and
	// stops responding to clicks, while the keyboard still works).
	ALLEGRO_DISPLAY* display = all_get_display();
	al_set_target_backbuffer(display);
	al_clear_to_color(clear_color);
	render_tree_draw(rti);
	al_flip_display();
	return true;
#else
	// A forced refresh re-converts every legacy bitmap (the refresh counter is a content
	// hash input) and must present the result, so it also breaks out of the skip below.
	bool force_refresh = render_refresh_tick();

	render_tree_draw_item_prepare(rti);
	// Convert a4 bitmaps first - a conversion that does actual work marks the tree dirty.
	a4_pixel_hash_begin_pass();
	render_tree_draw_item(rti, true);

	ALLEGRO_DISPLAY* display = all_get_display();
	bool pending_render = false;
	uint64_t sig = 0xcbf29ce484222325ULL;
	sig = hash_value(render_dirty_count, sig);
	sig = hash_value(all_get_render_generation(), sig);
	sig = hash_value(((uint64_t)(uint32_t)al_get_display_width(display) << 32) | (uint32_t)al_get_display_height(display), sig);
	sig = hash_bytes(&clear_color, sizeof(clear_color), sig);
	sig = render_tree_signature_item(rti, sig, pending_render);

	// When nothing changed, presenting again would show the identical frame - skip the
	// draw and flip entirely so an idle app costs (nearly) nothing.
	static uint64_t last_presented_sig;
	if (!force_refresh && !pending_render && sig == last_presented_sig)
	{
		zc_mouse_x = gui_mouse_x();
		zc_mouse_y = gui_mouse_y();
		if (render_verify_enabled())
			render_verify(rti, clear_color, false);
		return false;
	}

	last_presented_sig = sig;

	al_set_target_backbuffer(display);
	al_clear_to_color(clear_color);
	render_tree_draw_item(rti, false);

	zc_mouse_x = gui_mouse_x();
	zc_mouse_y = gui_mouse_y();

	al_flip_display();
	if (render_verify_enabled())
		render_verify(rti, clear_color, true);

	return true;
#endif
}

void render_tree_draw_debug(RenderTreeItem* rti)
{
	std::vector<std::string> lines;
	ALLEGRO_FONT* a5font = get_zc_font_a5(font_lfont_l);
	render_tree_draw_item_debug(rti, 0, lines);
	int font_scale = 4;
	ALLEGRO_BITMAP* bitmap = al_get_backbuffer(all_get_display());
	render_text_lines(bitmap, a5font, lines, TextJustify::left, TextAlignment::top, font_scale);
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

std::pair<float, float> fit_scale(int resx, int resy, int w, int h, bool keep_aspect, bool force_integer)
{
	float xscale = (float)resx / w;
	float yscale = (float)resy / h;
	if (keep_aspect)
		xscale = yscale = std::min(xscale, yscale);
	if (force_integer)
	{
		xscale = std::max(1, (int)xscale);
		yscale = std::max(1, (int)yscale);
	}
	return {xscale, yscale};
}

Transform letterbox_transform(int resx, int resy, int w, int h, float xscale, float yscale)
{
	return {
		.x = (float)((int)(resx - w*xscale) / 2),
		.y = (float)((int)(resy - h*yscale) / 2),
		.xscale = xscale,
		.yscale = yscale,
	};
}

static int rti_gui_mouse_x()
{
	return gui_mouse_target().rel_mouse().first;
}

static int rti_gui_mouse_y()
{
	return gui_mouse_target().rel_mouse().second;
}

void render_tree_present(RenderTreeItem* root, ALLEGRO_COLOR clear_color, const std::function<void()>& configure)
{
	if (is_headless())
		return;

	static bool mouse_hooks_installed;
	if (!mouse_hooks_installed)
	{
		mouse_hooks_installed = true;
		gui_mouse_x = rti_gui_mouse_x;
		gui_mouse_y = rti_gui_mouse_y;
	}

	ALLEGRO_STATE oldstate;
	al_store_state(&oldstate, ALLEGRO_STATE_TARGET_BITMAP);

	// An open dialog repoints `screen` at its own layer's bitmap; the app's base layer
	// wants the real one.
	BITMAP* tmp = screen;
	if (zqdialog_bg_bmp)
		screen = zqdialog_bg_bmp;

	configure();

	if (render_get_debug())
	{
		// The debug overlay reflects live state, so always draw when it is up.
		al_set_target_backbuffer(all_get_display());
		al_clear_to_color(clear_color);
		render_tree_draw(root);
		render_tree_draw_debug(root);
		al_flip_display();
	}
	else
	{
		// Skips the draw and flip when nothing on screen has changed, so an idle app - a
		// paused game, an untouched editor, the launcher sitting there - costs (nearly)
		// nothing instead of re-compositing and re-presenting the same pixels 60 times a
		// second.
		render_tree_draw_and_flip(root, clear_color);
	}

	screen = tmp;
	al_restore_state(&oldstate);
}

void RenderTreeItem::prepare() {}
void RenderTreeItem::render(bool bitmap_resized)
{
	if (render_cb)
		render_cb(this, bitmap_resized);
}

LegacyBitmapRTI::LegacyBitmapRTI(std::string name, RenderTreeItem* parent) : RenderTreeItem(name, parent) {}

LegacyBitmapRTI::~LegacyBitmapRTI()
{
	if (owned && a4_bitmap)
		destroy_bitmap(a4_bitmap);
}

void LegacyBitmapRTI::prepare()
{
	// We convert from a4->a5 every frame, but freeze these render items to prevent doing unnecessary work.
	dirty = true;
	// Ideally this is set in the constructor, but `LegacyBitmapRTI` is used as static global variables and this
	// function requires the config to be loaded already.
	bitmap_flags = get_bitmap_create_flags(true);
}

// -render-timings: once a second, report what the legacy-bitmap content hashing costs
// and how many conversions (+ texture uploads) it saved, per render item.
static bool render_timings_enabled()
{
	static bool value = get_flag_bool("-render-timings").value_or(false);
	return value;
}

static void render_timings_record(const std::string& name, int64_t hash_ns, std::optional<int64_t> convert_ns)
{
	struct Entry
	{
		int64_t hash_ns = 0, convert_ns = 0;
		int hashes = 0, converts = 0, skips = 0;
	};
	static std::map<std::string, Entry> entries;
	static double last_report_time = al_get_time();

	auto& e = entries[name];
	e.hash_ns += hash_ns;
	e.hashes++;
	if (convert_ns)
	{
		e.converts++;
		e.convert_ns += *convert_ns;
	}
	else
		e.skips++;

	double now = al_get_time();
	if (now - last_report_time < 1.0)
		return;

	last_report_time = now;
	for (auto& [n, en] : entries)
	{
		fmt::print("[render-timings] {}: hashed {}x in {:.2f}ms (avg {:.3f}ms), converted {}x in {:.2f}ms, hash saved {} conversions\n",
			n, en.hashes, en.hash_ns / 1e6, en.hashes ? en.hash_ns / 1e6 / en.hashes : 0.0,
			en.converts, en.convert_ns / 1e6, en.skips);
	}
	entries.clear();
}

// -no-render-hash: diagnostic switch that restores the old behavior of converting
// every frame without content hashing, for A/B measurements.
static bool render_hash_disabled()
{
	static bool value = get_flag_bool("-render-hash").value_or(true);
	return !value;
}

void LegacyBitmapRTI::render(bool size_changed)
{
	if (bitmap && a4_bitmap && (!a4_bitmap_rendered_once || !freeze))
	{
		// A baked item's bitmap holds the conversion *plus* its children, so a skipped
		// conversion would leave the previous bake in place and the bake below would draw
		// the children on top of themselves.
		if (render_hash_disabled() || bakes_children(this))
		{
			// The bitmap no longer matches the cached signature, so if this item stops
			// baking, the next hash check must not skip over the leftover bake.
			a4_content_dest = nullptr;
			render_mark_dirty();
			all_set_transparent_palette_index(transparency_index);
			all_render_a5_bitmap(a4_bitmap, bitmap);
			all_set_transparent_palette_index(-1);
			a4_bitmap_rendered_once = true;
			return;
		}

		bool timings = render_timings_enabled();
		auto time_now = [&]() -> int64_t {
			if (!timings)
				return 0;
			return std::chrono::duration_cast<std::chrono::nanoseconds>(
				std::chrono::steady_clock::now().time_since_epoch()).count();
		};

		// The conversion result depends only on these inputs - when none of them have
		// changed since the last conversion, the texture already holds the right pixels,
		// so skip the conversion and (much slower) texture upload. The generation counter
		// covers palette changes and display events that may invalidate the texture.
		int64_t t0 = time_now();
		uint64_t hash = a4_pixel_hash(a4_bitmap);
		// Mixed in per item, since two items sharing one a4 bitmap can convert it differently.
		hash = hash_value(all_get_render_generation(), hash);
		hash = hash_value(render_refresh_count, hash);
		hash = hash_value(transparency_index, hash);
		int64_t t1 = time_now();
		if (a4_bitmap_rendered_once && !size_changed && bitmap == a4_content_dest && hash == a4_content_hash)
		{
			if (timings)
				render_timings_record(name, t1 - t0, std::nullopt);
			return;
		}

		a4_content_hash = hash;
		a4_content_dest = bitmap;
		render_mark_dirty();

		all_set_transparent_palette_index(transparency_index);
		all_render_a5_bitmap(a4_bitmap, bitmap);
		all_set_transparent_palette_index(-1);
		a4_bitmap_rendered_once = true;
		if (timings)
			render_timings_record(name, t1 - t0, time_now() - t1);
	}
}

std::pair<int, int> zc_get_mouse()
{
	return {zc_mouse_x, zc_mouse_y};
}
