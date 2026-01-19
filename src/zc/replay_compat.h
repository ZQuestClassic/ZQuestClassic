#ifndef ZC_REPLAY_COMPAT_H_
#define ZC_REPLAY_COMPAT_H_

// Bugs that are fixed but not put behind a QR are often preserved in order to keep replays valid.
// These function all return true when the bug should be present.

bool replay_compat_pitslide_bug();
bool replay_compat_held_items_only_held_always_bug();
bool replay_compat_hookshot_snap_player_bug();
bool replay_compat_old_movement_off_by_one_bug();

#endif
