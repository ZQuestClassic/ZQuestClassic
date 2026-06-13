#ifndef ZC_REPLAY_COMPAT_H_
#define ZC_REPLAY_COMPAT_H_

// Bugs that are fixed but not put behind a QR are often preserved in order to keep replays valid.
// These function all return true when the bug should be present.

bool replay_compat_pitslide_bug();
bool replay_compat_held_items_only_held_always_bug();
bool replay_compat_hookshot_snap_player_bug();
bool replay_compat_old_movement_off_by_one_bug();
bool replay_compat_charging_during_scroll_bug();
bool replay_compat_whistle_stuck_bug();

// Selects the zc::math mode for the current session, based on what math the
// active replay (if any) was created with. Called from replay_start,
// replay_continue, and replay_stop, so the mode tracks every replay state
// transition.
void replay_compat_setup_zc_maths();

#endif
