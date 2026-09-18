#ifndef CORE_VIEWPORT_FOLLOW_H_
#define CORE_VIEWPORT_FOLLOW_H_

#include "base/ints.h"
#include "base/zfix.h"
#include <cstdint>

// The farthest the follow logic's focus may sit from the target's center on each axis: half
// the viewport minus half a hero, since the target's center can get no closer than 8px to a
// region edge. As long as the total displacement (half the deadzone plus the lookahead
// offset) stays within this, the viewport is flush against a region edge whenever the target
// stands on one, which is the only place a scroll can start - so scrolling geometry never
// depends on the follow settings. Uses the base viewport height even in extended-height mode,
// which is merely conservative there.
constexpr int VIEWPORT_FOLLOW_MAX_OFFSET_X = 256/2 - 8;
constexpr int VIEWPORT_FOLLOW_MAX_OFFSET_Y = 176/2 - 8;
// Cap on the follow logic's easing speeds (lookahead, recentering), in pixels per frame.
// Anything at or above the max offset is already an instant snap, so this is just a sanity
// bound.
constexpr int VIEWPORT_FOLLOW_MAX_SPEED = 240;

// The valid range of each follow setting, shared by the editor dialogs and the script setters.
// The deadzone may use up the whole max offset, which keeps the guarantee above.
constexpr int VIEWPORT_FOLLOW_MAX_DEADZONE_W = 2*VIEWPORT_FOLLOW_MAX_OFFSET_X;
constexpr int VIEWPORT_FOLLOW_MAX_DEADZONE_H = 2*VIEWPORT_FOLLOW_MAX_OFFSET_Y;
constexpr int VIEWPORT_FOLLOW_MIN_LOOKAHEAD = INT8_MIN;
constexpr int VIEWPORT_FOLLOW_MAX_LOOKAHEAD = INT8_MAX;
constexpr int VIEWPORT_FOLLOW_MAX_RECENTER_DELAY = 65535;

// How the viewport follows its target in scrolling regions: the "follow settings". Init Data
// holds the quest-wide defaults; a dmap with dmfCUSTOM_VIEWPORT_FOLLOW supplies its own set
// instead. Only observable in scrolling regions.
struct ViewportFollowSettings
{
	// Size of the viewport deadzone box, in pixels. While the viewport target stays within this
	// box (centered on the viewport focus), the viewport doesn't move. 0 disables (viewport locked
	// to the target).
	byte deadzone_w = 0, deadzone_h = 0;
	// How far the viewport aims ahead of (positive) or behind (negative) the direction the
	// viewport target is moving on each axis (movement against the direction it faces is
	// ignored), in pixels, and how fast that offset shifts in pixels per frame (fractional; 0 to 1 feels
	// best).
	int8_t lookahead_x = 0, lookahead_y = 0;
	zfix lookahead_speed = 1;
	// Idle recentering: once the viewport target has stood still for `recenter_delay` frames,
	// the viewport eases back toward it at `recenter_speed` pixels per frame, as far as the
	// deadzone box around the (lookahead-shifted) aim point allows. 0 speed disables.
	zfix recenter_speed = 0;
	word recenter_delay = 30;
};

#endif
