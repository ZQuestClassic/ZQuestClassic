#ifndef DIALOG_VIEWPORT_FOLLOW_HELP_H_
#define DIALOG_VIEWPORT_FOLLOW_HELP_H_

#include <string>

// Help text for the viewport follow settings, shared by the Init Data dialog (the quest-wide
// settings) and the DMap editor (a DMap's own set) so the two can't drift apart.

enum class ViewportFollowSetting
{
	DeadzoneWidth,
	DeadzoneHeight,
	LookaheadX,
	LookaheadY,
	LookaheadSpeed,
	RecenterSpeed,
	RecenterDelay,
};

// `for_dmap` picks the closing line on how scripts change the setting.
inline std::string viewport_follow_help(ViewportFollowSetting setting, bool for_dmap)
{
	std::string text;
	std::string name;
	switch (setting)
	{
		case ViewportFollowSetting::DeadzoneWidth:
			name = "DeadzoneWidth";
			text = "While the player (or other viewport target) stays within a box of this width"
				" centered on the viewport's focus point (its center, except where a region edge"
				" stops the viewport), the viewport does not move horizontally. Once they push"
				" past the box's edge, the viewport moves just enough to keep them on it. With a"
				" Lookahead, the box applies to the shifted aim point rather than the player's own"
				" position. 0 keeps the viewport locked to the player.";
			break;
		case ViewportFollowSetting::DeadzoneHeight:
			name = "DeadzoneHeight";
			text = "While the player (or other viewport target) stays within a box of this height"
				" centered on the viewport's focus point (its center, except where a region edge"
				" stops the viewport), the viewport does not move vertically. Once they push"
				" past the box's edge, the viewport moves just enough to keep them on it. With a"
				" Lookahead, the box applies to the shifted aim point rather than the player's own"
				" position. 0 keeps the viewport locked to the player.";
			break;
		case ViewportFollowSetting::LookaheadX:
			name = "LookaheadX";
			text = "How far the viewport aims ahead of the direction the player (or other viewport"
				" target) is moving horizontally, in pixels. Positive values lead in front (more"
				" frontal view); negative values trail behind. Each axis has its own distance,"
				" so diagonal movement leads by both."
				"\n\nThe offset builds while the target moves, at the Lookahead Speed, and holds"
				" while they stand still, so the viewport never drifts or reacts to turning in"
				" place. Movement against the direction they face is ignored, so being knocked"
				" back or pushed backward doesn't swing the viewport (sideways movement still"
				" counts). It is capped so the viewport can still reach the region edges,"
				" dropped by warps (whistle warps included) and camera cutscene effects"
				" (rebuilding as the target moves), and carried through scroll transitions.";
			break;
		case ViewportFollowSetting::LookaheadY:
			name = "LookaheadY";
			text = "How far the viewport aims ahead of the direction the player (or other viewport"
				" target) is moving vertically, in pixels. Works like Lookahead X; since the"
				" viewport is shorter than it is wide, a smaller distance usually feels right here."
				"\n\nIn sideview, jumping and falling count as vertical movement, so a jump shifts"
				" the viewport too. To keep jumps from moving it, give sideview DMaps their own"
				" follow settings with this set to 0.";
			break;
		case ViewportFollowSetting::LookaheadSpeed:
			name = "LookaheadSpeed";
			text = "How fast the lookahead offset shifts toward the Lookahead X/Y distance, in"
				" pixels per frame (0 freezes it in place). Values between 0 and 1 are"
				" recommended: while the target moves, this is added to their own speed, so"
				" anything higher makes the viewport visibly outrun them.";
			break;
		case ViewportFollowSetting::RecenterSpeed:
			name = "RecenterSpeed";
			text = "Once the player (or other viewport target) has stood still for the Recenter"
				" Delay, the viewport eases back toward them at this many pixels per frame until"
				" they are centered again. With a Lookahead, the viewport only moves toward them"
				" as far as the deadzone allows, and never further ahead. 0 (the default)"
				" disables recentering; values around 1 feel gentle. Any movement cancels it."
				" Has no effect without a deadzone, since the viewport then never leaves them.";
			break;
		case ViewportFollowSetting::RecenterDelay:
			name = "RecenterDelay";
			text = "How many frames the player (or other viewport target) must stand still before"
				" the viewport starts recentering. A short delay keeps brief pauses mid-walk from"
				" nudging the viewport. 30 by default.";
			break;
	}

	text += "\n\nOnly observable in scrolling regions. ";
	if (for_dmap)
		text += "Scripts can change this DMap's value via dmapdata->Viewport" + name
			+ ", and the value in effect via Viewport->" + name + ".";
	else
	{
		if (setting == ViewportFollowSetting::DeadzoneWidth)
			text += "A DMap can supply its own follow settings instead ('Customize Viewport Follow"
				" Settings' in the DMap editor). ";
		text += "Scripts can change this at runtime via Viewport->" + name + ".";
	}
	return text;
}

#endif
