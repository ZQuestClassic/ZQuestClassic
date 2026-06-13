#include "zc/replay_compat.h"
#include "base/zc_math.h"
#include "zc/replay.h"

// Returns true if the active replay was created in a build less recent than 2.55.patch.
//
// For bugs also fixed in 2.55, we must also check the ZC version rather than just the replay
// version, since in 2.55 the replay version is frozen at 39.
static bool check_2_55(int patch)
{
	if (!replay_is_active())
		return false;

	auto zc_version_created = replay_get_zc_version_created();
	if (!zc_version_created.well_formed)
		return false;

	if (zc_version_created.major < 2)
		return true; // Replays didn't exist at this point ... but whatever.
	if (zc_version_created.major == 2 && zc_version_created.minor < 55)
		return true; // Replays didn't exist at this point ... but whatever.
	if (zc_version_created.major == 2 && zc_version_created.minor == 55 && zc_version_created.patch < patch)
		return true;

	return false;
}

bool replay_compat_pitslide_bug()
{
	return check_2_55(11);
}

bool replay_compat_held_items_only_held_always_bug()
{
	return check_2_55(11);
}

bool replay_compat_hookshot_snap_player_bug()
{
	return check_2_55(12);
}

bool replay_compat_old_movement_off_by_one_bug()
{
	return check_2_55(12);
}

bool replay_compat_charging_during_scroll_bug()
{
	return check_2_55(13);
}

// https://discord.com/channels/876899628556091432/1479336319674220575
bool replay_compat_whistle_stuck_bug()
{
	return check_2_55(14);
}

// Dropsets that roll into 'Nothing' (e.g. via the SMARTDROPS_NOTHING qr or when all real
// drops are removed) used to still consume a zc_oldrand() roll before returning nothing.
// The new code early-exits without rolling, so older replays must keep the extra roll.
bool replay_compat_dropset_reroll_rng_bug()
{
	return check_2_55(15);
}

// New respawn points didn't account for damage combos or conveyors when deciding whether a
// spot was "safe". The new code adds those checks, so older replays must keep the old
// (incomplete) safety check to stay valid.
bool replay_compat_respawn_point_missing_dmg_conveyor_bug()
{
	return check_2_55(15);
}

// Shooter/crumble (and other timed) combos used to keep ticking in cpos_update() even while
// the screen was frozen (freeze/freezemsg/freezeff). The new code skips ticking when frozen,
// so older replays must keep ticking to stay valid.
bool replay_compat_frozen_combos_tick_bug()
{
	return check_2_55(15);
}

// Trig, inverse trig, and log/pow switched from libm (and the replay-only Q15
// trig) to the deterministic tables in zc_math.cpp in replay version 59 /
// ZC 2.55.15. This owns the entire mode policy so zc_math.cpp needs no
// knowledge of the replay system.
// https://discord.com/channels/876899628556091432/1509803675605008384
void replay_compat_setup_zc_maths()
{
	using zc::math::MathsMode;

	// legacy_maths can only be true when a replay is active: check_2_55 returns
	// false with no replay, so an inactive session always resolves to New. In
	// 2.55 the replay version is frozen at 39 (never bumped to 59), so which
	// math an active replay uses is keyed off the recording build's ZC version
	// rather than the replay version.
	bool legacy_maths = check_2_55(15);
	if (!legacy_maths)
		zc::math::set_maths_mode(MathsMode::New);
	else if (replay_version_check(21))
		zc::math::set_maths_mode(MathsMode::LegacyReplayV21);
	else if (replay_version_check(4))
		zc::math::set_maths_mode(MathsMode::LegacyReplayV4);
	else
		zc::math::set_maths_mode(MathsMode::LegacyReplayV0);
}
