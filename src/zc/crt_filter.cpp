#include "zc/crt_filter.h"
#include "zalleg/zalleg.h"
#include "a5alleg.h"
#include "zconfig.h"
#include <algorithm>
#include <cmath>
#include <optional>
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

// The two filters are ports of shaders distributed by the libretro project (RetroArch),
// modified for ZQuest Classic: all tunable parameters are fixed at their upstream
// defaults, uniforms are adapted to allegro's shader conventions, HLSL translations were
// added, and (for crt-geom) the interlacing simulation and tilt are omitted and part of
// the vertex-stage math is moved to the CPU. Like the rest of ZQuest Classic, these
// ports are distributed under the GNU General Public License v3; the original notices
// of each shader are preserved below.
//
// crt-easymode:
//   CRT Shader by EasyMode
//   License: GPL
//   A flat CRT: Lanczos sampling, brightness-adaptive scanlines, RGB aperture grille.
//   https://github.com/libretro/glsl-shaders/blob/master/crt/shaders/crt-easymode.glsl
//   (Unversioned GPL grant; per GPLv3 section 14 it may be used under any published
//   version, here combined under GPLv3.)
//
// crt-geom:
//   CRT-interlaced
//   Copyright (C) 2010-2012 cgwg, Themaister and DOLLS
//   This program is free software; you can redistribute it and/or modify it under the
//   terms of the GNU General Public License as published by the Free Software
//   Foundation; either version 2 of the License, or (at your option) any later version.
//   (cgwg gave their consent to have the original version of this shader distributed
//   under the GPL in this message:
//     http://board.byuu.org/viewtopic.php?p=26075#p26075
//     "Feel free to distribute my shaders under the GPL. After all, the barrel
//     distortion code was taken from the Curvature shader, which is under the GPL.")
//   A curved CRT: barrel distortion, rounded corners, beam-profile scanlines, dot mask.
//   https://github.com/libretro/glsl-shaders/blob/master/crt/shaders/crt-geom.glsl
//
// Each shader is written twice: GLSL (desktop GL and GLES/web) and HLSL SM3.0 (the
// Direct3D backend used by default on Windows). The GLSL fragment shaders pair with
// allegro's default vertex shader, which provides `varying_texcoord` normalized over
// the source bitmap and `varying_color` holding the draw call's tint. The HLSL ones
// pair with allegro's default `vs_main`/`VS_OUTPUT` (`Input.TexCoord`/`Input.Color`).
//
// Each source also builds a second program with OVERLAY defined (see build_shader): the
// full-resolution overlay variant used to composite the title logo over the shaded layer.
// It shares every constant and all of the scanline/mask/curvature math with the base pass
// - keeping the two passes in a single source is what guarantees they can't drift apart -
// and differs only where #ifdef OVERLAY marks it: the sampling front-end (one direct
// sub-rect sample instead of the game-pixel reconstruction) and the output alpha.

static const char* easymode_glsl = R"(
#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

uniform sampler2D al_tex;
uniform vec2 u_source_size;
uniform vec2 u_output_size;
#ifdef OVERLAY
uniform vec4 u_sub_rect;
#endif
varying vec2 varying_texcoord;
varying vec4 varying_color;

const float SHARPNESS_H = 0.5;
const float SHARPNESS_V = 1.0;
const float MASK_STRENGTH = 0.3;
const float MASK_DOT_WIDTH = 1.0;
const float MASK_DOT_HEIGHT = 1.0;
const float MASK_STAGGER = 0.0;
const float MASK_SIZE = 1.0;
const float SCANLINE_STRENGTH = 1.0;
const float SCANLINE_BEAM_WIDTH_MIN = 1.5;
const float SCANLINE_BEAM_WIDTH_MAX = 1.5;
const float SCANLINE_BRIGHT_MIN = 0.35;
const float SCANLINE_BRIGHT_MAX = 0.65;
const float SCANLINE_CUTOFF = 400.0;
const float GAMMA_INPUT = 2.0;
const float GAMMA_OUTPUT = 1.8;
const float BRIGHT_BOOST = 1.2;
const float DILATION = 1.0;

#define FIX(c) max(abs(c), 1e-5)
#define PI 3.141592653589
#define TEX2D(c) dilate(texture2D(al_tex, c))

vec4 dilate(vec4 col)
{
    vec4 x = mix(vec4(1.0), col, DILATION);
    return col * x;
}

float curve_distance(float x, float sharp)
{
    float x_step = step(0.5, x);
    float curve = 0.5 - sqrt(0.25 - (x - x_step) * (x - x_step)) * sign(0.5 - x);
    return mix(x, curve, sharp);
}

mat4 get_color_matrix(vec2 co, vec2 dx)
{
    return mat4(TEX2D(co - dx), TEX2D(co), TEX2D(co + dx), TEX2D(co + 2.0 * dx));
}

vec3 filter_lanczos(vec4 coeffs, mat4 color_matrix)
{
    vec4 col        = color_matrix * coeffs;
    vec4 sample_min = min(color_matrix[1], color_matrix[2]);
    vec4 sample_max = max(color_matrix[1], color_matrix[2]);
    col = clamp(col, sample_min, sample_max);
    return col.rgb;
}

void main()
{
    vec3 col, col2;
    float out_alpha;

#ifndef OVERLAY
    vec2 dx     = vec2(1.0 / u_source_size.x, 0.0);
    vec2 dy     = vec2(0.0, 1.0 / u_source_size.y);
    vec2 pix_co = varying_texcoord * u_source_size - vec2(0.5, 0.5);
    vec2 tex_co = (floor(pix_co) + vec2(0.5, 0.5)) / u_source_size;
    vec2 dist   = fract(pix_co);
    float curve_x;

    curve_x = curve_distance(dist.x, SHARPNESS_H * SHARPNESS_H);

    vec4 coeffs = PI * vec4(1.0 + curve_x, curve_x, 1.0 - curve_x, 2.0 - curve_x);
    coeffs = FIX(coeffs);
    coeffs = 2.0 * sin(coeffs) * sin(coeffs * 0.5) / (coeffs * coeffs);
    coeffs /= dot(coeffs, vec4(1.0));

    col  = filter_lanczos(coeffs, get_color_matrix(tex_co, dx));
    col2 = filter_lanczos(coeffs, get_color_matrix(tex_co + dy, dx));

    col = mix(col, col2, curve_distance(dist.y, SHARPNESS_V));
    col = pow(col, vec3(GAMMA_INPUT / (DILATION + 1.0)));
    out_alpha = 1.0;
#else
    // The OVERLAY build composites a full-resolution element (the title logo) over the
    // shaded layer: identical scanline and mask math, but the color comes from one direct
    // sample of the overlay bitmap - occupying u_sub_rect of the layer, in source uv -
    // rather than the Lanczos game-pixel reconstruction, and its alpha passes through.
    // varying_texcoord has GL's origin (v grows upward) while u_sub_rect is measured from
    // the layer's top, so flip into top-down layer space for the rect mapping - and flip
    // back when sampling, since GL stores the overlay texture bottom-up too. The scanline
    // and mask terms below keep the raw texcoord on purpose: the base pass underneath uses
    // it as-is, and matching it keeps the two passes phase-identical. (The HLSL twin has
    // no flips - Direct3D's texture origin is already top-down.) The base pass never
    // exposes the flip because all its vertical terms are symmetric.
    vec2 suv = vec2(varying_texcoord.x, 1.0 - varying_texcoord.y);
    vec2 uv = (suv - u_sub_rect.xy) / u_sub_rect.zw;
    vec2 in_rect = step(vec2(0.0), uv) * step(uv, vec2(1.0));
    vec2 cuv = clamp(uv, 0.0, 1.0);
    vec4 samp = texture2D(al_tex, vec2(cuv.x, 1.0 - cuv.y)) * in_rect.x * in_rect.y;

    // dilate() matches the base pass's input curve; diverging leaves the overlay in a
    // different color space than the layer under it, washing the artwork out.
    // Known tradeoff: samp is premultiplied-alpha, and this gamma (plus the scanline and
    // mask math below) runs on it directly, slightly darkening partial-alpha edge pixels.
    // Un-premultiplying around the math isn't worth the cost for how thin those edges are.
    col = pow(dilate(samp).rgb, vec3(GAMMA_INPUT / (DILATION + 1.0)));
    out_alpha = samp.a;
#endif

    float luma        = dot(vec3(0.2126, 0.7152, 0.0722), col);
    float bright      = (max(col.r, max(col.g, col.b)) + luma) * 0.5;
    float scan_bright = clamp(bright, SCANLINE_BRIGHT_MIN, SCANLINE_BRIGHT_MAX);
    float scan_beam   = clamp(bright * SCANLINE_BEAM_WIDTH_MAX, SCANLINE_BEAM_WIDTH_MIN, SCANLINE_BEAM_WIDTH_MAX);
    // Scanlines can't be resolved below ~2x output scale (they only alias) - fade them out.
    float scan_strength = SCANLINE_STRENGTH * clamp(u_output_size.y / u_source_size.y - 2.0, 0.0, 1.0);
    // Space the beam a whole number of output pixels apart. Upstream puts exactly one cycle on
    // each source row, which leaves a fractional period unless the game is scaled by an integer,
    // and then every row samples a different part of the beam - the scanlines come out unevenly
    // spaced instead of even. Rounding to the nearest whole pixel is exact when the scale is
    // already an integer, so that case is unchanged.
    float scan_period = max(2.0, floor(u_output_size.y / u_source_size.y + 0.5));
    // clamp() is load-bearing: at the peak of the beam the cosine is -1 and this is 0, but in
    // floating point it lands just below, and pow() of a negative base is undefined - it returns
    // NaN, which draws that row of the screen black.
    float scan_beam_pos = clamp(cos(varying_texcoord.y * u_output_size.y / scan_period * 2.0 * PI) * 0.5 + 0.5, 0.0, 1.0);
    float scan_weight = 1.0 - pow(scan_beam_pos, scan_beam) * scan_strength;

    float mask   = 1.0 - MASK_STRENGTH;
    vec2 mod_fac = floor(varying_texcoord * u_output_size / vec2(MASK_SIZE, MASK_DOT_HEIGHT * MASK_SIZE));
    int dot_no   = int(mod((mod_fac.x + mod(mod_fac.y, 2.0) * MASK_STAGGER) / MASK_DOT_WIDTH, 3.0));
    vec3 mask_weight;

    if      (dot_no == 0) mask_weight = vec3(1.0,  mask, mask);
    else if (dot_no == 1) mask_weight = vec3(mask, 1.0,  mask);
    else                  mask_weight = vec3(mask, mask, 1.0);

    if (u_source_size.y >= SCANLINE_CUTOFF)
        scan_weight = 1.0;

    col2 = col;
    col *= vec3(scan_weight);
    col  = mix(col, col2, scan_bright);
    col *= mask_weight;
    col  = pow(col, vec3(1.0 / GAMMA_OUTPUT));

    // Multiplying by the vertex color is what applies the tint passed to the draw call
    // (used to dim the game while a menu or dialog is up). The default shader does the
    // same; a shader that ignores it silently drops every tint.
    gl_FragColor = vec4(col * BRIGHT_BOOST, out_alpha) * varying_color;
}
)";

static const char* easymode_hlsl = R"(
texture al_tex;
sampler2D s = sampler_state {
    texture = <al_tex>;
};
float2 u_source_size;
float2 u_output_size;
#ifdef OVERLAY
float4 u_sub_rect;
#endif

static const float SHARPNESS_H = 0.5;
static const float SHARPNESS_V = 1.0;
static const float MASK_STRENGTH = 0.3;
static const float MASK_DOT_WIDTH = 1.0;
static const float MASK_DOT_HEIGHT = 1.0;
static const float MASK_STAGGER = 0.0;
static const float MASK_SIZE = 1.0;
static const float SCANLINE_STRENGTH = 1.0;
static const float SCANLINE_BEAM_WIDTH_MIN = 1.5;
static const float SCANLINE_BEAM_WIDTH_MAX = 1.5;
static const float SCANLINE_BRIGHT_MIN = 0.35;
static const float SCANLINE_BRIGHT_MAX = 0.65;
static const float SCANLINE_CUTOFF = 400.0;
static const float GAMMA_INPUT = 2.0;
static const float GAMMA_OUTPUT = 1.8;
static const float BRIGHT_BOOST = 1.2;
static const float DILATION = 1.0;

#define FIX(c) max(abs(c), 1e-5)
#define PI 3.141592653589
#define TEX2D(c) dilate(tex2D(s, c))

float4 dilate(float4 col)
{
    float4 x = lerp(float4(1.0, 1.0, 1.0, 1.0), col, DILATION);
    return col * x;
}

float curve_distance(float x, float sharp)
{
    float x_step = step(0.5, x);
    float curve = 0.5 - sqrt(0.25 - (x - x_step) * (x - x_step)) * sign(0.5 - x);
    return lerp(x, curve, sharp);
}

float3 filter_lanczos(float4 coeffs, float4 c0, float4 c1, float4 c2, float4 c3)
{
    float4 col        = c0 * coeffs.x + c1 * coeffs.y + c2 * coeffs.z + c3 * coeffs.w;
    float4 sample_min = min(c1, c2);
    float4 sample_max = max(c1, c2);
    col = clamp(col, sample_min, sample_max);
    return col.rgb;
}

float4 ps_main(VS_OUTPUT Input) : COLOR0
{
    float2 co = Input.TexCoord;
    float3 col, col2;
    float out_alpha;

#ifndef OVERLAY
    float2 dx     = float2(1.0 / u_source_size.x, 0.0);
    float2 dy     = float2(0.0, 1.0 / u_source_size.y);
    float2 pix_co = co * u_source_size - float2(0.5, 0.5);
    float2 tex_co = (floor(pix_co) + float2(0.5, 0.5)) / u_source_size;
    float2 dist   = frac(pix_co);
    float curve_x;

    curve_x = curve_distance(dist.x, SHARPNESS_H * SHARPNESS_H);

    float4 coeffs = PI * float4(1.0 + curve_x, curve_x, 1.0 - curve_x, 2.0 - curve_x);
    coeffs = FIX(coeffs);
    coeffs = 2.0 * sin(coeffs) * sin(coeffs * 0.5) / (coeffs * coeffs);
    coeffs /= dot(coeffs, float4(1.0, 1.0, 1.0, 1.0));

    col  = filter_lanczos(coeffs, TEX2D(tex_co - dx), TEX2D(tex_co), TEX2D(tex_co + dx), TEX2D(tex_co + 2.0 * dx));
    col2 = filter_lanczos(coeffs, TEX2D(tex_co - dx + dy), TEX2D(tex_co + dy), TEX2D(tex_co + dx + dy), TEX2D(tex_co + 2.0 * dx + dy));

    col = lerp(col, col2, curve_distance(dist.y, SHARPNESS_V));
    float gamma_in = GAMMA_INPUT / (DILATION + 1.0);
    col = pow(col, float3(gamma_in, gamma_in, gamma_in));
    out_alpha = 1.0;
#else
    // See the GLSL version. No coordinate flips here: Direct3D's texture origin is
    // already top-down, matching u_sub_rect.
    float2 uv = (co - u_sub_rect.xy) / u_sub_rect.zw;
    float2 in_rect = step(float2(0.0, 0.0), uv) * step(uv, float2(1.0, 1.0));
    float4 samp = tex2D(s, clamp(uv, 0.0, 1.0)) * in_rect.x * in_rect.y;

    float gamma_in = GAMMA_INPUT / (DILATION + 1.0);
    col = pow(dilate(samp).rgb, float3(gamma_in, gamma_in, gamma_in));
    out_alpha = samp.a;
#endif

    float luma        = dot(float3(0.2126, 0.7152, 0.0722), col);
    float bright      = (max(col.r, max(col.g, col.b)) + luma) * 0.5;
    float scan_bright = clamp(bright, SCANLINE_BRIGHT_MIN, SCANLINE_BRIGHT_MAX);
    float scan_beam   = clamp(bright * SCANLINE_BEAM_WIDTH_MAX, SCANLINE_BEAM_WIDTH_MIN, SCANLINE_BEAM_WIDTH_MAX);
    // Scanlines can't be resolved below ~2x output scale (they only alias) - fade them out.
    float scan_strength = SCANLINE_STRENGTH * clamp(u_output_size.y / u_source_size.y - 2.0, 0.0, 1.0);
    // Space the beam a whole number of output pixels apart. Upstream puts exactly one cycle on
    // each source row, which leaves a fractional period unless the game is scaled by an integer,
    // and then every row samples a different part of the beam - the scanlines come out unevenly
    // spaced instead of even. Rounding to the nearest whole pixel is exact when the scale is
    // already an integer, so that case is unchanged.
    float scan_period = max(2.0, floor(u_output_size.y / u_source_size.y + 0.5));
    // clamp() is load-bearing: at the peak of the beam the cosine is -1 and this is 0, but in
    // floating point it lands just below, and pow() of a negative base is undefined - it returns
    // NaN, which draws that row of the screen black.
    float scan_beam_pos = clamp(cos(co.y * u_output_size.y / scan_period * 2.0 * PI) * 0.5 + 0.5, 0.0, 1.0);
    float scan_weight = 1.0 - pow(scan_beam_pos, scan_beam) * scan_strength;

    float mask     = 1.0 - MASK_STRENGTH;
    float2 mod_fac = floor(co * u_output_size / float2(MASK_SIZE, MASK_DOT_HEIGHT * MASK_SIZE));
    int dot_no     = (int)fmod((mod_fac.x + fmod(mod_fac.y, 2.0) * MASK_STAGGER) / MASK_DOT_WIDTH, 3.0);
    float3 mask_weight;

    if      (dot_no == 0) mask_weight = float3(1.0,  mask, mask);
    else if (dot_no == 1) mask_weight = float3(mask, 1.0,  mask);
    else                  mask_weight = float3(mask, mask, 1.0);

    if (u_source_size.y >= SCANLINE_CUTOFF)
        scan_weight = 1.0;

    col2 = col;
    col *= scan_weight;
    col  = lerp(col, col2, scan_bright);
    col *= mask_weight;
    float gamma_out = 1.0 / GAMMA_OUTPUT;
    col  = pow(col, float3(gamma_out, gamma_out, gamma_out));

    // See the GLSL version: Input.Color is the tint from the draw call.
    return float4(col * BRIGHT_BOOST, out_alpha) * Input.Color;
}
)";

static const char* geom_glsl = R"(
#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
// The sphere-intersection math visibly bands and can NaN at mediump precision, so
// refuse to build - the failure is logged and the game renders unfiltered (which also
// disables the matching mouse warp, since crt_filter_warps_mouse checks the shader).
#error crt-geom requires highp fragment precision
#endif
#endif

uniform sampler2D al_tex;
uniform vec2 u_source_size;
uniform vec2 u_output_size;
uniform vec3 u_stretch;
uniform vec2 u_sinangle;
uniform vec2 u_cosangle;
#ifdef OVERLAY
uniform vec4 u_sub_rect;
#endif
varying vec2 varying_texcoord;
varying vec4 varying_color;

const float CRTgamma = 2.4;
const float monitorgamma = 2.2;
const float d = 1.6;
const float R = 2.0;
const float cornersize = 0.03;
const float cornersmooth = 1000.0;
const float overscan_x = 100.0;
const float overscan_y = 100.0;
const float DOTMASK = 0.3;
const float scanline_weight = 0.3;
const float lum = 0.0;
const vec2 aspect = vec2(1.0, 0.75);

#define FIX(c) max(abs(c), 1e-5)
#define PI 3.141592653589
#define TEX2D(c) pow(texture2D(al_tex, (c)), vec4(CRTgamma))

float intersect(vec2 xy)
{
    float A = dot(xy, xy) + d * d;
    float B = 2.0 * (R * (dot(xy, u_sinangle) - d * u_cosangle.x * u_cosangle.y) - d * d);
    float C = d * d + 2.0 * R * d * u_cosangle.x * u_cosangle.y;
    return (-B - sqrt(B * B - 4.0 * A * C)) / (2.0 * A);
}

vec2 bkwtrans(vec2 xy)
{
    float c = intersect(xy);
    vec2 point = vec2(c) * xy;
    point -= vec2(-R) * u_sinangle;
    point /= vec2(R);
    vec2 tang = u_sinangle / u_cosangle;
    vec2 poc = point / u_cosangle;
    float A = dot(tang, tang) + 1.0;
    float B = -2.0 * dot(poc, tang);
    float C = dot(poc, poc) - 1.0;
    float a = (-B + sqrt(B * B - 4.0 * A * C)) / (2.0 * A);
    vec2 uv = (point - a * u_sinangle) / u_cosangle;
    float r = FIX(R * acos(a));
    return uv * r / sin(r / R);
}

vec2 crt_transform(vec2 coord)
{
    coord = (coord - vec2(0.5)) * aspect * u_stretch.z + u_stretch.xy;
    return (bkwtrans(coord) / vec2(overscan_x / 100.0, overscan_y / 100.0) / aspect + vec2(0.5));
}

float corner(vec2 coord)
{
    coord = (coord - vec2(0.5)) * vec2(overscan_x / 100.0, overscan_y / 100.0) + vec2(0.5);
    coord = min(coord, vec2(1.0) - coord) * aspect;
    vec2 cdist = vec2(cornersize);
    coord = (cdist - min(coord, cdist));
    float dist = sqrt(dot(coord, coord));
    return clamp((cdist.x - dist) * cornersmooth, 0.0, 1.0) * 1.0001;
}

vec4 scanlineWeights(float dist, vec4 color)
{
    vec4 wid = 2.0 + 2.0 * pow(color, vec4(4.0));
    vec4 weights = vec4(dist / scanline_weight);
    return (lum + 1.4) * exp(-pow(weights * inversesqrt(0.5 * wid), wid)) / (0.6 + 0.2 * wid);
}

void main()
{
    vec2 xy = crt_transform(varying_texcoord);
    float cval = corner(xy);

    vec2 ratio_scale = xy * u_source_size - vec2(0.5);
    float filter_ = u_source_size.y / u_output_size.y;
    vec2 uv_ratio = fract(ratio_scale);

    vec4 col, col2;
    float out_alpha;

#ifndef OVERLAY
    xy = (floor(ratio_scale) + vec2(0.5)) / u_source_size;

    vec4 coeffs = PI * vec4(1.0 + uv_ratio.x, uv_ratio.x, 1.0 - uv_ratio.x, 2.0 - uv_ratio.x);
    coeffs = FIX(coeffs);
    coeffs = 2.0 * sin(coeffs) * sin(coeffs / 2.0) / (coeffs * coeffs);
    coeffs /= dot(coeffs, vec4(1.0));

    vec2 one = vec2(1.0) / u_source_size;
    col = clamp(mat4(
            TEX2D(xy + vec2(-one.x, 0.0)),
            TEX2D(xy),
            TEX2D(xy + vec2(one.x, 0.0)),
            TEX2D(xy + vec2(2.0 * one.x, 0.0))) * coeffs,
            0.0, 1.0);
    col2 = clamp(mat4(
            TEX2D(xy + vec2(-one.x, one.y)),
            TEX2D(xy + vec2(0.0, one.y)),
            TEX2D(xy + one),
            TEX2D(xy + vec2(2.0 * one.x, one.y))) * coeffs,
            0.0, 1.0);
    out_alpha = 1.0;
#else
    // The OVERLAY build composites a full-resolution element over the shaded layer - see
    // easymode. Same curvature, corner and scanline geometry, but one direct sample of
    // the overlay bitmap stands in for both scanline rows. GL's texture-space y runs
    // bottom-up while u_sub_rect is top-down, and the warp commutes with the flip
    // (vertically symmetric up to the 0.001 tilt offsets in u_sinangle - a sub-pixel
    // error), so flip after warping for the rect mapping and again when sampling. (The
    // HLSL twin has no flips - Direct3D is already top-down.)
    vec2 xyd = vec2(xy.x, 1.0 - xy.y);
    vec2 uv = (xyd - u_sub_rect.xy) / u_sub_rect.zw;
    vec2 in_rect = step(vec2(0.0), uv) * step(uv, vec2(1.0));
    vec2 cuv = clamp(uv, 0.0, 1.0);
    vec4 samp = texture2D(al_tex, vec2(cuv.x, 1.0 - cuv.y)) * in_rect.x * in_rect.y;
    col = pow(samp, vec4(CRTgamma));
    col2 = col;
    out_alpha = samp.a * cval;
#endif

    vec4 weights  = scanlineWeights(uv_ratio.y, col);
    vec4 weights2 = scanlineWeights(1.0 - uv_ratio.y, col2);

    uv_ratio.y = uv_ratio.y + 1.0 / 3.0 * filter_;
    weights  = (weights + scanlineWeights(uv_ratio.y, col)) / 3.0;
    weights2 = (weights2 + scanlineWeights(abs(1.0 - uv_ratio.y), col2)) / 3.0;
    uv_ratio.y = uv_ratio.y - 2.0 / 3.0 * filter_;
    weights  = weights + scanlineWeights(abs(uv_ratio.y), col) / 3.0;
    weights2 = weights2 + scanlineWeights(abs(1.0 - uv_ratio.y), col2) / 3.0;

    vec3 mul_res = (col * weights + col2 * weights2).rgb * vec3(cval);

    float mod_factor = varying_texcoord.x * u_output_size.x;
    vec3 dotMaskWeights = mix(
        vec3(1.0, 1.0 - DOTMASK, 1.0),
        vec3(1.0 - DOTMASK, 1.0, 1.0 - DOTMASK),
        floor(mod(mod_factor, 2.0)));
    mul_res *= dotMaskWeights;

    mul_res = pow(mul_res, vec3(1.0 / monitorgamma));

    // See easymode above: the vertex color carries the draw call's tint.
    gl_FragColor = vec4(mul_res, out_alpha) * varying_color;
}
)";

static const char* geom_hlsl = R"(
texture al_tex;
sampler2D s = sampler_state {
    texture = <al_tex>;
};
float2 u_source_size;
float2 u_output_size;
float3 u_stretch;
float2 u_sinangle;
float2 u_cosangle;
#ifdef OVERLAY
float4 u_sub_rect;
#endif

static const float CRTgamma = 2.4;
static const float monitorgamma = 2.2;
static const float d = 1.6;
static const float R = 2.0;
static const float cornersize = 0.03;
static const float cornersmooth = 1000.0;
static const float overscan_x = 100.0;
static const float overscan_y = 100.0;
static const float DOTMASK = 0.3;
static const float scanline_weight = 0.3;
static const float lum = 0.0;
static const float2 aspect = float2(1.0, 0.75);

#define FIX(c) max(abs(c), 1e-5)
#define PI 3.141592653589
#define TEX2D(c) pow(tex2D(s, (c)), float4(CRTgamma, CRTgamma, CRTgamma, CRTgamma))

float intersect(float2 xy)
{
    float A = dot(xy, xy) + d * d;
    float B = 2.0 * (R * (dot(xy, u_sinangle) - d * u_cosangle.x * u_cosangle.y) - d * d);
    float C = d * d + 2.0 * R * d * u_cosangle.x * u_cosangle.y;
    return (-B - sqrt(B * B - 4.0 * A * C)) / (2.0 * A);
}

float2 bkwtrans(float2 xy)
{
    float c = intersect(xy);
    float2 pnt = c * xy;
    pnt -= (-R) * u_sinangle;
    pnt /= R;
    float2 tang = u_sinangle / u_cosangle;
    float2 poc = pnt / u_cosangle;
    float A = dot(tang, tang) + 1.0;
    float B = -2.0 * dot(poc, tang);
    float C = dot(poc, poc) - 1.0;
    float a = (-B + sqrt(B * B - 4.0 * A * C)) / (2.0 * A);
    float2 uv = (pnt - a * u_sinangle) / u_cosangle;
    float r = FIX(R * acos(a));
    return uv * r / sin(r / R);
}

float2 crt_transform(float2 coord)
{
    coord = (coord - float2(0.5, 0.5)) * aspect * u_stretch.z + u_stretch.xy;
    return (bkwtrans(coord) / float2(overscan_x / 100.0, overscan_y / 100.0) / aspect + float2(0.5, 0.5));
}

float corner(float2 coord)
{
    coord = (coord - float2(0.5, 0.5)) * float2(overscan_x / 100.0, overscan_y / 100.0) + float2(0.5, 0.5);
    coord = min(coord, float2(1.0, 1.0) - coord) * aspect;
    float2 cdist = float2(cornersize, cornersize);
    coord = (cdist - min(coord, cdist));
    float dist = sqrt(dot(coord, coord));
    return clamp((cdist.x - dist) * cornersmooth, 0.0, 1.0) * 1.0001;
}

float4 scanlineWeights(float dist, float4 color)
{
    float4 wid = 2.0 + 2.0 * pow(color, float4(4.0, 4.0, 4.0, 4.0));
    float4 weights = float4(dist, dist, dist, dist) / scanline_weight;
    return (lum + 1.4) * exp(-pow(weights * rsqrt(0.5 * wid), wid)) / (0.6 + 0.2 * wid);
}

float4 ps_main(VS_OUTPUT Input) : COLOR0
{
    float2 xy = crt_transform(Input.TexCoord);
    float cval = corner(xy);

    float2 ratio_scale = xy * u_source_size - float2(0.5, 0.5);
    float filter_ = u_source_size.y / u_output_size.y;
    float2 uv_ratio = frac(ratio_scale);

    float4 col, col2;
    float out_alpha;

#ifndef OVERLAY
    xy = (floor(ratio_scale) + float2(0.5, 0.5)) / u_source_size;

    float4 coeffs = PI * float4(1.0 + uv_ratio.x, uv_ratio.x, 1.0 - uv_ratio.x, 2.0 - uv_ratio.x);
    coeffs = FIX(coeffs);
    coeffs = 2.0 * sin(coeffs) * sin(coeffs / 2.0) / (coeffs * coeffs);
    coeffs /= dot(coeffs, float4(1.0, 1.0, 1.0, 1.0));

    float2 one = float2(1.0, 1.0) / u_source_size;
    col = clamp(
            TEX2D(xy + float2(-one.x, 0.0)) * coeffs.x +
            TEX2D(xy) * coeffs.y +
            TEX2D(xy + float2(one.x, 0.0)) * coeffs.z +
            TEX2D(xy + float2(2.0 * one.x, 0.0)) * coeffs.w,
            0.0, 1.0);
    col2 = clamp(
            TEX2D(xy + float2(-one.x, one.y)) * coeffs.x +
            TEX2D(xy + float2(0.0, one.y)) * coeffs.y +
            TEX2D(xy + one) * coeffs.z +
            TEX2D(xy + float2(2.0 * one.x, one.y)) * coeffs.w,
            0.0, 1.0);
    out_alpha = 1.0;
#else
    // See the GLSL version. No coordinate flips here: Direct3D is already top-down.
    float2 uv = (xy - u_sub_rect.xy) / u_sub_rect.zw;
    float2 in_rect = step(float2(0.0, 0.0), uv) * step(uv, float2(1.0, 1.0));
    float4 samp = tex2D(s, clamp(uv, 0.0, 1.0)) * in_rect.x * in_rect.y;
    col = pow(samp, float4(CRTgamma, CRTgamma, CRTgamma, CRTgamma));
    col2 = col;
    out_alpha = samp.a * cval;
#endif

    float4 weights  = scanlineWeights(uv_ratio.y, col);
    float4 weights2 = scanlineWeights(1.0 - uv_ratio.y, col2);

    uv_ratio.y = uv_ratio.y + 1.0 / 3.0 * filter_;
    weights  = (weights + scanlineWeights(uv_ratio.y, col)) / 3.0;
    weights2 = (weights2 + scanlineWeights(abs(1.0 - uv_ratio.y), col2)) / 3.0;
    uv_ratio.y = uv_ratio.y - 2.0 / 3.0 * filter_;
    weights  = weights + scanlineWeights(abs(uv_ratio.y), col) / 3.0;
    weights2 = weights2 + scanlineWeights(abs(1.0 - uv_ratio.y), col2) / 3.0;

    float3 mul_res = (col * weights + col2 * weights2).rgb * cval;

    float mod_factor = Input.TexCoord.x * u_output_size.x;
    float3 dotMaskWeights = lerp(
        float3(1.0, 1.0 - DOTMASK, 1.0),
        float3(1.0 - DOTMASK, 1.0, 1.0 - DOTMASK),
        floor(fmod(mod_factor, 2.0)));
    mul_res *= dotMaskWeights;

    float gamma_out = 1.0 / monitorgamma;
    mul_res = pow(mul_res, float3(gamma_out, gamma_out, gamma_out));

    // See easymode above: Input.Color is the tint from the draw call.
    return float4(mul_res, out_alpha) * Input.Color;
}
)";

// CPU mirror of crt-geom's curvature math (with tilt fixed at 0), used both to compute
// the `u_stretch` uniform and to invert the mouse position through the same warp the
// shader applies. Constants must match the geom shaders above.
namespace geom
{
	constexpr double d = 1.6;
	constexpr double R = 2.0;
	constexpr double aspect_x = 1.0, aspect_y = 0.75;
	// Upstream evaluates sin/cos of the (zero) tilt angles and adds 0.001 to dodge
	// degenerate math. Keep the offsets so the numbers match the shader exactly.
	constexpr double sinangle_x = 0.001, sinangle_y = 0.001;
	constexpr double cosangle_x = 1.001, cosangle_y = 1.001;

	struct Vec2
	{
		double x, y;
	};

	static double intersect(Vec2 xy)
	{
		double A = xy.x * xy.x + xy.y * xy.y + d * d;
		double B = 2.0 * (R * ((xy.x * sinangle_x + xy.y * sinangle_y) - d * cosangle_x * cosangle_y) - d * d);
		double C = d * d + 2.0 * R * d * cosangle_x * cosangle_y;
		return (-B - std::sqrt(B * B - 4.0 * A * C)) / (2.0 * A);
	}

	static Vec2 bkwtrans(Vec2 xy)
	{
		double c = intersect(xy);
		Vec2 point = {c * xy.x + R * sinangle_x, c * xy.y + R * sinangle_y};
		point.x /= R;
		point.y /= R;
		Vec2 tang = {sinangle_x / cosangle_x, sinangle_y / cosangle_y};
		Vec2 poc = {point.x / cosangle_x, point.y / cosangle_y};
		double A = tang.x * tang.x + tang.y * tang.y + 1.0;
		double B = -2.0 * (poc.x * tang.x + poc.y * tang.y);
		double C = poc.x * poc.x + poc.y * poc.y - 1.0;
		double a = (-B + std::sqrt(B * B - 4.0 * A * C)) / (2.0 * A);
		Vec2 uv = {(point.x - a * sinangle_x) / cosangle_x, (point.y - a * sinangle_y) / cosangle_y};
		double r = std::max(std::abs(R * std::acos(std::clamp(a, -1.0, 1.0))), 1e-5);
		double scale = r / std::sin(r / R);
		return {uv.x * scale, uv.y * scale};
	}

	static Vec2 fwtrans(Vec2 uv)
	{
		double r = std::max(std::sqrt(uv.x * uv.x + uv.y * uv.y), 1e-5);
		double s = std::sin(r / R) / r;
		uv = {uv.x * s, uv.y * s};
		double x = 1.0 - std::cos(r / R);
		double D = d / R + x * cosangle_x * cosangle_y + uv.x * sinangle_x + uv.y * sinangle_y;
		return {d * (uv.x * cosangle_x - x * sinangle_x) / D, d * (uv.y * cosangle_y - x * sinangle_y) / D};
	}

	struct Stretch
	{
		double x, y, z;
	};

	static Stretch maxscale()
	{
		double denom = 1.0 + R / d * cosangle_x * cosangle_y;
		Vec2 c = bkwtrans({-R * sinangle_x / denom, -R * sinangle_y / denom});
		Vec2 a = {0.5 * aspect_x, 0.5 * aspect_y};
		Vec2 lo = {fwtrans({-a.x, c.y}).x / aspect_x, fwtrans({c.x, -a.y}).y / aspect_y};
		Vec2 hi = {fwtrans({+a.x, c.y}).x / aspect_x, fwtrans({c.x, +a.y}).y / aspect_y};
		return {
			(hi.x + lo.x) * aspect_x * 0.5,
			(hi.y + lo.y) * aspect_y * 0.5,
			std::max(hi.x - lo.x, hi.y - lo.y),
		};
	}

	static const Stretch& stretch()
	{
		static Stretch s = maxscale();
		return s;
	}
}

static std::optional<CrtFilterMode> loaded_mode;

CrtFilterMode crt_filter_get_mode()
{
	if (!loaded_mode)
	{
		int value = zc_get_config("zeldadx", "crt_filter", 0);
		if (value < 0 || value >= (int)CrtFilterMode::count)
			value = 0;
		loaded_mode = (CrtFilterMode)value;
	}
	return *loaded_mode;
}

void crt_filter_set_mode(CrtFilterMode mode)
{
	loaded_mode = mode;
	zc_set_config("zeldadx", "crt_filter", (int)mode);
	crt_filter_update_web_display_size();
}

void crt_filter_update_web_display_size()
{
#ifdef __EMSCRIPTEN__
	int w = 0, h = 0;
	if (crt_filter_get_mode() != CrtFilterMode::none)
	{
		int size[2] = {};
		EM_ASM({
			var s = window.zcCanvasSize ? window.zcCanvasSize() : null;
			HEAP32[$0 >> 2] = s ? s.bufW : 0;
			HEAP32[($0 >> 2) + 1] = s ? s.bufH : 0;
		}, size);
		w = size[0];
		h = size[1];
	}

	ALLEGRO_DISPLAY* display = all_get_display();
	if (!display)
	{
		// No display yet: just record the size to create it at.
		zalleg_set_web_display_size(w, h);
		return;
	}

	// With no filter the canvas goes back to the virtual screen size, which CSS scales up.
	// Neither obvious spelling of that works: SCREEN_W/SCREEN_H are gfx_driver->w/h, which track
	// the *display*, so the shrink-back condition below could never be true; and the global
	// `screen` is repointed at a popup's own bitmap while one is open - which is always, since
	// this runs from the CRT Filter menu callback, so it read the menu's size instead.
	if (w <= 0 || h <= 0)
	{
		auto [vw, vh] = zalleg_get_virtual_display_size();
		w = vw;
		h = vh;
	}
	// Must go through al_resize_display rather than sizing the canvas from JS: SDL reports the
	// drawable size from its own bookkeeping (window->w * pixel_ratio), not from the canvas, so
	// a canvas resized behind its back leaves allegro drawing at the old size into the corner
	// of the new backbuffer. main.js owns the CSS size; this owns the backbuffer.
	if (w != al_get_display_width(display) || h != al_get_display_height(display))
		al_resize_display(display, w, h);
#endif
}

#ifdef __EMSCRIPTEN__
// Called from web/main.js whenever the page viewport changes. While a filter is active the
// backbuffer has to keep matching the size the canvas is displayed at, so the display has to
// be resized along with the page - most visibly when going fullscreen.
extern "C" void zc_web_display_size_changed()
{
	crt_filter_update_web_display_size();
}
#endif

// Each filter's source builds two programs: the base pass, and (with `overlay` set, which
// prepends `#define OVERLAY`) the full-resolution overlay variant. Both languages'
// preprocessors handle the define, and neither source carries a #version line that would
// have to stay first.
static ALLEGRO_SHADER* build_shader(const char* name, const char* glsl_source, const char* hlsl_source, bool overlay = false)
{
	ALLEGRO_SHADER_PLATFORM platform = ALLEGRO_SHADER_AUTO;
	ALLEGRO_SHADER* shader = al_create_shader(platform);
	if (shader && al_get_shader_platform(shader) == ALLEGRO_SHADER_HLSL)
	{
		// These shaders exceed the ps_2_0 limits allegro compiles plain HLSL with.
		al_destroy_shader(shader);
		platform = ALLEGRO_SHADER_HLSL_SM_3_0;
		shader = al_create_shader(platform);
	}
	if (!shader)
	{
		Z_message("Could not create shader for CRT filter '%s' - was the display created with ALLEGRO_PROGRAMMABLE_PIPELINE?\n", name);
		return nullptr;
	}

	bool is_glsl = al_get_shader_platform(shader) == ALLEGRO_SHADER_GLSL;
	std::string pixel_source = is_glsl ? glsl_source : hlsl_source;
	if (overlay)
		pixel_source = "#define OVERLAY\n" + pixel_source;
	if (!al_attach_shader_source(shader, ALLEGRO_VERTEX_SHADER, al_get_default_shader_source(platform, ALLEGRO_VERTEX_SHADER)) ||
		!al_attach_shader_source(shader, ALLEGRO_PIXEL_SHADER, pixel_source.c_str()) ||
		!al_build_shader(shader))
	{
		Z_message("Failed to build CRT filter '%s': %s\n", name, al_get_shader_log(shader));
		al_destroy_shader(shader);
		return nullptr;
	}

	return shader;
}

ALLEGRO_SHADER* crt_filter_shader()
{
	CrtFilterMode mode = crt_filter_get_mode();
	if (mode == CrtFilterMode::none)
		return nullptr;

	static ALLEGRO_SHADER* shaders[(int)CrtFilterMode::count];
	static bool attempted[(int)CrtFilterMode::count];
	int index = (int)mode;
	if (!attempted[index])
	{
		attempted[index] = true;
		switch (mode)
		{
			case CrtFilterMode::easymode:
				shaders[index] = build_shader("easymode", easymode_glsl, easymode_hlsl);
				break;
			case CrtFilterMode::geom:
				shaders[index] = build_shader("geom", geom_glsl, geom_hlsl);
				break;
			default:
				break;
		}
	}
	return shaders[index];
}

ALLEGRO_SHADER* crt_filter_overlay_shader()
{
	CrtFilterMode mode = crt_filter_get_mode();
	if (mode == CrtFilterMode::none)
		return nullptr;

	static ALLEGRO_SHADER* shaders[(int)CrtFilterMode::count];
	static bool attempted[(int)CrtFilterMode::count];
	int index = (int)mode;
	if (!attempted[index])
	{
		attempted[index] = true;
		switch (mode)
		{
			case CrtFilterMode::easymode:
				shaders[index] = build_shader("easymode overlay", easymode_glsl, easymode_hlsl, true);
				break;
			case CrtFilterMode::geom:
				shaders[index] = build_shader("geom overlay", geom_glsl, geom_hlsl, true);
				break;
			default:
				break;
		}
	}
	return shaders[index];
}

void crt_filter_set_uniforms(int src_w, int src_h, int out_w, int out_h)
{
	float source_size[2] = {(float)src_w, (float)src_h};
	float output_size[2] = {(float)out_w, (float)out_h};
	al_set_shader_float_vector("u_source_size", 2, source_size, 1);
	al_set_shader_float_vector("u_output_size", 2, output_size, 1);

	if (crt_filter_get_mode() == CrtFilterMode::geom)
	{
		auto& s = geom::stretch();
		float stretch[3] = {(float)s.x, (float)s.y, (float)s.z};
		float sinangle[2] = {(float)geom::sinangle_x, (float)geom::sinangle_y};
		float cosangle[2] = {(float)geom::cosangle_x, (float)geom::cosangle_y};
		al_set_shader_float_vector("u_stretch", 3, stretch, 1);
		al_set_shader_float_vector("u_sinangle", 2, sinangle, 1);
		al_set_shader_float_vector("u_cosangle", 2, cosangle, 1);
	}
}

void crt_filter_set_overlay_uniforms(int src_w, int src_h, int out_w, int out_h,
	float rect_x, float rect_y, float rect_w, float rect_h)
{
	crt_filter_set_uniforms(src_w, src_h, out_w, out_h);
	float sub_rect[4] = {rect_x, rect_y, rect_w, rect_h};
	al_set_shader_float_vector("u_sub_rect", 4, sub_rect, 1);
}

bool crt_filter_warps_mouse()
{
	return crt_filter_get_mode() == CrtFilterMode::geom && crt_filter_shader() != nullptr;
}

std::pair<double, double> crt_filter_warp_uv(double u, double v)
{
	if (!crt_filter_warps_mouse())
		return {u, v};

	// Same mapping as the geom shader's crt_transform(). Clamp so the sphere math stays
	// defined when the mouse is far outside the game area.
	double cu = std::clamp(u, -0.5, 1.5);
	double cv = std::clamp(v, -0.5, 1.5);
	auto& s = geom::stretch();
	geom::Vec2 coord = {
		(cu - 0.5) * geom::aspect_x * s.z + s.x,
		(cv - 0.5) * geom::aspect_y * s.z + s.y,
	};
	geom::Vec2 t = geom::bkwtrans(coord);
	double ru = t.x / geom::aspect_x + 0.5;
	double rv = t.y / geom::aspect_y + 0.5;
	if (std::isnan(ru) || std::isnan(rv))
		return {u, v};
	return {ru, rv};
}
