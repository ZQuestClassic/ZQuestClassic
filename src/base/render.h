#ifndef _BASE_RENDER_TREE_H_
#define _BASE_RENDER_TREE_H_

#include "allegro5/color.h"
#include "base/zc_alleg.h"
#include "base/headers.h"
#include <vector>
#include <string>
#include <functional>

extern unsigned char info_opacity;

struct Transform
{
	bool operator==(const Transform&) const = default;

	int x, y;
	float xscale = 1;
	float yscale = 1;
};

struct Matrix
{
	static Matrix Identity()
	{
		return {.d {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
	}
	static Matrix Translate(float x, float y)
	{
		return {.d {{1, 0, x}, {0, 1, y}, {0, 0, 1}}};
	}
	static Matrix Scale(float w, float h)
	{
		return {.d {{w, 0, 0}, {0, h, 0}, {0, 0, 1}}};
	}
	static Matrix Rotate(double radians)
	{
		return {.d {{(float)cos(radians), (float)sin(radians), 0}, {(float)-sin(radians), (float)cos(radians), 0}, {0, 0, 1}}};
	}

	float d[3][3];

	std::pair<int, int> apply(int x, int y) const
	{
		return {
			d[0][0] * x + d[0][1] * y + d[0][2],
			d[1][0] * x + d[1][1] * y + d[1][2]
		};
	}

	Matrix mul(const Matrix& other) const
	{
		Matrix r;
		auto& a = this->d;
		auto& b = other.d;

		r.d[0][0] = a[0][0]*b[0][0]+a[0][1]*b[1][0]+a[0][2]*b[2][0];
		r.d[0][1] = a[0][0]*b[0][1]+a[0][1]*b[1][1]+a[0][2]*b[2][1];
		r.d[0][2] = a[0][0]*b[0][2]+a[0][1]*b[1][2]+a[0][2]*b[2][2];

		r.d[1][0] = a[1][0]*b[0][0]+a[1][1]*b[1][0]+a[1][2]*b[2][0];
		r.d[1][1] = a[1][0]*b[0][1]+a[1][1]*b[1][1]+a[1][2]*b[2][1];
		r.d[1][2] = a[1][0]*b[0][2]+a[1][1]*b[1][2]+a[1][2]*b[2][2];

		r.d[2][0] = a[2][0]*b[0][0]+a[2][1]*b[1][0]+a[2][2]*b[2][0];
		r.d[2][1] = a[2][0]*b[0][1]+a[2][1]*b[1][1]+a[2][2]*b[2][1];
		r.d[2][2] = a[2][0]*b[0][2]+a[2][1]*b[1][2]+a[2][2]*b[2][2];

		return r;
	}

	Matrix inverse() const
	{
		Matrix r;
		double det = d[0][0] * (d[1][1] * d[2][2] - d[2][1] * d[1][2]) -
					d[0][1] * (d[1][0] * d[2][2] - d[1][2] * d[2][0]) +
					d[0][2] * (d[1][0] * d[2][1] - d[1][1] * d[2][0]);
		double invdet = 1 / det;
		r.d[0][0] = (d[1][1] * d[2][2] - d[2][1] * d[1][2]) * invdet;
		r.d[0][1] = (d[0][2] * d[2][1] - d[0][1] * d[2][2]) * invdet;
		r.d[0][2] = (d[0][1] * d[1][2] - d[0][2] * d[1][1]) * invdet;
		r.d[1][0] = (d[1][2] * d[2][0] - d[1][0] * d[2][2]) * invdet;
		r.d[1][1] = (d[0][0] * d[2][2] - d[0][2] * d[2][0]) * invdet;
		r.d[1][2] = (d[1][0] * d[0][2] - d[0][0] * d[1][2]) * invdet;
		r.d[2][0] = (d[1][0] * d[2][1] - d[2][0] * d[1][1]) * invdet;
		r.d[2][1] = (d[2][0] * d[0][1] - d[0][0] * d[2][1]) * invdet;
		r.d[2][2] = (d[0][0] * d[1][1] - d[1][0] * d[0][1]) * invdet;
		return r;
	}

	ALLEGRO_TRANSFORM to_allegro_transform()
	{
		// Our transform is different from Allegro's.
		// Transpose it, and add a third spatial dimension.
		ALLEGRO_TRANSFORM t;

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				t.m[i][j] = d[j][i];
			}
		}

		t.m[3][0] = t.m[2][0];
		t.m[3][1] = t.m[2][1];
		t.m[3][2] = 0;
		t.m[3][3] = t.m[2][2];
		t.m[2][0] = 0;
		t.m[2][1] = 0;
		t.m[2][2] = 1;
		t.m[2][3] = 0;

		return t;
	}
};

enum
{
	RTI_TY_NONE,
	RTI_TY_DIALOG_A4,
	RTI_TY_DIALOG_A5,
	RTI_TY_POPUP_MENU,
};
class RenderTreeItem
{
public:
	string name;
	uint type = RTI_TY_NONE;
	bool visible = true;
	// -1 for no transparency.
	int transparency_index = -1;
	ALLEGRO_BITMAP* bitmap = nullptr;
	int bitmap_flags = -1;
	ALLEGRO_COLOR* tint = nullptr;
	bool owned = false, owned_tint = false;
	// When true, a4_bitmap -> bitmap will not happen, and neither will `cb` be called.
	// `bitmap` will still be rendered.
	bool freeze = false;
	bool dirty = true;
	bool skip_tint = false;

	RenderTreeItem(std::string name, RenderTreeItem* parent = nullptr);
	virtual ~RenderTreeItem();

	void remove();
	void add_child(RenderTreeItem* child);
	void add_child_before(RenderTreeItem* child, RenderTreeItem* before_child);
	void remove_child(RenderTreeItem* child);
	void remove_children();
	std::vector<RenderTreeItem*>& get_children();
	std::vector<RenderTreeItem*> const& get_children() const;
	bool has_children() const;
	bool has_parent() const;
	void set_size(int width, int height);
	void set_transform(Transform new_transform);
	const Transform& get_transform() const;
	const Matrix& get_transform_matrix();
	std::pair<int, int> world_to_local(int x, int y);
	std::pair<int, int> local_to_world(int x, int y);
	std::pair<int, int> pos();
	std::pair<int, int> rel_mouse();

	// Every frame, each visible render item will call prepare on itself and all its direct children.
	// This function should be used to update any properties of this render item (including visibility).
	virtual void prepare();
	// Every frame, each visible render item w/ `freeze` false and `dirty` on will call this function.
	// The `bitmap` will be created (or recreated) to match the `width` and `height`. It will be set as
	// the target bitmap and cleared before `render` is called, so all draw calls made within `render` will
	// draw to `bitmap`.
	virtual void render(bool bitmap_resized);

	int width = 0;
	int height = 0;

private:
	Transform transform;
	Matrix transform_matrix;
	bool transform_dirty = true;
	Matrix transform_matrix_inverse;
	bool transform_inverse_dirty = true;
	RenderTreeItem* parent = nullptr;
	std::vector<RenderTreeItem*> children;

	void handle_dirty_transform();
	void mark_transform_dirty();
};

class CustomRTI : public RenderTreeItem
{
public:
	CustomRTI(std::string name) : RenderTreeItem(name) {}
	CustomRTI(std::string name, std::function<void()> prepare_cb, std::function<void()> render_cb);
	~CustomRTI();

	std::function<void()> prepare_cb;
	std::function<void()> render_cb;

protected:
	void prepare();
	void render(bool);
};

class LegacyBitmapRTI : public RenderTreeItem
{
public:
	LegacyBitmapRTI(std::string name, RenderTreeItem* parent = nullptr);
	~LegacyBitmapRTI();

	BITMAP* a4_bitmap = nullptr;
	bool a4_bitmap_rendered_once = false;

protected:
	void prepare();
	void render(bool);
};

enum class TextJustify {
	left,
	right,
};
enum class TextAlign {
	top,
	bottom,
};
void render_text(ALLEGRO_BITMAP* bitmap, ALLEGRO_FONT* font, std::string text, int x, int y, int scale, ALLEGRO_COLOR color = al_map_rgb_f(1,1,1), ALLEGRO_COLOR bgcolor = al_map_rgba_f(0, 0, 0, 0));
void render_text_lines(ALLEGRO_BITMAP* bitmap, ALLEGRO_FONT* font, std::vector<std::string> lines, TextJustify justify, TextAlign align, int scale);

namespace MouseSprite
{
	#define MAX_MOUSESPRITE 35
	void assign(int index, BITMAP* spr, int xf = 1, int yf = 1);
	bool set(int index);
	void clear(int index);
};

extern RenderTreeItem rti_dialogs;
extern ALLEGRO_COLOR AL5_INVIS,AL5_BLACK,AL5_WHITE,AL5_YELLOW,
	AL5_PINK,AL5_DGRAY,AL5_LGRAY,AL5_BLUE,AL5_LRED,AL5_DRED,
	AL5_LGREEN,AL5_LAQUA;
int get_bitmap_create_flags(bool preserve_texture);
void set_bitmap_create_flags(bool preserve_texture);
void clear_a5_bmp(ALLEGRO_BITMAP* bmp = nullptr);
void clear_a5_bmp(ALLEGRO_COLOR col, ALLEGRO_BITMAP* bmp = nullptr);
ALLEGRO_BITMAP* create_a5_bitmap(int w, int h);
void render_tree_draw(RenderTreeItem* rti);
void render_tree_draw_debug(RenderTreeItem* rti);
void render_set_debug(bool debug);
bool render_get_debug();

void _init_render(int fmt);
uint32_t get_backend_a5_col(RGB const& c);
uint32_t repl_a5_backend_alpha(uint32_t back_col, unsigned char a);
void load_palette(uint32_t* backpal, ALLEGRO_COLOR* backcols, PALETTE pal, int start = 0, int end = 255);
void zc_set_palette(PALETTE pal);
void zc_set_palette_range(PALETTE pal, int start, int end, bool=false);
void render_a4_a5(BITMAP* src,int sx,int sy,int dx,int dy,int w,int h,int maskind = 0,uint32_t* backpal = nullptr);

extern BITMAP* zqdialog_bg_bmp;
extern ALLEGRO_COLOR* override_dlg_tint;
void zqdialog_set_skiptint(bool skipTint);
void zqdialog_name(string const& name);
void zqdialog_tag(uint tagid);
void get_zqdialog_xy(int& x, int& y);
void popup_zqdialog_start(string name, uint tagid, int x = 0, int y = 0, int w = -1, int h = -1, int transp = 0xFF);
void popup_zqdialog_start(int x = 0, int y = 0, int w = -1, int h = -1, int transp = 0xFF);
void popup_zqdialog_end();
void popup_zqdialog_start_a5();
void popup_zqdialog_end_a5();
RenderTreeItem* add_dlg_layer(int x = 0, int y = 0, int w = -1, int h = -1);
void remove_dlg_layer(RenderTreeItem* rti);
void reload_dialog_tint();
ALLEGRO_COLOR& get_dlg_tint();
void pause_dlg_tint(bool pause);
bool dlg_tint_paused();

void update_throttle_counter();
void throttleFPS(int32_t cap);

//

void clear_tooltip();

std::pair<int, int> zc_get_mouse();

int window_mouse_x();
int window_mouse_y();

#endif
