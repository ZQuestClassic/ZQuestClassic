#ifndef _BASE_RENDER_TREE_H_
#define _BASE_RENDER_TREE_H_

#include "base/zc_alleg.h"
#include <vector>
#include <string>

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
};

class RenderTreeItem
{
public:
	std::string name;
	bool visible = true;
	// -1 for no transparency.
	int transparency_index = -1;
	ALLEGRO_BITMAP* bitmap = nullptr;
	BITMAP* a4_bitmap = nullptr;
	bool freeze_a4_bitmap_render = false;
	ALLEGRO_COLOR* tint = nullptr;
	bool owned = false;

	RenderTreeItem(std::string name, RenderTreeItem* parent = nullptr);
	~RenderTreeItem();

	void add_child(RenderTreeItem* child);
	void add_child_before(RenderTreeItem* child, RenderTreeItem* before_child);
	void remove_child(RenderTreeItem* child);
	const std::vector<RenderTreeItem*>& get_children() const;
	bool has_children() const;
	void handle_dirty();
	void mark_dirty();
	void set_transform(Transform new_transform);
	const Transform& get_transform() const;
	const Matrix& get_transform_matrix();
	std::pair<int, int> world_to_local(int x, int y);
	std::pair<int, int> local_to_world(int x, int y);
	std::pair<int, int> pos();
	std::pair<int, int> rel_mouse();

private:
	Transform transform;
	Matrix transform_matrix;
	bool transform_dirty = true;
	Matrix transform_matrix_inverse;
	bool transform_inverse_dirty = true;
	RenderTreeItem* parent = nullptr;
	std::vector<RenderTreeItem*> children;
};

enum class TextJustify {
	left,
	right,
};
enum class TextAlign {
	top,
	bottom,
};
void render_text_lines(ALLEGRO_FONT* font, std::vector<std::string> lines, TextJustify justify, TextAlign align, int scale);

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
void popup_zqdialog_start(int x = 0, int y = 0, int w = -1, int h = -1, int transp = 0xFF);
void popup_zqdialog_end();
void popup_zqdialog_start_a5();
void popup_zqdialog_end_a5();
RenderTreeItem* add_dlg_layer(int x = 0, int y = 0, int w = -1, int h = -1);
void remove_dlg_layer(RenderTreeItem* rti);

#endif
