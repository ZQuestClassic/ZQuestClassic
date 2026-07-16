#include "core/msg_scroll.h"
#include "base/general.h"
#include "base/util.h"

void MsgScrollState::clear()
{
	can_scroll = segmented = active_scrolling = false;
	scroll_pos = target_scroll_pos = max_visible_pos = 0;
	body_height = 1;
	active_speed = 1;
	passive_speed = 1;
}

void MsgScrollState::set_body_height(int height, int margin_up, int margin_down)
{
	body_height = zc_max(1, height - margin_up - margin_down);
}

int MsgScrollState::max_scroll() const
{
	if (!can_scroll)
		return 0;
	if (max_visible_pos <= body_height)
		return 0;
	return zc_max(0, max_visible_pos - body_height);
}

int MsgScrollState::bound(int v) const
{
	return vbound(v, 0, max_scroll());
}

void MsgScrollState::clamp()
{
	scroll_pos = bound(scroll_pos);
	target_scroll_pos = bound(target_scroll_pos);
}

bool MsgScrollState::update_max_scroll(int pos)
{
	if (segmented && (pos % body_height))
	{
		// round up to next multiple of body_height
		pos += body_height - (pos % body_height);
	}

	if (pos <= max_visible_pos)
		return false;
	max_visible_pos = pos;
	return true;
}

bool MsgScrollState::ensure_scrolled_to(int pos, int h)
{
	int epos = pos + h;

	bool grew = update_max_scroll(epos);

	if (target_scroll_pos + body_height < epos)
	{
		if (segmented)
			target_scroll_pos = pos - (pos % body_height);
		else target_scroll_pos = epos - body_height;
	}
	else if (target_scroll_pos > pos)
	{
		if (segmented)
			target_scroll_pos = pos - (pos % body_height);
		else target_scroll_pos = pos;
	}

	return grew;
}

bool MsgScrollState::tick()
{
	if (!can_scroll || scroll_pos == target_scroll_pos)
		return false;
	uint spd = active_scrolling ? active_speed : passive_speed;
	if (!spd)
		scroll_pos = target_scroll_pos;
	else
		scroll_pos = util::move_towards(scroll_pos, target_scroll_pos, spd);
	if (scroll_pos == target_scroll_pos)
		active_scrolling = false;
	return true;
}

void MsgScrollState::scroll_input(bool up, bool down)
{
	if (up == down)
		return; // cancel out
	if (down ? scroll_pos >= max_scroll() : scroll_pos <= 0)
		return; // invalid scroll direction
	if (segmented)
	{
		active_scrolling = true;
		target_scroll_pos = bound(target_scroll_pos + (down ? body_height : -body_height));
	}
	else
	{
		scroll_pos = bound(scroll_pos + (down ? active_speed : -active_speed));
		target_scroll_pos = scroll_pos;
	}
}

void MsgScrollState::wheel(int delta)
{
	if (!delta || !can_scroll)
		return;
	if (segmented)
	{
		if (scroll_pos != target_scroll_pos)
			return; // only segmented scroll from still
		if (target_scroll_pos % body_height)
			target_scroll_pos -= (target_scroll_pos % body_height);
		if (delta > 0 && target_scroll_pos < max_scroll())
			target_scroll_pos += body_height;
		else if (delta < 0 && target_scroll_pos > 0)
			target_scroll_pos -= body_height;
	}
	else
	{
		target_scroll_pos = bound(target_scroll_pos + 4*delta);
	}
}

std::optional<int> MsgScrollState::segment_crossed(int ty, int ty2)
{
	if (!segmented)
		return std::nullopt;
	if (ty / body_height == (ty2-1) / body_height)
		return std::nullopt;
	max_visible_pos = ty - (ty % body_height) + body_height;
	return max_visible_pos;
}
