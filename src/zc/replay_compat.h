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
bool replay_compat_dropset_reroll_rng_bug();
bool replay_compat_respawn_point_missing_dmg_conveyor_bug();
bool replay_compat_frozen_combos_tick_bug();

#endif
