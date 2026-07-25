#ifndef ZC_CRT_FILTER_H_
#define ZC_CRT_FILTER_H_

// Post-process CRT display filters, applied when drawing the game layer to the screen.
// Purely cosmetic: everything here operates on the final a5 blit, so replays, snapshots,
// and all game logic are unaffected by the selected filter.

#include <allegro5/allegro.h>
#include <utility>

enum class CrtFilterMode
{
	// Order is saved to the config file and menu indices - append only.
	none,
	easymode,
	geom,
	count,
};

CrtFilterMode crt_filter_get_mode();
void crt_filter_set_mode(CrtFilterMode mode);

// Shader for the current mode. nullptr when the mode is none, or when the shader can't be
// built on this display (no programmable pipeline, compile failure). Lazily built and cached.
ALLEGRO_SHADER* crt_filter_shader();

// Sets the current shader's uniforms. Must be called while the shader is active (al_use_shader).
// (src_w, src_h) is the game bitmap size; (out_w, out_h) is the on-screen destination size.
void crt_filter_set_uniforms(int src_w, int src_h, int out_w, int out_h);

// Web only, no-op elsewhere: match the display size to the canvas's real pixel size while a
// filter is active, and return it to the virtual screen size when none is. Call before creating
// the display and again whenever the mode changes.
void crt_filter_update_web_display_size();

// Whether the current mode spatially warps the image, requiring mouse correction.
bool crt_filter_warps_mouse();

// Maps a [0,1] uv position on the destination rect to the source uv displayed there,
// mirroring the shader's curvature. Identity for modes that don't warp.
std::pair<double, double> crt_filter_warp_uv(double u, double v);

#endif
