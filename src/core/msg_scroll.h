#ifndef CORE_MSG_SCROLL_H_
#define CORE_MSG_SCROLL_H_

#include <optional>

// Message string scrolling state, shared between the player
// (zc/message_string.cpp) and the editor's string preview
// (zq/gui/msgstr_preview.cpp) so the math cannot drift between the two.
struct MsgScrollState
{
	bool can_scroll = false, segmented = false, active_scrolling = false;
	int scroll_pos = 0, target_scroll_pos = 0, max_visible_pos = 0;
	int body_height = 1;
	int active_speed = 1, passive_speed = 1;

	// Reset to the not-scrolling defaults.
	void clear();
	void set_body_height(int height, int margin_up, int margin_down);

	int cur_scroll() const { return can_scroll ? scroll_pos : 0; }
	int max_scroll() const;
	int bound(int v) const;
	bool at_max() const { return cur_scroll() == max_scroll(); }
	void clamp();

	// Grow the known content extent to include `pos` (rounded up to a full
	// segment in segmented mode). Returns true if the extent grew — callers
	// own a content bitmap that must be kept at least this tall.
	bool update_max_scroll(int pos);
	// Aim the scroll target so rows [pos, pos+h) are visible. Returns
	// update_max_scroll()'s result.
	bool ensure_scrolled_to(int pos, int h);
	// One frame of smooth-scrolling toward the target. Returns true if the
	// position changed.
	bool tick();
	// Manual scrolling from held/pressed input, clamped. No-op if up == down
	// or the direction has no room left.
	void scroll_input(bool up, bool down);
	// Mouse-wheel scrolling (editor preview). Positive delta scrolls down.
	void wheel(int delta);
	// For a line advance from content-row `ty` to `ty2`: if it crossed into a
	// new segment, grow the extent to the segment edge and return the
	// content-row the cursor should restart at. Segmented mode only.
	std::optional<int> segment_crossed(int ty, int ty2);
};

// Line-layout predicates shared between the player and the editor's string
// preview, so text flows the same way in both.
namespace msg_layout
{
	// A line at `cursor_y` has run past the bottom margin (only possible
	// when not scrolling).
	bool bottom_margin_clip(int cursor_y, int height, int margin_down,
		bool can_scroll, bool old_margins_qr, bool old_frame_qr);
	// A tile/menu-cursor of width `advance_w` doesn't fit on the current line.
	bool wrap_needed(int cursor_x, int hspace, int advance_w, int width,
		int margin_right);
	// The next word/character doesn't fit on the current line.
	bool char_wrap_needed(int cursor_x, int tlength, int width,
		int margin_right, bool wrap_flag, bool rem_word_is_space);
}

#endif
